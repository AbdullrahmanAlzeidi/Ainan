#include "environment/Environment.h"
#include "environment/ParticleSystem.h"
#include "environment/RadialLight.h"
#include "environment/SpotLight.h"
#include "json/json.hpp"
#include "Sprite.h"
#include "LitSprite.h"
#include "Model.h"
#include "CameraObject.h"

using json = nlohmann::json;

#define JSON_ARRAY_TO_VEC4(arr) glm::vec4(arr[0], arr[1], arr[2], arr[3])
#define JSON_ARRAY_TO_VEC3(arr) glm::vec3(arr[0], arr[1], arr[2])
#define JSON_ARRAY_TO_VEC2(arr) glm::vec2(arr[0], arr[1])
#define JSON_ARRAY_TO_MAT4(arr) glm::make_mat4(arr.data());

namespace Ainan {

	static void ParticleSystemFromJson(Environment* env, json& data, std::string id);
	static void SpriteFromJson(Environment* env,json& data, std::string id);
	static void LitSpriteFromJson(Environment* env,json& data, std::string id);
	static void ModelFromJson(Environment* env,json& data, std::string id);
	static void RadialLightFromJson(Environment* env, json& data, std::string id);
	static void SpotLightFromJson(Environment* env, json& data, std::string id);
	static void CameraFromJson(Environment* env, json& data, std::string id);
	static void SettingsFromJson(Environment* env, json& data);

	Environment* LoadEnvironment(const std::string& path)
	{
		json data;

		//we are assuming the .env file is in environment's top folder
		AssetManager::Init(std::filesystem::path(path).parent_path());

		try 
		{
			data = json::parse(AssetManager::ReadEntireTextFile(path));
		}
		catch (std::exception)
		{
			AINAN_LOG_ERROR("Cannot load environemnt. Environment file is invalid, loaded empty project instead.");
			return new Environment(Environment::Default());
		}

		Environment* env = new Environment;

		env->Name = data["EnvironmentName"].get<std::string>();

		SettingsFromJson(env, data);

		int32_t objectCount = data["ObjectCount"].get<int>();

		for (size_t i = 0; i < objectCount; i++)
		{
			std::string id = "obj" + std::to_string(i) + "_";
			std::string typeStr = data[id + "Type"].get<std::string>();

			EnvironmentObjectType type = StringToEnvironmentObjectType(typeStr);

			switch (type)
			{
			case ParticleSystemType:
				ParticleSystemFromJson(env, data, id);
				break;

			case RadialLightType:
				RadialLightFromJson(env, data, id);
				break;

			case SpotLightType:
				SpotLightFromJson(env, data, id);
				break;

			case SpriteType:
				SpriteFromJson(env, data, id);
				break;

			case LitSpriteType:
				LitSpriteFromJson(env, data, id);
				break;

			case ModelType:
				ModelFromJson(env, data, id);
				break;
			
			case CameraType:
				CameraFromJson(env, data, id);
				break;

			default:
				AINAN_LOG_FATAL("Invalid object enum");
				break;
			}
		}

		SkyMode mode = SkyModeFromStr(data["SkyboxMode"].get<std::string>());
		glm::vec4 color = JSON_ARRAY_TO_VEC4(data["SkyboxColor"].get<std::vector<float>>());
		std::array<std::filesystem::path, 6> paths;
		for (size_t i = 0; i < paths.size(); i++)
			paths[i] = data["SkyboxTexturePath" + std::to_string(i)].get<std::string>();
		env->EnvSkybox.Init(mode, color, paths);

		return env;
	}

	void SettingsFromJson(Environment* env, json& data)
	{
		env->BlurEnabled = data["BlurEnabled"].get<bool>();
		env->BlurRadius = data["BlurRadius"].get<float>();
	}

	void ParticleSystemFromJson(Environment* env, json& data, std::string id)
	{
		//create particle system
		std::unique_ptr<ParticleSystem> ps = std::make_unique<ParticleSystem>();

		//populate with data

		ps->ID.FromString(data[id + "UUID"].get<std::string>());
		ps->m_Name = data[id + "Name"].get<std::string>();
		ps->Customizer.Mode = GetTextAsMode(data[id + "Mode"].get<std::string>());
		ps->Customizer.m_ParticlesPerSecond = data[id + "ParticlesPerSecond"].get<float>();
		ps->Customizer.m_SpawnPosition = JSON_ARRAY_TO_VEC2(data[id + "SpawnPosition"].get<std::vector<float>>());
		ps->Customizer.m_LineLength = data[id + "LineLength"].get<float>();
		ps->Customizer.m_LineAngle = data[id + "LineAngle"].get<float>();
		ps->Customizer.m_CircleRadius = data[id + "CircleRadius"].get<float>();

		//Scale data
		ps->Customizer.m_ScaleCustomizer.m_RandomScale = data[id + "IsStartingScaleRandom"].get<bool>();
		ps->Customizer.m_ScaleCustomizer.m_MinScale = data[id + "MinScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_MaxScale = data[id + "MaxScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_DefinedScale = data[id + "DefinedScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_EndScale = data[id + "EndScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_InterpolationType = StringToInterpolationType(data[id + "ScaleInterpolationType"].get<std::string>());

		//Color data
		ps->Customizer.m_ColorCustomizer.StartColor = JSON_ARRAY_TO_VEC4(data[id + "DefinedColor"].get<std::vector<float>>());
		ps->Customizer.m_ColorCustomizer.EndColor = JSON_ARRAY_TO_VEC4(data[id + "EndColor"].get<std::vector<float>>());
		ps->Customizer.m_ColorCustomizer.m_InterpolationType = StringToInterpolationType(data[id + "ColorInterpolationType"].get<std::string>());

		//Lifetime data
		ps->Customizer.m_LifetimeCustomizer.m_RandomLifetime = data[id + "IsLifetimeRandom"].get<bool>();
		ps->Customizer.m_LifetimeCustomizer.m_DefinedLifetime = data[id + "DefinedLifetime"].get<float>();
		ps->Customizer.m_LifetimeCustomizer.m_MinLifetime = data[id + "MinLifetime"].get<float>();
		ps->Customizer.m_LifetimeCustomizer.m_MaxLifetime = data[id + "MaxLifetime"].get<float>();

		//Velocity data
		ps->Customizer.m_VelocityCustomizer.m_RandomVelocity = data[id + "IsStartingVelocityRandom"].get<bool>();
		ps->Customizer.m_VelocityCustomizer.m_DefinedVelocity = JSON_ARRAY_TO_VEC2(data[id + "DefinedVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MinVelocity = JSON_ARRAY_TO_VEC2(data[id + "MinVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MaxVelocity = JSON_ARRAY_TO_VEC2(data[id + "MaxVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.CurrentVelocityLimitType =  StringToLimitType(data[id + "VelocityLimitType"].get<std::string>());
		ps->Customizer.m_VelocityCustomizer.m_MinNormalVelocityLimit = data[id + "MinNormalVelocityLimit"].get<float>();
		ps->Customizer.m_VelocityCustomizer.m_MaxNormalVelocityLimit = data[id + "MaxNormalVelocityLimit"].get<float>();
		ps->Customizer.m_VelocityCustomizer.m_MinPerAxisVelocityLimit = JSON_ARRAY_TO_VEC2(data[id + "MinPerAxisVelocityLimit"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MaxPerAxisVelocityLimit = JSON_ARRAY_TO_VEC2(data[id + "MaxPerAxisVelocityLimit"].get<std::vector<float>>());

		//Noise data
		ps->Customizer.m_NoiseCustomizer.m_NoiseEnabled = data[id + "NoiseEnabled"].get<bool>();
		ps->Customizer.m_NoiseCustomizer.m_NoiseStrength = data[id + "NoiseStrength"].get<float>();
		ps->Customizer.m_NoiseCustomizer.m_NoiseFrequency = data[id + "NoiseFrequency"].get<float>();
		ps->Customizer.m_NoiseCustomizer.NoiseTarget = NoiseCustomizer::NoiseApplyTargetVal(data[id + "NoiseTarget"].get<std::string>());
		ps->Customizer.m_NoiseCustomizer.NoiseInterpolationMode = NoiseCustomizer::NoiseInterpolationModeVal(data[id + "NoiseInterpolationMode"].get<std::string>());

		//Texture data
		ps->Customizer.m_TextureCustomizer.UseDefaultTexture = data[id + "UseDefaultTexture"].get<bool>();
		ps->Customizer.m_TextureCustomizer.m_TexturePath = data[id + "TexturePath"].get<std::string>();
		if (!ps->Customizer.m_TextureCustomizer.UseDefaultTexture)
		{
			ps->Customizer.m_TextureCustomizer.ParticleTexture = Renderer::CreateTexture(Image::LoadFromFile(AssetManager::s_EnvironmentDirectory.u8string() + "\\" + ps->Customizer.m_TextureCustomizer.m_TexturePath.u8string()));
		}

		//Force data
		size_t forceCount = data[id + "Force Count"].get<size_t>();

		//delete the default force
		ps->Customizer.m_ForceCustomizer.m_Forces.erase("Gravity");

		for (size_t i = 0; i < forceCount; i++)
		{
			//make a new force with the data from the json file
			Force force;
			ps->Customizer.m_ForceCustomizer.m_Forces[data[id + "Force" + std::to_string(i).c_str() + "Key"].get<std::string>()] = force;
			Force& currentForce = ps->Customizer.m_ForceCustomizer.m_Forces[data[id + "Force" + std::to_string(i).c_str() + "Key"].get<std::string>()];

			currentForce.Enabled = data[id + "Force" + std::to_string(i).c_str() + "Enabled"].get<bool>();
			currentForce.Type = Force::StringToForceType(data[id + "Force" + std::to_string(i).c_str() + "Type"].get<std::string>());
			currentForce.DF_Value = JSON_ARRAY_TO_VEC2(data[id + "Force" + std::to_string(i).c_str() + "DF_Value"].get<std::vector<float>>());
			currentForce.RF_Target = JSON_ARRAY_TO_VEC2(data[id + "Force" + std::to_string(i).c_str() + "RF_Target"].get<std::vector<float>>());
			currentForce.RF_Strength = data[id + "Force" + std::to_string(i).c_str() + "RF_Strength"].get<float>();
		}
		
		//add particle system to environment
		pEnvironmentObject startingPSi((EnvironmentObjectInterface*)(ps.release()));
		env->Objects.push_back(std::move(startingPSi));
	}

	void RadialLightFromJson(Environment* env, json& data, std::string id)
	{
		//create radial light
		std::unique_ptr<RadialLight> light = std::make_unique<RadialLight>();

		//populate with data
		light->ID.FromString(data[id + "UUID"].get<std::string>());
		light->m_Name = data[id + "Name"].get <std::string>();
		light->ModelMatrix = JSON_ARRAY_TO_MAT4(data[id + "ModelMatrix"].get<std::vector<float>>());
		light->Color = JSON_ARRAY_TO_VEC4(data[id + "Color"].get<std::vector<float>>());
		light->Intensity = data[id + "Intensity"].get<float>();

		//add radial light to environment
		pEnvironmentObject startingPSi((EnvironmentObjectInterface*)(light.release()));
		env->Objects.push_back(std::move(startingPSi));
	}

	void SpotLightFromJson(Environment* env, json& data, std::string id)
	{
		//create radial light
		std::unique_ptr<SpotLight> light = std::make_unique<SpotLight>();

		//populate with data
		light->ID.FromString(data[id + "UUID"].get<std::string>());
		light->m_Name = data[id + "Name"].get <std::string>();
		light->ModelMatrix = JSON_ARRAY_TO_MAT4(data[id + "ModelMatrix"].get<std::vector<float>>());
		light->Color = JSON_ARRAY_TO_VEC4(data[id + "Color"].get<std::vector<float>>());
		light->OuterCutoff = data[id + "OuterCutoff"].get<float>();
		light->InnerCutoff = data[id + "InnerCutoff"].get<float>();
		light->Intensity = data[id + "Intensity"].get<float>();

		//add radial light to environment
		pEnvironmentObject obj((EnvironmentObjectInterface*)(light.release()));
		env->Objects.push_back(std::move(obj));
	}

	void SpriteFromJson(Environment* env, json& data, std::string id)
	{
		//create sprite
		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();

		//populate with data
		sprite->ID.FromString(data[id + "UUID"].get<std::string>());
		sprite->m_Name = data[id + "Name"].get<std::string>();
		sprite->ModelMatrix = JSON_ARRAY_TO_MAT4(data[id + "ModelMatrix"].get<std::vector<float>>());
		sprite->Tint = JSON_ARRAY_TO_VEC4(data[id + "Tint"].get<std::vector<float>>());
		sprite->Space = StrToObjSpace(data[id + "Space"].get<std::string>().c_str());
		sprite->m_TexturePath = data[id + "TexturePath"].get<std::string>();
		if(sprite->m_TexturePath != "")
			sprite->LoadTextureFromFile(AssetManager::s_EnvironmentDirectory.u8string() + "\\" + sprite->m_TexturePath.u8string());

		pEnvironmentObject obj((EnvironmentObjectInterface*)(sprite.release()));
		env->Objects.push_back(std::move(obj));
	}

	void LitSpriteFromJson(Environment* env, json& data, std::string id)
	{
		//create sprite
		std::unique_ptr<LitSprite> sprite = std::make_unique<LitSprite>();

		//populate with data
		sprite->ID.FromString(data[id + "UUID"].get<std::string>());
		sprite->m_Name = data[id + "Name"].get<std::string>();
		sprite->ModelMatrix = JSON_ARRAY_TO_MAT4(data[id + "ModelMatrix"].get<std::vector<float>>());
		sprite->m_Position = JSON_ARRAY_TO_VEC2(data[id + "Position"].get<std::vector<float>>());
		sprite->m_UniformBufferData.Tint = JSON_ARRAY_TO_VEC4(data[id + "Tint"].get<std::vector<float>>());
		sprite->m_UniformBufferData.BaseLight = data[id + "BaseLight"].get<float>();
		sprite->m_UniformBufferData.MaterialConstantCoefficient = data[id + "MaterialConstantCoefficient"].get<float>();
		sprite->m_UniformBufferData.MaterialLinearCoefficient = data[id + "MaterialLinearCoefficient"].get<float>();
		sprite->m_UniformBufferData.MaterialQuadraticCoefficient = data[id + "MaterialQuadraticCoefficient"].get<float>();

		pEnvironmentObject obj((EnvironmentObjectInterface*)(sprite.release()));
		env->Objects.push_back(std::move(obj));
	}

	void ModelFromJson(Environment* env, json& data, std::string id)
	{
		//create model
		std::unique_ptr<Model> model = std::make_unique<Model>();

		//populate with data
		model->ID.FromString(data[id + "UUID"].get<std::string>());
		model->m_Name = data[id + "Name"].get<std::string>();
		model->ModelMatrix = JSON_ARRAY_TO_MAT4(data[id + "ModelMatrix"].get<std::vector<float>>());
		model->CurrentModelPath = data[id + "ModelPath"].get<std::string>();
		model->FlipUVs = data[id + "FlipUVs"].get<bool>();
		if (model->CurrentModelPath != "")
			model->LoadModel(model->CurrentModelPath);

		pEnvironmentObject obj((EnvironmentObjectInterface*)(model.release()));
		env->Objects.push_back(std::move(obj));
	}

	void CameraFromJson(Environment* env, json& data, std::string id)
	{
		//get data
		auto projMode = StrToProjectionMode(data[id + "ProjMode"].get<std::string>());
		auto aspectRatio = JSON_ARRAY_TO_VEC2(data[id + "AspectRatio"].get<std::vector<float>>());
		auto modelMatrix = JSON_ARRAY_TO_MAT4(data[id + "ModelMatrix"].get<std::vector<float>>());
		auto name = data[id + "Name"].get<std::string>();

		//create camera
		std::unique_ptr<CameraObject> camera = std::make_unique<CameraObject>();

		camera->Init(name, modelMatrix, aspectRatio, projMode);
		camera->ID.FromString(data[id + "UUID"].get<std::string>());

		pEnvironmentObject obj((EnvironmentObjectInterface*)(camera.release()));
		env->Objects.push_back(std::move(obj));
	}

}

#undef JSON_ARRAY_TO_VEC4
#undef JSON_ARRAY_TO_VEC3
#undef JSON_ARRAY_TO_VEC2
#undef JSON_ARRAY_TO_MAT4

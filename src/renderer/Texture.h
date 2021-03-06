#pragma once

#include "Image.h"

#undef max

namespace Ainan {

	enum class TextureType
	{
		Unspecified,
		Texture2D,
		Cubemap
	};

	class Texture
	{
	public:
		uint32_t Identifier = std::numeric_limits<uint32_t>::max();

		bool IsValid()
		{
			return Identifier != std::numeric_limits<uint32_t>::max();
		}

		void UpdateData(std::shared_ptr<Image> image);
		void UpdateData(std::array<Image, 6> images); //used in cubemaps

		//used by ImGui
		uint64_t GetTextureID();
	};

	struct TextureDataView
	{
		uint64_t Identifier = std::numeric_limits<uint64_t>::max();
		uint64_t View = std::numeric_limits<uint64_t>::max(); //used in D3D
		uint64_t Sampler = std::numeric_limits<uint64_t>::max(); //used in D3D
		TextureType Type;
		glm::vec2 Size;
		TextureFormat Format;
		bool Deleted = false;
	};
}
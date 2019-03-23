#include <pch.h>
#include "Environment.h"

Environment::Environment()
{
	//setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(&Window::GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 400");
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

	timeStart = 0;
	timeEnd = 0;

	ParticleSystemObject startingPS;

	m_ParticleSystems.push_back(startingPS);

	GaussianBlur::Init();
}

Environment::~Environment()
{
	m_ParticleSystems.clear();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Environment::Update()
{
	timeEnd = clock();
	float deltaTime = (timeEnd - timeStart) / 1000.0f;
	timeStart = timeEnd;
	m_CurrentTimeBetweenFrameCapture -= deltaTime;

	//Update
	Window::Update();

	for(ParticleSystemObject& obj : m_ParticleSystems)
		obj.m_PS.Update(deltaTime);

	if (lastSize != Window::GetSize())
		m_FrameBuffer.SetSize(Window::GetSize());
	lastSize = Window::GetSize();
}

void Environment::Render()
{
	m_FrameBuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT);

	for (ParticleSystemObject& obj : m_ParticleSystems)
		obj.m_PS.Draw();

	m_FrameBuffer.Unbind();

	if(settings.GetBlurEnabled())
		GaussianBlur::Blur(m_FrameBuffer, settings.GetBlurScale(), settings.GetBlurStrength(), settings.GetBlurGaussianSigma());
	
	m_FrameBuffer.Bind();

	if (m_SaveNextFrameAsImage) 
	{
		Image image = Image::FromFrameBuffer(m_FrameBuffer);
		mkdir("output");
		image.SaveToFile("output/test", settings.GetImageFormat());
		m_SaveNextFrameAsImage = false;
	}
	m_FrameBuffer.RenderToScreen();
}

void Environment::RenderGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DisplayMainMenuBarGUI();

	if(m_ObjectInspectorWindowOpen)
		DisplayObjectInspecterGUI();

	if(m_GeneralSettingsWindowOpen)
		settings.DisplayGUI(m_GeneralSettingsWindowOpen);

	if(m_EnvironmentStatusWindowOpen)
		DisplayEnvironmentStatusGUI();

	for (ParticleSystemObject& obj : m_ParticleSystems)
		obj.DisplayGUI();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Environment::HandleInput()
{
	if (glfwGetMouseButton(&Window::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
	{
		for (ParticleSystemObject& obj : m_ParticleSystems)
			obj.m_PS.SpawnParticle(obj.m_PC.GetParticle());
	}

	if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
		for (ParticleSystemObject& obj : m_ParticleSystems)
			obj.m_PS.ClearParticles();

	if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_F11) == GLFW_PRESS && m_CurrentTimeBetweenFrameCapture < 0.0f) {
		m_SaveNextFrameAsImage = true;
		m_CurrentTimeBetweenFrameCapture = m_MinTimeBetweenFrameCapture;
	}
}

void Environment::DisplayObjectInspecterGUI()
{
	auto flags = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding;
	ImGui::Begin("Object Inspector", &m_ObjectInspectorWindowOpen, flags);

	for (int i = 0; i < m_ParticleSystems.size(); i++)
	{
		auto& particleSystem = m_ParticleSystems[i];

		ImGui::PushID(particleSystem.m_ID);
		ImGui::Text((particleSystem.m_Name.size() > 0) ? particleSystem.m_Name.c_str() : "No Name");

		ImGui::SameLine();
		if (ImGui::Button("Edit")) {
			particleSystem.m_EditorOpen = !particleSystem.m_EditorOpen;
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			m_ParticleSystems.erase(m_ParticleSystems.begin() + i);
		}

		ImGui::SameLine();
		if (ImGui::Button("Rename")) {
			particleSystem.m_RenameTextOpen = !particleSystem.m_RenameTextOpen;
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (particleSystem.m_RenameTextOpen) {
			auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
			if (ImGui::InputText("Name", &particleSystem.m_Name, flags)) {
				particleSystem.m_RenameTextOpen = !particleSystem.m_RenameTextOpen;
			}
		}

		ImGui::PopID();
	}

	if (ImGui::Button("Add Particle System"))
	{
		ParticleSystemObject pso;
		m_ParticleSystems.push_back(pso);
	}

	ImGui::End();
}

void Environment::DisplayEnvironmentStatusGUI()
{
	ImGui::Begin("Environment Status", &m_EnvironmentStatusWindowOpen);

	ImGui::Text("Particle Count :");
	ImGui::SameLine();

	unsigned int activeParticleCount = 0;
	for (ParticleSystemObject& pso : m_ParticleSystems)
		activeParticleCount += pso.m_PS.GetActiveParticleCount();

	ImGui::TextColored({ 0.0f,1.0f,0.0f,1.0f }, std::to_string(activeParticleCount).c_str());

	if (ImGui::TreeNode("Detailed Particle Distribution")) {

		for (auto& pso : m_ParticleSystems) 
		{
			if (ImGui::TreeNode(pso.m_Name.c_str())) {

				ImGui::Text("Particle Count :");

				ImGui::SameLine();
				ImGui::TextColored({ 0.0f,1.0f,0.0f,1.0f }, std::to_string(pso.m_PS.GetActiveParticleCount()).c_str());
				ImGui::TreePop();
			}

			ImGui::Spacing();
		}

		ImGui::TreePop();
	}


	ImGui::End();
}

void Environment::DisplayMainMenuBarGUI()
{
	int MenuBarHeight = 0;
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("Edit")) {

			if (ImGui::MenuItem("Clear Particle Systems")) {
				m_ParticleSystems.clear();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {

			if (ImGui::MenuItem("Object Inspector"))
				m_ObjectInspectorWindowOpen = !m_ObjectInspectorWindowOpen;

			if (ImGui::MenuItem("General Settings"))
				m_GeneralSettingsWindowOpen = !m_GeneralSettingsWindowOpen;

			if (ImGui::MenuItem("Environment Status"))
				m_EnvironmentStatusWindowOpen = !m_EnvironmentStatusWindowOpen;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Editor Style")) {

			if (ImGui::MenuItem("Dark"))
				ImGui::StyleColorsDark();

			if (ImGui::MenuItem("Light"))
				ImGui::StyleColorsLight();

			if (ImGui::MenuItem("Classic"))
				ImGui::StyleColorsClassic();

			ImGui::EndMenu();
		}

		MenuBarHeight = ImGui::GetWindowSize().y;

		ImGui::EndMainMenuBar();
	}

	//TODO change this to a seperate function
	ImGuiViewport viewport;
	viewport.Size = ImVec2(Window::GetSize().x, Window::GetSize().y);
	viewport.Pos = ImVec2(0, MenuBarHeight);
	ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruDockspace, 0);
}

//TODO change this
static int nameIndextemp = 0;
ParticleSystemObject::ParticleSystemObject() :
	m_EditorOpen(false),
	m_RenameTextOpen(false)
{
	m_Name = "Particle System (" + std::to_string(nameIndextemp) + ")";
	m_ID = nameIndextemp;
	nameIndextemp++;
}

void ParticleSystemObject::DisplayGUI()
{
	if(m_EditorOpen)
		m_PC.DisplayGUI(m_Name, m_EditorOpen);
}
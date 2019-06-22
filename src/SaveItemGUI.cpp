#include <pch.h>

#include "SaveItemGUI.h"

namespace ALZ {

	namespace fs = std::filesystem;

	SaveItemGUI::SaveItemGUI(const std::string& startingFolder, const std::string& windowName) :
		m_CurrentFolder(startingFolder),
		m_WindowName(windowName),
		m_InputFolder(startingFolder)
	{}

	void SaveItemGUI::OpenWindow()
	{
		m_WindowOpen = true;
		m_LastWindowState = true;
	}

	void SaveItemGUI::CloseWindow()
	{
		m_WindowOpen = false;
	}

	void SaveItemGUI::DisplayGUI(const std::function<void(const std::string&)>& func)
	{
		if (!m_WindowOpen)
			return;

		ImGui::Begin(m_WindowName.c_str(), &m_WindowOpen);

		ImGui::Text("Current Directory :");
		ImGui::SameLine();
		auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("##FolderPath", &m_InputFolder, flags)) {
			if (fs::exists(m_InputFolder))
				m_CurrentFolder = m_InputFolder;
		}

		ImGui::PushItemWidth(-1);
		if (ImGui::ListBoxHeader("##empty", 0, (int)std::distance(fs::directory_iterator(m_CurrentFolder), fs::directory_iterator{}) + 5)) {

			//check if we can go back
			if (std::count(m_CurrentFolder.begin(), m_CurrentFolder.end(), '\\') > 0) {
				//back button
				if (ImGui::Button("..")) {
					auto lastBackslashLoc = m_CurrentFolder.find_last_of('\\');
					if (std::count(m_CurrentFolder.begin(), m_CurrentFolder.end(), '\\') == 1)
						m_CurrentFolder.erase(lastBackslashLoc + 1, m_CurrentFolder.size() - lastBackslashLoc + 1);
					else
						m_CurrentFolder.erase(lastBackslashLoc, m_CurrentFolder.size() - lastBackslashLoc);
					m_InputFolder = m_CurrentFolder;
				}
			}
			for (const auto& entry : fs::directory_iterator(m_CurrentFolder)) {
				if (entry.status().type() == fs::file_type::directory) {
					if (ImGui::Button(entry.path().filename().u8string().c_str())) {
						m_CurrentFolder += "\\" + entry.path().filename().u8string();
						m_InputFolder = m_CurrentFolder;
					}
				}
			}

			ImGui::ListBoxFooter();
		}


		ImGui::PushItemWidth(500);

		if (ImGui::InputText("##FileName", &m_FileName, flags)) {
			if (m_FileName.size() > 0)
				m_FileNameChosen = true;
			else
				m_FileNameChosen = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Save"))
		{
			if (func != nullptr && m_FileNameChosen) {
				func(GetSelectedSavePath());
				m_WindowOpen = false;
			}
		}

		if (m_FileName.find(".") != std::string::npos)
		{
			m_FileNameChosen = false;
			ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Do not include file format!");
		}

		ImGui::End();

		//Handle OnCloseWindow if the new window state is false and the last one is true
		if (m_LastWindowState == true && m_WindowOpen == false) {
			if (OnCloseWindow)
				OnCloseWindow();
			m_LastWindowState = false;
		}
	}

	std::string SaveItemGUI::GetSelectedSavePath()
	{
		return m_CurrentFolder + '\\' + m_FileName;
	}
}
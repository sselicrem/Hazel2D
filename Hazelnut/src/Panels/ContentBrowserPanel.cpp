#include "hzpch.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>

namespace Hazel {

	const std::filesystem::path s_AssetPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectiry(s_AssetPath)
	{
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectiry != s_AssetPath)
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectiry = m_CurrentDirectiry.parent_path();
			}
		}

		for (auto& directory: std::filesystem::directory_iterator(m_CurrentDirectiry))
		{
			const auto& path = directory.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			std::string relativePathString = relativePath.string();
			if (directory.is_directory())
			{
				if (ImGui::Button(relativePathString.c_str()))
				{
					m_CurrentDirectiry /= path.filename();
				}

			}
			else
			{
				if (ImGui::Button(relativePathString.c_str()))
				{
				}
			}
		}

		ImGui::End();
	}
}
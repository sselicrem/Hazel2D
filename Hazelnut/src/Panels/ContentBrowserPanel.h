#pragma once

#include <filesystem>
#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void OnImGuiRender();
	private:
		std::filesystem::path m_CurrentDirectiry;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}
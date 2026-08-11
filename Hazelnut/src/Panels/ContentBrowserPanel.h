#pragma once

#include <filesystem>

namespace Hazel {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void OnImGuiRender();
	private:
		std::filesystem::path m_CurrentDirectiry;
	};
}
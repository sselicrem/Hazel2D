#pragma once

#include <Hazel.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Hazel/Renderer/EditorCamera.h"

namespace Hazel
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& event);
		void NewScene();
		void OpenScene();
		void Save();
		void SaveAs();
	private:
		OrthographicCameraController m_CameraController;

		//Temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;
		Ref<Texture2D> m_CheckerboardTexture;
		Ref<Framebuffer> m_Framebuffer;
		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;
		Entity m_HoveredEntity;

		EditorCamera m_EditorCamera;

		bool m_PrimaryCamera = true;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		int m_GizmoType = -1;

		std::string openedFile;
		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}
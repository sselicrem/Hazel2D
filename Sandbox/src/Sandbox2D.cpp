#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.f / 720.f)
{
}

void Sandbox2D::OnAttach()
{
	m_CheckerBoardTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();
	//Timer timer("Sandbox2D::OnUpdate", [&](auto profileResult) {m_ProfileResults.push_back(profileResult); });

	// Update
	{
		HZ_PROFILE_SCOPE("Camera Controller");
		m_CameraController.OnUpdate(ts);
	}
	// Render
	{
		HZ_PROFILE_SCOPE("Renderer Prep");	
		Hazel::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Renderer Draw");
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Hazel::Renderer2D::DrawQuad({ -1.f,0.f }, { 0.5f, 0.5f }, { 0.8f, 0.8f, 0.3f, 1.f });
		Hazel::Renderer2D::DrawQuad({ 1.f,0.5f }, { 1.25f, 0.5f }, { 0.8f, 0.2f, 0.3f, 1.f });

		Hazel::Renderer2D::DrawQuad({ 0.f, 0.f, -0.1f }, { 10.f, 10.0f }, m_CheckerBoardTexture);

		Hazel::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	m_CameraController.OnEvent(event);
}

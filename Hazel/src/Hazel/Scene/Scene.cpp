#include "hzpch.h"
#include "Scene.h"

#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"

#include "Hazel/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

namespace Hazel {

	static void DoMath(const glm::mat4& transform)
	{

	}

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
	}

	void Scene::OnUpdate(Timestep ts)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto& entity : group)
		{
			auto& [tranform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(tranform, sprite.Color);
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name);

		return entity;
	}
}
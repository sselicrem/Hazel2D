#include "hzpch.h"
#include "SceneSerializer.h"

#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"

#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Core/UUID.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Hazel::UUID>
	{
		static Node encode(const Hazel::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Hazel::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

}

namespace Hazel {

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.SetValue(data);                  \
		break;                                         \
	}

	static std::string RigidBody2d_BodyType_ToString(Rigidbody2DComponent::BodyType type) {
		switch (type)
		{
		case Hazel::Rigidbody2DComponent::BodyType::Static: return "Static";
		case Hazel::Rigidbody2DComponent::BodyType::Dynamic: return "Dynamic";
		case Hazel::Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
		}
		HZ_ASSERT(false, "Unknown Body Type!");
		return std::string();
	}

	static Rigidbody2DComponent::BodyType RigidBody2d_BodyType_FromString(const std::string& type) {
		if (type == "Static") return Rigidbody2DComponent::BodyType::Static;
		if (type == "Dynamic") return Rigidbody2DComponent::BodyType::Dynamic;
		if (type == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

		HZ_ASSERT(false, "Unknown Body Type!");
		return Rigidbody2DComponent::BodyType::Static;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		HZ_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Serializable Entity must have an IDComponent");

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera

			out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.GetProjectionType());
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out<<YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

			// Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					out << YAML::BeginMap; // ScriptField
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

					out << YAML::Key << "Data" << YAML::Value;
					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float, float);
						WRITE_SCRIPT_FIELD(Double, double);
						WRITE_SCRIPT_FIELD(Bool, bool);
						WRITE_SCRIPT_FIELD(Char, char);
						WRITE_SCRIPT_FIELD(Byte, int8_t);
						WRITE_SCRIPT_FIELD(Short, int16_t);
						WRITE_SCRIPT_FIELD(Int, int32_t);
						WRITE_SCRIPT_FIELD(Long, int64_t);
						WRITE_SCRIPT_FIELD(UByte, uint8_t);
						WRITE_SCRIPT_FIELD(UShort, uint16_t);
						WRITE_SCRIPT_FIELD(UInt, uint32_t);
						WRITE_SCRIPT_FIELD(ULong, uint64_t);
						WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
						WRITE_SCRIPT_FIELD(Entity, UUID);
					}
					out << YAML::EndMap; // ScriptFields
				}
				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // ScriptComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			if (spriteRendererComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << spriteRendererComponent.Texture->GetPath();

			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;
			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2d_BodyType_ToString(rb2d.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& boxComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << boxComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << boxComponent.Size;

			out << YAML::Key << "Density" << YAML::Value << boxComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << boxComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << boxComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << boxComponent.RestitutionThreshold;


			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			auto& boxComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << boxComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << boxComponent.Radius;

			out << YAML::Key << "Density" << YAML::Value << boxComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << boxComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << boxComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << boxComponent.RestitutionThreshold;


			out << YAML::EndMap; // CircleCollider2DComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.view<entt::entity>().each([&](auto entityID)
			{
				Entity entity{ entityID, m_Scene.get() };
				if (!entity)
					return;

				SerializeEntity(out, entity);
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			HZ_CORE_ERROR("Failed to load .hazel file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		HZ_CORE_TRACE("Deserializing Scene '{0}'", sceneName);

		if (auto entities = data["Entities"]; entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;

				if (auto tagComponent = entity["TagComponent"]; tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				HZ_CORE_TRACE("Deserialized entity with ID '{0}' and name '{1}'", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				if (auto transformComponent = entity["TransformComponent"]; transformComponent)
				{
					auto& component = deserializedEntity.GetComponent<TransformComponent>();
					component.Translation = transformComponent["Translation"].as<glm::vec3>();
					component.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					component.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				if (auto spriteComponent = entity["SpriteRendererComponent"]; spriteComponent)
				{
					auto& component = deserializedEntity.AddComponent<SpriteRendererComponent>();
					component.Color = spriteComponent["Color"].as<glm::vec4>();
					if (spriteComponent["TexturePath"])
						component.Texture = Texture2D::Create(spriteComponent["TexturePath"].as<std::string>());

					if (spriteComponent["TilingFactor"])
						component.TilingFactor = spriteComponent["TilingFactor"].as<float>();
				}

				if (auto circleRendererComponent = entity["CircleRendererComponent"]; circleRendererComponent)
				{
					auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
					crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
					crc.Thickness = circleRendererComponent["Thickness"].as<float>();
					crc.Fade = circleRendererComponent["Fade"].as<float>();
				}

				if (auto cameraComponent = entity["CameraComponent"]; cameraComponent)
				{
					auto& component = deserializedEntity.AddComponent<CameraComponent>();

					auto& cameraProps = cameraComponent["Camera"];

					component.Camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(cameraProps["ProjectionType"].as<int>()));
					component.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					component.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					component.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					component.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					component.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					component.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					component.Primary = cameraComponent["Primary"].as<bool>();
					component.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				if (auto scriptComponent = entity["ScriptComponent"]; scriptComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					sc.ClassName = scriptComponent["ClassName"].as<std::string>();

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						HZ_CORE_ASSERT(entityClass);
						const auto& fields = entityClass->GetFields();
						auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

						for (auto scriptField : scriptFields)
						{
							std::string name = scriptField["Name"].as<std::string>();
							std::string typeString = scriptField["Type"].as<std::string>();
							ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

							ScriptFieldInstance& fieldInstance = entityFields[name];

							// TODO(Yan): turn this assert into Hazelnut log warning
							HZ_CORE_ASSERT(fields.find(name) != fields.end());

							if (fields.find(name) == fields.end())
								continue;

							fieldInstance.Field = fields.at(name);

							switch (type)
							{
								READ_SCRIPT_FIELD(Float, float);
								READ_SCRIPT_FIELD(Double, double);
								READ_SCRIPT_FIELD(Bool, bool);
								READ_SCRIPT_FIELD(Char, char);
								READ_SCRIPT_FIELD(Byte, int8_t);
								READ_SCRIPT_FIELD(Short, int16_t);
								READ_SCRIPT_FIELD(Int, int32_t);
								READ_SCRIPT_FIELD(Long, int64_t);
								READ_SCRIPT_FIELD(UByte, uint8_t);
								READ_SCRIPT_FIELD(UShort, uint16_t);
								READ_SCRIPT_FIELD(UInt, uint32_t);
								READ_SCRIPT_FIELD(ULong, uint64_t);
								READ_SCRIPT_FIELD(Vector2, glm::vec2);
								READ_SCRIPT_FIELD(Vector3, glm::vec3);
								READ_SCRIPT_FIELD(Vector4, glm::vec4);
								READ_SCRIPT_FIELD(Entity, UUID);
							}
						}
					}
				}

				if (auto rb2dComponent = entity["Rigidbody2DComponent"]; rb2dComponent)
				{
					auto& component = deserializedEntity.AddComponent<Rigidbody2DComponent>();

					component.Type = RigidBody2d_BodyType_FromString(rb2dComponent["BodyType"].as<std::string>());
					component.FixedRotation = rb2dComponent["FixedRotation"].as<bool>();
				}

				if (auto boxColliderComponent = entity["BoxCollider2DComponent"]; boxColliderComponent)
				{
					auto& component = deserializedEntity.AddComponent<BoxCollider2DComponent>();

					component.Offset = boxColliderComponent["Offset"].as<glm::vec2>();
					component.Size = boxColliderComponent["Size"].as<glm::vec2>();

					component.Density = boxColliderComponent["Density"].as<float>();
					component.Friction = boxColliderComponent["Friction"].as<float>();
					component.Restitution = boxColliderComponent["Restitution"].as<float>();
					component.RestitutionThreshold = boxColliderComponent["RestitutionThreshold"].as<float>();
				}

				if (auto circleColliderComponent = entity["CircleCollider2DComponent"]; circleColliderComponent)
				{
					auto& component = deserializedEntity.AddComponent<CircleCollider2DComponent>();

					component.Offset = circleColliderComponent["Offset"].as<glm::vec2>();
					component.Radius = circleColliderComponent["Radius"].as<float>();

					component.Density = circleColliderComponent["Density"].as<float>();
					component.Friction = circleColliderComponent["Friction"].as<float>();
					component.Restitution = circleColliderComponent["Restitution"].as<float>();
					component.RestitutionThreshold = circleColliderComponent["RestitutionThreshold"].as<float>();
				}
			}
		}
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		HZ_CORE_ASSERT(false, "Runtime deserialization is not supported!");	
		return false;
	}

}
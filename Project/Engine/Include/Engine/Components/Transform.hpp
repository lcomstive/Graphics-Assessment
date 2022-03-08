#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/Components/Component.hpp>

namespace Engine::Graphics { class Shader; }

namespace Engine::Components
{
	struct Transform : public Component
	{
		glm::vec3 Scale		= { 1, 1, 1 };
		glm::vec3 Position	= { 0, 0, 0 };
		glm::vec3 Rotation	= { 0, 0, 0 };

		ENGINE_API void AddChild(Transform* child);
		ENGINE_API void SetParent(Transform* parent);
		ENGINE_API void RemoveChild(Transform* child);
		ENGINE_API void RemoveChild(unsigned int index);

		ENGINE_API glm::vec3 GetGlobalScale();
		ENGINE_API glm::vec3 GetGlobalPosition();
		ENGINE_API glm::vec3 GetGlobalRotation();
		ENGINE_API glm::mat4 GetGlobalRotationMatrix();

		ENGINE_API void ClearChildren();
		ENGINE_API Transform* GetParent();
		ENGINE_API std::vector<Transform*> GetChildren();

		ENGINE_API void FillShader(Engine::Graphics::Shader* shader);

	private:
		Transform* m_Parent = nullptr;
		std::vector<Transform*> m_Children;
	};
}
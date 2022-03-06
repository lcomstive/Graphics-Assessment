#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <Engine/Components/Component.hpp>

namespace Engine::Graphics { class Shader; }

namespace Engine::Components
{
	struct Transform : public Component
	{
		glm::vec3 Scale		= { 1, 1, 1 };
		glm::vec3 Position	= { 0, 0, 0 };
		glm::vec3 Rotation	= { 0, 0, 0 };

		void AddChild(Transform* child);
		void SetParent(Transform* parent);
		void RemoveChild(Transform* child);
		void RemoveChild(unsigned int index);

		glm::vec3 GetGlobalScale();
		glm::vec3 GetGlobalPosition();
		glm::vec3 GetGlobalRotation();
		glm::mat4 GetGlobalRotationMatrix();

		void ClearChildren();
		Transform* GetParent();
		std::vector<Transform*> GetChildren();

		void FillShader(Engine::Graphics::Shader* shader);

	private:
		Transform* m_Parent = nullptr;
		std::vector<Transform*> m_Children;
	};
}
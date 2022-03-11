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

		ENGINE_API glm::vec3 Up();
		ENGINE_API glm::vec3 Right();
		ENGINE_API glm::vec3 Forward();

		ENGINE_API void ClearChildren();
		ENGINE_API Transform* GetParent();
		ENGINE_API std::vector<Transform*> GetChildren();

		ENGINE_API void FillShader(Engine::Graphics::Shader* shader);

		/// <summary>
		/// Recalculates global & direction values when required
		/// </summary>
		/// <param name="deltaTime">Unused</param>
		ENGINE_API virtual void Update(float deltaTime) override;

	protected:
		ENGINE_API virtual void Added() override;

	private:
		Transform* m_Parent = nullptr;
		std::vector<Transform*> m_Children;

		bool m_Dirty;
		glm::mat4 m_ModelMatrix;
		glm::vec3 m_LastPos, m_LastRot, m_LastScale;

		glm::vec3 m_Forward, m_Right, m_Up;
		glm::vec3 m_GlobalScale, m_GlobalRotation, m_GlobalPosition;
	};
}
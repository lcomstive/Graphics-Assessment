#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Components/Transform.hpp>

using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

const vec3 WorldUp = { 0, 1, 0 };

void Transform::Added() { m_Dirty = true; }

vec3 Transform::GetGlobalScale()	{ return m_GlobalScale; }
vec3 Transform::GetGlobalPosition() { return m_GlobalPosition; }
vec3 Transform::GetGlobalRotation() { return m_GlobalRotation; }

mat4 Transform::GetGlobalRotationMatrix()
{
	vec3 rotation = GetGlobalRotation();
	return eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
}

vec3 Transform::Up() { return m_Up; }
vec3 Transform::Right() { return m_Right; }
vec3 Transform::Forward() { return m_Forward; }

void Transform::AddChild(Transform* child)
{
	m_Children.emplace_back(child);
	child->m_Parent = this;
}

void Transform::RemoveChild(Transform* child)
{
	const auto& it = find(m_Children.begin(), m_Children.end(), child);
	if (it != m_Children.end())
	{
		m_Children.erase(it);
		child->m_Parent = nullptr;
	}
}

void Transform::RemoveChild(unsigned int index)
{
	if (index >= (unsigned int)m_Children.size())
		return;
	m_Children[index]->m_Parent = nullptr;
	m_Children.erase(m_Children.begin() + index);
}

void Transform::ClearChildren()
{
	for (Transform* child : m_Children)
		child->m_Parent = nullptr;
	m_Children.clear();
}

void Transform::SetParent(Transform* parent)
{
	if (this == parent)
		return;

	if (m_Parent)
	{
		// Remove from previous parent
		for (uint32_t i = 0; i < (uint32_t)m_Parent->m_Children.size(); i++)
		{
			if (m_Parent->m_Children[i] != this)
				continue;
			m_Parent->m_Children.erase(m_Parent->m_Children.begin() + i);
			break;
		}
	}

	m_Parent = parent;
	if (m_Parent)
	{
		m_Dirty = true; // Recalculate global values
		m_Parent->m_Children.emplace_back(this);
	}
}

Transform* Transform::GetParent() { return m_Parent; }
std::vector<Transform*> Transform::GetChildren() { return m_Children; }

void Transform::FillShader(Shader* shader) { shader->Set("modelMatrix", m_ModelMatrix); }

void Transform::Update(float deltaTime)
{
#pragma region Dirty Check ;)
	m_Dirty = m_Dirty ||
				m_LastPos != Position ||
				m_LastRot != Rotation ||
				m_LastScale != Scale;

	if (!m_Dirty)
		return;
	m_Dirty = false;

	m_LastScale = Scale;
	m_LastPos = Position;
	m_LastRot = Rotation;
#pragma endregion

	// Calculate globals
	m_GlobalScale = Scale;
	m_GlobalPosition = Position;
	m_GlobalRotation = Rotation;
	
	if (m_Parent)
	{
		m_GlobalScale *= m_Parent->GetGlobalScale();
		m_GlobalPosition += m_Parent->GetGlobalPosition();
		m_GlobalRotation += m_Parent->GetGlobalRotation();
	}

	// Calculate directions
	m_Forward = normalize(vec3(
		cos(Rotation.y) * cos(Rotation.x),
		sin(Rotation.x),
		sin(Rotation.y) * cos(Rotation.x)
	));
	m_Right = normalize(cross(m_Forward, WorldUp));
	m_Up = normalize(cross(m_Right, m_Forward));

	// Generate model matrix
	mat4 translationMatrix = translate(mat4(1.0f), m_GlobalPosition);
	mat4 scaleMatrix = scale(mat4(1.0f), m_GlobalScale);

	m_ModelMatrix = translationMatrix * GetGlobalRotationMatrix() * scaleMatrix;
}
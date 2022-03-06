#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Components/Transform.hpp>

using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

vec3 Transform::GetGlobalScale()
{
	vec3 scale = Scale;
	if (m_Parent)
		scale *= m_Parent->GetGlobalScale();
	return scale;
}

vec3 Transform::GetGlobalPosition()
{
	vec3 pos = Position;
	if (m_Parent)
		pos += m_Parent->GetGlobalPosition();
	return pos;
}

vec3 Transform::GetGlobalRotation()
{
	vec3 rot = Rotation;
	if (m_Parent)
		rot += m_Parent->GetGlobalRotation();
	return rot;
}

mat4 Transform::GetGlobalRotationMatrix()
{
	vec3 rotation = GetGlobalRotation();
	return eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
}

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
			m_Parent->m_Children.erase(m_Parent->m_Children.begin() + i);
	}

	m_Parent = parent;
	if(m_Parent)
		m_Parent->m_Children.emplace_back(this);
}

Transform* Transform::GetParent() { return m_Parent; }
std::vector<Transform*> Transform::GetChildren() { return m_Children; }

void Transform::FillShader(Shader* shader)
{
	// Generate model matrix
	mat4 translationMatrix = translate(mat4(1.0f), GetGlobalPosition());
	mat4 scaleMatrix = scale(mat4(1.0f), GetGlobalScale());
	mat4 rotationMatrix = toMat4(quat(GetGlobalRotation())); // Convert quaternion to matrix

	mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	shader->Set("modelMatrix", modelMatrix);
}
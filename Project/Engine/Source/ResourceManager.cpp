#include <Engine/ResourceManager.hpp>

using namespace std;
using namespace Engine;

ResourceManager* ResourceManager::s_Instance = nullptr;

ResourceManager::ResourceManager()
{
	if (s_Instance == nullptr)
		s_Instance = this;
}

ResourceManager::~ResourceManager()
{
	if (s_Instance == this)
		s_Instance = nullptr;
}

ResourceID ResourceManager::AllocateID()
{
	unsigned int ID = 0;
	while (m_Instances.find(++ID) != m_Instances.end());
	return ID;
}

void ResourceManager::_Unload(ResourceID id)
{
	auto& it = m_Instances.find(id);
	if (it == m_Instances.end())
		return;

	delete it->second.Data;
	m_Instances.erase(it);
}

void ResourceManager::_Unload(string& name)
{
	auto& it = m_NamedInstances.find(name);
	if (it == m_NamedInstances.end())
		return;

	delete it->second.Data;
	m_NamedInstances.erase(it);
}

void ResourceManager::_UnloadAll()
{
	for (auto& pair : m_Instances)
		delete pair.second.Data;
	m_Instances.clear();
}

bool ResourceManager::_IsValid(ResourceID& id) { return m_Instances.find(id) != m_Instances.end(); }
bool ResourceManager::_IsValid(string& name) { return m_NamedInstances.find(name) != m_NamedInstances.end(); }

std::type_index ResourceManager::_GetType(ResourceID& id)
{
	auto& it = m_Instances.find(id);
	return it == m_Instances.end() ? typeid(bool) : it->second.Type;
}

std::type_index ResourceManager::_GetType(string& name)
{
	auto& it = m_NamedInstances.find(name);
	return it == m_NamedInstances.end() ? typeid(bool) : it->second.Type;
}
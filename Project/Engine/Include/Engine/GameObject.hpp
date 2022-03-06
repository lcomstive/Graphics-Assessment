#pragma once
#include <string>
#include <typeindex>
#include <unordered_map>
#include <Engine/Log.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Components/Transform.hpp>

namespace Engine
{
	class Scene;
	class GameObject
	{
		std::string m_Name;
		Scene* m_Scene = nullptr;
		Engine::Components::Transform* m_Transform;
		std::unordered_map<std::type_index, Components::Component*> m_Components;

	public:
		GameObject(std::string name = "GameObject");
		GameObject(Scene* scene, std::string name = "GameObject");
		GameObject(GameObject* parent, std::string name = "GameObject");
		GameObject(Engine::Components::Transform* parent, std::string name = "GameObject");
		~GameObject();

		void Draw();
		void DrawGizmos();
		void Update(float deltaTime);

		Scene* GetScene();
		std::string GetName();
		void SetName(std::string name);
		Components::Transform* GetTransform();

		/// <summary>
		/// Attaches a component onto a GameObject, or returns existing one if found
		/// </summary>
		template<typename T>
		T* AddComponent()
		{
			Log::Assert(std::is_base_of<Components::Component, T>(), "Added components need to derive from Engine::Components::Component");

			std::type_index type = typeid(T);
			Engine::Components::Component* instance = nullptr;

			// Check for existing component attached
			if (instance = GetComponent<T>(true)) // Also checks inherited components
				return (T*)instance;

			// Create & attach new component
			instance = new T();
			instance->m_GameObject = this;
			instance->Added();
			m_Components.emplace(type, instance);
			return (T*)instance;
		}

		template<typename T>
		void RemoveComponent()
		{
			Log::Assert(std::is_base_of<Components::Component, T>(), "Removed components need to derive from Engine::Components::Component");
			std::type_index type = typeid(T);

			// Check for existing component attached
			const auto& it = m_Components.find(type);
			if (it == m_Components.end())
				return;

			it->second->Removed();
			delete it->second;
			m_Components.erase(it);
		}

		/// <summary>
		/// Gets a component from a GameObject
		/// </summary>
		/// <typeparam name="T">Component type, must derive from Engine::Components::Component</typeparam>
		/// <returns>Component if found, otherwise nullptr</returns>
		template<typename T>
		T* GetComponent(bool checkInheritedClasses = false)
		{
			Log::Assert(std::is_base_of<Components::Component, T>(), "Tried getting component that did not derive from Engine::Components::Component");

			std::type_index type = typeid(T);

			// Check for existing component attached
			const auto& it = m_Components.find(type);
			if (it != m_Components.end())
				return (T*)it->second;
			else if (!checkInheritedClasses)
				return nullptr;

			for (auto& pair : m_Components)
			{
				T* instance = dynamic_cast<T*>(pair.second);
				if (instance)
					return instance;
			}
			return nullptr;
		}
		
		/// <summary>
		/// Gets all matching components from a GameObject
		/// </summary>
		/// <typeparam name="T">Component type, must derive from Engine::Components::Component</typeparam>
		template<typename T>
		std::vector<T*> GetComponents(bool checkInheritedClasses = false)
		{
			Log::Assert(std::is_base_of<Components::Component, T>(), "Tried getting component that did not derive from Engine::Components::Component");

			std::vector<T*> components = {};

			for (auto& pair : m_Components)
			{
				T* instance = dynamic_cast<T*>(pair.second);
				if (instance)
					components.emplace_back(instance);
			}

			return components;
		}
		
		/// <summary>
		/// Gets first valid component found from a GameObject or it's children
		/// </summary>
		/// <typeparam name="T">Component type, must derive from Engine::Components::Component</typeparam>
		/// <returns>Component if found, otherwise nullptr</returns>
		template<typename T>
		T* GetComponentInChildren(bool checkInheritedClasses = false)
		{
			Log::Assert(std::is_base_of<Components::Component, T>(), "Tried getting component that did not derive from Engine::Components::Component");

			T* instance = GetComponent<T>(checkInheritedClasses);

			std::vector<Components::Transform*> children = m_Transform->GetChildren();
			for (Components::Transform* child : children)
			{
				instance = child->GetGameObject()->GetComponentInChildren<T>(checkInheritedClasses);
				if (instance)
					return instance;
			}
			return nullptr;
		}
		
		/// <summary>
		/// Gets all valid components found from a GameObject or it's children
		/// </summary>
		/// <typeparam name="T">Component type, must derive from Engine::Components::Component</typeparam>
		template<typename T>
		std::vector<T*> GetComponentsInChildren(bool checkInheritedClasses = false)
		{
			std::vector<T*> components = {};
			GetComponentsInChildren(components, checkInheritedClasses);
			return components;
		}

		template<typename T>
		void GetComponentsInChildren(std::vector<T*>& output, bool checkInheritedClasses = false)
		{
			Log::Assert(std::is_base_of<Components::Component, T>(), "Tried getting component that did not derive from Engine::Components::Component");

			T* instance = GetComponent<T>(checkInheritedClasses);
			if (instance)
				output.emplace_back(instance);

			for (Components::Transform* child : GetTransform()->GetChildren())
				child->GetGameObject()->GetComponentsInChildren<T>(output, checkInheritedClasses);
		}

		template<typename T>
		bool HasComponent(bool checkInheritedClasses = false)
		{
			if (!std::is_base_of<Components::Component, T>())
			{
				Log::Warning("Tried checking component that did not derive from Engine::Components::Component");
				return false;
			}

			const auto& it = m_Components.find(typeid(T));
			if (it != m_Components.end())
				return true;
			else if (!checkInheritedClasses)
				return false;

			// Check inherited classes
			for (auto& pair : m_Components)
			{
				if (dynamic_cast<T*>(pair.second))
					return true;
			}
			return false;
		}
	};
}
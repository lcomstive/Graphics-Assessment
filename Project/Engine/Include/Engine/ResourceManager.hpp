#pragma once
#include <typeindex>
#include <Engine/Api.hpp>
#include <Engine/Log.hpp>
#include <Engine/Types.hpp>
#include <Engine/ResourceID.hpp>

namespace Engine
{
	class Application; // Forward declaration

	class ENGINE_API ResourceManager
	{
		static ResourceManager* s_Instance;

		friend class Engine::Application;

		struct ResourceInstance
		{
			void* Data;
			std::type_index Type;
		};

		EngineUnorderedMap<ResourceID, ResourceInstance> m_Instances;
		EngineUnorderedMap<EngineString, ResourceID> m_NamedInstances; // Name -> ResourceID

		ResourceManager();
		~ResourceManager();

		ResourceID AllocateID();

		template<typename T, class... Args>
		ResourceID _Load(Args... constructorArgs)
		{
			ResourceID id = AllocateID();
			m_Instances.emplace(id,
				ResourceInstance
				{
					nullptr,
					typeid(T)
				});
			m_Instances.at(id).Data = new T(constructorArgs...);
			return id;
		}
		
		template<typename T, class... Args>
		ResourceID _LoadNamed(std::string& name, Args... constructorArgs)
		{
			auto& it = m_NamedInstances.find(name);
			if (it != m_NamedInstances.end())
				return it->second;

			ResourceID id = _Load<T, Args...>(constructorArgs...);
			m_NamedInstances.emplace(make_pair(name, id));
			return id;
		}

		template<typename T>
		T* _Get(ResourceID& id)
		{
			auto& it = m_Instances.find(id);
			if (it == m_Instances.end())
				return nullptr;
#ifndef NDEBUG
			Log::Assert(it->second.Type == typeid(T),
				"Tried getting type by resource ID but types did not match - '" +
					string(it->second.Type.name()) + "' != " + string(typeid(T).name()) + "'");
#endif
			return (T*)it->second.Data;
		}
		
		template<typename T>
		T* _Get(std::string& name)
		{
			auto& it = m_NamedInstances.find(name);
			if (it == m_NamedInstances.end())
				return nullptr;
			return _Get<T>(it->second);
		}

		void _Unload(ResourceID id);
		void _Unload(std::string& name);

		void _UnloadAll();
		bool _IsValid(ResourceID& id);
		bool _IsValid(std::string& name);
		std::string _GetName(ResourceID id);

		std::type_index _GetType(ResourceID& id);
		std::type_index _GetType(std::string& name);

	public:
		template<typename T, class... Args>
		static ResourceID Load(Args... constructorArgs) { return s_Instance->_Load<T, Args...>(constructorArgs...); }

		template<typename T, class... Args>
		static ResourceID LoadNamed(std::string name, Args... constructorArgs) { return s_Instance->_LoadNamed<T, Args...>(name, constructorArgs...); }

		template<typename T>
		static T* Get(ResourceID id) { return s_Instance->_Get<T>(id); }

		template<typename T>
		static T* Get(std::string name) { return s_Instance->_Get<T>(name); }

		static std::string GetName(ResourceID id) { return s_Instance->_GetName(id); }

		static void Unload(ResourceID id) { return s_Instance->_Unload(id); }
		static void Unload(std::string name) { return s_Instance->_Unload(name); }

		static void UnloadAll() { return s_Instance->_UnloadAll(); }

		static bool IsValid(ResourceID id) { return s_Instance->_IsValid(id); }
		static bool IsValid(std::string name) { return s_Instance->_IsValid(name); }

		static std::type_index GetType(ResourceID id) { return s_Instance->_GetType(id); }
		static std::type_index GetType(std::string name) { return s_Instance->_GetType(name); }

		template<typename T>
		static bool IsType(ResourceID id) { return GetType(id) == typeid(T); }
	};
}
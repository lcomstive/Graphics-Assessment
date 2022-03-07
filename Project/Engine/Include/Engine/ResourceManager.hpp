#pragma once
#include <typeindex>
#include <unordered_map>
#include <Engine/Log.hpp>
#include <Engine/ResourceID.hpp>

namespace Engine
{
	class Application; // Forward declaration

	class ResourceManager
	{
		static ResourceManager* s_Instance;

		friend class Engine::Application;

		struct ResourceInstance
		{
			void* Data;
			std::type_index Type;
		};

		std::unordered_map<ResourceID, ResourceInstance> m_Instances;

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

		template<typename T>
		T* _Get(ResourceID id)
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

		void _Unload(ResourceID id);

		void _UnloadAll();
		bool _IsValid(ResourceID id);

		std::type_index& _GetType(ResourceID id);

	public:
		template<typename T, class... Args>
		static ResourceID Load(Args... constructorArgs) { return s_Instance->_Load<T, Args...>(constructorArgs...); }

		template<typename T>
		static T* Get(ResourceID id) { return s_Instance->_Get<T>(id); }

		static void Unload(ResourceID id) { return s_Instance->_Unload(id); }

		static void UnloadAll() { return s_Instance->_UnloadAll(); }

		static bool IsValid(ResourceID id) { return s_Instance->_IsValid(id); }

		static std::type_index& GetType(ResourceID id) { return s_Instance->_GetType(id); }

		template<typename T>
		static bool IsType(ResourceID id) { return GetType(id) == typeid(T); }
	};
}
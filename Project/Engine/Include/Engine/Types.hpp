#pragma once
#include <Engine/Api.hpp>

#if defined(NDEBUG)
#include <robin_hood.h>

template<typename K, typename V>
using EngineUnorderedMap = robin_hood::unordered_map<K, V>;

using EngineString = std::string;
#else
#include <unordered_map>

template<typename K, typename V>
using EngineUnorderedMap = std::unordered_map<K, V>;

using EngineString = std::string;
#endif
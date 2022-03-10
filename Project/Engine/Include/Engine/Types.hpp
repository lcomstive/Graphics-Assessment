#pragma once
#include <Engine/Api.hpp>

#define USE_STRING_ID	1 // Use hashing to map strings to IDs
#define USE_FASTER_MAPS 1 // Use robin_hood instead of stl unordered_maps

#if USE_FASTER_MAPS
#include <robin_hood.h>

template<typename K, typename V>
using EngineUnorderedMap = robin_hood::unordered_map<K, V>;
#else
#include <unordered_map>

template<typename K, typename V>
using EngineUnorderedMap = std::unordered_map<K, V>;
#endif

#if USE_STRING_ID
#include <iostream> // Because sid.h ostream overloads << but doesn't include iostream...
#include <sid/sid.h>
#else
#include <string>
#endif
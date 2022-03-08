#pragma once
#include <string>
#include <vector>
#include <exception>
#include <glm/glm.hpp>

#define _WRITESTREAM(type, streamType) template<> ENGINE_EXPORT DataStream* Write<type>(type t) { \
																		unsigned char data[sizeof(type)]; \
																		memcpy(data, &t, sizeof(type)); \
																		InternalWrite(streamType, data, sizeof(type)); \
																		return this; }
#define _READSTREAM(type, streamType) template<> ENGINE_EXPORT type Read<type>() { \
																		type t; \
																		memcpy(&t, InternalRead(streamType, sizeof(type)), sizeof(type)); \
																		return t; }

namespace Engine
{
	class DataStream
	{
	private:
		enum class StreamType
		{
			CHAR = 1,
			INT,
			UINT,
			FLOAT,
			DOUBLE,
			BOOL,
			LONG,
			LLONG,
			SHORT,
			USHORT,
			STRING,
			CHARARRAY
		};

		static const size_t s_InitialStreamSize;

		unsigned char* m_Data;
		bool m_Writing;
		size_t m_Index, m_Length;

		ENGINE_API void InternalWrite(StreamType streamType, unsigned char* GetData, size_t length);
		ENGINE_API unsigned char* InternalRead(StreamType streamType, size_t length);
		ENGINE_API unsigned char* InternalReadArray(StreamType streamType, size_t* length);
	public:
		ENGINE_API DataStream(std::string path);
		ENGINE_API DataStream(DataStream* other);
		ENGINE_API DataStream(const DataStream& other);
		ENGINE_API DataStream(unsigned char* data, size_t length);
		ENGINE_API DataStream(size_t initialSize = s_InitialStreamSize);
		ENGINE_API DataStream(std::vector<unsigned char> data, size_t length = 0);
		ENGINE_API ~DataStream();

		ENGINE_EXPORT static DataStream Empty()
		{
			DataStream stream = DataStream((size_t)0);
			stream.m_Writing = false;
			return stream;
		}

		ENGINE_EXPORT bool IsReading() const { return !m_Writing; }
		ENGINE_EXPORT bool IsWriting() const { return m_Writing; }
		ENGINE_EXPORT bool IsEmpty() const { return GetLength() == 0; }
		ENGINE_EXPORT size_t GetLength() const { return IsWriting() ? m_Index : m_Length; }
		ENGINE_EXPORT bool AvailableLength() const { return IsWriting() ? false : m_Index < (m_Length - 1); }

		ENGINE_API void SetReading();
		ENGINE_API void SetWriting();

		ENGINE_API void SaveTo(std::string path);

		ENGINE_EXPORT std::vector<unsigned char> GetData()
		{
			std::vector<unsigned char> d;
			GetData(&d);
			return d;
		}

		ENGINE_EXPORT void GetData(std::vector<unsigned char>* output)
		{
			if (!output)
				return;
			output->resize(GetLength());
			if (GetLength() > 0)
				memcpy(output->data(), m_Data, GetLength());
		}

		ENGINE_EXPORT void GetData(unsigned char output[])
		{
			if (output == nullptr)
				output = new unsigned char[GetLength()];
			memcpy(output, m_Data, (size_t)GetLength());
		}

		ENGINE_API void Reserve(size_t additionalLength);

		template<typename T>
		ENGINE_EXPORT DataStream* Write(T t) { throw std::runtime_error("Tried writing invalid type to stream"); }

		ENGINE_EXPORT DataStream* Write(unsigned char* c, unsigned int length) { InternalWrite(StreamType::CHARARRAY, c, length); return this; }

		template<> ENGINE_EXPORT DataStream* Write<std::byte>(std::byte b) { Write((unsigned char)b); return this; }
		template<> ENGINE_EXPORT DataStream* Write<unsigned char>(unsigned char c) { InternalWrite(StreamType::CHAR, &c, sizeof(unsigned char)); return this; }
		template<> ENGINE_EXPORT DataStream* Write<bool>(bool b) { unsigned char c = b ? '1' : '0'; InternalWrite(StreamType::BOOL, &c, sizeof(bool)); return this; }
		template<> ENGINE_EXPORT DataStream* Write<char*>(char* c) { InternalWrite(StreamType::CHARARRAY, (unsigned char*)c, (unsigned int)strlen((const char*)c)); return this; }
		template<> ENGINE_EXPORT DataStream* Write<const char*>(const char* c) { InternalWrite(StreamType::CHARARRAY, (unsigned char*)c, (unsigned int)strlen(c)); return this; }
		template<> ENGINE_EXPORT DataStream* Write<unsigned char*>(unsigned char* c) { InternalWrite(StreamType::CHARARRAY, c, (unsigned int)strlen((const char*)c)); return this; }
		template<> ENGINE_EXPORT DataStream* Write<std::string>(std::string s) { InternalWrite(StreamType::STRING, (unsigned char*)s.c_str(), (unsigned int)s.size()); return this; }
		template<> ENGINE_EXPORT DataStream* Write<glm::vec2>(glm::vec2 v) { Write<float>(v.x); Write<float>(v.y); return this; }
		template<> ENGINE_EXPORT DataStream* Write<glm::vec3>(glm::vec3 v) { Write<float>(v[0]); Write<float>(v[1]); Write<float>(v[2]); return this; }
		template<> ENGINE_EXPORT DataStream* Write<glm::vec4>(glm::vec4 v) { Write<float>(v[0]); Write<float>(v[1]); Write<float>(v[2]); Write<float>(v[3]); return this; }
		template<> ENGINE_EXPORT DataStream* Write<size_t>(size_t t) { Write<unsigned int>((unsigned int)t); return this; }
		template<> ENGINE_EXPORT DataStream* Write<glm::ivec2>(glm::ivec2 d) { Write<int>(d.x); Write<int>(d.y); return this; }

		_WRITESTREAM(int, StreamType::INT)
		_WRITESTREAM(unsigned int, StreamType::UINT)
		_WRITESTREAM(float, StreamType::FLOAT)
		_WRITESTREAM(double, StreamType::DOUBLE)
		_WRITESTREAM(long, StreamType::LONG)
		_WRITESTREAM(long long, StreamType::LLONG)
		_WRITESTREAM(short, StreamType::SHORT)
		_WRITESTREAM(unsigned short, StreamType::USHORT)

		template<typename T> ENGINE_EXPORT T Read() { throw std::runtime_error("Tried reading invalid type"); }
		template<typename T> ENGINE_EXPORT T ReadArray(size_t* length) { throw std::runtime_error("Tried reading invalid type"); }

		template<> ENGINE_EXPORT char* ReadArray<char*>(size_t* length)
		{
			if (length == nullptr) return nullptr;
			return (char*)InternalReadArray(StreamType::CHARARRAY, length);
		}

		template<> ENGINE_EXPORT const char* ReadArray<const char*>(size_t* length)
		{
			if (length == nullptr) return nullptr;
			return (const char*)InternalReadArray(StreamType::CHARARRAY, length);
		}

		template<> ENGINE_EXPORT unsigned char* ReadArray<unsigned char*>(size_t* length)
		{
			if (length == nullptr) return nullptr;
			return (unsigned char*)InternalReadArray(StreamType::CHARARRAY, length);
		}

		template<> ENGINE_EXPORT std::byte Read<std::byte>() { return (std::byte)Read<unsigned char>(); }
		template<> ENGINE_EXPORT unsigned char Read<unsigned char>() { return InternalRead(StreamType::CHAR, sizeof(unsigned char))[0]; }
		template<> ENGINE_EXPORT bool Read<bool>() { return InternalRead(StreamType::BOOL, sizeof(bool))[0] == (unsigned char)'1'; }
		template<> ENGINE_EXPORT char* Read<char*>() { size_t length = 0; return ReadArray<char*>(&length); }
		template<> ENGINE_EXPORT const char* Read<const char*>() { size_t length = 0; return ReadArray<const char*>(&length); }
		template<> ENGINE_EXPORT unsigned char* Read<unsigned char*>() { size_t length = 0; return ReadArray<unsigned char*>(&length); }
		template<> ENGINE_EXPORT std::string Read<std::string>()
		{
			size_t length = 0;
			unsigned char* GetData = InternalReadArray(StreamType::STRING, &length);
			return std::string((char*)GetData, length);
		}
		template<> ENGINE_EXPORT glm::vec2 Read<glm::vec2>() { return { Read<float>(), Read<float>() }; }
		template<> ENGINE_EXPORT glm::vec3 Read<glm::vec3>() { return { Read<float>(), Read<float>(), Read<float>() }; }
		template<> ENGINE_EXPORT glm::vec4 Read<glm::vec4>() { return { Read<float>(), Read<float>(), Read<float>(), Read<float>() }; }
		template<> ENGINE_EXPORT size_t Read<size_t>() { return (size_t)Read<unsigned int>(); }
		template<> ENGINE_EXPORT glm::ivec2 Read<glm::ivec2>() { return { Read<int>(), Read<int>() }; }

		_READSTREAM(int, StreamType::INT)
		_READSTREAM(unsigned int, StreamType::UINT)
		_READSTREAM(float, StreamType::FLOAT)
		_READSTREAM(double, StreamType::DOUBLE)
		_READSTREAM(long, StreamType::LONG)
		_READSTREAM(long long, StreamType::LLONG)
		_READSTREAM(short, StreamType::SHORT)
		_READSTREAM(unsigned short, StreamType::USHORT)

		template<typename T>
		ENGINE_EXPORT void Serialize(T* data)
		{
			if (IsReading())
				*data = Read<T>();
			else
				Write<T>(*data);
		}
	};
}

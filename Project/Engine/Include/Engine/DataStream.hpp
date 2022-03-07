#pragma once
#include <string>
#include <vector>
#include <exception>
#include <glm/glm.hpp>

#define _WRITESTREAM(type, streamType) template<> DataStream* Write<type>(type t) { \
																		unsigned char data[sizeof(type)]; \
																		memcpy(data, &t, sizeof(type)); \
																		InternalWrite(streamType, data, sizeof(type)); \
																		return this; }
#define _READSTREAM(type, streamType) template<> type Read<type>() { \
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

		void InternalWrite(StreamType streamType, unsigned char* GetData, size_t length);
		unsigned char* InternalRead(StreamType streamType, size_t length);
		unsigned char* InternalReadArray(StreamType streamType, size_t* length);
	public:
		DataStream(std::string path);
		DataStream(DataStream* other);
		DataStream(const DataStream& other);
		DataStream(unsigned char* data, size_t length);
		DataStream(size_t initialSize = s_InitialStreamSize);
		DataStream(std::vector<unsigned char> data, size_t length = 0);
		~DataStream();

		static DataStream Empty()
		{
			DataStream stream = DataStream((size_t)0);
			stream.m_Writing = false;
			return stream;
		}

		bool IsReading() const { return !m_Writing; }
		bool IsWriting() const { return m_Writing; }
		bool IsEmpty() const { return GetLength() == 0; }
		size_t GetLength() const { return IsWriting() ? m_Index : m_Length; }
		bool AvailableLength() const { return IsWriting() ? false : m_Index < (m_Length - 1); }

		void SetReading();
		void SetWriting();

		void SaveTo(std::string path);

		std::vector<unsigned char> GetData()
		{
			std::vector<unsigned char> d;
			GetData(&d);
			return d;
		}

		void GetData(std::vector<unsigned char>* output)
		{
			if (!output)
				return;
			output->resize(GetLength());
			if (GetLength() > 0)
				memcpy(output->data(), m_Data, GetLength());
		}
		void GetData(unsigned char output[])
		{
			if (output == nullptr)
				output = new unsigned char[GetLength()];
			memcpy(output, m_Data, (size_t)GetLength());
		}

		void Reserve(size_t additionalLength);

		template<typename T> DataStream* Write(T t) { throw std::runtime_error("Tried writing invalid type to stream"); }

		DataStream* Write(unsigned char* c, unsigned int length) { InternalWrite(StreamType::CHARARRAY, c, length); return this; }

		template<> DataStream* Write<std::byte>(std::byte b) { Write((unsigned char)b); return this; }
		template<> DataStream* Write<unsigned char>(unsigned char c) { InternalWrite(StreamType::CHAR, &c, sizeof(unsigned char)); return this; }
		template<> DataStream* Write<bool>(bool b) { unsigned char c = b ? '1' : '0'; InternalWrite(StreamType::BOOL, &c, sizeof(bool)); return this; }
		template<> DataStream* Write<char*>(char* c) { InternalWrite(StreamType::CHARARRAY, (unsigned char*)c, (unsigned int)strlen((const char*)c)); return this; }
		template<> DataStream* Write<const char*>(const char* c) { InternalWrite(StreamType::CHARARRAY, (unsigned char*)c, (unsigned int)strlen(c)); return this; }
		template<> DataStream* Write<unsigned char*>(unsigned char* c) { InternalWrite(StreamType::CHARARRAY, c, (unsigned int)strlen((const char*)c)); return this; }
		template<> DataStream* Write<std::string>(std::string s) { InternalWrite(StreamType::STRING, (unsigned char*)s.c_str(), (unsigned int)s.size()); return this; }
		template<> DataStream* Write<glm::vec2>(glm::vec2 v) { Write<float>(v.x); Write<float>(v.y); return this; }
		template<> DataStream* Write<glm::vec3>(glm::vec3 v) { Write<float>(v[0]); Write<float>(v[1]); Write<float>(v[2]); return this; }
		template<> DataStream* Write<glm::vec4>(glm::vec4 v) { Write<float>(v[0]); Write<float>(v[1]); Write<float>(v[2]); Write<float>(v[3]); return this; }
		template<> DataStream* Write<size_t>(size_t t) { Write<unsigned int>((unsigned int)t); return this; }
		template<> DataStream* Write<glm::ivec2>(glm::ivec2 d) { Write<int>(d.x); Write<int>(d.y); return this; }

		_WRITESTREAM(int, StreamType::INT)
		_WRITESTREAM(unsigned int, StreamType::UINT)
		_WRITESTREAM(float, StreamType::FLOAT)
		_WRITESTREAM(double, StreamType::DOUBLE)
		_WRITESTREAM(long, StreamType::LONG)
		_WRITESTREAM(long long, StreamType::LLONG)
		_WRITESTREAM(short, StreamType::SHORT)
		_WRITESTREAM(unsigned short, StreamType::USHORT)

		template<typename T> T Read() { throw std::runtime_error("Tried reading invalid type"); }
		template<typename T> T ReadArray(size_t* length) { throw std::runtime_error("Tried reading invalid type"); }

		template<> char* ReadArray<char*>(size_t* length)
		{
			if (length == nullptr) return nullptr;
			return (char*)InternalReadArray(StreamType::CHARARRAY, length);
		}

		template<> const char* ReadArray<const char*>(size_t* length)
		{
			if (length == nullptr) return nullptr;
			return (const char*)InternalReadArray(StreamType::CHARARRAY, length);
		}

		template<> unsigned char* ReadArray<unsigned char*>(size_t* length)
		{
			if (length == nullptr) return nullptr;
			return (unsigned char*)InternalReadArray(StreamType::CHARARRAY, length);
		}

		template<> std::byte Read<std::byte>() { return (std::byte)Read<unsigned char>(); }
		template<> unsigned char Read<unsigned char>() { return InternalRead(StreamType::CHAR, sizeof(unsigned char))[0]; }
		template<> bool Read<bool>() { return InternalRead(StreamType::BOOL, sizeof(bool))[0] == (unsigned char)'1'; }
		template<> char* Read<char*>() { size_t length = 0; return ReadArray<char*>(&length); }
		template<> const char* Read<const char*>() { size_t length = 0; return ReadArray<const char*>(&length); }
		template<> unsigned char* Read<unsigned char*>() { size_t length = 0; return ReadArray<unsigned char*>(&length); }
		template<> std::string Read<std::string>()
		{
			size_t length = 0;
			unsigned char* GetData = InternalReadArray(StreamType::STRING, &length);
			return std::string((char*)GetData, length);
		}
		template<> glm::vec2 Read<glm::vec2>() { return { Read<float>(), Read<float>() }; }
		template<> glm::vec3 Read<glm::vec3>() { return { Read<float>(), Read<float>(), Read<float>() }; }
		template<> glm::vec4 Read<glm::vec4>() { return { Read<float>(), Read<float>(), Read<float>(), Read<float>() }; }
		template<> size_t Read<size_t>() { return (size_t)Read<unsigned int>(); }
		template<> glm::ivec2 Read<glm::ivec2>() { return { Read<int>(), Read<int>() }; }

		_READSTREAM(int, StreamType::INT)
		_READSTREAM(unsigned int, StreamType::UINT)
		_READSTREAM(float, StreamType::FLOAT)
		_READSTREAM(double, StreamType::DOUBLE)
		_READSTREAM(long, StreamType::LONG)
		_READSTREAM(long long, StreamType::LLONG)
		_READSTREAM(short, StreamType::SHORT)
		_READSTREAM(unsigned short, StreamType::USHORT)

		template<typename T>
		void Serialize(T* data)
		{
			if (IsReading())
				*data = Read<T>();
			else
				Write<T>(*data);
		}
	};
}

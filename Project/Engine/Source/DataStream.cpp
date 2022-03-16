#include <string>
#include <iostream>
#include <Engine/Utilities.hpp>
#include <Engine/DataStream.hpp>

using namespace std;
using namespace Engine;

const size_t DataStream::s_InitialStreamSize = 4096; // 4KB

DataStream::DataStream(size_t initialSize) : m_Writing(true), m_Index(0), m_Length(initialSize), m_Data(new unsigned char[initialSize]) { }
DataStream::DataStream(const DataStream& other) : m_Index(0), m_Writing(false)
{
	m_Length = other.GetLength();
	m_Data = new unsigned char[m_Length];
	memcpy(m_Data, other.m_Data, (size_t)m_Length);
}

DataStream::DataStream(DataStream* other) : m_Index(0), m_Writing(false)
{
	m_Length = (unsigned int)other->GetLength();
	m_Data = new unsigned char[m_Length];
	memcpy(m_Data, other->m_Data, (size_t)m_Length);
}

DataStream::DataStream(vector<unsigned char> GetData, size_t length) : m_Index(0), m_Writing(false)
{
	m_Length = length > 0 ? length : (unsigned int)GetData.size();
	m_Data = new unsigned char[m_Length];
	memcpy(m_Data, GetData.data(), (size_t)m_Length);
}

DataStream::DataStream(unsigned char* GetData, size_t length) : m_Index(0), m_Writing(false)
{
	if (length <= 0)
		throw std::runtime_error("Length is not assigned");

	m_Length = length > 0 ? length : (unsigned int)length;
	m_Data = new unsigned char[m_Length];
	memcpy(m_Data, GetData, (size_t)m_Length);
}

DataStream::DataStream(string path) : DataStream(Engine::Read(path)) { }

DataStream::~DataStream() { delete[] m_Data; }

void DataStream::SaveTo(string path) { Engine::Write(path, vector<unsigned char>(m_Data, m_Data + m_Index)); }
DataStream DataStream::ReadFrom(string path) { return DataStream(Engine::Read(path)); }

void DataStream::SetReading()
{
	m_Writing = false;
	m_Length = m_Index;
	m_Index = 0;
}

void DataStream::SetWriting()
{
	m_Writing = true;
	m_Index = 0;
}

void DataStream::Reserve(size_t additionalLength)
{
	size_t newLength = m_Length + additionalLength;
	unsigned char* newData = new unsigned char[newLength];
	memcpy(newData, m_Data, m_Length);
	delete[] m_Data;
	m_Data = newData;
	m_Length = newLength;
}

void DataStream::InternalWrite(StreamType type, unsigned char* GetData, size_t length)
{
	if (!m_Writing)
	{
#ifndef NDEBUG
		cerr << "Tried to write to DataStream in reading mode" << endl;
#endif
		throw std::runtime_error("Tried to write to DataStream that was in reading mode");
	}

	if ((length + m_Index + 1) > m_Length)
		Reserve(s_InitialStreamSize + length * 2);

	m_Data[m_Index++] = (unsigned char)type;
	if (type == StreamType::STRING || type == StreamType::CHARARRAY)
	{
		memcpy(m_Data + m_Index, &length, sizeof(unsigned int));
		m_Index += sizeof(unsigned int);
	}

	memcpy(m_Data + m_Index, GetData, length);
	m_Index += length;
}

unsigned char* DataStream::InternalRead(StreamType expectedType, size_t length) { return InternalReadArray(expectedType, &length); }
unsigned char* DataStream::InternalReadArray(StreamType expectedType, size_t* length)
{
	if (m_Writing)
	{
#ifndef NDEBUG
		cerr << "Tried reading DataStream while in write mode" << endl;
#endif
		throw std::runtime_error("Tried reading DataStream but is in writing mode");
	}

	StreamType type = (StreamType)m_Data[m_Index++];
	if (type != expectedType)
	{
#ifndef NDEBUG
		cerr << "Read wrong type, aborting..." << endl;
#endif
		throw std::runtime_error("Read wrong type, aborting...");
	}

	if (type == StreamType::STRING || type == StreamType::CHARARRAY)
	{
		memcpy(length, m_Data + m_Index, sizeof(unsigned int));
		m_Index += sizeof(unsigned int);
	}

	m_Index += *length;
	return m_Data + m_Index - *length;
}

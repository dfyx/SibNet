#include <blocks/binaryblock.h>
#include <cstring>

REGISTER_BLOCKTYPE(BinaryBlock, BLOCKTYPE_BINARY)

BinaryBlock::BinaryBlock()
{
	m_pcData = NULL;
	m_iSize = 0;
}

BinaryBlock::BinaryBlock(char* p_pcData, int p_iSize)
{
	// Set new data
	m_pcData = new char[p_iSize];
	memcpy(m_pcData, p_pcData, p_iSize);

	// Update size
	m_iSize = p_iSize;
}

BinaryBlock::~BinaryBlock()
{
	delete m_pcData;
}

int BinaryBlock::GetData(char** p_ppcData)
{
	if(p_ppcData != NULL)
	{
		*p_ppcData = new char[m_iSize];
		memcpy(*p_ppcData, m_pcData, m_iSize);
		return m_iSize;
	}
	else
	{
		return -1;
	}
}

void BinaryBlock::SetData(char* p_pcData, int p_iSize)
{
	// Delete old data
	if (m_pcData != NULL)
	{
		delete m_pcData;
	}

	// Set new data
	m_pcData = new char[p_iSize];
	memcpy(m_pcData, p_pcData, p_iSize);

	// Update size
	m_iSize = p_iSize;
}

int BinaryBlock::GetDataSize()
{
	return m_iSize;
}

size_t BinaryBlock::Serialize(char** p_ppcBuffer)
{
	return GetData(p_ppcBuffer);
}

void BinaryBlock::Deserialize(char* p_pcBuffer, size_t p_iSize)
{
	SetData(p_pcBuffer, p_iSize);
}
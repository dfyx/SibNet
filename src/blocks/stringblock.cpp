#include <blocks/stringblock.h>
#include <cstring>

using namespace std;

REGISTER_BLOCKTYPE(StringBlock, BLOCKTYPE_STRING)

StringBlock::StringBlock()
{
}

StringBlock::StringBlock(string p_strData)
{
	m_strData = p_strData;
}

string StringBlock::GetData()
{
	return m_strData;
}

void StringBlock::SetData(string p_strData)
{
	m_strData = p_strData;
}

size_t StringBlock::Serialize(char** p_ppcBuffer)
{
	if(p_ppcBuffer != NULL)
	{
		size_t iLength = m_strData.length();
		*p_ppcBuffer = new char[iLength];
		memcpy(*p_ppcBuffer, m_strData.data(), iLength);
		return iLength;
	}
	else
	{
		return -1;
	}
}

void StringBlock::Deserialize(char* p_pcBuffer, size_t p_iSize)
{
	m_strData = string(p_pcBuffer, p_iSize);
}
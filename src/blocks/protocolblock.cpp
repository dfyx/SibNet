#include <blocks/protocolblock.h>

using namespace std;

REGISTER_BLOCKTYPE(ProtocolBlock, BLOCKTYPE_PROTOCOL)

ProtocolBlock::ProtocolBlock()
{
	m_strName = "";
	m_iVersion = 0;
}

ProtocolBlock::ProtocolBlock(string p_strName, long p_iVersion)
{
	m_strName = p_strName;
	m_iVersion = p_iVersion;
}

string ProtocolBlock::getName()
{
	return m_strName;
}

long ProtocolBlock::getVersion()
{
	return m_iVersion;
}

size_t ProtocolBlock::Serialize(char** p_ppcBuffer)
{
	long iConvertedVersion = htonl(m_iVersion);

	*p_ppcBuffer = new char[4 + m_strName.length()];
	memcpy(*p_ppcBuffer, &iConvertedVersion, 4);
	memcpy(*p_ppcBuffer + 4, m_strName.c_str(), m_strName.length());

	return m_strName.length() + 4;
}

void ProtocolBlock::Deserialize(char* p_pcBuffer, size_t p_iSize)
{
	memcpy(&m_iVersion, p_pcBuffer, 4);
	m_iVersion = ntohl(m_iVersion);
	m_strName = string(p_pcBuffer + 4, p_iSize - 4);
}
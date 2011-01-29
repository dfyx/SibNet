#ifndef PROTOCOLBLOCK_H
#define PROTOCOLBLOCK_H

#include <networking.h>
#include <blocks/block.h>
#include <string>

class DLLDIR ProtocolBlock: public Block {
  // Associations
  // Attributes
protected:
	std::string m_strName;
	uint32_t m_iVersion;
  // Operations
public:
	ProtocolBlock();
	ProtocolBlock(std::string p_strName, long p_iVersion);
	std::string getName();
    long getVersion();

	virtual blocksize_t Serialize(char** p_ppcBuffer);
	virtual void Deserialize(char* p_pcBuffer, blocksize_t p_iSize);

	MAKE_BLOCKTYPE(ProtocolBlock, BLOCKTYPE_PROTOCOL)
};

#endif

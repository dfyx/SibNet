#ifndef COMPLEXBLOCK_H
#define COMPLEXBLOCK_H

#include <networking.h>
#include <blocks/block.h>
#include <vector>

class DLLDIR ComplexBlock: public Block
{
  // Associations
protected:
	std::vector<Block*> m_vSubblocks;
  // Attributes
  // Operations
protected:
	~ComplexBlock();

public:
	virtual uint16_t GetTypeID() = 0;
	virtual size_t Serialize(char** p_ppcBuffer);
	virtual void Deserialize(char* p_pcBuffer, size_t p_iSize);
};

#endif

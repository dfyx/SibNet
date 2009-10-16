#ifndef BLOCK_H
#define BLOCK_H

#include <networking.h>
#include <blockinterpreter.h>
#include <map>

#include <stdint.h>

DLLDIR enum
{
	BLOCKTYPE_NONE = 0,
	BLOCKTYPE_PROTOCOL,
	BLOCKTYPE_BINARY,
	BLOCKTYPE_STRING,
	BLOCKTYPE_FIRSTFREE
};

class DLLDIR Block
{
  // Associations
  // Attributes
private:
	static std::map<uint16_t, Block* (*)()>* m_mapBlockTypes;

  // Operations

public:
    virtual uint16_t GetTypeID() = 0;
    virtual size_t Serialize(char** p_ppcBuffer) = 0;
    virtual void Deserialize(char* p_pcBuffer, size_t p_iSize) = 0;
    virtual void Execute(BlockInterpreter* p_inInterpreter);

	// Block type management
	static void RegisterBlockTypeID(uint16_t p_iBlockType, Block* (*p_pConstructor)());
	static Block* GetBlockFromTypeID(uint16_t p_iBlockType);
};

/* Automatic block type registration */
#define MAKE_BLOCKTYPE(Type, id)								\
public:															\
	static Block* Create() { return new Type(); }				\
	virtual uint16_t GetTypeID() { return id; }					\
private:														\
	static const char unused;

#define REGISTER_BLOCKTYPE(Type, id)								\
const char Type::unused =										\
	(Block::RegisterBlockTypeID(id,	&(Type::Create)), 0);

#endif

#ifndef BLOCK_H
#define BLOCK_H

#include <networking.h>
#include <blockinterpreter.h>
#include <map>

#include <stdint_wrap.h>

DLLDIR enum
{
	BLOCKTYPE_NONE = 0,
	BLOCKTYPE_PROTOCOL,
	BLOCKTYPE_BINARY,
	BLOCKTYPE_STRING,
	BLOCKTYPE_FIRSTFREE
};

// Definitions for blocksize and stuff
DLLDIR typedef uint16_t blocksize_t;

class DLLDIR Block
{
  // Associations
  // Attributes
private:
	static std::map<uint16_t, Block* (*)()>* m_mapBlockTypes;

  // Operations

public:
    virtual uint16_t GetTypeID() = 0;
    virtual blocksize_t Serialize(char** p_ppcBuffer) = 0;
    virtual void Deserialize(char* p_pcBuffer, blocksize_t p_iSize) = 0;
    virtual void Execute(BlockInterpreter* p_inInterpreter);

	// Block type management
	static void RegisterBlockTypeID(uint16_t p_iBlockType, Block* (*p_pConstructor)());
	static Block* GetBlockFromTypeID(uint16_t p_iBlockType);
};


/* Automatic block type registration */
// Macro version
	#define MAKE_BLOCKTYPE(Type, id)								\
	public:															\
		static Block* Create() { return new Type(); }				\
		virtual uint16_t GetTypeID() { return id; }					\
	private:														\
		static const char unused;

	#define REGISTER_BLOCKTYPE(Type, id)								\
	const char Type::unused =										\
		(Block::RegisterBlockTypeID(id,	&(Type::Create)), 0);


// Template version
	template<class _t, uint16_t _id>
	class Block_t : public Block
	{
	public:
		static Block* Create() { return new _t(); }
		virtual uint16_t GetTypeID() { return _id; }
	public:
		static const char unused;
	};
	template<class _t, uint16_t _id>
	const char Block_t<_t, _id>::unused = (Block::RegisterBlockTypeID(_id,	&(Block_t<_t, _id>::Create)), 42);

#endif

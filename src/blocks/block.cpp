#include <blocks/block.h>
#include <exceptions.h>
#include <iostream>

using namespace std;

map<uint16_t, Block* (*)()>* Block::m_mapBlockTypes = NULL;

Block* Block::GetBlockFromTypeID(uint16_t p_iBlockType)
{
	if(m_mapBlockTypes != NULL && m_mapBlockTypes->count(p_iBlockType))
	{
		Block* pBlock = (*m_mapBlockTypes)[p_iBlockType]();
		return pBlock;
	}
	else
	{
		return NULL;
	}
}

void Block::RegisterBlockTypeID(uint16_t p_iBlockType, Block* (*p_pConstructor)())
{
	if(m_mapBlockTypes == NULL)
	{
		m_mapBlockTypes = new map<uint16_t, Block* (*)()>();
	}

	(*m_mapBlockTypes)[p_iBlockType] = p_pConstructor;
}

void Block::Execute(BlockInterpreter* p_inInterpreter)
{
	throw NotExecutableException(this);
}
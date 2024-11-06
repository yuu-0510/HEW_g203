#pragma once
#include "ObjectBase.h"
#include "Block.h"

#define MAP_COLUMN		(100)
#define MAP_ROW			(30)

class CMap : public CObjectBase
{
public:
	CMap();
	~CMap();
private:
	CBlock* m_Map[MAP_ROW][MAP_COLUMN];
};


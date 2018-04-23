#include "BGSAttackData.h"


BGSAttackData * BGSAttackData::Create()
{
	BGSAttackData *attackData = (BGSAttackData *) Heap_Allocate(sizeof(BGSAttackData));
	if (attackData)
	{
		CALL_MEMBER_FN(attackData, ctor)();
	}

	return attackData;
}
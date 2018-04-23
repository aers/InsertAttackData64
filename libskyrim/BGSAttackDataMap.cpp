#include "BGSAttackDataMap.h"


BGSAttackData * BGSAttackDataMap::Get(const BSFixedString & eventName)
{
	auto iter = map.find(eventName);
	return (iter != map.end()) ? iter->value : nullptr;
}

BGSAttackData * BGSAttackDataMap::Add(const BSFixedString & eventName)
{
	BSFixedString evName(eventName.c_str()); //refcount+1

	BGSAttackData * ptr = BGSAttackData::Create();
	ptr->IncRef();
	if (ptr)
	{
		ptr->eventName = evName;
		map.insert(evName, ptr);
	}

	return ptr;
}

BGSAttackData * BGSAttackDataMap::Add(BGSAttackData *attackData)
{
	if (attackData)
	{
		BGSAttackData * ptr = attackData;

		map.insert(attackData->eventName, ptr);
	}

	return attackData;
}

void BGSAttackDataMap::Remove(const BSFixedString & eventName)
{
	auto iter = map.find(eventName);
	if (iter != map.end())
	{
		map.erase(eventName);
	}
}
#pragma once

#include "BGSAttackData.h"
#include "BSTHashMap.h"
#include "../skse64/NiObjects.h"

#include "../skse64_common/Utilities.h"

class TESRace;
class TESFile;

/*==============================================================================
class BGSAttackDataMap +0000 (_vtbl=010C013C)
0000: class BGSAttackDataMap
0000: |   class NiRefObject
==============================================================================*/
// 2C
class BGSAttackDataMap : public NiRefObject
{
public:
	typedef BSTHashMap<BSFixedString, BGSAttackData *> HashMap;

	BGSAttackData * Get(const BSFixedString & eventName);
	BGSAttackData * Add(const BSFixedString & eventName);
	BGSAttackData * Add(BGSAttackData *attackData);
	void Remove(const BSFixedString &eventName);

	static HashMap::iterator begin(HashMap &a_map) {
		return a_map.begin();
	}

		//0 vftable
		//8 m_UiRefCount
		//C pad4
		//10 HashMap
			// -- base class padding --
			//1C m_size uint32
			//20 m_freeCount uint32
			//24 m_freeOffset uint32
			//28 * m_eolPtr uintptr_t
			//30 pad8
			//38 * m_entries uintptr_t
		//40 * attackRace uintptr_t

		//size(0x48)

	// @members
	//void	** _vtbl		// 00 - 010C013C
	HashMap	map;			// 10
	TESRace	* attackRace;	// 40 - init'd 0

	MEMBER_FN_PREFIX(BGSAttackDataMap);
	DEFINE_MEMBER_FN(AddFromMod, void, 0x003E81E0, TESRace *race, TESFile *mod);
	DEFINE_MEMBER_FN(ctor, BGSAttackDataMap*, 0x003E8080);
};

//class BGSAttackDataMap	size(72) :
//	+---
//0     | +--- (base class NiRefObject)
//0	    | | {vfptr}
//8	    | | m_uiRefCount
//0C    | | pad0C
//  | +---
//10    | ? $BSTHashMap@URef@StringCache@@V?$NiPointer@VBGSAttackData@@@@UBSTDefaultScatterTable@@ map
//40    | attackRace
//	+---

STATIC_ASSERT(offsetof(BGSAttackDataMap, map.m_size) == 0x1C);
STATIC_ASSERT(offsetof(BGSAttackDataMap, map.m_eolPtr) == 0x28);
STATIC_ASSERT(offsetof(BGSAttackDataMap, map.m_entries) == 0x38);
STATIC_ASSERT(offsetof(BGSAttackDataMap, attackRace) == 0x40);
STATIC_ASSERT(sizeof(BGSAttackDataMap) == 0x48);
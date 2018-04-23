#pragma once

#include "../skse64/NiObjects.h"
#include "../skse64/GameTypes.h"

#include "../skse64_common/Utilities.h"

class SpellItem;
class BGSKeyword;

/*==============================================================================
class BGSAttackData +0000 (_vtbl=010C0130)
0000: class BGSAttackData
0000: |   class NiRefObject
==============================================================================*/
// 38
class BGSAttackData : public NiRefObject
{
public:

	static BGSAttackData * Create();

	// @members
	//void			** _vtbl			// 00 - 161F5D0
	BSFixedString	eventName;			// 10
	float			damageMult;			// 18 init'd 1.0f
	float			attackChance;		// 1C init'd 1.0f
	SpellItem		* attackSpell;		// 20 init'd 0
	struct
	{
		bool	ignoreWeapon : 1;
		bool	bashAttack : 1;
		bool	powerAttack : 1;
		bool	leftAttack : 1;
		bool	rotatingAttack : 1;
	} flags;							// 28
	float			attackAngle;		// 2C init'd 0
	float			strikeAngle;		// 30 init'd [1E056B0]
	float			stagger;			// 34 init'd 0	
	BGSKeyword		* attackType;		// 38 init'd 0
	float			knockdown;			// 40 init'd 0
	float			recoveryTime;		// 44 init'd 0
	float			staminaMult;		// 48 init'd 1.0f


	MEMBER_FN_PREFIX(BGSAttackData);
	DEFINE_MEMBER_FN(ctor, BGSAttackData *, 0x003E7F60);	
};

STATIC_ASSERT(sizeof(BGSAttackData) == 0x50);
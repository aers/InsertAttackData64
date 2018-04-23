#include "../skse64/PluginAPI.h"
#include "../skse64_common/skse_version.h"
#include "../skse64_common/Relocation.h"
#include "../skse64_common/SafeWrite.h"
#include "../skse64/GameForms.h"
#include "../skse64/GameData.h"
#include "../skse64/GameStreams.h"
#include <shlobj.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <filesystem>
#include <inttypes.h>

#include "picojson\picojson.h"
#include "libskyrim\BGSAttackDataMap.h"

#ifdef _DEBUG
#include "DebugUtil.h"
#endif

// truncated name warning
#pragma warning(disable: 4503)

typedef std::vector<std::pair<std::string, picojson::value>> attackdata;
typedef std::map<std::string, attackdata> event_map;
typedef std::map<std::string, event_map> setting_map;

static setting_map s_setting;


inline static void Chomp(std::string & str, int c = '\r')
{
	std::size_t len = str.length();
	if (len > 0 && str[len - 1] == c)
	{
		str.resize(len - 1);
	}
}


inline static void ToLower(std::string & str)
{
	for (auto &n : str)
	{
		n = tolower(n);
	}
}


static std::string ToString(UInt32 id)
{
	DataHandler *dhnd = DataHandler::GetSingleton();
	UInt8 modIndex = id >> 24;
	if (modIndex >= dhnd->modList.loadedMods.count)
		return std::string("XXXXXX");

	std::stringstream ss;

	if (modIndex) {
		std::string modName = dhnd->modList.loadedMods[modIndex]->name;
		ToLower(modName);
		ss << modName << " ";
	}

	for (UInt32 i = 0; i < dhnd->modList.loadedMods.count; ++i) {
	}

	ss << std::hex << std::setfill('0') << std::setw(6) << (id & 0x00FFFFFF);
	return ss.str();
}


static bool ParseJson(std::string jsonFileName, picojson::value &val)
{
	std::stringstream ss;
	std::ifstream fileStream(jsonFileName);

	if (!fileStream)
	{
		_MESSAGE("**** error **** cannot read JSON file: %s", jsonFileName.c_str());
		return false;
	}

	ss << fileStream.rdbuf();

	std::string err = picojson::parse(val, ss);
	if (!err.empty())
	{
		_MESSAGE("**** error **** invalid JSON in %s\n  %s", jsonFileName, err.c_str());
		return false;
	}
	if (!val.is<picojson::object>())
	{
		_MESSAGE("**** error **** invalid top-level type in %s", jsonFileName);
		return false;
	}

	return true;
}


static void RemapJson(picojson::value &json, setting_map &setting)
{
	if (!json.is<picojson::object>())
		return;

	for (auto &kv : json.get<picojson::object>())
	{
		picojson::value &v = kv.second;
		if (!v.is<picojson::object>())
			continue;

		std::string key = kv.first;
		if (key.empty())
			continue;
		ToLower(key);

		picojson::object &events_in = v.get<picojson::object>();
		event_map &events_out = setting[key];

		for (auto &event : events_in)
		{
			picojson::value &value = event.second;
			if (!value.is<picojson::object>())
				continue;

			std::string eventName = event.first;
			if (eventName.empty())
				continue;

			picojson::object &attackdata_in = value.get<picojson::object>();
			attackdata &attackdata_out = events_out[eventName];
			attackdata_out.reserve(attackdata_in.size());

			for (auto &data : attackdata_in)
			{
				std::string name = data.first;
				if (name.empty())
					continue;
				ToLower(name);

				attackdata_out.push_back(std::make_pair(name, data.second));
			}
		}
	}
}


static void SetAttackData(BGSAttackData* attackData, attackdata &data)
{
	for (auto &val : data)
	{
		std::string &valName = val.first;

		_MESSAGE("    %s = %s", valName.c_str(), val.second.serialize().c_str());

		if (valName == "damagemult") {
			attackData->damageMult = val.second.get<double>();
		}
		else if (valName == "attackchance") {
			attackData->attackChance = val.second.get<double>();
		}
		else if (valName == "attackangle") {
			attackData->attackAngle = val.second.get<double>();
		}
		else if (valName == "strikeangle") {
			attackData->strikeAngle = val.second.get<double>();
		}
		else if (valName == "stagger") {
			attackData->stagger = val.second.get<double>();
		}
		else if (valName == "knockdown") {
			attackData->knockdown = val.second.get<double>();
		}
		else if (valName == "recoverytime") {
			attackData->recoveryTime = val.second.get<double>();
		}
		else if (valName == "staminamult") {
			attackData->staminaMult = val.second.get<double>();
		}
		else if (valName == "ignoreweapon") {
			attackData->flags.ignoreWeapon = val.second.get<bool>();
		}
		else if (valName == "bashattack") {
			attackData->flags.bashAttack = val.second.get<bool>();
		}
		else if (valName == "powerattack") {
			attackData->flags.powerAttack = val.second.get<bool>();
		}
		else if (valName == "leftattack") {
			attackData->flags.leftAttack = val.second.get<bool>();
		}
		else if (valName == "rotatingattack") {
			attackData->flags.rotatingAttack = val.second.get<bool>();
		}
		else {
			_MESSAGE("**** warning **** unknown record name \" %s \"", valName);
		}
	}
}


static void InsertAttackDataRecords(TESRace* race, event_map &events)
{
	for (auto &ev : events)
	{
		BSFixedString eventName(ev.first.c_str());

		BGSAttackDataMap * adm = (BGSAttackDataMap *)race->attackData.unk08.ptr;

		BGSAttackData *attackData = adm->Get(eventName);

		if (!attackData)
		{
			_MESSAGE("(NEW) %s::%s", race->editorId.c_str(), ev.first.c_str());
			attackData = adm->Add(eventName);
		}
		else // being lazy here
		{
			_MESSAGE("%s::%s", race->editorId.c_str(), ev.first.c_str());
		}

		if (attackData)
		{
			SetAttackData(attackData, ev.second);
		}
	}
}

static void InitAttackDataJson()
{
	namespace sys = std::tr2::sys;

	std::string baseDir("Meshes\\AttackData");

	// read json files
	sys::path p(std::string("Data\\") + baseDir);
	std::for_each(sys::directory_iterator(p), sys::directory_iterator(),
		[&baseDir](const sys::path& p)
	{
		if (!sys::is_regular_file(p))
			return;

		std::string ext = p.extension().string();
		if (_stricmp(ext.c_str(), ".json") != 0)
			return;

		std::string fileName = "Data\\" + baseDir + "\\" + p.filename().string();

		_MESSAGE("found file %s", fileName.c_str());
		// read json
		picojson::value val;
		if (ParseJson(fileName, val))
		{
			RemapJson(val, s_setting);
		}
	});
}

typedef bool(*_TESRaceLoadForm)(TESRace * race, int64_t unk1);
RelocAddr<_TESRaceLoadForm> TESRaceOriginalLoadForm(0x00384E20);
RelocAddr<uintptr_t> vtbl_TESRaceLoadForm(0x015BD138); // vtable[6]

bool HookedLoadForm(TESRace * race, int64_t unk1)
{
#ifdef _DEBUG
	_DMESSAGE("HookedLoadForm(0x%016" PRIXPTR ", %d) called", (uintptr_t) race, unk1);
#endif
	bool result = TESRaceOriginalLoadForm(race, unk1);

	if (result)
	{
		_MESSAGE("successfully loaded race with formid %08X and name %s", race->formID, race->fullName.GetName());
	}

	if (!race->attackData.unk08.ptr)
	{
		_MESSAGE("race has no attack data, returning");
		return result;
	}

	BGSAttackDataMap * adm = (BGSAttackDataMap *)race->attackData.unk08.ptr;

#ifdef _DEBUG
	PrintADMDebugInfo(adm);
#endif

	std::string keys[3];

	keys[0] = race->behaviorGraph[0].GetModelName();
	if (race->editorId)
	{
#ifdef _DEBUG
		_DMESSAGE("race has editor id %s", race->editorId);
#endif
		keys[1] = race->editorId;
	}
	keys[2] = ToString(race->formID);

	for (std::string &key : keys)
	{
		if (key.empty())
			continue;

		ToLower(key);

		auto it = s_setting.find(key);
		if (it != s_setting.end())
		{
			InsertAttackDataRecords(race, it->second);
		}
	}
	
#ifdef _DEBUG
	PrintADMDebugInfo(adm);
#endif

	return result;
}



extern "C" {
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\InsertAttackData64.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_Error);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		_MESSAGE("InsertAttackData64 Plugin");

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "InsertAttackData64 plugin";
		info->version = 1;

		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");
			return false;
		}
		else if (skse->runtimeVersion != RUNTIME_VERSION_1_5_39)
		{
			_MESSAGE("unsupported runtime version %08X", skse->runtimeVersion);
			return false;
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse) {
		_MESSAGE("Hooking TESRace::LoadForm");
		SafeWrite64(vtbl_TESRaceLoadForm.GetUIntPtr(), uintptr_t(HookedLoadForm));
		_MESSAGE("Patched");

		_MESSAGE("Reading attack data json");
		InitAttackDataJson();
		_MESSAGE("Done");
		return true;
	}
};


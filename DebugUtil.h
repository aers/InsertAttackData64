#pragma once

#include "../skse64/PluginAPI.h"

#include <iomanip>
#include <sstream>


void PrintMemory(byte * memory, size_t len)
{
	std::stringstream buf;
	
	buf << std::hex << std::setfill('0');

	for (size_t i = 0; i < len; i++, memory++)
	{
		if (i != 0 && i % 16 == 0) {
			_DMESSAGE("%04X | %s", i - 16, buf.str().c_str());
			buf.str(std::string());
		}

		buf << std::setw(2) << static_cast<unsigned>(*memory) << " ";
	}

	_DMESSAGE("%04X | %s", len - 16 + (len % 16), buf.str().c_str());
}

void PrintADMDebugInfo(BGSAttackDataMap * adm)
{
	_DMESSAGE("race attack data size %d", adm->map.size());
	_DMESSAGE("race adm object");


	PrintMemory((byte *)adm, 0x48);


	_DMESSAGE("m_size=%d, free_count=%d, offset=%d, entries_ptr=0x%016" PRIXPTR, adm->map.m_size, adm->map.m_freeCount, adm->map.m_freeOffset, (uintptr_t)adm->map.m_entries);
	_DMESSAGE("adm entries table");


	PrintMemory((byte *)adm->map.m_entries, 24 * adm->map.m_size);


	for (size_t i = 0; i < adm->map.m_size; i++)
	{
		auto entry = adm->map.m_entries[i];

		if (entry.next == nullptr)
		{
			_DMESSAGE("entry %d empty", i);
			continue;
		}

		auto hash = adm->map.get_hash(entry.key);

		uint32_t entry_hash = hash & (adm->map.m_size - 1);

		_DMESSAGE("entry %d (ptr 0x%016" PRIXPTR ") hash %08X and entry from hash %d", i, adm->map.m_entries + i, hash, entry_hash);

		if (entry_hash != i)
		{
			_DMESSAGE("entry stored outside its own bucket, continuing");
			continue;
		}

		_DMESSAGE("entry key %s entry attack data pointer 0x%016" PRIXPTR " next ptr 0x%016" PRIXPTR, entry.key.c_str(), entry.value, entry.next);

		auto next_entry = entry.next;

		while (next_entry != adm->map.m_eolPtr)
		{
			_DMESSAGE("entry key %s entry attack data pointer 0x%016" PRIXPTR " next ptr 0x%016" PRIXPTR, next_entry->key.c_str(), next_entry->value, next_entry->next);
			next_entry = next_entry->next;
		}

	}
}
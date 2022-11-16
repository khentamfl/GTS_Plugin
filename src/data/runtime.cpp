#include "data/runtime.hpp"
#include "Config.hpp"
#include <articuno/archives/ryml/ryml.h>
#include <articuno/types/auto.h>

using namespace articuno;
using namespace articuno::ryml;
using namespace SKSE;
using namespace RE;


namespace {
	template <class T>
	T* find_form(std::string_view lookup_id) {
		// From https://github.com/Exit-9B/MCM-Helper/blob/a39b292909923a75dbe79dc02eeda161763b312e/src/FormUtil.cpp
		std::string lookup_id_str(lookup_id);
		std::istringstream ss{ lookup_id_str };
		std::string plugin, id;

		std::getline(ss, plugin, '|');
		std::getline(ss, id);
		RE::FormID relativeID;
		std::istringstream{ id } >> std::hex >> relativeID;
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		return dataHandler ? dataHandler->LookupForm<T>(relativeID, plugin) : nullptr;
	}

	struct RuntimeConfig {
		std::unordered_map<std::string, std::string> sounds;
		std::unordered_map<std::string, std::string> spellEffects;
		std::unordered_map<std::string, std::string> spells;
		std::unordered_map<std::string, std::string> perks;
		std::unordered_map<std::string, std::string> explosions;
		std::unordered_map<std::string, std::string> globals;
		std::unordered_map<std::string, std::string> quests;
		std::unordered_map<std::string, std::string> factions;

		articuno_serde(ar) {
			ar <=> kv(sounds, "sounds");
			ar <=> kv(spellEffects, "spellEffects");
			ar <=> kv(spells, "spells");
			ar <=> kv(perks, "perks");
			ar <=> kv(explosions, "explosions");
			ar <=> kv(globals, "globals");
			ar <=> kv(quests, "quests");
			ar <=> kv(factions, "factions");
		}
	}
}

namespace Gts {
	Runtime& Runtime::GetSingleton() noexcept {
		static Runtime instance;
		return instance;
	}

	std::string Runtime::DebugName() {
		return "Runtime";
	}


	// Sound
	BSISoundDescriptor* Runtime::GetSound(std::string_view tag) {
		BSISoundDescriptor* data = nullptr;
		try {
			data = Runtime::GetSingleton().sounds.at(tag);
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
		}
		return data;
	}
	void Runtime::PlaySound(std::string_view tag, Actor* actor, float volume, float frequency) {
		auto soundDescriptor = Runtime::GetSound(tag);
		if (!soundDescriptor) {
			log::error("Sound invalid");
			return;
		}
		auto audioManager = BSAudioManager::GetSingleton();
		if (!audioManager) {
			log::error("Audio Manager invalid");
			return;
		}
		BSSoundHandle soundHandle;
		bool success = audioManager->BuildSoundDataFromDescriptor(soundHandle, soundDescriptor);
		if (success) {
			//soundHandle.SetFrequency(Frequency);
			soundHandle.SetVolume(Volume);
			NiAVObject* follow = nullptr;
			if (Receiver) {
				NiAVObject* current_3d = Receiver->GetCurrent3D();
				if (current_3d) {
					follow = current_3d;
				}
			}
			soundHandle.SetObjectToFollow(follow);
			soundHandle.Play();
		} else {
			log::error("Could not build sound");
		}
	}

	// Spell Effects
	EffectSetting* Runtime::GetMagicEffect(std::string_view tag) {
		EffectSetting* data = nullptr;
		try {
			data = Runtime::GetSingleton().spellEffects.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			log::warn("MagicEffect: {} not found", tag);
			data = nullptr;
		}
		return data;
	}

	bool Runtime::HasMagicEffect(Actor* actor, std::string_view tag) {
		return Runtime::HasMagicEffectOr(actor, tag, false);
	}

	bool Runtime::HasMagicEffectOr(Actor* actor, std::string_view tag, bool default) {
		auto data = Runtime::GetMagicEffect(tag);
		if (data) {
			return actor->HasMagicEffect(data);
		} else {
			return default;
		}
	}

	// Spells
	SpellItem* Runtime::GetSpell(std::string_view tag) {
		SpellItem* data = nullptr;
		try {
			data = Runtime::GetSingleton().spells.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			log::warn("Spell: {} not found", tag);
			data = nullptr;
		}
		return data;
	}

	bool Runtime::AddSpell(Actor* actor, std::string_view tag) {
		auto data = Runtime::GetSpell(tag);
		if (data) {
			if (!Runtime::HasSpell(data)) {
				actor->AddSpell(data);
			}
		}
	}
	bool Runtime::RemoveSpell(Actor* actor, std::string_view tag) {
		auto data = Runtime::GetSpell(tag);
		if (data) {
			if (Runtime::HasSpell(data)) {
				actor->RemoveSpell(data);
			}
		}
	}

	bool Runtime::HasSpell(Actor* actor, std::string_view tag) {
		return Runtime::HasSpellOr(actor, tag, false);
	}

	bool Runtime::HasSpellOr(Actor* actor, std::string_view tag, bool default) {
		auto data = Runtime::GetSpell(tag);
		if (data) {
			return actor->HasSpell(data);
		} else {
			return default;
		}
	}

	bool Runtime::CastSpell(Actor* caster, Actor* target, std::string_view tag) {
		auto data = GetSpell(tag);
		if (data) {
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(data, false, target, 1.00f, false, 0.0f, caster);
		}
	}

	// Perks
	BGSPerk* Runtime::GetPerk(std::string_view tag) {
		BGSPerk* data = nullptr;
		try {
			data = Runtime::GetSingleton().perks.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			log::warn("Perk: {} not found", tag);
		}
		return data;
	}

	bool Runtime::AddPerk(Actor* actor, std::string_view tag) {
		auto data = Runtime::GetPerk(tag);
		if (data) {
			if (!Runtime::HasSpell(data)) {
				actor->AddPerk(data);
			}
		}
	}
	bool Runtime::RemovePerk(Actor* actor, std::string_view tag) {
		auto data = Runtime::GetPerk(tag);
		if (data) {
			if (Runtime::HasPerk(data)) {
				actor->RemovePerk(data);
			}
		}
	}

	bool Runtime::HasPerk(Actor* actor, std::string_view tag) {
		return Runtime::HasPerkOr(actor, tag, false);
	}

	bool Runtime::HasPerkOr(Actor* actor, std::string_view tag, bool default) {
		auto data = Runtime::GetPerk(tag);
		if (data) {
			return actor->HasPerk(data);
		} else {
			return default;
		}
	}

	// Explosion
	BGSExplosion* Runtime::GetExplosion(std::string_view tag) {
		BGSExplosion* data = nullptr;
		try {
			data = Runtime::GetSingleton().explosions.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			log::warn("Explosion: {} not found", tag);
		}
		return data;
	}

	void Runtime::CreateExplosion(Actor* actor, float scale, std::string_view tag) {
		if (actor) {
			CreateExplosionAtPos(actor, actor->GetPosition(), scale, tag);
		}
	}

	void Runtime::CreateExplosionAtNode(Actor* actor, std::string_view node, float scale, std::string_view tag) {
		if (actor) {
			if (actor->Is3DLoaded()) {
				auto model = actor->GetCurrent3D();
				if (model) {
					std::string nodeName = node;
					auto node = model->GetObjectByName(nodeName);
					if (node) {
						CreateExplosionAtPos(actor, node->world.translate, scale, tag);
					}
				}
			}
		}
	}

	void Runtime::CreateExplosionAtPos(Actor* actor, NiPoint3 pos, float scale, std::string_view tag) {
		auto data = GetExplosion(tag);
		if (data) {
			NiPointer<TESObjectREFR> instance_ptr = target->PlaceObjectAtMe(data, false);
			if (!instance_ptr) {
				return;
			}
			TESObjectREFR* instance = instance_ptr.get();
			if (!instance) {
				return;
			}
			instance->SetPosition(pos);
			Explosion* explosion = instance->AsExplosion();
			explosion->radius *= scale;
			explosion->imodRadius *= scale;
		}
	}

	// Globals
	TESGlobal* Runtime::GetGlobal(std::string_view tag) {
		TESGlobal* data = nullptr;
		try {
			data = Runtime::GetSingleton().globals.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			log::warn("Global: {} not found", tag);
		}
		return data;
	}

	bool Runtime::GetBool(std::string_view tag) {
		return Runtime::GetBoolOr(tag, false);
	}

	bool Runtime::GetBoolOr(std::string_view tag, bool default) {
		auto data = GetGlobal(tag);
		if (data) {
			return fabs(data->value - 0.0) > 1e-4;
		} else {
			return default;
		}
	}

	void Runtime::SetBool(std::string_view tag, bool value) {
		auto data = GetGlobal(tag);
		if (data) {
			if (value) {
				data->value = 1.0;
			} else {
				data->value = 0.0;
			}
		}
	}

	int Runtime::GetInt(std::string_view tag) {
		return Runtime::GetIntOr(tag, false);
	}

	int Runtime::GetIntOr(std::string_view tag, int default) {
		auto data = GetGlobal(tag);
		if (data) {
			return data->value;
		} else {
			return default;
		}
	}

	void Runtime::SetInt(std::string_view tag, int value) {
		auto data = GetGlobal(tag);
		if (data) {
			data->value = value;
		}
	}

	int Runtime::GetFloat(std::string_view tag) {
		return Runtime::GetFloatOr(tag, false);
	}

	int Runtime::GetFloatOr(std::string_view tag, float default) {
		auto data = GetGlobal(tag);
		if (data) {
			return data->value;
		} else {
			return default;
		}
	}

	void Runtime::SetFloat(std::string_view tag, float value) {
		auto data = GetGlobal(tag);
		if (data) {
			data->value = value;
		}
	}

	// Quests
	TESQuest* Runtime::GetQuest(std::string_view tag) {
		TESQuest* data = nullptr;
		try {
			data = Runtime::GetSingleton().quests.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			log::warn("Quest: {} not found", tag);
		}
		return data;
	}

	std::uint16_t Runtime::GetStage(std::string_view tag) {
		return Runtime::GetStageOr(tag, 0);
	}

	std::uint16_t Runtime::GetStageOr(std::string_view tag, std::uint16_t default) {
		auto data = GetQuest(tag);
		if (data) {
			return data->GetCurrentStageID();
		} else {
			return default;
		}
	}

	// Factions
	TESFaction* Runtime::GetFaction(std::string_view tag) {
		TESFaction* data = nullptr;
		try {
			data = Runtime::GetSingleton().factions.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
		}
		return data;
	}


	bool Runtime::InFaction(Actor* actor, std::string_view tag) {
		return Runtime::InFactionOr(actor, tag, false);
	}

	bool Runtime::InFactionOr(Actor* actor, std::string_view tag, bool default) {
		auto data = GetQuest(tag);
		if (data) {
			return actor->IsInFaction(data);
		} else {
			return default;
		}
	}

	// Impacts
	BGSImpactDataSet* GetImpactEffect(std::string_view tag) {
		BGSImpactDataSet* data = nullptr;
		try {
			data = Runtime::GetSingleton().impacts.at(tag).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			log::warn("ImpactEffect: {} not found", tag);
		}
		return data;
	}
	void Runtime::PlayImpactEffect(Actor* actor, std::string_view tag, std::string_view node, NiPoint3& direction, float length, bool applyRotation, bool useLocalRotation) {
		auto data = GetImpactEffect(tag);
		if (data) {
			auto impact = BGSImpactManager::GetSingleton();
			impact->PlayImpactEffect(actor, data, node, direction, length, applyRotation, useLocalRotation);
		}
	}

	// Team Functions
	bool Runtime::HasMagicEffectTeam(Actor* actor, std::string_view tag) {
		return Runtime::HasMagicEffectTeamOr(actor, tag, false);
	}

	bool Runtime::HasMagicEffectTeamOr(Actor* actor, std::string_view tag, bool default) {
		if (Runtime::HasMagicEffectOr(actor, tag, default)) {
			return true;
		}
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			auto player = PlayerCharacter::GetSingleton();
			return Runtime::HasMagicEffectOr(player, tag, default);
		} else {
			return false;
		}
	}

	bool Runtime::HasSpellTeam(Actor* actor, std::string_view tag) {
		return Runtime::HasMagicEffectTeamOr(actor, tag, false);
	}

	bool Runtime::HasSpellTeamOr(Actor* actor, std::string_view tag, bool default) {
		if (Runtime::HasSpellTeamOr(actor, tag, default)) {
			return true;
		}
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			auto player = PlayerCharacter::GetSingleton();
			return Runtime::HasSpellTeamOr(player, tag, default);
		} else {
			return false;
		}
	}

	bool Runtime::HasPerkTeam(Actor* actor, std::string_view tag) {
		return Runtime::HasPerkTeamOr(actor, tag, false);
	}

	bool Runtime::HasPerkTeamOr(Actor* actor, std::string_view tag) {
		if (Runtime::HasPerkOr(actor, tag, default)) {
			return true;
		}
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			auto player = PlayerCharacter::GetSingleton();
			return Runtime::HasPerkOr(player, tag, default);
		} else {
			return false;
		}
	}

	void Runtime::DataReady() {
		RuntimeConfig config;

		std::ifstream in(R"(Data\SKSE\Plugins\GtsRuntime.yaml)");
		yaml_source src(in);
		src >> config;

		for (auto &[key, value]: config.sounds) {
			auto form = find_form<BGSSoundDescriptorForm>(value);
			if (form) {
				this->sounds.try_emplace(key, form);
			} else {
				log::warn("SoundDescriptorform not found for {}", key);
			}
		}

		for (auto &[key, value]: config.spellEffects) {
			auto form = find_form<EffectSetting>(value);
			if (form) {
				this->spellEffects.try_emplace(key, form);
			} else {
				log::warn("EffectSetting form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.spells) {
			auto form = find_form<SpellItem>(value);
			if (form) {
				this->spells.try_emplace(key, form);
			} else {
				log::warn("SpellItem form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.perks) {
			auto form = find_form<BGSPerk>(value);
			if (form) {
				this->perks.try_emplace(key, form);
			} else {
				log::warn("Perk form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.explosions) {
			auto form = find_form<BGSExplosion>(value);
			if (form) {
				this->explosions.try_emplace(key, form);
			} else {
				log::warn("Explosion form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.globals) {
			auto form = find_form<TESGlobal>(value);
			if (form) {
				this->globals.try_emplace(key, form);
			} else {
				log::warn("Global form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.quests) {
			auto form = find_form<TESQuest>(value);
			if (form) {
				this->quests.try_emplace(key, form);
			} else {
				log::warn("Quest form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.factions) {
			auto form = find_form<TESFaction>(value);
			if (form) {
				this->factions.try_emplace(key, form);
			} else {
				log::warn("FactionData form not found for {}", key);
			}
		}

	}
}

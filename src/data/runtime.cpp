#include "data/runtime.hpp"
#include "Config.hpp"
#include <articuno/archives/ryml/ryml.h>
#include <articuno/types/auto.h>
#include "node.hpp"

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
		std::unordered_map<std::string, std::string> impacts;
		std::unordered_map<std::string, std::string> races;

		articuno_serde(ar) {
			ar <=> kv(sounds, "sounds");
			ar <=> kv(spellEffects, "spellEffects");
			ar <=> kv(spells, "spells");
			ar <=> kv(perks, "perks");
			ar <=> kv(explosions, "explosions");
			ar <=> kv(globals, "globals");
			ar <=> kv(quests, "quests");
			ar <=> kv(factions, "factions");
			ar <=> kv(impacts, "impacts");
			ar <=> kv(races, "races");
		}
	};
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
	BSISoundDescriptor* Runtime::GetSound(const std::string_view& tag) {
		BSISoundDescriptor* data = nullptr;
		try {
			data = Runtime::GetSingleton().sounds.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			if (!Runtime::Logged("sond", tag)) {
				log::warn("Sound: {} not found", tag);
			}
		}
		return data;
	}
	void Runtime::PlaySound(const std::string_view& tag, Actor* actor, const float& volume, const float& frequency) {
		auto soundDescriptor = Runtime::GetSound(tag);
		if (!soundDescriptor) {
			log::error("Sound invalid: {}", tag);
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
			//soundHandle.SetFrequency(frequency);
			soundHandle.SetVolume(volume);
			NiAVObject* follow = nullptr;
			if (actor) {
				NiAVObject* current_3d = actor->GetCurrent3D();
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
	void Runtime::PlaySoundAtNode(const std::string_view& tag, Actor* actor, const float& volume, const float& frequency, const std::string_view& node) {
		Runtime::PlaySoundAtNode(tag, actor, volume, frequency, find_node(actor, node));
	}
	void Runtime::PlaySoundAtNode(const std::string_view& tag, Actor* actor, const float& volume, const float& frequency, NiAVObject* node) {
		if (node) {
			Runtime::PlaySoundAtNode(tag, actor, volume, frequency, *node);
		}
	}
	void Runtime::PlaySoundAtNode(const std::string_view& tag, Actor* actor, const float& volume, const float& frequency, NiAVObject& node) {
		auto soundDescriptor = Runtime::GetSound(tag);
		if (!soundDescriptor) {
			log::error("Sound invalid: {}", tag);
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
			//soundHandle.SetFrequency(frequency);
			soundHandle.SetVolume(volume);
			soundHandle.SetObjectToFollow(&node);
			soundHandle.Play();
		} else {
			log::error("Could not build sound");
		}
	}

	// Spell Effects
	EffectSetting* Runtime::GetMagicEffect(const std::string_view& tag) {
		EffectSetting* data = nullptr;
		try {
			data = Runtime::GetSingleton().spellEffects.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			if (!Runtime::Logged("mgef", tag)) {
				log::warn("MagicEffect: {} not found", tag);
			}
			data = nullptr;
		}
		return data;
	}

	bool Runtime::HasMagicEffect(Actor* actor, const std::string_view& tag) {
		return Runtime::HasMagicEffectOr(actor, tag, false);
	}

	bool Runtime::HasMagicEffectOr(Actor* actor, const std::string_view& tag, const bool& default_value) {
		auto data = Runtime::GetMagicEffect(tag);
		if (data) {
			return actor->AsMagicTarget()->HasMagicEffect(data);
		} else {
			return default_value;
		}
	}

	// Spells
	SpellItem* Runtime::GetSpell(const std::string_view& tag) {
		SpellItem* data = nullptr;
		try {
			data = Runtime::GetSingleton().spells.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			if (!Runtime::Logged("spel", tag)) {
				log::warn("Spell: {} not found", tag);
			}
			data = nullptr;
		}
		return data;
	}

	void Runtime::AddSpell(Actor* actor, const std::string_view& tag) {
		auto data = Runtime::GetSpell(tag);
		if (data) {
			if (!Runtime::HasSpell(actor, tag)) {
				actor->AddSpell(data);
			}
		}
	}
	void Runtime::RemoveSpell(Actor* actor, const std::string_view& tag) {
		auto data = Runtime::GetSpell(tag);
		if (data) {
			if (Runtime::HasSpell(actor, tag)) {
				actor->RemoveSpell(data);
			}
		}
	}

	bool Runtime::HasSpell(Actor* actor, const std::string_view& tag) {
		return Runtime::HasSpellOr(actor, tag, false);
	}

	bool Runtime::HasSpellOr(Actor* actor, const std::string_view& tag, const bool& default_value) {
		auto data = Runtime::GetSpell(tag);
		if (data) {
			return actor->HasSpell(data);
		} else {
			return default_value;
		}
	}

	void Runtime::CastSpell(Actor* caster, Actor* target, const std::string_view& tag) {
		auto data = GetSpell(tag);
		if (data) {
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(data, false, target, 1.00f, false, 0.0f, caster);
		}
	}

	// Perks
	BGSPerk* Runtime::GetPerk(const std::string_view& tag) {
		BGSPerk* data = nullptr;
		try {
			data = Runtime::GetSingleton().perks.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			if (!Runtime::Logged("perk", tag)) {
				log::warn("Perk: {} not found", tag);
			}
		}
		return data;
	}

	void Runtime::AddPerk(Actor* actor, const std::string_view& tag) {
		auto data = Runtime::GetPerk(tag);
		if (data) {
			if (!Runtime::HasPerk(actor, tag)) {
				actor->AddPerk(data);
			}
		}
	}
	void Runtime::RemovePerk(Actor* actor, const std::string_view& tag) {
		auto data = Runtime::GetPerk(tag);
		if (data) {
			if (Runtime::HasPerk(actor, tag)) {
				actor->RemovePerk(data);
			}
		}
	}

	bool Runtime::HasPerk(Actor* actor, const std::string_view& tag) {
		return Runtime::HasPerkOr(actor, tag, false);
	}

	bool Runtime::HasPerkOr(Actor* actor, const std::string_view& tag, const bool& default_value) {
		auto data = Runtime::GetPerk(tag);
		if (data) {
			return actor->HasPerk(data);
		} else {
			return default_value;
		}
	}

	// Explosion
	BGSExplosion* Runtime::GetExplosion(const std::string_view& tag) {
		BGSExplosion* data = nullptr;
		try {
			data = Runtime::GetSingleton().explosions.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			if (!Runtime::Logged("expl", tag)) {
				log::warn("Explosion: {} not found", tag);
			}
		}
		return data;
	}

	void Runtime::CreateExplosion(Actor* actor, const float& scale, const std::string_view& tag) {
		if (actor) {
			CreateExplosionAtPos(actor, actor->GetPosition(), scale, tag);
		}
	}

	void Runtime::CreateExplosionAtNode(Actor* actor, const std::string_view& node_name, const float& scale, const std::string_view& tag) {
		if (actor) {
			if (actor->Is3DLoaded()) {
				auto model = actor->GetCurrent3D();
				if (model) {
					auto node = model->GetObjectByName(std::string(node_name));
					if (node) {
						CreateExplosionAtPos(actor, node->world.translate, scale, tag);
					}
				}
			}
		}
	}

	void Runtime::CreateExplosionAtPos(Actor* actor, NiPoint3 pos, const float& scale, const std::string_view& tag) {
		auto data = GetExplosion(tag);
		if (data) {
			NiPointer<TESObjectREFR> instance_ptr = actor->PlaceObjectAtMe(data, false);
			if (!instance_ptr) {
				return;
			}
			TESObjectREFR* instance = instance_ptr.get();
			if (!instance) {
				return;
			}
			instance->SetPosition(pos);
			Explosion* explosion = instance->AsExplosion();
			explosion->GetExplosionRuntimeData().radius *= scale;
			explosion->GetExplosionRuntimeData().imodRadius *= scale;
		}
	}

	// Globals
	TESGlobal* Runtime::GetGlobal(const std::string_view& tag) {
		TESGlobal* data = nullptr;
		try {
			data = Runtime::GetSingleton().globals.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			if (!Runtime::Logged("glob", tag)) {
				log::warn("Global: {} not found", tag);
			}
		}
		return data;
	}

	bool Runtime::GetBool(const std::string_view& tag) {
		return Runtime::GetBoolOr(tag, false);
	}

	bool Runtime::GetBoolOr(const std::string_view& tag, const bool& default_value) {
		auto data = GetGlobal(tag);
		if (data) {
			return fabs(data->value - 0.0) > 1e-4;
		} else {
			return default_value;
		}
	}

	void Runtime::SetBool(const std::string_view& tag, const bool& value) {
		auto data = GetGlobal(tag);
		if (data) {
			if (value) {
				data->value = 1.0;
			} else {
				data->value = 0.0;
			}
		}
	}

	int Runtime::GetInt(const std::string_view& tag) {
		return Runtime::GetIntOr(tag, false);
	}

	int Runtime::GetIntOr(const std::string_view& tag, const int& default_value) {
		auto data = GetGlobal(tag);
		if (data) {
			return data->value;
		} else {
			return default_value;
		}
	}

	void Runtime::SetInt(const std::string_view& tag, const int& value) {
		auto data = GetGlobal(tag);
		if (data) {
			data->value = value;
		}
	}

	float Runtime::GetFloat(const std::string_view& tag) {
		return Runtime::GetFloatOr(tag, false);
	}

	float Runtime::GetFloatOr(const std::string_view& tag, const float& default_value) {
		auto data = GetGlobal(tag);
		if (data) {
			return data->value;
		} else {
			return default_value;
		}
	}

	void Runtime::SetFloat(const std::string_view& tag, const float& value) {
		auto data = GetGlobal(tag);
		if (data) {
			data->value = value;
		}
	}

	// Quests
	TESQuest* Runtime::GetQuest(const std::string_view& tag) {
		TESQuest* data = nullptr;
		try {
			data = Runtime::GetSingleton().quests.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			if (!Runtime::Logged("qust", tag)) {
				log::warn("Quest: {} not found", tag);
			}
		}
		return data;
	}

	std::uint16_t Runtime::GetStage(const std::string_view& tag) {
		return Runtime::GetStageOr(tag, 0);
	}

	std::uint16_t Runtime::GetStageOr(const std::string_view& tag, const std::uint16_t& default_value) {
		auto data = GetQuest(tag);
		if (data) {
			return data->GetCurrentStageID();
		} else {
			return default_value;
		}
	}

	// Factions
	TESFaction* Runtime::GetFaction(const std::string_view& tag) {
		TESFaction* data = nullptr;
		try {
			data = Runtime::GetSingleton().factions.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
		}
		return data;
	}


	bool Runtime::InFaction(Actor* actor, const std::string_view& tag) {
		return Runtime::InFactionOr(actor, tag, false);
	}

	bool Runtime::InFactionOr(Actor* actor, const std::string_view& tag, const bool& default_value) {
		auto data = GetFaction(tag);
		if (data) {
			return actor->IsInFaction(data);
		} else {
			return default_value;
		}
	}

	// Impacts
	BGSImpactDataSet* Runtime::GetImpactEffect(const std::string_view& tag) {
		BGSImpactDataSet* data = nullptr;
		try {
			data = Runtime::GetSingleton().impacts.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			if (!Runtime::Logged("impc", tag)) {
				log::warn("ImpactEffect: {} not found", tag);
			}
		}
		return data;
	}
	void Runtime::PlayImpactEffect(Actor* actor, const std::string_view& tag, const std::string_view& node, NiPoint3 direction, const float& length, const bool& applyRotation, const bool& useLocalRotation) {
		auto data = GetImpactEffect(tag);
		if (data) {
			auto impact = BGSImpactManager::GetSingleton();
			impact->PlayImpactEffect(actor, data, node, direction, length, applyRotation, useLocalRotation);
		}
	}

	// Races
	TESRace* Runtime::GetRace(const std::string_view& tag) {
		TESRace* data = nullptr;
		try {
			data = Runtime::GetSingleton().races.at(std::string(tag)).data;
		}  catch (const std::out_of_range& oor) {
			data = nullptr;
			if (!Runtime::Logged("impc", tag)) {
				log::warn("ImpactEffect: {} not found", tag);
			}
		}
		return data;
	}
	bool Runtime::IsRace(Actor* actor, const std::string_view& tag) {
		auto data = GetRace(tag);
		if (data) {
			return actor->GetRace() == data;
		} else {
			return false;
		}
	}

	// Team Functions
	bool Runtime::HasMagicEffectTeam(Actor* actor, const std::string_view& tag) {
		return Runtime::HasMagicEffectTeamOr(actor, tag, false);
	}

	bool Runtime::HasMagicEffectTeamOr(Actor* actor, const std::string_view& tag, const bool& default_value) {
		if (Runtime::HasMagicEffectOr(actor, tag, default_value)) {
			return true;
		}
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			auto player = PlayerCharacter::GetSingleton();
			return Runtime::HasMagicEffectOr(player, tag, default_value);
		} else {
			return false;
		}
	}

	bool Runtime::HasSpellTeam(Actor* actor, const std::string_view& tag) {
		return Runtime::HasMagicEffectTeamOr(actor, tag, false);
	}

	bool Runtime::HasSpellTeamOr(Actor* actor, const std::string_view& tag, const bool& default_value) {
		if (Runtime::HasSpellTeam(actor, tag)) {
			return true;
		}
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			auto player = PlayerCharacter::GetSingleton();
			return Runtime::HasSpellTeamOr(player, tag, default_value);
		} else {
			return default_value;
		}
	}

	bool Runtime::HasPerkTeam(Actor* actor, const std::string_view& tag) {
		return Runtime::HasPerkTeamOr(actor, tag, false);
	}

	bool Runtime::HasPerkTeamOr(Actor* actor, const std::string_view& tag, const bool& default_value) {
		if (Runtime::HasPerk(actor, tag)) {
			return true;
		}
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			auto player = PlayerCharacter::GetSingleton();
			return Runtime::HasPerkOr(player, tag, default_value);
		} else {
			return default_value;
		}
	}

	bool Runtime::Logged(const std::string_view& catagory, const std::string_view& key) {
		auto& m = Runtime::GetSingleton().logged;
		std::string logKey = std::format("{}::{}", catagory, key);
		bool shouldLog = !(m.find(logKey) == m.end());
		m.emplace(logKey);
		return shouldLog;
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
			} else if (!Runtime::Logged("sond", key)) {
				log::warn("SoundDescriptorform not found for {}", key);
			}
		}

		for (auto &[key, value]: config.spellEffects) {
			auto form = find_form<EffectSetting>(value);
			if (form) {
				this->spellEffects.try_emplace(key, form);
			} else if (!Runtime::Logged("mgef", key)) {
				log::warn("EffectSetting form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.spells) {
			auto form = find_form<SpellItem>(value);
			if (form) {
				this->spells.try_emplace(key, form);
			} else if (!Runtime::Logged("spel", key)) {
				log::warn("SpellItem form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.perks) {
			auto form = find_form<BGSPerk>(value);
			if (form) {
				this->perks.try_emplace(key, form);
			} else if (!Runtime::Logged("perk", key)) {
				log::warn("Perk form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.explosions) {
			auto form = find_form<BGSExplosion>(value);
			if (form) {
				this->explosions.try_emplace(key, form);
			} else if (!Runtime::Logged("expl", key)) {
				log::warn("Explosion form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.globals) {
			auto form = find_form<TESGlobal>(value);
			if (form) {
				this->globals.try_emplace(key, form);
			} else if (!Runtime::Logged("glob", key)) {
				log::warn("Global form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.quests) {
			auto form = find_form<TESQuest>(value);
			if (form) {
				this->quests.try_emplace(key, form);
			} else if (!Runtime::Logged("qust", key)) {
				log::warn("Quest form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.factions) {
			auto form = find_form<TESFaction>(value);
			if (form) {
				this->factions.try_emplace(key, form);
			} else if (!Runtime::Logged("facn", key)) {
				log::warn("FactionData form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.impacts) {
			auto form = find_form<BGSImpactDataSet>(value);
			if (form) {
				this->impacts.try_emplace(key, form);
			} else if (!Runtime::Logged("impc", key)) {
				log::warn("ImpactData form not found for {}", key);
			}
		}

		for (auto &[key, value]: config.races) {
			auto form = find_form<TESRace>(value);
			if (form) {
				this->races.try_emplace(key, form);
			} else if (!Runtime::Logged("race", key)) {
				log::warn("RaceData form not found for {}", key);
			}
		}

	}
}

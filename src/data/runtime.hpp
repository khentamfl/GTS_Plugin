#pragma once
// Module that holds data that is loaded at runtime
// This includes various forms
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct SoundData {
		BGSSoundDescriptorForm* data;
	};

	struct SpellEffectData {
		EffectSetting* data;
	};

	struct SpellData {
		SpellItem* data;
	};

	struct PerkData {
		BGSPerk* data;
	};

	struct ExplosionData {
		BGSExplosion* data;
	};

	struct GlobalData {
		TESGlobal* data;
	};

	struct QuestData {
		TESQuest* data;
	};

	struct FactionData {
		TESFaction* data;
	};

	struct ImpactData {
		BGSImpactDataSet* data;
	};

	class Runtime : public EventListener {
		public:
			[[nodiscard]] static Runtime& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void DataReady() override;

			static BSISoundDescriptor* GetSound(std::string_view tag);
			static void PlaySound(std::string_view tag, Actor* actor, float volume, float frequency);
			// Spell Effects
			static EffectSetting* GetMagicEffect(std::string_view tag);
			static bool HasMagicEffect(Actor* actor, std::string_view tag);
			static bool HasMagicEffectOr(Actor* actor, std::string_view tag, bool default_value);
			// Spells
			static SpellItem* GetSpell(std::string_view tag);
			static bool AddSpell(Actor* actor, std::string_view tag);
			static bool RemoveSpell(Actor* actor, std::string_view tag);
			static bool HasSpell(Actor* actor, std::string_view tag);
			static bool HasSpellOr(Actor* actor, std::string_view tag, bool default_value);
			static bool CastSpell(Actor* caster, Actor* target, std::string_view tag);
			// Perks
			static BGSPerk* GetPerk(std::string_view tag);
			static bool AddPerk(Actor* actor, std::string_view tag);
			static bool RemovePerk(Actor* actor, std::string_view tag);
			static bool HasPerk(Actor* actor, std::string_view tag);
			static bool HasPerkOr(Actor* actor, std::string_view tag, bool default_value);
			// Explosion
			static BGSExplosion* GetExplosion(std::string_view tag);
			static void CreateExplosion(Actor* actor, float scale, std::string_view tag);
			static void CreateExplosionAtNode(Actor* actor, std::string_view node, float scale, std::string_view tag);
			static void CreateExplosionAtPos(Actor* actor, NiPoint3 pos, float scale, std::string_view tag);
			// Globals
			static TESGlobal* GetGlobal(std::string_view tag);
			static bool GetBool(std::string_view tag);
			static bool GetBoolOr(std::string_view tag, bool default_value);
			static void SetBool(std::string_view tag, bool value);
			static int GetInt(std::string_view tag);
			static int GetIntOr(std::string_view tag, int default_value);
			static void SetInt(std::string_view tag, int value);
			static int GetFloat(std::string_view tag);
			static int GetFloatOr(std::string_view tag, float default_value);
			static void SetFloat(std::string_view tag, float value);
			// Quests
			static TESQuest* GetQuest(std::string_view tag);
			static std::uint16_t GetStage(std::string_view tag);
			static std::uint16_t GetStageOr(std::string_view tag, std::uint16_t default_value);
			static void GetStage(std::string_view tag, std::uint16_t value);
			// Factions
			static TESFaction* GetFaction(std::string_view tag);
			static bool InFaction(Actor* actor, std::string_view tag);
			static bool InFactionOr(Actor* actor, std::string_view tag, bool default_value);
			// Impacts
			static BGSImpactDataSet* GetImpactEffect(std::string_view tag);
			static PlayImpactEffect(Actor* actor, std::string_view tag, std::string_view node, NiPoint3& direction, float length, bool applyRotation, bool useLocalRotation);

			// Team Functions
			static bool HasMagicEffectTeam(Actor* actor, std::string_view tag);
			static bool HasMagicEffectTeamOr(Actor* actor, std::string_view tag, bool default_value);
			static bool HasSpellTeam(Actor* actor, std::string_view tag);
			static bool HasSpellTeamOr(Actor* actor, std::string_view tag, bool default_value);
			static bool HasPerkTeam(Actor* actor, std::string_view tag);
			static bool HasPerkTeamOr(Actor* actor, std::string_view tag, bool default_value);


			std::unordered_map<std::string, SoundData> sounds;
			std::unordered_map<std::string, SpellEffectData> spellEffects;
			std::unordered_map<std::string, SpellData> spells;
			std::unordered_map<std::string, PerkData> perks;
			std::unordered_map<std::string, ExplosionData> explosions;
			std::unordered_map<std::string, GlobalData> globals;
			std::unordered_map<std::string, QuestData> quests;
			std::unordered_map<std::string, FactionData> factions;
			std::unordered_map<std::string, ImpactData> impacts;
	};
}

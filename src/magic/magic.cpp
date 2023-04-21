#include "magic/magic.hpp"
#include "Config.hpp"
#include "magic/effects/common.hpp"
#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/explosive_growth.hpp"
#include "magic/effects/EnchGigantism.hpp"
#include "magic/effects/grow_button.hpp"
#include "magic/effects/grow_other.hpp"
#include "magic/effects/tracksize.hpp"
#include "magic/effects/grow_other_button.hpp"
#include "magic/effects/growth.hpp"
#include "magic/effects/CrushGrowth.hpp"
#include "magic/effects/shrink.hpp"
#include "magic/effects/shrink_back.hpp"
#include "magic/effects/shrink_back_other.hpp"
#include "magic/effects/shrink_button.hpp"
#include "magic/effects/shrink_foe.hpp"
#include "magic/effects/shrink_other.hpp"
#include "magic/effects/shrink_other_button.hpp"
#include "magic/effects/slow_grow.hpp"
#include "magic/effects/sword_of_size.hpp"
#include "magic/effects/vore_growth.hpp"
#include "magic/effects/SizeRelatedDamage.hpp"
#include "magic/effects/SmallMassiveThreat.hpp"
#include "magic/effects/Potions/GrowthPotion.hpp"
#include "magic/effects/Potions/SizePotion.hpp"
#include "magic/effects/Potions/SizeStealPotion.hpp"
#include "magic/effects/Poisons/Poison_Of_Shrinking.hpp"
#include "magic/effects/Potions/ShrinkPotion.hpp"
#include "managers/Attributes.hpp"
#include "data/runtime.hpp"

namespace Gts {

	void Magic::OnStart() {
	}
	void Magic::OnUpdate() {
	}
	void Magic::OnFinish() {
	}

	std::string Magic::GetName() {
		return "";
	}


	Magic::Magic(ActiveEffect* effect) : activeEffect(effect) {
		if (this->activeEffect) {
			auto spell = this->activeEffect->spell;
			this->effectSetting = this->activeEffect->GetBaseObject();
			MagicTarget* m_target = this->activeEffect->target;
			if (m_target) {
				if (m_target->MagicTargetIsActor()) {
					this->target = skyrim_cast<Actor*>(m_target);
				}
			}
			if (this->activeEffect->caster) {
				this->caster = this->activeEffect->caster.get().get();
			}
			this->hasDuration = this->HasDuration();
		}
	}

	bool Magic::HasDuration()  {
		if (this->activeEffect) {
			auto spell = this->activeEffect->spell;
			if (spell) {
				// switch (this->activeEffect->spell->GetSpellType()) {
				// 	case  MagicSystem::SpellType::kEnchantment: {
				// 		return false;
				// 	}
				// }
				switch (spell->GetCastingType()) {
					case  MagicSystem::CastingType::kConstantEffect: {
						return false;
					}
				}
			}
		}
		auto effectSetting = this->effectSetting;
		if (effectSetting) {
			if (effectSetting->data.flags.all(EffectSetting::EffectSettingData::Flag::kNoDuration)) {
				return false;
			}
		}
		return true;
	}

	void Magic::poll() {
		switch (this->state) {
			case State::Init:
			{
				this->dual_casted = this->IsDualCasting();

				this->state = State::Start;
				break;
			}
			case State::Start: {
				this->OnStart();
				this->state = State::Update;
				break;
			}
			case State::Update: {
				if (this->activeEffect->flags & ActiveEffect::Flag::kInactive) {
					break;
				}
				this->OnUpdate();
				if ((this->activeEffect->flags & ActiveEffect::Flag::kDispelled)
				    || (this->hasDuration && (this->activeEffect->elapsedSeconds >= this->activeEffect->duration))) {
					this->state = State::Finish;
				}
				break;
			}
			case State::Finish: {
				this->OnFinish();
				this->state = State::CleanUp;
				break;
			}
			case State::CleanUp: {
				break;
			}
		}
	}

	Actor* Magic::GetTarget() {
		return this->target;
	}
	Actor* Magic::GetCaster() {
		return this->caster;
	}

	ActiveEffect* Magic::GetActiveEffect() {
		return this->activeEffect;
	}

	EffectSetting* Magic::GetBaseEffect() {
		return this->effectSetting;
	}

	void Magic::Dispel() {
		if (this->activeEffect) {
			this->activeEffect->Dispel(false); // Not forced
		}
	}

	bool Magic::IsDualCasting() {
		if (this->caster) {
			auto casting_type = GetActiveEffect()->castingSource;
			if (casting_type == MagicSystem::CastingSource::kLeftHand || casting_type == MagicSystem::CastingSource::kRightHand) {
				auto source = this->caster->GetMagicCaster(casting_type);
				if (source) {
					return source->GetIsDualCasting();
				}
			}
		}
		return false;
	}

	MagicManager& MagicManager::GetSingleton() noexcept {
		static MagicManager instance;
		return instance;
	}

	void MagicManager::ProcessActiveEffects(Actor* actor) {
		auto effect_list = actor->AsMagicTarget()->GetActiveEffectList();
		if (!effect_list) {
			return;
		}
		for (auto effect: (*effect_list)) {
			this->numberOfEffects += 1;
			if (this->active_effects.find(effect) == this->active_effects.end()) {
				EffectSetting* base_spell = effect->GetBaseObject();
				Profilers::Start("MagicRuntime");
				auto factorySearch = this->factories.find(base_spell);
				Profilers::Stop("MagicRuntime");
				if (factorySearch != this->factories.end()) {
					auto &[key, factory] = (*factorySearch);
					auto magic_effect = factory->MakeNew(effect);
					if (magic_effect) {
						this->active_effects.try_emplace(effect, std::move(magic_effect));
					}
				}
			}
		}
	}

	std::string MagicManager::DebugName() {
		return "MagicManager";
	}

	void MagicManager::Update() {
		Profilers::Start("MagicLookup");
		for (auto actor: find_actors()) {
			this->ProcessActiveEffects(actor);
		}
		Profilers::Stop("MagicLookup");

		for (auto i = this->active_effects.begin(); i != this->active_effects.end();) {
			this->numberOfOurEffects += 1;
			auto& magic = (*i);


			Profilers::Start(magic.second->GetName());
			magic.second->poll();
			Profilers::Stop(magic.second->GetName());
			if (magic.second->IsFinished()) {
				i = this->active_effects.erase(i);
			} else {
				++i;
			}
		}
	}

	void MagicManager::Reset() {
		this->active_effects.clear();
	}

	void MagicManager::DataReady() {
		RegisterMagic<Gigantism>("EnchGigantism");
		RegisterMagic<GrowthPotion>("EffectGrowthPotion");
		RegisterMagic<SizePotion>("EffectSizePotionStrong");
		RegisterMagic<SizePotion>("EffectSizePotionNormal");
		RegisterMagic<SizePotion>("EffectSizePotionWeak");
		RegisterMagic<SizePotion>("EffectSizePotionExtreme");
		RegisterMagic<SizeHunger>("EffectSizeHungerPotion");
		RegisterMagic<CrushGrowth>("CrushGrowthMGEF");
		RegisterMagic<TrackSize>("TrackSize");
		RegisterMagic<ShrinkFoe>("ShrinkEnemy");
		RegisterMagic<ShrinkFoe>("ShrinkEnemyAOE");
		RegisterMagic<ShrinkFoe>("ShrinkEnemyAOEMast");
		RegisterMagic<ShrinkFoe>("ShrinkBolt");
		RegisterMagic<ShrinkFoe>("ShrinkStorm");


		RegisterMagic<SwordOfSize>("SwordEnchant");
		RegisterMagic<ShrinkButton>("ShrinkPCButton");
		RegisterMagic<GrowButton>("GrowPcButton");
		RegisterMagic<SlowGrow>("SlowGrowth");
		RegisterMagic<SmallMassiveThreat>("SmallMassiveThreat");
		RegisterMagic<Growth>("GrowthSpell");
		RegisterMagic<Growth>("GrowthSpellAdept");
		RegisterMagic<Growth>("GrowthSpellExpert");
		RegisterMagic<Shrink>("ShrinkSpell");
		RegisterMagic<GrowOther>("GrowAlly");
		RegisterMagic<GrowOther>("GrowAllyAdept");
		RegisterMagic<GrowOther>("GrowAllyExpert");

		RegisterMagic<ShrinkOther>("ShrinkAlly");
		RegisterMagic<ShrinkOther>("ShrinkAllyAdept");
		RegisterMagic<ShrinkOther>("ShrinkAllyExpert");
		RegisterMagic<GrowOtherButton>("GrowAllySizeButton");
		RegisterMagic<ShrinkOtherButton>("ShrinkAllySizeButton");
		RegisterMagic<ShrinkBack>("ShrinkBack");
		RegisterMagic<ShrinkBackOther>("ShrinkBackNPC");
		RegisterMagic<VoreGrowth>("GlobalVoreGrowth");
		RegisterMagic<SizeDamage>("SizeRelatedDamage0");
		RegisterMagic<SizeDamage>("SizeRelatedDamage1");
		RegisterMagic<SizeDamage>("SizeRelatedDamage2");
		RegisterMagic<Absorb>("AbsorbMGEF");
		RegisterMagic<Absorb>("TrueAbsorb");
		RegisterMagic<ExplosiveGrowth>("explosiveGrowth1");
		RegisterMagic<ExplosiveGrowth>("explosiveGrowth2");
		RegisterMagic<ExplosiveGrowth>("explosiveGrowth3");
	}
}

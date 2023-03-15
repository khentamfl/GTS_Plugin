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
#include "magic/effects/GrowthPotion.hpp"
#include "magic/effects/SizePotion.hpp"
#include "magic/effects/sizestealpotion.hpp"
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
				this->runtimeProfiler.Start();
				auto factorySearch = this->factories.find(base_spell);
				this->runtimeProfiler.Stop();
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
		this->lookupProfiler.Start();
		for (auto actor: find_actors()) {
			this->ProcessActiveEffects(actor);
		}
		this->lookupProfiler.Stop();

		for (auto i = this->active_effects.begin(); i != this->active_effects.end();) {
			this->numberOfOurEffects += 1;
			auto& magic = (*i);


			Profiler* profiler = nullptr;
			if (Config::GetSingleton().GetDebug().ShouldProfile()) {
				// This can CTD
				// We should cache the base spell form ID and look up that way instead
				// But this way is faster
				auto base_spell = magic.first->GetBaseObject();

				if (base_spell) {
					try {
						profiler = &this->profilers.at(base_spell);
					}  catch (const std::out_of_range& oor) {
						profiler = nullptr;
					}
				}
			}

			if (profiler) {
				profiler->Start();
			}
			magic.second->poll();
			if (profiler) {
				profiler->Stop();
			}
			if (magic.second->IsFinished()) {
				i = this->active_effects.erase(i);
			} else {
				++i;
			}
		}
		if (Config::GetSingleton().GetDebug().ShouldProfile()) {
			static Timer timer = Timer(5.0);
			if (timer.ShouldRun()) {
				this->PrintReport();
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

	void MagicManager::PrintReport() {
		static std::uint64_t last_report_frame = 0;
		static double last_report_time = 0.0;
		std::uint64_t current_report_frame = Time::WorldTimeElapsed();
		double current_report_time = Time::WorldTimeElapsed();
		double total_time = current_report_time - last_report_time;
		float averageNumberOfEffects = float(this->numberOfEffects) / (float(current_report_frame) - float(last_report_frame));
		float averageNumberOfOurEffects = float(this->numberOfOurEffects) / (float(current_report_frame) - float(last_report_frame));
		std::string report = "Reporting Spell Profilers:";
		report += std::format("\nAverage Number of Spells Per Frame: {:.3f}", averageNumberOfEffects);
		report += std::format("\nAverage Number of Our Spells Per Frame: {:.3f}", averageNumberOfOurEffects);
		report += std::format("\n|{:20}|", "Name");
		report += std::format("{:15s}|", "Seconds");
		report += std::format("{:15s}|", "% OurCode");
		report += std::format("{:15s}|", "s per frame");
		report += std::format("{:15s}|", "% of frame");
		report += "\n------------------------------------------------------------------------------------------------";

		double total = 0.0;
		for (auto &[key, profiler]: this->profilers) {
			total += profiler.Elapsed();
		}
		total += this->lookupProfiler.Elapsed();
		total += this->runtimeProfiler.Elapsed();

		double elapsed = this->lookupProfiler.Elapsed();
		double spf = elapsed / (current_report_frame - last_report_frame);
		double time_percent = elapsed/total_time*100;
		report += std::format("\n {:20}:{:15.3f}|{:14.1f}%|{:15.3f}|{:14.3f}%", "Lookup", elapsed, elapsed*100.0/total, spf, time_percent);

		elapsed = this->runtimeProfiler.Elapsed();
		spf = elapsed / (current_report_frame - last_report_frame);
		time_percent = elapsed/total_time*100;
		report += std::format("\n {:20}:{:15.3f}|{:14.1f}%|{:15.3f}|{:14.3f}%", "Runtime", elapsed, elapsed*100.0/total, spf, time_percent);

		for (auto &[baseSpell, profiler]: this->profilers) {
			elapsed = profiler.Elapsed();
			spf = elapsed / (current_report_frame - last_report_frame);
			time_percent = elapsed/total_time*100;
			report += std::format("\n {:20}:{:15.3f}|{:14.1f}%|{:15.3f}|{:14.3f}%", profiler.GetName(), elapsed, elapsed*100.0/total, spf, time_percent);
			profiler.Reset();
		}
		log::info("{}", report);

		this->numberOfEffects = 0;
		this->numberOfOurEffects = 0;
		this->lookupProfiler.Reset();
		this->runtimeProfiler.Reset();
		last_report_frame = current_report_frame;
		last_report_time = current_report_time;
	}
}

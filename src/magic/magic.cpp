#include "magic/magic.h"
#include "util.h"
#include "magic/effects/common.h"
#include "magic/effects/explosive_growth.h"
#include "magic/effects/grow_button.h"
#include "magic/effects/grow_other.h"
#include "magic/effects/grow_other_button.h"
#include "magic/effects/growth.h"
#include "magic/effects/shrink.h"
#include "magic/effects/shrink_back.h"
#include "magic/effects/shrink_back_other.h"
#include "magic/effects/shrink_button.h"
#include "magic/effects/shrink_foe.h"
#include "magic/effects/shrink_other.h"
#include "magic/effects/shrink_other_button.h"
#include "magic/effects/slow_grow.h"
#include "magic/effects/sword_of_size.h"
#include "magic/effects/vore_growth.h"
#include "data/runtime.h"

namespace Gts {

	void Magic::OnStart() {
	}
	void Magic::OnUpdate() {
	}
	void Magic::OnFinish() {
	}

	void Magic::poll() {
		switch (this->state) {
			case State::Init:
			{
				// Setup target etc
				if (this->activeEffect) {
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
				}
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
				this->OnUpdate();
				bool found = false;
				if (this->target) {
					for (auto effect: (*this->target->GetActiveEffectList())) {
						if (effect == this->activeEffect) {
							found = true;
						}
					}
				}
				if (!found) {
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
		return this->target;
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
			auto source = this->caster->GetMagicCaster(MagicSystem::CastingSource::kLeftHand);
			if (source) {
				return source->GetIsDualCasting();
			}
		}
		return false;
	}

	MagicManager& MagicManager::GetSingleton() noexcept {
		static MagicManager instance;
		return instance;
	}

	void MagicManager::poll() {
		auto actors = find_actors();
		auto& runtime = Runtime::GetSingleton();
		for (auto actor: actors) {
			auto effect_list =actor->GetActiveEffectList();
			if (!effect_list) {
				continue;
			}
			for (auto effect: (*effect_list)) {
				EffectSetting* base_spell = effect->GetBaseObject();
				if (ExplosiveGrowth::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new ExplosiveGrowth(effect));
				}

				if (ShrinkFoe::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new ShrinkFoe(effect));
				}

				if (SwordOfSize::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new SwordOfSize(effect));
				}

				if (ShrinkButton::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new ShrinkButton(effect));
				}
				if (GrowButton::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new GrowButton(effect));
				}

				if (SlowGrow::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new SlowGrow(effect));
				}

				if (Growth::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new Growth(effect));
				}
				if (Shrink::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new Shrink(effect));
				}

				if (GrowOther::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new GrowOther(effect));
				}
				if (ShrinkOther::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new ShrinkOther(effect));
				}

				if (GrowOtherButton::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new GrowOtherButton(effect));
				}
				if (ShrinkOtherButton::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new ShrinkOtherButton(effect));
				}

				if (ShrinkBack::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new ShrinkBack(effect));
				}
				if (ShrinkBackOther::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new ShrinkBackOther(effect));
				}

				if (VoreGrowth::StartEffect(base_spell)) {
					this->active_effects.try_emplace(effect, new VoreGrowth(effect));
				}
			}
		}

		for (auto i = this->active_effects.begin(); i != this->active_effects.end();) {
			auto& magic = (*i);
			magic.second->poll();
			if (magic.second->IsFinished()) {
				i = this->active_effects.erase(i);
			} else {
				++i;
			}
		}
	}
}

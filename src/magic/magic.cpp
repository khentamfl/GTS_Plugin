#include "magic/magic.h"
#include "util.h"
#include "magic/explosive_growth.h"
#include "magic/MagicManager.h"
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
					this->baseEffect = this->activeEffect->GetBaseObject();
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
				this->duel_casted = this->IsDuelCasting();

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
			this->activeEffect->Dispel();
		}
	}

	bool Magic::IsDuelCasting() {
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
			if (!effect_list) continue;
			for (auto effect: (*effect_list)) {
				EffectSetting* base_spell = effect->GetBaseObject();
				ManageGameModePC();

				if (base_spell == runtime.GtsMarkAlly) {
					ManageGameModeNPC(target);
				}
				else if (ExplosiveGrowth::StartEffect(base_spell)) {
					ExplosiveGrowth(caster);
					this->active_effects.push_back(new ExplosiveGrowth(effect));
				}
				if (base_spell == runtime.ShrinkPCButton) {
					ShrinkPCFunction(caster);
				}
				if (base_spell == runtime.GrowPcButton) {
					GrowPCFunction(caster);
				}

				if (base_spell == runtime.ShrinkEnemy) {
					ShrinkFoe(caster, target);
				}
				if (base_spell == runtime.ShrinkEnemyAOE) {
					ShrinkFoeAoe(caster, target);
				}
				if (base_spell == runtime.ShrinkEnemyAOEMast) {
					ShrinkFoeAoeMast(caster, target);
				}

				if (base_spell == runtime.SlowGrowth) {
					SlowGrowthFunction(caster);
				}

				if (base_spell == runtime.GrowthSpell) {
					GrowthSpellFunction(caster);
				}

				if (base_spell == runtime.ShrinkSpell) {
					ShrinkSpellFunction(caster);
				}

				if (base_spell == runtime.GrowAlly) {
					GrowAllyFunction(caster, target);
				}

				if (base_spell == runtime.ShrinkAlly) {
					ShrinkAllyFunction(caster, target);
				}

				if (base_spell == runtime.GrowAllySizeButton) {
					GrowAllyButtonFunction(target);
				}

				if (base_spell == runtime.ShrinkAllySizeButton) {
					ShrinkAllyButtonFunction(target);
				}


				if (base_spell == runtime.ShrinkBack) {
					ShrinkBackFunction(caster);
				}

				if (base_spell == runtime.ShrinkBackNPC) {
					ShrinkBackNPCFunction(target);
				}

				if (base_spell == runtime.GlobalVoreGrowth) {
					VoreGrowthFunction(caster);
				}
			}
		}

		for (auto i = this->active_effects.begin(); i != this->active_effects.end();) {
			auto magic = (*i);
			magic->poll();
			if (magic->IsFinished()) {
				i = this->active_effects.erase(i);
			} else {
				++i;
			}
		}
	}
}

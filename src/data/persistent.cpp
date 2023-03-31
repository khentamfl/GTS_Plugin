#include "managers/GtsSizeManager.hpp"
#include "data/persistent.hpp"
#include "scale/modscale.hpp"


using namespace SKSE;
using namespace RE;

namespace {
	inline const auto ActorDataRecord = _byteswap_ulong('ACTD');
	inline const auto ScaleMethodRecord = _byteswap_ulong('SCMD');
	inline const auto HighHeelCorrectionRecord = _byteswap_ulong('HHCO');
	inline const auto HighHeelFurnitureRecord = _byteswap_ulong('HHFO');
	inline const auto AllowPlayerVoreRecord = _byteswap_ulong('APVR');
	inline const auto IsSpeedAdjustedRecord = _byteswap_ulong('ANAJ');
	inline const auto TremorScales = _byteswap_ulong('TREM');
	inline const auto CamCollisions = _byteswap_ulong('CAMC');

	const float DEFAULT_MAX_SCALE = 65535.0;
	const float DEFAULT_HALF_LIFE = 1.0;
}

namespace Gts {
	Persistent& Persistent::GetSingleton() noexcept {
		static Persistent instance;
		return instance;
	}

	std::string Persistent::DebugName() {
		return "Persistent";
	}

	void Persistent::Reset() {
		std::unique_lock lock(this->_lock);
		this->_actor_data.clear();
	}

	void Persistent::OnRevert(SerializationInterface*) {
		GetSingleton().Reset();
	}

	void Persistent::OnGameLoaded(SerializationInterface* serde) {
		std::uint32_t type;
		std::uint32_t size;
		std::uint32_t version;

		SizeManager::GetSingleton().Reset();

		while (serde->GetNextRecordInfo(type, version, size)) {
			if (type == ActorDataRecord) {
				if (version >= 1) {
					std::size_t count;
					serde->ReadRecordData(&count, sizeof(count));
					for (; count > 0; --count) {
						RE::FormID actorFormID;
						serde->ReadRecordData(&actorFormID, sizeof(actorFormID));
						RE::FormID newActorFormID;
						if (!serde->ResolveFormID(actorFormID, newActorFormID)) {
							log::warn("Actor ID {:X} could not be found after loading the save.", actorFormID);
							continue;
						}
						float native_scale;
						serde->ReadRecordData(&native_scale, sizeof(native_scale));
						if (std::isnan(native_scale)) {
							native_scale = 1.0;
						}

						float visual_scale;
						serde->ReadRecordData(&visual_scale, sizeof(visual_scale));
						if (std::isnan(visual_scale)) {
							visual_scale = 1.0;
						}

						float visual_scale_v;
						serde->ReadRecordData(&visual_scale_v, sizeof(visual_scale_v));
						if (std::isnan(visual_scale_v)) {
							visual_scale_v = 0.0;
						}

						float target_scale;
						serde->ReadRecordData(&target_scale, sizeof(target_scale));
						if (std::isnan(target_scale)) {
							target_scale = 1.0;
						}

						float max_scale;
						serde->ReadRecordData(&max_scale, sizeof(max_scale));
						if (std::isnan(max_scale)) {
							max_scale = DEFAULT_MAX_SCALE;
						}

						float half_life;
						if (version >= 2) {
							serde->ReadRecordData(&half_life, sizeof(half_life));
						} else {
							half_life = DEFAULT_HALF_LIFE;
						}
						if (std::isnan(half_life)) {
							half_life = DEFAULT_HALF_LIFE;
						}

						float anim_speed;
						if (version >= 3) {
							serde->ReadRecordData(&anim_speed, sizeof(anim_speed));
						} else {
							anim_speed = 1.0;
						}
						if (std::isnan(anim_speed)) {
							anim_speed = 1.0;
						}

						float effective_multi;
						if (version >= 4) {
							serde->ReadRecordData(&effective_multi, sizeof(effective_multi));
						} else {
							effective_multi = 1.0;
						}
						if (std::isnan(effective_multi)) {
							effective_multi = 1.0;
						}

						float bonus_hp;
						if (version >= 5) {
							serde->ReadRecordData(&bonus_hp, sizeof(bonus_hp));
						} else {
							bonus_hp = 0.0;
						}
						if (std::isnan(bonus_hp)) {
							bonus_hp = 0.0;
						}

						float bonus_carry;
						if (version >= 5) {
							serde->ReadRecordData(&bonus_carry, sizeof(bonus_carry));
						} else {
							bonus_carry = 0.0;
						}
						if (std::isnan(bonus_carry)) {
							bonus_carry = 0.0;
						}

						float bonus_max_size;
						if (version >= 5) {
							serde->ReadRecordData(&bonus_max_size, sizeof(bonus_max_size));
						} else {
							bonus_max_size = 0.0;
						}
						if (std::isnan(bonus_max_size)) {
							bonus_max_size = 0.0;
						}
						float smt_run_speed;
						if (version >= 6) {
							serde->ReadRecordData(&smt_run_speed, sizeof(smt_run_speed));
						} else {
							smt_run_speed = 0.0;
						}
						if (std::isnan(smt_run_speed)) {
							smt_run_speed = 0.0;
						}

						float NormalDamage; //0
						if (version >= 6) {
							serde->ReadRecordData(&NormalDamage, sizeof(NormalDamage));
						} else {
							NormalDamage = 0.0;
						}
						if (std::isnan(NormalDamage)) {
							NormalDamage = 0.0;
						}

						float SprintDamage; //1
						if (version >= 6) {
							serde->ReadRecordData(&SprintDamage, sizeof(SprintDamage));
						} else {
							SprintDamage = 0.0;
						}
						if (std::isnan(SprintDamage)) {
							SprintDamage = 0.0;
						}

						float FallDamage; //2
						if (version >= 6) {
							serde->ReadRecordData(&FallDamage, sizeof(FallDamage));
						} else {
							FallDamage = 0.0;
						}
						if (std::isnan(FallDamage)) {
							FallDamage = 0.0;
						}

						float HHDamage; //3
						if (version >= 6) {
							serde->ReadRecordData(&HHDamage, sizeof(HHDamage));
						} else {
							HHDamage = 0.0;
						}
						if (std::isnan(HHDamage)) {
							HHDamage = 0.0;
						}

						float SizeVulnerability;
						if (version >= 6) {
							serde->ReadRecordData(&SizeVulnerability, sizeof(SizeVulnerability));
						} else {
							SizeVulnerability = 0.0;
						}
						if (std::isnan(SizeVulnerability)) {
							SizeVulnerability = 0.0;
						}

						float AllowHitGrowth;
						if (version >= 6) {
							serde->ReadRecordData(&AllowHitGrowth, sizeof(AllowHitGrowth));
						} else {
							AllowHitGrowth = 1.0;
						}
						if (std::isnan(AllowHitGrowth)) {
							AllowHitGrowth = 0.0;
						}

						float SizeReserve;
						if (version >= 6) {
							serde->ReadRecordData(&SizeReserve, sizeof(SizeReserve));
						} else {
							SizeReserve = 0.0;
						}
						if (std::isnan(SizeReserve)) {
							SizeReserve = 0.0;
						}


						float target_scale_v;
						if (version >= 7) {
							serde->ReadRecordData(&target_scale_v, sizeof(target_scale_v));
						} else {
							target_scale_v = 0.0;
						}
						if (std::isnan(target_scale_v)) {
							target_scale_v = 0.0;
						}

						float scaleOverride;
						if (version >= 8) {
							serde->ReadRecordData(&scaleOverride, sizeof(scaleOverride));
						} else {
							scaleOverride = -1.0;
						}
						if (std::isnan(scaleOverride)) {
							scaleOverride = -1.0;
						}

						ActorData data = ActorData();
						log::info("Loading Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}, half_life: {}, anim_speed: {}, bonus_hp: {}, bonus_carry: {}", newActorFormID, native_scale, visual_scale, visual_scale_v, target_scale, max_scale, half_life, anim_speed, bonus_hp, bonus_carry);
						data.native_scale = native_scale;
						data.visual_scale = visual_scale;
						data.visual_scale_v = visual_scale_v;
						data.target_scale = target_scale;
						data.max_scale = max_scale;
						data.half_life = half_life;
						data.anim_speed = anim_speed;
						data.effective_multi = effective_multi;
						data.bonus_hp = bonus_hp;
						data.bonus_carry = bonus_carry;
						data.bonus_max_size = bonus_max_size;
						data.smt_run_speed = smt_run_speed;
						data.NormalDamage = NormalDamage;
						data.SprintDamage = SprintDamage;
						data.FallDamage = FallDamage;
						data.HHDamage = HHDamage;
						data.SizeVulnerability = SizeVulnerability;
						data.AllowHitGrowth = AllowHitGrowth;
						data.SizeReserve = SizeReserve;
						data.target_scale_v = target_scale_v;
						data.scaleOverride = scaleOverride;
						TESForm* actor_form = TESForm::LookupByID<Actor>(newActorFormID);
						if (actor_form) {
							Actor* actor = skyrim_cast<Actor*>(actor_form);
							if (actor) {
								GetSingleton()._actor_data.insert_or_assign(newActorFormID, data);
							} else {
								log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
							}
						} else {
							log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
						}
					}
				} else {
					log::info("Disregarding version 0 cosave info.");
				}
			} else if (type == ScaleMethodRecord) {
				int size_method;
				serde->ReadRecordData(&size_method, sizeof(size_method));
				switch (size_method) {
					case 0:
						GetSingleton().size_method = SizeMethod::ModelScale;
						break;
					case 1:
						GetSingleton().size_method = SizeMethod::RootScale;
						break;
					case 2:
						GetSingleton().size_method = SizeMethod::RefScale;
						break;
				}
			} else if (type == HighHeelCorrectionRecord) {
				bool highheel_correction;
				serde->ReadRecordData(&highheel_correction, sizeof(highheel_correction));
				GetSingleton().highheel_correction = highheel_correction;
			} else if (type == HighHeelFurnitureRecord) {
				bool highheel_furniture;
				serde->ReadRecordData(&highheel_furniture, sizeof(highheel_furniture));
				GetSingleton().highheel_furniture = highheel_furniture;
			} else if (type == AllowPlayerVoreRecord) {
				bool vore_allowplayervore;
				serde->ReadRecordData(&vore_allowplayervore, sizeof(vore_allowplayervore));
				GetSingleton().vore_allowplayervore = vore_allowplayervore;
			}
			
			 else if (type == IsSpeedAdjustedRecord) {
				bool is_speed_adjusted;
				serde->ReadRecordData(&is_speed_adjusted, sizeof(is_speed_adjusted));
				GetSingleton().is_speed_adjusted = is_speed_adjusted;
				if (version >= 1) {
					float k;
					serde->ReadRecordData(&k, sizeof(k));
					GetSingleton().speed_adjustment.k = k;
					float n;
					serde->ReadRecordData(&n, sizeof(n));
					GetSingleton().speed_adjustment.n = n;
					float s;
					serde->ReadRecordData(&s, sizeof(s));
					GetSingleton().speed_adjustment.s = s;
					float o = 1.0;
					GetSingleton().speed_adjustment.o = o;
				}
			} else if (type == TremorScales) {
				float tremor_scale;
				serde->ReadRecordData(&tremor_scale, sizeof(tremor_scale));
				GetSingleton().tremor_scale = tremor_scale;
				float npc_tremor_scale;
				serde->ReadRecordData(&npc_tremor_scale, sizeof(npc_tremor_scale));
				GetSingleton().npc_tremor_scale = npc_tremor_scale;
			} else if (type == CamCollisions) {
				bool enable_trees;
				serde->ReadRecordData(&enable_trees, sizeof(enable_trees));
				GetSingleton().camera_collisions.enable_trees = enable_trees;
				bool enable_debris;
				serde->ReadRecordData(&enable_debris, sizeof(enable_debris));
				GetSingleton().camera_collisions.enable_debris = enable_debris;
				bool enable_terrain;
				serde->ReadRecordData(&enable_terrain, sizeof(enable_terrain));
				GetSingleton().camera_collisions.enable_terrain = enable_terrain;
				bool enable_actor;
				serde->ReadRecordData(&enable_actor, sizeof(enable_actor));
				GetSingleton().camera_collisions.enable_actor = enable_actor;
				if (version >= 1) {
					bool enable_static;
					serde->ReadRecordData(&enable_static, sizeof(enable_static));
					GetSingleton().camera_collisions.enable_static = enable_static;
				}
				float above_scale;
				serde->ReadRecordData(&above_scale, sizeof(above_scale));
				GetSingleton().camera_collisions.above_scale = above_scale;
			} else {
				log::warn("Unknown record type in cosave.");
				__assume(false);
			}
		}
	}

	void Persistent::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		if (!serde->OpenRecord(ActorDataRecord, 8)) {
			log::error("Unable to open actor data record to write cosave data.");
			return;
		}

		auto count = GetSingleton()._actor_data.size();
		serde->WriteRecordData(&count, sizeof(count));
		for (auto const& [form_id_t, data] : GetSingleton()._actor_data) {
			FormID form_id = form_id_t;
			float native_scale = data.native_scale;
			float visual_scale = data.visual_scale;
			float visual_scale_v = data.visual_scale_v;
			float target_scale = data.target_scale;
			float max_scale = data.max_scale;
			float half_life = data.half_life;
			float anim_speed = data.anim_speed;
			float effective_multi = data.effective_multi;
			float bonus_hp = data.bonus_hp;
			float bonus_carry = data.bonus_carry;
			float bonus_max_size = data.bonus_max_size;
			float smt_run_speed = data.smt_run_speed;
			float NormalDamage = data.NormalDamage;
			float SprintDamage = data.SprintDamage;
			float FallDamage = data.FallDamage;
			float HHDamage = data.HHDamage;
			float SizeVulnerability = data.SizeVulnerability;
			float AllowHitGrowth = data.AllowHitGrowth;
			float SizeReserve = data.SizeReserve;
			float target_scale_v = data.target_scale_v;
			float scaleOverride = data.scaleOverride;
			log::info("Saving Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}, half_life: {}, anim_speed: {}, effective_multi: {}, effective_multi: {}, bonus_hp: {}, bonus_carry: {}, bonus_max_size: {}", form_id, native_scale, visual_scale, visual_scale_v, target_scale, max_scale, half_life, anim_speed, effective_multi, effective_multi, bonus_hp, bonus_carry, bonus_max_size);
			serde->WriteRecordData(&form_id, sizeof(form_id));
			serde->WriteRecordData(&native_scale, sizeof(native_scale));
			serde->WriteRecordData(&visual_scale, sizeof(visual_scale));
			serde->WriteRecordData(&visual_scale_v, sizeof(visual_scale_v));
			serde->WriteRecordData(&target_scale, sizeof(target_scale));
			serde->WriteRecordData(&max_scale, sizeof(max_scale));
			serde->WriteRecordData(&half_life, sizeof(half_life));
			serde->WriteRecordData(&anim_speed, sizeof(anim_speed));
			serde->WriteRecordData(&effective_multi, sizeof(effective_multi));
			serde->WriteRecordData(&bonus_hp, sizeof(bonus_hp));
			serde->WriteRecordData(&bonus_carry, sizeof(bonus_carry));
			serde->WriteRecordData(&bonus_max_size, sizeof(bonus_max_size));
			serde->WriteRecordData(&smt_run_speed, sizeof(smt_run_speed));

			serde->WriteRecordData(&NormalDamage, sizeof(NormalDamage));
			serde->WriteRecordData(&SprintDamage, sizeof(SprintDamage));
			serde->WriteRecordData(&FallDamage, sizeof(FallDamage));
			serde->WriteRecordData(&HHDamage, sizeof(HHDamage));
			serde->WriteRecordData(&SizeVulnerability, sizeof(SizeVulnerability));
			serde->WriteRecordData(&AllowHitGrowth, sizeof(AllowHitGrowth));
			serde->WriteRecordData(&SizeReserve, sizeof(SizeReserve));

			serde->WriteRecordData(&target_scale_v, sizeof(target_scale_v));
			serde->WriteRecordData(&scaleOverride, sizeof(scaleOverride));
		}

		if (!serde->OpenRecord(ScaleMethodRecord, 0)) {
			log::error("Unable to open scale method record to write cosave data.");
			return;
		}

		int size_method = GetSingleton().size_method;
		serde->WriteRecordData(&size_method, sizeof(size_method));

		if (!serde->OpenRecord(HighHeelCorrectionRecord, 0)) {
			log::error("Unable to open high heel correction record to write cosave data.");
			return;
		}

		bool highheel_correction = GetSingleton().highheel_correction;
		serde->WriteRecordData(&highheel_correction, sizeof(highheel_correction));

		if (!serde->OpenRecord(HighHeelFurnitureRecord, 0)) {
			log::error("Unable to open high heel furniture record to write cosave data.");
			return;
		}

		bool highheel_furniture = GetSingleton().highheel_furniture;
		serde->WriteRecordData(&highheel_furniture, sizeof(highheel_furniture));


		if (!serde->OpenRecord(AllowPlayerVoreRecord, 0)) {
			log::error("Unable to open Allow Player Vore record to write cosave data.");
			return;
		}

		bool vore_allowplayervore = GetSingleton().vore_allowplayervore;
		serde->WriteRecordData(&vore_allowplayervore, sizeof(vore_allowplayervore));

		if (!serde->OpenRecord(IsSpeedAdjustedRecord, 1)) {
			log::error("Unable to open is speed adjusted record to write cosave data.");
			return;
		}

		bool is_speed_adjusted = GetSingleton().is_speed_adjusted;
		serde->WriteRecordData(&is_speed_adjusted, sizeof(is_speed_adjusted));
		float k = GetSingleton().speed_adjustment.k;
		serde->WriteRecordData(&k, sizeof(k));
		float n = GetSingleton().speed_adjustment.n;
		serde->WriteRecordData(&n, sizeof(n));
		float s = GetSingleton().speed_adjustment.s;
		serde->WriteRecordData(&s, sizeof(s));

		if (!serde->OpenRecord(TremorScales, 0)) {
			log::error("Unable to open tremor scale record to write cosave data.");
			return;
		}

		float tremor_scale = GetSingleton().tremor_scale;
		serde->WriteRecordData(&tremor_scale, sizeof(tremor_scale));
		float npc_tremor_scale = GetSingleton().npc_tremor_scale;
		serde->WriteRecordData(&npc_tremor_scale, sizeof(npc_tremor_scale));

		if (!serde->OpenRecord(CamCollisions, 1)) {
			log::error("Unable to open camera collisions record to write cosave data.");
			return;
		}

		bool enable_trees = GetSingleton().camera_collisions.enable_trees;
		serde->WriteRecordData(&enable_trees, sizeof(enable_trees));
		bool enable_debris = GetSingleton().camera_collisions.enable_debris;
		serde->WriteRecordData(&enable_debris, sizeof(enable_debris));
		bool enable_terrain = GetSingleton().camera_collisions.enable_terrain;
		serde->WriteRecordData(&enable_terrain, sizeof(enable_terrain));
		bool enable_actor = GetSingleton().camera_collisions.enable_actor;
		serde->WriteRecordData(&enable_actor, sizeof(enable_actor));
		bool enable_static = GetSingleton().camera_collisions.enable_static;
		serde->WriteRecordData(&enable_static, sizeof(enable_static));
		float above_scale = GetSingleton().camera_collisions.above_scale;
		serde->WriteRecordData(&above_scale, sizeof(above_scale));
	}

	ActorData::ActorData() {
		// Uninit data
		// Make sure it is set elsewhere
	}
	ActorData::ActorData(Actor* actor) {
		// DEFAULT VALUES FOR NEW ACTORS
		auto scale = get_scale(actor);
		this->native_scale = scale;
		this->visual_scale = scale;
		this->visual_scale_v = 0.0;
		this->target_scale = scale;
		this->max_scale = DEFAULT_MAX_SCALE;
		this->half_life = DEFAULT_HALF_LIFE;
		this->anim_speed = 1.0;
		this->bonus_hp = 0.0;
		this->bonus_carry = 0.0;
		this->bonus_max_size = 0.0;
		this->smt_run_speed = 0.0;
		this->NormalDamage = 1.0;
		this->SprintDamage = 1.0;
		this->FallDamage = 1.0;
		this->HHDamage = 1.0;
		this->SizeVulnerability = 0.0;
		this->AllowHitGrowth = 1.0;
		this->SizeReserve = 0.0;
		this->scaleOverride = -1.0;
	}

	ActorData* Persistent::GetActorData(Actor* actor) {
		if (!actor) {
			return nullptr;
		}
		return this->GetActorData(*actor);
	}
	ActorData* Persistent::GetActorData(Actor& actor) {
		std::unique_lock lock(this->_lock);
		auto key = actor.formID;
		ActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			// Add new
			if (!actor.Is3DLoaded()) {
				return nullptr;
			}
			auto scale = get_scale(&actor);
			if (scale < 0.0) {
				return nullptr;
			}
			this->_actor_data.try_emplace(key, &actor);
			result = &this->_actor_data.at(key);
		}
		return result;
	}

	ActorData* Persistent::GetData(TESObjectREFR* refr) {
		if (!refr) {
			return nullptr;
		}
		return this->GetData(*refr);
	}
	ActorData* Persistent::GetData(TESObjectREFR& refr) {
		auto key = refr.formID;
		ActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			return nullptr;
		}
		return result;
	}

	void Persistent::ResetActor(Actor* actor) {
		// Fired after a TESReset event
		//  This event should be when the game attempts to reset their
		//  actor values etc when the cell resets
		auto data = this->GetData(actor);
		if (data) {
			data->visual_scale = data->native_scale;
			data->target_scale = data->native_scale;
			data->max_scale = DEFAULT_MAX_SCALE;
			data->visual_scale_v = 0.0;
			data->half_life = DEFAULT_HALF_LIFE;
			data->anim_speed = 1.0;
			data->bonus_hp = 0.0;
			data->bonus_carry = 0.0;
			data->bonus_max_size = 0.0;
			data->smt_run_speed = 0.0;
			data->NormalDamage = 1.0;
			data->SprintDamage = 1.0;
			data->FallDamage = 1.0;
			data->HHDamage = 1.0;
			data->SizeVulnerability = 0.0;
			data->AllowHitGrowth = 1.0;
			data->SizeReserve = 0.0;
			data->scaleOverride = -1.0;
		}
	}
}

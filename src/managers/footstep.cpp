#include "managers/footstep.h"
#include "scale/scale.h"
#include "managers/modevent.h"
#include "util.h"
#include "node.h"
#include "data/runtime.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	enum Foot {
		Left,
		Right,
		Front,
		Back,
		Unknown,
	};

	BGSImpactDataSet* get_foot_impactdata(Foot foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().FootstepL;
				break;
			case Foot::Right:
			case Foot::Back:
			case Foot::Unknown:
				return Runtime::GetSingleton().FootstepR;
				break;
		}
		return nullptr;
	}
}
namespace Gts {
	FootStepManager& FootStepManager::GetSingleton() noexcept {
		static FootStepManager instance;
		return instance;
	}

	void FootStepManager::HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		if (a_event) {
			auto actor = a_event->actor.get().get();
			if (actor) {
				auto base_actor = actor->GetActorBase();
			}
			std::string tag = a_event->tag.c_str();
			auto event_manager = ModEventManager::GetSingleton();
			event_manager.m_onfootstep.SendEvent(actor,tag);

			if (actor->formID != 0x14) return;

			log::info("{} for {}", tag, actor_name(actor));
			// Foot step time
			float scale = get_scale(actor);
			float k = 4.0;
			float n = 5.6;
			float a = 1.1;
			if (scale >= a) {
				log::info("Actor is big enough");
				float volume = pow(k*(scale-a), n);

				NiAVObject* foot = nullptr;
				Foot foot_kind = Foot::Unknown;
				if (starts_with(tag, "FootLeft")) {
					log::info("Trying to find FootLeft");
					foot = find_node_regex_any(actor, ".*(L.*Foot|L.*Leg.*Tip).*");
					foot_kind = Foot::Left;
				} else if (starts_with(tag, "FootRight")) {
					log::info("Trying to find FootRight");
					foot = find_node_regex_any(actor, ".*(R.*Foot|R.*Leg.*Tip).*");
					foot_kind = Foot::Right;
				} else if (starts_with(tag, "FootFont")) {
					log::info("Trying to find FootFont");
					foot = find_node_regex_any(actor, ".*((R|L).*Hand|(R|L)b.*Arm.*Tip).*");
					foot_kind = Foot::Front;
				} else if (starts_with(tag, "FootBack")) {
					log::info("Trying to find FootBack");
					foot = find_node_regex_any(actor, ".*((R|L).*Foot|(R|L)b.*Leg.*Tip).*");
					foot_kind = Foot::Back;
				}
				if (!foot) {
					log::info("Couldnt find the foot node");
				}
				if (foot && impact) {
					auto audio_manager = BSAudioManager::GetSingleton();
					if (!audio_manager) return;
					// log::info("Getting TES");
					// auto tes = TES::GetSingleton();
					// if (!tes) return;
					// log::info("Getting Land");
					// NiPoint3 pos = actor->GetPosition() + foot->world.translate;
					// auto land = tes->GetLandTexture(pos);
					// if (!land) return;
					log::info("Getting Material");
					// TODO get better way of finding material
					auto material_id = MATERIAL_ID::kGrass;
					auto material = BGSMaterialType::GetMaterialType(material_id);
					if (!material) return;
					log::info("Getting impact data set");
					auto imapact_data_set = get_foot_impactdata(foot_kind);
					if (!imapact_data_set) return;
					log::info("Getting impact data for material");
					auto imapact_data = imapact_data_set->impactMap.find(material)->second;
					if (!imapact_data) return;
					log::info("Creating sound info");
					NiPoint3 sound_position;
					sound_position.x = 0.0;
					sound_position.y = 0.0;
					sound_position.z = 0.0;
					BSSoundHandle sound_handle_1 = BSSoundHandle::BSSoundHandle();
					BSSoundHandle sound_handle_2 = BSSoundHandle::BSSoundHandle();
					audio_manager->BuildSoundDataFromDescriptor(sound_handle_1, imapact_data->sound1);
					audio_manager->BuildSoundDataFromDescriptor(sound_handle_2, imapact_data->sound2);
					// sound_handle_1.SetVolume(volume);
					// sound_handle_2.SetVolume(volume);

					BGSImpactManager::ImpactSoundData sound;
					sound.impactData      = imapact_data;
					sound.position        = &sound_position;
					sound.objectToFollow  = foot;
					sound.sound1          = &sound_handle_1;
					sound.sound2          = &sound_handle_2;
					sound.playSound1      = true;
					sound.playSound2      = false;
					sound.unk2A           = false;
					sound.unk30           = nullptr;
					log::info("Playing sound");
					impact->PlayImpactDataSounds(sound);
				}
			}
		}
	}
}

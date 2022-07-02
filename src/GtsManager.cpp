#include <SKSE/SKSE.h>
#include <Config.h>
#include <GtsManager.h>
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void update_height(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto base_actor = actor->GetActorBase();
		auto actor_name = base_actor->GetFullName();
        if (!actor_name) {
            return;
        }
        
        bool follower = actor->VisitFactions([actor_name](TESFaction* a_faction, std::int8_t a_rank) {
            if (!a_faction) {
                return false;
            }
            auto name = a_faction>GetFullName();
            log::info("{} is a mamber of {}", actor_name, name);
            return false;
        });
		// Check all data is loaded
		auto actor_data = GtsManager::GetSingleton().get_actor_extra_data(actor);
		if (!actor_data) {
            log::info("No actor data cached");
			return;
		}
		auto char_controller = actor->GetCharController();
		if (!char_controller) {
			log::info("No char controller: {}", actor_name);
			return;
		}
		auto& base_height_data = actor_data->base_height;

		// Get scales
		float prev_height = actor_data->prev_height; // On last update by this script
		float scale = get_scale(actor);

		// Test scale
		float test_scale = GtsManager::GetSingleton().test_scale;
		if (test_scale > 1e-5) {
			if (fabs(test_scale - scale) > 1e-5) {
				if (!set_scale(actor, test_scale)) {
					log::info("Unable to set test scale");
					return;
				}
				scale = get_scale(actor);
                log::info("Scale set to {} for {}", scale, actor_name);
			}
		}

		// Has scaled changed?
		if (fabs(prev_height - scale) <= 1e-5) {
			return;
		}

		// Is scale too small
		if (scale <= 1e-5) {
			return;
		}
        
        log::info("Scale changed from {} to {}. Updating",prev_height,scale);

		auto& test_config = Gts::Config::GetSingleton().GetTest();

		// Ready start updating
		if (test_config.CloneBound()) {
			if (!actor_data->initialised) {
				clone_bound(actor);
			}
		}
		if (test_config.UpdateModelBound()) {
			auto bsbound = get_bound(actor);
			if (!bsbound) {
				log::info("No bound: {}", actor_name);
				return;
			}
			uncache_bound(&base_height_data.collisionBound, bsbound);
			bsbound->extents *= scale;
			bsbound->center *= scale;
		}
		if (test_config.UpdateCharControllerBound()) {
			uncache_bound(&base_height_data.collisionBound, &char_controller->collisionBound);
			char_controller->collisionBound.extents *= scale;
			char_controller->collisionBound.center *= scale;
		}
		if (test_config.UpdateModelBumper()) {
			auto bumper = get_bumper(actor);
			if (bumper) {
				bumper->local.translate = base_height_data.bumper_transform.translate * scale;
				bumper->local.scale = base_height_data.bumper_transform.scale * scale;
			}
		}
		if (test_config.UpdateCharControllerBumper()) {
			uncache_bound(&base_height_data.bumperCollisionBound, &char_controller->bumperCollisionBound);
			char_controller->bumperCollisionBound.extents *= scale;
			char_controller->bumperCollisionBound.center *= scale;
		}
		if (test_config.UpdateCharControllerScale()) {
			char_controller->scale = scale;
		}
		auto model = actor->Get3D();
		if (!model) {
			log::info("No model: {}", actor_name);
			return;
		}
		auto ai_process = actor->currentProcess;

		// 3D resets
		if (test_config.Update3DModel()) {
			actor->Update3DModel();
		}
		auto task = SKSE::GetTaskInterface();
		auto node = actor->Get3D();
		task->AddTask([node]() {
			if (node) {
				NiUpdateData ctx;
				ctx.flags |= NiUpdateData::Flag::kDirty;
				node->UpdateWorldData(&ctx);
			}
		});
		if (test_config.UpdateWorldBound()) {
			task->AddTask([node]() {
				if (node) {
					node->UpdateWorldBound();
				}
			});
		}
		if (test_config.UpdateRigid()) {
			task->AddTask([node]() {
				if (node) {
					NiUpdateData ctx;
					ctx.flags |= NiUpdateData::Flag::kDirty;
					node->UpdateRigidDownwardPass(ctx, 0);
				}
			});
		}
		if (test_config.UpdateRigidConstraints()) {
			task->AddTask([node]() {
				if (node) {
					node->UpdateRigidConstraints(true);
				}
			});
		}
		actor_data->initialised = true;
        actor_data->prev_height = scale;
        log::info("height set for {)", actor_name);
	}

	//

	// auto char_controller = actor->GetCharController();
	// if (char_controller) {
	// 	// Havok Direct SCALE
	// 	hkTransform fill_me;
	// 	auto char_controller_transform = char_controller->GetTransformImpl(fill_me);
	// 	auto translation = char_controller_transform.translation;
	// 	float output[4];
	// 	_mm_storeu_ps(&output, translation);
	// 	log::info("Char Controler transform: pos={},{},{},{}", output[0], output[1], output[2], output[3]);
	// 	// Time to cheat the transform
	// 	// There's no scale on the hk transform there are two ways to get it
	// 	// 1. Put the scale in the w of the translation
	// 	// 2. Put the scale in the cross diagnoal of the rotation
	// 	auto multi = _mm_set_ps(1.0, 1.0, 1.0, scale);
	// 	auto result = _mm_mul_ps(translation, multi);
	// 	char_controller_transform.translation = result;
	// }
}

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;

	static std::atomic_bool initialized;
	static std::latch latch(1);
	if (!initialized.exchange(true)) {
		instance.test_scale = Gts::Config::GetSingleton().GetTest().GetScale();
		instance.size_method = SizeMethod::All;
		latch.count_down();
	}
	latch.wait();

	return instance;
}

// Poll for updates
void GtsManager::poll() {
	auto player_char = RE::PlayerCharacter::GetSingleton();
	if (!player_char) {
		return;
	}
	if (!player_char->Is3DLoaded()) {
		return;
	}

	auto ui = RE::UI::GetSingleton();
	if (!ui->GameIsPaused()) {
		const auto& frame_config = Gts::Config::GetSingleton().GetFrame();
		auto init_delay = frame_config.GetInitDelay();
		auto step = frame_config.GetStep() + 1; // 1 Based index

		auto current_frame = this->frame_count.fetch_add(1);
		if (current_frame < init_delay) {
			return;
		}
		if ((current_frame - init_delay) % step != 0) {
			return;
		}

		// Key presses
		auto keyboard = this->get_keyboard();
		if (keyboard) {
			log::info("Querying keyboard");
			if (keyboard->IsPressed(BSKeyboardDevice::Keys::kBracketLeft)) {
				log::info("Size UP");
				this->test_scale += 0.1;
			}
			else if (keyboard->IsPressed(BSKeyboardDevice::Keys::kBracketRight)) {
				log::info("Size Down");
				this->test_scale -= 0.1;
			}
			if (this->test_scale < 0.0) {
				this->test_scale = 0.0; // 0.is disabled
			}
		} else {
			log::info("No keyboard!");
		}
	}
}

void GtsManager::poll_actor(Actor* actor) {
	if (actor) {
		auto base_actor = actor->GetActorBase();
		auto actor_name = base_actor->GetFullName();

		auto race = actor->GetRace();
		auto race_name = race->GetFullName();


		// log::trace("Updating height of {}", actor_name);
		update_height(actor);
		// walk_nodes(actor);
	}
}

ActorExtraData* GtsManager::get_actor_extra_data(Actor* actor) {
	auto key = actor->GetFormID();
	try {
		auto no_discard = this->actor_data.at(key);
	} catch (const std::out_of_range& oor) {
		// Try to add
		log::info("Init bounding box");
		ActorExtraData result;

		auto bsbound = get_bound(actor);
		if (!bsbound) {
			return nullptr;
		}
		auto char_controller = actor->GetCharController();
		if (!char_controller) {
			return nullptr;
		}

		cache_bound(bsbound, &result.base_height.collisionBound);
		cache_bound(&char_controller->bumperCollisionBound, &result.base_height.bumperCollisionBound);
		result.base_height.actorHeight = char_controller->actorHeight;
		result.base_height.swimFloatHeightRatio = char_controller->swimFloatHeight / char_controller->actorHeight;
		auto bumper = get_bumper(actor);
		if (bumper) {
			result.base_height.bumper_transform = bumper->local;
		}
		result.initialised = false;
		this->actor_data[key] = result;
	}
	return &this->actor_data[key];
}

void Gts::cache_bound(BSBound* src, CachedBound* dst) {
	dst->center[0] = src->center.x;
	dst->center[1] = src->center.y;
	dst->center[2] = src->center.z;
	dst->extents[0] = src->extents.x;
	dst->extents[1] = src->extents.y;
	dst->extents[2] = src->extents.z;
}
void Gts::uncache_bound(CachedBound* src, BSBound* dst) {
	dst->center.x = src->center[0];
	dst->center.y = src->center[1];
	dst->center.z = src->center[2];
	dst->extents.x = src->extents[0];
	dst->extents.y = src->extents[1];
	dst->extents.z = src->extents[2];
}

BSWin32KeyboardDevice* GtsManager::get_keyboard() {
	if (!this->keyboard) {
		auto input_manager = BSInputDeviceManager::GetSingleton();
		if (input_manager) {
			this->keyboard = input_manager->GetKeyboard();
		}
	}
	return this->keyboard;
}

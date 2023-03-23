// Module that handles rumbling
#include "managers/Rumble.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"
#include "data/time.hpp"
#include "data/runtime.hpp"
#include "node.hpp"
#include "scale/scale.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	RumbleData::RumbleData(float intensity, float duration, std::string node) :
		state(RumpleState::RampingUp),
		duration(duration),
		currentIntensity(Spring(0.0, 0.5)),
		node(node),
		startTime(0.0) {
	}

	RumbleData::RumbleData(float intensity, float duration, std::string_view node) : RumbleData(intensity, duration, std::string(node)) {
	}

	void RumbleData::ChangeTargetIntensity(float intensity) {
		this->currentIntensity.target = intensity;
		this->state = RumpleState::RampingUp;
		this->startTime = 0.0;
	}
	void RumbleData::ChangeDuration(float duration) {
		this->duration = duration;
		this->state = RumpleState::RampingUp;
		this->startTime = 0.0;
	}

	ActorRumbleData::ActorRumbleData()  : delay(Timer(0.40)) {
	}

	Rumble& Rumble::GetSingleton() noexcept {
		static Rumble instance;
		return instance;
	}

	std::string Rumble::DebugName() {
		return "Rumble";
	}

	void Rumble::Reset() {
		this->data.clear();
	}
	void Rumble::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void Rumble::Start(std::string_view tag, Actor* giant, float intensity, std::string_view node) {
		Rumble::For(tag, giant, intensity, node, 0);
	}
	void Rumble::Start(std::string_view tag, Actor* giant, float intensity) {
		Rumble::For(tag, giant, intensity, "NPC Root [Root]", 0);
	}
	void Rumble::Stop(std::string_view tagsv, Actor* giant) {
		string tag = std::string(tagsv);
		auto& me = Rumble::GetSingleton();
		try {
			me.data.at(giant).tags.at(tag).state = RumpleState::RampingDown;
		} catch (std::out_of_range e) {}
	}

	void Rumble::For(std::string_view tagsv, Actor* giant, float intensity, std::string_view nodesv, float duration) {
		std::string tag = std::string(tagsv);
		std::string node = std::string(nodesv);
		auto& me = Rumble::GetSingleton();
		me.data.try_emplace(giant);
		me.data.at(giant).tags.try_emplace(tag, intensity, duration, node);
		// Reset if alreay there (but don't reset the intensity this will let us smooth into it)
		me.data.at(giant).tags.at(tag).ChangeTargetIntensity(intensity);
		me.data.at(giant).tags.at(tag).ChangeDuration(duration);
	}

	void Rumble::Once(std::string_view tag, Actor* giant, float intensity, std::string_view node) {
		Rumble::For(tag, giant, intensity, node, 1.0);
	}

	void Rumble::Once(std::string_view tag, Actor* giant, float intensity) {
		Rumble::Once(tag, giant, intensity, "NPC Root [Root]");
	}


	void Rumble::Update() {
		for (auto& [actor, data]: this->data) {
			//if (data.delay.ShouldRun()) {
				// Update values based on time passed
				for (auto& [tag, rumbleData]: data.tags) {
					switch (rumbleData.state) {
						case RumpleState::RampingUp: {
							// Increasing intensity just let the spring do its thing
							if (fabs(rumbleData.currentIntensity.value - rumbleData.currentIntensity.target) < 1e-3) {
								// When spring is done move the state onwards
								rumbleData.state = RumpleState::Rumbling;
								rumbleData.startTime = Time::WorldTimeElapsed();
							}
							break;
						}
						case RumpleState::Rumbling: {
							// At max intensity
							rumbleData.currentIntensity.value = rumbleData.currentIntensity.target;
							if (Time::WorldTimeElapsed() > rumbleData.startTime + rumbleData.duration) {
								rumbleData.state = RumpleState::RampingDown;
							}
							break;
						}
						case RumpleState::RampingDown: {
							// Stoping the rumbling
							rumbleData.currentIntensity.target = 0; // Ensure ramping down is going to zero intensity
							if (fabs(rumbleData.currentIntensity.value) <= 1e-3) {
								// Stopped
								rumbleData.state = RumpleState::Still;
							}
							break;
						}
						case RumpleState::Still: {
							// All finished cleanup
							data.tags.erase(tag);
						}
					}
				}
				// Now collect the data
				//    - Multiple effects can add rumble to the same node
				//    - We sum those effects up into cummulativeIntensity
				std::unordered_map<NiAVObject*, float> cummulativeIntensity;
				for (const auto &[tag, rumbleData]: data.tags) {
					auto node = find_node(actor, rumbleData.node);
					if (node) {
						cummulativeIntensity.try_emplace(node);
						cummulativeIntensity.at(node) += rumbleData.currentIntensity.value;
					}
				}
				// Now do the rumble
				//   - Also add up the volume for the rumble
				//   - Since we can only have one rumble (skyrim limitation)
				//     we do a weighted average to find the location to rumble from
				//     and sum the intensities
					NiPoint3 averagePos = NiPoint3(0.0, 0.0, 0.0);
       				float totalWeight = 0.0;
				for (const auto &[node, intensity]: cummulativeIntensity) {
					auto& point = node->world.translate;
          			averagePos = averagePos + point*intensity;
          			totalWeight += intensity;

					float volume = 8 * get_visual_scale(actor)/get_distance_to_camera(point);
					// Lastly play the sound at each node
					if (data.delay.ShouldRun()) {
						log::info("Playing sound at: {}, Intensity: {}", actor->GetDisplayFullName(), intensity);
						Runtime::PlaySoundAtNode("RumbleWalkSound", actor, volume, 1.0, node);
					}
				}
        		averagePos = averagePos * (1.0 / totalWeight);
        		ApplyShakeAtPoint(actor, 0.4 * totalWeight, averagePos);
			}
		}
	//}
}

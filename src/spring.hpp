#pragma once
// Critically Damped Springs
//
#include "events.hpp"

using namespace SKSE;

namespace Gts {
	class Spring {
		public:
			float value = 0.0;
			float target = 0.0;
			float velocity = 0.0;
			float halflife = 1.0;

			void Update(float delta);

			Spring();
			Spring(float initial, float halflife);

			~Spring();
	};

	class SpringManager : public EventListener {
		public:
			static SpringManager& GetSingleton();

			static void AddSpring(Spring* spring);
			static void RemoveSpring(Spring* spring);

			virtual void Update() override;

			std::unordered_set<Spring*> springs;
	}
}

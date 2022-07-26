#pragma once
// Module that holds data that is loaded at runtime
// This includes various forms
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class Runtime {
		public:
			[[nodiscard]] static Runtime& GetSingleton() noexcept;

			void Load();

			BGSSoundDescriptorForm* FootstepSoundL = nullptr;
			BGSSoundDescriptorForm* FootstepSoundR = nullptr;

			BGSSoundDescriptorForm* RumbleSoundL = nullptr;
			BGSSoundDescriptorForm* RumbleSoundR = nullptr;

			BGSSoundDescriptorForm* SprintSoundL = nullptr;
			BGSSoundDescriptorForm* SprintSoundR = nullptr;

			BGSSoundDescriptorForm* ExtraLargeFeetSoundL = nullptr;
			BGSSoundDescriptorForm* ExtraLargeFeetSoundR = nullptr;

			BGSSoundDescriptorForm* ExtraExtraLargeFeetSoundL = nullptr;
			BGSSoundDescriptorForm* ExtraExtraLargeFeetSoundR = nullptr;

			BGSSoundDescriptorForm* JumpLandSound = nullptr;

			EffectSetting* SmallMassiveThreat = nullptr;
	};
}

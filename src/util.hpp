#pragma once
// Misc codes
#include "utils/actorUtils.hpp"
#include "utils/av.hpp"
#include "utils/camera.hpp"
#include "utils/debug.hpp"
#include "utils/findActor.hpp"
#include "utils/papyrusUtils.hpp"
#include "utils/smooth.hpp"
#include "utils/sound.hpp"
#include "utils/text.hpp"
#include "utils/units.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	inline static RE::GMatrix3D* WorldToCamMatrix = (RE::GMatrix3D*) REL::RelocationID(519579, 406126).address();
	inline static RE::NiRect<float>* ViewPort = (RE::NiRect<float>*) REL::RelocationID(519618,406160).address();
	inline static float* g_worldScale = (float*)RELOCATION_ID(231896, 188105).address();
	inline static float* g_worldScaleInverse = (float*)RELOCATION_ID(230692, 187407).address();
}
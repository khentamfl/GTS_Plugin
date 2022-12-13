#pragma once
// Module that handles footsteps
#include "hooks/RE.hpp"
#include "managers/collider/capsule_data.hpp"
#include "managers/collider/convex_data.hpp"
#include "managers/collider/list_data.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class CharContData {
		public:
			CharContData();
			CharContData(bhkCharacterController* charCont);
			CharContData(CharContData&& old) : charCont(std::move(old.charCont)), capsule_data(std::move(old.capsule_data)), convex_data(std::move(old.convex_data)), list_data(std::move(old.list_data)), center(std::move(old.center)) {
			};
			~CharContData();
			void Init(bhkCharacterController* charCont);

			void ApplyScale(const float& new_scale, const hkVector4& vecScale);
			void ApplyPose(const hkVector4& origin, const float& new_scale);
			void UpdateColliders(bhkCharacterController* charCont);
			void PruneColliders(Actor* actor);
			void AddShape(hkpShape* shape);
			void AddCapsule(hkpCapsuleShape* capsule);
			void AddConvexVerts(hkpConvexVerticesShape* convex);
			void AddCapsule(hkpConvexVerticesShape* convex);
			void AddList(hkpListShape* list);

			bhkCharacterController* charCont = nullptr;
		private:
			std::unordered_map<hkpCapsuleShape*, CapsuleData> capsule_data;
			std::unordered_map<hkpConvexVerticesShape*, ConvexData> convex_data;
			std::unordered_map<hkpListShape*, ListData> list_data;
			float center = 0.0;
			float scale = 0.0;
			hkVector4 rotCenter;

			mutable std::mutex _lock;
	};
}
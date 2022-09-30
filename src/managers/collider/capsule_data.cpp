#include "managers/collider/capsule_data.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace REL;

namespace Gts {
	const std::uint32_t CAPSULE_MARKER = 494148758080886;
	hkpCapsuleShape* MakeCapsule() {
		hkpCapsuleShape* x = new hkpCapsuleShape();
		// First value is the vtable pointer
		REL::Relocation<std::uintptr_t> vptr(reinterpret_cast<std::uintptr_t>(x));

		// This is the skyrim vtable location
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};

		// Make it use skyrims vtable not our hacky one
		safe_write(vptr.address(), vtable.address());

		x->vertexA = hkVector4(0.0);
		x->vertexB = hkVector4(0.0);
		x->radius = 0.0;
		x->userData = nullptr;
		x->type = hkpShapeType::kCapsule;
		x->pad28 = CAPSULE_MARKER;
		x->pad2C = 0;
		x->pad1C = 0;
		x->memSizeAndFlags = 0xffff; // 0xffff Indicates normal size for type; If 0x0 disable ref counting

		return x;
	}

	CapsuleData::CapsuleData(hkpCapsuleShape* orig_capsule) {
		this->capsule = orig_capsule;
		this->capsule->AddReference();

		this->start = orig_capsule->vertexA;
		this->end = orig_capsule->vertexB;
		this->radius = orig_capsule->radius;
	}

	CapsuleData::~CapsuleData () {
		this->capsule->RemoveReference();
	}

	void CapsuleData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		hkVector4 origin = hkVector4(this->origin.x, this->origin.y, this->origin.z, 0.0);
		this->capsule->vertexA = (this->start - origin) * vecScale + origin;
		this->capsule->vertexB = (this->end - origin) * vecScale + origin;
		this->capsule->radius = this->radius * scale;
	}

	void CapsuleData::SetOriginMinZ() {
		hkVector4 lowest;
		if (this->start.quad.m128_f32[2] < this->end.quad.m128_f32[2]) {
			lowest = this->start;
		} else {
			lowest = this->end;
		}
		lowest.quad.m128_f32[0] = 0.0;
		lowest.quad.m128_f32[1] = 0.0;
		lowest.quad.m128_f32[3] = 0.0;
		this->SetOrigin(lowest);
	}
}

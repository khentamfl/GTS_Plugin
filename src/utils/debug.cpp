#pragma once
#include "utils/debug.hpp"
#include <ehdata.h>
#include <rttidata.h>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	std::string GetRawName(const void* obj_c) {
		// Get the meta entry in vftable
		void* obj = const_cast<void*>(obj_c);
		_RTTICompleteObjectLocator* col = reinterpret_cast<_RTTICompleteObjectLocator***>(obj)[0][-1];

		// Calculate image base by subtracting the RTTICompleteObjectLocator's pSelf offset from RTTICompleteObjectLocator's pointer
		uintptr_t imageBase = reinterpret_cast<uintptr_t>(col) - col->pSelf;

		// Get the type descriptor by adding TypeDescriptor's offset to the image base
		TypeDescriptor* tDesc = reinterpret_cast<TypeDescriptor*>(imageBase + col->pTypeDescriptor);

		// At the end, we can get the type's mangled name
		const char* colName = tDesc->name;
		return colName;
	}

	std::string Vector2Str(const hkVector4& vector) {
		return std::format("{:.2f},{:.2f},{:.2f},{:.2f}", vector.quad.m128_f32[0], vector.quad.m128_f32[1], vector.quad.m128_f32[2], vector.quad.m128_f32[3]);
	}

	std::string Vector2Str(const hkVector4* vector) {
		if (vector) {
			return std::format("{:.2f},{:.2f},{:.2f},{:.2f}", vector->quad.m128_f32[0], vector->quad.m128_f32[1], vector->quad.m128_f32[2], vector->quad.m128_f32[3]);
		} else {
			return "";
		}
	}

	std::string Vector2Str_matrixToAngles(const NiMatrix3& matrix) {
		NiPoint3 angles;
		matrix->ToEulerAnglesXYZ(angles);
		return std::format("{:.2f},{:.2f},{:.2f}", angles);
	}

	std::string Vector2Str_matrixToAngles(const NiMatrix3* matrix) {
		NiPoint3 angles;
		matrix->ToEulerAnglesXYZ(angles);
		return std::format("{:.2f},{:.2f},{:.2f}", angles);
	}

	std::string Vector2Str(const NiMatrix3* matrix) {
		return std::format("{:.2f},{:.2f},{:.2f} ; {:.2f},{:.2f},{:.2f}; {:.2f},{:.2f},{:.2f}", matrix.entry[0][0], matrix.entry[0][1], matrix.entry[0][2],
		matrix.entry[1][0], matrix.entry[1][1], matrix.entry[1][2],
		matrix.entry[2][0], matrix.entry[2][1], matrix.entry[2][2]
		);
	}

	std::string Vector2Str(const NiMatrix3& matrix) {
		return std::format("{:.2f},{:.2f},{:.2f} ; {:.2f},{:.2f},{:.2f}; {:.2f},{:.2f},{:.2f}", matrix.entry[0][0], matrix.entry[0][1], matrix.entry[0][2],
		matrix.entry[1][0], matrix.entry[1][1], matrix.entry[1][2],
		matrix.entry[2][0], matrix.entry[2][1], matrix.entry[2][2]
		);
	}

	std::string Vector2Str(const NiMatrix3* matrix) {
		return std::format("{:.2f},{:.2f},{:.2f} ; {:.2f},{:.2f},{:.2f}; {:.2f},{:.2f},{:.2f}", matrix.entry[0][0], matrix.entry[0][1], matrix.entry[0][2],
		matrix.entry[1][0], matrix.entry[1][1], matrix.entry[1][2],
		matrix.entry[2][0], matrix.entry[2][1], matrix.entry[2][2]
		);
	}

	std::string Vector2Str(const NiPoint3& vector) {
		return std::format("{:.2f},{:.2f},{:.2f}", vector.x, vector.y, vector.z);
	}

	std::string Vector2Str(const NiPoint3* vector) {
		if (vector) {
			return std::format("{:.2f},{:.2f},{:.2f}", vector->x, vector->y, vector->z);
		} else {
			return "";
		}
	}

	std::string Vector2Str(const NiQuaternion& vector) {
		return std::format("{:.2f},{:.2f},{:.2f},{:.2f}", vector.x, vector.y, vector.z, vector.w);
	}

	std::string Vector2Str(const NiQuaternion* vector) {
		if (vector) {
			return std::format("{:.2f},{:.2f},{:.2f},{:.2f}", vector->x, vector->y, vector->z, vector->w);
		} else {
			return "";
		}
	}
}

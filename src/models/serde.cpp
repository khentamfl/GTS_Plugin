#pragma once
// Serde generalisation
//
#include "models/serde.hpp"
#include "SKSE/Interfaces.h"
#include <cassert>

namespace Gts {
	void SerString(SerializationInterface* serde, string_view str) {
		vector<uint8_t> bytes(str.begin(), str.end());
		uint32_t version = 0;
		size_t strLength = bytes.size();
		serde->WriteRecordData(&version, sizeof(version));
		serde->WriteRecordData(&strLength, sizeof(strLength));
		for (uint8_t aChar: bytes) {
			serde->WriteRecordData(&aChar, sizeof(aChar));
		}
	}
	std::string DesString(SerializationInterface* serde) {
		uint32_t version = 0;
		serde->ReadRecordData(&version, sizeof(version));
		switch (version) {
			case 0: {
				size_t strLength = 0;
				serde->ReadRecordData(&strLength, sizeof(strLength));
				vector<uint8_t> bytes = {};
				for (size_t i; i < strLength; i++) {
					uint8_t aChar = 0;
					serde->ReadRecordData(&aChar, sizeof(aChar));
					bytes.push_back(aChar);
				}
				return std::string(bytes.begin(), bytes.end());
			}
		}
		throw std::runtime_error("Cannot deserilize string of this version");
	}
}

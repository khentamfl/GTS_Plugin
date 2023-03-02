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
	
	void Serde::Ser(SerializationInterface* serde) {
		{
			// Float Storage
			uint32_t magic = 'FLTS';
			serde->WriteRecordData(&magic, sizeof(magic));
			this->floatStorage.Ser(serde);
		}
		
		{
			// Int Storage
			uint32_t magic = 'INTS';
			serde->WriteRecordData(&magic, sizeof(magic));
			this->intStorage.Ser(serde);
		}
		
		{
			// Bool Storage
			uint32_t magic = 'BOOL';
			serde->WriteRecordData(&magic, sizeof(magic));
			this->boolStorage.Ser(serde);
		}
		
		{
			// Spring Storage
			uint32_t magic = 'SPRG';
			serde->WriteRecordData(&magic, sizeof(magic));
			this->springStorage.Ser(serde);
		}		
	}

	void Serde::De(SerializationInterface* serde, uint32_t version) {
		switch version {
			case 0: {
				{
					{
						// Floats
						uint32_t magic = 0;
						serde->ReadRecordData(&magic, sizeof(magic));
						assert( (magic == 'FLTS') && "Unexpected data in deser");
						this->floatStorage.clear();
						this->floatStorage.Ser(serde);
					}
					{
						// Int
						uint32_t magic = 0;
						serde->ReadRecordData(&magic, sizeof(magic));
						assert( (magic == 'INTS') && "Unexpected data in deser");
						this->intStorage.clear();
						this->intStorage.Ser(serde);
					}
					{
						// Bool
						uint32_t magic = 0;
						serde->ReadRecordData(&magic, sizeof(magic));
						assert( (magic == 'BOOL') && "Unexpected data in deser");
						this->boolStorage.clear();
						this->boolStorage.Ser(serde);
					}
					{
						// Spring
						uint32_t magic = 0;
						serde->ReadRecordData(&magic, sizeof(magic));
						assert( (magic == 'SPRG') && "Unexpected data in deser");
						this->springStorage.clear();
						this->springStorage.Ser(serde);
					}
				}
				return;
			}
			default: {
				throw std::runtime_error("Unknown GTS cosave version");
			}
		}
	}
}

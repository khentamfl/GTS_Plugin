#pragma once
// Serde generalisation
//
#include <type_traits>
#include "spring.hpp"

using namespace std;
using namespace SKSE;

namespace Gts {
	/// Helper to Serialises a String
	void SerString(SerializationInterface* serde, string_view str);
	/// Helper to DeSerialises a String
	std::string DesString(SerializationInterface* serde);
		
	class SerdeObject {
		virtual void Ser(SerializationInterface* serde) = 0;
		virtual void Des(SerializationInterface* serde) = 0;
	};
	
	template<typename T, typename=void>
	class Storage: public SerdeObject {
		public:
			void Ser(SerializationInterface* serde) override {
				uint32_t version = 0;
				serde->WriteRecordData(&version, sizeof(version));
				size_t records = this->values.size();
				serde->WriteRecordData(&records, sizeof(records));
				for (const auto &[name, value]: this->values) {
					SerString(serde, name);
					T aValue = value;
					serde->WriteRecordData(&aValue, sizeof(aValue));
				}
			}
			void Des(SerializationInterface* serde) override {
				uint32_t version = 0;
				serde->ReadRecordData(&version, sizeof(version));
				switch (version) {
					case 0: {
						size_t records = 0;
						serde->ReadRecordData(&records, sizeof(records));
						for (size_t i = 0; i<records; i++) {
							auto name = DesString(serde);
							T aValue;
							serde->ReadRecordData(&aValue, sizeof(aValue));
							this->values.insert(name, aValue);
						}
					}
					default: {
						throw std::runtime_error("Unknown Data Storage version");
					}
				}
				
			}
			T& Get(string_view name) {
				string sname(name);
				return this->values.at(&sname);
			}
			T& GetOr(string_view name, T defaultValue) {
				string sname(name);
				try {
					return this->values.at(&sname);
				} catch (std::out_of_range) {
					this->values.insert(sname, defaultValue);
					return this->values.at(&sname);
				}
			}
			void Insert(string_view name, T value) {
				string sname(name);
				this->values.insert(sname, value);
			}
		private:
			std::unordered_map<std::string, T> values;
	};
	
	template<typename T>
	class Storage<T, std::enable_if_t<std::is_base_of_v<SerdeObject, T>>>: public SerdeObject {
		public:
			void Ser(SerializationInterface* serde) override {
				uint32_t version = 0;
				serde->WriteRecordData(&version, sizeof(version));
				size_t records = this->values.size();
				serde->WriteRecordData(&records, sizeof(records));
				for (const auto &[name, value]: this->values) {
					SerString(serde, name);
					value.Ser(serde);
				}
			}
			void Des(SerializationInterface* serde) override {
				uint32_t version = 0;
				serde->ReadRecordData(&version, sizeof(version));
				switch (version) {
					case 0: {
						size_t records = 0;
						serde->ReadRecordData(&records, sizeof(records));
						for (size_t i = 0; i<records; i++) {
							auto name = DesString(serde);
							T aValue;
							aValue.Des(serde);
							this->values.insert(name, aValue);
						}
					}
					default: {
						throw std::runtime_error("Unknown Data Storage version");
					}
				}
				
			}
			T& Get(string_view name) {
				string sname(name);
				return this->values.at(&sname);
			}
			T& GetOr(string_view name, T defaultValue) {
				string sname(name);
				try {
					return this->values.at(&sname);
				} catch (std::out_of_range) {
					this->values.insert(sname, defaultValue);
					return this->values.at(&sname);
				}
			}
			void Insert(string_view name, T value) {
				string sname(name);
				this->values.insert(sname, value);
			}
		private:
			std::unordered_map<std::string, T> values;
	};
	
	class Serde {
		public:
			// Name used for debug purposes
			virtual std::string SerdeName() = 0;

			// Serialisation (save) method
			virtual void Ser(SerializationInterface* serde);

			// Desearilation (load) method
			virtual void Des(SerializationInterface* serde, uint32_t version);

			// Serialisation version for new entries
			virtual std::uint32_t SerVersion() = 0;
			
			Storage<float> floatStorage;
			Storage<int> intStorage;
			Storage<bool> boolStorage;
			Storage<Spring> springStorage;
	};
	
	
}

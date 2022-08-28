#pragma once
// Module that handles AttributeValues


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct AtributeActorData {
		float last_carry_boost = 0.0;
		float last_hp_boost = 0.0;
	};

	class AttributeManager {
		public:
			[[nodiscard]] static AttributeManager& GetSingleton() noexcept;

			void Update();
			void UpdateNpc(Actor* Npc);

			inline AtributeActorData* GetActorData(Actor* actor) {
				std::unique_lock lock(this->lock);
				if (!actor) {
					return nullptr;
				}
				auto key = actor;
				AtributeActorData* result = nullptr;
				try {
					result = &this->actor_data.at(key);
				} catch (const std::out_of_range& oor) {
					// Try to add
					AtributeActorData new_data;
					new_data.last_carry_boost =  0.0;
					new_data.last_hp_boost = 0.0;

					this->actor_data.try_emplace(key, new_data);
					try {
						result = &this->actor_data.at(key);
					} catch (const std::out_of_range& oor) {
						result = nullptr;
					}
				}
				return result;
			}

		private:
			mutable std::mutex lock;
			std::unordered_map<Actor*, AtributeActorData> actor_data;
	};
}

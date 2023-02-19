#pragma once

#include <articuno/articuno.h>

namespace Gts {
	class Debug {
		public:
			[[nodiscard]] inline spdlog::level::level_enum GetLogLevel() const noexcept {
				return _logLevel;
			}

			[[nodiscard]] inline spdlog::level::level_enum GetFlushLevel() const noexcept {
				return _flushLevel;
			}

			[[nodiscard]] inline bool ShouldProfile() const noexcept {
				return _shouldProfile;
			}

		private:
			articuno_serialize(ar) {
				auto logLevel = spdlog::level::to_string_view(_logLevel);
				auto flushLevel = spdlog::level::to_string_view(_flushLevel);
				ar <=> articuno::kv(logLevel, "logLevel");
				ar <=> articuno::kv(flushLevel, "flushLevel");
				ar <=> articuno::kv(_shouldProfile, "profile");
			}

			articuno_deserialize(ar) {
				*this = Debug();
				std::string logLevel;
				std::string flushLevel;
				if (ar <=> articuno::kv(logLevel, "logLevel")) {
					_logLevel = spdlog::level::from_str(logLevel);
				}
				if (ar <=> articuno::kv(flushLevel, "flushLevel")) {
					_flushLevel = spdlog::level::from_str(flushLevel);
				}
				ar <=> articuno::kv(_shouldProfile, "profile");
			}

			spdlog::level::level_enum _logLevel{spdlog::level::level_enum::info};
			spdlog::level::level_enum _flushLevel{spdlog::level::level_enum::trace};
			bool _shouldProfile = false;

			friend class articuno::access;
	};

	class Frame {
		public:
			[[nodiscard]] inline int GetInitDelay() const noexcept {
				return _initDelay;
			}

			[[nodiscard]] inline int GetStep() const noexcept {
				return _step;
			}

		private:
			articuno_serialize(ar) {
				ar <=> articuno::kv(_initDelay, "initDelay");
				ar <=> articuno::kv(_step, "step");
			}

			articuno_deserialize(ar) {
				*this = Frame();
				int step;
				int initDelay;
				if (ar <=> articuno::kv(step, "step")) {
					_step = step;
				}
				if (ar <=> articuno::kv(initDelay, "initDelay")) {
					_initDelay = initDelay;
				}
			}

			int _step = 0;
			int _initDelay = 0;

			friend class articuno::access;
	};

	class Sound {
		public:
			[[nodiscard]] inline std::string GetLFootstepL() const noexcept {
				return _lFootstepL;
			}
			[[nodiscard]] inline std::string GetLFootstepR() const noexcept {
				return _lFootstepR;
			}

			[[nodiscard]] inline std::string GetLJumpLand() const noexcept {
				return _lJumpLand;
			}

			[[nodiscard]] inline std::string GetXLFootstepL() const noexcept {
				return _xlFootstepL;
			}
			[[nodiscard]] inline std::string GetXLFootstepR() const noexcept {
				return _xlFootstepR;
			}

			[[nodiscard]] inline std::string GetXLRumbleL() const noexcept {
				return _xlRumbleL;
			}
			[[nodiscard]] inline std::string GetXLRumbleR() const noexcept {
				return _xlRumbleR;
			}

			[[nodiscard]] inline std::string GetXLSprintL() const noexcept {
				return _xlSprintL;
			}
			[[nodiscard]] inline std::string GetXLSprintR() const noexcept {
				return _xlSprintR;
			}

			[[nodiscard]] inline std::string GetXXLFootstepL() const noexcept {
				return _xxlFootstepL;
			}
			[[nodiscard]] inline std::string GetXXLFootstepR() const noexcept {
				return _xxlFootstepR;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_lFootstepL, "lFootstepL");
				ar <=> articuno::kv(_lFootstepR, "lFootstepR");

				ar <=> articuno::kv(_lJumpLand, "lJumpLand");

				ar <=> articuno::kv(_xlFootstepL, "xlFootstepL");
				ar <=> articuno::kv(_xlFootstepR, "xlFootstepR");

				ar <=> articuno::kv(_xlRumbleL, "xlRumbleL");
				ar <=> articuno::kv(_xlRumbleR, "xlRumbleR");

				ar <=> articuno::kv(_xlSprintL, "xlSprintL");
				ar <=> articuno::kv(_xlSprintR, "xlSprintR");

				ar <=> articuno::kv(_xxlFootstepL, "xxlFootstepL");
				ar <=> articuno::kv(_xxlFootstepR, "xxlFootstepR");


			}

			std::string _lFootstepL;
			std::string _lFootstepR;

			std::string _lJumpLand;

			std::string _xlFootstepL;
			std::string _xlFootstepR;

			std::string _xlRumbleL;
			std::string _xlRumbleR;

			std::string _xlSprintL;
			std::string _xlSprintR;

			std::string _xxlFootstepL;
			std::string _xxlFootstepR;



			friend class articuno::access;
	};

	class SpellEffects {
		public:
			[[nodiscard]] inline std::string GetSmallMassiveThreat() const noexcept {
				return _SmallMassiveThreat;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_SmallMassiveThreat, "SmallMassiveThreat");
			}

			std::string _SmallMassiveThreat;

			friend class articuno::access;
	};

	class Explosions {
		public:
			[[nodiscard]] inline std::string GetFootstepExplosion() const noexcept {
				return _footstepExplosion;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_footstepExplosion, "footstepExplosion");
			}

			std::string _footstepExplosion;

			friend class articuno::access;
	};

	class Perks {
		public:
			[[nodiscard]] inline std::string GetHHBonus() const noexcept {
				return _hhBonus;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_hhBonus, "hhBonus");
			}

			std::string _hhBonus;

			friend class articuno::access;
	};

	class Tremor {
		public:
			[[nodiscard]] inline std::string GetMethod() const noexcept {
				return _method;
			}
			[[nodiscard]] inline float GetHalfScale() const noexcept {
				return _halfScale;
			}
			[[nodiscard]] inline float GetPowerAtMin() const noexcept {
				return _powerAtMin;
			}
			[[nodiscard]] inline float GetPowerAtMax() const noexcept {
				return _powerAtMax;
			}
			[[nodiscard]] inline float GetMaxScale() const noexcept {
				return _maxScale;
			}
			[[nodiscard]] inline float GetMinScale() const noexcept {
				return _minScale;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_method, "method");
				ar <=> articuno::kv(_halfScale, "halfScale");
				ar <=> articuno::kv(_powerAtMin, "powerAtMin");
				ar <=> articuno::kv(_powerAtMax, "powerAtMax");
				ar <=> articuno::kv(_maxScale, "maxScale");
				ar <=> articuno::kv(_minScale, "minScale");
			}

			std::string _method;
			float _halfScale;
			float _powerAtMin;
			float _powerAtMax;
			float _maxScale;
			float _minScale;

			friend class articuno::access;
	};

	class Config {
		public:
			[[nodiscard]] inline const Debug& GetDebug() const noexcept {
				return _debug;
			}

			[[nodiscard]] inline const Frame& GetFrame() const noexcept {
				return _frame;
			}

			[[nodiscard]] inline const Sound& GetSound() const noexcept {
				return _sound;
			}

			[[nodiscard]] inline const SpellEffects& GetSpellEffects() const noexcept {
				return _spellEffects;
			}

			[[nodiscard]] inline const Explosions& GetExplosions() const noexcept {
				return _explosions;
			}

			[[nodiscard]] inline const Perks& GetPerks() const noexcept {
				return _perks;
			}

			[[nodiscard]] inline const Tremor& GetTremor() const noexcept {
				return _tremor;
			}

			[[nodiscard]] static const Config& GetSingleton() noexcept;

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_debug, "debug");
				ar <=> articuno::kv(_frame, "frame");
				ar <=> articuno::kv(_sound, "sound");
				ar <=> articuno::kv(_spellEffects, "spellEffects");
				ar <=> articuno::kv(_explosions, "explosions");
				ar <=> articuno::kv(_perks, "perks");
				ar <=> articuno::kv(_tremor, "tremor");
			}

			Debug _debug;
			Frame _frame;
			Sound _sound;
			SpellEffects _spellEffects;
			Explosions _explosions;
			Perks _perks;
			Tremor _tremor;

			friend class articuno::access;
	};
}

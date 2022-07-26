#pragma once

#include <articuno/articuno.h>
#include <SKSE/SKSE.h>

namespace Gts {
	class Debug {
		public:
			[[nodiscard]] inline spdlog::level::level_enum GetLogLevel() const noexcept {
				return _logLevel;
			}

			[[nodiscard]] inline spdlog::level::level_enum GetFlushLevel() const noexcept {
				return _flushLevel;
			}

		private:
			articuno_serialize(ar) {
				auto logLevel = spdlog::level::to_string_view(_logLevel);
				auto flushLevel = spdlog::level::to_string_view(_flushLevel);
				ar <=> articuno::kv(logLevel, "logLevel");
				ar <=> articuno::kv(flushLevel, "flushLevel");
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
			}

			spdlog::level::level_enum _logLevel{spdlog::level::level_enum::info};
			spdlog::level::level_enum _flushLevel{spdlog::level::level_enum::trace};

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
			[[nodiscard]] inline std::string GetFootStepR() const noexcept {
				return _footstepR;
			}
			[[nodiscard]] inline std::string GetFootStepL() const noexcept {
				return _footstepL;
			}

			[[nodiscard]] inline std::string GetRumbleL() const noexcept {
				return _rumbleL;
			}
			[[nodiscard]] inline std::string GetRumbleR() const noexcept {
				return _rumbleR;
			}

			[[nodiscard]] inline std::string GetSprintL() const noexcept {
				return _sprintL;
			}
			[[nodiscard]] inline std::string GetSprintR() const noexcept {
				return _sprintR;
			}

			[[nodiscard]] inline std::string GetXLFeetL() const noexcept {
				return _xlFeetL;
			}
			[[nodiscard]] inline std::string GetXLFeetR() const noexcept {
				return _xlFeetR;
			}

			[[nodiscard]] inline std::string GetXXLFeetL() const noexcept {
				return _xxlFeetL;
			}
			[[nodiscard]] inline std::string GetXXLFeetR() const noexcept {
				return _xxlFeetR;
			}

			[[nodiscard]] inline std::string GetJumpLand() const noexcept {
				return _jumpLand;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_footstepL, "footstepL");
				ar <=> articuno::kv(_footstepR, "footstepR");

				ar <=> articuno::kv(_rumbleL, "rumbleL");
				ar <=> articuno::kv(_rumbleR, "rumbleR");

				ar <=> articuno::kv(_sprintL, "sprintL");
				ar <=> articuno::kv(_sprintR, "sprintR");

				ar <=> articuno::kv(_xlFeetL, "xlFeetL");
				ar <=> articuno::kv(_xlFeetR, "xlFeetR");

				ar <=> articuno::kv(_xxlFeetL, "xxlFeetL");
				ar <=> articuno::kv(_xxlFeetR, "xxlFeetR");

				ar <=> articuno::kv(_jumpLand, "jumpLand");
			}

			std::string _footstepL;
			std::string _footstepR;

			std::string _rumbleL;
			std::string _rumbleR;

			std::string _sprintL;
			std::string _sprintR;

			std::string _xlFeetL;
			std::string _xlFeetR;

			std::string _xxlFeetL;
			std::string _xxlFeetR;

			std::string _jumpLand;

			friend class articuno::access;
	};

	class SpellEffects {
		public:
			[[nodiscard]] inline std::string GetSmallMassiveThreat() const noexcept {
				return _smallMassiveThreat;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_smallMassiveThreat, "smallMassiveThreat");
			}

			std::string _smallMassiveThreat;

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

			[[nodiscard]] static const Config& GetSingleton() noexcept;

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_debug, "debug");
				ar <=> articuno::kv(_frame, "frame");
				ar <=> articuno::kv(_sound, "sound");
				ar <=> articuno::kv(_spellEffects, "spellEffects");
			}

			Debug _debug;
			Frame _frame;
			Sound _sound;
			SpellEffects _spellEffects;

			friend class articuno::access;
	};
}

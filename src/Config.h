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

			[[nodiscard]] inline std::string GetXLJumpLand() const noexcept {
				return _xlJumpLand;
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

				ar <=> articuno::kv(_xlJumpLand, "xlJumpLand");


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

			std::string _xlJumpLand;

			std::string _xxlFootstepL;
			std::string _xxlFootstepR;



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

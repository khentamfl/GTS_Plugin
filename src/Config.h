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

	class Config {
		public:
			[[nodiscard]] inline const Debug& GetDebug() const noexcept {
				return _debug;
			}

			[[nodiscard]] inline const Frame& GetFrame() const noexcept {
				return _frame;
			}

			[[nodiscard]] static const Config& GetSingleton() noexcept;

			[[nodiscard]] inline int GetExperiment() const noexcept {
				return _experiment;
			}

			[[nodiscard]] inline std::string GetFootstepLForm() const noexcept {
				return _footstepLForm;
			}
			[[nodiscard]] inline std::string GetFootstepRForm() const noexcept {
				return _footstepRForm;
			}

			[[nodiscard]] inline std::string GetFootstepSoundLForm() const noexcept {
				return _footstepSoundLForm;
			}

			[[nodiscard]] inline std::string GetFootstepSoundRForm() const noexcept {
				return _footstepSoundRForm;
			}

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_debug, "debug");
				ar <=> articuno::kv(_frame, "frame");
				ar <=> articuno::kv(_experiment, "experiment");
				ar <=> articuno::kv(_footstepLForm, "footstepLForm");
				ar <=> articuno::kv(_footstepRForm, "footstepRForm");
				ar <=> articuno::kv(_footstepSoundLForm, "footstepSoundLForm");
				ar <=> articuno::kv(_footstepSoundRForm, "footstepSoundRForm");
			}

			Debug _debug;
			Frame _frame;
			int _experiment;
			std::string _footstepLForm;
			std::string _footstepRForm;
			std::string _footstepSoundLForm;
			std::string _footstepSoundRForm;

			friend class articuno::access;
	};
}

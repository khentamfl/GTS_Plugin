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

	class Test {
		public:
			[[nodiscard]] inline float GetScale() const noexcept {
				return _scale;
			}
			[[nodiscard]] inline float CloneBound() const noexcept {
				return _clone_bound;
			}
			[[nodiscard]] inline float UpdateModelBound() const noexcept {
				return _update_model_bound;
			}
			[[nodiscard]] inline float UpdateCharControllerBound() const noexcept {
				return _update_char_bound;
			}
			[[nodiscard]] inline float UpdateModelBumper() const noexcept {
				return _update_model_bumper;
			}
			[[nodiscard]] inline float UpdateCharControllerBumper() const noexcept {
				return _update_char_bumper;
			}
			[[nodiscard]] inline float UpdateCharControllerScale() const noexcept {
				return _update_char_scale;
			}
			[[nodiscard]] inline float Update3DModel() const noexcept {
				return _update_3d_model;
			}
			[[nodiscard]] inline float UpdateRigid() const noexcept {
				return _update_rigid_model;
			}
			[[nodiscard]] inline float UpdateRigidConstraints() const noexcept {
				return _update_rigid_constraint_model;
			}
			[[nodiscard]] inline float UpdateWorldBound() const noexcept {
				return _update_world_bound;
			}

		private:
			articuno_serialize(ar) {
				ar <=> articuno::kv(_scale, "scale");
			}

			articuno_deserialize(ar) {
				*this = Test();
				float scale;
				if (ar <=> articuno::kv(scale, "scale")) {
					_scale = scale;
				}
				bool clone_bound;
				if (ar <=> articuno::kv(clone_bound, "cloneBound")) {
					_clone_bound = clone_bound;
				}
				bool update_model_bound;
				if (ar <=> articuno::kv(update_model_bound, "updateModelBound")) {
					_update_model_bound = update_model_bound;
				}
				bool update_char_bound;
				if (ar <=> articuno::kv(update_char_bound, "updateCharControllerBound")) {
					_update_char_bound = update_char_bound;
				}
				bool update_model_bumper;
				if (ar <=> articuno::kv(update_model_bumper, "updateModelBumper")) {
					_update_model_bumper = update_model_bumper;
				}
				bool update_char_bumper;
				if (ar <=> articuno::kv(update_char_bumper, "updateCharControllerBumper")) {
					_update_char_bumper = update_char_bumper;
				}
				bool update_char_scale;
				if (ar <=> articuno::kv(update_char_scale, "updateCharControllerScale")) {
					_update_char_scale = update_char_scale;
				}
				bool update_3d_model;
				if (ar <=> articuno::kv(clone_bound, "update3DModel")) {
					_update_3d_model = update_3d_model;
				}
				bool update_rigid_model;
				if (ar <=> articuno::kv(update_rigid_model, "updateRigidModel")) {
					_update_rigid_model = update_rigid_model;
				}
				bool update_rigid_constraint_model;
				if (ar <=> articuno::kv(update_rigid_constraint_model, "updateRigidConstraintsModel")) {
					_update_rigid_constraint_model = update_rigid_constraint_model;
				}
				bool _update_world_bound;
				if (ar <=> articuno::kv(_update_world_bound, "updateWorldBound")) {
					_update_world_bound = _update_world_bound;
				}
			}

			float _scale = 5.0;
			bool _clone_bound = true;
			bool _update_model_bound = true;
			bool _update_char_bound = true;
			bool _update_model_bumper = true;
			bool _update_char_bumper = true;
			bool _update_char_scale = true;
			bool _update_3d_model = false;
			bool _update_rigid_model = true;
			bool _update_rigid_constraint_model = true;
			bool _update_world_bound = true;

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

			[[nodiscard]] inline const Test& GetTest() const noexcept {
				return _test;
			}

			[[nodiscard]] static const Config& GetSingleton() noexcept;

		private:
			articuno_serde(ar) {
				ar <=> articuno::kv(_debug, "debug");
				ar <=> articuno::kv(_frame, "frame");
				ar <=> articuno::kv(_test, "test");
			}

			Debug _debug;
			Frame _frame;
			Test _test;

			friend class articuno::access;
	};
}

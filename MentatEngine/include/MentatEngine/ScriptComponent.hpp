/**
 *
 * @brief SCriptComponent base class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __SCRIPTCOMPONENT_HPP__
#define __SCRIPTCOMPONENT_HPP__ 1

#include "../KFS/include/mat4.h"
#include "../KFS/include/vec3.h"
#include <cmath>
#include <memory>
#include <string>
#include <sol/sol.hpp>
#include "../Include/MentatEngine/Entity.hpp"
#include "../Include/MentatEngine/TransformComponent.hpp"
#include "../Include/MentatEngine/LightComponent.hpp"
#include "../Include/MentatEngine/CameraComponent.hpp"
#include "../Include/MentatEngine/Input.hpp"
#include "../Include/MentatEngine/Core.hpp"
#include "../Include/MentatEngine/ECSManager.hpp"
#include "../deps/KFS/include/vec3.h"

namespace ME {
	class Input;
	class ScriptComponent {
		public:
			/**
			 * @brief Constructs the Script Component with the id of the owner.
			 * @params id The id of the owner.
			 */
			ScriptComponent(unsigned long id);

			ScriptComponent(ScriptComponent&& other) noexcept : code_{ other.code_ }, state{ std::move(other.state) }, code_path{ other.code_path },
			enabled{ other.enabled }, id_{ other.id_ } {};

			ScriptComponent& operator=(ScriptComponent&& other) noexcept {
				if (this != &other) {
					code_ = other.code_;
					state = std::move(other.state);
					code_path = other.code_path;
					enabled = other.enabled;
					id_ = other.id_;
				}
				return *this;
			}

			ScriptComponent(const ScriptComponent&) = delete;
			ScriptComponent& operator=(const ScriptComponent&) = delete;

            /**
             * @brief Gets a shared pointer to the script source code string.
             * @return A std::shared_ptr containing the code.
             */
            std::shared_ptr<std::string> GetSharedCode() {
                return code_;
            }

            /**
             * @brief Enables or disables the execution of the script code.
             * @param res True to enable, false to disable.
             */
            void EnableCode(bool res) {
                enabled = res;
            }

            /**
             * @brief Checks if the script code execution is currently enabled.
             * @return True if enabled, false otherwise.
             */
            bool IsCodeEnabled() {
                return enabled;
            }

            /**
             * @brief Launches a specific function within the script state.
             *
             * This is a variadic template that forwards arguments to the script function.
             * It checks if the function exists and handles execution errors.
             *
             * @tparam Args Template parameter pack for the function arguments.
             * @param fName The name of the function to be called.
             * @param args The arguments to pass to the function.
             */
            template<typename... Args>
            void launchFunction(std::string fName, Args&&... args) {
                if (!enabled) return;

                sol::protected_function ff = state[fName];
                if (!ff.valid()) {
                    printf("The function %s does not exist\n", fName.c_str());
                    return;
                }

                auto result = ff(std::forward<Args>(args)...);

                if (!result.valid()) {
                    sol::error err = result;
                    std::cerr << "Error executing " << fName << ": " << err.what() << std::endl;
                }
            }

            /**
             * @brief Executes the start-up logic for the script.
             * @param dt Delta time since the last update.
             */
            void launchOnStart(float dt);

            /**
             * @brief Executes the per-frame update logic.
             * @param dt Delta time since the last update.
             */
            void launchOnFrame(float dt);

            /**
             * @brief Loads and sets the script code from a given file path.
             * @param codePath String representing the file path.
             */
            void setCodeByPath(std::string codePath);

            /**
             * @brief Sets the input handler for this specific instance.
             * @param input Pointer to the Input object.
             */
            void SetInput(Input* input);

            /**
             * @brief Gets the path of the currently loaded script.
             * @return The file path as a std::string.
             */
            std::string getCurrentScriptPath() { return code_path; }

            /**
            * @brief Registers a copyable C++ value as a global variable in the script state.
            *
            * This method is intended for plain values and containers that should be copied
            * into Lua, such as std::string, Vec3, std::vector, integers or floats.
            *
            * @tparam T The type of the value.
            * @param name The name that will be used to access the value within the script.
            * @param value The value to be copied into the Lua state.
            */
            template <typename T>
            void addGlobalValue(const std::string& name, const T& value) {
                state[name] = value;
            }

            /**
             * @brief Registers a C++ object as a global variable in the script state by reference.
             *
             * This method is intended for engine objects or systems that must be exposed
             * to Lua through their address, such as Core, Window, Input or other runtime objects.
             *
             * @tparam T The type of the object.
             * @param name The name that will be used to access the object within the script.
             * @param object Reference to the object to be registered.
             */
            template <typename T>
            void addGlobalRef(const std::string& name, T& object) {
                state[name] = &object;
            }

            /**
             * @brief Sets the global input handler for all script instances.
             * @param input Pointer to the global Input object.
             */
            static void SetGlobalInput(Input* input);

		private:
			std::shared_ptr<std::string> code_;
			sol::state state;
			std::string code_path;
			bool enabled;
			unsigned long id_;
			static Input* global_input_;
	};
}

#endif // __SCRIPTCOMPONENT_HPP__
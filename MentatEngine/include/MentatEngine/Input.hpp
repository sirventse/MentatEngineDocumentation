/**
 *
 * @brief Input manager.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __INPUT_HPP__
#define __INPUT_HPP__ 1

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <../include/MentatEngine/Window.hpp>
#include <../deps/KFS/include/vec2.h>
#include "../deps/imgui/imgui.h"
#include <../deps/imgui/imgui_impl_glfw.h>
#include <unordered_map>

namespace ME {
	class Input {
		public:
			enum class Context {
				Editor,
				Runtime
			};

			/**
			 * @brief Constructs the Input system associated with a specific window.
			 * @param window Reference to the window context for event polling.
			 */
			Input(Window& window);

			~Input();
			Input(const Input& right) = delete;
			Input& operator=(const Input& right) = delete;

			Input(Input&& other);
			Input& operator=(Input&& other);

			/**
			 * @brief Polls and updates the internal state of all keys and mouse buttons.
			 * Should be called once per frame.
			 */
			void Update();

			enum Action {
				MoveUp,
				MoveDown,
				MoveLeft,
				MoveRight,
				//RotateLeft, // deprecated on 3D
				//RotateRight, // deprecated on 3D
				ScaleUp,
				ScaleDown,
				ChangeColor,
				ChangeShape,
				RotatePositiveX,
				RotateNegativeX,
				RotatePositiveY,
				RotateNegativeY,
				RotatePositiveZ,
				RotateNegativeZ,
				MoveForward,
				MoveBackward,
				Sprint,

				FlyEnable,
				FlyForward,
				FlyBackward,
				FlyLeft,
				FlyRight,
				FlyUp,
				FlyDown,
				CycleGizmoMode

			};

			struct KeyState
			{
				int scancode;
				int mod;
				int action;
				int lastAction;
			};

			enum InputKey {
				D = GLFW_KEY_D,
				W = GLFW_KEY_W,
				A = GLFW_KEY_A,
				S = GLFW_KEY_S,
				Q = GLFW_KEY_Q,
				E = GLFW_KEY_E,
				O = GLFW_KEY_O,
				P = GLFW_KEY_P,
				R = GLFW_KEY_R,
				F = GLFW_KEY_F,
				T = GLFW_KEY_T,
				G = GLFW_KEY_G,
				Y = GLFW_KEY_Y,
				H = GLFW_KEY_H,
				L = GLFW_KEY_L,
				MB_Left = GLFW_MOUSE_BUTTON_LEFT,
				MB_Right = GLFW_MOUSE_BUTTON_RIGHT,
				MB_Middle = GLFW_MOUSE_BUTTON_MIDDLE,
				SPACEBAR = GLFW_KEY_SPACE,

				// TODO -> Obligado a utilizar flag global y PollGamepad
				XBOX_A = GLFW_GAMEPAD_BUTTON_A,
				XBOX_B = GLFW_GAMEPAD_BUTTON_B,
				XBOX_X = GLFW_GAMEPAD_BUTTON_X,
				XBOX_Y = GLFW_GAMEPAD_BUTTON_Y
			};

			/**
			 * @brief Gets the current mouse cursor position in window coordinates.
			 * @return A Vec2 containing the X and Y coordinates.
			 */
			Vec2 MousePos() const;

			/**
			 * @brief Binds an action to a specific key for Editor mode.
			 * @param action The logical action to trigger.
			 * @param key The physical key code to bind.
			 */
			void BindEditorKey(Action action, InputKey key);

			/**
			 * @brief Binds an action to a specific key for Runtime (Game) mode.
			 * @param action The logical action to trigger.
			 * @param key The physical key code to bind.
			 */
			void BindRuntimeKey(Action action, InputKey key);

			/** @brief Removes all existing Editor-mode key bindings. 
			*/
			void ClearEditorBindings();

			/** @brief Removes all existing Runtime-mode key bindings. 
			*/
			void ClearRuntimeBindings();

			/** @brief Clears both Editor and Runtime key bindings. 
			*/
			void ClearAllBindings();

			/**
			 * @brief Checks if an action is currently held down in the active mode.
			 * @param a The action to check.
			 * @return True if the key is pressed.
			 */
			bool checkActionPressed(Action a);

			/**
			 * @brief Checks if an action was pressed exactly in this frame.
			 * @param a The action to check.
			 * @return True if the key transitioned from released to pressed.
			 */
			bool checkActionJustPressed(Action a);

			/**
			 * @brief Checks if an action was released exactly in this frame.
			 * @param a The action to check.
			 * @return True if the key transitioned from pressed to released.
			 */
			bool checkActionJustReleased(Action a);

			/** @name Editor-Specific Checks
			 *  Methods to query actions specifically within the Editor context.
			 * @{ */
			bool checkEditorActionPressed(Action a);
			bool checkEditorActionJustPressed(Action a);
			bool checkEditorActionJustReleased(Action a);
			/** @} */

			/** @name Runtime-Specific Checks
			 *  Methods to query actions specifically within the Game/Runtime context.
			 * @{ */
			bool checkRuntimeActionPressed(Action a);
			bool checkRuntimeActionJustPressed(Action a);
			bool checkRuntimeActionJustReleased(Action a);
			/** @} */

			/**
			 * @brief Typedef for a function pointer that determines if the engine is in Runtime mode.
			 */
			using RuntimeModeFn = bool(*)();

			/**
			 * @brief Sets the provider function used to check the current engine mode.
			 * @param fn Function pointer returning true if in Runtime, false if in Editor.
			 */
			void SetRuntimeModeProvider(RuntimeModeFn fn);

			/**
			 * @brief Checks if the engine is currently in Runtime mode via the provider.
			 * @return True if in Runtime mode.
			 */
			bool IsRuntimeMode() const;

			/**
			 * @brief Retrieves the current map of editor bindings.
			 * @return A reference to the internal map of Action to key codes.
			 */
			const std::unordered_map<Action, std::vector<int>>& GetEditorBindings() const;

			/**
			 * @brief Converts an Action enum to its string representation.
			 * @param a The action to convert.
			 * @return A C-string with the action name.
			 */
			static const char* ActionToString(Action a);

			/**
			 * @brief Converts a key code to its string representation.
			 * @param key The key code.
			 * @return A C-string with the key name (e.g., "W", "Space").
			 */
			static const char* KeyToString(int key);

		private:
			GLFWwindow* windowReference_;

			static std::unordered_map<GLFWwindow*, Input*> window_input_map_;

			/**
			 * @brief Static entry point for GLFW key events.
			 *
			 * This static method bridges GLFW's C-style callback with the specific Input instance.
			 * It first forwards events to ImGui. If ImGui does not consume the input,
			 * it dispatches the event to the appropriate Input instance.
			 *
			 * @param w The GLFW window that received the event.
			 * @param key The keyboard key that was pressed or released.
			 * @param scancode The system-specific scancode of the key.
			 * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
			 * @param mods Bit field describing which modifier keys were held down.
			 */
			static void staticKeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);

			/**
			 * @brief Internal handler for keyboard events.
			 *
			 * Updates the internal #keyStates_ map, tracking the transition between
			 * the current and previous key actions to support "JustPressed" logic.
			 *
			 * @param w The GLFW window context.
			 * @param key The keyboard key code.
			 * @param scancode The key's scancode.
			 * @param action The current key action.
			 * @param mods The active modifier keys.
			 */
			void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);

			/**
			 * @brief Static entry point for GLFW mouse button events.
			 *
			 * Forwards mouse input to ImGui. If the cursor is not over an ImGui element
			 * (@c WantCaptureMouse is false), the event is passed to the instance-level mouse handler.
			 *
			 * @param w The GLFW window that received the event.
			 * @param btn The mouse button that was pressed or released.
			 * @param action GLFW_PRESS or GLFW_RELEASE.
			 * @param mods Bit field describing which modifier keys were held down.
			 */
			static void staticMouseCallback(GLFWwindow* w, int btn, int action, int mods);

			/**
			 * @brief Internal handler for mouse button events.
			 *
			 * Updates the state of mouse buttons within the internal key state map.
			 *
			 * @param w The GLFW window context.
			 * @param btn The mouse button code.
			 * @param action The current button action.
			 * @param mods The active modifier keys.
			 */
			void mouseCallback(GLFWwindow* w, int btn, int action, int mods);

			std::unordered_map<Action, std::vector<int>> editorActionsWithKeys_;
			std::unordered_map<Action, std::vector<int>> runtimeActionsWithKeys_;
			RuntimeModeFn runtimeModeProvider_ = nullptr;
			std::unordered_map<int, KeyState> keyStates_;
			std::unordered_map<int, bool> previousKeys_;
	};
}

#endif //__INPUT_HPP__
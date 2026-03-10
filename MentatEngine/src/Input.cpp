/**
 *
 * @brief Input manager implementation.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#include <../include/MentatEngine/Input.hpp>

namespace ME {

	namespace {
		bool CheckPressedInMap(
			const std::unordered_map<ME::Input::Action, std::vector<int>>& bindings,
			const std::unordered_map<int, ME::Input::KeyState>& keyStates,
			ME::Input::Action a)
		{
			auto it = bindings.find(a);
			if (it == bindings.end()) return false;

			for (int key : it->second) {
				auto ksIt = keyStates.find(key);
				if (ksIt == keyStates.end()) continue;

				const auto& ks = ksIt->second;
				if (ks.action == GLFW_PRESS || ks.action == GLFW_REPEAT) return true;
			}
			return false;
		}

		bool CheckJustPressedInMap(
			const std::unordered_map<ME::Input::Action, std::vector<int>>& bindings,
			const std::unordered_map<int, ME::Input::KeyState>& keyStates,
			ME::Input::Action a)
		{
			auto it = bindings.find(a);
			if (it == bindings.end()) return false;

			for (int key : it->second) {
				auto ksIt = keyStates.find(key);
				if (ksIt == keyStates.end()) continue;

				const auto& ks = ksIt->second;
				if (ks.action == GLFW_PRESS && ks.lastAction == GLFW_RELEASE) return true;
			}
			return false;
		}

		bool CheckJustReleasedInMap(
			const std::unordered_map<ME::Input::Action, std::vector<int>>& bindings,
			const std::unordered_map<int, ME::Input::KeyState>& keyStates,
			ME::Input::Action a)
		{
			auto it = bindings.find(a);
			if (it == bindings.end()) return false;

			for (int key : it->second) {
				auto ksIt = keyStates.find(key);
				if (ksIt == keyStates.end()) continue;

				const auto& ks = ksIt->second;
				if (ks.action == GLFW_RELEASE && ks.lastAction == GLFW_PRESS) return true;
			}
			return false;
		}
	}
	std::unordered_map<GLFWwindow*, ME::Input*> ME::Input::window_input_map_{};
	Input::Input(Window& window) {
		windowReference_ = window.getGLFWwindow();
		if (window_input_map_.empty()) {
			glfwSetKeyCallback(windowReference_, staticKeyCallback);
			glfwSetMouseButtonCallback(windowReference_, staticMouseCallback);
			//glfwSetGamepadCallback()
			//glfwSetJoystickCallback TODO
			//glfwSetCursorPosCallback() TODO
		}
		Input::window_input_map_[windowReference_] = this;

	}

	void Input::staticKeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods)
	{
		ImGui_ImplGlfw_KeyCallback(w, key, scancode, action, mods);
		if (!ImGui::GetIO().WantCaptureKeyboard) {
			Input::window_input_map_[w]->keyCallback(w, key, scancode, action, mods);
		}
	}

	void Input::keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods)
	{
		auto& ks = keyStates_[key];  // crea si no existe y devuelve ref
		ks.scancode = scancode;
		ks.mod = mods;
		ks.lastAction = ks.action;
		ks.action = action;

		//printf("Key: %d\n Action: %d\n", key, action);
	}

	void Input::staticMouseCallback(GLFWwindow* w, int btn, int action, int mods)
	{
		ImGui_ImplGlfw_MouseButtonCallback(w, btn, action, mods);
		if (!ImGui::GetIO().WantCaptureMouse) {
			Input::window_input_map_[w]->mouseCallback(w, btn, action, mods);
		}
	}

	void Input::mouseCallback(GLFWwindow* w, int bnt, int action, int mods)
	{
		auto& ks = keyStates_[bnt];  // crea si no existe y devuelve ref
		ks.scancode = -1;
		ks.mod = mods;
		ks.lastAction = ks.action;
		ks.action = action;

		//printf("Key: %d\n Action: %d\n", bnt, action);
	}

	Input::~Input() {
		if (windowReference_ != nullptr) {
			auto it = window_input_map_.find(windowReference_);
			if (it != window_input_map_.end() && it->second == this) {
				window_input_map_.erase(it);
			}
			glfwSetKeyCallback(windowReference_, nullptr);
			glfwSetMouseButtonCallback(windowReference_, nullptr);

			windowReference_ = nullptr;
		}
		windowReference_ = nullptr;
		Input::window_input_map_.erase(windowReference_);
	}

	Input::Input(Input&& other)
	{
		*this = std::move(other); // same code as operator =
	}

	Input& Input::operator=(Input&& other){ 
		if (this == &other) return *this;

		if (windowReference_) {
			auto it = window_input_map_.find(windowReference_);
			if (it != window_input_map_.end() && it->second == this) {
				window_input_map_.erase(it);
			}
		}

		windowReference_ = other.windowReference_;
		editorActionsWithKeys_ = std::move(other.editorActionsWithKeys_);
		runtimeActionsWithKeys_ = std::move(other.runtimeActionsWithKeys_);
		runtimeModeProvider_ = other.runtimeModeProvider_;
		keyStates_ = std::move(other.keyStates_);

		if (windowReference_) {
			window_input_map_[windowReference_] = this;
			glfwSetKeyCallback(windowReference_, &Input::staticKeyCallback);
			glfwSetMouseButtonCallback(windowReference_, &Input::staticMouseCallback);
		}

		other.windowReference_ = nullptr;
		other.editorActionsWithKeys_.clear();
		other.runtimeActionsWithKeys_.clear();
		other.runtimeModeProvider_ = nullptr;
		other.keyStates_.clear();

		return *this;
	}

	void Input::Update()
	{
		for (auto& [key, state] : keyStates_) {
			state.lastAction = state.action;
		}
	}

	Vec2 Input::MousePos() const
	{
		double x, y;
		glfwGetCursorPos(windowReference_, &x, &y);
		return Vec2{ static_cast<float>(x), static_cast<float>(y) };
	}

	void Input::BindEditorKey(Action a, InputKey k)
	{
		editorActionsWithKeys_[a].push_back(k);
	}

	void Input::BindRuntimeKey(Action a, InputKey k)
	{
		runtimeActionsWithKeys_[a].push_back(k);
	}

	void Input::ClearEditorBindings()
	{
		editorActionsWithKeys_.clear();
	}

	void Input::ClearRuntimeBindings()
	{
		runtimeActionsWithKeys_.clear();
	}

	void Input::ClearAllBindings()
	{
		editorActionsWithKeys_.clear();
		runtimeActionsWithKeys_.clear();
	}

	void Input::SetRuntimeModeProvider(RuntimeModeFn fn)
	{
		runtimeModeProvider_ = fn;
	}

	bool Input::IsRuntimeMode() const
	{
		return runtimeModeProvider_ ? runtimeModeProvider_() : false;
	}

	const std::unordered_map<Input::Action, std::vector<int>>& Input::GetEditorBindings() const
	{
		return editorActionsWithKeys_;
	}

	const char* Input::ActionToString(Action a)
	{
		switch (a) {
		case Action::MoveForward:  return "MoveForward";
		case Action::MoveBackward: return "MoveBackward";
		case Action::MoveLeft:     return "MoveLeft";
		case Action::MoveRight:    return "MoveRight";
		case Action::MoveUp:       return "MoveUp";
		case Action::MoveDown:     return "MoveDown";
		case Action::Sprint:       return "Sprint";
		case Action::FlyEnable:    return "FlyEnable";
		case Action::FlyForward:    return "Fly forward";
		case Action::FlyBackward:    return "Fly backward";
		case Action::FlyLeft:    return "Fly left";
		case Action::FlyRight:    return "Fly right";
		case Action::FlyUp:    return "Fly up";
		case Action::FlyDown:    return "Fly down";
		case Action::CycleGizmoMode: return "Cycle gizmo mode";
		default:                   return "UnknownAction";
		}
	}

	const char* Input::KeyToString(int key)
	{
		switch (key) {
		case InputKey::W:         return "W";
		case InputKey::A:         return "A";
		case InputKey::S:         return "S";
		case InputKey::D:         return "D";
		case InputKey::Q:         return "Q";
		case InputKey::E:         return "E";
		case InputKey::SPACEBAR:  return "SPACEBAR";
		case InputKey::MB_Right:  return "Mouse button right";
		default:                  return "UnknownKey";
		}
	}

	bool Input::checkEditorActionPressed(Action a)
	{
		return CheckPressedInMap(editorActionsWithKeys_, keyStates_, a);
	}

	bool Input::checkEditorActionJustPressed(Action a)
	{
		return CheckJustPressedInMap(editorActionsWithKeys_, keyStates_, a);
	}

	bool Input::checkEditorActionJustReleased(Action a)
	{
		return CheckJustReleasedInMap(editorActionsWithKeys_, keyStates_, a);
	}

	bool Input::checkRuntimeActionPressed(Action a)
	{
		return CheckPressedInMap(runtimeActionsWithKeys_, keyStates_, a);
	}

	bool Input::checkRuntimeActionJustPressed(Action a)
	{
		return CheckJustPressedInMap(runtimeActionsWithKeys_, keyStates_, a);
	}

	bool Input::checkRuntimeActionJustReleased(Action a)
	{
		return CheckJustReleasedInMap(runtimeActionsWithKeys_, keyStates_, a);
	}

	bool Input::checkActionPressed(Action a)
	{
		return IsRuntimeMode()
			? checkRuntimeActionPressed(a)
			: checkEditorActionPressed(a);
	}

	bool Input::checkActionJustPressed(Action a)
	{
		return IsRuntimeMode()
			? checkRuntimeActionJustPressed(a)
			: checkEditorActionJustPressed(a);
	}

	bool Input::checkActionJustReleased(Action a)
	{
		return IsRuntimeMode()
			? checkRuntimeActionJustReleased(a)
			: checkEditorActionJustReleased(a);
	}
}


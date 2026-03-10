#include "../Include/MentatEngine/ScriptComponent.hpp"
#include "../include/MentatEngine/OpenGL/RenderizableComponent.hpp"
#include <fstream>
#include <iostream>

namespace ME {

	Input* ScriptComponent::global_input_ = nullptr;

	void ScriptComponent::SetGlobalInput(Input* input)
	{
		global_input_ = input;
	}

	ScriptComponent::ScriptComponent(unsigned long id) : enabled{ false }, code_path{ "" }, id_{ id } {
		code_ = std::make_shared<std::string>(R"(print("SCRIPT WITHOUT CODE"))");
		
		state.open_libraries();

		state.new_usertype<Vec3>("Vec3",
			sol::constructors<Vec3(), Vec3(float, float, float)>(),
			"x_", &Vec3::x_,
			"y_", &Vec3::y_,
			"z_", &Vec3::z_
		);

		state.new_usertype<Entity>("Entity",

			sol::constructors<Entity()>(),
			"GetID", &Entity::GetID,

			// --- ADDERS 

			"AddTransformComp", [](Entity& self) -> ME::TransformComponent& {
				return self.AddComponent<ME::TransformComponent>();
			},

			"AddScriptComp", [](Entity& self, unsigned long selfid) -> ME::ScriptComponent& {
				return self.AddComponent<ME::ScriptComponent>(selfid);
			},

			"AddRenderComp", [](ME::Entity& self, std::vector<Vec3> v, size_t vn, std::vector<unsigned int> i, 
				size_t in, Vec3 c, std::string path) -> ME::RenderizableComponent& {
				return self.AddComponent<ME::RenderizableComponent>(v.data(), v.size(), i.data(), i.size(), c, path);
			},

			"AddLightComp", [](ME::Entity& self) -> ME::LightComponent& {
				return self.AddComponent<ME::LightComponent>();
			},

			"AddCameraComp", [](ME::Entity& self) -> ME::CameraComponent& {
				return self.AddComponent<ME::CameraComponent>();
			},

			// --- GETTERS

			"GetTransformComp", &Entity::GetComponent<ME::TransformComponent>,
			"GetRenderComp", &Entity::GetComponent<ME::RenderizableComponent>,
			"GetScriptComp", &Entity::GetComponent<ME::ScriptComponent>,
			"GetLightComp", &Entity::GetComponent<ME::LightComponent>,
			"GetCameraComp", &Entity::GetComponent<ME::CameraComponent>
		);

		state.new_usertype<Window>("Window");

		state.new_usertype<TransformComponent>("TransformComponent",
			"Position",
			&TransformComponent::Position,
			"SetPosition",
			&TransformComponent::SetPosition,
			"Rotation",
			&TransformComponent::Rotation,
			"SetRotation",
			&TransformComponent::SetRotation,
			"Scale",
			&TransformComponent::Scale,
			"SetScale",
			&TransformComponent::SetScale
		);

		state.new_usertype<ME::RenderizableComponent>("RenderizableComponent",
			sol::constructors<ME::RenderizableComponent(
				const Vec3*, const size_t&,
				const unsigned int*, const size_t&,
				const Vec3*, std::string path
			)>(),
			"Bind", &ME::RenderizableComponent::Bind,
			"Unbind", &ME::RenderizableComponent::Unbind,
			"UpdateGeometry", [](ME::RenderizableComponent& self, std::vector<Vec3> v, size_t vn) {
				self.UpdateGeometry(v.data(), v.size());
			},
			"GetColor", &ME::RenderizableComponent::GetColor,
			"SetColor", & ME::RenderizableComponent::SetColor,
			"IsColorActive", &ME::RenderizableComponent::IsColorActive,
			"SetVisibility",&ME::RenderizableComponent::SetVisibility,
			"Visible", &ME::RenderizableComponent::Visible
		);

		
		state.new_usertype<ScriptComponent>("ScriptComponent",
			sol::constructors<ME::ScriptComponent(
				unsigned long selfid
			)>(),
			"EnableCode",
			&ScriptComponent::EnableCode,
			"IsCodeEnabled",
			&ScriptComponent::IsCodeEnabled,
			"LaunchOnFrame",
			&ScriptComponent::launchOnFrame,
			"SetCodeByPath", [](ME::ScriptComponent& self, std::string path) {
				self.setCodeByPath((char*)path.c_str());
			}
		);
		

		state.new_enum("Action",
			"MoveUp", Input::Action::MoveUp,
			"MoveDown", Input::Action::MoveDown,
			"MoveLeft", Input::Action::MoveLeft,
			"MoveRight", Input::Action::MoveRight,
			"MoveForward", Input::Action::MoveForward,
			"MoveBackward", Input::Action::MoveBackward,


			"RotatePositiveX", Input::Action::RotatePositiveX,
			"RotateNegativeX", Input::Action::RotateNegativeX,
			"RotatePositiveY", Input::Action::RotatePositiveY,
			"RotateNegativeY", Input::Action::RotateNegativeY,
			"RotatePositiveZ", Input::Action::RotatePositiveZ,
			"RotateNegativeZ", Input::Action::RotateNegativeZ,

			"ScaleUp", Input::Action::ScaleUp,
			"ScaleDown", Input::Action::ScaleDown,
			"Sprint", Input::Action::Sprint
		);

		state.new_enum("APIType",
			"OpenGL", APIType::OpenGL
		);

		state.new_enum("InputKey",
			"W", Input::InputKey::W,
			"A", Input::InputKey::A,
			"S", Input::InputKey::S,
			"D", Input::InputKey::D,

			"R", Input::InputKey::R,
			"F", Input::InputKey::F,
			"T", Input::InputKey::T,
			"G", Input::InputKey::G,
			"Y", Input::InputKey::Y,
			"H", Input::InputKey::H,

			"E", Input::InputKey::E,
			"Q", Input::InputKey::Q,
			"O", Input::InputKey::O,
			"L", Input::InputKey::L,
			"SPACEBAR", Input::InputKey::SPACEBAR
		);


		state.new_usertype<Core>("Core",
			"create",
			&ME::Core::create,
			"InitRenderer",
			&ME::Core::InitRenderer,
			"StartFrame",
			&ME::Core::StartFrame,
			"EndFrame",
			&ME::Core::EndFrame,
			"DeltaTime",
			&ME::Core::DeltaTime,
			"IsWindowOpen",
			&ME::Core::IsWindowOpen,
			"CleanUp",
			&ME::Core::CleanUp
		);
		state.new_usertype<Input>("Input",
			sol::constructors<Input(Window&)>(),
			"BindKey",
			&Input::BindRuntimeKey,
			"ClearBindings",
			&Input::ClearRuntimeBindings,
			"checkActionPressed",
			&Input::checkRuntimeActionPressed,
			"checkActionJustPressed",
			&Input::checkRuntimeActionJustPressed,
			"checkActionJustReleased",
			&Input::checkRuntimeActionJustReleased
		);

		
		if (global_input_ != nullptr) {
			state["InputSystem"] = global_input_;
		}

		state.new_usertype<ECS>("ECS",
			"AddEntity",
			&ECS::AddEntity,
			"RemoveEntity",
			&ECS::RemoveEntity,
			//"RemoveComponent",
			//&ECS::RemoveComponent,

			// --- SETTERS

			"AddTransformComp", [](unsigned long selfid) -> ME::TransformComponent& {
				return ME::ECS::AddComponent<ME::TransformComponent>(selfid);
			},

			"AddScriptComp", [](unsigned long selfid) -> ME::ScriptComponent& {
				return ME::ECS::AddComponent<ME::ScriptComponent>(selfid, selfid);
			},

			"AddRenderComp", [](unsigned long selfid, std::vector<Vec3> v, size_t vn, std::vector<unsigned int> i, 
				size_t in, Vec3 c, std::string path) -> ME::RenderizableComponent& {
				return ME::ECS::AddComponent<ME::RenderizableComponent>(selfid, v.data(), v.size(), i.data(), i.size(), c, path);
			},

			"AddLightComp", [](unsigned long selfid) -> ME::LightComponent& {
				return ME::ECS::AddComponent<ME::LightComponent>(selfid);
			},

			"AddCameraComp", [](unsigned long selfid) -> ME::CameraComponent& {
				return ME::ECS::AddComponent<ME::CameraComponent>(selfid);
			},

			// --- GETTERS

			"GetTransformComp", [](unsigned long selfid) -> ME::TransformComponent* {
				return ME::ECS::GetComponent<ME::TransformComponent>(selfid);
			},

			"GetRenderComp", [](unsigned long selfid) -> ME::RenderizableComponent* {
				return ME::ECS::GetComponent<ME::RenderizableComponent>(selfid);
			},

			"GetScriptComp", [](unsigned long selfid) -> ME::ScriptComponent* {
				return ME::ECS::GetComponent<ME::ScriptComponent>(selfid);
			},

			"GetLightComp", [](unsigned long selfid) -> ME::LightComponent* {
				return ME::ECS::GetComponent<ME::LightComponent>(selfid);
			},

			"GetCameraComp", [](unsigned long selfid) -> ME::CameraComponent* {
				return ME::ECS::GetComponent<ME::CameraComponent>(selfid);
			},

			"GetEntityByTag", [](std::string tag) -> unsigned long {
				return ME::ECS::GetEntityByTag(tag);
			}

		);
	};

	void ScriptComponent::launchOnStart(float dt) {
		if (!enabled) return;
		sol::protected_function ff = state["OnStart"];
		if (ff.valid()) {
			auto result = ff(dt, this->id_);
			if (!result.valid()) {
				sol::error err = result;
				std::cerr << "Error: " << err.what() << std::endl;
			}
		}
	}

	void ScriptComponent::launchOnFrame(float dt) {
		if (!enabled) return;
		sol::protected_function ff = state["OnFrame"];
		if (ff.valid()) {
			auto result = ff(dt, this->id_);
			if (!result.valid()) {
				sol::error err = result;
				std::cerr << "Error: " << err.what() << std::endl;
			}
		}
	}

	void ScriptComponent::setCodeByPath(std::string codePath)
	{
		std::ifstream archivo(codePath);

		if (!archivo.is_open()) {
			return;
		}

		code_path = codePath;
		std::stringstream buffer;
		buffer << archivo.rdbuf();
		code_ = std::make_shared<std::string>(buffer.str()); // seteamos el code del interior en el code del script compomnent
		sol::load_result chunk = state.load(*code_);
		if (!chunk.valid()) printf("Compiling errors on script\n");
		assert(chunk.valid());
		chunk();
		enabled = true;
	}
	void ScriptComponent::SetInput(Input* input)
	{
		state["InputSystem"] = input;
	}
}

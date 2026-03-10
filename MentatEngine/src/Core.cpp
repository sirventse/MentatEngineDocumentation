#include <../include/MentatEngine/Core.hpp>
#include <../include/MentatEngine/Vulkan/VulkanAPI.hpp>
#include <../include/MentatEngine/OpenGL/OpenGLAPI.hpp>
#include <MentatEngine/XMLManager.hpp>
#include <MentatEngine/ScriptComponent.hpp>
#include <MentatEngine/CameraComponent.hpp>
#include <MentatEngine/LightComponent.hpp>
#include <MentatEngine/OpenGL/RenderizableComponent.hpp>

namespace ME {
    Core& Core::operator=(Core&& other){
        window_ = std::move(other.window_);
        api_type_ = std::move(other.api_type_);
        api_ = std::move(other.api_);
        return *this;
    }

    bool Core::InScenesPage = false;
    std::string Core::CurrentSceneName = "";

    std::optional<Core> Core::create(int width, int height, const char* title, ME::APIType api) {
        std::unique_ptr<GraphicAPI> r;

        if (api == APIType::OpenGL) {
            r = std::make_unique<OpenGLAPI>();
        }
        else {
            r = std::make_unique<VulkanAPI>();
        }

        std::optional<Window> w = Window::create(width, height, title, api);
        if (!w) return std::nullopt;

        return std::make_optional(Core(std::move(w.value()), api, std::move(r)));
    }

    void Core::InitRenderer(bool useImGui) {
        if (api_) {
            api_->Init(window_.getGLFWwindow(), useImGui);
        }
    }

    bool Core::IsWindowOpen() const{
        return window_.isWindowOpen();
    }

    Window& Core::window() {
        return window_;
    }
    
    void Core::StartFrame(){
        window_.pollEvents();

        currentFrame_ = (float) glfwGetTime();
        deltaTime_ = currentFrame_ - lastFrame_;
        lastFrame_ = currentFrame_;
        /*
            if (si ha pasado 1frame desde el ultimo) renderer->ExecuteOnframes();
            if (si ha pasado 4frame desde el ultimo)renderer->ExecutePhysics();
            if (si ha pasado 1frame desde el ultimo)renderer->ExecuteRender();
            if (si ha pasado 10frame desde el ultimo)renderer->ExecuteAI();
        */

    }

    void Core::EndFrame(){
        if (api_) {
            api_->Draw(deltaTime_);
        }
		window_.swapBuffers();

        ME::XMLManager::UpdatePendingResources();
    }

    std::unique_ptr<GraphicAPI>& Core::GetGraphicAPI() {
        return api_;
    }

    APIType Core::GetGraphicAPIType() {
        return api_type_;
    }

    void Core::CleanUp() {
        if (api_) {
            api_->Cleanup();
        }
    }

    Core::~Core() { }

    float Core::LastFrame() const {
        return lastFrame_;
    }

    float Core::DeltaTime() const {
        return deltaTime_;
    }

    float Core::CurrentFrame() const {
        return currentFrame_;
    }
}
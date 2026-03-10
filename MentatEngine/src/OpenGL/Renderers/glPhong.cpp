#include <GL/glew.h>  
#include <iostream>  
#include "../Include/MentatEngine/ImGuiManager.hpp"
#include <../include/MentatEngine/OpenGL/OpenGLAPI.hpp>
#include <../include/MentatEngine/OpenGL/RenderizableComponent.hpp>
#include <../include/MentatEngine/TransformComponent.hpp>
#include <../include/MentatEngine/LightComponent.hpp>
#include <../include/MentatEngine/ScriptComponent.hpp>
#include <../Include/MentatEngine/CameraComponent.hpp>
#include <../../include/MentatEngine/OpenGL/Renderers/glPhong.hpp>

namespace ME {
    // Helper local: pide la location de un uniform en el programa.
    // Devuelve -1 si no existe/no está activo (por ejemplo optimizado por el compilador).
    static GLint getUniformValue(GLuint prog, const char* name) {
        GLint loc = glGetUniformLocation(prog, name);
        return loc;
    }
	glPhong::glPhong()
		: shader_(Shader("../../deps/shaders/phong/phong.vert","../../deps/shaders/phong/phong.frag")), program_(Program()) {
		
        program_.CreateProgram();
        program_.AttachShader(&shader_);
        program_.LinkProgram();
		shader_.CleanShader();

        GLuint p = program_.GetProgram();
        GLint linked = 0;
        glGetProgramiv(p, GL_LINK_STATUS, &linked);
        if (!linked) {
            uniforms_.initialized = false; // si no linka el program no seguimos
            return;
        }


        glUseProgram(p);

        uniforms_.uModel = getUniformValue(p, "uModel");
        uniforms_.uView = getUniformValue(p, "uView");
        uniforms_.uProj = getUniformValue(p, "uProj");

        uniforms_.uCameraPos = getUniformValue(p, "uCameraPos");

        uniforms_.uMatDiffuse = getUniformValue(p, "uMatDiffuse");
        uniforms_.uMatSpecular = getUniformValue(p, "uMatSpecular");
        uniforms_.uMatShininess = getUniformValue(p, "uMatShininess");

        uniforms_.uAmbientColor = getUniformValue(p, "uAmbientColor");
        uniforms_.uAmbientStrength = getUniformValue(p, "uAmbientStrength");

        uniforms_.l_type = getUniformValue(p, "uLight.type");
        uniforms_.l_enabled = getUniformValue(p, "uLight.is_enabled");
        uniforms_.l_pos = getUniformValue(p, "uLight.pos");
        uniforms_.l_dir = getUniformValue(p, "uLight.dir");
        uniforms_.l_diff = getUniformValue(p, "uLight.diff_color");
        uniforms_.l_spec = getUniformValue(p, "uLight.spec_color");
        uniforms_.l_specIntensity = getUniformValue(p, "uLight.spec_intensity");
        uniforms_.l_cAtt = getUniformValue(p, "uLight.constant_att");
        uniforms_.l_lAtt = getUniformValue(p, "uLight.linear_att");
        uniforms_.l_qAtt = getUniformValue(p, "uLight.quad_att");
        uniforms_.l_cut = getUniformValue(p, "uLight.cut_off");
        uniforms_.l_outerCut = getUniformValue(p, "uLight.outer_cut_off");

        uniforms_.initialized = true;
	}

    Mat4 glPhong::MulM4(const Mat4& A, const Mat4& B) {
        Mat4 out;
        for (int i = 0; i < 16; ++i) out.M_[i] = 0.0f;

        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                out.M_[col * 4 + row] =
                    A.M_[0 * 4 + row] * B.M_[col * 4 + 0] +
                    A.M_[1 * 4 + row] * B.M_[col * 4 + 1] +
                    A.M_[2 * 4 + row] * B.M_[col * 4 + 2] +
                    A.M_[3 * 4 + row] * B.M_[col * 4 + 3];
            }
        }
        return out;
    }

    void glPhong::UpdateTransformRecursive(unsigned long id)
    {
        auto* t = ME::ECS::GetComponent<ME::TransformComponent>(id);
        if (!t) return;

        if (t->GetParent() == 0)
        {
            t->SetWorldMatrix(t->LocalMatrix());
        }
        else
        {
            auto* parent = ME::ECS::GetComponent<ME::TransformComponent>(t->GetParent());
            if (parent)
                t->SetWorldMatrix(MulM4(parent->WorldMatrix(), t->LocalMatrix()));
            else
                t->SetWorldMatrix(t->LocalMatrix());
        }

        for (unsigned long child : ME::ECS::GetChildren(id)) {
            UpdateTransformRecursive(child);
        }
    }

    void glPhong::Render(float dt) {
		if (!uniforms_.initialized) return; // si no estan los uniforms, no seguimos
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (ME::ECS::is_dirty) ME::ECS::SortLists();
        
        // Update Transforms in recursive before painting
        {
            unsigned int current = 0;
            ME::Iterator<ME::TransformComponent> tc_it, tc_end;
            ME::TransformComponent* tc = nullptr;
            SetIterators(&tc_it, &tc_end, &tc);

            while(tc_it != tc_end) {
                if (tc->GetParent() == 0) {
                    UpdateTransformRecursive(current);
                }
                CheckIteratorsFinished(&tc_it, &tc_end, &tc, current);
                current++;
            }
        }

        // iteradores como ya haces...
        // ren_it/tc_it/lc_it/cam_it ...

        // 1) Cámara
        Mat4 view, proj; view.identity(); proj.identity();
        Vec3 cameraPos(0.0f, 1.0f, 5.0f);

        bool has_camera = false;
        {
            ME::Iterator<ME::CameraComponent> cam_it, cam_end;
            ME::CameraComponent* cam = nullptr;
            SetIterators<ME::CameraComponent>(&cam_it, &cam_end, &cam);
            if (cam_it != cam_end && !cam_it.IsEmpty() && cam && cam->active) {
                view = cam->view;
                proj = cam->proj;
                cameraPos = cam->position;
                has_camera = true;
            }
        }
        if (!has_camera) {
            // tu fallback actual, pero también deja cameraPos consistente
        }

        // 2) Cache de luces (Transform + Light)
        std::vector<std::pair<ME::TransformComponent, ME::LightComponent>> lightCache;
        {
            ME::Iterator<ME::TransformComponent> tc_it, tc_end;
            ME::Iterator<ME::LightComponent> lc_it, lc_end;
            ME::TransformComponent* tc = nullptr;
            ME::LightComponent* lc = nullptr;

            SetIterators<ME::TransformComponent>(&tc_it, &tc_end, &tc);
            SetIterators<ME::LightComponent>(&lc_it, &lc_end, &lc);

            unsigned int current = 0;
            while (tc_it != tc_end || lc_it != lc_end) {
                if (lc_it != lc_end && lc_it.owner() == current && !lc_it.IsEmpty()) {
                    // copia componentes
                    auto tcc = *tc;
                    auto lcc = *lc;

                    // recomendado: que la luz use el transform para posición
                    Vec3 wp = tcc.Position(); // si tu TransformComponent tiene position pública
                    lcc.SetPos(wp);

                    // dirección: depende de tu TransformComponent, si no tienes forward, deja lcc.Dir() manual.
                    lightCache.push_back({ tcc, lcc });
                }

                CheckIteratorsFinished(&tc_it, &tc_end, &tc, current);
                CheckIteratorsFinished(&lc_it, &lc_end, &lc, current);
                current++;
            }
        }

        // si no hay luces, fuerza 1 “pass” ambiente (para no renderizar negro)
        if (lightCache.empty()) {
            ME::LightComponent dummy;
            dummy.SetEnabled(false);
            dummy.SetType(ME::LightType::DIRECTIONAL);
            lightCache.push_back({ ME::TransformComponent{}, dummy });
        }

        GLuint p = program_.GetProgram();
        glUseProgram(p);

        // view/proj/camera una vez por pass o por frame
        glUniformMatrix4fv(uniforms_.uView, 1, GL_FALSE, view.M_);
        glUniformMatrix4fv(uniforms_.uProj, 1, GL_FALSE, proj.M_);
        glUniform3f(uniforms_.uCameraPos, cameraPos.x_, cameraPos.y_, cameraPos.z_);

        // material fijo por ahora (si no tienes MaterialComponent)
        glUniform3f(uniforms_.uMatSpecular, 1.0f, 1.0f, 1.0f);
        glUniform1f(uniforms_.uMatShininess, 32.0f);

        // ambiente global (ajústalo luego con ImGui si quieres)
        glUniform3f(uniforms_.uAmbientColor, 1.0f, 1.0f, 1.0f);

        GLuint lastVAO = 0; // si el VAO no cambia, no volvemos a bindear
        GLuint lastIndexCount = 0;
        Vec3 lastDiffuse = { -9999.0f, -9999.0f, -9999.0f }; // si el color no cambia, no uMatDiffusse

        bool first = true;
        for (auto& [ltc, llc] : lightCache) {

            if (first) {
                glDisable(GL_BLEND);
                glDepthMask(GL_TRUE);
                glDepthFunc(GL_LESS);
                glUniform1f(uniforms_.uAmbientStrength, 0.08f);
            }
            else {
                bool additiveStateSet = false;
                if (!additiveStateSet) { // buscamos no repetir blendfunc
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE, GL_ONE);
                    additiveStateSet = true;
                }
                glDepthMask(GL_FALSE);
                glDepthFunc(GL_EQUAL);
                glUniform1f(uniforms_.uAmbientStrength, 0.0f);
            }

            // sube uLight.*
            glUniform1i(uniforms_.l_type, (int)llc.Type());
            glUniform1i(uniforms_.l_enabled, llc.Enabled() ? 1 : 0);

            Vec3 lp = llc.Pos();
            Vec3 ld = llc.Dir();
            Vec3 dc = llc.DiffColor();
            Vec3 sc = llc.SpecColor();

            glUniform3f(uniforms_.l_pos, lp.x_, lp.y_, lp.z_);
            glUniform3f(uniforms_.l_dir, ld.x_, ld.y_, ld.z_);
            glUniform3f(uniforms_.l_diff, dc.x_, dc.y_, dc.z_);
            glUniform3f(uniforms_.l_spec, sc.x_, sc.y_, sc.z_);
                       
            glUniform1f(uniforms_.l_specIntensity, llc.SpecIntensity());
            glUniform1f(uniforms_.l_cAtt, llc.ConstantAtt());
            glUniform1f(uniforms_.l_lAtt, llc.LinearAtt());
            glUniform1f(uniforms_.l_qAtt, llc.QuadAtt());
            glUniform1f(uniforms_.l_cut, llc.CutOff());
            glUniform1f(uniforms_.l_outerCut, llc.OuterCutOff());

            // 3) Dibuja todos los renderizables (subiendo model + diffuse)
            ME::Iterator<ME::RenderizableComponent> ren_it, ren_end;
            ME::Iterator<ME::TransformComponent> tc_it, tc_end;
            ME::RenderizableComponent* ren = nullptr;
            ME::TransformComponent* tc = nullptr;
            SetIterators<ME::RenderizableComponent>(&ren_it, &ren_end, &ren);
            SetIterators<ME::TransformComponent>(&tc_it, &tc_end, &tc);

            unsigned int current = 0;
            while (ren_it != ren_end || tc_it != tc_end) {

                if (tc_it != tc_end && tc_it.owner() == current && !tc_it.IsEmpty()) {
                    Mat4 model = tc->WorldMatrix();
                    glUniformMatrix4fv(uniforms_.uModel, 1, GL_FALSE, model.M_);
                }

                if (ren_it != ren_end && ren_it.owner() == current && !ren_it.IsEmpty()) {
                    if (ren->Visible()) {
                        Vec3 c = ren->GetColor();
                        if (c.x_ != lastDiffuse.x_ || c.y_ != lastDiffuse.y_ || c.z_ != lastDiffuse.z_) {
                            glUniform3f(uniforms_.uMatDiffuse, c.x_, c.y_, c.z_); // solo si cambia
                            lastDiffuse = c;
                        }

                        GLuint vao = ren->GetVAO();
                        GLuint idxCount = ren->GetIndexCount();

                        if (vao != lastVAO) {
                            glBindVertexArray(vao);// solo si cambia
                            lastVAO = vao;
                            lastIndexCount = 0; // invalida por si quieres cachear idxCount también
                        }

                        if (idxCount != lastIndexCount) {
                            lastIndexCount = idxCount;
                        }

                        glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, 0);
                    }
                }

                CheckIteratorsFinished(&tc_it, &tc_end, &tc, current);
                CheckIteratorsFinished(&ren_it, &ren_end, &ren, current);
                current++;
            }
            first = false;
        }

        // SCRIPTING START/UPDATE
        if (ImGuiManager::core_is_playing_ && !ImGuiManager::core_is_paused_) {
            ME::Iterator<ME::ScriptComponent> sc_it, sc_end;
            ME::ScriptComponent* scomp;
            SetIterators(&sc_it, &sc_end, &scomp);
            int current = 0;
            while (sc_it != sc_end) {
                if (sc_it != sc_end && sc_it.owner() == current && !sc_it.IsEmpty()) {
                    if (!ImGuiManager::core_is_start_done_) {
                        scomp->launchOnStart(dt);
                    }
                    else {
                        scomp->launchOnFrame(dt);
                    }
                }

                CheckIteratorsFinished(&sc_it, &sc_end, &scomp, current);
                current++;
            }

            if (!ImGuiManager::core_is_start_done_) {
                ImGuiManager::core_is_start_done_ = true;
            }
        }

        // restore state
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    Mat4 glPhong::MakeLookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
    {
        Vec3 f = center;
        f.substract(eye);
        f.normalize();

        Vec3 s(
            f.y_ * up.z_ - f.z_ * up.y_,
            f.z_ * up.x_ - f.x_ * up.z_,
            f.x_ * up.y_ - f.y_ * up.x_
        );
        s.normalize();

        Vec3 u(
            s.y_ * f.z_ - s.z_ * f.y_,
            s.z_ * f.x_ - s.x_ * f.z_,
            s.x_ * f.y_ - s.y_ * f.x_
        );
        u.normalize();

        Mat4 m;
        m.identity();

        // Column-major layout (como glm, OpenGL est�ndar)
        m.M_[0] = s.x_;
        m.M_[4] = s.y_;
        m.M_[8] = s.z_;
        m.M_[12] = -(s.x_ * eye.x_ + s.y_ * eye.y_ + s.z_ * eye.z_);

        m.M_[1] = u.x_;
        m.M_[5] = u.y_;
        m.M_[9] = u.z_;
        m.M_[13] = -(u.x_ * eye.x_ + u.y_ * eye.y_ + u.z_ * eye.z_);

        m.M_[2] = -f.x_;
        m.M_[6] = -f.y_;
        m.M_[10] = -f.z_;
        m.M_[14] = (f.x_ * eye.x_ + f.y_ * eye.y_ + f.z_ * eye.z_);

        m.M_[3] = 0.0f;
        m.M_[7] = 0.0f;
        m.M_[11] = 0.0f;
        m.M_[15] = 1.0f;

        return m;
    }

    Mat4 glPhong::MakePerspective(float fovRadians, float aspect, float znear, float zfar)
    {
        float tanHalfFov = tanf(fovRadians * 0.5f);

        Mat4 m;
        m.identity();

        for (int i = 0; i < 16; ++i) m.M_[i] = 0.0f;

        m.M_[0] = 1.0f / (aspect * tanHalfFov);
        m.M_[5] = 1.0f / (tanHalfFov);
        m.M_[10] = -(zfar + znear) / (zfar - znear);
        m.M_[11] = -1.0f;
        m.M_[14] = -(2.0f * zfar * znear) / (zfar - znear);
        m.M_[15] = 0.0f;

        return m;
    }

	Program* glPhong::GetProgram() {
		return &program_;
	}

	Shader* glPhong::GetShader() {
		return &shader_;
	}

    void glPhong::GetError(const char* type) {
        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR)
        {
            std::string error;
            switch (errorCode)
            {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            }
            std::cout << type << ": " << error << std::endl;
        }
    }
}
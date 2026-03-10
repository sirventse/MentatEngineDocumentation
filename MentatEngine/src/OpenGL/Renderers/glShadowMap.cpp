#include <GL/glew.h>
#include <iostream>
#include <cmath>

#include "../Include/MentatEngine/ImGuiManager.hpp"
#include <../include/MentatEngine/OpenGL/OpenGLAPI.hpp>
#include <../include/MentatEngine/OpenGL/RenderizableComponent.hpp>
#include <../include/MentatEngine/TransformComponent.hpp>
#include <../include/MentatEngine/LightComponent.hpp>
#include <../Include/MentatEngine/ScriptComponent.hpp>
#include <../Include/MentatEngine/CameraComponent.hpp>

#include <../../include/MentatEngine/OpenGL/Renderers/glShadowMap.hpp>

namespace ME {

    static Mat4 MulMat4(const Mat4& A, const Mat4& B)
    {
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

    struct ActiveShadowLight {
        bool valid = false;
        ME::LightType type = ME::LightType::DIRECTIONAL;
        Vec3 pos;
        Vec3 dir;
        Vec3 color;
        float cutOffDeg = 0.0f;
        float outerCutOffDeg = 0.0f;
    };

    

    glShadowMap::glShadowMap()
        : shadow_shader_ (Shader("../../deps/shaders/shadowmap/shadowpass.vert","../../deps/shaders/shadowmap/shadowpass.frag")),
        light_shader_ (Shader("../../deps/shaders/shadowmap/lightpass.vert","../../deps/shaders/shadowmap/lightpass.frag")),
		point_shadow_shader_(
            Shader(
                "../../deps/shaders/shadowmap/point_shadow.vert",
                "../../deps/shaders/shadowmap/point_shadow.frag",
                "../../deps/shaders/shadowmap/point_shadow.geom"
            ))
    {
        // INICIALIZAR SHADOW PASS
        InitializeShadowPass();
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        // INICIALIZAR LIGHT PASS
        InitializeLightPass();
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        // INICIALIZAR DEBUG PASS PARA VER EL PINTADO SOBRE LA TEXTURA DE SHADOW
        InitializeDebugPass();
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        InitializePointShadowPass();
    }

    glShadowMap::~glShadowMap()
    {
        if (shadow_texture_ != 0) glDeleteTextures(1, &shadow_texture_);
        if (shadow_fbo_ != 0) glDeleteFramebuffers(1, &shadow_fbo_);
        if (debugVBO_ != 0) glDeleteBuffers(1, &debugVBO_);
        if (debugVAO_ != 0) glDeleteVertexArrays(1, &debugVAO_);
        if (point_shadow_cubemap_ != 0) glDeleteTextures(1, &point_shadow_cubemap_);
        if (point_shadow_fbo_ != 0) glDeleteFramebuffers(1, &point_shadow_fbo_);

        shadow_texture_ = 0;
        shadow_fbo_ = 0;
        debugVBO_ = 0;
        debugVAO_ = 0;
        point_shadow_cubemap_ = 0;
        point_shadow_fbo_ = 0;
    }

    Mat4 glShadowMap::MakeLookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
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
        for (int i = 0; i < 16; ++i) m.M_[i] = 0.0f;
        m.M_[15] = 1.0f;

        m.M_[0] = s.x_;  m.M_[4] = s.y_;  m.M_[8] = s.z_;
        m.M_[12] = -(s.x_ * eye.x_ + s.y_ * eye.y_ + s.z_ * eye.z_);

        m.M_[1] = u.x_;  m.M_[5] = u.y_;  m.M_[9] = u.z_;
        m.M_[13] = -(u.x_ * eye.x_ + u.y_ * eye.y_ + u.z_ * eye.z_);

        m.M_[2] = -f.x_; m.M_[6] = -f.y_; m.M_[10] = -f.z_;
        m.M_[14] = (f.x_ * eye.x_ + f.y_ * eye.y_ + f.z_ * eye.z_);

        return m;
    }

    Mat4 glShadowMap::MakePerspective(float fovRad, float aspect, float znear, float zfar)
    {
        const float f = 1.0f / tanf(fovRad * 0.5f);

        Mat4 m;
        for (int i = 0; i < 16; ++i) m.M_[i] = 0.0f;

        m.M_[0] = f / aspect;
        m.M_[5] = f;
        m.M_[10] = (zfar + znear) / (znear - zfar);
        m.M_[11] = -1.0f;
        m.M_[14] = (2.0f * zfar * znear) / (znear - zfar);

        return m;
    }

    Mat4 glShadowMap::MakeOrtho(float l, float r, float b, float t, float n, float f)
    {
        Mat4 m;
        for (int i = 0; i < 16; ++i) m.M_[i] = 0.0f;

        m.M_[0] = 2.0f / (r - l);
        m.M_[5] = 2.0f / (t - b);
        m.M_[10] = -2.0f / (f - n);
        m.M_[12] = -(r + l) / (r - l);
        m.M_[13] = -(t + b) / (t - b);
        m.M_[14] = -(f + n) / (f - n);
        m.M_[15] = 1.0f;

        return m;
    }

    Mat4 glShadowMap::MulM4(const Mat4& A, const Mat4& B) {
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
    
    void glShadowMap::InitializeShadowPass()
    {
        shadow_pass_.CreateProgram();
        shadow_pass_.AttachShader(&shadow_shader_);
        shadow_pass_.LinkProgram();
        shadow_shader_.CleanShader();
        GLuint sp = shadow_pass_.GetProgram();
        shadow_u_.uModel = glGetUniformLocation(sp, "uModel");
        shadow_u_.uLightViewProj = glGetUniformLocation(sp, "uLightViewProj");
    }

    void glShadowMap::InitializeLightPass()
    {
        light_pass_.CreateProgram();
        light_pass_.AttachShader(&light_shader_);
        light_pass_.LinkProgram();
        light_shader_.CleanShader();
        GLuint lp = light_pass_.GetProgram();
        light_u_.uModel = glGetUniformLocation(lp, "uModel");
        light_u_.uView = glGetUniformLocation(lp, "uView");
        light_u_.uProj = glGetUniformLocation(lp, "uProj");

        light_u_.uLightPos = glGetUniformLocation(lp, "uLightPos");
        light_u_.uLightDir = glGetUniformLocation(lp, "uLightDir");
        light_u_.uLightColor = glGetUniformLocation(lp, "uLightColor");
        light_u_.uCameraPos = glGetUniformLocation(lp, "uCameraPos");

        light_u_.uLightType = glGetUniformLocation(lp, "uLightType");
        light_u_.uCastsShadow = glGetUniformLocation(lp, "uCastsShadow");

        light_u_.uConstAtt = glGetUniformLocation(lp, "uConstAtt");
        light_u_.uLinearAtt = glGetUniformLocation(lp, "uLinearAtt");
        light_u_.uQuadAtt = glGetUniformLocation(lp, "uQuadAtt");

        light_u_.uCutOff = glGetUniformLocation(lp, "uCutOff");
        light_u_.uOuterCutOff = glGetUniformLocation(lp, "uOuterCutOff");

        light_u_.uLightSpaceMatrix = glGetUniformLocation(lp, "uLightSpaceMatrix");
        light_u_.uShadowMap = glGetUniformLocation(lp, "uShadowMap");

        light_u_.uObjectColor = glGetUniformLocation(lp, "uObjectColor");
        light_u_.uUseAmbient = glGetUniformLocation(lp, "uUseAmbient");
        light_u_.uAmbientColor = glGetUniformLocation(lp, "uAmbientColor");

        light_u_.uPointShadowMap = glGetUniformLocation(lp, "uPointShadowMap");
        light_u_.uPointShadowFarPlane = glGetUniformLocation(lp, "uPointShadowFarPlane");

        glUseProgram(lp);
        if (light_u_.uShadowMap >= 0) glUniform1i(light_u_.uShadowMap, 1);
        if (light_u_.uPointShadowMap >= 0) glUniform1i(light_u_.uPointShadowMap, 3);

        uViewProj_ = glGetUniformLocation(lp, "uViewProj_"); //  // Proj * View de la cámara
        CreateShadowBuffer();
        CreatePointShadowBuffer();

    }

    void glShadowMap::InitializeDebugPass()
    {
        debug_shader_ =
            Shader("../../deps/shaders/shadowmap/debug_depth.vert",
                "../../deps/shaders/shadowmap/debug_depth.frag");
        debug_pass_.CreateProgram();
        debug_pass_.AttachShader(&debug_shader_);
        debug_pass_.LinkProgram();
        debug_shader_.CleanShader();
        GLuint dp = debug_pass_.GetProgram();
        uDepth_ = glGetUniformLocation(dp, "uDepth");
        glUseProgram(dp);
        if (uDepth_ >= 0) glUniform1i(uDepth_, 2);
        float quad[] =
        {
          -1.0f, -1.0f,  0.0f, 0.0f,
          -0.4f, -1.0f,  1.0f, 0.0f,
          -0.4f, -0.4f,  1.0f, 1.0f,

          -1.0f, -1.0f,  0.0f, 0.0f,
          -0.4f, -0.4f,  1.0f, 1.0f,
          -1.0f, -0.4f,  0.0f, 1.0f
        };
        glGenVertexArrays(1, &debugVAO_);
        glGenBuffers(1, &debugVBO_);
        glBindVertexArray(debugVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, debugVBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void glShadowMap::InitializePointShadowPass()
    {
        point_shadow_pass_.CreateProgram();
        point_shadow_pass_.AttachShader(&point_shadow_shader_);
        point_shadow_pass_.LinkProgram();
        point_shadow_shader_.CleanShader();

        GLuint pp = point_shadow_pass_.GetProgram();

        point_shadow_u_.uModel = glGetUniformLocation(pp, "uModel");
        point_shadow_u_.uLightPos = glGetUniformLocation(pp, "uLightPos");
        point_shadow_u_.uFarPlane = glGetUniformLocation(pp, "uFarPlane");

        for (int i = 0; i < 6; ++i) {
            std::string name = "uShadowMatrices[" + std::to_string(i) + "]";
            point_shadow_u_.uShadowMatrices[i] = glGetUniformLocation(pp, name.c_str());
        }
    }

    void glShadowMap::CreateShadowBuffer()
    {
        glGenFramebuffers(1, &shadow_fbo_); // frame buffer object de sombra sobre la que pintar
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_); // bindeo del frame buffer object

        glGenTextures(1, &shadow_texture_); // textura de sombra sobre la que pintar
        glBindTexture(GL_TEXTURE_2D, shadow_texture_); // bindeo de textura

        glTexImage2D(GL_TEXTURE_2D, // target
            0, // level
            GL_DEPTH_COMPONENT24, // internal format
            shadow_width_, shadow_height_, // shadow size
            0, // border
            GL_DEPTH_COMPONENT, // format
            GL_FLOAT, // type
            nullptr); // pixels

        // parametros de la tetxura, evitamos bordes de sombra a lo lejos
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // Para debug_depth.frag (leer depth crudo)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        float border[4] = { 1.f, 1.f, 1.f, 1.f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_texture_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { // si no esta bien configurado el FBO
            printf("Shadow FBO ERROR\n");
        }

        glBindTexture(GL_TEXTURE_2D, 0); // bindeamos la textura como frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // volvemos a bindear la pantalla como bind buffer, el 0 es por defecto
    }

    void glShadowMap::UpdateTransformRecursive(unsigned long id)
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

    void glShadowMap::Render(float dt)
    {
        if (ECS::is_dirty) ECS::SortLists();

        // Update transforms recursively before rendering
        {
            unsigned int current = 0;
            ME::Iterator<ME::TransformComponent> tc_it, tc_end;
            ME::TransformComponent* tc = nullptr;
            SetIterators(&tc_it, &tc_end, &tc);

            while (tc_it != tc_end) {
                if (tc && tc->GetParent() == 0) {
                    UpdateTransformRecursive(current);
                }
                CheckIteratorsFinished(&tc_it, &tc_end, &tc, current);
                current++;
            }
        }

        GLint prevViewport[4] = { 0,0,0,0 };
        glGetIntegerv(GL_VIEWPORT, prevViewport);

#pragma region Camera
        Mat4 camView, camProj;
        camView.identity();
        camProj.identity();
        Vec3 camPos(0.0f, 0.0f, 0.0f);

        {
            Iterator<CameraComponent> cIt, cEnd;
            CameraComponent* cam = nullptr;
            SetIterators(&cIt, &cEnd, &cam);

            if (cIt != cEnd && cam && cam->active) {
                camView = cam->view;
                camProj = cam->proj;
                camPos = cam->position;
            }
        }
#pragma endregion

#pragma region Lights init
        struct ActiveLight
        {
            ME::LightType type = ME::LightType::POINT;
            Vec3 pos{ 0.0f, 0.0f, 0.0f };
            Vec3 dir{ 0.0f, -1.0f, 0.0f };
            Vec3 color{ 1.0f, 1.0f, 1.0f };

            float constantAtt = 1.0f;
            float linearAtt = 0.09f;
            float quadAtt = 0.032f;

            float cutOffDeg = 12.5f;
            float outerCutOffDeg = 15.0f;

            bool castsShadow = false;
            float shadowNear = 0.1f;
            float shadowFar = 200.0f;
        };

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
                if (lc_it != lc_end && lc && lc_it.owner() == current && !lc_it.IsEmpty()) {
                    ME::TransformComponent tcc{};
                    if (tc_it != tc_end && tc && tc_it.owner() == current && !tc_it.IsEmpty()) {
                        tcc = *tc;
                    }

                    ME::LightComponent lcc = *lc;

                    // Si la entidad tiene transform, usamos su posición mundial
                    Vec3 wp = tcc.Position();
                    lcc.SetPos(wp);

                    lightCache.push_back({ tcc, lcc });
                }

                CheckIteratorsFinished(&tc_it, &tc_end, &tc, current);
                CheckIteratorsFinished(&lc_it, &lc_end, &lc, current);
                current++;
            }
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

        std::vector<ActiveLight> activeLights;
        Vec3 ambientColor(0.0f, 0.0f, 0.0f);
        for (const auto& [tcc, lcc] : lightCache)
        {
            if (!lcc.Enabled()) continue;

			if (lcc.Type() == ME::LightType::AMBIENT) // si es ambient nos quedamos con el color, pero no añadimos a activeLights
            {
                Vec3 c = lcc.DiffColor();
                ambientColor.x_ += c.x_;
                ambientColor.y_ += c.y_;
                ambientColor.z_ += c.z_;
                continue;
            }

            ActiveLight light;
            light.type = lcc.Type();
            light.pos = lcc.Pos();
            light.dir = lcc.Dir();
            light.color = lcc.DiffColor();

            light.constantAtt = lcc.ConstantAtt();
            light.linearAtt = lcc.LinearAtt();
            light.quadAtt = lcc.QuadAtt();

            light.cutOffDeg = lcc.CutOff();
            light.outerCutOffDeg = lcc.OuterCutOff();

            light.castsShadow = lcc.CastsShadows();
            light.shadowNear = lcc.ShadowNear();
            light.shadowFar = lcc.ShadowFar();

            activeLights.push_back(light);
        }

        if (activeLights.empty())
        {
            ActiveLight fallback;
            fallback.type = ME::LightType::DIRECTIONAL;
            fallback.pos = Vec3(100.0f, 200.0f, 100.0f);
            fallback.dir = Vec3(-0.5f, -1.0f, -0.3f);
            fallback.color = Vec3(1.0f, 1.0f, 1.0f);
            fallback.cutOffDeg = 12.5f;
            fallback.outerCutOffDeg = 15.0f;
            fallback.castsShadow = true;
            fallback.shadowNear = 1.0f;
            fallback.shadowFar = 2000.0f;
            ambientColor = Vec3(0.05f, 0.05f, 0.05f); // color ambiente si no hay ninguna luz
            activeLights.push_back(fallback);
        }
#pragma endregion

        // =====================================================================================
        // LOOP POR LUCES
        // =====================================================================================
        for (size_t i = 0; i < activeLights.size(); ++i)
        {
            const ActiveLight& activeLight = activeLights[i];
            std::vector<Mat4> pointShadowTransforms;

            Vec3 lightPos(100.0f, 200.0f, 100.0f);
            Vec3 lightTarget(0.0f, 0.0f, 0.0f);
            Mat4 lightProj;
            Mat4 lightView;
            Mat4 lightViewProj;
            lightProj.identity();
            lightView.identity();
            lightViewProj.identity();

            bool hasLightMatrix = false;

            // ============================
            // Build light camera
            // ============================
            if (activeLight.type == ME::LightType::DIRECTIONAL)
            {
                Vec3 dir = activeLight.dir;
                dir.normalize();

                float shadowDistance = 150.0f;

                lightPos = Vec3(
                    -dir.x_ * shadowDistance,
                    -dir.y_ * shadowDistance,
                    -dir.z_ * shadowDistance
                );

                lightTarget = Vec3(0.0f, 0.0f, 0.0f);
                lightView = MakeLookAt(lightPos, lightTarget, Vec3(0.0f, 1.0f, 0.0f));
                lightProj = MakeOrtho(-80.0f, 80.0f, -80.0f, 80.0f, activeLight.shadowNear, activeLight.shadowFar);
                lightViewProj = MulMat4(lightProj, lightView);
                hasLightMatrix = true;
            }
            else if (activeLight.type == ME::LightType::SPOT)
            {
                Vec3 dir = activeLight.dir;
                dir.normalize();

                lightPos = activeLight.pos;
                lightTarget = Vec3(
                    lightPos.x_ + dir.x_,
                    lightPos.y_ + dir.y_,
                    lightPos.z_ + dir.z_
                );

                float fovDeg = activeLight.outerCutOffDeg * 2.0f;
                float fovRad = fovDeg * 3.14159265358979323846f / 180.0f;
                float aspect = static_cast<float>(shadow_width_) / static_cast<float>(shadow_height_);

                Vec3 up(0.0f, 1.0f, 0.0f);
                float dot = dir.x_ * up.x_ + dir.y_ * up.y_ + dir.z_ * up.z_;
                if (fabs(dot) > 0.99f) {
                    up = Vec3(0.0f, 0.0f, 1.0f);
                }

                lightView = MakeLookAt(lightPos, lightTarget, up);
                lightProj = MakePerspective(fovRad, aspect, activeLight.shadowNear, activeLight.shadowFar);
                lightViewProj = MulMat4(lightProj, lightView);
                hasLightMatrix = true;
            }
            else if (activeLight.type == ME::LightType::POINT)
            {
                // POINT: de momento sin sombras
                lightPos = activeLight.pos;

                if (activeLight.castsShadow)
                {
                    pointShadowTransforms = BuildPointShadowTransforms(
                        lightPos,
                        activeLight.shadowNear,
                        activeLight.shadowFar
                    );
                }
            }

            // ============================
            // Shadow pass (solo lights con shadow)
            // ============================
            if (activeLight.castsShadow) {
				if (activeLight.type == ME::LightType::POINT)
				{
					// shadow pass 3d
                    glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbo_);
                    glViewport(0, 0, point_shadow_size_, point_shadow_size_);

                    glDisable(GL_BLEND);
                    glDisable(GL_CULL_FACE);
                    glEnable(GL_DEPTH_TEST);
                    glDepthMask(GL_TRUE);
                    glDepthFunc(GL_LESS);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                    glClearDepth(1.0f);
                    glClear(GL_DEPTH_BUFFER_BIT);

                    glUseProgram(point_shadow_pass_.GetProgram());

                    for (int face = 0; face < 6; ++face)
                    {
                        if (point_shadow_u_.uShadowMatrices[face] >= 0)
                            glUniformMatrix4fv(point_shadow_u_.uShadowMatrices[face], 1, GL_FALSE, pointShadowTransforms[face].M_);
                    }

                    if (point_shadow_u_.uLightPos >= 0)
                        glUniform3f(point_shadow_u_.uLightPos, lightPos.x_, lightPos.y_, lightPos.z_);

                    if (point_shadow_u_.uFarPlane >= 0)
                        glUniform1f(point_shadow_u_.uFarPlane, activeLight.shadowFar);

                    Iterator<RenderizableComponent> r_it, r_end;
                    Iterator<TransformComponent> t_it, t_end;
                    RenderizableComponent* r = nullptr;
                    TransformComponent* t = nullptr;

                    SetIterators(&r_it, &r_end, &r);
                    SetIterators(&t_it, &t_end, &t);

                    unsigned e = 0;
                    while ((r_it != r_end && !r_it.IsEmpty()) || (t_it != t_end && !t_it.IsEmpty()))
                    {
                        Mat4 model;
                        model.identity();

                        if (t_it != t_end && !t_it.IsEmpty() && t && t_it.owner() == e)
                            model = t->WorldMatrix();

                        if (r_it != r_end && !r_it.IsEmpty() && r && r_it.owner() == e && r->Visible())
                        {
                            if (point_shadow_u_.uModel >= 0)
                                glUniformMatrix4fv(point_shadow_u_.uModel, 1, GL_FALSE, model.M_);

                            glBindVertexArray(r->GetVAO());
                            glDrawElements(GL_TRIANGLES, r->GetIndexCount(), GL_UNSIGNED_INT, 0);
                        }

                        CheckIteratorsFinished(&t_it, &t_end, &t, (int)e);
                        CheckIteratorsFinished(&r_it, &r_end, &r, (int)e);
                        ++e;
                    }

                    glBindVertexArray(0);
				}
				else if (activeLight.type == ME::LightType::DIRECTIONAL || activeLight.type == ME::LightType::SPOT)
				{
					// shadow pass 2d
                    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
                    glViewport(0, 0, shadow_width_, shadow_height_);

                    glDisable(GL_BLEND);
                    glDisable(GL_CULL_FACE);
                    glEnable(GL_DEPTH_TEST);
                    glDepthMask(GL_TRUE);
                    glDepthFunc(GL_LESS);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                    glClearDepth(1.0f);
                    glClear(GL_DEPTH_BUFFER_BIT);

                    glUseProgram(shadow_pass_.GetProgram());

                    if (shadow_u_.uLightViewProj >= 0) {
                        glUniformMatrix4fv(shadow_u_.uLightViewProj, 1, GL_FALSE, lightViewProj.M_);
                    }

                    Iterator<RenderizableComponent> r_it, r_end;
                    Iterator<TransformComponent> t_it, t_end;
                    RenderizableComponent* r = nullptr;
                    TransformComponent* t = nullptr;

                    SetIterators(&r_it, &r_end, &r);
                    SetIterators(&t_it, &t_end, &t);

                    unsigned e = 0;
                    while ((r_it != r_end && !r_it.IsEmpty()) || (t_it != t_end && !t_it.IsEmpty()))
                    {
                        Mat4 model;
                        model.identity();

                        if (t_it != t_end && !t_it.IsEmpty() && t && t_it.owner() == e)
                            model = t->WorldMatrix();

                        if (r_it != r_end && !r_it.IsEmpty() && r && r_it.owner() == e && r->Visible())
                        {
                            if (shadow_u_.uModel >= 0)
                                glUniformMatrix4fv(shadow_u_.uModel, 1, GL_FALSE, model.M_);

                            glBindVertexArray(r->GetVAO());
                            glDrawElements(GL_TRIANGLES, r->GetIndexCount(), GL_UNSIGNED_INT, 0);
                        }

                        CheckIteratorsFinished(&t_it, &t_end, &t, (int)e);
                        CheckIteratorsFinished(&r_it, &r_end, &r, (int)e);
                        ++e;
                    }

                    glBindVertexArray(0);
				}
            }

            // ============================
            // Light pass
            // ============================
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_DEPTH_TEST);

            if (i == 0)
            {
                glDisable(GL_BLEND);
                glDepthMask(GL_TRUE);
                glDepthFunc(GL_LESS);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            else
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);
                glDepthMask(GL_FALSE);
                glDepthFunc(GL_EQUAL);
            }

            glUseProgram(light_pass_.GetProgram());

            if (light_u_.uView >= 0) glUniformMatrix4fv(light_u_.uView, 1, GL_FALSE, camView.M_);
            if (light_u_.uProj >= 0) glUniformMatrix4fv(light_u_.uProj, 1, GL_FALSE, camProj.M_);

            if (uViewProj_ >= 0 && hasLightMatrix)
                glUniformMatrix4fv(uViewProj_, 1, GL_FALSE, lightViewProj.M_);

            if (light_u_.uLightSpaceMatrix >= 0 && hasLightMatrix)
                glUniformMatrix4fv(light_u_.uLightSpaceMatrix, 1, GL_FALSE, lightViewProj.M_);

            if (light_u_.uLightPos >= 0)
                glUniform3f(light_u_.uLightPos, lightPos.x_, lightPos.y_, lightPos.z_);

            if (light_u_.uLightDir >= 0)
                glUniform3f(light_u_.uLightDir, activeLight.dir.x_, activeLight.dir.y_, activeLight.dir.z_);

            if (light_u_.uLightColor >= 0)
                glUniform3f(light_u_.uLightColor, activeLight.color.x_, activeLight.color.y_, activeLight.color.z_);

            if (light_u_.uCameraPos >= 0)
                glUniform3f(light_u_.uCameraPos, camPos.x_, camPos.y_, camPos.z_);

            if (light_u_.uLightType >= 0)
                glUniform1i(light_u_.uLightType, (int)activeLight.type);

            if (light_u_.uCastsShadow >= 0)
                glUniform1i(light_u_.uCastsShadow, activeLight.castsShadow ? 1 : 0);

            if (light_u_.uUseAmbient >= 0)
                glUniform1i(light_u_.uUseAmbient, (i == 0) ? 1 : 0);

            if (light_u_.uAmbientColor >= 0)
                glUniform3f(light_u_.uAmbientColor, ambientColor.x_, ambientColor.y_, ambientColor.z_);

            if (light_u_.uConstAtt >= 0)
                glUniform1f(light_u_.uConstAtt, activeLight.constantAtt);

            if (light_u_.uLinearAtt >= 0)
                glUniform1f(light_u_.uLinearAtt, activeLight.linearAtt);

            if (light_u_.uQuadAtt >= 0)
                glUniform1f(light_u_.uQuadAtt, activeLight.quadAtt);

            if (light_u_.uPointShadowFarPlane >= 0)
                glUniform1f(light_u_.uPointShadowFarPlane, activeLight.shadowFar);

            // Guardamos el componente en grados, pero lo pasamos al shader en coseno
            const float PI = 3.14159265358979323846f;
            float cutOffCos = cosf(activeLight.cutOffDeg * PI / 180.0f);
            float outerCutOffCos = cosf(activeLight.outerCutOffDeg * PI / 180.0f);

            if (light_u_.uCutOff >= 0)
                glUniform1f(light_u_.uCutOff, cutOffCos);

            if (light_u_.uOuterCutOff >= 0)
                glUniform1f(light_u_.uOuterCutOff, outerCutOffCos);


            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadow_texture_);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_cubemap_);

            Iterator<RenderizableComponent> r_it, r_end;
            Iterator<TransformComponent> t_it, t_end;

            RenderizableComponent* r = nullptr;
            TransformComponent* t = nullptr;

            SetIterators(&r_it, &r_end, &r);
            SetIterators(&t_it, &t_end, &t);

            unsigned e = 0;
            while ((r_it != r_end && !r_it.IsEmpty()) ||
                (t_it != t_end && !t_it.IsEmpty()))
            {
                Mat4 model;
                model.identity();

                if (t_it != t_end && !t_it.IsEmpty() && t && t_it.owner() == e)
                    model = t->WorldMatrix();

                if (r_it != r_end && !r_it.IsEmpty() && r && r_it.owner() == e && r->Visible())
                {
                    Vec3 c = r->GetColor();

                    if (light_u_.uObjectColor >= 0)
                        glUniform3f(light_u_.uObjectColor, c.x_, c.y_, c.z_);

                    if (light_u_.uModel >= 0)
                        glUniformMatrix4fv(light_u_.uModel, 1, GL_FALSE, model.M_);

                    glBindVertexArray(r->GetVAO());
                    glDrawElements(GL_TRIANGLES, r->GetIndexCount(), GL_UNSIGNED_INT, 0);
                }

                CheckIteratorsFinished(&t_it, &t_end, &t, (int)e);
                CheckIteratorsFinished(&r_it, &r_end, &r, (int)e);
                ++e;
            }

            glBindVertexArray(0);

            // ============================
            // Debug pass
            // ============================
            if (i == 0 && showShadowDebug_)
            {
                glDisable(GL_DEPTH_TEST);

                glUseProgram(debug_pass_.GetProgram());

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, shadow_texture_);

                glBindVertexArray(debugVAO_);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);

                glEnable(GL_DEPTH_TEST);
            }
        }

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    void glShadowMap::CreatePointShadowBuffer()
    {
        glGenFramebuffers(1, &point_shadow_fbo_);
        glGenTextures(1, &point_shadow_cubemap_);

        glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_cubemap_);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_DEPTH_COMPONENT24,
                point_shadow_size_,
                point_shadow_size_,
                0,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                nullptr
            );
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbo_);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, point_shadow_cubemap_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("Point Shadow FBO ERROR\n");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    std::vector<Mat4> glShadowMap::BuildPointShadowTransforms(const Vec3& lightPos, float nearPlane, float farPlane)
    {
        float aspect = 1.0f;
        float fovRad = 90.0f * 3.14159265358979323846f / 180.0f;

        Mat4 shadowProj = MakePerspective(fovRad, aspect, nearPlane, farPlane);

        std::vector<Mat4> transforms;
        transforms.reserve(6);

        transforms.push_back(MulM4(
            shadowProj,
            MakeLookAt(lightPos, Vec3(lightPos.x_ + 1.0f, lightPos.y_, lightPos.z_), Vec3(0.0f, -1.0f, 0.0f))
        ));

        transforms.push_back(MulM4(
            shadowProj,
            MakeLookAt(lightPos, Vec3(lightPos.x_ - 1.0f, lightPos.y_, lightPos.z_), Vec3(0.0f, -1.0f, 0.0f))
        ));

        transforms.push_back(MulM4(
            shadowProj,
            MakeLookAt(lightPos, Vec3(lightPos.x_, lightPos.y_ + 1.0f, lightPos.z_), Vec3(0.0f, 0.0f, 1.0f))
        ));

        transforms.push_back(MulM4(
            shadowProj,
            MakeLookAt(lightPos, Vec3(lightPos.x_, lightPos.y_ - 1.0f, lightPos.z_), Vec3(0.0f, 0.0f, -1.0f))
        ));

        transforms.push_back(MulM4(
            shadowProj,
            MakeLookAt(lightPos, Vec3(lightPos.x_, lightPos.y_, lightPos.z_ + 1.0f), Vec3(0.0f, -1.0f, 0.0f))
        ));

        transforms.push_back(MulM4(
            shadowProj,
            MakeLookAt(lightPos, Vec3(lightPos.x_, lightPos.y_, lightPos.z_ - 1.0f), Vec3(0.0f, -1.0f, 0.0f))
        ));

        return transforms;
    }

} // namespace ME
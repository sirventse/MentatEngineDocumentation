#include <../include/MentatEngine/CameraComponent.hpp>
#include <../include/MentatEngine/Input.hpp>
#include <../include/MentatEngine/Window.hpp>
#include <GLFW/glfw3.h>
#include <cmath>

namespace {

    // helpers simples (sin depender del renderer)
    static Mat4 MakeLookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = center; f.substract(eye); f.normalize();

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

        m.M_[0] = s.x_; m.M_[4] = s.y_; m.M_[8] = s.z_;
        m.M_[12] = -(s.x_ * eye.x_ + s.y_ * eye.y_ + s.z_ * eye.z_);

        m.M_[1] = u.x_; m.M_[5] = u.y_; m.M_[9] = u.z_;
        m.M_[13] = -(u.x_ * eye.x_ + u.y_ * eye.y_ + u.z_ * eye.z_);

        m.M_[2] = -f.x_; m.M_[6] = -f.y_; m.M_[10] = -f.z_;
        m.M_[14] = (f.x_ * eye.x_ + f.y_ * eye.y_ + f.z_ * eye.z_);

        return m;
    }

    static Mat4 MakePerspective(float fovRadians, float aspect, float znear, float zfar) {
        float tanHalfFov = tanf(fovRadians * 0.5f);

        Mat4 m;
        for (int i = 0; i < 16; ++i) m.M_[i] = 0.0f;

        m.M_[0] = 1.0f / (aspect * tanHalfFov);
        m.M_[5] = 1.0f / (tanHalfFov);
        m.M_[10] = -(zfar + znear) / (zfar - znear);
        m.M_[11] = -1.0f;
        m.M_[14] = -(2.0f * zfar * znear) / (zfar - znear);
        return m;
    }

    static float Clamp(float v, float a, float b) {
        return (v < a) ? a : (v > b) ? b : v;
    }

} // anon

namespace ME {

    CameraComponent::CameraComponent() {
        position = Vec3(0.0f, 1.0f, 5.0f);
        yaw_deg = -90.0f;   // mirando hacia -Z
        pitch_deg = 0.0f;
        view.identity();
        proj.identity();
    }

    void CameraComponent::RebuildMatrices(int w, int h) {
        const float yaw = yaw_deg * 3.14159265f / 180.0f;
        const float pitch = pitch_deg * 3.14159265f / 180.0f;

        Vec3 forward(
            cosf(yaw) * cosf(pitch),
            sinf(pitch),
            sinf(yaw) * cosf(pitch)
        );
        forward.normalize();

        Vec3 worldUp(0.0f, 1.0f, 0.0f);

        Vec3 right = forward;
        right.crossProduct(worldUp);
        right.normalize();

        Vec3 up = right;
        up.crossProduct(forward);
        up.normalize();

        Vec3 target = position;
        target.add(forward);

        view = MakeLookAt(position, target, up);

        float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
        float fovRad = fov_deg * 3.14159265f / 180.0f;
        proj = MakePerspective(fovRad, aspect, near_plane, far_plane);
    }

    void CameraComponent::UpdateFly(Input& input, Window& window, float dt) {
        if (!active) return;

        GLFWwindow* gw = window.getGLFWwindow();
        const bool rmb = input.checkActionPressed(Input::Action::FlyEnable);
        const bool rmb_just_released = input.checkActionJustReleased(Input::Action::FlyEnable);

        if (rmb && !cursor_locked_) {
            glfwSetInputMode(gw, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursor_locked_ = true;
            first_mouse_ = true;
        }
        else if (rmb_just_released && cursor_locked_) {
            glfwSetInputMode(gw, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursor_locked_ = false;
        }

        int w = window.width();
        int h = window.height();

        if (cursor_locked_) {
            Vec2 mp = input.MousePos();
            if (first_mouse_) {
                last_mouse_ = mp;
                first_mouse_ = false;
            }

            float dx = mp.x_ - last_mouse_.x_;
            float dy = mp.y_ - last_mouse_.y_;
            last_mouse_ = mp;

            yaw_deg += dx * mouse_sens;
            pitch_deg -= dy * mouse_sens;
            pitch_deg = Clamp(pitch_deg, -89.0f, 89.0f);

            // movimiento
            const float yaw = yaw_deg * 3.14159265f / 180.0f;
            const float pitch = pitch_deg * 3.14159265f / 180.0f;

            Vec3 forward(
                cosf(yaw) * cosf(pitch),
                sinf(pitch),
                sinf(yaw) * cosf(pitch)
            );
            forward.normalize();

            Vec3 worldUp(0.0f, 1.0f, 0.0f);

            Vec3 right = forward;
            right.crossProduct(worldUp);
            right.normalize();

            Vec3 move(0.0f, 0.0f, 0.0f);
            if (input.checkActionPressed(Input::Action::FlyForward))  move.add(forward);
            if (input.checkActionPressed(Input::Action::FlyBackward)) { Vec3 t = forward; t.multByConst(-1.0f); move.add(t); }
            if (input.checkActionPressed(Input::Action::FlyRight))    move.add(right);
            if (input.checkActionPressed(Input::Action::FlyLeft)) { Vec3 t = right; t.multByConst(-1.0f); move.add(t); }
            if (input.checkActionPressed(Input::Action::FlyUp))       move.y_ += 1.0f;
            if (input.checkActionPressed(Input::Action::FlyDown))     move.y_ -= 1.0f;

            float mag = move.getMag();
            if (mag > 0.0001f) move.multByConst(1.0f / mag);

            float speed = move_speed;

            move.multByConst(speed * dt);
            position.add(move);
        }

        RebuildMatrices(w, h);
    }

} // namespace ME
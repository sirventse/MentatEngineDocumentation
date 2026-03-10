/**
 *
 * @brief camera component core class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */


#ifndef __CAMERA_COMPONENT_HPP__
#define __CAMERA_COMPONENT_HPP__ 1

#include <../deps/KFS/include/vec2.h>
#include <../deps/KFS/include/vec3.h>
#include <../deps/KFS/include/mat4.h>

namespace ME {

    class Input;
    class Window;

    class CameraComponent {
    public:
        /**
         * @brief Constructs basic data of the Camera.
         */
        CameraComponent();

        // camera params
        float fov_deg = 60.0f;
        float near_plane = 0.1f;
        float far_plane = 200.0f;

        // fly cam
        float move_speed = 6.0f;
        float mouse_sens = 0.12f; // grades per pixel aprox
        bool  active = true;

        // state
        Vec3 position;
        float yaw_deg;
        float pitch_deg;

        // matrices ready for the renderer
        Mat4 view;
        Mat4 proj;

        /**
         * @brief Updates the camera's position and rotation based on flying-style input.
         *
         * Handles WASD movement and mouse looking. The speed is adjusted
         * by the provided delta time.
         *
         * @param input Reference to the Input system to poll keys and mouse.
         * @param window Reference to the Window for cursor and dimension context.
         * @param dt Delta time since the last frame.
         */
        void UpdateFly(ME::Input& input, Window& window, float dt);

    private:
        bool first_mouse_ = true;
        Vec2 last_mouse_{ 0.0f, 0.0f };
        bool cursor_locked_ = false;
        
        /**
         * @brief Recalculates the view and projection matrices.
         *
         * This internal method should be called whenever the camera moves,
         * rotates, or the window is resized.
         *
         * @param w The new width of the viewport.
         * @param h The new height of the viewport.
         */
        void RebuildMatrices(int w, int h);
    };

}

#endif // __CAMERA_COMPONENT_HPP__
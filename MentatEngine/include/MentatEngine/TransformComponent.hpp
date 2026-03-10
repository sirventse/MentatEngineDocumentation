/**
 *
 * @brief TransformComponent base class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __TRANSFORMCOMPONENT_HPP__
#define __TRANSFORMCOMPONENT_HPP__ 1

#include "../KFS/include/mat4.h"
#include "../KFS/include/vec3.h"
#include <cmath>
#include <vector>

namespace ME {
	class TransformComponent {
		public:
            /**
             * @brief Default constructor.
             * Initializes the transform with position, rotation, scale and parent as 0.
             */
			TransformComponent() : position_{ 0.0f,0.0f,0.0f }, rotation_{ 0.0f,0.0f,0.0f }, scale_{ 1.0f,1.0f,1.0f }, parent_{ 0 } {};
			~TransformComponent() {};

			TransformComponent(TransformComponent&& other) noexcept
				: position_{ other.position_ },
				rotation_{ other.rotation_ },
				scale_{ other.scale_ },
				parent_{ other.parent_ }
			{}

			TransformComponent(const TransformComponent& other)
				: position_{ other.position_ },
				rotation_{ other.rotation_ },
				scale_{ other.scale_ },
				parent_{ other.parent_ }
			{}

			TransformComponent& operator=(TransformComponent&& other) noexcept {
				position_ = other.position_;
				rotation_ = other.rotation_;
				scale_ = other.scale_;
				parent_ = other.parent_;
				return *this;
			}

			TransformComponent& operator=(const TransformComponent& other) {
				position_ = other.position_;
				rotation_ = other.rotation_;
				scale_ = other.scale_;
				parent_ = other.parent_;
				return *this;
			}

            /**
             * @brief Sets the parent entity ID.
             * @param e The unsigned long ID of the parent.
             */
            void SetParent(unsigned long e) { parent_ = e; }

            /**
             * @brief Gets the parent entity ID.
             * @return The ID of the parent entity.
             */
            unsigned long GetParent() { return parent_; }

            /**
             * @brief Gets the local position of the entity.
             * @return A Vec3 representing the position.
             */
            Vec3 Position() const { return position_; };

            /**
             * @brief Gets the local rotation of the entity.
             * @return A Vec3 representing the rotation Euler angles.
             */
            Vec3 Rotation() const { return rotation_; };

            /**
             * @brief Gets the local scale of the entity.
             * @return A Vec3 representing the scale factors.
             */
            Vec3 Scale() const { return scale_; };

            /**
             * @brief Sets a new local position.
             * @param newPos The new position vector.
             */
            void SetPosition(Vec3 newPos) { position_ = newPos; };

            /**
             * @brief Sets a new local rotation.
             * @param newRot The new rotation vector (Euler angles).
             */
            void SetRotation(Vec3 newRot) { rotation_ = newRot; };

            /**
             * @brief Sets a new local scale.
             * @param newScale The new scale vector.
             */
            void SetScale(Vec3 newScale) { scale_ = newScale; };

            /**
             * @brief Calculates or retrieves the local transformation matrix.
             * @return The local Mat4 transformation matrix.
             */
            Mat4 LocalMatrix() const;

            /**
             * @brief Gets the world transformation matrix.
             * @return The current world Mat4 matrix.
             */
            Mat4 WorldMatrix() const { return world_; }

            /**
             * @brief Sets the world transformation matrix.
             * @param m The new world Mat4 matrix.
             */
            void SetWorldMatrix(const Mat4& m) { world_ = m; }

		private:
			Vec3 position_;
			Vec3 rotation_; // rotation_.x = pitch, rotation_.y = yaw, rotation_.z = roll
			Vec3 scale_;
			Mat4 world_;
			unsigned long parent_;
	};
}

#endif // __TRANSFORMCOMPONENT_HPP__
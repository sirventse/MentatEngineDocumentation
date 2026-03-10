/**
 *
 * @brief Entity class.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __ENTITY_HPP__
#define __ENTITY_HPP__ 1

#include <../include/MentatEngine/ECSManager.hpp>
#include <../include/MentatEngine/TagComponent.hpp>

namespace ME {
	class Entity {
    public:
        /**
          * @brief Default constructor.
          *
          * Registers a new entity in the ECS and automatically adds a default
          * TagComponent labeled with the assigned unique ID.
          */
        Entity() : id_{ ME::ECS::AddEntity() }
        {
            char label[64];
            sprintf(label, "Entity %lu", id_);
            ME::ECS::AddComponent<ME::TagComponent>(id_, label);
        };

        /**
         * @brief Retrieves a component of a specific type from this entity.
         * @tparam T The type of the component to search for.
         * @return A pointer to the component if found, nullptr otherwise.
         */
        template<typename T> T* GetComponent() {
            return ME::ECS::GetComponent<T>(id_);
        };

        /**
         * @brief Adds a new component to this entity.
         *
         * Forwards the given arguments to the component's constructor.
         *
         * @tparam T The type of the component to add.
         * @tparam Args Template parameter pack for constructor arguments.
         * @param args Arguments to be forwarded to the component constructor.
         * @return A reference to the newly created component.
         */
        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            return ME::ECS::AddComponent<T>(id_, std::forward<Args>(args)...);
        };

        /**
         * @brief Removes a component of a specific type from this entity.
         * @tparam T The type of the component to be removed.
         */
        template<typename T>
        void RemoveComponent() {
            ME::ECS::RemoveComponent<T>(id_);
        };

        /**
         * @brief Gets the unique identifier of the entity.
         * @return The unsigned long ID.
         */
        unsigned long GetID() { return id_; };

        /**
         * @brief Sets another entity as the parent of this one.
         * @param e Reference to the parent Entity object.
         * @return True if the parent was successfully set, false otherwise.
         */
        bool SetParent(const Entity& e);

        /**
         * @brief Gets the unique ID of the parent entity.
         * @return The ID of the parent, or a null-equivalent ID if it has no parent.
         */
        unsigned long GetParentID();

    private: 
        unsigned long id_;
	};
}

#endif // __ENTITY_HPP__
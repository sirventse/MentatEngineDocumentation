/**
 *
 * @brief Window opener with GLFW.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#include <GLFW/glfw3.h>
#include <../include/MentatEngine/ECSManager.hpp>
#include <../include/MentatEngine/TransformComponent.hpp>
#include <../include/MentatEngine/Entity.hpp>
#include <../include/MentatEngine/Iterator.hpp>

namespace ME {

    // Definición de la variable en el cpp al ser estática
    std::unordered_map<std::size_t, std::unique_ptr<ECSListBase>> ECS::component_map_;
    unsigned long ECS::current_index;
    bool ECS::is_dirty;

    void ECS::SortLists()
    {
        for (auto& cl : component_map_) {
            cl.second->SortList();
        }
        is_dirty = false;
    }

    unsigned long ECS::AddEntity()
    {
        current_index++;
        return current_index - 1;
    }

    void ECS::RemoveEntity(unsigned long id) 
    {
        for (auto& cl : component_map_) {
            cl.second->ClearEntity(id);
        }
    }

    std::vector<unsigned long> ECS::GetChildren(unsigned long id) {
        std::vector<unsigned long> children;
        auto& transforms = ME::ECS::GetComponentList<ME::TransformComponent>();

        for (auto& pair : transforms) {
            if (pair.second.GetParent() == id) children.push_back(pair.first);
        }
       
        return children;
    }
    unsigned long ECS::GetEntityByTag(std::string tag)
    {
        unsigned long res = -9999999;
        int counter = 0;
        ME::Iterator<ME::TagComponent> tag_it, tag_end;
        ME::TagComponent* tc = nullptr;

        ME::ContainerFacade<ME::TagComponent> container{ ME::ECS::GetComponentList<ME::TagComponent>() };

        if (!container.isEmpty()) {
            tag_it = container.begin();
            tag_it.SetEmpty(false);
            tc = *tag_it;
            tag_end = container.end();
            tag_end.SetEmpty(false);
        }
        else {
            tag_it.SetEmpty(true);
            tag_end.SetEmpty(true);
        }

        while (tag_it != tag_end) {
            if (tag_it != tag_end && !tag_it.IsEmpty() && tc) {
                if (tag == tc->getTag()) {
                    res = counter;
                }
            }
            if (tag_it != tag_end && tag_it.owner() == counter && !tag_it.IsEmpty()) {
                ++tag_it;
                if (tag_it != tag_end) {
                    tc = *tag_it;
                }
            }
            counter++;
        }
        return res;
    }
}


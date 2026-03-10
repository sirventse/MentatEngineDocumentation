#include "../../Include/MentatEngine/Entity.hpp"
#include "../../Include/MentatEngine/TransformComponent.hpp"

namespace ME {
    bool Entity::SetParent(const Entity& e)
    {
        ME::TransformComponent* tc = e.GetComponent<TransformComponent>();
        if (tc != nullptr && tc->GetParent() != id_) {
            tc->SetParent(id_);
            return true;
        }
        else {
            return false;
        }
    }

    unsigned long Entity::GetParentID()
    {
        ME::TransformComponent* tc = ME::ECS::GetComponent<ME::TransformComponent>(id_);
        if (tc != nullptr) {
            return tc->GetParent();
        }
        else {
            return 0;
        }
    }
}

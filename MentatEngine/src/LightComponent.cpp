#include "../include/MentatEngine/LightComponent.hpp"
#include <utility>

namespace ME {

	LightComponent::~LightComponent()
	{
	}

    LightComponent::LightComponent(LightComponent&& other)
    {
        *this = std::move(other);
    }

    // Constructor de Copia
    LightComponent::LightComponent(const LightComponent& other) {
        *this = other;
    }

    // Operador de Asignación por Copia
    LightComponent& LightComponent::operator=(const LightComponent& other) {
        if (this != &other) {
            type_ = other.type_;
            is_enabled_ = other.is_enabled_;
            pos_ = other.pos_;
            dir_ = other.dir_;
            diff_color_ = other.diff_color_;
            spec_color_ = other.spec_color_;
            shininess_ = other.shininess_;
            spec_intensity_ = other.spec_intensity_;
            constant_att_ = other.constant_att_;
            linear_att_ = other.linear_att_;
            quad_att_ = other.quad_att_;
            cut_off_ = other.cut_off_;
            outer_cut_off_ = other.outer_cut_off_;
			casts_shadows_ = other.casts_shadows_;
			shadow_near_ = other.shadow_near_;
			shadow_far_ = other.shadow_far_;
        }
        return *this;
    }

    // Operador de Asignación por Movimiento
    LightComponent& LightComponent::operator=(LightComponent&& other) noexcept {
        if (this != &other) {
            type_ = std::move(other.type_);
            is_enabled_ = std::move(other.is_enabled_);
            pos_ = std::move(other.pos_);
            dir_ = std::move(other.dir_);
            diff_color_ = std::move(other.diff_color_);
            spec_color_ = std::move(other.spec_color_);
            shininess_ = std::move(other.shininess_);
            spec_intensity_ = std::move(other.spec_intensity_);
            constant_att_ = std::move(other.constant_att_);
            linear_att_ = std::move(other.linear_att_);
            quad_att_ = std::move(other.quad_att_);
            cut_off_ = std::move(other.cut_off_);
            outer_cut_off_ = std::move(other.outer_cut_off_);
			casts_shadows_ = std::move(other.casts_shadows_);
			shadow_far_ = std::move(other.shadow_far_);
			shadow_near_ = std::move(other.shadow_near_);
        }
        return *this;
    }
}

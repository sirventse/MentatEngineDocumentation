/**
 *
 * @brief LightComponent base class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __LIGHTCOMPONENT_HPP__
#define __LIGHTCOMPONENT_HPP__ 1

#include "../KFS/include/vec3.h"

namespace ME {
    enum LightType
    {
        AMBIENT,
        POINT,
        DIRECTIONAL,
        SPOT
    };
	class LightComponent {
		const double PI = 3.14159265358979323846;
	    public:
            /**
             * @brief Default constructor.
             * Initializes the Light by default.
             */
            LightComponent()
                : type_(LightType::POINT), // por defecto point light
                is_enabled_(1),
                pos_(0.0f, 0.0f, 0.0f),
                dir_(0.0f, -1.0f, 0.0f),
                diff_color_(1.0f, 1.0f, 1.0f),
                spec_color_(1.0f, 1.0f, 1.0f),
                shininess_(32.0f),
                spec_intensity_(1.0f),
                constant_att_(1.0f),
                linear_att_(0.09f),
                quad_att_(0.032f),
                cut_off_(12.5f),
                outer_cut_off_(15.0f),
                casts_shadows_(false),
                shadow_near_(0.1f),
                shadow_far_(200.0f)
            {
            };
            ~LightComponent();
            LightComponent(LightComponent&& other);
            LightComponent(const LightComponent& other);
            LightComponent& operator=(LightComponent&& other) noexcept;
            LightComponent& operator=(const LightComponent& other);

            /**
             * @brief Gets the type of the light (e.g., Point, Directional, Spot).
             * @return The current LightType.
             */
            LightType Type() const { return type_; }

            /**
             * @brief Sets the type of the light.
             * @param type The new LightType to apply.
             */
            void SetType(LightType type) { type_ = type; }

            /**
             * @brief Checks if the light source is active.
             * @return True if enabled, false otherwise.
             */
            bool Enabled() const { return is_enabled_; }

            /**
             * @brief Enables or disables the light source.
             * @param b True to enable, false to disable.
             */
            void SetEnabled(bool b) { is_enabled_ = b; }

            /**
             * @brief Gets the world position of the light.
             * @return A Vec3 representing the position.
             */
            Vec3 Pos() const { return pos_; }

            /**
             * @brief Sets the world position of the light.
             * @param p The new position vector.
             */
            void SetPos(Vec3 p) { pos_ = p; }

            /**
             * @brief Gets the direction vector of the light.
             * @return A Vec3 representing the light direction.
             */
            Vec3 Dir() const { return dir_; }

            /**
             * @brief Sets the direction vector of the light.
             * @param d The new direction vector.
             */
            void SetDir(Vec3 d) { dir_ = d; }

            /**
             * @brief Gets the diffuse color component.
             * @return A Vec3 representing the RGB diffuse color.
             */
            Vec3 DiffColor() const { return diff_color_; }

            /**
             * @brief Sets the diffuse color component.
             * @param dc The new RGB diffuse color.
             */
            void SetDiffColor(Vec3 dc) { diff_color_ = dc; }

            /**
             * @brief Gets the specular color component.
             * @return A Vec3 representing the RGB specular color.
             */
            Vec3 SpecColor() const { return spec_color_; }

            /**
             * @brief Sets the specular color component.
             * @param sc The new RGB specular color.
             */
            void SetSpecColor(Vec3 sc) { spec_color_ = sc; }

            /**
             * @brief Gets the shininess exponent for specular highlights.
             * @return The shininess value.
             */
            float Shininess() const { return shininess_; }

            /**
             * @brief Sets the shininess exponent for specular highlights.
             * @param sh The new shininess value.
             */
            void SetShininess(float sh) { shininess_ = sh; }

            /**
             * @brief Gets the intensity multiplier for specular highlights.
             * @return The specular intensity factor.
             */
            float SpecIntensity() const { return spec_intensity_; }

            /**
             * @brief Sets the intensity multiplier for specular highlights.
             * @param si The new specular intensity factor.
             */
            void SetSpecIntensity(float si) { spec_intensity_ = si; }

            /**
             * @brief Gets the constant attenuation factor.
             * @return The constant attenuation value.
             */
            float ConstantAtt() const { return constant_att_; }

            /**
             * @brief Sets the constant attenuation factor.
             * @param catt The new constant attenuation value.
             */
            void SetConstantAtt(float catt) { constant_att_ = catt; }

            /**
             * @brief Gets the linear attenuation factor.
             * @return The linear attenuation value.
             */
            float LinearAtt() const { return linear_att_; }

            /**
             * @brief Sets the linear attenuation factor.
             * @param latt The new linear attenuation value.
             */
            void SetLinearAtt(float latt) { linear_att_ = latt; }

            /**
             * @brief Gets the quadratic attenuation factor.
             * @return The quadratic attenuation value.
             */
            float QuadAtt() const { return quad_att_; }

            /**
             * @brief Sets the quadratic attenuation factor.
             * @param qatt The new quadratic attenuation value.
             */
            void SetQuadAtt(float qatt) { quad_att_ = qatt; }

            /**
             * @brief Gets the inner cutoff angle for spotlights.
             * @return The cutoff angle in degrees or radians.
             */
            float CutOff() const { return cut_off_; }

            /**
             * @brief Sets the inner cutoff angle for spotlights.
             * @param coff The new cutoff angle.
             */
            void SetCutOff(float coff) { cut_off_ = coff; }

            /**
             * @brief Gets the outer cutoff angle for spotlights (for soft edges).
             * @return The outer cutoff angle.
             */
            float OuterCutOff() const { return outer_cut_off_; }

            /**
             * @brief Sets the outer cutoff angle for spotlights.
             * @param ocoff The new outer cutoff angle.
             */
            void SetOuterCutOff(float ocoff) { outer_cut_off_ = ocoff; }

            /**
             * @brief Checks if this light source casts shadows.
             * @return True if shadows are enabled, false otherwise.
             */
            bool CastsShadows() const { return casts_shadows_; }

            /**
             * @brief Enables or disables shadow casting for this light.
             * @param b True to cast shadows.
             */
            void SetCastsShadows(bool b) { casts_shadows_ = b; }

            /**
             * @brief Gets the near plane distance for shadow mapping.
             * @return The near plane value.
             */
            float ShadowNear() const { return shadow_near_; }

            /**
             * @brief Sets the near plane distance for shadow mapping.
             * @param n The new near plane distance.
             */
            void SetShadowNear(float n) { shadow_near_ = n; }

            /**
             * @brief Gets the far plane distance for shadow mapping.
             * @return The far plane value.
             */
            float ShadowFar() const { return shadow_far_; }

            /**
             * @brief Sets the far plane distance for shadow mapping.
             * @param f The new far plane distance.
             */
            void SetShadowFar(float f) { shadow_far_ = f; }

	    private:
            LightType type_;
            int is_enabled_;
            Vec3 pos_;
            Vec3 dir_;
            Vec3 diff_color_;
            Vec3 spec_color_;
            float shininess_;
            float spec_intensity_;
            float constant_att_;
            float linear_att_;
            float quad_att_;
            float cut_off_;
            float outer_cut_off_;
            bool casts_shadows_;
            float shadow_near_;
            float shadow_far_;
	};
}

#endif // __LIGHTCOMPONENT_HPP__
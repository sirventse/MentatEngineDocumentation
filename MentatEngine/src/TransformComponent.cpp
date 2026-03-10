#include "../Include/MentatEngine/TransformComponent.hpp"
#include "../Include/MentatEngine/ECSManager.hpp"

namespace ME {
    static Mat4 MulCM(const Mat4& A, const Mat4& B) {
        Mat4 R;
        R.reset();
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                R.M_[c * 4 + r] =
                    A.M_[0 * 4 + r] * B.M_[c * 4 + 0] +
                    A.M_[1 * 4 + r] * B.M_[c * 4 + 1] +
                    A.M_[2 * 4 + r] * B.M_[c * 4 + 2] +
                    A.M_[3 * 4 + r] * B.M_[c * 4 + 3];
            }
        }
        return R;
    }

    Mat4 TransformComponent::LocalMatrix() const
    {
        Mat4 T; T.identity();
        Vec3 v = position_;
        T.translation(v);

        Mat4 RX; RX.identity(); RX.rotateX(rotation_.x_);
        Mat4 RY; RY.identity(); RY.rotateY(rotation_.y_);
        Mat4 RZ; RZ.identity(); RZ.rotateZ(rotation_.z_);

        Mat4 S; S.identity();
        Vec3 sHelper = scale_;
        S.scale(sHelper);

        // R = Rz * Ry * Rx
        Mat4 R = MulCM(RZ, MulCM(RY, RX));

        // M = T * R * S
        Mat4 M = MulCM(T, MulCM(R, S));
        return M;
    }

}
#include "../include/MentatEngine/ImGuiManager.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include "../include/MentatEngine/LightComponent.hpp"
#include "../include/MentatEngine/ECSManager.hpp"
#include <MentatEngine/TransformComponent.hpp>
#include <MentatEngine/OpenGL/RenderizableComponent.hpp>

#include <MentatEngine/ScriptComponent.hpp>
#include <MentatEngine/XMLManager.hpp>

namespace fs = std::filesystem;

static unsigned long tmp_parent;
static bool add_component_pressed;
static int component_flags;
static std::string tmp_serialized_content;
static unsigned long selected_entity = 999999;
static int pending_update_component = -1;
static Vec3 current_selected_color_;
static ME::ImGuiManager::GizmoAxis PickGizmoAxis(const ImVec2& mousePos);

// Base cube
Vec3 cubeVerts[] = {
    {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, // back
    {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}  // front
};

unsigned int cubeIdx[] = {
    0,1,2, 2,3,0, // back
    4,5,6, 6,7,4, // front
    4,7,3, 3,0,4, // left
    5,1,2, 2,6,5, // right
    7,6,2, 2,3,7, // top
    4,5,1, 1,0,4  // bottom
};

// --- RECURSIVE FUNCTIONS FOR IMGUI

static float Distance2D(const ImVec2& a, const ImVec2& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

void DrawFileExplorer(const fs::path& path)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        const auto& p = entry.path();
        std::string name = p.filename().string();

        if (entry.is_directory())
        {
            // Carpeta: usar TreeNodeEx para control de clic
            if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth))
            {
                DrawFileExplorer(p); // recursivo
                ImGui::TreePop();
            }
        }
        else
        {
            // Archivo: usar Selectable para poder arrastrar
            ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                std::string fullPath = p.generic_string();
                ImGui::SetDragDropPayload(
                    "EXPLORER_RESOURCE",
                    fullPath.c_str(),
                    fullPath.size() + 1
                );
                ImGui::Text("%s", name.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
}

static float DistancePointToSegment2D(const ImVec2& p, const ImVec2& a, const ImVec2& b)
{
    float abx = b.x - a.x;
    float aby = b.y - a.y;
    float apx = p.x - a.x;
    float apy = p.y - a.y;

    float abLenSq = abx * abx + aby * aby;
    if (abLenSq <= 0.0001f) {
        float dx = p.x - a.x;
        float dy = p.y - a.y;
        return sqrtf(dx * dx + dy * dy);
    }

    float t = (apx * abx + apy * aby) / abLenSq;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float closestX = a.x + t * abx;
    float closestY = a.y + t * aby;

    float dx = p.x - closestX;
    float dy = p.y - closestY;
    return sqrtf(dx * dx + dy * dy);
}

static ME::ImGuiManager::GizmoAxis PickGizmoAxis(const ImVec2& mousePos)
{
    using GizmoAxis = ME::ImGuiManager::GizmoAxis;

    if (!ME::ImGuiManager::gizmo_state_.visible)
        return GizmoAxis::NONE;

    const ImVec2 origin = ME::ImGuiManager::gizmo_state_.originScreen;
    const ImVec2 xEnd = ME::ImGuiManager::gizmo_state_.axisXEnd;
    const ImVec2 yEnd = ME::ImGuiManager::gizmo_state_.axisYEnd;
    const ImVec2 zEnd = ME::ImGuiManager::gizmo_state_.axisZEnd;

    float dx = DistancePointToSegment2D(mousePos, origin, xEnd);
    float dy = DistancePointToSegment2D(mousePos, origin, yEnd);
    float dz = DistancePointToSegment2D(mousePos, origin, zEnd);

    const float threshold = 10.0f;

    float best = threshold;
    GizmoAxis bestAxis = GizmoAxis::NONE;

    if (dx < best) { best = dx; bestAxis = GizmoAxis::X; }
    if (dy < best) { best = dy; bestAxis = GizmoAxis::Y; }
    if (dz < best) { best = dz; bestAxis = GizmoAxis::Z; }

    return bestAxis;
}

static void GetGizmoAxes(const ME::TransformComponent* tc, Vec3& axisX, Vec3& axisY, Vec3& axisZ)
{
    if (ME::ImGuiManager::current_gizmo_space_ == ME::ImGuiManager::GizmoSpace::WORLD || tc == nullptr)
    {
        axisX = Vec3(1.0f, 0.0f, 0.0f);
        axisY = Vec3(0.0f, 1.0f, 0.0f);
        axisZ = Vec3(0.0f, 0.0f, 1.0f);
        return;
    }

    Mat4 wm = tc->WorldMatrix();

    axisX = Vec3(wm.M_[0], wm.M_[1], wm.M_[2]);
    axisY = Vec3(wm.M_[4], wm.M_[5], wm.M_[6]);
    axisZ = Vec3(wm.M_[8], wm.M_[9], wm.M_[10]);

    axisX.normalize();
    axisY.normalize();
    axisZ.normalize();
}

static bool WorldToScreen(const Vec3& worldPos, const Mat4& view, const Mat4& proj, ImVec2& outScreen)
{
    float x = worldPos.x_;
    float y = worldPos.y_;
    float z = worldPos.z_;

    float vx =
        view.M_[0] * x + view.M_[4] * y + view.M_[8] * z + view.M_[12];
    float vy =
        view.M_[1] * x + view.M_[5] * y + view.M_[9] * z + view.M_[13];
    float vz =
        view.M_[2] * x + view.M_[6] * y + view.M_[10] * z + view.M_[14];
    float vw =
        view.M_[3] * x + view.M_[7] * y + view.M_[11] * z + view.M_[15];

    float px =
        proj.M_[0] * vx + proj.M_[4] * vy + proj.M_[8] * vz + proj.M_[12] * vw;
    float py =
        proj.M_[1] * vx + proj.M_[5] * vy + proj.M_[9] * vz + proj.M_[13] * vw;
    float pz =
        proj.M_[2] * vx + proj.M_[6] * vy + proj.M_[10] * vz + proj.M_[14] * vw;
    float pw =
        proj.M_[3] * vx + proj.M_[7] * vy + proj.M_[11] * vz + proj.M_[15] * vw;

    if (pw <= 0.0001f) return false;

    float ndcX = px / pw;
    float ndcY = py / pw;
    float ndcZ = pz / pw;

    if (ndcZ < -1.0f || ndcZ > 1.0f) return false;

    ImGuiIO& io = ImGui::GetIO();
    outScreen.x = (ndcX * 0.5f + 0.5f) * io.DisplaySize.x;
    outScreen.y = (1.0f - (ndcY * 0.5f + 0.5f)) * io.DisplaySize.y;

    return true;
}

static void DrawProjectedOrientedRing(
    const Vec3& origin,
    const Vec3& basisA,
    const Vec3& basisB,
    float radius,
    const Mat4& camView,
    const Mat4& camProj,
    ImU32 color,
    float thickness)
{
    const int segments = 64;
    bool hasPrev = false;
    ImVec2 prevScreen{};

    auto* draw = ImGui::GetForegroundDrawList();

    for (int i = 0; i <= segments; ++i)
    {
        float t = (float)i / (float)segments;
        float a = t * 6.28318530718f;

        Vec3 p(
            origin.x_ + basisA.x_ * cosf(a) * radius + basisB.x_ * sinf(a) * radius,
            origin.y_ + basisA.y_ * cosf(a) * radius + basisB.y_ * sinf(a) * radius,
            origin.z_ + basisA.z_ * cosf(a) * radius + basisB.z_ * sinf(a) * radius
        );

        ImVec2 screen;
        bool visible = WorldToScreen(p, camView, camProj, screen);

        if (visible)
        {
            if (hasPrev) {
                draw->AddLine(prevScreen, screen, color, thickness);
            }
            prevScreen = screen;
            hasPrev = true;
        }
        else
        {
            hasPrev = false;
        }
    }
}


static float DistanceToProjectedOrientedRing(
    const ImVec2& mousePos,
    const Vec3& origin,
    const Vec3& basisA,
    const Vec3& basisB,
    float radius,
    const Mat4& camView,
    const Mat4& camProj)
{
    const int segments = 64;
    float best = 1e9f;

    bool hasPrev = false;
    ImVec2 prevScreen{};

    for (int i = 0; i <= segments; ++i)
    {
        float t = (float)i / (float)segments;
        float a = t * 6.28318530718f;

        Vec3 p(
            origin.x_ + basisA.x_ * cosf(a) * radius + basisB.x_ * sinf(a) * radius,
            origin.y_ + basisA.y_ * cosf(a) * radius + basisB.y_ * sinf(a) * radius,
            origin.z_ + basisA.z_ * cosf(a) * radius + basisB.z_ * sinf(a) * radius
        );

        ImVec2 screen;
        bool visible = WorldToScreen(p, camView, camProj, screen);

        if (visible)
        {
            if (hasPrev)
            {
                float d = DistancePointToSegment2D(mousePos, prevScreen, screen);
                if (d < best) best = d;
            }

            prevScreen = screen;
            hasPrev = true;
        }
        else
        {
            hasPrev = false;
        }
    }

    return best;
}

static ME::ImGuiManager::GizmoAxis PickRotateOrientedRingAxis(
    const ImVec2& mousePos,
    const Vec3& origin,
    const Vec3& axisX,
    const Vec3& axisY,
    const Vec3& axisZ,
    float radius,
    const Mat4& camView,
    const Mat4& camProj)
{
    using GizmoAxis = ME::ImGuiManager::GizmoAxis;

    // Ring X -> plane generado por Y y Z
    float dx = DistanceToProjectedOrientedRing(mousePos, origin, axisY, axisZ, radius, camView, camProj);

    // Ring Y -> plane generado por X y Z
    float dy = DistanceToProjectedOrientedRing(mousePos, origin, axisX, axisZ, radius, camView, camProj);

    // Ring Z -> plane generado por X y Y
    float dz = DistanceToProjectedOrientedRing(mousePos, origin, axisX, axisY, radius, camView, camProj);

    const float threshold = 10.0f;

    float best = threshold;
    GizmoAxis bestAxis = GizmoAxis::NONE;

    if (dx < best) { best = dx; bestAxis = GizmoAxis::X; }
    if (dy < best) { best = dy; bestAxis = GizmoAxis::Y; }
    if (dz < best) { best = dz; bestAxis = GizmoAxis::Z; }

    return bestAxis;
}


static float DistanceToProjectedRing(
    const ImVec2& mousePos,
    const Vec3& origin,
    float radius,
    int axis, // 0 = X, 1 = Y, 2 = Z
    const Mat4& camView,
    const Mat4& camProj)
{
    const int segments = 64;
    float best = 1e9f;

    bool hasPrev = false;
    ImVec2 prevScreen{};

    for (int i = 0; i <= segments; ++i)
    {
        float t = (float)i / (float)segments;
        float a = t * 6.28318530718f;

        Vec3 p = origin;

        if (axis == 0) { // X ring -> YZ plane
            p.y_ += cosf(a) * radius;
            p.z_ += sinf(a) * radius;
        }
        else if (axis == 1) { // Y ring -> XZ plane
            p.x_ += cosf(a) * radius;
            p.z_ += sinf(a) * radius;
        }
        else { // Z ring -> XY plane
            p.x_ += cosf(a) * radius;
            p.y_ += sinf(a) * radius;
        }

        ImVec2 screen;
        bool visible = WorldToScreen(p, camView, camProj, screen);

        if (visible)
        {
            if (hasPrev)
            {
                float d = DistancePointToSegment2D(mousePos, prevScreen, screen);
                if (d < best) best = d;
            }

            prevScreen = screen;
            hasPrev = true;
        }
        else
        {
            hasPrev = false;
        }
    }

    return best;
}

static ME::ImGuiManager::GizmoAxis PickRotateProjectedRingAxis(
    const ImVec2& mousePos,
    const Vec3& origin,
    float radius,
    const Mat4& camView,
    const Mat4& camProj)
{
    using GizmoAxis = ME::ImGuiManager::GizmoAxis;

    float dx = DistanceToProjectedRing(mousePos, origin, radius, 0, camView, camProj);
    float dy = DistanceToProjectedRing(mousePos, origin, radius, 1, camView, camProj);
    float dz = DistanceToProjectedRing(mousePos, origin, radius, 2, camView, camProj);

    const float threshold = 10.0f;

    float best = threshold;
    GizmoAxis bestAxis = GizmoAxis::NONE;

    if (dx < best) { best = dx; bestAxis = GizmoAxis::X; }
    if (dy < best) { best = dy; bestAxis = GizmoAxis::Y; }
    if (dz < best) { best = dz; bestAxis = GizmoAxis::Z; }

    return bestAxis;
}


static void DrawProjectedRing(
    const Vec3& origin,
    float radius,
    int axis, // 0 = X, 1 = Y, 2 = Z
    const Mat4& camView,
    const Mat4& camProj,
    ImU32 color,
    float thickness)
{
    const int segments = 64;
    bool hasPrev = false;
    ImVec2 prevScreen{};

    auto* draw = ImGui::GetForegroundDrawList();

    for (int i = 0; i <= segments; ++i)
    {
        float t = (float)i / (float)segments;
        float a = t * 6.28318530718f;

        Vec3 p = origin;

        if (axis == 0) { // X ring -> YZ plane
            p.y_ += cosf(a) * radius;
            p.z_ += sinf(a) * radius;
        }
        else if (axis == 1) { // Y ring -> XZ plane
            p.x_ += cosf(a) * radius;
            p.z_ += sinf(a) * radius;
        }
        else { // Z ring -> XY plane
            p.x_ += cosf(a) * radius;
            p.y_ += sinf(a) * radius;
        }

        ImVec2 screen;
        bool visible = WorldToScreen(p, camView, camProj, screen);

        if (visible)
        {
            if (hasPrev) {
                draw->AddLine(prevScreen, screen, color, thickness);
            }

            prevScreen = screen;
            hasPrev = true;
        }
        else
        {
            hasPrev = false;
        }
    }
}
static ImVec2 Normalize2D(const ImVec2& v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y);
    if (len <= 0.0001f) return ImVec2(0.0f, 0.0f);
    return ImVec2(v.x / len, v.y / len);
}

static float ProjectMouseDeltaOnAxis(const ImVec2& mouseDelta, const ImVec2& axisDir)
{
    return mouseDelta.x * axisDir.x + mouseDelta.y * axisDir.y;
}

static float SignedAngle2D(const ImVec2& a, const ImVec2& b)
{
    float cross = a.x * b.y - a.y * b.x;
    float dot = a.x * b.x + a.y * b.y;
    return atan2f(cross, dot); // radianes
}

struct RingPickInfo {
    float bestDistance = 1e9f;
    ImVec2 closestPoint{ 0.0f, 0.0f };
    ImVec2 tangent{ 0.0f, 0.0f };
};

static RingPickInfo ComputeProjectedOrientedRingPickInfo(
    const ImVec2& mousePos,
    const Vec3& origin,
    const Vec3& basisA,
    const Vec3& basisB,
    float radius,
    const Mat4& camView,
    const Mat4& camProj)
{
    const int segments = 96;

    RingPickInfo result;
    bool hasPrev = false;
    ImVec2 prevScreen{};

    for (int i = 0; i <= segments; ++i)
    {
        float t = (float)i / (float)segments;
        float a = t * 6.28318530718f;

        Vec3 p(
            origin.x_ + basisA.x_ * cosf(a) * radius + basisB.x_ * sinf(a) * radius,
            origin.y_ + basisA.y_ * cosf(a) * radius + basisB.y_ * sinf(a) * radius,
            origin.z_ + basisA.z_ * cosf(a) * radius + basisB.z_ * sinf(a) * radius
        );

        ImVec2 screen;
        bool visible = WorldToScreen(p, camView, camProj, screen);

        if (visible)
        {
            if (hasPrev)
            {
                float d = DistancePointToSegment2D(mousePos, prevScreen, screen);
                if (d < result.bestDistance)
                {
                    result.bestDistance = d;

                    // punto m�s cercano sobre el segmento
                    ImVec2 ab(screen.x - prevScreen.x, screen.y - prevScreen.y);
                    ImVec2 ap(mousePos.x - prevScreen.x, mousePos.y - prevScreen.y);

                    float abLenSq = ab.x * ab.x + ab.y * ab.y;
                    float u = 0.0f;
                    if (abLenSq > 0.0001f) {
                        u = (ap.x * ab.x + ap.y * ab.y) / abLenSq;
                        if (u < 0.0f) u = 0.0f;
                        if (u > 1.0f) u = 1.0f;
                    }

                    result.closestPoint = ImVec2(
                        prevScreen.x + ab.x * u,
                        prevScreen.y + ab.y * u
                    );

                    result.tangent = Normalize2D(ab);
                }
            }

            prevScreen = screen;
            hasPrev = true;
        }
        else
        {
            hasPrev = false;
        }
    }

    return result;
}

static RingPickInfo GetRotateRingPickInfoForAxis(
    ME::ImGuiManager::GizmoAxis axis,
    const ImVec2& mousePos,
    const Vec3& origin,
    const Vec3& axisX,
    const Vec3& axisY,
    const Vec3& axisZ,
    float radius,
    const Mat4& camView,
    const Mat4& camProj)
{
    using GizmoAxis = ME::ImGuiManager::GizmoAxis;

    if (axis == GizmoAxis::X) {
        return ComputeProjectedOrientedRingPickInfo(mousePos, origin, axisY, axisZ, radius, camView, camProj);
    }
    else if (axis == GizmoAxis::Y) {
        return ComputeProjectedOrientedRingPickInfo(mousePos, origin, axisX, axisZ, radius, camView, camProj);
    }
    else if (axis == GizmoAxis::Z) {
        return ComputeProjectedOrientedRingPickInfo(mousePos, origin, axisX, axisY, radius, camView, camProj);
    }

    return RingPickInfo{};
}

static void DrawRotateGizmoOverlay(unsigned long selected_entity, const Mat4& camView, const Mat4& camProj)
{
	float ringRadius = 1.2f;
    ME::ImGuiManager::gizmo_state_.visible = false;
    ME::ImGuiManager::gizmo_state_.hotAxis = ME::ImGuiManager::GizmoAxis::NONE;

    if (selected_entity == 999999) return;

    auto* tc = ME::ECS::GetComponent<ME::TransformComponent>(selected_entity);
    if (!tc) return;

    Mat4 wm = tc->WorldMatrix();
    Vec3 origin(wm.M_[12], wm.M_[13], wm.M_[14]);

    Vec3 axisX, axisY, axisZ;
    GetGizmoAxes(tc, axisX, axisY, axisZ);

    ImVec2 originScreen;

    if (!WorldToScreen(origin, camView, camProj, originScreen))
        return;

    ME::ImGuiManager::gizmo_state_.visible = true;
    ME::ImGuiManager::gizmo_state_.originScreen = originScreen;

    ImVec2 mousePos = ImGui::GetMousePos();
    ME::ImGuiManager::gizmo_state_.hotAxis =
        PickRotateOrientedRingAxis(mousePos, origin, axisX, axisY, axisZ, ringRadius, camView, camProj);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
    {
        if (ME::ImGuiManager::gizmo_state_.hotAxis != ME::ImGuiManager::GizmoAxis::NONE)
        {
            ME::ImGuiManager::gizmo_state_.activeAxis = ME::ImGuiManager::gizmo_state_.hotAxis;
            ME::ImGuiManager::gizmo_state_.dragStartMouse = ImGui::GetMousePos();
            ME::ImGuiManager::gizmo_state_.dragStartRotation = tc->Rotation();

            RingPickInfo pickInfo = GetRotateRingPickInfoForAxis(
                ME::ImGuiManager::gizmo_state_.activeAxis,
                ME::ImGuiManager::gizmo_state_.dragStartMouse,
                origin,
                axisX, axisY, axisZ,
                ringRadius,
                camView, camProj
            );

            ME::ImGuiManager::gizmo_state_.dragRingTangent = pickInfo.tangent;

            // cu�ntos p�xeles ocupa aproximadamente una vuelta del anillo proyectado
            // aproximaci�n: radio proyectado local * 2*pi
            float approxRadiusPixels = Distance2D(originScreen, pickInfo.closestPoint);
            float circumferencePixels = 6.28318530718f * approxRadiusPixels;

            if (circumferencePixels > 0.0001f)
                ME::ImGuiManager::gizmo_state_.rotatePixelsToDegrees = 360.0f / circumferencePixels;
            else
                ME::ImGuiManager::gizmo_state_.rotatePixelsToDegrees = 1.0f;
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        ME::ImGuiManager::gizmo_state_.activeAxis = ME::ImGuiManager::GizmoAxis::NONE;
    }

    if (ME::ImGuiManager::gizmo_state_.activeAxis != ME::ImGuiManager::GizmoAxis::NONE)
    {
        const float rotationSensitivity = 0.5f;
        ImVec2 currentMouse = ImGui::GetMousePos();
        ImVec2 mouseDelta(
            currentMouse.x - ME::ImGuiManager::gizmo_state_.dragStartMouse.x,
            currentMouse.y - ME::ImGuiManager::gizmo_state_.dragStartMouse.y
        );

        Vec3 rot = ME::ImGuiManager::gizmo_state_.dragStartRotation;

        float projectedPixels = ProjectMouseDeltaOnAxis(
            mouseDelta,
            ME::ImGuiManager::gizmo_state_.dragRingTangent
        );

        float deltaAngleDeg =
            projectedPixels *
            ME::ImGuiManager::gizmo_state_.rotatePixelsToDegrees *
            ME::ImGuiManager::gizmo_rotate_sensitivity_;

        using GizmoAxis = ME::ImGuiManager::GizmoAxis;

        if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::X) {
            rot.x_ += deltaAngleDeg;
        }
        else if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::Y) {
            rot.y_ += deltaAngleDeg;
        }
        else if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::Z) {
            rot.z_ += deltaAngleDeg;
        }

        tc->SetRotation(rot);
    }

    auto* draw = ImGui::GetForegroundDrawList();

    using GizmoAxis = ME::ImGuiManager::GizmoAxis;
    GizmoAxis active = ME::ImGuiManager::gizmo_state_.activeAxis;
    GizmoAxis hot = (active != GizmoAxis::NONE) ? active : ME::ImGuiManager::gizmo_state_.hotAxis;

    const float baseRadius = 40.0f;

    ImU32 xColor = (hot == GizmoAxis::X) ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 60, 60, 255);
    ImU32 yColor = (hot == GizmoAxis::Y) ? IM_COL32(255, 255, 0, 255) : IM_COL32(60, 255, 60, 255);
    ImU32 zColor = (hot == GizmoAxis::Z) ? IM_COL32(255, 255, 0, 255) : IM_COL32(80, 140, 255, 255);

    float xThickness = (hot == GizmoAxis::X) ? 4.5f : 2.5f;
    float yThickness = (hot == GizmoAxis::Y) ? 4.5f : 2.5f;
    float zThickness = (hot == GizmoAxis::Z) ? 4.5f : 2.5f;


    DrawProjectedOrientedRing(origin, axisY, axisZ, ringRadius, camView, camProj, xColor, xThickness); // X
    DrawProjectedOrientedRing(origin, axisX, axisZ, ringRadius, camView, camProj, yColor, yThickness); // Y
    DrawProjectedOrientedRing(origin, axisX, axisY, ringRadius, camView, camProj, zColor, zThickness); // Z
}

static void DrawTranslateGizmoOverlay(unsigned long selected_entity, const Mat4& camView, const Mat4& camProj)
{
    ME::ImGuiManager::gizmo_state_.visible = false;
    ME::ImGuiManager::gizmo_state_.hotAxis = ME::ImGuiManager::GizmoAxis::NONE;

    if (selected_entity == 999999) return;

    auto* tc = ME::ECS::GetComponent<ME::TransformComponent>(selected_entity);
    if (!tc) return;

    Vec3 origin = tc->Position();

    float gizmoSize = 1.0f;

    Vec3 axisX, axisY, axisZ;
    GetGizmoAxes(tc, axisX, axisY, axisZ);

    Vec3 xEndWorld(
        origin.x_ + axisX.x_ * gizmoSize,
        origin.y_ + axisX.y_ * gizmoSize,
        origin.z_ + axisX.z_ * gizmoSize
    );

    Vec3 yEndWorld(
        origin.x_ + axisY.x_ * gizmoSize,
        origin.y_ + axisY.y_ * gizmoSize,
        origin.z_ + axisY.z_ * gizmoSize
    );

    Vec3 zEndWorld(
        origin.x_ + axisZ.x_ * gizmoSize,
        origin.y_ + axisZ.y_ * gizmoSize,
        origin.z_ + axisZ.z_ * gizmoSize
    );

    ImVec2 originScreen, xEndScreen, yEndScreen, zEndScreen;

    bool originVisible = WorldToScreen(origin, camView, camProj, originScreen);
    bool xVisible = WorldToScreen(xEndWorld, camView, camProj, xEndScreen);
    bool yVisible = WorldToScreen(yEndWorld, camView, camProj, yEndScreen);
    bool zVisible = WorldToScreen(zEndWorld, camView, camProj, zEndScreen);

    if (!originVisible) return;

    ME::ImGuiManager::gizmo_state_.visible = true;
    ME::ImGuiManager::gizmo_state_.originScreen = originScreen;
    ME::ImGuiManager::gizmo_state_.axisXEnd = xEndScreen;
    ME::ImGuiManager::gizmo_state_.axisYEnd = yEndScreen;
    ME::ImGuiManager::gizmo_state_.axisZEnd = zEndScreen;

    // PICK ANTES DE DIBUJAR
    ImVec2 mousePos = ImGui::GetMousePos();
    ME::ImGuiManager::gizmo_state_.hotAxis = PickGizmoAxis(mousePos);

    auto* draw = ImGui::GetForegroundDrawList();

    using GizmoAxis = ME::ImGuiManager::GizmoAxis;
    GizmoAxis active = ME::ImGuiManager::gizmo_state_.activeAxis;
    GizmoAxis hot = (active != GizmoAxis::NONE) ? active : ME::ImGuiManager::gizmo_state_.hotAxis;

    auto length2D = [](const ImVec2& a, const ImVec2& b) -> float {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return sqrtf(dx * dx + dy * dy);
        };

    ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitX = length2D(originScreen, xEndScreen) / gizmoSize;
    ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitY = length2D(originScreen, yEndScreen) / gizmoSize;
    ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitZ = length2D(originScreen, zEndScreen) / gizmoSize;

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
    {
        if (ME::ImGuiManager::gizmo_state_.hotAxis != ME::ImGuiManager::GizmoAxis::NONE)
        {
            ME::ImGuiManager::gizmo_state_.activeAxis = ME::ImGuiManager::gizmo_state_.hotAxis;
            ME::ImGuiManager::gizmo_state_.dragStartMouse = ImGui::GetMousePos();
            ME::ImGuiManager::gizmo_state_.dragStartWorldPos = origin;
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        ME::ImGuiManager::gizmo_state_.activeAxis = ME::ImGuiManager::GizmoAxis::NONE;
    }

    if (ME::ImGuiManager::gizmo_state_.activeAxis != ME::ImGuiManager::GizmoAxis::NONE)
    {
        ImVec2 currentMouse = ImGui::GetMousePos();
        ImVec2 mouseDelta(
            currentMouse.x - ME::ImGuiManager::gizmo_state_.dragStartMouse.x,
            currentMouse.y - ME::ImGuiManager::gizmo_state_.dragStartMouse.y
        );

        Vec3 newPos = ME::ImGuiManager::gizmo_state_.dragStartWorldPos;

        Vec3 axisX, axisY, axisZ;
        GetGizmoAxes(tc, axisX, axisY, axisZ);

        using GizmoAxis = ME::ImGuiManager::GizmoAxis;

        if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::X)
        {
            ImVec2 axisDir = Normalize2D(ImVec2(
                xEndScreen.x - originScreen.x,
                xEndScreen.y - originScreen.y
            ));

            float projectedPixels = ProjectMouseDeltaOnAxis(mouseDelta, axisDir);
            float deltaWorld = 0.0f;

            if (ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitX > 0.0001f)
                deltaWorld = (projectedPixels / ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitX)
                * ME::ImGuiManager::gizmo_translate_sensitivity_;

            if (ME::ImGuiManager::current_gizmo_space_ == ME::ImGuiManager::GizmoSpace::WORLD) {
                newPos.x_ += deltaWorld;
            }
            else {
                newPos.x_ += axisX.x_ * deltaWorld;
                newPos.y_ += axisX.y_ * deltaWorld;
                newPos.z_ += axisX.z_ * deltaWorld;
            }
        }
        else if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::Y)
        {
            ImVec2 axisDir = Normalize2D(ImVec2(
                yEndScreen.x - originScreen.x,
                yEndScreen.y - originScreen.y
            ));

            float projectedPixels = ProjectMouseDeltaOnAxis(mouseDelta, axisDir);
            float deltaWorld = 0.0f;

            if (ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitY > 0.0001f)
                deltaWorld = (projectedPixels / ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitY)
                * ME::ImGuiManager::gizmo_translate_sensitivity_;

            if (ME::ImGuiManager::current_gizmo_space_ == ME::ImGuiManager::GizmoSpace::WORLD) {
                newPos.y_ += deltaWorld;
            }
            else {
                newPos.x_ += axisY.x_ * deltaWorld;
                newPos.y_ += axisY.y_ * deltaWorld;
                newPos.z_ += axisY.z_ * deltaWorld;
            }
        }
        else if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::Z)
        {
            ImVec2 axisDir = Normalize2D(ImVec2(
                zEndScreen.x - originScreen.x,
                zEndScreen.y - originScreen.y
            ));

            float projectedPixels = ProjectMouseDeltaOnAxis(mouseDelta, axisDir);
            float deltaWorld = 0.0f;

            if (ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitZ > 0.0001f)
                deltaWorld = (projectedPixels / ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitZ)
                * ME::ImGuiManager::gizmo_translate_sensitivity_;

            if (ME::ImGuiManager::current_gizmo_space_ == ME::ImGuiManager::GizmoSpace::WORLD) {
                newPos.z_ += deltaWorld;
            }
            else {
                newPos.x_ += axisZ.x_ * deltaWorld;
                newPos.y_ += axisZ.y_ * deltaWorld;
                newPos.z_ += axisZ.z_ * deltaWorld;
            }
        }

        tc->SetPosition(newPos);
    }

    ImU32 xColor = (hot == GizmoAxis::X) ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 60, 60, 255);
    ImU32 yColor = (hot == GizmoAxis::Y) ? IM_COL32(255, 255, 0, 255) : IM_COL32(60, 255, 60, 255);
    ImU32 zColor = (hot == GizmoAxis::Z) ? IM_COL32(255, 255, 0, 255) : IM_COL32(80, 140, 255, 255);

    float xThickness = (hot == GizmoAxis::X) ? 5.0f : 3.0f;
    float yThickness = (hot == GizmoAxis::Y) ? 5.0f : 3.0f;
    float zThickness = (hot == GizmoAxis::Z) ? 5.0f : 3.0f;

    if (xVisible) {
        draw->AddLine(originScreen, xEndScreen, xColor, xThickness);
        draw->AddCircleFilled(xEndScreen, 5.0f, xColor);
    }

    if (yVisible) {
        draw->AddLine(originScreen, yEndScreen, yColor, yThickness);
        draw->AddCircleFilled(yEndScreen, 5.0f, yColor);
    }

    if (zVisible) {
        draw->AddLine(originScreen, zEndScreen, zColor, zThickness);
        draw->AddCircleFilled(zEndScreen, 5.0f, zColor);
    }
}

static void DrawScaleGizmoOverlay(unsigned long selected_entity, const Mat4& camView, const Mat4& camProj)
{
    ME::ImGuiManager::gizmo_state_.visible = false;
    ME::ImGuiManager::gizmo_state_.hotAxis = ME::ImGuiManager::GizmoAxis::NONE;

    if (selected_entity == 999999) return;

    auto* tc = ME::ECS::GetComponent<ME::TransformComponent>(selected_entity);
    if (!tc) return;

    Mat4 wm = tc->WorldMatrix();
    Vec3 origin(wm.M_[12], wm.M_[13], wm.M_[14]);

    float gizmoSize = 1.0f;

    Vec3 axisX, axisY, axisZ;
    GetGizmoAxes(tc, axisX, axisY, axisZ);


    Vec3 xEndWorld(
        origin.x_ + axisX.x_ * gizmoSize,
        origin.y_ + axisX.y_ * gizmoSize,
        origin.z_ + axisX.z_ * gizmoSize
    );

    Vec3 yEndWorld(
        origin.x_ + axisY.x_ * gizmoSize,
        origin.y_ + axisY.y_ * gizmoSize,
        origin.z_ + axisY.z_ * gizmoSize
    );

    Vec3 zEndWorld(
        origin.x_ + axisZ.x_ * gizmoSize,
        origin.y_ + axisZ.y_ * gizmoSize,
        origin.z_ + axisZ.z_ * gizmoSize
    );

    ImVec2 originScreen, xEndScreen, yEndScreen, zEndScreen;

    bool originVisible = WorldToScreen(origin, camView, camProj, originScreen);
    bool xVisible = WorldToScreen(xEndWorld, camView, camProj, xEndScreen);
    bool yVisible = WorldToScreen(yEndWorld, camView, camProj, yEndScreen);
    bool zVisible = WorldToScreen(zEndWorld, camView, camProj, zEndScreen);

    if (!originVisible) return;

    ME::ImGuiManager::gizmo_state_.visible = true;
    ME::ImGuiManager::gizmo_state_.originScreen = originScreen;
    ME::ImGuiManager::gizmo_state_.axisXEnd = xEndScreen;
    ME::ImGuiManager::gizmo_state_.axisYEnd = yEndScreen;
    ME::ImGuiManager::gizmo_state_.axisZEnd = zEndScreen;

    auto length2D = [](const ImVec2& a, const ImVec2& b) -> float {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return sqrtf(dx * dx + dy * dy);
        };

    ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitX = length2D(originScreen, xEndScreen) / gizmoSize;
    ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitY = length2D(originScreen, yEndScreen) / gizmoSize;
    ME::ImGuiManager::gizmo_state_.pixelsPerWorldUnitZ = length2D(originScreen, zEndScreen) / gizmoSize;

    ImVec2 mousePos = ImGui::GetMousePos();
    ME::ImGuiManager::gizmo_state_.hotAxis = PickGizmoAxis(mousePos);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
    {
        if (ME::ImGuiManager::gizmo_state_.hotAxis != ME::ImGuiManager::GizmoAxis::NONE)
        {
            ME::ImGuiManager::gizmo_state_.activeAxis = ME::ImGuiManager::gizmo_state_.hotAxis;
            ME::ImGuiManager::gizmo_state_.dragStartMouse = ImGui::GetMousePos();
            ME::ImGuiManager::gizmo_state_.dragStartScale = tc->Scale();
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        ME::ImGuiManager::gizmo_state_.activeAxis = ME::ImGuiManager::GizmoAxis::NONE;
    }

    if (ME::ImGuiManager::gizmo_state_.activeAxis != ME::ImGuiManager::GizmoAxis::NONE)
    {
        ImVec2 currentMouse = ImGui::GetMousePos();
        ImVec2 mouseDelta(
            currentMouse.x - ME::ImGuiManager::gizmo_state_.dragStartMouse.x,
            currentMouse.y - ME::ImGuiManager::gizmo_state_.dragStartMouse.y
        );

        Vec3 sca = ME::ImGuiManager::gizmo_state_.dragStartScale;

        using GizmoAxis = ME::ImGuiManager::GizmoAxis;

        if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::X)
        {
            ImVec2 axisDir = Normalize2D(ImVec2(
                xEndScreen.x - originScreen.x,
                xEndScreen.y - originScreen.y
            ));

            float projectedPixels = ProjectMouseDeltaOnAxis(mouseDelta, axisDir);
            float deltaScale = projectedPixels * ME::ImGuiManager::gizmo_scale_sensitivity_;
            sca.x_ += deltaScale;
        }
        else if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::Y)
        {
            ImVec2 axisDir = Normalize2D(ImVec2(
                yEndScreen.x - originScreen.x,
                yEndScreen.y - originScreen.y
            ));

            float projectedPixels = ProjectMouseDeltaOnAxis(mouseDelta, axisDir);
            float deltaScale = projectedPixels * ME::ImGuiManager::gizmo_scale_sensitivity_;
            sca.y_ += deltaScale;
        }
        else if (ME::ImGuiManager::gizmo_state_.activeAxis == GizmoAxis::Z)
        {
            ImVec2 axisDir = Normalize2D(ImVec2(
                zEndScreen.x - originScreen.x,
                zEndScreen.y - originScreen.y
            ));

            float projectedPixels = ProjectMouseDeltaOnAxis(mouseDelta, axisDir);
            float deltaScale = projectedPixels * ME::ImGuiManager::gizmo_scale_sensitivity_;
            sca.z_ += deltaScale;
        }

        // evitar escalas negativas o demasiado peque�as
        if (sca.x_ < 0.05f) sca.x_ = 0.05f;
        if (sca.y_ < 0.05f) sca.y_ = 0.05f;
        if (sca.z_ < 0.05f) sca.z_ = 0.05f;

        tc->SetScale(sca);
    }

    auto* draw = ImGui::GetForegroundDrawList();

    using GizmoAxis = ME::ImGuiManager::GizmoAxis;
    GizmoAxis active = ME::ImGuiManager::gizmo_state_.activeAxis;
    GizmoAxis hot = (active != GizmoAxis::NONE) ? active : ME::ImGuiManager::gizmo_state_.hotAxis;

    ImU32 xColor = (hot == GizmoAxis::X) ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 60, 60, 255);
    ImU32 yColor = (hot == GizmoAxis::Y) ? IM_COL32(255, 255, 0, 255) : IM_COL32(60, 255, 60, 255);
    ImU32 zColor = (hot == GizmoAxis::Z) ? IM_COL32(255, 255, 0, 255) : IM_COL32(80, 140, 255, 255);

    float xThickness = (hot == GizmoAxis::X) ? 5.0f : 3.0f;
    float yThickness = (hot == GizmoAxis::Y) ? 5.0f : 3.0f;
    float zThickness = (hot == GizmoAxis::Z) ? 5.0f : 3.0f;

    if (xVisible) {
        draw->AddLine(originScreen, xEndScreen, xColor, xThickness);
        draw->AddRectFilled(ImVec2(xEndScreen.x - 5, xEndScreen.y - 5), ImVec2(xEndScreen.x + 5, xEndScreen.y + 5), xColor);
    }

    if (yVisible) {
        draw->AddLine(originScreen, yEndScreen, yColor, yThickness);
        draw->AddRectFilled(ImVec2(yEndScreen.x - 5, yEndScreen.y - 5), ImVec2(yEndScreen.x + 5, yEndScreen.y + 5), yColor);
    }

    if (zVisible) {
        draw->AddLine(originScreen, zEndScreen, zColor, zThickness);
        draw->AddRectFilled(ImVec2(zEndScreen.x - 5, zEndScreen.y - 5), ImVec2(zEndScreen.x + 5, zEndScreen.y + 5), zColor);
    }
}

void UpdateChildrenVisibility(unsigned long id, bool vis) {
    if (auto* ren = ME::ECS::GetComponent<ME::RenderizableComponent>(id)) {
        ren->SetVisibility(vis);
    }
    auto children_list = ME::ECS::GetChildren(id);
    if (!children_list.empty()) {
        for (unsigned long child_id : children_list) {
            UpdateChildrenVisibility(child_id, vis);
        }
    }
}

bool IsChildOf(unsigned long child, unsigned long parent)
{
    auto children = ME::ECS::GetChildren(parent);

    for (auto c : children)
    {
        if (c == child) return true;
        if (IsChildOf(child, c)) return true;
    }

    return false;
}

void DrawEntityParent(unsigned long id)
{
    ImGui::PushID((int)id);

    auto children = ME::ECS::GetChildren(id);

    ImGuiTreeNodeFlags flags = children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnArrow;

    if (selected_entity == id) flags |= ImGuiTreeNodeFlags_Selected;

    auto* tagc = ME::ECS::GetComponent<ME::TagComponent>(id);
    static char nameBuffer[128];
    strncpy(nameBuffer, tagc->getTag().c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    bool opened = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "%s", nameBuffer);

    // Click selection
    if (ImGui::IsItemClicked()) {
        selected_entity = id;
        add_component_pressed = false;
        if (auto* rc = ME::ECS::GetComponent<ME::RenderizableComponent>(id)) {
            current_selected_color_ = rc->GetColor();
        }
    }

    //---------------------------------------------------------
    // DRAG SOURCE
    //---------------------------------------------------------
    if (ImGui::BeginDragDropSource())
    {
        unsigned long payload = id;
        ImGui::SetDragDropPayload("ENTITY_ID", &payload, sizeof(unsigned long));

        ImGui::Text("Move %s", tagc->getTag().c_str());

        ImGui::EndDragDropSource();
    }

    //---------------------------------------------------------
    // DRAG TARGET
    //---------------------------------------------------------
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ID"))
        {
            unsigned long dropped = *(unsigned long*)payload->Data;

            if (dropped != id && !IsChildOf(id, dropped))
            {
                if (auto* t = ME::ECS::GetComponent<ME::TransformComponent>(dropped)) {
                    t->SetParent(id);
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    //---------------------------------------------------------
    // VISIBILITY BUTTON
    //---------------------------------------------------------
    auto* render = ME::ECS::GetComponent<ME::RenderizableComponent>(id);

    if (render)
    {
        ImGui::SameLine();

        const char* eyeIcon = render->Visible() ? "(0)" : "(-)";

        if (ImGui::Button(eyeIcon))
        {
            render->SetVisibility(!render->Visible());

            if (!children.empty())
            {
                for (unsigned long child_id : children)
                {
                    UpdateChildrenVisibility(child_id, render->Visible());
                }
            }
        }
    }

    //---------------------------------------------------------
    // CHILDREN RECURSION
    //---------------------------------------------------------
    if (opened)
    {
        for (unsigned long child : children)
        {
            DrawEntityParent(child);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

static const char* GizmoModeToString(ME::ImGuiManager::GizmoMode mode)
{
    switch (mode) {
    case ME::ImGuiManager::GizmoMode::TRANSLATE: return "Translate";
    case ME::ImGuiManager::GizmoMode::ROTATE:    return "Rotate";
    case ME::ImGuiManager::GizmoMode::SCALE:     return "Scale";
    default:                                     return "Unknown";
    }
}

static ME::ImGuiManager::GizmoMode NextGizmoMode(ME::ImGuiManager::GizmoMode mode)
{
    switch (mode) {
    case ME::ImGuiManager::GizmoMode::TRANSLATE: return ME::ImGuiManager::GizmoMode::ROTATE;
    case ME::ImGuiManager::GizmoMode::ROTATE:    return ME::ImGuiManager::GizmoMode::SCALE;
    case ME::ImGuiManager::GizmoMode::SCALE:     return ME::ImGuiManager::GizmoMode::TRANSLATE;
    default:                                     return ME::ImGuiManager::GizmoMode::TRANSLATE;
    }
}

static const char* GizmoSpaceToString(ME::ImGuiManager::GizmoSpace space)
{
    switch (space) {
    case ME::ImGuiManager::GizmoSpace::WORLD: return "World";
    case ME::ImGuiManager::GizmoSpace::LOCAL: return "Local";
    default:                                  return "Unknown";
    }
}

// --- RECURSIVE FUNCTIONS FOR IMGUI

namespace ME {
    bool ImGuiManager::show_performance_window_ = true;
    bool ImGuiManager::show_outline_window_ = true;
    bool ImGuiManager::show_details_window_ = true;
    bool ImGuiManager::show_camera_config_window_ = false;
    bool ImGuiManager::show_spawning_window_ = false;
    bool ImGuiManager::show_explorer_window_ = true;
    bool ImGuiManager::show_play_window_ = true;
    bool ImGuiManager::core_is_playing_ = false;
    bool ImGuiManager::core_is_paused_ = false;
    bool ImGuiManager::core_is_start_done_ = false;
    ME::ImGuiManager::GizmoState ME::ImGuiManager::gizmo_state_{};
    ME::ImGuiManager::GizmoMode ME::ImGuiManager::current_gizmo_mode_ = ME::ImGuiManager::GizmoMode::TRANSLATE;
    bool ME::ImGuiManager::show_gizmo_window_ = true;

    ME::ImGuiManager::GizmoSpace ME::ImGuiManager::current_gizmo_space_ =
        ME::ImGuiManager::GizmoSpace::WORLD;

    Input* ImGuiManager::input_ = nullptr;
    bool ImGuiManager::show_editor_bindings_window_ = false;

    float ME::ImGuiManager::gizmo_translate_sensitivity_ = 0.25f;
    float ME::ImGuiManager::gizmo_rotate_sensitivity_ = 0.15f;
    float ME::ImGuiManager::gizmo_scale_sensitivity_ = 0.01f;

    ImGuiManager::BasicShapeInfo ImGuiManager::selectedBasicShapeToSpawn_ = { ImGuiManager::BasicShape::CUBE , "Cube"};

    std::vector<ImGuiManager::BasicShapeInfo> ImGuiManager::basic_shapes_info_ = {
        {ImGuiManager::BasicShape::CUBE, "Cube"},
        {ImGuiManager::BasicShape::SPHERE, "Sphere"},
    };

    ImGuiManager::LightSpawnInfo ImGuiManager::selectedLightToSpawn_ = { ME::ImGuiManager::LightType::POINT, "Point" };

    std::vector<ImGuiManager::LightSpawnInfo> ImGuiManager::light_spawn_info_ = {
        { ME::ImGuiManager::LightType::AMBIENT, "Ambient" },
        { ME::ImGuiManager::LightType::POINT, "Point" },
        { ME::ImGuiManager::LightType::DIRECTIONAL, "Directional" },
        { ME::ImGuiManager::LightType::SPOT, "Spot" },
    };

    void ImGuiManager::RegisterWindow(GuiCallback cb) {
        s_callbacks_.push_back(std::move(cb));
    }

    void ImGuiManager::RegisterMainWindow() {
        // GET MAIN CAMERA
        auto& cam = ME::ECS::GetComponentList<ME::CameraComponent>().at(0).second;

        GuiCallback cb = [&]() {
            auto LightTypeToString = [](ME::LightType t) -> const char* {
                switch (t) {
                case ME::LightType::AMBIENT:     return "AMBIENT";
                case ME::LightType::POINT:       return "POINT";
                case ME::LightType::DIRECTIONAL: return "DIRECTIONAL";
                case ME::LightType::SPOT:        return "SPOT";
                default:                         return "UNKNOWN";
                }
                };
            auto LightTypeFromIndex = [](int idx) -> ME::LightType {
                switch (idx) {
                case 0: return ME::LightType::AMBIENT;
                case 1: return ME::LightType::POINT;
                case 2: return ME::LightType::DIRECTIONAL;
                case 3: return ME::LightType::SPOT;
                default: return ME::LightType::POINT;
                }
                };
            auto LightTypeToIndex = [](ME::LightType t) -> int {
                switch (t) {
                case ME::LightType::AMBIENT:     return 0;
                case ME::LightType::POINT:       return 1;
                case ME::LightType::DIRECTIONAL: return 2;
                case ME::LightType::SPOT:        return 3;
                default:                         return 1;
                }
                };

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Nuevo", "Ctrl+N")) { /* Acci�n */ }
                    if (ImGui::MenuItem("Abrir", "Ctrl+O")) { /* Acci�n */ }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Windows")) {
                    ImGui::MenuItem("Performance", NULL, &ImGuiManager::show_performance_window_);
                    ImGui::MenuItem("Details", NULL, &ImGuiManager::show_details_window_);
                    ImGui::MenuItem("Outliner", NULL, &ImGuiManager::show_outline_window_);
                    ImGui::MenuItem("Fly cam config", NULL, &ImGuiManager::show_camera_config_window_);
                    ImGui::MenuItem("Spawn entities", NULL, &ImGuiManager::show_spawning_window_);
                    ImGui::MenuItem("Explorer", NULL, &ImGuiManager::show_explorer_window_);
                    ImGui::MenuItem("Editor Input Bindings", NULL, &ImGuiManager::show_editor_bindings_window_);
                    ImGui::MenuItem("Gizmo Controls", NULL, &ImGuiManager::show_gizmo_window_);
                    if (ImGui::MenuItem("Save Scene")) {
                        ME::XMLManager::SceneToFile();
                    };

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            if (ImGuiManager::show_gizmo_window_) {
                ImGui::Begin("Gizmo mode", &ImGuiManager::show_gizmo_window_);
                ImGui::Text("Current Mode: %s", GizmoModeToString(ImGuiManager::current_gizmo_mode_));
                ImGui::Separator();

                const char* spaceItems[] = { "World", "Local" };
                int currentSpace = (ImGuiManager::current_gizmo_space_ == ImGuiManager::GizmoSpace::WORLD) ? 0 : 1;
                if (ImGui::Combo("Transform Space", &currentSpace, spaceItems, 2)) {
                    ImGuiManager::current_gizmo_space_ =
                        (currentSpace == 0) ? ImGuiManager::GizmoSpace::WORLD : ImGuiManager::GizmoSpace::LOCAL;
                }

                ImGui::Separator();
                ImGui::DragFloat("Translate Sensitivity", &ImGuiManager::gizmo_translate_sensitivity_, 0.01f, 0.01f, 5.0f);
                ImGui::DragFloat("Rotate Sensitivity", &ImGuiManager::gizmo_rotate_sensitivity_, 0.01f, 0.01f, 5.0f);
                ImGui::DragFloat("Scale Sensitivity", &ImGuiManager::gizmo_scale_sensitivity_, 0.001f, 0.001f, 1.0f);
                ImGui::End();
            }

            if (ImGuiManager::show_editor_bindings_window_) {
                ImGui::Begin("Editor Input Bindings", &ImGuiManager::show_editor_bindings_window_);

                if (ImGuiManager::input_ == nullptr) {
                    ImGui::Text("No Input system connected.");
                }
                else {
                    const auto& bindings = ImGuiManager::input_->GetEditorBindings();

                    if (bindings.empty()) {
                        ImGui::Text("No editor bindings registered.");
                    }
                    else if (ImGui::BeginTable("EditorBindingsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                        ImGui::TableSetupColumn("Action");
                        ImGui::TableSetupColumn("Keys");
                        ImGui::TableHeadersRow();

                        for (const auto& [action, keys] : bindings) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", ME::Input::ActionToString(action));

                            ImGui::TableSetColumnIndex(1);

                            std::string joinedKeys;
                            for (size_t i = 0; i < keys.size(); ++i) {
                                if (i > 0) joinedKeys += ", ";
                                joinedKeys += ME::Input::KeyToString(keys[i]);
                            }

                            ImGui::Text("%s", joinedKeys.c_str());
                        }

                        ImGui::EndTable();
                    }
                }

                ImGui::End();
            }

            if (ImGuiManager::show_performance_window_) {
                ImGui::Begin("Performance", &ImGuiManager::show_performance_window_);

                ImGuiIO& io = ImGui::GetIO();
                ImGui::Text("Rendimiento: %.1f FPS", io.Framerate);
                ImGui::Text("Delta Time: %.3f ms/frame", io.DeltaTime * 1000.0f);

                ImGui::Separator();

                static float values[90] = { 0 };
                static int values_offset = 0;
                values[values_offset] = io.Framerate;
                values_offset = (values_offset + 1) % 90;
                ImGui::PlotLines("##FPS", values, 90, values_offset, "Historical FPS", 0.0f, 120.0f, ImVec2(0, 80));
                ImGui::End();
            }

            if (ImGuiManager::show_play_window_) {
                ImGui::Begin("Player", &ImGuiManager::show_play_window_);

                float spacingY = ImGui::GetStyle().ItemSpacing.y;
                float availWidth = ImGui::GetContentRegionAvail().x;
                float availHeight = ImGui::GetContentRegionAvail().y;
                ImVec2 buttonSize = ImVec2(availWidth, (availHeight - (2 * spacingY)) / 3.0f);
                ImVec4 activeColor = ImVec4(0.20f, 0.45f, 0.70f, 1.0f);

                // BOT�N PLAY
                bool tmpPlaying = ImGuiManager::core_is_playing_;
                if (tmpPlaying) ImGui::BeginDisabled();
                if (ImGui::Button("Play", buttonSize)) {
                    ImGuiManager::core_is_playing_ = true;
                    ImGuiManager::core_is_paused_ = false;
                    ImGuiManager::core_is_start_done_ = false;
                    tmp_serialized_content = ME::XMLManager::SaveScene(ME::Core::CurrentSceneName);

                    // playmode style
                    ImGui::StyleColorsDark(); 
                    ImGuiStyle& style = ImGui::GetStyle();
                    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 0.09f, 0.12f, 1.00f);
                    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.45f, 0.70f, 1.00f);
                }
                if (tmpPlaying) ImGui::EndDisabled();

                // BOT�N PAUSE
                bool tmpPaused = ImGuiManager::core_is_paused_;
                if (!tmpPlaying) ImGui::BeginDisabled();
                if (tmpPaused) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                if (ImGui::Button("Pause", buttonSize)) {
                    if (ImGuiManager::core_is_playing_) {
                        ImGuiManager::core_is_paused_ = !ImGuiManager::core_is_paused_;
                    }
                }
                if (tmpPaused) ImGui::PopStyleColor();

                // BOT�N STOP
                if (ImGui::Button("Stop", buttonSize)) {
                    ImGuiManager::core_is_playing_ = false;
                    ImGuiManager::core_is_paused_ = false;
                    ME::XMLManager::LoadScene(tmp_serialized_content);
                    tmp_serialized_content = "";
                    selected_entity = 999999;
                    ImGui::StyleColorsDark();
                }
                if (!tmpPlaying) ImGui::EndDisabled();

                ImGui::End();
            }

            if (ImGuiManager::show_outline_window_) {
                if (ImGui::Begin("Entity Hierarchy", &ImGuiManager::show_outline_window_)) {
                    auto& transforms = ME::ECS::GetComponentList<ME::TransformComponent>();
                    float row_height = ImGui::GetFrameHeight();

                    for (auto& pair : transforms) {
                        unsigned long id = pair.first;
                        auto& transform = pair.second;

                        if (transform.GetParent() == 0) {
                            DrawEntityParent(id);
                        }
                    }

                    //----------------------------------------------------
                    // ROOT DROP AREA (zona vac�a del panel)
                    //----------------------------------------------------
                    float remaining = ImGui::GetContentRegionAvail().y;
                    if (remaining < 50.0f) remaining = 50.0f; // asegurar �rea m�nima

                    ImGui::InvisibleButton("HierarchyRootDrop", ImVec2(-1, remaining));

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ID"))
                        {
                            unsigned long dropped = *(unsigned long*)payload->Data;

                            auto* t = ME::ECS::GetComponent<ME::TransformComponent>(dropped);
                            if (t) t->SetParent(0); // ROOT
                        }

                        ImGui::EndDragDropTarget();
                    }

                    ImGui::End();
                }
            }

            if (ImGuiManager::show_details_window_) {
                char window_title[128];
                if (selected_entity != 999999) {
                    auto* tagc = ME::ECS::GetComponent<ME::TagComponent>(selected_entity);
                    sprintf(window_title, "Details: %s###DetailsTab", tagc->getTag().c_str());
                }
                else {
                    sprintf(window_title, "Details###DetailsTab");
                }
                ImGui::Begin(window_title, &ImGuiManager::show_details_window_);

                if (!add_component_pressed) {
                    if (selected_entity != 999999) {
                        // 0) Tag
                        auto* tagc = ME::ECS::GetComponent<ME::TagComponent>(selected_entity);
                        static char nameBuffer[128];
                        strncpy(nameBuffer, tagc->getTag().c_str(), sizeof(nameBuffer));
                        nameBuffer[sizeof(nameBuffer) - 1] = '\0'; // asegurarse de terminar el string

                        if (ImGui::InputText("##EntityName", nameBuffer, sizeof(nameBuffer))) {
                            tagc->setTag(std::string(nameBuffer));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Delete Entity")) {
                            ME::ECS::RemoveEntity(selected_entity);
                            selected_entity = 999999;
                            add_component_pressed = false;
                            ImGui::End(); // Cerramos la ventana de este frame
                            return;
                        }

                        // 1) Transform
                        if (ME::TransformComponent* tc = ME::ECS::GetComponent<ME::TransformComponent>(selected_entity)) {
                            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                                if (ImGui::Button("Remove Transform Component")) {
                                    pending_update_component = 5;
                                }

                                Vec3 pos = tc->Position();
                                Vec3 rot = tc->Rotation();
                                Vec3 sca = tc->Scale();

                                if (ImGui::DragFloat3("Position", &pos.x_, 0.1f, -9999.9f, 9999.9f)) tc->SetPosition(pos);
                                if (ImGui::DragFloat3("Rotation", &rot.x_, 0.1f, -9999.9f, 9999.9f)) tc->SetRotation(rot);
                                if (ImGui::DragFloat3("Scale", &sca.x_, 0.1f, -9999.9f, 9999.9f)) tc->SetScale(sca);

                                ImGui::Dummy(ImVec2(0, 15));
                            }
                        }

                        // 2) Renderizable
                        if (ME::RenderizableComponent* rc = ME::ECS::GetComponent<ME::RenderizableComponent>(selected_entity)) {
                            if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen)) {

                                if (ImGui::Button("Remove Render Component")) {
                                    pending_update_component = 6;
                                }

                                bool vis = rc->Visible();
                                if (ImGui::Checkbox("Visible", &vis)) rc->SetVisibility(vis);
                                ImGui::SameLine();
                                bool ca = rc->IsColorActive();
                                if (ImGui::Checkbox("Color Active", &ca)) rc->SetColorActive(ca);

                                if (!ca) {
                                    // --- ZONA DE DROP PARA TEXTURA ---
                                    ImGui::Text("Texture file:");
                                    ImGui::SameLine();

                                    float dropWidth = ImGui::GetContentRegionAvail().x;
                                    float dropHeight = 25.0f;
                                    ImVec2 dropPosMin = ImGui::GetCursorScreenPos();
                                    ImVec2 dropSize = ImVec2(dropWidth, dropHeight);

                                    // Bot�n invisible para el target
                                    ImGui::InvisibleButton("TextureDropTarget", dropSize);

                                    // L�gica Visual
                                    ImU32 bgColor = ImGui::IsItemHovered() ? IM_COL32(100, 150, 250, 150) : IM_COL32(45, 45, 45, 255);
                                    ImGui::GetWindowDrawList()->AddRectFilled(dropPosMin, ImVec2(dropPosMin.x + dropSize.x, dropPosMin.y + dropSize.y), bgColor, 3.0f);

                                    // Texto din�mico: mostrar el path actual o "Drop .obj here"
                                    std::string texturepath = rc->GetTexurePath(); // Aseg�rate de tener este getter en rc
                                    std::string boxText = texturepath.empty() ? "Drop .png here" : std::filesystem::path(texturepath).filename().string();
                                    ImGui::GetWindowDrawList()->AddText(ImVec2(dropPosMin.x + 10, dropPosMin.y + 4), IM_COL32(200, 200, 200, 255), boxText.c_str());
                                }

                                if (ca) {
                                    if (ImGui::DragFloat3("Diffuse Color", &current_selected_color_.x_, -0.01f, 0.0f, 1.0f)) {
                                        rc->SetColor(current_selected_color_);
                                    }
                                }

                                ImGui::Separator();

                                // --- ZONA DE DROP PARA MESH ---
                                ImGui::Text("Mesh file:");
                                ImGui::SameLine();

                                float dropWidth = ImGui::GetContentRegionAvail().x;
                                float dropHeight = 25.0f;
                                ImVec2 dropPosMin = ImGui::GetCursorScreenPos();
                                ImVec2 dropSize = ImVec2(dropWidth, dropHeight);

                                // Bot�n invisible para el target
                                ImGui::InvisibleButton("MeshDropTarget", dropSize);

                                // L�gica Visual
                                ImU32 bgColor = ImGui::IsItemHovered() ? IM_COL32(100, 150, 250, 150) : IM_COL32(45, 45, 45, 255);
                                ImGui::GetWindowDrawList()->AddRectFilled(dropPosMin, ImVec2(dropPosMin.x + dropSize.x, dropPosMin.y + dropSize.y), bgColor, 3.0f);

                                // Texto din�mico: mostrar el path actual o "Drop .obj here"
                                std::string meshPath = rc->GetMeshPath(); // Aseg�rate de tener este getter en rc
                                std::string boxText = meshPath.empty() ? "Drop .obj here" : std::filesystem::path(meshPath).filename().string();
                                ImGui::GetWindowDrawList()->AddText(ImVec2(dropPosMin.x + 10, dropPosMin.y + 4), IM_COL32(200, 200, 200, 255), boxText.c_str());

                                // L�gica de Drag & Drop
                                if (ImGui::BeginDragDropTarget()) {
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER_RESOURCE")) {
                                        std::string pathStr = (char*)payload->Data;
                                        std::filesystem::path filePath(pathStr);

                                        if (filePath.extension() == ".obj") {
                                            // Usamos el JobSystem a trav�s de tu referencia en XMLManager
                                            if (ME::XMLManager::js_ref_) {
                                                auto task = [pathStr]() {
                                                    return ME::MeshLoader::LoadObjPositionsFlat(pathStr);
                                                    };

                                                // A�adimos a la cola de mallas pendientes que ya gestionas en cada frame
                                                ME::XMLManager::pending_meshes_.push_back({
                                                    selected_entity,
                                                    ME::XMLManager::js_ref_->tEnqueue(std::move(task)),
                                                    pathStr
                                                    });
                                            }
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }

                                ImGui::Text("VAO: %u", rc->GetVAO());
                                ImGui::Text("IndexCount: %u", rc->GetIndexCount());
                                ImGui::Dummy(ImVec2(0, 15));
                            }
                        }

                        // 3) Light
                        if (ME::LightComponent* lc = ME::ECS::GetComponent<ME::LightComponent>(selected_entity)) {
                            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {

                                if (ImGui::Button("Remove Light Component")) {
                                    pending_update_component = 7;
                                }

                                auto LightTypeToIndex = [](ME::LightType t) -> int {
                                    switch (t) {
                                    case ME::LightType::AMBIENT:     return 0;
                                    case ME::LightType::POINT:       return 1;
                                    case ME::LightType::DIRECTIONAL: return 2;
                                    case ME::LightType::SPOT:        return 3;
                                    default:                         return 1;
                                    }
                                    };

                                auto LightTypeFromIndex = [](int idx) -> ME::LightType {
                                    switch (idx) {
                                    case 0: return ME::LightType::AMBIENT;
                                    case 1: return ME::LightType::POINT;
                                    case 2: return ME::LightType::DIRECTIONAL;
                                    case 3: return ME::LightType::SPOT;
                                    default: return ME::LightType::POINT;
                                    }
                                    };

                                ImGui::SeparatorText("General");

                                bool en = lc->Enabled();
                                if (ImGui::Checkbox("Enabled", &en)) {
                                    lc->SetEnabled(en);
                                }

                                const char* types[] = { "AMBIENT", "POINT", "DIRECTIONAL", "SPOT" };
                                int tIdx = LightTypeToIndex(lc->Type());
                                if (ImGui::Combo("Type", &tIdx, types, 4)) {
                                    lc->SetType(LightTypeFromIndex(tIdx));
                                }
                                // =========================================================
                                // SHADOWS
                                // =========================================================
                                ImGui::SeparatorText("Shadows");

                                bool castsShadows = lc->CastsShadows();
                                if (ImGui::Checkbox("Casts Shadows", &castsShadows)) {
                                    lc->SetCastsShadows(castsShadows);
                                }

                                if (lc->Type() == ME::LightType::DIRECTIONAL || lc->Type() == ME::LightType::SPOT) {
                                    float shadowNear = lc->ShadowNear();
                                    float shadowFar = lc->ShadowFar();

                                    if (ImGui::DragFloat("Shadow Near", &shadowNear, 0.01f, 0.001f, 100.0f)) {
                                        if (shadowNear < 0.001f) shadowNear = 0.001f;
                                        if (shadowNear >= shadowFar) shadowNear = shadowFar - 0.001f;
                                        lc->SetShadowNear(shadowNear);
                                    }

                                    if (ImGui::DragFloat("Shadow Far", &shadowFar, 0.1f, 0.01f, 5000.0f)) {
                                        if (shadowFar <= shadowNear) shadowFar = shadowNear + 0.001f;
                                        lc->SetShadowFar(shadowFar);
                                    }
                                }

                                // =========================================================
                                // POSITION CONTROL
                                // =========================================================
                                // En tu renderer, la posici�n efectiva sale del Transform si existe.
                                // As� que aqu� editamos el Transform cuando est� presente.
                                ME::TransformComponent* tc_light = ME::ECS::GetComponent<ME::TransformComponent>(selected_entity);



                                if (lc->Type() == ME::LightType::POINT || lc->Type() == ME::LightType::SPOT) {
                                    ImGui::SeparatorText("Position");

                                    if (tc_light) {
                                        Vec3 pos = tc_light->Position();
                                        float p[3] = { pos.x_, pos.y_, pos.z_ };

                                        if (ImGui::DragFloat3("World Position", p, 0.1f, -9999.0f, 9999.0f)) {
                                            tc_light->SetPosition(Vec3(p[0], p[1], p[2]));
                                            lc->SetPos(Vec3(p[0], p[1], p[2])); // sync visual/debug
                                        }

                                        ImGui::TextDisabled("Using TransformComponent position");
                                    }
                                    else {
                                        Vec3 pos = lc->Pos();
                                        float p[3] = { pos.x_, pos.y_, pos.z_ };

                                        if (ImGui::DragFloat3("Light Position", p, 0.1f, -9999.0f, 9999.0f)) {
                                            lc->SetPos(Vec3(p[0], p[1], p[2]));
                                        }

                                        ImGui::TextDisabled("No TransformComponent: using LightComponent::Pos()");
                                    }
                                }

                                // =========================================================
                                // DIRECTION CONTROL
                                // =========================================================
                                if (lc->Type() == ME::LightType::DIRECTIONAL || lc->Type() == ME::LightType::SPOT) {
                                    ImGui::SeparatorText("Direction");

                                    Vec3 dir = lc->Dir();
                                    float d[3] = { dir.x_, dir.y_, dir.z_ };

                                    if (ImGui::DragFloat3("Direction", d, 0.01f, -1.0f, 1.0f)) {
                                        float len = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
                                        if (len > 1e-6f) {
                                            d[0] /= len;
                                            d[1] /= len;
                                            d[2] /= len;
                                        }
                                        lc->SetDir(Vec3(d[0], d[1], d[2]));
                                    }

                                    if (ImGui::Button("Normalize Direction")) {
                                        float len = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
                                        if (len > 1e-6f) {
                                            d[0] /= len;
                                            d[1] /= len;
                                            d[2] /= len;
                                            lc->SetDir(Vec3(d[0], d[1], d[2]));
                                        }
                                    }

                                    ImGui::SameLine();
                                    if (ImGui::Button("Point Down")) {
                                        lc->SetDir(Vec3(0.0f, -1.0f, 0.0f));
                                    }

                                    ImGui::SameLine();
                                    if (ImGui::Button("Point Forward")) {
                                        lc->SetDir(Vec3(0.0f, 0.0f, -1.0f));
                                    }
                                }

                                // =========================================================
                                // COLORS
                                // =========================================================
                                ImGui::SeparatorText("Colors");

                                Vec3 diff = lc->DiffColor();
                                float dcol[3] = { diff.x_, diff.y_, diff.z_ };
                                if (ImGui::ColorEdit3("Diffuse", dcol)) {
                                    lc->SetDiffColor(Vec3(dcol[0], dcol[1], dcol[2]));
                                }

                                Vec3 spec = lc->SpecColor();
                                float scol[3] = { spec.x_, spec.y_, spec.z_ };
                                if (ImGui::ColorEdit3("Specular", scol)) {
                                    lc->SetSpecColor(Vec3(scol[0], scol[1], scol[2]));
                                }

                                // =========================================================
                                // LIGHT RESPONSE
                                // =========================================================
                                ImGui::SeparatorText("Response");

                                float shininess = lc->Shininess();
                                if (ImGui::DragFloat("Shininess", &shininess, 0.25f, 1.0f, 256.0f)) {
                                    lc->SetShininess(shininess);
                                }

                                float specInt = lc->SpecIntensity();
                                if (ImGui::DragFloat("Spec Intensity", &specInt, 0.01f, 0.0f, 50.0f)) {
                                    lc->SetSpecIntensity(specInt);
                                }

                                // =========================================================
                                // ATTENUATION
                                // =========================================================
                                if (lc->Type() == ME::LightType::POINT || lc->Type() == ME::LightType::SPOT) {
                                    ImGui::SeparatorText("Attenuation");

                                    float cAtt = lc->ConstantAtt();
                                    float lAtt = lc->LinearAtt();
                                    float qAtt = lc->QuadAtt();

                                    if (ImGui::DragFloat("Constant", &cAtt, 0.01f, 0.0f, 10.0f)) lc->SetConstantAtt(cAtt);
                                    if (ImGui::DragFloat("Linear", &lAtt, 0.001f, 0.0f, 5.0f)) lc->SetLinearAtt(lAtt);
                                    if (ImGui::DragFloat("Quadratic", &qAtt, 0.001f, 0.0f, 5.0f)) lc->SetQuadAtt(qAtt);

                                    if (ImGui::Button("No Attenuation")) {
                                        lc->SetConstantAtt(1.0f);
                                        lc->SetLinearAtt(0.0f);
                                        lc->SetQuadAtt(0.0f);
                                    }

                                    ImGui::SameLine();
                                    if (ImGui::Button("Default Attenuation")) {
                                        lc->SetConstantAtt(1.0f);
                                        lc->SetLinearAtt(0.09f);
                                        lc->SetQuadAtt(0.032f);
                                    }
                                }

                                // =========================================================
                                // SPOT CONE
                                // =========================================================
                                if (lc->Type() == ME::LightType::SPOT) {
                                    ImGui::SeparatorText("Spot Cone");

                                    float inner = lc->CutOff();
                                    float outer = lc->OuterCutOff();

                                    if (ImGui::DragFloat("Inner CutOff (deg)", &inner, 0.1f, 0.0f, 89.0f)) {
                                        if (inner > outer) inner = outer;
                                        lc->SetCutOff(inner);
                                    }

                                    if (ImGui::DragFloat("Outer CutOff (deg)", &outer, 0.1f, 0.0f, 89.0f)) {
                                        if (outer < inner) outer = inner;
                                        lc->SetOuterCutOff(outer);
                                    }

                                    if (ImGui::Button("Cone 15 / 20")) {
                                        lc->SetCutOff(15.0f);
                                        lc->SetOuterCutOff(20.0f);
                                    }

                                    ImGui::SameLine();
                                    if (ImGui::Button("Cone 25 / 35")) {
                                        lc->SetCutOff(25.0f);
                                        lc->SetOuterCutOff(35.0f);
                                    }

                                    ImGui::SameLine();
                                    if (ImGui::Button("Cone 35 / 45")) {
                                        lc->SetCutOff(35.0f);
                                        lc->SetOuterCutOff(45.0f);
                                    }

                                    // Informaci�n �til de depuraci�n
                                    const float PI = 3.14159265358979323846f;
                                    float innerCos = cosf(inner * PI / 180.0f);
                                    float outerCos = cosf(outer * PI / 180.0f);
                                    float shadowFov = outer * 2.0f;

                                    ImGui::Text("Inner cos: %.4f", innerCos);
                                    ImGui::Text("Outer cos: %.4f", outerCos);
                                    ImGui::Text("Shadow FOV (approx): %.2f deg", shadowFov);

                                    if (outer <= 0.5f) {
                                        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Outer cutoff is too narrow.");
                                    }
                                    if (shadowFov > 120.0f) {
                                        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Very wide cone: shadows may lose detail.");
                                    }
                                }

                                ImGui::Dummy(ImVec2(0, 15));
                            }
                        }

                        // 4) Camera (si un entity puede tener c�mara y no es la �cam� local)
                        if (ME::CameraComponent* cc = ME::ECS::GetComponent<ME::CameraComponent>(selected_entity)) {
                            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                                if (ImGui::Button("Remove Camera Component")) {
                                    pending_update_component = 8;
                                }

                                ImGui::DragFloat3("Position", &cc->position.x_, 0.1f, -9999.9f, 9999.9f);
                                ImGui::SliderFloat("FOV", &cc->fov_deg, 30.0f, 150.0f);
                                ImGui::DragFloat("Move speed", &cc->move_speed, 0.1f, 0.0f, 200.0f);
                                ImGui::DragFloat("Mouse sens", &cc->mouse_sens, 0.01f, 0.0f, 10.0f);
                                ImGui::DragFloat("Near", &cc->near_plane, 0.01f, 0.001f, 100.0f);
                                ImGui::DragFloat("Far", &cc->far_plane, 1.0f, 1.0f, 100000.0f);
                                ImGui::Checkbox("Active", &cc->active);
                                ImGui::SliderFloat("Pitch", &cc->pitch_deg, -90.0f, 90.0f);
                                ImGui::SliderFloat("Yaw", &cc->yaw_deg, -180.0f, 180.0f);
                            }
                            ImGui::Dummy(ImVec2(0, 15));
                        }

                        // 5) Script (placeholder si existe)
                        if (ME::ScriptComponent* sc = ME::ECS::GetComponent<ME::ScriptComponent>(selected_entity)) {
                            if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {

                                // 1. Bot�n para eliminar el componente (Ocupa todo el ancho)
                                if (ImGui::Button("Remove Script Component")) {
                                    pending_update_component = 9;
                                }

                                ImGui::Dummy(ImVec2(0, 5));

                                ImGui::Text(".lua script:");
                                ImGui::SameLine();

                                float dropWidth = ImGui::GetContentRegionAvail().x;
                                float dropHeight = 25.0f;
                                ImVec2 dropPosMin = ImGui::GetCursorScreenPos();
                                ImVec2 dropSize = ImVec2(dropWidth, dropHeight);

                                ImGui::InvisibleButton("ScriptDropTarget##script", dropSize);

                                ImU32 bgColor = ImGui::IsItemHovered() ? IM_COL32(100, 150, 250, 150) : IM_COL32(45, 45, 45, 255);
                                ImGui::GetWindowDrawList()->AddRectFilled(dropPosMin, ImVec2(dropPosMin.x + dropSize.x, dropPosMin.y + dropSize.y), bgColor, 3.0f);

                                std::string scriptPath = sc->getCurrentScriptPath();
                                std::string boxText = scriptPath.empty() ? "Drop here" : std::filesystem::path(scriptPath).filename().string();

                                ImGui::GetWindowDrawList()->AddText(ImVec2(dropPosMin.x + 10, dropPosMin.y + 4), IM_COL32(200, 200, 200, 255), boxText.c_str());

                                // 3. L�gica de Drag & Drop (Sobrescribe al soltar uno nuevo)
                                if (ImGui::BeginDragDropTarget()) {
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER_RESOURCE")) {
                                        std::string pathStr = std::string((char*)payload->Data);
                                        std::filesystem::path filePath(pathStr);

                                        if (filePath.extension() == ".lua") {
                                            sc->setCodeByPath(pathStr);
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }

                                ImGui::Dummy(ImVec2(0, 10));
                            }
                        }

                        float buttonWidth = 250.0f;
                        ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
                        ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
                        float availWidth = regionMax.x - regionMin.x;
                        float xPos = regionMin.x + (availWidth - buttonWidth) * 0.5f;
                        if (xPos < regionMin.x) xPos = regionMin.x;

                        // mueve el cursor horizontal para centrar
                        ImGui::SetCursorPosX(xPos);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));
                        bool clicked = ImGui::Button("Add Component", ImVec2(buttonWidth, 0));
                        if (clicked) {
                            component_flags = 0;
                            if (ME::ECS::GetComponent<ME::TransformComponent>(selected_entity) == nullptr) component_flags |= 1 << 0; // bit 0 = falta Transform
                            if (ME::ECS::GetComponent<ME::RenderizableComponent>(selected_entity) == nullptr) component_flags |= 1 << 1;
                            if (ME::ECS::GetComponent<ME::LightComponent>(selected_entity) == nullptr) component_flags |= 1 << 2;
                            if (ME::ECS::GetComponent<ME::CameraComponent>(selected_entity) == nullptr) component_flags |= 1 << 3;
                            if (ME::ECS::GetComponent<ME::ScriptComponent>(selected_entity) == nullptr) component_flags |= 1 << 4;
                            add_component_pressed = true;
                        }
                        ImGui::PopStyleVar();
                        ImGui::Dummy(ImVec2(0, 15));
                    }
                    else {
                        ImGui::Text("Select an entity in an outliner to see details");
                    }
                }
                else {
                    if (ImGui::Button("<")) {
                        add_component_pressed = false;
                    }

                    float buttonWidth = 250.0f;
                    // calcula el ancho disponible en la ventana p�blica
                    ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
                    ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
                    float availWidth = regionMax.x - regionMin.x;
                    float xPos = regionMin.x + (availWidth - buttonWidth) * 0.5f;
                    if (xPos < regionMin.x) xPos = regionMin.x;
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));

                    std::string texts[5] = { "Add Transform Component", "Add Renderizable Component",
                        "Add Light Component", "Add Camera Component", "Add Script Component" };

                    for (int i = 0; i < 5; i++) {
                        ImGui::SetCursorPosX(xPos);

                        // Si no se puede a�adir, deshabilitamos el bot�n
                        bool canAdd = (component_flags & (1 << i)) != 0;

                        if (!canAdd) ImGui::BeginDisabled();

                        if (ImGui::Button(texts[i].c_str(), ImVec2(buttonWidth, 0))) {
                            pending_update_component = i;
                            add_component_pressed = false;
                        }

                        if (!canAdd) ImGui::EndDisabled();
                    }

                    ImGui::PopStyleVar();
                }

                ImGui::End();
            }

            if (ImGuiManager::show_camera_config_window_) {
                ImGui::Begin("Fly cam configuration", &ImGuiManager::show_camera_config_window_);

                if (ImGui::Button("Reset to defaults")) {
                    cam.fov_deg = 60.0f;
                    cam.move_speed = 6.0f;
                    cam.mouse_sens = 0.12f;
                    cam.near_plane = 0.1f;
                    cam.far_plane = 200.0f;
                    cam.active = true;
                    cam.position = Vec3(0, 0, 0);
                }
                ImGui::SliderFloat("FOV", &cam.fov_deg, 30.0f, 150.0f, "%.2f");
                ImGui::DragFloat("Move speed", &cam.move_speed, 1.0f, 1.00f, 100.0f);
                ImGui::DragFloat("Mouse sensibility", &cam.mouse_sens, 0.01f, 0.01f, 5.0f, "%.2f");
                ImGui::DragFloat("Near plane", &cam.near_plane, 1.0f, 0.01f, 50.0f);
                ImGui::DragFloat("Far plane", &cam.far_plane, 1.0f, 0.01f, 5000.0f);
                ImGui::Checkbox("Active", &cam.active);
                ImGui::DragFloat3("Position", &cam.position.x_, 1.0f, -9999.9f, 9999.9f);
                ImGui::SliderFloat("Pitch", &cam.pitch_deg, -90.0f, 90.0f, "%.1f");
                ImGui::SliderFloat("Yaw", &cam.yaw_deg, -180.0f, 180.0f, "%.1f");
                ImGui::End();
            }

            if (ImGuiManager::show_spawning_window_) {
                ImGui::Begin("Spawn", &ImGuiManager::show_spawning_window_);

                // =========================
                // SHAPES
                // =========================
                if (ImGui::BeginCombo("Shape", ImGuiManager::selectedBasicShapeToSpawn_.name.c_str())) {
                    for (int i = 0; i < ImGuiManager::basic_shapes_info_.size(); i++) {
                        const bool is_selected = (ImGuiManager::basic_shapes_info_[i].shape == ImGuiManager::selectedBasicShapeToSpawn_.shape);
                        if (ImGui::Selectable(ImGuiManager::basic_shapes_info_[i].name.c_str(), is_selected)) {
                            ImGuiManager::selectedBasicShapeToSpawn_ = ImGuiManager::basic_shapes_info_[i];
                        }
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Spawn shape")) {
                    ME::Entity e;
                    std::string pathStr = "../../deps/resources/cube.obj";

                    if (ME::XMLManager::js_ref_) {
                        auto task = [pathStr]() {
                            return ME::MeshLoader::LoadObjPositionsFlat(pathStr);
                            };

                        ME::XMLManager::pending_meshes_.push_back({
                            e.GetID(),
                            ME::XMLManager::js_ref_->tEnqueue(std::move(task)),
                            pathStr
                            });
                    }

                    auto& etc = e.AddComponent<ME::TransformComponent>();
                    etc.SetPosition(cam.position);

                    e.AddComponent<ME::RenderizableComponent>(
                        cubeVerts, std::size(cubeVerts),
                        cubeIdx, std::size(cubeIdx),
                        Vec3{ 1.0f, 1.0f, 0.0f },
                        pathStr
                    );
                }

                ImGui::Separator();

                // =========================
                // LIGHTS
                // =========================
                if (ImGui::BeginCombo("Light", ImGuiManager::selectedLightToSpawn_.name.c_str())) {
                    for (int i = 0; i < ImGuiManager::light_spawn_info_.size(); i++) {
                        const bool is_selected = (ImGuiManager::light_spawn_info_[i].type == ImGuiManager::selectedLightToSpawn_.type);
                        if (ImGui::Selectable(ImGuiManager::light_spawn_info_[i].name.c_str(), is_selected)) {
                            ImGuiManager::selectedLightToSpawn_ = ImGuiManager::light_spawn_info_[i];
                        }
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Spawn light")) {
                    ME::Entity e;

                    std::string lightName = "Light";

                    switch (ImGuiManager::selectedLightToSpawn_.type)
                    {
                    case ME::ImGuiManager::LightType::AMBIENT:     lightName = "AmbientLight"; break;
                    case ME::ImGuiManager::LightType::POINT:       lightName = "PointLight"; break;
                    case ME::ImGuiManager::LightType::DIRECTIONAL: lightName = "DirectionalLight"; break;
                    case ME::ImGuiManager::LightType::SPOT:        lightName = "SpotLight"; break;
                    }

                    // NO FUNCIONA REVISAR
                    lightName += " " + std::to_string(e.GetID());
                    e.GetComponent<ME::TagComponent>()->setTag(lightName.c_str());

                    auto& t = e.AddComponent<ME::TransformComponent>();
                    t.SetPosition(cam.position);

                    auto& l = e.AddComponent<ME::LightComponent>();
                    switch (ImGuiManager::selectedLightToSpawn_.type)
                    {
                        case ImGuiManager::LightType::AMBIENT:l.SetType(ME::LightType::AMBIENT); break;
                        case ImGuiManager::LightType::DIRECTIONAL:l.SetType(ME::LightType::DIRECTIONAL); break;
                        case ImGuiManager::LightType::SPOT:l.SetType(ME::LightType::SPOT); break;
                        case ImGuiManager::LightType::POINT:l.SetType(ME::LightType::POINT); break;
                        default:
                            break;
                    }
                    l.SetEnabled(true);
                    l.SetDiffColor({ 1.0f, 1.0f, 1.0f });
                    l.SetSpecColor({ 1.0f, 1.0f, 1.0f });
                    l.SetShininess(32.0f);
                    l.SetSpecIntensity(1.0f);
                    l.SetConstantAtt(1.0f);
                    l.SetLinearAtt(0.09f);
                    l.SetQuadAtt(0.032f);
                    l.SetCutOff(12.5f);
                    l.SetOuterCutOff(20.0f);
                    l.SetCastsShadows(false);
                    l.SetShadowNear(0.1f);
                    l.SetShadowFar(25.0f);

                    // Ajustes espec�ficos por tipo
                    switch (ImGuiManager::selectedLightToSpawn_.type)
                    {
                    case ME::LightType::AMBIENT:
                        l.SetDiffColor({ 0.08f, 0.08f, 0.08f });
                        l.SetSpecColor({ 0.0f, 0.0f, 0.0f });
                        l.SetCastsShadows(false);
                        break;

                    case ME::LightType::POINT:
                        l.SetDiffColor({ 1.0f, 1.0f, 1.0f });
                        l.SetCastsShadows(true);
                        l.SetShadowNear(0.1f);
                        l.SetShadowFar(25.0f);
                        break;

                    case ME::LightType::DIRECTIONAL:
                        l.SetDir({ -0.5f, -1.0f, -0.3f });
                        l.SetCastsShadows(true);
                        l.SetShadowNear(1.0f);
                        l.SetShadowFar(2000.0f);
                        break;

                    case ME::LightType::SPOT:
                        l.SetDir({ 0.0f, -1.0f, 0.0f });
                        l.SetCutOff(12.5f);
                        l.SetOuterCutOff(20.0f);
                        l.SetCastsShadows(true);
                        l.SetShadowNear(0.1f);
                        l.SetShadowFar(25.0f);
                        break;
                    }
                }

                ImGui::End();
            }

            if (ImGuiManager::show_explorer_window_)
            {
                if (ImGui::Begin("Explorer", &ImGuiManager::show_explorer_window_)) {
                    static std::filesystem::path root = "../../";
                    DrawFileExplorer(root);
                    ImGui::End();
                }
            }

            if (!ImGuiManager::core_is_playing_ && ImGuiManager::input_ != nullptr) {
                if (ImGuiManager::input_->checkEditorActionJustPressed(ME::Input::Action::CycleGizmoMode)) {
                    ImGuiManager::current_gizmo_mode_ = NextGizmoMode(ImGuiManager::current_gizmo_mode_);
                }
            }

			
            switch (ImGuiManager::current_gizmo_mode_) {
                case ImGuiManager::GizmoMode::TRANSLATE:
                    DrawTranslateGizmoOverlay(selected_entity, cam.view, cam.proj);
                    break;

                case ImGuiManager::GizmoMode::ROTATE:
                    DrawRotateGizmoOverlay(selected_entity, cam.view, cam.proj);
                break;

                case ImGuiManager::GizmoMode::SCALE:
                    DrawScaleGizmoOverlay(selected_entity, cam.view, cam.proj);
                break;
            }

        };

        
        RegisterWindow(cb);
    }

    void ImGuiManager::RegisterScenesWindow()
    {
        enum class MenuState { Main, NewScene, LoadScene };
        static MenuState currentState = MenuState::Main;
        static char sceneName[128] = ""; // Buffer para el nombre

        GuiCallback cb = [&]() {
            float t = (float)ImGui::GetTime();
            float r = 0.6f + 0.1f * sinf(t * 0.4f);
            float g = 0.4f + 0.1f * sinf(t * 0.6f);
            float b = 0.25f + 0.05f * sinf(t * 0.3f);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(r, g, b, 1.0f));

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

            // EL BEGIN DEBE IR AQU� PARA QUE TODO EST� DENTRO DE LA SCENE
            if (ImGui::Begin("Title scene", nullptr, flags)) {
                ImVec2 windowSize = ImGui::GetWindowSize();

                if (currentState == MenuState::Main) {
                    // --- VISTA PRINCIPAL ---
                    const char* title = "MENTAT ENGINE";
                    ImGui::SetWindowFontScale(10.0f);
                    ImVec2 textSize = ImGui::CalcTextSize(title);
                    ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f - 50.0f));
                    ImGui::Text(title);
                    ImGui::SetWindowFontScale(1.0f);

                    ImVec2 buttonSize(200, 70);
                    float totalWidth = (buttonSize.x * 2) + 20.0f;
                    ImGui::SetCursorPos(ImVec2((windowSize.x - totalWidth) * 0.5f, windowSize.y - 100.0f));

                    if (ImGui::Button("NEW SCENE", buttonSize)) {
                        currentState = MenuState::NewScene; // Cambiamos de estado
                    }
                    ImGui::SameLine(0, 20.0f);
                    if (ImGui::Button("LOAD SCENE", buttonSize)) { 
                        currentState = MenuState::LoadScene;
                    }

                }
                else if (currentState == MenuState::NewScene) {
                    // --- VISTA NEW SCENE ---
                    float inputWidth = 300.0f;

                    // Centramos el grupo entero calculando el inicio
                    ImGui::SetCursorPos(ImVec2((windowSize.x - inputWidth) * 0.5f, windowSize.y * 0.5f - 50.0f));

                    ImGui::BeginGroup();
                    ImGui::Text("SCENE NAME:");
                    // input width
                    ImGui::SetNextItemWidth(inputWidth);
                    ImGui::InputText("##name", sceneName, IM_ARRAYSIZE(sceneName));

                    ImGui::Spacing();
                    ImGui::Spacing();

                    // action buttons down the input
                    if (ImGui::Button("START", ImVec2(145, 40))) {
                        std::string nameStr(sceneName);

                        bool isEmpty = nameStr.find_first_not_of(' ') == std::string::npos;

                        bool hasInvalidChars = nameStr.find_first_of("\\/:*?\"<>|") != std::string::npos;

                        if (!isEmpty && !hasInvalidChars) {
                            try {
                                ME::XMLManager::FileToScene("DefaultScene", true);
                                ME::Core::InScenesPage = false;
                                ME::Core::CurrentSceneName = nameStr;
                                ClearWindows();
                                RegisterMainWindow();
                            }
                            catch (const std::filesystem::filesystem_error& e) {
                                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to load default scene file.");
                            }
                        }
                        else {
                            ImGui::OpenPopup("BadNamePopUp");
                        }
                    }

                    if (ImGui::BeginPopup("BadNamePopUp")) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Invalid name (use letters/numbers, avoid \\/:*?\"<>|)");
                        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                    }

                    ImGui::SameLine(0, 10.0f);

                    if (ImGui::Button("BACK", ImVec2(145, 40))) {
                        currentState = MenuState::Main;
                        memset(sceneName, 0, sizeof(sceneName));
                    }
                    ImGui::EndGroup();
                }
                else if (currentState == MenuState::LoadScene) {
                    float inputWidth = 300.0f;
                    std::string path = "../../deps/resources/scenes/";

                    // Centramos el grupo igual que en New Scene
                    ImGui::SetCursorPos(ImVec2((windowSize.x - inputWidth) * 0.5f, windowSize.y * 0.5f - 100.0f));

                    ImGui::BeginGroup();
                    ImGui::Text("SELECT AN EXISTING SCENE:");
                    ImGui::Spacing();

                    // Contenedor para la lista de archivos con scroll
                    ImGui::BeginChild("SceneList", ImVec2(inputWidth, 200.0f), true);
                    try {
                        for (const auto& entry : std::filesystem::directory_iterator(path)) {
                            std::string filename = entry.path().filename().string();

                            // only files filter
                            if (entry.is_regular_file() && !entry.path().has_extension()) {
                                std::string filename = entry.path().filename().string();

                                if (ImGui::Button(filename.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 35.0f))) {
                                    try {
                                        ME::XMLManager::FileToScene(filename, false);
                                        ME::Core::InScenesPage = false;
                                        ME::Core::CurrentSceneName = filename;
                                        ClearWindows();
                                        RegisterMainWindow();
                                    }
                                    catch (const std::filesystem::filesystem_error& e) {
                                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to load file, incorrect format.");
                                    }
                                }
                            }
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to load scenes folder, make sure it exists on engineRoot/deps/resources/scenes");
                    }
                    ImGui::EndChild();

                    ImGui::Spacing();
                    ImGui::Spacing();

                    // Bot�n BACK con el mismo tama�o que el de New Scene
                    if (ImGui::Button("BACK", ImVec2(inputWidth, 40))) {
                        currentState = MenuState::Main;
                    }
                    ImGui::EndGroup();
                }
            }
            ImGui::End();
            ImGui::PopStyleColor();
            };
        RegisterWindow(cb);
    }

    void ImGuiManager::DrawRegisteredWindows() {
        for (auto& cb : s_callbacks_) {
            if (cb) cb();   // aqu� dentro se ejecuta la ventana de imgui que le digamos en el callback
        }

        // path for the basic shape cube
        std::string pathStr = "../../deps/resources/cube.obj";
        
        // After drawing all the windows, update the state of pending to update component
        if (pending_update_component != -1) {
            switch (pending_update_component) {
            case 0: ME::ECS::AddComponent<ME::TransformComponent>(selected_entity); break;
            case 1: 
                if (ME::XMLManager::js_ref_) {
                    auto task = [pathStr]() {
                        return ME::MeshLoader::LoadObjPositionsFlat(pathStr);
                        };

                    ME::XMLManager::pending_meshes_.push_back({
                        selected_entity,
                        ME::XMLManager::js_ref_->tEnqueue(std::move(task)),
                        pathStr
                        });
                }
                ME::ECS::AddComponent<ME::RenderizableComponent>(selected_entity, cubeVerts, std::size(cubeVerts), 
                    cubeIdx, std::size(cubeIdx), Vec3{ 1.0f, 1.0f, 0.0f}, pathStr);
                break;
            case 2: ME::ECS::AddComponent<ME::LightComponent>(selected_entity); break;
            case 3: ME::ECS::AddComponent<ME::CameraComponent>(selected_entity); break;
            case 4: ME::ECS::AddComponent<ME::ScriptComponent>(selected_entity, selected_entity); break;
            case 5: ME::ECS::RemoveComponent<ME::TransformComponent>(selected_entity); break;
            case 6: ME::ECS::RemoveComponent<ME::RenderizableComponent>(selected_entity); break;
            case 7: ME::ECS::RemoveComponent<ME::LightComponent>(selected_entity); break;
            case 8: ME::ECS::RemoveComponent<ME::CameraComponent>(selected_entity); break;
            case 9: ME::ECS::RemoveComponent<ME::ScriptComponent>(selected_entity); break;
            }
            pending_update_component = -1;
        }
        
    }

    void ImGuiManager::ClearWindows() {
        s_callbacks_.clear();
    }
    bool ImGuiManager::IsRuntimeInputMode()
    {
        return ME::ImGuiManager::core_is_playing_ && !ME::ImGuiManager::core_is_paused_;
    }
    void ImGuiManager::SetInput(Input* input)
    {
        input_ = input;
    }
}

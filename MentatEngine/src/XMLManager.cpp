#include "../Include/MentatEngine/XMLManager.hpp"
#include "../Include/MentatEngine/ECSManager.hpp"
#include <../include/MentatEngine/TagComponent.hpp>
#include <../include/MentatEngine/OpenGl/RenderizableComponent.hpp>
#include <../include/MentatEngine/ScriptComponent.hpp>
#include <../include/MentatEngine/LightComponent.hpp>
#include <../include/MentatEngine/TransformComponent.hpp>

#include <pugixml.hpp>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace ME {
    std::vector<XMLManager::PendingMesh> XMLManager::pending_meshes_;
    JobSystem* XMLManager::js_ref_;

    std::string XMLManager::SaveScene(std::string scene_name)
    {
        pugi::xml_document doc;

        // Nodo raíz
        auto root = doc.append_child("Scene");
        root.append_attribute("name") = scene_name.c_str();

        int current = 0;

        while (current < ME::ECS::current_index) {
            if (auto* tagc = ECS::GetComponent<ME::TagComponent>(current)) {
                auto entityNode = root.append_child("Entity");
                entityNode.append_attribute("tag") = tagc->getTag();
                entityNode.append_attribute("id") = current;

                if (auto* transc = ECS::GetComponent<ME::TransformComponent>(current)) {
                    auto node = entityNode.append_child("Transform");
                    node.append_attribute("px") = transc->Position().x_;
                    node.append_attribute("py") = transc->Position().y_;
                    node.append_attribute("pz") = transc->Position().z_;
                    node.append_attribute("sx") = transc->Scale().x_;
                    node.append_attribute("sy") = transc->Scale().y_;
                    node.append_attribute("sz") = transc->Scale().z_;
                    node.append_attribute("rx") = transc->Rotation().x_;
                    node.append_attribute("ry") = transc->Rotation().y_;
                    node.append_attribute("rz") = transc->Rotation().z_;
                    node.append_attribute("id") = transc->GetParent();
                }

                if (auto* rendc = ECS::GetComponent<ME::RenderizableComponent>(current)) {
                    auto node = entityNode.append_child("Renderizable");
                    node.append_attribute("meshpath") = rendc->GetMeshPath();
                    node.append_attribute("texturepath") = rendc->GetTexurePath();
                    node.append_attribute("cx") = rendc->GetColor().x_;
                    node.append_attribute("cy") = rendc->GetColor().y_;
                    node.append_attribute("cz") = rendc->GetColor().z_;
                    node.append_attribute("visibility") = rendc->Visible();
                    node.append_attribute("color_active") = rendc->IsColorActive();

                }

                if (auto* scriptc = ECS::GetComponent<ME::ScriptComponent>(current)) {
                    auto node = entityNode.append_child("Script");
                    node.append_attribute("path") = scriptc->getCurrentScriptPath();
                    node.append_attribute("enabled") = scriptc->IsCodeEnabled();
                }

                if (auto* lightc = ECS::GetComponent<ME::LightComponent>(current)) {
                    auto node = entityNode.append_child("Light");
                    node.append_attribute("type") = lightc->Type();
                    node.append_attribute("enabled") = lightc->Enabled();
                    node.append_attribute("px") = lightc->Pos().x_;
                    node.append_attribute("py") = lightc->Pos().y_;
                    node.append_attribute("pz") = lightc->Pos().z_;
                    node.append_attribute("dirx") = lightc->Dir().x_;
                    node.append_attribute("diry") = lightc->Dir().y_;
                    node.append_attribute("dirz") = lightc->Dir().z_;
                    node.append_attribute("diffcx") = lightc->DiffColor().x_;
                    node.append_attribute("diffcy") = lightc->DiffColor().y_;
                    node.append_attribute("diffcz") = lightc->DiffColor().z_;
                    node.append_attribute("speccx") = lightc->SpecColor().x_;
                    node.append_attribute("speccy") = lightc->SpecColor().y_;
                    node.append_attribute("speccz") = lightc->SpecColor().z_;
                    node.append_attribute("shininess") = lightc->Shininess();
                    node.append_attribute("spec_inten") = lightc->SpecIntensity();
                    node.append_attribute("const_att") = lightc->ConstantAtt();
                    node.append_attribute("linear_att") = lightc->LinearAtt();
                    node.append_attribute("quad_att") = lightc->QuadAtt();
                    node.append_attribute("cutoff") = lightc->CutOff();
                    node.append_attribute("outercutoff") = lightc->OuterCutOff();
                }

                if (auto* camerac = ECS::GetComponent<ME::CameraComponent>(current)) {
                    auto node = entityNode.append_child("Camera");
                    node.append_attribute("px") = camerac->position.x_;
                    node.append_attribute("py") = camerac->position.y_;
                    node.append_attribute("pz") = camerac->position.z_;
                    node.append_attribute("fov_deg") = camerac->fov_deg;
                    node.append_attribute("near_plane") = camerac->near_plane;
                    node.append_attribute("far_plane") = camerac->far_plane;
                    node.append_attribute("move_speed") = camerac->move_speed;
                    node.append_attribute("mouse_sens") = camerac->mouse_sens;
                    node.append_attribute("active") = camerac->active;
                    node.append_attribute("yaw_deg") = camerac->yaw_deg;
                    node.append_attribute("pitch_deg") = camerac->pitch_deg;
                }

            }
            current++;
        }

        // Exportar a string
        std::stringstream ss;
        doc.save(ss);
        return ss.str();
    }

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

    void XMLManager::LoadScene(const std::string& xml_content) {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xml_content.c_str());

        if (!result) {
            printf("Error al cargar el XML de la escena: %s\n", result.description());
            return;
        }

        int current = 0;
        while (current < ECS::current_index) {
            ECS::RemoveEntity(current);
            current++;
        }
        ECS::current_index = 1;

        pugi::xml_node scene = doc.child("Scene");

        for (pugi::xml_node entityNode : scene.children("Entity")) {
            Entity e;
            unsigned long entityID = e.GetID();
            const char* tag = entityNode.attribute("tag").as_string();
            auto& tagc = ECS::AddComponent<ME::TagComponent>(entityID);
            tagc.setTag(tag);

            if (pugi::xml_node node = entityNode.child("Transform")) {
                auto& transc = ECS::AddComponent<ME::TransformComponent>(entityID);
                transc.SetPosition({ node.attribute("px").as_float(), node.attribute("py").as_float(), node.attribute("pz").as_float() });
                transc.SetScale({ node.attribute("sx").as_float(), node.attribute("sy").as_float(), node.attribute("sz").as_float() });
                transc.SetRotation({ node.attribute("rx").as_float(), node.attribute("ry").as_float(), node.attribute("rz").as_float() });
                transc.SetParent(node.attribute("id").as_int());
            }

            if (pugi::xml_node node = entityNode.child("Renderizable")) {
                std::string meshpath = node.attribute("meshpath").as_string();
                std::string texturepath = node.attribute("texturepath").as_string();
                auto& ren = ECS::AddComponent<ME::RenderizableComponent>(entityID, cubeVerts, std::size(cubeVerts),
                    cubeIdx, std::size(cubeIdx), Vec3{ node.attribute("cx").as_float(), 
                    node.attribute("cy").as_float(), node.attribute("cz").as_float() }, meshpath);
                ren.SetVisibility(node.attribute("visibility").as_bool());
                ren.SetColorActive(node.attribute("color_active").as_bool());
                if (texturepath != "") {
                    ren.SetTexurePath(texturepath);
                }

                if (!std::filesystem::exists(meshpath)) {
                    std::cerr << "[XMLManager] Mesh path not found at: " << meshpath << std::endl;
                }
                else {
                    auto task = [meshpath]() { return MeshLoader::LoadObjPositionsFlat(meshpath); };
                    pending_meshes_.push_back({ entityID, js_ref_->tEnqueue(std::move(task)), meshpath });
                }
            }

            if (pugi::xml_node node = entityNode.child("Script")) {
                auto& scriptc = ECS::AddComponent<ME::ScriptComponent>(entityID, entityID);
                scriptc.setCodeByPath(std::string((char*)node.attribute("path").as_string()));
                scriptc.EnableCode(node.attribute("enabled").as_bool());
            }

            if (pugi::xml_node node = entityNode.child("Light")) {
                auto& lightc = ECS::AddComponent<ME::LightComponent>(entityID);
                switch (node.attribute("type").as_int())
                {
                case 0: lightc.SetType(ME::LightType::AMBIENT); break;
                case 1: lightc.SetType(ME::LightType::POINT); break;
                case 2: lightc.SetType(ME::LightType::DIRECTIONAL); break;
                case 3: lightc.SetType(ME::LightType::SPOT); break;
                }
                lightc.SetEnabled(node.attribute("enabled").as_bool());
                lightc.SetPos({ node.attribute("px").as_float(), node.attribute("py").as_float(), node.attribute("pz").as_float() });
                lightc.SetDir({ node.attribute("dirx").as_float(), node.attribute("diry").as_float(), node.attribute("dirz").as_float() });
                lightc.SetDiffColor({ node.attribute("diffcx").as_float(), node.attribute("diffcy").as_float(), node.attribute("diffcz").as_float() });
                lightc.SetSpecColor({ node.attribute("speccx").as_float(), node.attribute("speccy").as_float(), node.attribute("speccz").as_float() });
                lightc.SetShininess(node.attribute("shininess").as_float());
                lightc.SetSpecIntensity(node.attribute("spec_inten").as_float());
                lightc.SetConstantAtt(node.attribute("const_att").as_float());
                lightc.SetLinearAtt(node.attribute("linear_att").as_float());
                lightc.SetQuadAtt(node.attribute("quad_att").as_float());
                lightc.SetCutOff(node.attribute("cutoff").as_float());
                lightc.SetOuterCutOff(node.attribute("outercutoff").as_float());
            }

            if (pugi::xml_node node = entityNode.child("Camera")) {
                auto& camerac = ECS::AddComponent<ME::CameraComponent>(entityID);
                camerac.position = { node.attribute("px").as_float(), node.attribute("py").as_float(), node.attribute("pz").as_float() };
                camerac.fov_deg = node.attribute("fov_deg").as_float();
                camerac.near_plane = node.attribute("near_plane").as_float();
                camerac.far_plane = node.attribute("far_plane").as_float();
                camerac.move_speed = node.attribute("move_speed").as_float();
                camerac.mouse_sens = node.attribute("mouse_sens").as_float();
                camerac.active = node.attribute("active").as_bool();
                camerac.yaw_deg = node.attribute("yaw_deg").as_float();
                camerac.pitch_deg = node.attribute("pitch_deg").as_float();
            }
        }
    }

    void XMLManager::SceneToFile() {
        std::string directory = "../../deps/resources/scenes/";
        std::string fullPath = directory + ME::Core::CurrentSceneName;

        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directories(directory);
        }

        std::string res = SaveScene(ME::Core::CurrentSceneName);

        pugi::xml_document doc;
        pugi::xml_parse_result parseResult = doc.load_string(res.c_str());

        if (parseResult) {
            bool result = doc.save_file(fullPath.c_str(), PUGIXML_TEXT("  "));

            if (result) {
                printf("[XMLManager] Scene saved at: %s\n", fullPath.c_str());
            }
            else {
                printf("[Error] Coulndt write in the scene at: %s\n", fullPath.c_str());
            }
        }
        else {
            printf("[Error] Unvalid scene generated.\n");
        }
    }

    void XMLManager::FileToScene(std::string scene_name, bool fromDefault)
    {
        std::string directory = (fromDefault) ? "../../deps/resources/scenes/default/" : "../../deps/resources/scenes/";

        std::string fullPath = directory + scene_name;

        if (!std::filesystem::exists(fullPath)) {
            printf("[Error] Scene'%s' not found at route: %s\n", scene_name.c_str(), fullPath.c_str());
            return;
        }

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(fullPath.c_str());

        if (result) {
            std::stringstream ss;
            doc.save(ss);
            printf("[XMLManager] Loading scene from file: %s\n", fullPath.c_str());
            LoadScene(ss.str());
        }
        else {
            printf("[Error] Couldnt serialize the scene: %s\n", result.description());
        }
    }

    void XMLManager::UpdatePendingResources() {
        for (auto it = pending_meshes_.begin(); it != pending_meshes_.end(); ) {
            auto status = it->future.wait_for(std::chrono::seconds(0));

            if (status == std::future_status::ready) {
                ME::LoadedDrawable data = it->future.get();

                if (auto* render = ECS::GetComponent<ME::RenderizableComponent>(it->entityID)) {
                    render->ReplaceMesh(
                        data.vertices.data(), data.vertices.size(),
                        data.indexes.data(), data.indexes.size(),
                        const_cast<char*>(it->path.c_str())
                    );
                }
                else {
                    printf("[Error] RenderizableComponent not found with ID: %lu\n", it->entityID);
                }
                it = pending_meshes_.erase(it);
            }
            else {
                ++it;
            }
        }
    }

}
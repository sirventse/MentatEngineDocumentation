/**
 *
 * @brief Mesh loader using tyniobjectloader.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __MESHLOADER_HPP__
#define __MESHLOADER_HPP__ 1


#include <GLFW/glfw3.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "../deps/KFS/include/vec3.h"
#include <vector>

namespace ME {
    struct LoadedDrawable {
        LoadedDrawable() {};

        LoadedDrawable(LoadedDrawable&& other) noexcept
            : vertices(std::move(other.vertices)),
            indexes(std::move(other.indexes))
        {
        }

        LoadedDrawable& operator=(LoadedDrawable&& other) noexcept {
            if (this != &other) {
                vertices = std::move(other.vertices);
                indexes = std::move(other.indexes);
            }
            return *this;
        }

        std::vector<Vec3> vertices;

        std::vector<unsigned int> indexes;
    };

    class MeshLoader
    {
        public:

            MeshLoader();

            ~MeshLoader();

            /**
             * @brief Loads an .obj file as a flat list of positions.
             * @param filepath Path to the .obj file.
             * @return A LoadedDrawable structure containing the processed data.
             */
            static LoadedDrawable LoadObjPositionsFlat(const std::string& filepath);

        private:
    };
}


#endif //MeshLoader
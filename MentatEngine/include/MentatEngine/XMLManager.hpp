/**
 *
 * @brief TransformComponent base class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __XML_MANAGER_HPP__
#define __XML_MANAGER_HPP__ 1

#include <string>
#include <MentatEngine/MeshLoader.hpp>
#include <MentatEngine/JobSystem.hpp>

namespace ME {
	class XMLManager {

		public:

			struct PendingMesh {
				unsigned long entityID;
				std::future<ME::LoadedDrawable> future;
				std::string path;
			};

			/**
			 * @brief Saves the scene reading the ECS as a XML std::string.
			 * @param scene_name The name of the scene to save.
			 * @return A std::string with the info of the ECS as XML std::string.
			 */
			static std::string SaveScene(std::string scene_name);

			/**
			 * @brief Cleans the current ECS and reads a XML std::string to load the data.
			 * @param xml_content The saved data.
			 */
			static void LoadScene(const std::string& xml_content);

			/**
			 * @brief Reads a XML scene from disk with base path "deps/resources/scenes" and loads it in the ECS.
			 * @param fileName Name of the scene found in the scenes path.
			 */
			static void SceneToFile();

			/**
			 * @brief Saves the scene in the disk as a XML at "deps/resources/scenes" folder.
			 * @param scene_name The name of the scene to save.
			 */
			static void FileToScene(std::string scene_name, bool fromDefault);

			/**
			 * @brief Checks the pending meshes loading with the jobsystem to load them.
			 */
			static void UpdatePendingResources();

			static std::vector<PendingMesh> pending_meshes_;

			static JobSystem* js_ref_;
	};
}

#endif // __XML_MANAGER_HPP__
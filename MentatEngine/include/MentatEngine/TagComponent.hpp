/**
 *
 * @brief TransformComponent base class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __TAG_COMPONENT_HPP__
#define __TAG_COMPONENT_HPP__ 1

#include <string>

namespace ME {
	class TagComponent {
		public:
			/**
			 * @brief Default constructor.
			 * Initializes the tag with an empty string.
			 */
			TagComponent() : tag("") {};

			/**
			 * @brief Constructs a TagComponent with a specific tag.
			 * @param t Pointer to a character array (C-string) to initialize the tag.
			 */
			TagComponent(char* t) : tag(t) {};

			~TagComponent() {};

			TagComponent(TagComponent&& other) noexcept : tag { other.tag } {};
			TagComponent(const TagComponent& other) : tag { other.tag } {};

			TagComponent& operator=(TagComponent&& other) noexcept {
				tag = other.tag;
				return *this;
			}

			TagComponent& operator=(const TagComponent& other) {
				tag = other.tag;
				return *this;
			}

			/**
			* @brief Updates the tag string.
			* @param t The new string to assign as a tag.
			*/
			void setTag(std::string t) { tag = t; };

			/**
			 * @brief Retrieves the current tag.
			 * @return The tag as a std::string.
			 */
			std::string getTag() { return tag; };

		private:
			std::string tag;
	};
}

#endif // __TAG_COMPONENT_HPP__
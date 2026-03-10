/**
 *
 * @brief Drawable class.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __ECS_ITERATOR_HPP__
#define __ECS_ITERATOR_HPP__

#include <vector>
#include <optional>

namespace ME {
    template<typename T>
    struct Iterator {

        Iterator& operator++() {
            it_++;
            return *this;
        }
        T* operator*() {
            return &it_->second;
        }

        bool operator==(const Iterator& other) const {
            return it_ == other.it_;
        }

        bool operator!=(const Iterator& other) const {
            return it_ != other.it_;
        }

        /**
         * @brief Gets the owner ID of the current element.
         * @return The size_t key (first element of the pair).
         */
        size_t owner() {
            return it_->first;
        }

        /**
         * @brief Manually sets the empty state of the iterator.
         * @param in Boolean value to set the empty state.
         */
        void SetEmpty(bool in) {
            empty = in;
        }

        /**
         * @brief Checks if the iterator is marked as empty.
         * @return True if the empty flag is set.
         */
        bool IsEmpty() {
            return empty;
        }

        typename std::vector<std::pair<size_t, T>>::iterator it_;
        bool empty;
    };

    template<typename T>
    struct ContainerFacade {

        /**
         * @brief Constructs a facade for an existing list.
         * @param list Reference to the vector of pairs to be managed.
         */
        ContainerFacade(std::vector<std::pair<size_t, T>>& list) : list_(list) {}

        /**
         * @brief Returns an iterator to the beginning of the list.
         * @return An Iterator instance at the first position.
         */
        Iterator<T> begin() {
            return Iterator<T>{ list_.begin() };
        }

        /**
         * @brief Returns an iterator to the end of the list.
         * @return An Iterator instance at the past-the-end position.
         */
        Iterator<T> end() {
            return Iterator<T>{ list_.end() };
        }

        /**
         * @brief Checks if the underlying list is empty.
         * @return True if the size is 0, false otherwise.
         */
        bool isEmpty() const {
            return list_.empty(); // Simplified check
        }

        std::vector<std::pair<size_t, T>>& list_;
    };
}

#endif // __ECS_ITERATOR_HPP__
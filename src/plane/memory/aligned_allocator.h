#ifndef WAVE_MODEL_MEMORY_ALIGNED_ALLOCATOR_H_
#define WAVE_MODEL_MEMORY_ALIGNED_ALLOCATOR_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include <cstdlib>

/// @brief
namespace wave_model {

/**
 * @brief Allocates memory with respect to type alignment
 * @tparam T Type of objects to allocate
 * @tparam NA Desired alignment (defaults to alignof(T))
 */
template<typename T, size_t NA = alignof(T)>
class WmAlignedAllocator
{
public:
    /// See basic allocator interface
    template<typename U, size_t NB = alignof(U)>
    struct rebind
    {
        using other = WmAlignedAllocator<U, NB>;
    };

    /// See basic allocator interface
    using value_type = T;
    /// See basic allocator interface
    using size_type = size_t;
    /// See basic allocator interface
    using difference_type = ptrdiff_t;

    /// Desired alignment
    static constexpr size_t align_value = NA;

    /// Default constructor
    WmAlignedAllocator() = default;

    /// See basic allocator interface
    template<typename U, size_t NB> 
    constexpr WmAlignedAllocator(const WmAlignedAllocator<U, NB>&) noexcept:
        WmAlignedAllocator()
    {}

    /**
     * @brief Allocates memory (using std::aligned_alloc)
     * @param count Number of objects to allocate memory for
     * @return Pointer to the allocated memory (respectively aligned)
     */
    value_type* allocate(size_type count)
    {
        return static_cast<value_type*>(
                std::aligned_alloc(align_value, count * sizeof(value_type)));
    }

    /**
     * @brief Deallocates memory (using std::free)
     * @param ptr Pointer to the memory being previously allocate()'d
     * @param count [unused] Number of objects the memory contains
     */
    void deallocate(value_type* ptr, [[maybe_unused]] size_type count) noexcept
    {
        return std::free(ptr);
    }

private:
};

/// See basic allocator interface
template<typename T, typename U, size_t NA, size_t NB>
bool operator == (const WmAlignedAllocator<T, NA>&, 
                  const WmAlignedAllocator<U, NB>&) 
{ 
    return true; 
}

/// See basic allocator interface
template<typename T, typename U, size_t NA, size_t NB>
bool operator != (const WmAlignedAllocator<T, NA>&, 
                  const WmAlignedAllocator<U, NB>&) 
{ 
    return false; 
}

} // namespace wave_model

#endif // WAVE_MODEL_MEMORY_ALIGNED_ALLOCATOR_H_

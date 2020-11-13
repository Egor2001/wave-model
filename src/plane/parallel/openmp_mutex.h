#ifndef WAVE_MODEL_PARALLEL_OPENMP_MUTEX_H_
#define WAVE_MODEL_PARALLEL_OPENMP_MUTEX_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#if defined(_OPENMP)
    #include <omp.h> 
#endif // _OPENMP

/// @brief
namespace wave_model {

#if defined(_OPENMP)

/**
 * @brief Simple implementation of the mutex using omp primitives
 * Is implemented as dummy if no openmp is presented.
 */
class WmOpenMPMutex
{
public:
    WmOpenMPMutex()
    {
        omp_init_lock(&lock_);
    }

    WmOpenMPMutex             (const WmOpenMPMutex&) = delete;
    WmOpenMPMutex& operator = (const WmOpenMPMutex&) = delete;
    WmOpenMPMutex             (WmOpenMPMutex&&) = delete;
    WmOpenMPMutex& operator = (WmOpenMPMutex&&) = delete;

    ~WmOpenMPMutex()
    {
        omp_destroy_lock(&lock_);
        lock_ = {};
    }

    void lock()
    {
        omp_set_lock(&lock_);
    }

    bool try_lock()
    {
        return !!omp_test_lock(&lock_);
    }

    void unlock()
    {
        omp_unset_lock(&lock_);
    }

private:
    omp_lock_t lock_{};
};

#else // _OPENMP

class WmOpenMPMutex
{
public:
    WmOpenMPMutex() = default;

    WmOpenMPMutex             (const WmOpenMPMutex&) = delete;
    WmOpenMPMutex& operator = (const WmOpenMPMutex&) = delete;
    WmOpenMPMutex             (WmOpenMPMutex&&) = delete;
    WmOpenMPMutex& operator = (WmOpenMPMutex&&) = delete;

    ~WmOpenMPMutex() = default;

    void lock() {}
    bool try_lock() { return true; }
    void unlock() {}
};

#endif // _OPENMP

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_OPENMP_MUTEX_H_

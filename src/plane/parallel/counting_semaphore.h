#ifndef WAVE_MODEL_PARALLEL_COUNTING_SEMAPHORE_
#define WAVE_MODEL_PARALLEL_COUNTING_SEMAPHORE_

#if defined(__cpp_lib_semaphore)
    #include <semaphore>
#else // __cpp_lib_semaphore
    #include <mutex>
    #include <atomic>
    #include <condition_variable>
#endif

namespace wave_model {

#if defined(__cpp_lib_semaphore)

template<ptrdiff_t NLeastMaxValue>
class WmCountingSemaphore
{
public:
    explicit WmCountingSemaphore(ptrdiff_t value):
        semaphore_{ value }
    {}

    void acquire()
    {
        semaphore_.acquire();
    }

    void release(ptrdiff_t update = 1)
    {
        semaphore_.release(update);
    }

private:
    std::counting_semaphore<NLeastMaxValue> semaphore_;
};

#else // __cpp_lib_semaphore

template<ptrdiff_t NLeastMaxValue>
class WmCountingSemaphore
{
public:
    explicit WmCountingSemaphore(ptrdiff_t value):
        value_{ value }
    {}

    void acquire()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]() -> bool { return value_ > 0; });
        --value_;
    }

    void release(ptrdiff_t update = 1)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        value_ += update;
        lock.unlock();

        while (update-- > 0)
            cond_var_.notify_one();
    }

private:
    std::mutex mutex_{};
    std::condition_variable cond_var_{};
    ptrdiff_t value_ = 0;
};

#endif // __cpp_lib_semaphore

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_COUNTING_SEMAPHORE_

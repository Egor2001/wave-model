#ifndef PARALLEL_THREAD_POOL_H_
#define PARALLEL_THREAD_POOL_H_

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

// for Test struct
#include <atomic>

namespace wave_model {

class WmThreadPool
{
public:
    struct Test;

    static constexpr size_t NDefaultConcurrency = 4;

    // TODO: to check how much threads to run
    explicit WmThreadPool(size_t workers_cnt = 
            std::thread::hardware_concurrency())
    {
        if (workers_cnt == 0)
            workers_cnt = NDefaultConcurrency;

        for (size_t worker_idx = 0; worker_idx < workers_cnt; ++worker_idx)
        {
            workers_.emplace_back([this]() {
                    while (true)
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cond_var_.wait(lock, [this]() -> bool {
                                return !tasks_.empty() || stopped_;
                            });

                        if (stopped_) 
                            break;

                        auto task = tasks_.front();
                        tasks_.pop();
                        // ++running_;

                        lock.unlock();
                        task();
                    }
                });
        }
    }

    WmThreadPool             (const WmThreadPool&) = delete;
    WmThreadPool& operator = (const WmThreadPool&) = delete;

    WmThreadPool             (WmThreadPool&&) noexcept = default;
    WmThreadPool& operator = (WmThreadPool&&) noexcept = default;

    ~WmThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stopped_ = true;
        }

        cond_var_.notify_all();
        for (auto& worker : workers_)
            worker.join();
    }

    template<typename TFunc>
    void enqueue(TFunc&& func)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.emplace(std::forward<TFunc>(func));

        lock.unlock();
        cond_var_.notify_one();
    }

private:
    bool stopped_ = false;
    // size_t running_ = 0;

    std::mutex mutex_{};
    std::condition_variable cond_var_{};

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
};

struct WmThreadPool::Test
{
    template<typename TStream>
    static TStream& test_init(TStream& stream)
    {
        WmThreadPool thread_pool(WmThreadPool::NDefaultConcurrency);

        return stream
    }

    template<typename TStream>
    static TStream& test_run(TStream& stream)
    {
        WmThreadPool thread_pool(WmThreadPool::NDefaultConcurrency);

        std::atomic<size_t> counter{0};
        for (size_t idx = 0; idx < WmThreadPool::NDefaultConcurrency; ++idx)
        {
            thread_pool.enqueue([&counter, inc = idx + 1]() { 
                    stream << counter.fetch_add(inc); 
                });
        }

        return stream;
    }
};

} // namespace wave_model 

#endif // PARALLEL_THREAD_POOL_H_

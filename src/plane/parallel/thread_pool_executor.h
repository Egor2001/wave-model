#ifndef PARALLEL_THREAD_POOL_EXECUTOR_H_
#define PARALLEL_THREAD_POOL_EXECUTOR_H_

#include "abstract_executor.h"

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

// for Test struct
#include <atomic>

namespace wave_model {

class WmThreadPoolExecutor final : public WmAbstractExecutor
{
public:
    struct Test;

    static constexpr size_t NDefaultConcurrency = 4;

    // TODO: to check how much threads to run
    explicit WmThreadPoolExecutor(size_t workers_cnt = 
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

                        if (tasks_.empty() && stopped_) 
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

    ~WmThreadPoolExecutor() override final
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stopped_ = true;
        }

        cond_var_.notify_all();
        for (auto& worker : workers_)
            worker.join();
    }

    void enqueue(std::function<void()> func) override final
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push(std::move(func));

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

struct WmThreadPoolExecutor::Test
{
    template<typename TStream>
    static TStream& test_init(TStream& stream)
    {
        static constexpr size_t NDefaultConcurrency = 
            WmThreadPoolExecutor::NDefaultConcurrency;

        WmThreadPoolExecutor thread_pool(NDefaultConcurrency);

        return stream;
    }

    template<typename TStream>
    static TStream& test_run(TStream& stream)
    {
        static constexpr size_t NDefaultConcurrency = 
            WmThreadPoolExecutor::NDefaultConcurrency;

        size_t counter = 0;
        std::mutex mutex;

        {
            WmThreadPoolExecutor thread_pool(NDefaultConcurrency);

            for (size_t idx = 0; idx < NDefaultConcurrency; ++idx)
            {
                thread_pool.enqueue([&mutex, &counter, inc = idx + 1]() { 
                        std::unique_lock<std::mutex> lock(mutex);
                        counter += inc; 
                    });
            }
        }

        stream << counter;

        return stream;
    }
};

} // namespace wave_model 

#endif // PARALLEL_THREAD_POOL_EXECUTOR_H_

#ifndef WAVE_MODEL_TEST_PARALLEL_THREAD_POOL_EXECUTOR_H_
#define WAVE_MODEL_TEST_PARALLEL_THREAD_POOL_EXECUTOR_H_

#include "parallel/thread_pool_executor.h"

namespace wave_model {

template<typename TStream>
TStream& wm_test_thread_pool_executor(TStream& stream)
{
    WmThreadPoolExecutor::Test::test_init(stream);
    WmThreadPoolExecutor::Test::test_run(stream);

    return stream;
}

} // namespace wave_model

#endif // WAVE_MODEL_TEST_PARALLEL_THREAD_POOL_EXECUTOR_H_

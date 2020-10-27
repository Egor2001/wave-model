#ifndef WAVE_MODEL_TEST_PARALLEL_THREAD_POOL_H_
#define WAVE_MODEL_TEST_PARALLEL_THREAD_POOL_H_

#include "parallel/thread_pool.h"

namespace wave_model {

template<typename TStream>
TStream& wm_test_thread_pool(TStream& stream)
{
    WmThreadPool::Test::test_init(stream);
    WmThreadPool::Test::test_run(stream);

    return stream;
}

} // namespace wave_model

#endif // WAVE_MODEL_TEST_PARALLEL_THREAD_POOL_H_

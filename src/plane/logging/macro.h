#ifndef WAVE_MODEL_LOGGING_MACRO_H_
#define WAVE_MODEL_LOGGING_MACRO_H_

#include <cassert>

#define WM_ASSERT(COND, MSG) \
    assert(((MSG), (COND)))

#endif // WAVE_MODEL_LOGGING_MACRO_H_

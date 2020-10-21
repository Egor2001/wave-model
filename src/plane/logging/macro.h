#ifndef WAVE_MODEL_LOGGING_MACRO_H_
#define WAVE_MODEL_LOGGING_MACRO_H_

#include "logger.h"
#include <cassert>

#if defined(WM_LOG_MACROS)
    #define WM_ASSERT(COND, MSG) \
        assert(((MSG) && (COND)))

    #define WM_LOG(...) \
        WmLogger::log(__VA_ARGS__)

#else // defined(WM_LOG_MACROS)
    #define WM_ASSERT(COND, MSG)
    #define WM_LOG(...)

#endif // defined(WM_LOG_MACROS)

#endif // WAVE_MODEL_LOGGING_MACRO_H_

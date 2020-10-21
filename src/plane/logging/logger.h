#ifndef WAVE_MODEL_LOGGING_LOGGER_H_
#define WAVE_MODEL_LOGGING_LOGGER_H_

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

namespace wave_model {

// TODO: to use spdlog/Boost.Log instead
class WmLogger
{
public:
    // allow only namespace-like usage
    WmLogger() = delete;

    static std::ofstream& stream()
    {
        // TODO: to use mutex for multithreaded version
        static std::ofstream stream;

        return stream;
    }

    static void init(const char* filename)
    {
        stream() = std::ofstream(filename);
    }

    template<typename... Types>
    static void log(Types&&... args)
    {
        auto timepoint = 
            std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now()
                    );

        ((stream() << "[" << std::ctime(&timepoint) << "\b] ")
                   << ... << std::forward<Types>(args)) << '\n';
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_LOGGING_LOGGER_H_

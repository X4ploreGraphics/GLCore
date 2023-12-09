

#ifndef _GLCORE_NITROS_INTERNAL_LOGGER_HPP
#define _GLCORE_NITROS_INTERNAL_LOGGER_HPP

#include <spdlog/spdlog.h>

namespace nitros::glcore
{
    namespace log{
        [[nodiscard]] auto Logger() -> std::shared_ptr<spdlog::logger>;
    }
} // namespace nitros::editor

#define LOG_D(...)    ::nitros::glcore::log::Logger()->debug(__VA_ARGS__);
#define LOG_W(...)    ::nitros::glcore::log::Logger()->warn (__VA_ARGS__);
#define LOG_I(...)    ::nitros::glcore::log::Logger()->info (__VA_ARGS__);
#define LOG_C(...)    ::nitros::glcore::log::Logger()->critical(__VA_ARGS__);
#define LOG_E(...)    ::nitros::glcore::log::Logger()->error(__VA_ARGS__);
#define LOG_Trace(...)    ::nitros::glcore::log::Logger()->trace(__VA_ARGS__);


#endif
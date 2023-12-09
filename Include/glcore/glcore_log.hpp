

#ifndef ENGINE_NITROS_EXPORT_LOGGER_HPP
#define ENGINE_NITROS_EXPORT_LOGGER_HPP

#include "glcore_export.h"
#include <spdlog/spdlog.h>

namespace nitros::glcore
{
    GLCORE_EXPORT std::shared_ptr<spdlog::logger> setup_logger(std::vector<spdlog::sink_ptr> sinks);
} // namespace nitros::engine


#endif
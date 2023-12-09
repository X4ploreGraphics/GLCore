

#include "glcore/glcore_log.hpp"
#include "logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace nitros::glcore
{
    std::shared_ptr<spdlog::logger> setup_logger(std::vector<spdlog::sink_ptr> sinks)
    {
        const auto logger_string = std::string{ "glcore" };
        auto logger = spdlog::get(logger_string);
        if(!logger)
        {
            if(sinks.size() > 0) {
                logger = std::make_shared<spdlog::logger>(logger_string, sinks.begin(), sinks.end());
                spdlog::register_logger(logger);
            }
            else {
                logger = spdlog::stdout_color_mt(logger_string);
            }
        }

        return logger;
    }

    namespace log
    {
        auto Logger() -> std::shared_ptr<spdlog::logger>
        {
            auto logger = spdlog::get( std::string{"glcore"} );
            if(logger)
                return logger;
            else
                return setup_logger({});
        }
        
    } // namespace log
}
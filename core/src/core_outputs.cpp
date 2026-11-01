#include "pinkrobin/core.hpp"

namespace pinkrobin {

    std::optional<Output> Core::next_output() const noexcept {
        return outputs_.front();
    }

    bool Core::acknowledge_output(
        std::uint64_t sequence) noexcept {

        return outputs_.acknowledge(sequence);
    }

    OutputEnqueueResult Core::request_weather_answer() noexcept {
        const ActionProposal proposal{
            WeatherAnswerAction{}
        };

        if (!supports_action(capabilities_, proposal)) {
            return OutputEnqueueResult::unsupported;
        }
        
        auto kind = WeatherAnswerKind::unknown;

        const auto forecast = weather();

        if (forecast) {
            if (forecast->precipitation_mm >= 10.0 &&
                forecast->wind_gust_kmh >= 60.0) {
                kind = WeatherAnswerKind::storm;
            }
            else if (forecast->precipitation_mm > 0.0) {
                kind = WeatherAnswerKind::rain;
            }
            else {
                kind = WeatherAnswerKind::dry;
            }
        }

        return outputs_.enqueue(WeatherAnswer{ kind });
    }
}
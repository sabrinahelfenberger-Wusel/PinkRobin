#pragma once

#include "pinkrobin/robin_state.hpp"
#include "pinkrobin/world_model.hpp"

namespace pinkrobin {

    class WeatherBehavior final {
    public:
        Transformation propose(
            const WorldModel& world) const noexcept;
    };

}
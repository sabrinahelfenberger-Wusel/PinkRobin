#pragma once

#include <cstdint>

namespace pinkrobin {
    enum class Mode : std::uint32_t { 
        sleeping = 0, 
        waking = 1, 
        awake = 2 
    };
    
    enum class Mood : std::uint32_t { 
        curious = 0, 
        happy = 1, 
        affectionate = 2 
    };

    enum class Expression : std::uint32_t { 
        sleeping = 0, 
        curious = 1, 
        happy = 2, 
        heart_eyes = 3 
    };

    enum class Transformation : std::uint32_t {
        none = 0,
        octopus = 1,
        raincoat = 2
    };

    struct RobinState {
        Mode mode{ Mode::sleeping };
        Mood mood{ Mood::curious };
        Expression expression{ Expression::sleeping };
        bool head_contact{ false };
        Transformation transformation{ Transformation::none };
    };
}
#pragma once
#include <cstdint>
#include <optional>
#include <cmath>
#include "pinkrobin/message.hpp"

namespace pinkrobin {
    enum class Mode : std::uint32_t { sleeping = 0, waking = 1, awake = 2 };
    enum class Mood : std::uint32_t { curious = 0, happy = 1, affectionate = 2 };
    enum class Expression : std::uint32_t { sleeping = 0, curious = 1, happy = 2, heart_eyes = 3 };
    enum class Event : std::uint32_t { restart = 0, heart_touched = 1, head_started = 2, head_ended = 3, interaction_cancelled = 4 };
    enum class Result : std::uint32_t { ok = 0, invalid_argument = 1, time_reversed = 2, busy = 3 };
    enum class Transformation : std::uint32_t { 
        none = 0, 
        octopus = 1,
        raincoat = 2
    };

    struct Snapshot {
        Mode mode;
        Mood mood;
        Expression expression;
        bool head_contact;
        std::uint64_t time_ms;
        std::uint64_t revision;
        Transformation transformation{ Transformation::none };
    };

    // One instance per robot or demo session. No hardware, clock, network or threads.
    // Adapters serialize access and supply monotonic milliseconds from one clock.
    class Core final {
    public:
        explicit Core(std::uint64_t now_ms = 0) noexcept;
        Result advance_to(std::uint64_t now_ms) noexcept;
        Result dispatch(Event event, std::uint64_t now_ms) noexcept;
        Result start_transformation(Transformation transformation, std::uint64_t now_ms) noexcept;
        Result post_weather(const WeatherUpdated& message) noexcept;
        std::optional<WeatherForecast> weather() const noexcept;
        Snapshot snapshot() const noexcept;
    private:
        void curious() noexcept;
        Mode mode_{Mode::sleeping};
        Mood mood_{Mood::curious};
        Expression expression_{Expression::sleeping};
        bool head_contact_{false};
        bool heart_pending_{false};
        std::uint64_t now_;
        std::uint64_t greeting_started_;
        std::uint64_t interaction_started_;
        std::uint64_t revision_{0};

        std::optional<WeatherUpdated> pending_weather_;
        std::optional<WeatherForecast> weather_;
        bool can_start_transformation() const noexcept;
        Transformation choose_weather_transformation() const noexcept;

        Transformation pending_transformation_{ Transformation::none };
        std::optional<std::uint64_t> weather_retry_started_;
        std::optional<std::uint64_t> weather_reaction_started_;
        void process_weather_reaction() noexcept;

        Transformation transformation_{ Transformation::none };
        std::uint64_t transformation_started_{ 0 };
    };
}

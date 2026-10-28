#pragma once
#include <cstdint>
#include <optional>
#include <cmath>
#include <array>
#include <cstddef>
#include "pinkrobin/message.hpp"

namespace pinkrobin {
    enum class Mode : std::uint32_t { sleeping = 0, waking = 1, awake = 2 };
    enum class Mood : std::uint32_t { curious = 0, happy = 1, affectionate = 2 };
    enum class Expression : std::uint32_t { sleeping = 0, curious = 1, happy = 2, heart_eyes = 3 };
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
        
        MessageAcceptance receive(const Message& message, std::uint64_t now_ms) noexcept;
        
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
        std::uint64_t pending_weather_received_at_ms_{ 0 };

        std::optional<WeatherForecast> weather_;

        bool can_start_transformation() const noexcept;
        Transformation choose_weather_transformation() const noexcept;

        Transformation pending_transformation_{ Transformation::none };
        std::optional<std::uint64_t> weather_retry_started_;
        std::optional<std::uint64_t> weather_reaction_started_;
        void process_weather_reaction() noexcept;
        
        static Result validate_weather(
            const WeatherForecast& forecast,
            std::uint64_t now_ms) noexcept;

        Transformation transformation_{ Transformation::none };
        std::uint64_t transformation_started_{ 0 };

        struct PendingInteraction {
            MessageMetadata metadata;
            InteractionReceived interaction;
            std::uint64_t received_at_ms;
        };

        static constexpr std::size_t interaction_capacity = 8;

        std::array<PendingInteraction, interaction_capacity>
            pending_interactions_{};

        std::size_t interaction_head_{ 0 };
        std::size_t interaction_count_{ 0 };

        MessageAcceptance enqueue_interaction(
            const MessageMetadata& metadata,
            const InteractionReceived& interaction,
            std::uint64_t received_at_ms) noexcept;

        std::optional<PendingInteraction> dequeue_interaction(
            std::uint64_t now_ms) noexcept;

        void apply_interaction(Event event) noexcept;
        std::uint64_t last_message_received_at_ms_{ 0 };
        Result advance_state_to(std::uint64_t now_ms) noexcept;
    };
}

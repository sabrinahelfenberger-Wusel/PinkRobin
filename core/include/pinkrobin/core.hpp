#pragma once
#include <cstdint>
#include <optional>
#include <cmath>
#include <array>
#include <cstddef>
#include "pinkrobin/message.hpp"
#include "pinkrobin/world_model.hpp"
#include "pinkrobin/robin_state.hpp"
#include "pinkrobin/weather_behavior.hpp"
#include "pinkrobin/behavior_controller.hpp"
#include "pinkrobin/output_queue.hpp"
#include "pinkrobin/platform_capabilities.hpp"
#include "pinkrobin/conversation_state.hpp"

namespace pinkrobin {
    enum class Result : std::uint32_t {
        ok = 0,
        invalid_argument = 1,
        time_reversed = 2,
        busy = 3,
        unsupported = 7
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
        explicit Core(
            std::uint64_t now_ms = 0,
            PlatformCapabilities capabilities = {}) noexcept;

        Result advance_to(std::uint64_t now_ms) noexcept;
        Result dispatch(Event event, std::uint64_t now_ms) noexcept;
        Result start_transformation(Transformation transformation, std::uint64_t now_ms) noexcept;
        Result post_weather(const WeatherUpdated& message) noexcept;
        
        MessageAcceptance receive(const Message& message, std::uint64_t now_ms) noexcept;
        
        std::optional<WeatherForecast> weather() const noexcept;
        Snapshot snapshot() const noexcept;

        const WorldModel& world() const noexcept;
        const RobinState& state() const noexcept;

        std::optional<Output> next_output() const noexcept;

        bool acknowledge_output(std::uint64_t sequence) noexcept;
        OutputEnqueueResult request_weather_answer() noexcept;

        const ConversationState& conversation() const noexcept;
        
    private:
        void curious() noexcept;
        bool heart_pending_{false};
        std::uint64_t now_;
        std::uint64_t greeting_started_;
        std::uint64_t interaction_started_;
        std::uint64_t revision_{0};

        std::optional<WeatherUpdated> pending_weather_;
        InputSource pending_weather_source_{ InputSource::backend };
        std::uint64_t pending_weather_received_at_ms_{ 0 };

        bool can_start_transformation() const noexcept;
        Transformation choose_weather_transformation() const noexcept;

        void process_weather_reaction() noexcept;
        
        static Result validate_weather(
            const WeatherForecast& forecast,
            std::uint64_t now_ms) noexcept;

        std::uint64_t transformation_started_{ 0 };

        using InputPayload = std::variant<
            InteractionReceived,
            TextReceived
        >;

        struct PendingInput {
            MessageMetadata metadata;
            InputPayload payload;
            std::uint64_t received_at_ms;
        };

        static constexpr std::size_t input_capacity = 8;

        std::array<PendingInput, input_capacity>
            pending_inputs_{};

        std::size_t input_head_{ 0 };
        std::size_t input_count_{ 0 };

        MessageAcceptance enqueue_input(
            const MessageMetadata& metadata,
            const InputPayload& payload,
            std::uint64_t received_at_ms) noexcept;

        std::optional<PendingInput> dequeue_input(
            std::uint64_t now_ms) noexcept;

        void apply_interaction(Event event) noexcept;
        std::uint64_t last_message_received_at_ms_{ 0 };
        Result advance_state_to(std::uint64_t now_ms) noexcept;

        PlatformCapabilities capabilities_;
        WorldModel world_;
        RobinState state_;
        ConversationState conversation_;
        WeatherBehavior weather_behavior_;
        BehaviorController behavior_controller_;
        OutputQueue outputs_;
    };
}

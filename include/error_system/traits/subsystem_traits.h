#pragma once

namespace error_system::traits {

    /**
     * @brief 子系统类型萃取基类模板
     */
    template <typename T, typename Enable = void>
    struct subsystem_traits {
        // using subsystem_t = subsystem::ai_llm_subsystem_t;
        
        // using underlying_t = std::underlying_type_t<subsystem_t>;
        
        // static constexpr T from_int(underlying_t value) noexcept;

        // static constexpr const char* to_string(T value) noexcept;
        
        // static constexpr bool is_valid(underlying_t value) noexcept;
        
        // static constexpr T from_string(const char* string) noexcept;
        
        // static constexpr uint16_t to_int(T value) noexcept;
    };

}  // namespace error_system::traits
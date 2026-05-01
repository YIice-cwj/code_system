#pragma once

namespace error_system::traits {

    /**
     * @brief 模块类型萃取基类模板
     */
    template <typename T, typename Enable = void>
    struct module_traits {
        // using module_t = module::system_module_t;
        
        // using underlying_t = std::underlying_type_t<module_t>;
        
        // static constexpr T from_int(underlying_t value) noexcept;

        //static constexpr const char* to_string(T value) noexcept;
        
        // static constexpr bool is_valid(underlying_t value) noexcept;
        
        // static constexpr T from_string(const char* string) noexcept;
        
        // static constexpr uint16_t to_int(T value) noexcept;
    };

}  // namespace error_system::traits
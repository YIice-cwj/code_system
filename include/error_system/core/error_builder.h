#pragma once
#include "error_system/core/error_code.h"
#include <cstdint>

/**
 * @file error_builder.h
 * @brief 错误码构建器
 * @details 提供构建错误码的工厂方法
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 错误码构建器
     * @details 提供静态工厂方法构建错误码
     */
    class error_builder_t {
        public:
        /**
         * @brief 构建错误码
         * @param level 错误等级
         * @param system 系统域
         * @param subsys 子系统值
         * @param module 模块值
         * @param number 错误编号
         * @return error_code_t 错误码对象
         */
        static constexpr error_code_t make_error_code(error_level_t level,
                                                      domain::system_domain_t system,
                                                      uint16_t subsys,
                                                      uint16_t module,
                                                      uint16_t number) noexcept {
            error_code_t::error_code_union_t error_union;
            error_union.fields.sign = 1;
            error_union.fields.reserved = 0;
            error_union.fields.level = level;
            error_union.fields.system = system;
            error_union.fields.subsys = subsys;
            error_union.fields.module = module;
            error_union.fields.number = number;
            return error_code_t(error_union.code);
        }

        /**
         * @brief 从原始码构建错误码
         * @param code 64位错误码
         * @return error_code_t 错误码对象
         */
        static constexpr error_code_t make_error_code(code_t code) noexcept { return error_code_t(code); }
    };

}  // namespace error_system::core

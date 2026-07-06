#pragma once
#include <cstdint>
#include <cstring>

#include "error_system/core/error_code.h"

/**
 * @file c_abi_export.h
 * @brief error_system C ABI 导出层（头文件声明 + inline C++ 实现）
 * @details 提供 C 兼容的句柄接口，使 error_code_t 可被 Python/Go/Rust 等
 *          通过 FFI 直接消费。本头文件仅涉及错误码（不涉及 error_context_t，
 *          后者因含 std::string/std::shared_ptr 等 C++ 类型不适合 C ABI）。
 *
 *          ABI 设计原则：
 *          - C ABI 句柄类型为 const void*，调用方不可解引用
 *          - 所有句柄必须通过 error_system_code_handle_free 释放
 *          - 所有接口 noexcept，仅使用 C 兼容类型
 *          - 字符串返回通过 out 参数 + 长度，调用方负责拷贝
 *
 *          本头文件提供 inline C++ 实现（C++ 调用方直接使用）；
 *          C 调用方需链接 src/bridge/c_abi_export.cc 中的 extern "C" 定义。
 *
 *          典型 FFI 用法（Python ctypes）：
 *          ```python
 *          lib = ctypes.CDLL("liberror_system.so")
 *          handle = lib.error_system_code_create(0x0000FFFE00010010)
 *          buf = ctypes.create_string_buffer(64)
 *          lib.error_system_code_message(handle, buf, 64)
 *          lib.error_system_code_handle_free(handle)
 *          ```
 *
 *          本头文件为可选包含，不污染 core 头文件依赖关系。
 * @author yiice
 * @version 3.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */
namespace error_system::abi {

    using error_system::core::code_t;
    using error_system::core::error_code_t;

    /**
     * @brief C ABI 句柄的 C++ 侧类型别名
     * @details 内部实现使用 const error_code_t*，C ABI 边界用 const void* 传递，
     *          避免在 extern "C" 函数签名中暴露 C++ 类型。
     */
    using handle_t = const error_code_t*;

    /**
     * @brief 句柄创建（C++ 侧实现）
     * @param raw_code 64 位原始错误码值
     * @return handle_t 句柄指针，调用方必须用 free_handle 释放；分配失败返回 nullptr
     *
     * 实现思路：堆分配 error_code_t 并返回指针。分配失败返回 nullptr。
     */
    inline handle_t create_handle(code_t raw_code) noexcept {
        try {
            return new error_code_t{raw_code};
        } catch (...) {
            return nullptr;
        }
    }

    /**
     * @brief 句柄释放（C++ 侧实现）
     * @param handle 待释放句柄，可为 nullptr（安全空操作）
     */
    inline void free_handle(handle_t handle) noexcept {
        delete handle;
    }

    /**
     * @brief void* 转 handle_t（边界转换辅助）
     * @details 将 C ABI 边界传入的 const void* 安全转换为内部 handle_t
     * @param opaque C ABI 边界传入的不透明指针
     * @return handle_t 转换后的句柄，可为 nullptr
     */
    inline handle_t to_handle(const void* opaque) noexcept {
        return static_cast<handle_t>(opaque);
    }

    /**
     * @brief handle_t 转 void*（边界转换辅助）
     * @param handle 待转换句柄，可为 nullptr
     * @return const void* C ABI 边界可用的不透明指针
     */
    inline const void* from_handle(handle_t handle) noexcept {
        return static_cast<const void*>(handle);
    }

    /**
     * @brief 获取原始 64 位错误码（C++ 侧实现）
     * @return code_t 原始错误码值；handle 为 nullptr 时返回 0
     */
    inline code_t get_raw(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->get_code();
    }

    /**
     * @brief 判断是否为错误码（C++ 侧实现）
     * @return int 1=错误，0=成功或 handle 无效
     */
    inline int is_error(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->is_error_code() ? 1 : 0;
    }

    /**
     * @brief 判断是否为成功码（C++ 侧实现）
     * @return int 1=成功，0=错误或 handle 无效
     */
    inline int is_success(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->is_success_code() ? 1 : 0;
    }

    /**
     * @brief 获取错误等级（C++ 侧实现）
     * @param handle 句柄，可为 nullptr
     * @return uint8_t 错误等级；handle 为 nullptr 时返回 0
     */
    inline uint8_t get_level(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return static_cast<uint8_t>(handle->get_level());
    }

    /**
     * @brief 获取系统域（C++ 侧实现）
     * @param handle 句柄，可为 nullptr
     * @return uint8_t 系统域；handle 为 nullptr 时返回 0
     */
    inline uint8_t get_system(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return static_cast<uint8_t>(handle->get_system());
    }

    /**
     * @brief 获取子系统 ID（C++ 侧实现）
     * @param handle 句柄，可为 nullptr
     * @return uint16_t 子系统 ID；handle 为 nullptr 时返回 0
     */
    inline uint16_t get_subsystem(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->get_subsys();
    }

    /**
     * @brief 获取模块 ID（C++ 侧实现）
     * @param handle 句柄，可为 nullptr
     * @return uint16_t 模块 ID；handle 为 nullptr 时返回 0
     */
    inline uint16_t get_module(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->get_module();
    }

    /**
     * @brief 获取错误编号（C++ 侧实现）
     * @param handle 句柄，可为 nullptr
     * @return uint16_t 错误编号；handle 为 nullptr 时返回 0
     */
    inline uint16_t get_number(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->get_number();
    }

    /**
     * @brief 判断是否可重试（C++ 侧实现）
     * @param handle 句柄，可为 nullptr
     * @return int 1=可重试，0=不可重试或 handle 无效
     */
    inline int is_retryable(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->is_retryable() ? 1 : 0;
    }

    /**
     * @brief 判断是否为瞬态错误（C++ 侧实现）
     * @param handle 句柄，可为 nullptr
     * @return int 1=瞬态错误，0=非瞬态或 handle 无效
     */
    inline int is_transient(handle_t handle) noexcept {
        if (handle == nullptr) { return 0; }
        return handle->is_transient() ? 1 : 0;
    }

    /**
     * @brief 获取错误码的十六进制字符串表示（C++ 侧实现）
     * @param handle 句柄
     * @param buf 输出缓冲区
     * @param buf_size 缓冲区容量
     * @return int 实际写入字节数（含末尾 '\0'）；buf 为空或 buf_size 不足返回 0
     *
     * 实现思路：写入形如 "0x0011223344556677" 的 18 字节字符串（含 '\0'）。
     *           缓冲区不足时返回 0，不写入。
     */
    inline int get_message(handle_t handle, char* buf, int buf_size) noexcept {
        if (handle == nullptr || buf == nullptr || buf_size < 19) { return 0; }
        std::snprintf(buf, static_cast<size_t>(buf_size), "0x%016llx",
                      static_cast<unsigned long long>(handle->get_code()));
        return 19;
    }

}  // namespace error_system::abi

/**
 * @brief C ABI 导出函数声明
 * @details 实现位于 src/bridge/c_abi_export.cc，C 调用方通过这些符号 FFI 访问。
 *          C++ 调用方应直接使用 error_system::abi 命名空间下的 inline 函数。
 */
#ifdef __cplusplus
extern "C" {
#endif

const void* error_system_code_create(uint64_t raw_code);
void error_system_code_handle_free(const void* handle);
uint64_t error_system_code_raw(const void* handle);
int error_system_code_is_error(const void* handle);
int error_system_code_is_success(const void* handle);
uint8_t error_system_code_level(const void* handle);
uint8_t error_system_code_system(const void* handle);
uint16_t error_system_code_subsystem(const void* handle);
uint16_t error_system_code_module(const void* handle);
uint16_t error_system_code_number(const void* handle);
int error_system_code_is_retryable(const void* handle);
int error_system_code_is_transient(const void* handle);
int error_system_code_message(const void* handle, char* buf, int buf_size);

#ifdef __cplusplus
}  // extern "C"
#endif

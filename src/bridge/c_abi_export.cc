#include "error_system/bridge/c_abi_export.h"

/**
 * @file c_abi_export.cc
 * @brief error_system C ABI extern "C" 定义
 * @details 为 C ABI 头文件声明提供 extern "C" 符号定义，使 error_code_t
 *          可被 Python/Go/Rust 等通过 FFI 直接消费。所有函数委托给
 *          error_system::abi 命名空间下的 inline C++ 实现，避免逻辑重复。
 * @author yiice
 * @version 3.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */

extern "C" {

const void* error_system_code_create(uint64_t raw_code) {
    return error_system::abi::from_handle(error_system::abi::create_handle(raw_code));
}

void error_system_code_handle_free(const void* handle) {
    error_system::abi::free_handle(error_system::abi::to_handle(handle));
}

uint64_t error_system_code_raw(const void* handle) {
    return error_system::abi::get_raw(error_system::abi::to_handle(handle));
}

int error_system_code_is_error(const void* handle) {
    return error_system::abi::is_error(error_system::abi::to_handle(handle));
}

int error_system_code_is_success(const void* handle) {
    return error_system::abi::is_success(error_system::abi::to_handle(handle));
}

uint8_t error_system_code_level(const void* handle) {
    return error_system::abi::get_level(error_system::abi::to_handle(handle));
}

uint8_t error_system_code_system(const void* handle) {
    return error_system::abi::get_system(error_system::abi::to_handle(handle));
}

uint16_t error_system_code_subsystem(const void* handle) {
    return error_system::abi::get_subsystem(error_system::abi::to_handle(handle));
}

uint16_t error_system_code_module(const void* handle) {
    return error_system::abi::get_module(error_system::abi::to_handle(handle));
}

uint16_t error_system_code_number(const void* handle) {
    return error_system::abi::get_number(error_system::abi::to_handle(handle));
}

int error_system_code_is_retryable(const void* handle) {
    return error_system::abi::is_retryable(error_system::abi::to_handle(handle));
}

int error_system_code_is_transient(const void* handle) {
    return error_system::abi::is_transient(error_system::abi::to_handle(handle));
}

int error_system_code_message(const void* handle, char* buf, int buf_size) {
    return error_system::abi::get_message(error_system::abi::to_handle(handle), buf, buf_size);
}

}  // extern "C"

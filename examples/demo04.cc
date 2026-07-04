/**
 * @file demo04.cc
 * @brief error_exception_t 异常传递机制 API 全览
 * @details 演示 error_exception_t 的全部公共 API：构造、what()、context()、code()、
 *          异常捕获、嵌套异常、与 result_t 互转。
 * @note 项目规范要求函数 noexcept，生产代码优先用 result_t。throw 仅为演示异常机制。
 */

#include <iostream>
#include <string>

#include "error_system.h"
// IWYU pragma: begin_exports
#include "trade_service_errors.h"
// IWYU pragma: end_exports

using namespace error_system::core;
using namespace error_system::domain;

namespace {

/** @brief 打印小节标题 */
void section(const std::string& title) {
    std::cout << "\n--- " << title << " ---" << std::endl;
}

/** @brief 模拟抛出异常的订单处理 */
void process_order(int order_id) {
    if (order_id <= 0) {
        throw error_exception_t(error_context_t{
            biz::trade_errors::ERR_ORDER_NOT_FOUND, "无效的订单ID"});
    }
    if (order_id == 404) {
        throw error_exception_t(error_context_t{
            biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单不存在"});
    }
    std::cout << "  订单 " << order_id << " 处理成功" << std::endl;
}

/** @brief 1.1 构造：error_exception_t(error_context_t) */
void demo_construct() {
    section("1.1 构造：error_exception_t(error_context_t)");
    try {
        process_order(-1);
    } catch (const error_exception_t&) {
        std::cout << "  捕获异常" << std::endl;
    }
}

/** @brief 2.1 what() 获取错误描述（缓存字符串） */
void demo_what() {
    section("2.1 what() 获取错误描述（缓存字符串）");
    try {
        process_order(404);
    } catch (const error_exception_t& e) {
        std::cout << "  what() = " << e.what() << std::endl;
    }
}

/** @brief 2.2 context() 获取完整错误上下文 */
void demo_context() {
    section("2.2 context() 获取完整错误上下文");
    try {
        process_order(404);
    } catch (const error_exception_t& e) {
        const error_context_t& ctx = e.context();
        std::cout << "  message = " << ctx.message << std::endl;
        std::cout << "  is_fatal = " << ctx.is_fatal() << std::endl;
        std::cout << "  to_string = " << ctx.to_string() << std::endl;
    }
}

/** @brief 2.3 code() 获取错误码 */
void demo_code() {
    section("2.3 code() 获取错误码");
    try {
        process_order(404);
    } catch (const error_exception_t& e) {
        std::cout << "  identity_code = " << e.code().get_identity_code() << std::endl;
        std::cout << "  level = " << static_cast<int>(e.code().get_level()) << std::endl;
        std::cout << "  is_error_code = " << e.code().is_error_code() << std::endl;
    }
}

/** @brief 3.1 作为 std::exception 捕获（基类兼容） */
void demo_catch_std_exception() {
    section("3.1 作为 std::exception 捕获（基类兼容）");
    try {
        process_order(404);
    } catch (const std::exception& e) {
        std::cout << "  std::exception::what() = " << e.what() << std::endl;
    }
}

/** @brief 4.1 嵌套 try-catch（内层异常 + 外层包装） */
void demo_nested_exception() {
    section("4.1 嵌套 try-catch（内层异常 + 外层包装）");
    try {
        try {
            process_order(404);
        } catch (const error_exception_t& inner) {
            std::cout << "  内层: " << inner.what() << std::endl;
            error_context_t outer_ctx{biz::trade_errors::ERR_CART_IS_EMPTY, "订单服务调用失败"};
            outer_ctx.with("inner_code", std::to_string(inner.code().get_identity_code()));
            throw error_exception_t(std::move(outer_ctx));
        }
    } catch (const error_exception_t& outer) {
        std::cout << "  外层: " << outer.what() << std::endl;
        std::cout << "  payload 项数: " << outer.context().payload_size() << std::endl;
    }
}

/** @brief 5.1 异常转 result_t（桥接 noexcept 边界） */
void demo_exception_to_result() {
    section("5.1 异常转 result_t（桥接 noexcept 边界）");
    auto safe_call = [](int id) -> result_t<int> {
        try {
            process_order(id);
            return result_t<int>{id * 100};
        } catch (const error_exception_t& e) {
            return result_t<int>{e.context()};
        }
    };

    auto r1 = safe_call(123);
    std::cout << "  成功: value = " << r1.value() << std::endl;

    auto r2 = safe_call(404);
    std::cout << "  失败: " << r2.error().message << std::endl;
}

/** @brief 6.1 正常流程（无异常抛出） */
void demo_normal_flow() {
    section("6.1 正常流程（无异常抛出）");
    try {
        process_order(123);
        std::cout << "  流程正常完成" << std::endl;
    } catch (const error_exception_t& e) {
        std::cout << "  不应到达: " << e.what() << std::endl;
    }
}

}  // namespace

int main() {
    std::cout << "===== Demo 4: error_exception_t API 全览 =====" << std::endl;

    demo_construct();

    demo_what();
    demo_context();
    demo_code();

    demo_catch_std_exception();

    demo_nested_exception();

    demo_exception_to_result();

    demo_normal_flow();

    return 0;
}

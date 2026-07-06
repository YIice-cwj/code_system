/**
 * @file demo06.cc
 * @brief 高级特性 API 全览：序列化器 / 因果链 / 自定义格式化器 / join_errors / TRY 宏 / 端到端
 * @details 演示 error_context_serializer_t 的全部公共 API（to_string/to_json/to_binary 静态方法、
 *          from_json/from_binary 反序列化、BINARY_MAGIC/BINARY_VERSION 常量、
 *          set_subsystem_module_resolver 自定义解析器）、error_context_t::wrap 因果链序列化、
 *          formatter_config_t 自定义格式化器、join_errors 聚合多个错误、
 *          ERROR_SYSTEM_TRY 早返回宏、以及一个综合的订单服务端到端场景。
 *          每个示例独立成函数，标题即 API 名。
 * @note 项目规范要求函数 noexcept，生产代码优先用 result_t；本示例仅演示 API 用法。
 */

#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "error_system.h"
#include "error_system/config/error_config.h"
#include "error_system/core/serializer/error_context_serializer.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/i18n/i_subsystem_module_resolver.h"
#include "error_system/plugin/plugin_registry.h"
// IWYU pragma: begin_exports
#include "payment_service_errors.h"
#include "redis_component_errors.h"
#include "trade_service_errors.h"
#include "user_service_errors.h"
// IWYU pragma: end_exports

using namespace error_system::core;
using namespace error_system::config;
using namespace error_system::domain;
namespace i18n = error_system::i18n;
using error_system::plugin::i_error_plugin_t;
using error_system::plugin::plugin_registry_t;

namespace {

/** @brief 打印小节标题 */
void section(const std::string& title) {
    std::cout << "\n--- " << title << " ---" << std::endl;
}

/** @brief 自定义统计插件：统计 error 及以上级别错误次数 */
class stats_plugin_t : public i_error_plugin_t {
    std::unordered_map<uint64_t, std::atomic<int>> counters_;

public:
    std::string_view name() const noexcept override { return "stats"; }

    error_level_t min_level() const noexcept override {
        return error_level_t::error;
    }

    void on_error(const error_context_t& context) noexcept override {
        try {
            ++counters_[context.get_code().get_code()];
        } catch (const std::bad_alloc&) {
        }
    }

    int total() const noexcept {
        int sum = 0;
        for (const auto& [_, c] : counters_) { sum += c.load(); }
        return sum;
    }
};

/** @brief 自定义子系统/模块解析器：返回固定的中文显示名（演示注入接口） */
class custom_resolver_t : public i18n::i_subsystem_module_resolver_t {
public:
    i18n::subsystem_module_info_t resolve_subsystem_module(
        i18n::locale_t /*output_locale*/,
        i18n::locale_t /*fallback_locale*/,
        uint16_t subsystem_id,
        uint16_t module_id) const noexcept override {
        i18n::subsystem_module_info_t info;
        info.subsystem_name = "自定义子系统#" + std::to_string(subsystem_id);
        info.module_name = "自定义模块#" + std::to_string(module_id);
        return info;
    }
};

/** @brief 模拟订单查询：成功返回金额，失败返回错误 */
result_t<int> query_order(int order_id) {
    if (order_id <= 0) {
        return result_t<int>::make_error(biz::trade_errors::ERR_ORDER_NOT_FOUND, "无效的订单ID");
    }
    if (order_id == 404) {
        return result_t<int>::make_error(biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单不存在");
    }
    return result_t<int>{order_id * 1000};
}

/** @brief 1.1 error_context_t::to_string 渲染为可读文本 */
void demo_to_string_member() {
    section("1.1 error_context_t::to_string 渲染为可读文本");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单查询失败"};
    ctx.with("order_id", 8848).with("retry_count", 3);
    std::cout << ctx.to_string() << std::endl;
}

/** @brief 1.2 error_context_t::to_json 渲染为 JSON 字符串 */
void demo_to_json_member() {
    section("1.2 error_context_t::to_json 渲染为 JSON 字符串");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "JSON 序列化测试"};
    ctx.with("user_id", "8848").with("channel", "mobile");
    std::cout << "  " << ctx.to_json() << std::endl;
}

/** @brief 1.3 error_context_t::to_binary 渲染为紧凑二进制 */
void demo_to_binary_member() {
    section("1.3 error_context_t::to_binary 渲染为紧凑二进制");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "二进制序列化测试"};
    ctx.with("user_id", "8848");
    const std::string blob = ctx.to_binary();
    std::cout << "  二进制大小 = " << blob.size() << " bytes" << std::endl;
}

/** @brief 2.1 error_context_serializer_t::to_string 静态调用 */
void demo_serializer_to_string() {
    section("2.1 error_context_serializer_t::to_string 静态调用");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "静态 to_string"};
    std::cout << error_context_serializer_t::to_string(ctx) << std::endl;
}

/** @brief 2.2 error_context_serializer_t::to_json 静态调用 */
void demo_serializer_to_json() {
    section("2.2 error_context_serializer_t::to_json 静态调用");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "静态 to_json"};
    ctx.with("key", "value");
    std::cout << "  " << error_context_serializer_t::to_json(ctx) << std::endl;
}

/** @brief 2.3 error_context_serializer_t::to_binary 静态调用 */
void demo_serializer_to_binary() {
    section("2.3 error_context_serializer_t::to_binary 静态调用");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "静态 to_binary"};
    const std::string blob = error_context_serializer_t::to_binary(ctx);
    std::cout << "  二进制大小 = " << blob.size() << " bytes" << std::endl;
}

/** @brief 2.4 BINARY_MAGIC / BINARY_VERSION 常量 */
void demo_binary_constants() {
    section("2.4 BINARY_MAGIC / BINARY_VERSION 常量");
    std::cout << "  BINARY_MAGIC   = 0x" << std::hex << error_context_serializer_t::BINARY_MAGIC
              << std::dec << std::endl;
    std::cout << "  BINARY_VERSION = " << static_cast<int>(error_context_serializer_t::BINARY_VERSION)
              << std::endl;
}

/** @brief 3.1 from_json 反序列化合法 JSON（往返一致性验证） */
void demo_from_json_valid() {
    section("3.1 from_json 反序列化合法 JSON（往返一致性验证）");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "JSON 往返测试"};
    ctx.with("user_id", "8848").with("retry_count", 3);
    const std::string json_str = ctx.to_json();
    auto restored = error_context_serializer_t::from_json(json_str);
    if (restored.has_value()) {
        const bool same = restored->get_code().get_code() == ctx.get_code().get_code()
                          && restored->get_message() == ctx.get_message()
                          && restored->payload_size() == ctx.payload_size();
        std::cout << "  code 还原    = " << restored->get_code().get_code() << std::endl;
        std::cout << "  message 还原 = " << restored->get_message() << std::endl;
        std::cout << "  payload 项数 = " << restored->payload_size() << std::endl;
        std::cout << "  往返一致性   = " << (same ? "通过" : "失败") << std::endl;
    } else {
        std::cout << "  反序列化失败" << std::endl;
    }
}

/** @brief 3.2 from_json 处理非法 JSON（返回 nullopt） */
void demo_from_json_invalid() {
    section("3.2 from_json 处理非法 JSON（返回 nullopt）");
    auto bad = error_context_serializer_t::from_json("{not a json");
    std::cout << "  非法 JSON has_value = " << bad.has_value() << std::endl;
}

/** @brief 3.3 from_binary 反序列化合法二进制（往返一致性验证） */
void demo_from_binary_valid() {
    section("3.3 from_binary 反序列化合法二进制（往返一致性验证）");
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "二进制往返测试"};
    ctx.with("user_id", "8848").with("channel", "mobile");
    const std::string blob = ctx.to_binary();
    auto restored = error_context_serializer_t::from_binary(blob);
    if (restored.has_value()) {
        const bool same = restored->get_code().get_code() == ctx.get_code().get_code()
                          && restored->get_message() == ctx.get_message();
        std::cout << "  code 还原    = " << restored->get_code().get_code() << std::endl;
        std::cout << "  message 还原 = " << restored->get_message() << std::endl;
        std::cout << "  往返一致性   = " << (same ? "通过" : "失败") << std::endl;
    } else {
        std::cout << "  反序列化失败" << std::endl;
    }
}

/** @brief 3.4 from_binary 处理截断数据（返回 nullopt） */
void demo_from_binary_truncated() {
    section("3.4 from_binary 处理截断数据（返回 nullopt）");
    auto bad_bin = error_context_serializer_t::from_binary(std::string_view{"\x00\x01"});
    std::cout << "  截断数据 has_value = " << bad_bin.has_value() << std::endl;
}

/** @brief 4.1 wrap(const error_context_t&) 包装底层错误（左值版本） */
void demo_wrap_lvalue() {
    section("4.1 wrap(const error_context_t&) 包装底层错误（左值版本）");
    error_context_t root{infra::redis_errors::ERR_KEY_NOT_FOUND, "Redis 键不存在"};
    root.with("redis_key", "session:user:8848");
    error_context_t top{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单创建失败"};
    const error_context_t& root_ref = root;
    error_context_t wrapped = top.wrap(root_ref.clone());
    std::cout << "  顶层 message = " << wrapped.get_message() << std::endl;
    if (wrapped.cause()) {
        std::cout << "  cause message = " << wrapped.cause()->get_message() << std::endl;
    }
}

/** @brief 4.2 wrap(error_context_t&&) 包装底层错误（右值版本） */
void demo_wrap_rvalue() {
    section("4.2 wrap(error_context_t&&) 包装底层错误（右值版本）");
    error_context_t top{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单创建失败"};
    error_context_t wrapped = top.wrap(error_context_t{infra::redis_errors::ERR_KEY_NOT_FOUND, "Redis 键不存在"});
    std::cout << "  顶层 message = " << wrapped.get_message() << std::endl;
    if (wrapped.cause()) {
        std::cout << "  cause message = " << wrapped.cause()->get_message() << std::endl;
    }
}

/** @brief 4.3 因果链 to_json 序列化（递归渲染 cause 字段） */
void demo_chain_to_json() {
    section("4.3 因果链 to_json 序列化（递归渲染 cause 字段）");
    error_context_t root{infra::redis_errors::ERR_KEY_NOT_FOUND, "Redis 键不存在"};
    error_context_t mid{biz::payment_errors::ERR_ACCOUNT_FROZEN, "支付服务调用失败"};
    error_context_t top{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单创建失败"};
    error_context_t chain = top.wrap(mid.wrap(std::move(root)));
    std::cout << "  " << chain.to_json() << std::endl;
}

/** @brief 4.4 因果链 to_binary 大小对比（单层 vs 三层） */
void demo_chain_to_binary_size() {
    section("4.4 因果链 to_binary 大小对比（单层 vs 三层）");
    error_context_t root{infra::redis_errors::ERR_KEY_NOT_FOUND, "Redis 键不存在"};
    error_context_t mid{biz::payment_errors::ERR_ACCOUNT_FROZEN, "支付服务调用失败"};
    error_context_t top{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单创建失败"};
    error_context_t chain = top.wrap(mid.wrap(std::move(root)));
    std::cout << "  单层大小 = " << top.to_binary().size() << " bytes" << std::endl;
    std::cout << "  三层大小 = " << chain.to_binary().size() << " bytes" << std::endl;
}

/** @brief 4.5 因果链 from_binary 反序列化还原（递归还原 cause 链） */
void demo_chain_from_binary() {
    section("4.5 因果链 from_binary 反序列化还原（递归还原 cause 链）");
    error_context_t root{infra::redis_errors::ERR_KEY_NOT_FOUND, "Redis 键不存在"};
    error_context_t mid{biz::payment_errors::ERR_ACCOUNT_FROZEN, "支付服务调用失败"};
    error_context_t top{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单创建失败"};
    error_context_t chain = top.wrap(mid.wrap(std::move(root)));
    auto restored = error_context_serializer_t::from_binary(chain.to_binary());
    if (restored.has_value() && restored->cause()) {
        std::cout << "  顶层 message = " << restored->get_message() << std::endl;
        std::cout << "  中层 message = " << restored->cause()->get_message() << std::endl;
        if (restored->cause()->cause()) {
            std::cout << "  底层 message = " << restored->cause()->cause()->get_message() << std::endl;
        }
    } else {
        std::cout << "  反序列化失败" << std::endl;
    }
}

/** @brief 4.6 equals_strict 严格比较因果链（含 cause 链深比较） */
void demo_equals_strict() {
    section("4.6 equals_strict 严格比较因果链（含 cause 链深比较）");
    error_context_t root_a{infra::redis_errors::ERR_KEY_NOT_FOUND, "Redis 键不存在"};
    error_context_t top_a{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单创建失败"};
    error_context_t chain_a = top_a.wrap(std::move(root_a));

    error_context_t root_b{infra::redis_errors::ERR_KEY_NOT_FOUND, "Redis 键不存在"};
    error_context_t top_b{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单创建失败"};
    error_context_t chain_b = top_b.wrap(std::move(root_b));

    std::cout << "  equals_strict (同结构) = " << chain_a.equals_strict(chain_b) << std::endl;
    std::cout << "  equals_by_code         = " << chain_a.equals_by_code(chain_b) << std::endl;
    std::cout << "  operator==             = " << (chain_a == chain_b) << std::endl;
}

/** @brief 5.1 set_custom_formatter 注册 logfmt 格式器 */
void demo_set_custom_formatter() {
    section("5.1 set_custom_formatter 注册 logfmt 格式器");
    formatter_config_t::set_custom_formatter(
        [](const error_context_t& ctx) -> std::string {
            std::string out = "level=";
            out += to_string(ctx.get_code().get_level());
            out += " msg=\"";
            out += ctx.get_message();
            out += "\" code=";
            out += std::to_string(ctx.get_code().get_identity_code());
            ctx.for_each_payload([&out](const std::string& k, const std::string& v) {
                out += " ";
                out += k;
                out += "=";
                out += v;
            });
            return out;
        });
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "自定义格式器测试"};
    ctx.with("user_id", "8848").with("retry_count", 3);
    std::cout << "  " << ctx.to_string() << std::endl;
}

/** @brief 5.2 get_custom_formatter 获取当前格式化器 */
void demo_get_custom_formatter() {
    section("5.2 get_custom_formatter 获取当前格式化器");
    auto current = formatter_config_t::get_custom_formatter();
    std::cout << "  已设置自定义格式器 = " << (current ? 1 : 0) << std::endl;
}

/** @brief 5.3 set_custom_formatter(nullptr) 恢复默认格式器 */
void demo_clear_custom_formatter() {
    section("5.3 set_custom_formatter(nullptr) 恢复默认格式器");
    formatter_config_t::set_custom_formatter(nullptr);
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "恢复默认格式器"};
    std::cout << "  " << ctx.to_string() << std::endl;
}

/** @brief 6.1 set_subsystem_module_resolver 注入自定义解析器 */
void demo_set_custom_resolver() {
    section("6.1 set_subsystem_module_resolver 注入自定义解析器");
    custom_resolver_t resolver;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver);
    i18n_config_t::set_enable_i18n(true);
    i18n_config_t::clear_output_locale();
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "自定义解析器测试"};
    std::cout << ctx.to_string() << std::endl;
}

/** @brief 6.2 set_subsystem_module_resolver(nullptr) 恢复默认解析器 */
void demo_clear_custom_resolver() {
    section("6.2 set_subsystem_module_resolver(nullptr) 恢复默认解析器");
    error_context_serializer_t::set_subsystem_module_resolver(nullptr);
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "恢复默认解析器"};
    std::cout << ctx.to_string() << std::endl;
}

/** @brief 7.1 join_errors 空列表（返回默认成功上下文） */
void demo_join_errors_empty() {
    section("7.1 join_errors 空列表（返回默认成功上下文）");
    std::vector<error_context_t> empty_errs;
    auto joined = join_errors(std::move(empty_errs));
    std::cout << "  is_success = " << joined.is_success() << std::endl;
    std::cout << "  is_error   = " << joined.is_error() << std::endl;
}

/** @brief 7.2 join_errors 单元素（直接返回，零开销） */
void demo_join_errors_single() {
    section("7.2 join_errors 单元素（直接返回，零开销）");
    std::vector<error_context_t> single_errs;
    single_errs.emplace_back(biz::trade_errors::ERR_ORDER_NOT_FOUND, "单条错误");
    auto joined = join_errors(std::move(single_errs));
    std::cout << "  message      = " << joined.get_message() << std::endl;
    std::cout << "  payload 项数 = " << joined.payload_size() << " (单元素不附加 payload)" << std::endl;
}

/** @brief 7.3 join_errors 多元素（主错误 + joined_error_1..N payload） */
void demo_join_errors_multi() {
    section("7.3 join_errors 多元素（主错误 + joined_error_1..N payload）");
    std::vector<error_context_t> multi_errs;
    multi_errs.reserve(10);
    for (int i = 0; i < 10; ++i) {
        multi_errs.emplace_back(biz::trade_errors::ERR_ORDER_NOT_FOUND, "错误" + std::to_string(i));
    }
    auto joined = join_errors(std::move(multi_errs));
    std::cout << "  预期 payload 项数 = 9 (joined_error_1..9)" << std::endl;
    std::cout << "  主错误 message    = " << joined.get_message() << std::endl;
    std::cout << "  实际 payload 项数 = " << joined.payload_size() << std::endl;
    joined.for_each_payload([](const std::string& k, const std::string& v) {
        std::cout << "    " << k << " = " << v << std::endl;
    });
}

/** @brief 8.1 ERROR_SYSTEM_TRY 保留成功值并早返回错误 */
void demo_try_macro() {
    section("8.1 ERROR_SYSTEM_TRY 保留成功值并早返回错误");
    auto try_demo = [](int id) -> result_t<int> {
        ERROR_SYSTEM_TRY(amount, query_order(id));
        return result_t<int>{amount.value() * 2};
    };
    std::cout << "  成功: " << try_demo(123).value() << std::endl;
    std::cout << "  失败: " << try_demo(404).error().get_message() << std::endl;
}

/** @brief 8.2 ERROR_SYSTEM_TRY_DISCARD 丢弃成功值（仅传播错误） */
void demo_try_discard_macro() {
    section("8.2 ERROR_SYSTEM_TRY_DISCARD 丢弃成功值（仅传播错误）");
    auto try_discard = [](int id) -> result_t<int> {
        ERROR_SYSTEM_TRY_DISCARD(query_order(id));
        return result_t<int>{0};
    };
    std::cout << "  成功: " << try_discard(123).value() << " (丢弃查询值)" << std::endl;
    std::cout << "  失败: " << try_discard(404).error().get_message() << std::endl;
}

/** @brief 8.3 and_then 链式（跨类型：查订单 → 校验金额 → 支付） */
void demo_and_then_pipeline() {
    section("8.3 and_then 链式（跨类型：查订单 → 校验金额 → 支付）");
    auto validate_amount = [](int amount) -> result_t<int> {
        if (amount <= 0) {
            return result_t<int>::make_error(biz::payment_errors::ERR_INSUFFICIENT_BALANCE, "金额必须大于 0");
        }
        return result_t<int>{amount};
    };
    auto pay = [](int amount) -> result_t<std::string> {
        return result_t<std::string>::make_success("PAY_" + std::to_string(amount));
    };
    auto pipeline = [&](int order_id) -> result_t<std::string> {
        return query_order(order_id)
            .and_then(validate_amount)
            .and_then(pay);
    };
    std::cout << "  成功: " << pipeline(123).value() << std::endl;
    std::cout << "  失败: " << pipeline(404).error().get_message() << std::endl;
}

/** @brief 9.1 register_plugin_ref 注册统计插件（min_level 过滤） */
void demo_register_stats_plugin(stats_plugin_t& stats) {
    section("9.1 register_plugin_ref 注册统计插件（min_level=error 过滤）");
    auto& plugin_registry = plugin_registry_t::instance();
    plugin_registry.register_plugin_ref(stats);
    std::cout << "  插件数量 = " << plugin_registry.size() << std::endl;
    std::cout << "  初始错误数 = " << stats.total() << std::endl;
}

/** @brief 9.2 综合场景：用 and_then + context + wrap 串联订单流程 */
result_t<void> run_order_pipeline(int user_id, int quantity, int amount) {
    auto validate_user = [](int uid) -> result_t<void> {
        if (uid <= 0) {
            return result_t<void>::make_error(biz::user_errors::ERR_TOKEN_EXPIRED, "无效的用户ID");
        }
        return result_t<void>::make_success();
    };

    auto deduct_stock = [](int qty) -> result_t<void> {
        if (qty <= 0) {
            return result_t<void>::make_error(biz::trade_errors::ERR_ORDER_NOT_FOUND, "库存数量必须大于 0");
        }
        return result_t<void>::make_success();
    };

    auto process_payment = [](int amt) -> result_t<void> {
        if (amt <= 0) {
            error_context_t root_cause{infra::redis_errors::ERR_KEY_NOT_FOUND, "支付通道签名缓存丢失"};
            error_context_t wrapper{biz::payment_errors::ERR_ACCOUNT_FROZEN, "支付处理失败"};
            return result_t<void>::make_error(wrapper.wrap(std::move(root_cause)));
        }
        return result_t<void>::make_success();
    };

    return validate_user(user_id)
        .and_then([&](void) { return deduct_stock(quantity); })
        .and_then([&](void) { return process_payment(amount); })
        .context("user_id", std::to_string(user_id))
        .context("quantity", std::to_string(quantity));
}

/** @brief 9.3 场景A：全部成功 */
void demo_scenario_all_success() {
    section("9.3 场景A：全部成功");
    auto result_a = run_order_pipeline(8848, 2, 199);
    std::cout << "  结果 = " << (result_a.is_success() ? "成功" : "失败") << std::endl;
}

/** @brief 9.4 场景B：支付失败（带因果链 + context 附加） */
void demo_scenario_payment_failed() {
    section("9.4 场景B：支付失败（带因果链 + context 附加）");
    auto result_b = run_order_pipeline(8848, 2, 0);
    if (result_b.is_error()) {
        std::cout << result_b.error().to_string() << std::endl;
        std::cout << "  payload 项数 = " << result_b.error().payload_size() << std::endl;
        if (result_b.error().cause()) {
            std::cout << "  cause message = " << result_b.error().cause()->get_message() << std::endl;
        }
    }
}

/** @brief 9.5 场景C：用户校验失败 */
void demo_scenario_user_invalid() {
    section("9.5 场景C：用户校验失败");
    auto result_c = run_order_pipeline(-1, 2, 199);
    if (result_c.is_error()) {
        std::cout << "  " << result_c.error().get_message() << std::endl;
    }
}

/** @brief 9.6 批量验证 min_level 过滤：循环 50 error + 50 info */
void demo_batch_min_level_filter() {
    section("9.6 批量验证 min_level 过滤：循环 50 error + 50 info");
    constexpr int BATCH_ERROR = 50;
    constexpr int BATCH_INFO = 50;
    for (int i = 0; i < BATCH_ERROR; ++i) {
        error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "批量error" + std::to_string(i)};
        (void)ctx;
    }
    for (int i = 0; i < BATCH_INFO; ++i) {
        error_context_t ctx{biz::user_errors::ERR_TOKEN_EXPIRED, "批量info" + std::to_string(i)};
        (void)ctx;
    }
    std::cout << "  预期新增 " << BATCH_ERROR
              << " (error 被统计，info 被 min_level=error 过滤)" << std::endl;
}

/** @brief 9.7 插件统计结果 + unregister_plugin */
void demo_stats_result_and_unregister(stats_plugin_t& stats) {
    section("9.7 插件统计结果 + unregister_plugin");
    auto& plugin_registry = plugin_registry_t::instance();
    constexpr int EXPECTED_TOTAL = 1 + 50;
    std::cout << "  预期 total = " << EXPECTED_TOTAL
              << " (场景B fatal 1 + 批量 error 50)" << std::endl;
    std::cout << "  实际 total = " << stats.total() << std::endl;
    plugin_registry.unregister_plugin("stats");
    std::cout << "  注销后插件数量 = " << plugin_registry.size() << std::endl;
}

}  // namespace

int main() {
    std::cout << "===== Demo 6: 高级特性 API 全览 =====" << std::endl;

    demo_to_string_member();
    demo_to_json_member();
    demo_to_binary_member();

    demo_serializer_to_string();
    demo_serializer_to_json();
    demo_serializer_to_binary();
    demo_binary_constants();

    demo_from_json_valid();
    demo_from_json_invalid();
    demo_from_binary_valid();
    demo_from_binary_truncated();

    demo_wrap_lvalue();
    demo_wrap_rvalue();
    demo_chain_to_json();
    demo_chain_to_binary_size();
    demo_chain_from_binary();
    demo_equals_strict();

    demo_set_custom_formatter();
    demo_get_custom_formatter();
    demo_clear_custom_formatter();

    demo_set_custom_resolver();
    demo_clear_custom_resolver();

    demo_join_errors_empty();
    demo_join_errors_single();
    demo_join_errors_multi();

    demo_try_macro();
    demo_try_discard_macro();
    demo_and_then_pipeline();

    stats_plugin_t stats;
    demo_register_stats_plugin(stats);
    demo_scenario_all_success();
    demo_scenario_payment_failed();
    demo_scenario_user_invalid();
    demo_batch_min_level_filter();
    demo_stats_result_and_unregister(stats);

    return 0;
}

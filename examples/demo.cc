#include <iostream>
#include <memory>
#include "error_system.h"
#include "error_system/i18n/json_translator.h"

using namespace error_system;

// ==========================================
// 1. 底层：模拟数据库连接失败
// ==========================================
core::result_t<void> connect_database(const std::string& host) {
    if (host.empty()) {
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, 
            domain::system_domain_t::database, 
            traits::subsystem_traits<subsystem::database_subsystem_t>::to_int(subsystem::database_subsystem_t::mysql),
            traits::module_traits<module::database_module_t>::to_int(module::database_module_t::connection),
            1001 // Invalid Host
        );
        // 使用带有参数格式化的错误上下文
        return core::result_t<void>(core::error_context_t{code, "Failed to connect to database. Host is empty."});
    }
    std::cout << "✅ Database connected to " << host << std::endl;
    return core::result_t<void>(); // 成功
}

// ==========================================
// 2. 服务层：模拟初始化服务，捕获底层错误并包装
// ==========================================
core::result_t<void> init_service() {
    auto db_res = connect_database("");
    if (db_res.is_error()) {
        auto service_code = core::error_builder_t::make_error_code(
            core::error_level_t::fatal,
            domain::system_domain_t::service,
            traits::subsystem_traits<subsystem::service_subsystem_t>::to_int(subsystem::service_subsystem_t::auth),
            traits::module_traits<module::service_module_t>::to_int(module::service_module_t::auth_manager),
            5000 // Init Failure
        );
        
        // 包装底层错误（形成错误链）
        core::error_context_t service_context(service_code, "Service initialization aborted.");
        service_context = service_context.wrap(db_res.error());

        return core::result_t<void>(service_context);
    }
    return core::result_t<void>();
}

// ==========================================
// 3. 应用层：入口与多语言展示
// ==========================================
int main() {
    std::cout << "=========================================\n";
    std::cout << "🚀 Error System 演示项目启动\n";
    std::cout << "=========================================\n\n";

    // 实例化并注册全局翻译器
    auto translator = std::make_unique<i18n::json_translator_t>();
    i18n::translator_registry_t::instance().set(translator.get());

    std::cout << "▶️ 尝试初始化服务...\n";
    auto result = init_service();
    
    if (result.is_error()) {
        const auto& ctx = result.error();

        std::cout << "❌ 服务初始化失败！\n\n";

        std::cout << "-----------------------------------------\n";
        std::cout << "🌍 [English] 错误追踪与翻译\n";
        std::cout << "-----------------------------------------\n";
        translator->set_language(i18n::language_t::en_us); // 切换为英文
        std::cout << ctx.to_string() << "\n\n";

        std::cout << "-----------------------------------------\n";
        std::cout << "🌍 [中文] 错误追踪与翻译\n";
        std::cout << "-----------------------------------------\n";
        translator->set_language(i18n::language_t::zh_cn); // 切换为中文
        std::cout << ctx.to_string() << "\n\n";
    }

    return 0;
}

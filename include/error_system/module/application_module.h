#pragma once
#include <cstdint>

/**
 * @file application_module.h
 * @brief 应用层功能模块定义
 * @details 定义应用层相关的功能模块分类
 *          范围：0x0300 - 0x03FF（与应用层系统域 0x03 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 应用层功能模块分类
     * @details 定义应用层相关的功能模块，范围 0x0300 - 0x03FF
     */
    enum class application_module_t : uint16_t {
        none = 0x0300,             // 无模块
        controller = 0x0301,       // 控制器
        service = 0x0302,          // 业务服务
        repository = 0x0303,       // 数据仓库
        dto = 0x0304,              // 数据传输对象
        view = 0x0305,             // 视图层
        template_engine = 0x0306,  // 模板引擎
        form = 0x0307,             // 表单处理
        validator = 0x0308,        // 数据验证
        converter = 0x0309,        // 数据转换
        interceptor = 0x030A,      // 拦截器
        filter = 0x030B,           // 过滤器
        listener = 0x030C,         // 监听器
        resolver = 0x030D,         // 解析器
        router = 0x030E,           // 路由管理
        middleware = 0x030F,       // 中间件处理
        serializer = 0x0310,       // 序列化器
        deserializer = 0x0311,     // 反序列化器
        paginator = 0x0312,        // 分页器
        exporter = 0x0313,         // 数据导出器
        importer = 0x0314,         // 数据导入器
        renderer = 0x0315,         // 渲染器
        animator = 0x0316,         // 动画引擎
        physics = 0x0317,          // 物理引擎
        ai_model = 0x0318,         // AI模型推理
        shader = 0x0319,           // 着色器管理
    };

}  // namespace error_system::module

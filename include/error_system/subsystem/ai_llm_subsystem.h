#pragma once
#include <cstdint>

/**
 * @file ai_llm_subsystem.h
 * @brief 人工智能大模型子系统分类定义
 * @details 定义人工智能大模型相关的子系统分类
 *          范围：0x1000 - 0x10FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 人工智能大模型子系统分类
     * @details 定义人工智能大模型相关的子系统，范围 0x1000 - 0x10FF
     */
    enum class ai_llm_subsystem_t : uint16_t {
        none = 0x1000,             // 无子系统
        text_generation = 0x1001,  // 文本生成服务
        chat_completion = 0x1002,  // 对话补全服务
        embedding = 0x1003,        // 文本嵌入服务
        fine_tuning = 0x1004,      // 模型微调服务
        prompt_chaining = 0x1005,  // 提示链服务
        rag_pipeline = 0x1006,     // RAG流水线
        model_serving = 0x1007,    // 模型部署服务
        quantization = 0x1008,     // 模型量化服务
        distillation = 0x1009,     // 知识蒸馏服务
    };

}  // namespace error_system::subsystem

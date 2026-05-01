#pragma once
#include <cstdint>

/**
 * @file ai_module.h
 * @brief 人工智能功能模块定义
 * @details 定义人工智能相关的功能模块分类
 *          范围：0x0800 - 0x08FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 人工智能功能模块分类
     * @details 定义人工智能相关的功能模块，范围 0x0800 - 0x08FF
     */
    enum class ai_module_t : uint16_t {
        none = 0x0800,                // 无模块
        model_loader = 0x0801,        // 模型加载器
        model_trainer = 0x0802,       // 模型训练器
        inference_engine = 0x0803,    // 推理引擎
        tokenizer = 0x0804,           // 文本分词器
        embedder = 0x0805,            // 向量嵌入器
        vector_search = 0x0806,       // 向量搜索器
        prompt_engineer = 0x0807,     // 提示工程器
        fine_tuner = 0x0808,          // 模型微调器
        data_labeler = 0x0809,        // 数据标注器
        feature_extractor = 0x080A,   // 特征提取器
        preprocessor = 0x080B,        // 数据预处理器
        postprocessor = 0x080C,       // 数据后处理器
        batch_predictor = 0x080D,     // 批量预测器
        realtime_predictor = 0x080E,  // 实时预测器
        model_validator = 0x080F,     // 模型验证器
        drift_detector = 0x0810,      // 漂移检测器
        explainability = 0x0811,      // 可解释性模块
    };

}  // namespace error_system::module

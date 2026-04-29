#pragma once
#include <cstdint>

/**
 * @file ai_module.h
 * @brief 人工智能层功能模块定义
 * @details 定义人工智能层相关的功能模块分类
 *          范围：0x0900 - 0x09FF（与人工智能层系统域 0x09 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 人工智能层功能模块分类
     * @details 定义人工智能层相关的功能模块，范围 0x0900 - 0x09FF
     */
    enum class ai_module_t : uint16_t {
        none = 0x0900,                // 无模块
        model_loader = 0x0901,        // 模型加载器
        model_trainer = 0x0902,       // 模型训练器
        inference_engine = 0x0903,    // 推理引擎
        tokenizer = 0x0904,           // 分词器
        embedder = 0x0905,            // 嵌入器
        vector_search = 0x0906,       // 向量搜索
        prompt_engineer = 0x0907,     // 提示工程
        fine_tuner = 0x0908,          // 微调器
        data_labeler = 0x0909,        // 数据标注器
        feature_extractor = 0x090A,   // 特征提取器
        preprocessor = 0x090B,        // 预处理器
        postprocessor = 0x090C,       // 后处理器
        batch_predictor = 0x090D,     // 批量预测器
        realtime_predictor = 0x090E,  // 实时预测器
        model_validator = 0x090F,     // 模型验证器
        drift_detector = 0x0910,      // 漂移检测器
        explainability = 0x0911,      // 可解释性
    };

}  // namespace error_system::module

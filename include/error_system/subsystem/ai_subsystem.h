#pragma once
#include <cstdint>

/**
 * @file ai_subsystem.h
 * @brief 人工智能层子系统分类定义
 * @details 定义人工智能层相关的子系统分类，用于标识错误码所属的人工智能层子系统
 *          范围：0x0900 - 0x09FF（与人工智能层系统域 0x09 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 人工智能层子系统分类
     * @details 定义人工智能层相关的子系统，范围 0x0900 - 0x09FF
     */
    enum class ai_subsystem_t : uint16_t {
        none = 0x0900,                // 无子系统
        llm = 0x0901,                 // 大语言模型
        computer_vision = 0x0902,     // 计算机视觉
        speech_recognition = 0x0903,  // 语音识别
        nlp = 0x0904,                 // 自然语言处理
        recommendation = 0x0905,      // 智能推荐
        anomaly_detection = 0x0906,   // 异常检测
        ocr = 0x0907,                 // 光学字符识别
        face_recognition = 0x0908,    // 人脸识别
        automl = 0x0909,              // 自动机器学习
        mlpipeline = 0x090A,          // 机器学习流水线
        model_serving = 0x090B,       // 模型服务
        feature_store = 0x090C,       // 特征存储
        vector_db = 0x090D,           // 向量数据库
        rag = 0x090E,                 // 检索增强生成
        agent = 0x090F,               // 智能体
        multimodal = 0x0910,          // 多模态模型
    };

}  // namespace error_system::subsystem

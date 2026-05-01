#pragma once
#include <cstdint>

/**
 * @file ai_vision_subsystem.h
 * @brief 人工智能视觉子系统分类定义
 * @details 定义人工智能视觉相关的子系统分类
 *          范围：0x1100 - 0x11FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 人工智能视觉子系统分类
     * @details 定义人工智能视觉相关的子系统，范围 0x1100 - 0x11FF
     */
    enum class ai_vision_subsystem_t : uint16_t {
        none = 0x1100,              // 无子系统
        image_classifier = 0x1101,  // 图像分类器
        object_detector = 0x1102,   // 目标检测器
        segmenter = 0x1103,         // 图像分割器
        ocr_engine = 0x1104,        // OCR识别引擎
        face_recognizer = 0x1105,   // 人脸识别器
        pose_estimator = 0x1106,    // 姿态估计器
        video_analyzer = 0x1107,    // 视频分析器
        image_generator = 0x1108,   // 图像生成器
    };

}  // namespace error_system::subsystem

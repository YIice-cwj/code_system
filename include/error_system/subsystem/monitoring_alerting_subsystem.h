#pragma once
#include <cstdint>

/**
 * @file monitoring_alerting_subsystem.h
 * @brief 监控告警子系统分类定义
 * @details 定义监控告警相关的子系统分类
 *          范围：0x2100 - 0x21FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 监控告警子系统分类
     * @details 定义监控告警相关的子系统，范围 0x2100 - 0x21FF
     */
    enum class monitoring_alerting_subsystem_t : uint16_t {
        none = 0x2100,          // 无子系统
        alertmanager = 0x2101,  // Alertmanager告警
        pagerduty = 0x2102,     // PagerDuty告警
        opsgenie = 0x2103,      // OpsGenie告警
        victorops = 0x2104,     // VictorOps告警
        sensu = 0x2105,         // Sensu监控告警
        icinga = 0x2106,        // Icinga监控告警
        zabbix = 0x2107,        // Zabbix监控告警
        nagios = 0x2108,        // Nagios监控告警
    };

}  // namespace error_system::subsystem

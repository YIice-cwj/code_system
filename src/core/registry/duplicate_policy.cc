#include "error_system/core/registry/duplicate_policy.h"

#include "error_system/core/registry/error_registry.h"

namespace error_system::core {

    /**
     * @brief 处理 warn 策略
     * @details 本函数内部加锁拷贝 callback，释放锁后调用回调
     * @param raw_code 错误码原始值
     * @param existing 已存在的元数据
     * @return bool 是否继续注册流程（warn 返回 false）
     */
    bool duplicate_policy_handler_t::handle_duplicate_warn_(code_t raw_code,
                                                            const error_metadata_t& existing) const noexcept {
        duplicate_warn_callback_t callback_copy;
        try {
            {
                std::shared_lock<std::shared_mutex> lock(mutex_);
                callback_copy = warn_callback_;
            }
            if (callback_copy) {
                callback_copy(raw_code, existing);
            }
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[duplicate_policy] warn callback: std::bad_alloc\n");
        }
        return false;
    }

    /**
     * @brief 根据当前策略处理重复错误码
     * @details warn 策略下：锁内拷贝 callback、读取策略，释放锁后再调用回调
     * @param raw_code 错误码原始值
     * @param existing 已存在的元数据指针（无重复时为 nullptr）
     * @return bool 是否继续注册流程（true=继续，调用方需先擦除旧条目；false=中止）
     */
    bool duplicate_policy_handler_t::apply_duplicate_policy(code_t raw_code,
                                                             const error_metadata_t* existing) noexcept {
        if (existing == nullptr) {
            return true;
        }
        duplicate_policy_t policy_snapshot;
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            policy_snapshot = policy_;
        }
        switch (policy_snapshot) {
            case duplicate_policy_t::skip:
                return handle_duplicate_skip_(raw_code);
            case duplicate_policy_t::overwrite:
                return handle_duplicate_overwrite_(raw_code);
            case duplicate_policy_t::warn:
                return handle_duplicate_warn_(raw_code, *existing);
        }
        return false;
    }

}  // namespace error_system::core

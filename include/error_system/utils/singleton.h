#pragma once
#include <cassert>
#include <mutex>

/**
 * @file singleton.h
 * @brief CRTP 单例基类
 * @details 基于 std::call_once + 函数局部静态保证线程安全的单例初始化，
 *          符合规范 22「单例模式必须使用 std::call_once + std::once_flag」。
 *          子类需满足：
 *          - 以 `public singleton_t<Derived>` 方式继承
 *          - 私有默认构造函数，并在 private 区声明 `friend class singleton_t<Derived>;`
 *          - 无需再声明 once_flag_、instance() 及拷贝/移动删除（均由本基类提供）
 * @author yiice
 * @version 4.4.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief CRTP 单例基类
     * @details 子类通过 CRTP 继承获得线程安全的 instance() 实现。
     *          拷贝与移动语义被显式删除，保证单例唯一性。
     *          instance() 内部使用 assert 防御性校验指针非空（call_once 保证成立）。
     * @tparam Derived 实际单例类型
     */
    template <typename Derived>
    class singleton_t {
    public:
        /**
         * @brief 获取单例实例
         * @details 首次调用通过 std::call_once 初始化函数局部静态实例，
         *          后续调用直接返回已初始化指针，线程安全。
         * @return Derived 单例引用
         */
        static Derived& instance() noexcept {
            static Derived* instance_ptr = nullptr;
            static std::once_flag once;
            std::call_once(once, [] {
                static Derived instance;
                instance_ptr = &instance;
            });
            assert(instance_ptr != nullptr);
            return *instance_ptr;
        }

        singleton_t(const singleton_t&) = delete;
        singleton_t& operator=(const singleton_t&) = delete;
        singleton_t(singleton_t&&) = delete;
        singleton_t& operator=(singleton_t&&) = delete;

    protected:
        singleton_t() noexcept = default;
        ~singleton_t() noexcept = default;
    };

}  // namespace error_system::utils

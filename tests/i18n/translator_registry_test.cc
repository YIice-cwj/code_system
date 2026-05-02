#include "error_system/i18n/json_translator.h"
#include "error_system/i18n/language.h"
#include "error_system/i18n/translator_registry.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace error_system::i18n {

    class translator_registry_test : public ::testing::Test {
        protected:
        void SetUp() override { translator_registry_t::instance().set(nullptr); }

        void TearDown() override { translator_registry_t::instance().set(nullptr); }
    };

    TEST_F(translator_registry_test, set_and_get_single_thread) {
        auto& registry = translator_registry_t::instance();
        json_translator_t translator(language_t::en_us);

        registry.set(&translator);
        EXPECT_EQ(registry.get(), &translator);
        EXPECT_TRUE(registry.has_translator());
    }

    TEST_F(translator_registry_test, get_returns_default_when_not_set) {
        auto& registry = translator_registry_t::instance();
        registry.set(nullptr);

        auto* translator = registry.get();
        EXPECT_NE(translator, nullptr);
        EXPECT_EQ(translator->get_language(), language_t::zh_cn);
    }

    TEST_F(translator_registry_test, concurrent_set_and_get) {
        auto& registry = translator_registry_t::instance();
        json_translator_t translator_a(language_t::zh_cn);
        json_translator_t translator_b(language_t::en_us);

        constexpr int thread_count = 8;
        constexpr int iterations = 500;

        std::atomic<size_t> a_count{0};
        std::atomic<size_t> b_count{0};
        std::atomic<size_t> default_count{0};

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < iterations; ++j) {
                    if (i % 2 == 0) {
                        registry.set(&translator_a);
                    } else {
                        registry.set(&translator_b);
                    }

                    auto* current = registry.get();
                    if (current == &translator_a) {
                        ++a_count;
                    } else if (current == &translator_b) {
                        ++b_count;
                    } else {
                        ++default_count;
                    }
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        EXPECT_EQ(a_count.load() + b_count.load() + default_count.load(),
                  static_cast<size_t>(thread_count * iterations));
    }

    TEST_F(translator_registry_test, concurrent_get_without_set) {
        auto& registry = translator_registry_t::instance();
        registry.set(nullptr);

        constexpr int thread_count = 8;
        constexpr int iterations = 200;

        std::atomic<size_t> success_count{0};

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < iterations; ++j) {
                    auto* translator = registry.get();
                    if (translator != nullptr) {
                        ++success_count;
                    }
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        EXPECT_EQ(success_count.load(), static_cast<size_t>(thread_count * iterations));
    }

    TEST_F(translator_registry_test, has_translator_is_thread_safe) {
        auto& registry = translator_registry_t::instance();
        json_translator_t translator(language_t::zh_cn);

        constexpr int thread_count = 8;
        constexpr int iterations = 500;

        std::atomic<size_t> has_count{0};
        std::atomic<size_t> no_count{0};

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < iterations; ++j) {
                    if (j % 2 == 0) {
                        registry.set(i % 2 == 0 ? &translator : nullptr);
                    }

                    if (registry.has_translator()) {
                        ++has_count;
                    } else {
                        ++no_count;
                    }
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        EXPECT_EQ(has_count.load() + no_count.load(), static_cast<size_t>(thread_count * iterations));
    }

}  // namespace error_system::i18n

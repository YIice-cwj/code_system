#include "error_system/utils/stack_trace_utils.h"

#include <string>

#include <gtest/gtest.h>

namespace error_system::utils {

    class stack_trace_utils_test_t : public ::testing::Test {};

    /**
     * @brief 辅助函数：用于验证栈帧中包含当前函数名
     * @note 禁止内联：Release 模式下内联会让该函数从栈帧中消失，导致 Windows 上测试失败
     */
#if defined(_MSC_VER)
    __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    __attribute__((noinline))
#endif
    static void helper_for_stacktrace(std::vector<std::string>& output) {
        output = stack_trace_utils_t::generate(0);
    }

    TEST_F(stack_trace_utils_test_t, generate_returns_non_empty) {
        auto trace = stack_trace_utils_t::generate(0);
        EXPECT_FALSE(trace.empty());
    }

    TEST_F(stack_trace_utils_test_t, generate_with_skip_reduces_frames) {
        auto trace_no_skip = stack_trace_utils_t::generate(0, 64);
        auto trace_skip_1 = stack_trace_utils_t::generate(1, 64);

        ASSERT_FALSE(trace_no_skip.empty());
        ASSERT_FALSE(trace_skip_1.empty());
        // skip=1 应比 skip=0 少一帧
        EXPECT_EQ(trace_skip_1.size() + 1, trace_no_skip.size());
    }

    TEST_F(stack_trace_utils_test_t, generate_with_zero_max_returns_empty) {
        auto trace = stack_trace_utils_t::generate(1, 0);
        EXPECT_TRUE(trace.empty());
    }

    TEST_F(stack_trace_utils_test_t, generate_contains_current_function) {
        std::vector<std::string> trace;
        helper_for_stacktrace(trace);

        ASSERT_FALSE(trace.empty());
        // 检查符号是否可解析（stripped 二进制下可能只有地址）
        bool has_symbol = false;
        for (const auto& frame : trace) {
            if (frame.find("0x") == std::string::npos || frame.find("helper") != std::string::npos
                || frame.find("stack_trace") != std::string::npos) {
                has_symbol = true;
                break;
            }
        }
        if (!has_symbol) {
            GTEST_SKIP() << "符号不可解析（stripped 二进制），跳过符号验证";
        }
        bool found = false;
        for (const auto& frame : trace) {
            if (frame.find("helper_for_stacktrace") != std::string::npos) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "栈帧中应包含 helper_for_stacktrace 函数名";
    }

    TEST_F(stack_trace_utils_test_t, generate_contains_test_function) {
        auto trace = stack_trace_utils_t::generate(0, 32);

        ASSERT_FALSE(trace.empty());
        // 检查符号是否可解析（stripped 二进制下可能只有地址）
        bool has_symbol = false;
        for (const auto& frame : trace) {
            if (frame.find("0x") == std::string::npos || frame.find("helper") != std::string::npos
                || frame.find("stack_trace") != std::string::npos) {
                has_symbol = true;
                break;
            }
        }
        if (!has_symbol) {
            GTEST_SKIP() << "符号不可解析（stripped 二进制），跳过符号验证";
        }
        bool found_test = false;
        for (const auto& frame : trace) {
            if (frame.find("stack_trace_utils_test") != std::string::npos ||
                frame.find("generate_contains_test_function") != std::string::npos) {
                found_test = true;
                break;
            }
        }
        EXPECT_TRUE(found_test) << "栈帧中应包含测试函数相关信息";
    }

    TEST_F(stack_trace_utils_test_t, generate_frames_are_readable) {
        auto trace = stack_trace_utils_t::generate(0, 8);
        ASSERT_FALSE(trace.empty()) << "应至少捕获一帧";
        for (const auto& frame : trace) {
            EXPECT_FALSE(frame.empty());
        }
    }

    TEST_F(stack_trace_utils_test_t, generate_respects_max_frames) {
        constexpr int max_frames = 4;
        auto trace = stack_trace_utils_t::generate(0, max_frames);
        EXPECT_LE(static_cast<int>(trace.size()), max_frames);
    }

    TEST_F(stack_trace_utils_test_t, generate_skip_more_than_available_returns_empty) {
        auto trace = stack_trace_utils_t::generate(1000, 64);
        EXPECT_TRUE(trace.empty());
    }

}  // namespace error_system::utils

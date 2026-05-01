#include "error_system/traits/subsystem_dispatcher.h"
#include "error_system/traits/subsystem_traits.h"

#include <gtest/gtest.h>

namespace error_system::traits {

    class subsystem_traits_test : public ::testing::Test {};

    TEST_F(subsystem_traits_test, ai_llm_to_int) {
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::to_int(subsystem::ai_llm_subsystem_t::none), 0x1000);
        EXPECT_EQ(
            subsystem_traits<subsystem::ai_llm_subsystem_t>::to_int(subsystem::ai_llm_subsystem_t::text_generation),
            0x1001);
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::to_int(subsystem::ai_llm_subsystem_t::distillation),
                  0x1009);
    }

    TEST_F(subsystem_traits_test, ai_llm_from_int_valid) {
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_int(0x1000),
                  subsystem::ai_llm_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_int(0x1005),
                  subsystem::ai_llm_subsystem_t::prompt_chaining);
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_int(0x1009),
                  subsystem::ai_llm_subsystem_t::distillation);
    }

    TEST_F(subsystem_traits_test, ai_llm_from_int_invalid_returns_none) {
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_int(0x0FFF),
                  subsystem::ai_llm_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_int(0x1100),
                  subsystem::ai_llm_subsystem_t::none);
    }

    TEST_F(subsystem_traits_test, ai_llm_is_valid) {
        EXPECT_TRUE(subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(0x1000));
        EXPECT_TRUE(subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(0x1007));
        EXPECT_TRUE(subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(0x1009));
        EXPECT_FALSE(subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(0x0FFF));
        EXPECT_FALSE(subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(0x100A));
    }

    TEST_F(subsystem_traits_test, ai_llm_to_string) {
        EXPECT_STREQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::to_string(subsystem::ai_llm_subsystem_t::none),
                     "none");
        EXPECT_STREQ(
            subsystem_traits<subsystem::ai_llm_subsystem_t>::to_string(subsystem::ai_llm_subsystem_t::rag_pipeline),
            "rag_pipeline");
        EXPECT_STREQ(
            subsystem_traits<subsystem::ai_llm_subsystem_t>::to_string(subsystem::ai_llm_subsystem_t::quantization),
            "quantization");
    }

    TEST_F(subsystem_traits_test, ai_llm_from_string) {
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_string("none"),
                  subsystem::ai_llm_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_string("chat_completion"),
                  subsystem::ai_llm_subsystem_t::chat_completion);
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_string("model_serving"),
                  subsystem::ai_llm_subsystem_t::model_serving);
    }

    TEST_F(subsystem_traits_test, ai_llm_from_string_invalid_returns_none) {
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_string("unknown"),
                  subsystem::ai_llm_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::ai_llm_subsystem_t>::from_string(""),
                  subsystem::ai_llm_subsystem_t::none);
    }

    TEST_F(subsystem_traits_test, kernel_cpu_roundtrip) {
        auto original = subsystem::kernel_cpu_subsystem_t::thermal_manager;
        auto int_val = subsystem_traits<subsystem::kernel_cpu_subsystem_t>::to_int(original);
        auto recovered = subsystem_traits<subsystem::kernel_cpu_subsystem_t>::from_int(int_val);
        EXPECT_EQ(original, recovered);

        auto str = subsystem_traits<subsystem::kernel_cpu_subsystem_t>::to_string(original);
        auto from_str = subsystem_traits<subsystem::kernel_cpu_subsystem_t>::from_string(str);
        EXPECT_EQ(original, from_str);
    }

    TEST_F(subsystem_traits_test, resolve_subsystem_dispatcher) {
        EXPECT_STREQ(resolve_subsystem(0x1006), "rag_pipeline");
        EXPECT_STREQ(resolve_subsystem(0x0305), "thermal_manager");
        EXPECT_STREQ(resolve_subsystem(0xFFFF), "none");
    }

    TEST_F(subsystem_traits_test, subsystem_from_string_dispatcher) {
        EXPECT_EQ(subsystem_from_string("ai_llm", "embedding"), 0x1003);
        EXPECT_EQ(subsystem_from_string("kernel_cpu", "scheduler"), 0x0301);
        EXPECT_EQ(subsystem_from_string("invalid_type", "any"), 0);
    }

}  // namespace error_system::traits

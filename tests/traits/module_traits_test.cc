#include "error_system/traits/module_dispatcher.h"
#include "error_system/traits/module_traits.h"

#include <gtest/gtest.h>

namespace error_system::traits {

    class module_traits_test : public ::testing::Test {};

    TEST_F(module_traits_test, ai_module_to_int) {
        EXPECT_EQ(module_traits<module::ai_module_t>::to_int(module::ai_module_t::none), 0x0800);
        EXPECT_EQ(module_traits<module::ai_module_t>::to_int(module::ai_module_t::model_loader), 0x0801);
        EXPECT_EQ(module_traits<module::ai_module_t>::to_int(module::ai_module_t::explainability), 0x0811);
    }

    TEST_F(module_traits_test, ai_module_from_int_valid) {
        EXPECT_EQ(module_traits<module::ai_module_t>::from_int(0x0800), module::ai_module_t::none);
        EXPECT_EQ(module_traits<module::ai_module_t>::from_int(0x0805), module::ai_module_t::embedder);
        EXPECT_EQ(module_traits<module::ai_module_t>::from_int(0x0811), module::ai_module_t::explainability);
    }

    TEST_F(module_traits_test, ai_module_from_int_invalid_returns_none) {
        EXPECT_EQ(module_traits<module::ai_module_t>::from_int(0x0700), module::ai_module_t::none);
        EXPECT_EQ(module_traits<module::ai_module_t>::from_int(0x0900), module::ai_module_t::none);
    }

    TEST_F(module_traits_test, ai_module_is_valid) {
        EXPECT_TRUE(module_traits<module::ai_module_t>::is_valid(0x0800));
        EXPECT_TRUE(module_traits<module::ai_module_t>::is_valid(0x080A));
        EXPECT_TRUE(module_traits<module::ai_module_t>::is_valid(0x0811));
        EXPECT_FALSE(module_traits<module::ai_module_t>::is_valid(0x07FF));
        EXPECT_FALSE(module_traits<module::ai_module_t>::is_valid(0x0812));
    }

    TEST_F(module_traits_test, ai_module_to_string) {
        EXPECT_STREQ(module_traits<module::ai_module_t>::to_string(module::ai_module_t::none), "none");
        EXPECT_STREQ(module_traits<module::ai_module_t>::to_string(module::ai_module_t::tokenizer), "tokenizer");
        EXPECT_STREQ(module_traits<module::ai_module_t>::to_string(module::ai_module_t::explainability),
                     "explainability");
    }

    TEST_F(module_traits_test, ai_module_from_string) {
        EXPECT_EQ(module_traits<module::ai_module_t>::from_string("none"), module::ai_module_t::none);
        EXPECT_EQ(module_traits<module::ai_module_t>::from_string("inference_engine"),
                  module::ai_module_t::inference_engine);
        EXPECT_EQ(module_traits<module::ai_module_t>::from_string("vector_search"), module::ai_module_t::vector_search);
    }

    TEST_F(module_traits_test, ai_module_from_string_invalid_returns_none) {
        EXPECT_EQ(module_traits<module::ai_module_t>::from_string("invalid_name"), module::ai_module_t::none);
        EXPECT_EQ(module_traits<module::ai_module_t>::from_string(""), module::ai_module_t::none);
    }

    TEST_F(module_traits_test, common_module_roundtrip) {
        auto original = module::common_module_t::balancer;
        auto int_val = module_traits<module::common_module_t>::to_int(original);
        auto recovered = module_traits<module::common_module_t>::from_int(int_val);
        EXPECT_EQ(original, recovered);

        auto str = module_traits<module::common_module_t>::to_string(original);
        auto from_str = module_traits<module::common_module_t>::from_string(str);
        EXPECT_EQ(original, from_str);
    }

    TEST_F(module_traits_test, resolve_module_dispatcher) {
        EXPECT_STREQ(resolve_module(0x0803), "inference_engine");
        EXPECT_STREQ(resolve_module(0x0B0C), "interceptor");
        EXPECT_STREQ(resolve_module(0xFFFF), "none");
    }

    TEST_F(module_traits_test, module_from_string_dispatcher) {
        EXPECT_EQ(module_from_string("ai", "fine_tuner"), 0x0808);
        EXPECT_EQ(module_from_string("invalid_type", "any"), 0);
    }

}  // namespace error_system::traits

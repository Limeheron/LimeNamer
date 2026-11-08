#include "text_service_test.h"

TEST_F(TEXT_SERVICE_TEST, to_nfc_composes_characters)
{
    EXPECT_EQ(service_->to_nfc(U""), U"");
    EXPECT_EQ(service_->to_nfc(U"abc"), U"abc");
    EXPECT_EQ(service_->to_nfc(U"é"), U"é");
    EXPECT_EQ(service_->to_nfc(U"Å"), U"Å");
    EXPECT_EQ(service_->to_nfc(U"Å"), U"Å");
    EXPECT_EQ(service_->to_nfc(U"한글"), U"한글");
    EXPECT_EQ(service_->to_nfc(U"한글 2024.txt"), U"한글 2024.txt");
    EXPECT_EQ(service_->to_nfc(U"\U0001F600"), U"\U0001F600");
}

TEST_F(TEXT_SERVICE_TEST, to_upper_maps_common_scripts)
{
    EXPECT_EQ(service_->to_upper(U""), U"");
    EXPECT_EQ(service_->to_upper(U"Hello World"), U"HELLO WORLD");
    EXPECT_EQ(service_->to_upper(U"éàüñ"), U"ÉÀÜÑ");
    EXPECT_EQ(service_->to_upper(U"αω"), U"ΑΩ");
    EXPECT_EQ(service_->to_upper(U"жя"), U"ЖЯ");
    EXPECT_EQ(service_->to_upper(U"한글 123 _-.()"), U"한글 123 _-.()");
}

TEST_F(TEXT_SERVICE_TEST, to_lower_maps_common_scripts)
{
    EXPECT_EQ(service_->to_lower(U""), U"");
    EXPECT_EQ(service_->to_lower(U"Hello World"), U"hello world");
    EXPECT_EQ(service_->to_lower(U"ÉÀÜÑ"), U"éàüñ");
    EXPECT_EQ(service_->to_lower(U"ΑΩ"), U"αω");
    EXPECT_EQ(service_->to_lower(U"ЖЯ"), U"жя");
    EXPECT_EQ(service_->to_lower(U"한글 123 _-.()"), U"한글 123 _-.()");
}

TEST_F(TEXT_SERVICE_TEST, fold_key_ignores_case_and_normalization)
{
    EXPECT_EQ(service_->fold_key(U""), U"");
    EXPECT_EQ(service_->fold_key(U"ABC"), service_->fold_key(U"abc"));
    EXPECT_EQ(service_->fold_key(U"Photo.JPG"), service_->fold_key(U"photo.jpg"));
    EXPECT_EQ(service_->fold_key(U"É"), service_->fold_key(U"é"));
    EXPECT_EQ(service_->fold_key(U"É"), service_->fold_key(U"é"));
    EXPECT_EQ(service_->fold_key(U"ΑΩ"), service_->fold_key(U"αω"));
    EXPECT_EQ(service_->fold_key(U"한글"),
        service_->fold_key(U"한글"));
    EXPECT_NE(service_->fold_key(U"a"), service_->fold_key(U"b"));
    EXPECT_NE(service_->fold_key(U"a.txt"), service_->fold_key(U"a.txt "));
    EXPECT_NE(service_->fold_key(U"한"), service_->fold_key(U"글"));
}

TEST_F(TEXT_SERVICE_TEST, is_letter_or_digit_classifies_characters)
{
    const char32_t letters_and_digits[] = {
        U'a', U'Z', U'0', U'9', U'é', U'ω', U'ж', U'한', U'ㄱ',
        U'あ', U'漢'};
    const char32_t others[] = {
        U' ', U'_', U'-', U'.', U',', U'(', U')', U'[', U']', U'\'', U'!', U'@', U'　',
        U'\U0001F600'};
    for (const char32_t c : letters_and_digits) {
        EXPECT_TRUE(service_->is_letter_or_digit(c)) << "U+" << std::hex << uint32_t{c};
    }
    for (const char32_t c : others) {
        EXPECT_FALSE(service_->is_letter_or_digit(c)) << "U+" << std::hex << uint32_t{c};
    }
}

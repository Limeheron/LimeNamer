#include "mac_text_service.h"

static_assert(std::endian::native == std::endian::little);

std::u32string MAC_TEXT_SERVICE::to_nfc(std::u32string_view text) const
{
    if (text.empty()) {
        return {};
    }
    CF_STRING value = to_cf(text);
    CFStringNormalize(value.get(), kCFStringNormalizationFormC);
    return from_cf(value.get());
}

std::u32string MAC_TEXT_SERVICE::to_upper(std::u32string_view text) const
{
    if (text.empty()) {
        return {};
    }
    CF_STRING value = to_cf(text);
    CFStringUppercase(value.get(), nullptr);
    return from_cf(value.get());
}

std::u32string MAC_TEXT_SERVICE::to_lower(std::u32string_view text) const
{
    if (text.empty()) {
        return {};
    }
    CF_STRING value = to_cf(text);
    CFStringLowercase(value.get(), nullptr);
    return from_cf(value.get());
}

std::u32string MAC_TEXT_SERVICE::fold_key(std::u32string_view text) const
{
    if (text.empty()) {
        return {};
    }
    CF_STRING value = to_cf(text);
    CFStringNormalize(value.get(), kCFStringNormalizationFormC);
    CFStringFold(value.get(), kCFCompareCaseInsensitive, nullptr);
    return from_cf(value.get());
}

bool MAC_TEXT_SERVICE::is_letter_or_digit(char32_t c) const
{
    CFCharacterSetRef set = CFCharacterSetGetPredefined(kCFCharacterSetAlphaNumeric);
    return CFCharacterSetIsLongCharacterMember(set, static_cast<UTF32Char>(c));
}

MAC_TEXT_SERVICE::CF_STRING MAC_TEXT_SERVICE::to_cf(std::u32string_view text)
{
    CFStringRef source = CFStringCreateWithBytes(kCFAllocatorDefault,
        reinterpret_cast<const UInt8 *>(text.data()),
        static_cast<CFIndex>(text.size() * sizeof(char32_t)), kCFStringEncodingUTF32LE, false);
    if (source == nullptr) {
        throw std::runtime_error("CFStringCreateWithBytes failed");
    }
    CFMutableStringRef copy = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, source);
    CFRelease(source);
    if (copy == nullptr) {
        throw std::runtime_error("CFStringCreateMutableCopy failed");
    }
    return CF_STRING(copy, &CFRelease);
}

std::u32string MAC_TEXT_SERVICE::from_cf(CFStringRef text)
{
    const CFRange range = CFRangeMake(0, CFStringGetLength(text));
    CFIndex size = 0;
    CFStringGetBytes(text, range, kCFStringEncodingUTF32LE, 0, false, nullptr, 0, &size);
    std::u32string result(static_cast<size_t>(size) / sizeof(char32_t), U'\0');
    CFIndex used = 0;
    const CFIndex converted = CFStringGetBytes(text, range, kCFStringEncodingUTF32LE, 0, false,
        reinterpret_cast<UInt8 *>(result.data()), size, &used);
    if (converted != range.length || used != size) {
        throw std::runtime_error("CFStringGetBytes failed");
    }
    return result;
}

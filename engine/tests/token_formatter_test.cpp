#include "token_formatter_test.h"

namespace
{
NUMBER_SETTINGS number_settings(int64_t start, int64_t step, uint32_t digits)
{
    NUMBER_SETTINGS settings;
    settings.start = start;
    settings.step = step;
    settings.digits = digits;
    return settings;
}

SIZE_SETTINGS size_settings(SIZE_UNIT unit)
{
    SIZE_SETTINGS settings;
    settings.unit = unit;
    return settings;
}

DATE_PARTS date_parts(int year, int month, int day, int hour, int minute, int second)
{
    DATE_PARTS parts;
    parts.year = year;
    parts.month = month;
    parts.day = day;
    parts.hour = hour;
    parts.minute = minute;
    parts.second = second;
    return parts;
}
}

TEST(TOKEN_FORMATTER, numbers_items_in_order)
{
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(1, 1, 1), 0), U"1");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(1, 1, 1), 9), U"10");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(1, 1, 3), 0), U"001");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(1, 1, 3), 99), U"100");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(1, 1, 3), 999), U"1000");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(10, 5, 1), 2), U"20");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(10, -1, 2), 3), U"07");
}

TEST(TOKEN_FORMATTER, numbers_negative_values)
{
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(0, -1, 3), 3), U"-003");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(-5, 2, 1), 0), U"-5");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(-5, 2, 1), 3), U"1");
}

TEST(TOKEN_FORMATTER, handles_the_largest_numbers)
{
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(1000000000, 1000000000, 1), 1000000000),
        U"1000000001000000000");
    EXPECT_EQ(TOKEN_FORMATTER::number(number_settings(-1000000000, -1000000000, 1), 1000000000),
        U"-1000000001000000000");
    EXPECT_THROW(TOKEN_FORMATTER::number(number_settings(1, 1, 1), 1000000001),
        std::out_of_range);
}

TEST(TOKEN_FORMATTER, formats_dates)
{
    const DATE_PARTS parts = date_parts(2024, 12, 31, 23, 59, 58);
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyyyMMdd", parts), U"20241231");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyyy-MM-dd", parts), U"2024-12-31");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyyy_MM_dd", parts), U"2024_12_31");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyMMdd", parts), U"241231");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyyyMMdd_HHmmss", parts), U"20241231_235958");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyyy년 MM월", parts),
        U"2024년 12월");
}

TEST(TOKEN_FORMATTER, pads_and_keeps_literal_date_text)
{
    const DATE_PARTS parts = date_parts(2009, 1, 5, 7, 8, 9);
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyyy-MM-dd HH.mm.ss", parts),
        U"2009-01-05 07.08.09");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyMMdd", parts), U"090105");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"y", parts), U"y");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"yyy", parts), U"09y");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"Date", parts), U"Date");
    EXPECT_EQ(TOKEN_FORMATTER::format_date(U"", parts), U"");
}

TEST(TOKEN_FORMATTER, converts_to_local_time)
{
    const int64_t ms = 1735689599123;
    const std::optional<DATE_PARTS> parts = TOKEN_FORMATTER::local_parts(ms);
    ASSERT_TRUE(parts.has_value());
    std::tm local {};
    local.tm_year = parts->year - 1900;
    local.tm_mon = parts->month - 1;
    local.tm_mday = parts->day;
    local.tm_hour = parts->hour;
    local.tm_min = parts->minute;
    local.tm_sec = parts->second;
    local.tm_isdst = -1;
    EXPECT_EQ(static_cast<int64_t>(std::mktime(&local)), ms / 1000);
}

TEST(TOKEN_FORMATTER, rounds_milliseconds_down)
{
    EXPECT_EQ(TOKEN_FORMATTER::local_parts(999), TOKEN_FORMATTER::local_parts(0));
    EXPECT_EQ(TOKEN_FORMATTER::local_parts(-1), TOKEN_FORMATTER::local_parts(-1000));
}

TEST(TOKEN_FORMATTER, formats_sizes_in_fixed_units)
{
    EXPECT_EQ(TOKEN_FORMATTER::size(size_settings(SIZE_UNIT::bytes), 2048, false), U"2048B");
    EXPECT_EQ(TOKEN_FORMATTER::size(size_settings(SIZE_UNIT::kb), 2048, false), U"2KB");
    EXPECT_EQ(TOKEN_FORMATTER::size(size_settings(SIZE_UNIT::kb), 1536, false), U"2KB");
    EXPECT_EQ(TOKEN_FORMATTER::size(size_settings(SIZE_UNIT::kb), 1535, false), U"1KB");
    EXPECT_EQ(TOKEN_FORMATTER::size(size_settings(SIZE_UNIT::kb), 100, false), U"0KB");
    EXPECT_EQ(TOKEN_FORMATTER::size(size_settings(SIZE_UNIT::mb), 3145728, false), U"3MB");
    EXPECT_EQ(TOKEN_FORMATTER::size(size_settings(SIZE_UNIT::gb), 5368709120, false), U"5GB");
}

TEST(TOKEN_FORMATTER, chooses_the_size_unit)
{
    const SIZE_SETTINGS automatic = size_settings(SIZE_UNIT::automatic);
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, 0, false), U"0B");
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, 500, false), U"500B");
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, 1024, false), U"1KB");
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, 1536000, false), U"1MB");
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, 1048575, false), U"1MB");
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, 10737418240, false), U"10GB");
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, 2048, true), U"");
    EXPECT_EQ(TOKEN_FORMATTER::size(automatic, -1, false), U"");
}

TEST(TOKEN_FORMATTER, builds_the_value)
{
    const MODIFICATION mod = MOD_JSON::parse(R"({"kind":"replace","position":{"type":"all"},
        "value":[{"text":"IMG_"},{"token":"number"},{"text":"_"},{"token":"original_name"},
            {"text":"_"},{"token":"parent_folder"},{"text":"_"},{"token":"size"}],
        "tokens":{"number":{"start":1,"step":1,"digits":3},"size":{"unit":"kb"}}})");
    TOKEN_CONTEXT context;
    context.index = 4;
    context.original_name = U"photo.final.jpg";
    context.parent_name = U"여행";
    context.size = 3072;
    EXPECT_EQ(TOKEN_FORMATTER::build(mod, context), U"IMG_005_photo.final_여행_3KB");
}

TEST(TOKEN_FORMATTER, builds_folder_values)
{
    const MODIFICATION mod = MOD_JSON::parse(R"({"kind":"replace","position":{"type":"all"},
        "value":[{"token":"original_name"},{"text":"_"},{"token":"size"}]})");
    TOKEN_CONTEXT context;
    context.original_name = U"my.folder";
    context.is_dir = true;
    context.size = 4096;
    EXPECT_EQ(TOKEN_FORMATTER::build(mod, context), U"my.folder_");
}

TEST(TOKEN_FORMATTER, builds_date_values)
{
    const MODIFICATION mod = MOD_JSON::parse(R"({"kind":"insert","position":{"type":"start"},
        "value":[{"token":"modified_date"},{"text":"-"},{"token":"created_date"}],
        "tokens":{"modified_date":{"format":"yyyy"},"created_date":{"format":"MMdd"}}})");
    TOKEN_CONTEXT context;
    context.modified_ms = 1735689599123;
    context.created_ms = 1735689599123;
    const std::optional<DATE_PARTS> parts = TOKEN_FORMATTER::local_parts(context.modified_ms);
    ASSERT_TRUE(parts.has_value());
    EXPECT_EQ(TOKEN_FORMATTER::build(mod, context),
        TOKEN_FORMATTER::format_date(U"yyyy-MMdd", *parts));
}

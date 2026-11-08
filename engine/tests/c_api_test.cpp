#include "c_api_test.h"

TEST(C_API, version_matches_token)
{
    const std::string prefix = "LIMENAMER_VERSION=";
    EXPECT_EQ(prefix + ln_version(), SESSION::VERSION_TOKEN);
}

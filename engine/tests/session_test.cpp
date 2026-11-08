#include "session_test.h"

TEST(SESSION, version_is_x_y_z)
{
    EXPECT_TRUE(std::regex_match(SESSION::version(), std::regex(R"(\d+\.\d+\.\d+)")));
}

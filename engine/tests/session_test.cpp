#include "session_test.h"

TEST(SESSION, version_is_x_y_z)
{
    EXPECT_TRUE(std::regex_match(SESSION::version(), std::regex(R"(\d+\.\d+\.\d+)")));
}

TEST(SESSION, keeps_given_services)
{
    std::unique_ptr<TEXT_SERVICE> text_service = PLATFORM::create_text_service();
    std::unique_ptr<FILE_SYSTEM> file_system = PLATFORM::create_file_system();
    const TEXT_SERVICE *text_service_address = text_service.get();
    const FILE_SYSTEM *file_system_address = file_system.get();
    SESSION session(std::move(text_service), std::move(file_system));
    EXPECT_EQ(&session.text_service(), text_service_address);
    EXPECT_EQ(&session.file_system(), file_system_address);
}

TEST(SESSION, rejects_missing_services)
{
    EXPECT_THROW(SESSION(nullptr, PLATFORM::create_file_system()), std::invalid_argument);
    EXPECT_THROW(SESSION(PLATFORM::create_text_service(), nullptr), std::invalid_argument);
}

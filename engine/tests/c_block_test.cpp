#include "c_block_test.h"

TEST(C_BLOCK, aligns_items_after_the_head)
{
    C_BLOCK<LN_PROBLEM, uint64_t> block(3, 0);
    const uintptr_t head = reinterpret_cast<uintptr_t>(&block.head());
    const uintptr_t items = reinterpret_cast<uintptr_t>(block.items());
    EXPECT_EQ(items % alignof(uint64_t), 0u);
    EXPECT_GE(items, head + sizeof(LN_PROBLEM));
    EXPECT_EQ(block.items()[2], 0u);
}

TEST(C_BLOCK, copies_texts_after_the_items)
{
    C_BLOCK<LN_PROBLEMS, LN_PROBLEM> block(2, 6);
    const char *first = block.copy("ab");
    const char *second = block.copy("");
    const char *third = block.copy("c");
    EXPECT_EQ(std::string(first), "ab");
    EXPECT_EQ(std::string(second), "");
    EXPECT_EQ(std::string(third), "c");
    EXPECT_GE(reinterpret_cast<uintptr_t>(first),
        reinterpret_cast<uintptr_t>(block.items() + 2));
    EXPECT_THROW(block.copy(""), std::logic_error);
}

TEST(C_BLOCK, has_no_items_when_empty)
{
    C_BLOCK<LN_PROBLEMS, LN_PROBLEM> block(0, 0);
    EXPECT_EQ(block.items(), nullptr);
    EXPECT_EQ(block.head().count, 0u);
}

TEST(C_BLOCK, releases_the_memory_to_the_caller)
{
    LN_PROBLEMS *problems = nullptr;
    {
        C_BLOCK<LN_PROBLEMS, LN_PROBLEM> block(1, 0);
        block.head().count = 1;
        problems = block.release();
        EXPECT_THROW(block.copy(""), std::logic_error);
    }
    EXPECT_EQ(problems->count, 1u);
    std::free(problems);
}

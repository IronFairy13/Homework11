#include <gtest/gtest.h>

#include "join_server/command.hpp"
#include "join_server/tables.hpp"

using join_server::CommandProcessor;
using join_server::CommandOutput;
using join_server::TableId;
using join_server::TablesStore;

TEST(CommandProcessorSuite, InsertSuccess)
{
  TablesStore store;
  CommandProcessor processor(store);

  const CommandOutput output = processor.execute("INSERT A 0 lean");
  ASSERT_TRUE(output.success);
  ASSERT_EQ(1U, output.lines.size());
  EXPECT_EQ("OK", output.lines.front());
}

TEST(CommandProcessorSuite, InsertRejectsDuplicate)
{
  TablesStore store;
  CommandProcessor processor(store);

  ASSERT_TRUE(processor.execute("INSERT A 1 sweater").success);
  const auto duplicate = processor.execute("INSERT A 1 coat");
  ASSERT_FALSE(duplicate.success);
  ASSERT_EQ(1U, duplicate.lines.size());
  EXPECT_EQ("ERR duplicate 1", duplicate.lines.front());
}

TEST(CommandProcessorSuite, TruncateClearsTables)
{
  TablesStore store;
  CommandProcessor processor(store);

  ASSERT_TRUE(processor.execute("INSERT A 2 frank").success);
  ASSERT_TRUE(processor.execute("TRUNCATE A").success);

  const auto result = processor.execute("INTERSECTION");
  ASSERT_TRUE(result.success);
  ASSERT_EQ(1U, result.lines.size());
  EXPECT_EQ("OK", result.lines.front());
}

TEST(CommandProcessorSuite, IntersectionOutputMatchesExpectation)
{
  TablesStore store;
  CommandProcessor processor(store);

  ASSERT_TRUE(processor.execute("INSERT A 3 violation").success);
  ASSERT_TRUE(processor.execute("INSERT A 4 quality").success);
  ASSERT_TRUE(processor.execute("INSERT A 5 precision").success);

  ASSERT_TRUE(processor.execute("INSERT B 3 proposal").success);
  ASSERT_TRUE(processor.execute("INSERT B 4 example").success);
  ASSERT_TRUE(processor.execute("INSERT B 5 lake").success);

  const auto result = processor.execute("INTERSECTION");
  ASSERT_TRUE(result.success);
  ASSERT_EQ(4U, result.lines.size());
  EXPECT_EQ("3,violation,proposal", result.lines[0]);
  EXPECT_EQ("4,quality,example", result.lines[1]);
  EXPECT_EQ("5,precision,lake", result.lines[2]);
  EXPECT_EQ("OK", result.lines[3]);
}

TEST(CommandProcessorSuite, SymmetricDifferenceOutputMatchesExpectation)
{
  TablesStore store;
  CommandProcessor processor(store);

  ASSERT_TRUE(processor.execute("INSERT A 0 lean").success);
  ASSERT_TRUE(processor.execute("INSERT A 1 sweater").success);
  ASSERT_TRUE(processor.execute("INSERT A 2 frank").success);
  ASSERT_TRUE(processor.execute("INSERT A 5 precision").success);

  ASSERT_TRUE(processor.execute("INSERT B 5 lake").success);
  ASSERT_TRUE(processor.execute("INSERT B 6 flour").success);
  ASSERT_TRUE(processor.execute("INSERT B 7 wonder").success);
  ASSERT_TRUE(processor.execute("INSERT B 8 selection").success);

  const auto result = processor.execute("SYMMETRIC_DIFFERENCE");
  ASSERT_TRUE(result.success);
  ASSERT_EQ(7U, result.lines.size());
  EXPECT_EQ("0,lean,", result.lines[0]);
  EXPECT_EQ("1,sweater,", result.lines[1]);
  EXPECT_EQ("2,frank,", result.lines[2]);
  EXPECT_EQ("6,,flour", result.lines[3]);
  EXPECT_EQ("7,,wonder", result.lines[4]);
  EXPECT_EQ("8,,selection", result.lines[5]);
  EXPECT_EQ("OK", result.lines[6]);
}

TEST(CommandProcessorSuite, UnknownCommandReturnsError)
{
  TablesStore store;
  CommandProcessor processor(store);

  const auto result = processor.execute("PING");
  ASSERT_FALSE(result.success);
  ASSERT_EQ(1U, result.lines.size());
  EXPECT_EQ("ERR unknown command PING", result.lines.front());
}

TEST(CommandProcessorSuite, InvalidInsertFormat)
{
  TablesStore store;
  CommandProcessor processor(store);

  const auto result = processor.execute("INSERT A 10");
  ASSERT_FALSE(result.success);
  ASSERT_EQ("ERR wrong command format", result.lines.front());
}

TEST(CommandProcessorSuite, InvalidTableName)
{
  TablesStore store;
  CommandProcessor processor(store);

  const auto result = processor.execute("TRUNCATE C");
  ASSERT_FALSE(result.success);
  ASSERT_EQ("ERR unknown table C", result.lines.front());
}

TEST(CommandProcessorSuite, InvalidId)
{
  TablesStore store;
  CommandProcessor processor(store);

  const auto result = processor.execute("INSERT A abc value");
  ASSERT_FALSE(result.success);
  ASSERT_EQ("ERR invalid id abc", result.lines.front());
}

#include <gtest/gtest.h>

#include "join_server/tables.hpp"

TEST(TablesStoreSuite, InsertsAndRejectsDuplicates)
{
  join_server::TablesStore store;
  std::string error;

  EXPECT_TRUE(store.insert(join_server::TableId::A, 0, "lean", error));
  EXPECT_TRUE(store.insert(join_server::TableId::B, 3, "proposal", error));

  EXPECT_FALSE(store.insert(join_server::TableId::A, 0, "duplicate", error));
  EXPECT_EQ("duplicate 0", error);
}

TEST(TablesStoreSuite, TruncateRemovesEntries)
{
  join_server::TablesStore store;
  std::string error;

  ASSERT_TRUE(store.insert(join_server::TableId::A, 0, "lean", error));
  ASSERT_TRUE(store.insert(join_server::TableId::A, 1, "sweater", error));

  store.truncate(join_server::TableId::A);

  const auto rows = store.intersection();
  EXPECT_TRUE(rows.empty());
}

TEST(TablesStoreSuite, IntersectionReturnsCommonIds)
{
  join_server::TablesStore store;
  std::string error;

  ASSERT_TRUE(store.insert(join_server::TableId::A, 3, "violation", error));
  ASSERT_TRUE(store.insert(join_server::TableId::A, 4, "quality", error));
  ASSERT_TRUE(store.insert(join_server::TableId::A, 5, "precision", error));

  ASSERT_TRUE(store.insert(join_server::TableId::B, 3, "proposal", error));
  ASSERT_TRUE(store.insert(join_server::TableId::B, 4, "example", error));
  ASSERT_TRUE(store.insert(join_server::TableId::B, 5, "lake", error));

  const auto rows = store.intersection();
  ASSERT_EQ(3U, rows.size());
  EXPECT_EQ(3, rows[0].id);
  EXPECT_EQ("violation", rows[0].from_a);
  EXPECT_EQ("proposal", rows[0].from_b);

  EXPECT_EQ(4, rows[1].id);
  EXPECT_EQ("quality", rows[1].from_a);
  EXPECT_EQ("example", rows[1].from_b);

  EXPECT_EQ(5, rows[2].id);
  EXPECT_EQ("precision", rows[2].from_a);
  EXPECT_EQ("lake", rows[2].from_b);
}

TEST(TablesStoreSuite, SymmetricDifferenceReturnsUniqueIds)
{
  join_server::TablesStore store;
  std::string error;

  ASSERT_TRUE(store.insert(join_server::TableId::A, 0, "lean", error));
  ASSERT_TRUE(store.insert(join_server::TableId::A, 1, "sweater", error));
  ASSERT_TRUE(store.insert(join_server::TableId::A, 2, "frank", error));
  ASSERT_TRUE(store.insert(join_server::TableId::A, 5, "precision", error));

  ASSERT_TRUE(store.insert(join_server::TableId::B, 5, "lake", error));
  ASSERT_TRUE(store.insert(join_server::TableId::B, 6, "flour", error));
  ASSERT_TRUE(store.insert(join_server::TableId::B, 7, "wonder", error));
  ASSERT_TRUE(store.insert(join_server::TableId::B, 8, "selection", error));

  const auto rows = store.symmetric_difference();
  ASSERT_EQ(6U, rows.size());

  EXPECT_EQ(0, rows[0].id);
  EXPECT_EQ("lean", rows[0].from_a);
  EXPECT_TRUE(rows[0].from_b.empty());

  EXPECT_EQ(1, rows[1].id);
  EXPECT_EQ("sweater", rows[1].from_a);
  EXPECT_TRUE(rows[1].from_b.empty());

  EXPECT_EQ(2, rows[2].id);
  EXPECT_EQ("frank", rows[2].from_a);
  EXPECT_TRUE(rows[2].from_b.empty());

  EXPECT_EQ(6, rows[3].id);
  EXPECT_TRUE(rows[3].from_a.empty());
  EXPECT_EQ("flour", rows[3].from_b);

  EXPECT_EQ(7, rows[4].id);
  EXPECT_TRUE(rows[4].from_a.empty());
  EXPECT_EQ("wonder", rows[4].from_b);

  EXPECT_EQ(8, rows[5].id);
  EXPECT_TRUE(rows[5].from_a.empty());
  EXPECT_EQ("selection", rows[5].from_b);
}

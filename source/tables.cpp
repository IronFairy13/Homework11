#include "join_server/tables.hpp"


namespace join_server
{

TablesStore::TablesStore() = default;

bool TablesStore::insert(TableId table, int id, const std::string &value, std::string &error)
{
  std::lock_guard<std::mutex> lk(mtx_);
  auto &target = table_ref(table);
  const auto [it, inserted] = target.emplace(id, value);
  if (!inserted)
  {
    error = "duplicate " + std::to_string(id);
    return false;
  }
  return true;
}

void TablesStore::truncate(TableId table)
{
  std::lock_guard<std::mutex> lk(mtx_);
  table_ref(table).clear();
}

std::vector<DataRow> TablesStore::intersection() const
{
  std::lock_guard<std::mutex> lk(mtx_);
  std::vector<DataRow> rows;
  auto it_a = table_a_.begin();
  auto it_b = table_b_.begin();
  while (it_a != table_a_.end() && it_b != table_b_.end())
  {
    if (it_a->first == it_b->first)
    {
      rows.push_back(DataRow{it_a->first, it_a->second, it_b->second});
      ++it_a;
      ++it_b;
      continue;
    }
    if (it_a->first < it_b->first)
    {
      ++it_a;
    }
    else
    {
      ++it_b;
    }
  }
  return rows;
}

std::vector<DataRow> TablesStore::symmetric_difference() const
{
  std::lock_guard<std::mutex> lk(mtx_);
  std::vector<DataRow> rows;
  auto it_a = table_a_.begin();
  auto it_b = table_b_.begin();
  while (it_a != table_a_.end() || it_b != table_b_.end())
  {
    if (it_b == table_b_.end() || (it_a != table_a_.end() && it_a->first < it_b->first))
    {
      rows.push_back(DataRow{it_a->first, it_a->second, {}});
      ++it_a;
      continue;
    }
    if (it_a == table_a_.end() || it_b->first < it_a->first)
    {
      rows.push_back(DataRow{it_b->first, {}, it_b->second});
      ++it_b;
      continue;
    }

    // keys are equal -> present in both, skip
    ++it_a;
    ++it_b;
  }
  return rows;
}

TablesStore::Table &TablesStore::table_ref(TableId table)
{
  return table == TableId::A ? table_a_ : table_b_;
}

const TablesStore::Table &TablesStore::table_ref(TableId table) const
{
  return table == TableId::A ? table_a_ : table_b_;
}

} // namespace join_server

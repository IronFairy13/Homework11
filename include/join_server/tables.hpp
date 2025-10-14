#pragma once

#include <map>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

namespace join_server
{

enum class TableId
{
  A,
  B
};

struct DataRow
{
  int id{};
  std::string from_a;
  std::string from_b;
};

class TablesStore
{
public:
  TablesStore();

  bool insert(TableId table, int id, const std::string &value, std::string &error);
  void truncate(TableId table);

  std::vector<DataRow> intersection() const;
  std::vector<DataRow> symmetric_difference() const;

private:
  using Table = std::map<int, std::string>;

  Table &table_ref(TableId table);
  const Table &table_ref(TableId table) const;

  Table table_a_;
  Table table_b_;
  mutable std::mutex mtx_;
};

} // namespace join_server

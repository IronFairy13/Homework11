#pragma once

#include "join_server/tables.hpp"

#include <string>
#include <vector>

namespace join_server
{

struct CommandOutput
{
  std::vector<std::string> lines;
  bool success{false};
};

class CommandProcessor
{
public:
  explicit CommandProcessor(TablesStore &store);

  CommandOutput execute(const std::string &command_line);

private:
  TablesStore &store_;
};

} // namespace join_server

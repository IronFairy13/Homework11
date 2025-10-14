#include "join_server/command.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>

namespace
{

std::string trim_copy(const std::string &value)
{
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return {};
  const auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1);
}

std::string to_upper_copy(std::string value)
{
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                 { return static_cast<char>(std::toupper(ch)); });
  return value;
}

bool parse_table_id(const std::string &token, join_server::TableId &out, std::string &error)
{
  const auto normalized = to_upper_copy(token);
  if (normalized == "A")
  {
    out = join_server::TableId::A;
    return true;
  }
  if (normalized == "B")
  {
    out = join_server::TableId::B;
    return true;
  }
  error = "unknown table " + token;
  return false;
}

bool parse_int(const std::string &token, int &out)
{
  try
  {
    size_t idx{};
    const long value = std::stol(token, &idx, 10);
    if (idx != token.size())
      return false;
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
      return false;
    out = static_cast<int>(value);
    return true;
  }
  catch (const std::exception &)
  {
    return false;
  }
}

std::string format_row(const join_server::DataRow &row)
{
  return std::to_string(row.id) + ',' + row.from_a + ',' + row.from_b;
}

} // namespace

namespace join_server
{

CommandProcessor::CommandProcessor(TablesStore &store) : store_(store) {}

CommandOutput CommandProcessor::execute(const std::string &command_line)
{
  CommandOutput output;

  const auto trimmed = trim_copy(command_line);
  if (trimmed.empty())
  {
    output.lines.push_back("ERR empty command");
    return output;
  }

  std::istringstream iss(trimmed);
  std::string command_token;
  iss >> command_token;
  const auto command = to_upper_copy(command_token);

  if (command == "INSERT")
  {
    std::string table_token;
    std::string id_token;
    if (!(iss >> table_token >> id_token))
    {
      output.lines.push_back("ERR wrong command format");
      return output;
    }

    std::string name_token;
    std::getline(iss, name_token);
    name_token = trim_copy(name_token);
    if (name_token.empty())
    {
      output.lines.push_back("ERR wrong command format");
      return output;
    }

    TableId table_id;
    std::string table_error;
    if (!parse_table_id(table_token, table_id, table_error))
    {
      output.lines.push_back("ERR " + table_error);
      return output;
    }

    int id{};
    if (!parse_int(id_token, id))
    {
      output.lines.push_back("ERR invalid id " + id_token);
      return output;
    }

    std::string error;
    if (!store_.insert(table_id, id, name_token, error))
    {
      output.lines.push_back("ERR " + error);
      return output;
    }

    output.lines.push_back("OK");
    output.success = true;
    return output;
  }

  if (command == "TRUNCATE")
  {
    std::string table_token;
    if (!(iss >> table_token))
    {
      output.lines.push_back("ERR wrong command format");
      return output;
    }

    std::string extra;
    if (iss >> extra)
    {
      output.lines.push_back("ERR wrong command format");
      return output;
    }

    TableId table_id;
    std::string table_error;
    if (!parse_table_id(table_token, table_id, table_error))
    {
      output.lines.push_back("ERR " + table_error);
      return output;
    }

    store_.truncate(table_id);
    output.lines.push_back("OK");
    output.success = true;
    return output;
  }

  if (command == "INTERSECTION")
  {
    std::string extra;
    if (iss >> extra)
    {
      output.lines.push_back("ERR wrong command format");
      return output;
    }

    const auto rows = store_.intersection();
    for (const auto &row : rows)
      output.lines.push_back(format_row(row));
    output.lines.push_back("OK");
    output.success = true;
    return output;
  }

  if (command == "SYMMETRIC_DIFFERENCE")
  {
    std::string extra;
    if (iss >> extra)
    {
      output.lines.push_back("ERR wrong command format");
      return output;
    }

    const auto rows = store_.symmetric_difference();
    for (const auto &row : rows)
      output.lines.push_back(format_row(row));
    output.lines.push_back("OK");
    output.success = true;
    return output;
  }

  output.lines.push_back("ERR unknown command " + command_token);
  return output;
}

} // namespace join_server

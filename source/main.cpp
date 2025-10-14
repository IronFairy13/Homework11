#include "join_server/server.hpp"
#include "join_server/tables.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

namespace
{

constexpr unsigned long kMaxPort = 65535UL;

} // namespace

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: join_server <port>\n";
    return EXIT_FAILURE;
  }

  try
  {
    const unsigned long value = std::stoul(argv[1]);
    if (value > kMaxPort)
      throw std::out_of_range("port overflow");

    const auto port = static_cast<uint16_t>(value);
    auto store = std::make_shared<join_server::TablesStore>();
    join_server::TcpServer server(port, std::move(store));
    server.run();
  }
  catch (const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

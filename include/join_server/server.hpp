#pragma once

#include <cstdint>
#include <memory>

namespace join_server
{

class TablesStore;

class TcpServer
{
public:
  TcpServer(uint16_t port, std::shared_ptr<TablesStore> store);
  ~TcpServer();

  TcpServer(const TcpServer &) = delete;
  TcpServer &operator=(const TcpServer &) = delete;

  void run();

private:
  void close_listener();
  void handle_client(int client_fd);
  bool send_all(int client_fd, const std::string &data);

  int listener_{-1};
  uint16_t port_{};
  std::shared_ptr<TablesStore> store_;
};

} // namespace join_server

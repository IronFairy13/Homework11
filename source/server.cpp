#include "join_server/server.hpp"

#include "join_server/command.hpp"
#include "join_server/tables.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

namespace
{

constexpr int kBacklog = 16;
constexpr std::size_t kBufferSize = 4096;

} // namespace

namespace join_server
{

TcpServer::TcpServer(uint16_t port, std::shared_ptr<TablesStore> store)
    : port_(port), store_(std::move(store))
{
  if (!store_)
    throw std::invalid_argument("TablesStore pointer must not be null");

  listener_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listener_ < 0)
    throw std::runtime_error(std::string("socket failed: ") + std::strerror(errno));

  int opt = 1;
  ::setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port_);

  if (::bind(listener_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
  {
    const auto err = std::string("bind failed: ") + std::strerror(errno);
    close_listener();
    throw std::runtime_error(err);
  }

  if (::listen(listener_, kBacklog) < 0)
  {
    const auto err = std::string("listen failed: ") + std::strerror(errno);
    close_listener();
    throw std::runtime_error(err);
  }
}

TcpServer::~TcpServer()
{
  close_listener();
}

void TcpServer::close_listener()
{
  if (listener_ >= 0)
  {
    ::close(listener_);
    listener_ = -1;
  }
}

void TcpServer::run()
{
  std::cout << "join_server listening on port " << port_ << std::endl;
  for (;;)
  {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = ::accept(listener_, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
    if (client_fd < 0)
    {
      if (errno == EINTR)
        continue;
      std::cerr << "accept failed: " << std::strerror(errno) << std::endl;
      break;
    }

    std::thread(&TcpServer::handle_client, this, client_fd).detach();
  }
}

bool TcpServer::send_all(int client_fd, const std::string &data)
{
  std::size_t total_sent = 0;
  const char *buffer = data.data();
  const std::size_t size = data.size();

  while (total_sent < size)
  {
    const ssize_t sent = ::send(client_fd, buffer + total_sent, size - total_sent, 0);
    if (sent < 0)
    {
      if (errno == EINTR)
        continue;
      std::cerr << "send failed: " << std::strerror(errno) << std::endl;
      return false;
    }
    total_sent += static_cast<std::size_t>(sent);
  }
  return true;
}

void TcpServer::handle_client(int client_fd)
{
  CommandProcessor processor(*store_);
  std::string buffer;
  buffer.reserve(kBufferSize);
  char chunk[kBufferSize];

  bool running = true;
  for (; running;)
  {
    const ssize_t received = ::recv(client_fd, chunk, sizeof(chunk), 0);
    if (received == 0)
      break; // connection closed

    if (received < 0)
    {
      if (errno == EINTR)
        continue;
      std::cerr << "recv failed: " << std::strerror(errno) << std::endl;
      break;
    }

    buffer.append(chunk, static_cast<std::size_t>(received));

    std::size_t processed = 0;
    while (true)
    {
      const auto newline_pos = buffer.find('\n', processed);
      if (newline_pos == std::string::npos)
      {
        if (processed > 0)
          buffer.erase(0, processed);
        break;
      }

      std::string line = buffer.substr(processed, newline_pos - processed);
      if (!line.empty() && line.back() == '\r')
        line.pop_back();

      processed = newline_pos + 1;

      const auto result = processor.execute(line);
      std::string response;
      response.reserve(result.lines.size() * 8);
      for (const auto &entry : result.lines)
      {
        response.append(entry);
        response.push_back('\n');
      }

      if (!send_all(client_fd, response))
      {
        running = false;
        break;
      }
    }
  }

  ::close(client_fd);
}

} // namespace join_server

#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>

#include "unique_fd.h"

namespace protei
{

  class Server
  {
  public:
    Server(uint16_t port) : port_(port),
                            fd_(socket(AF_INET, SOCK_STREAM, 0))
    {

      if (0 > fd_)
      {
        throw std::runtime_error("no socket can't be created");
      }

      sockaddr_in serverAddress;
      serverAddress.sin_family = AF_INET;
      serverAddress.sin_port = htons(port);
      serverAddress.sin_addr.s_addr = INADDR_ANY;

      int opt = 1;
      if (setsockopt(fd_.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
      {
        throw std::runtime_error("Error setting options");
      }

      if (bind(fd_.get(), reinterpret_cast<struct sockaddr *>(&serverAddress),
               sizeof(serverAddress)) < 0)
      {
        throw std::runtime_error("Error socket binding");
      }

      if (set_nonblocking(fd_) < 0)
      {
        throw std::runtime_error("Error set non blocking");
      }
    }

    void start()
    {

      // if (listend())
    }

  private:
    uint16_t port_;
    UniqueFd fd_;

    int set_nonblocking(const UniqueFd &fd)
    {
      return fcntl(fd.get(), fcntl(fd.get(), F_SETFL, F_GETFD, 0) | O_NONBLOCK);
    }
  };
} // namespace protei

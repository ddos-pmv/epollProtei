#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <condition_variable>

#include <cstdint>

#include "unique_fd.h"

namespace protei
{

  class Server
  {
  public:
    Server(uint16_t port, int thread_count = 6);

    void start();

  private:
    void worker_thread();

    void accept_new_connection();
    void add_to_queue(int fd);
    void process_client(int fd);

    uint16_t port_;
    UniqueFd fd_;
    UniqueFd epoll_fd_;

    std::vector<UniqueFd> clients_;

    int thread_count_;
    std::vector<std::thread>
        thread_pool_;
    std::queue<int>
        connection_queue_;
    alignas(64) std::condition_variable cv_;
    alignas(64) std::mutex queue_mtx_;
    alignas(64) std::atomic_bool stop_;
  };
} // namespace protei

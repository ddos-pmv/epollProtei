#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <iostream>

#define MAX_EVENTS 1024

namespace protei
{

    Server::Server(uint16_t port, int thread_count) : port_(port),
                                                      fd_(socket(AF_INET, SOCK_STREAM, 0)),
                                                      epoll_fd_(epoll_create1(0)),
                                                      thread_count_(thread_count),
                                                      stop_(false)
    {
        if (0 > fd_)
            throw std::runtime_error("no socket can't be created");

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        int opt = 1;
        if (setsockopt(fd_.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
            throw std::runtime_error("Error setting options");

        // Bind server socket to port
        if (bind(fd_.get(), reinterpret_cast<struct sockaddr *>(&serverAddress),
                 sizeof(serverAddress)) < 0)
            throw std::runtime_error("Error socket binding");

        // Make listening
        if (listen(fd_.get(), SOMAXCONN) < 0)
            throw std::runtime_error("Error listening on socket");

        // Check if epoll was created
        if (epoll_fd_ < 0)
            throw std::runtime_error("Erroe creating epoll");

        epoll_event ev{};
        ev.events = EPOLLIN; // LT by default

        auto server_ctx = new ClientCtx;
        server_ctx->fd = fd_.get();
        ev.data.ptr = server_ctx;

        std::cout
            << "SIZEOF(epool_event)" << sizeof(epoll_event) << std::endl;

        std::cout << "sizeof(UniqueFd)" << sizeof(UniqueFd) << std::endl;

        // Add server's fd_ to get accept events
        if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, fd_.get(), &ev) < 0)
            throw std::runtime_error("Error adding server socket to epoll");

        // Start thread pool
        for (int i = 0; i < thread_count_; i++)
            thread_pool_.emplace_back([this]()
                                      { this->worker_thread(); });
    }

    void Server::start()
    {
        epoll_event events[MAX_EVENTS];
        while (!stop_)
        {
            int event_count = epoll_wait(epoll_fd_.get(), events, MAX_EVENTS, -1);

            if (event_count < 0 & errno != EINTR)
                std::runtime_error("Error in epoll_wait");

            for (int i = 0; i < event_count; i++)
            {
                if ((reinterpret_cast<ClientCtx *>(events[i].data.ptr))->fd == fd_)
                {
                    try
                    {
                        accept_new_connection();
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
                else
                {
                    add_to_queue(*(ClientCtx *)(events[i].data.ptr));
                }
            }
        }
    }

    void Server::worker_thread()
    {
        while (!stop_.load(std::memory_order_acquire))
        {
            ClientCtx client_ctx{};
            {
                std::unique_lock lock(queue_mtx_);
                cv_.wait(lock, [this]()
                         { return stop_ || !connection_queue_.empty(); });

                if (stop_ && connection_queue_.empty())
                    break;

                client_ctx = connection_queue_.front();
                connection_queue_.pop();
                {
                    std::unique_lock set_lock(set_mtx_);
                    if (busy_clients_.contains(client_ctx.fd))
                        continue;
                    else
                        busy_clients_.insert(client_ctx.fd);
                }
            }

            process_client(client_ctx);
        }
    }

    void Server::accept_new_connection()
    {
        sockaddr_in clinet_addr{};
        socklen_t client_addr_size = sizeof(sockaddr_in);
        UniqueFd client_fd(accept(fd_.get(), reinterpret_cast<sockaddr *>(&clinet_addr),
                                  &client_addr_size));

        if (client_fd < 0)
            throw std::runtime_error("Error acceptin client");

        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = client_fd.get();

        // Add client_fd to epoll
        if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, client_fd.get(), &ev) < 0)
            throw std::runtime_error("Error adding client socket to epoll");

        // Save client_fd
        // clients_.emplace_back(std::move(client_fd));
        client_fd.release();
    }

    void Server::add_to_queue(ClientCtx fd)
    {
        std::unique_lock lock(queue_mtx_);
        connection_queue_.push(fd);
        cv_.notify_one();
    }

    void Server::process_client(ClientCtx client_fd)
    {

        // Пример обработки
        char buffer[1024];

        // Здесь мы должны прочитать арифметическое выржаение
        // посчтиать выржаение и отпрваить ответ
        // выржаения разделяются пробелами, но
        // мы можем прочесть выражегние не полностью...
        // ssize_t n = read(client_fd, buffer, sizeof(buffer));
        // if (n > 0)
        // {
        //     std::cout << "Received: " << std::string(buffer, n) << std::endl;
        //     // Отправка ответа
        //     write(client_fd, "OK", 2);
        // }
        // else
        // {
        //     std::cerr << "Error reading from client" << std::endl;
        // }

        // close(client_fd);
    }

    // void Server::safe_write(int client_fd, const std::string &data)
    // {
    //     ::write(client_fd, data.data(), data.size());
    // }

} // namespace protei

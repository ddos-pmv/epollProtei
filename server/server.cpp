#include "server.h"

#include "unique_ctx.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <memory>
#include <iostream>

#define MAX_EVENTS 1024

namespace protei
{

    Server::Server(uint16_t port, int thread_count) : port_(port),
                                                      server_ctx_(std::make_shared<UniqueCtx>(socket(AF_INET, SOCK_STREAM, 0))),
                                                      epoll_fd_(epoll_create1(0)),
                                                      thread_count_(thread_count),
                                                      stop_(false)
    {
        if (0 > server_ctx_->fd())
            throw std::runtime_error("no socket can't be created");

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        int opt = 1;
        if (setsockopt(server_ctx_->fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
            throw std::runtime_error("Error setting options");

        // Bind server socket to port
        if (bind(server_ctx_->fd(), reinterpret_cast<struct sockaddr *>(&serverAddress),
                 sizeof(serverAddress)) < 0)
            throw std::runtime_error("Error socket binding");

        // Make listening
        if (listen(server_ctx_->fd(), SOMAXCONN) < 0)
            throw std::runtime_error("Error listening on socket");

        // Check if epoll was created
        if (epoll_fd_ < 0)
            throw std::runtime_error("Erroe creating epoll");

        // Setting evpoll_event with server context
        epoll_event ev{};
        ev.events = EPOLLIN; // LT by default
        ev.data.ptr = server_ctx_.get();

        // Add server's fd_ to get accept events
        if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, server_ctx_->fd(), &ev) < 0)
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
                UniqueCtx *ctx_ptr = static_cast<UniqueCtx *>(events[i].data.ptr);
                if (ctx_ptr->fd() == server_ctx_->fd())
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
                    add_to_queue(ctx_ptr);
                }
            }
        }
    }

    void Server::worker_thread()
    {
        while (!stop_.load(std::memory_order_acquire))
        {
            UniqueCtx *client_ctx;

            {
                std::unique_lock lock(queue_mtx_);
                cv_.wait(lock, [this]()
                         { return stop_ || !connection_queue_.empty(); });

                if (stop_ && connection_queue_.empty())
                    break;

                client_ctx = connection_queue_.front();
                connection_queue_.pop();
            }
            {
                std::unique_lock set_lock(set_mtx_);
                // Skip this client if it's even in thread
                if (busy_clients_.contains(client_ctx->fd()))
                    continue;
                else
                {
                    busy_clients_.insert(client_ctx->fd());
                    process_client(client_ctx);
                }
            }
        }
    }

    void Server::accept_new_connection()
    {
        sockaddr_in clinet_addr{};
        socklen_t client_addr_size = sizeof(sockaddr_in);
        CtxSharedPtr client_ctx = std::make_shared<UniqueCtx>(accept(server_ctx_->fd(), reinterpret_cast<sockaddr *>(&clinet_addr),
                                                                     &client_addr_size));

        if (client_ctx->fd() < 0)
            throw std::runtime_error("Error acceptin client");
        else
        {
            std::unique_lock lock(map_mtx_);
            client_buffers_[client_ctx->fd()] = client_ctx;
        }

        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.ptr = client_ctx.get();

        // Add client_fd to epoll
        if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, client_ctx->fd(), &ev) < 0)
            throw std::runtime_error("Error adding client socket to epoll");
    }

    void Server::add_to_queue(UniqueCtx *const client_ctx)
    {
        std::unique_lock lock(queue_mtx_);
        connection_queue_.push(client_ctx);
        cv_.notify_one();
    }

    void Server::process_client(UniqueCtx *const client_ctx)
    {

        // Пример обработки

        // Здесь мы должны прочитать арифметическое выржаение
        // посчтиать выржаение и отпрваить ответ
        // выржаения разделяются пробелами, но
        // мы можем прочесть выражегние не полностью...

        // ssize_t n = read(client_ctx->fd(), client_ctx->buffer().data(), sizeof());

        // write(client_ctx->fd(), client_ctx->buffer().data)
    }

} // namespace protei

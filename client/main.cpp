#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <generator.h>
#include <calculator.h>

constexpr int MAX_EVENTS = 1024;

std::vector<std::string> fragment_expression(const std::string &expr)
{
    static std::mt19937 rng{std::random_device{}()};
    std::vector<std::string> fragments;
    size_t pos = 0;
    while (pos < expr.size())
    {
        std::uniform_int_distribution<size_t> len_dist(1, expr.size() - pos);
        size_t len = len_dist(rng);
        fragments.push_back(expr.substr(pos, len));
        pos += len;
    }
    return fragments;
}
struct Connection
{
    int fd;
    std::string expression;
    std::vector<std::string> fragments;
    size_t fragment_index = 0;
    std::string recv_buffer;
    int expected_result;
    bool done = false;
};

int make_socket(const std::string &ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0)
        throw std::runtime_error("socket() failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    connect(fd, (sockaddr *)&addr, sizeof(addr));
    return fd;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: ./client <n> <connections> <server_addr> <server_port>\n";
        return 1;
    }

    int n = std::stoi(argv[1]);
    int conn_count = std::stoi(argv[2]);
    std::string server_ip = argv[3];
    int port = std::stoi(argv[4]);

    int epoll_fd = epoll_create1(0);
    std::unordered_map<int, Connection> connections;

    for (int i = 0; i < conn_count; ++i)
    {
        try
        {
            int fd = make_socket(server_ip, port);
            std::string expr = protei::generate_expression(n);
            int result = protei::eval_expr(expr);
            auto fragments = fragment_expression(expr);

            connections[fd] = Connection{fd, expr, fragments, 0, "", result, false};

            epoll_event ev{};
            ev.events = EPOLLOUT | EPOLLIN | EPOLLET;
            ev.data.fd = fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Connection failed: " << e.what() << "\n";
        }
    }

    epoll_event events[MAX_EVENTS];

    while (!connections.empty())
    {
        int nfd = epoll_wait(epoll_fd, events, MAX_EVENTS, 5000);
        if (nfd < 0)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfd; ++i)
        {
            int fd = events[i].data.fd;
            auto &conn = connections[fd];

            if (events[i].events & EPOLLOUT)
            {
                while (conn.fragment_index < conn.fragments.size())
                {
                    ssize_t sent = send(fd, conn.fragments[conn.fragment_index].data(),
                                        conn.fragments[conn.fragment_index].size(), 0);
                    if (sent < 0)
                        break;
                    ++conn.fragment_index;
                }
            }

            if (events[i].events & EPOLLIN)
            {
                char buf[1024];
                ssize_t len = recv(fd, buf, sizeof(buf), 0);
                if (len <= 0)
                {
                    close(fd);
                    connections.erase(fd);
                    continue;
                }
                conn.recv_buffer.append(buf, len);

                try
                {
                    int server_result = std::stoi(conn.recv_buffer);
                    if (server_result != conn.expected_result)
                    {
                        std::cerr << "Wrong result!\n"
                                  << "Expr: " << conn.expression << "\n"
                                  << "Expected: " << conn.expected_result << "\n"
                                  << "Got: " << server_result << "\n";
                    }
                    close(fd);
                    connections.erase(fd);
                }
                catch (...)
                {
                    // еще не вся строка пришла
                }
            }
        }
    }

    return 0;
}
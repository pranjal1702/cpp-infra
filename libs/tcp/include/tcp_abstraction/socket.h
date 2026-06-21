#pragma once
#include<cstdint>
#include<sys/socket.h>
#include<netinet/in.h>

namespace infra::net{
class Socket{
public:
    // factory function for creating TCP socket
    static Socket createTcp();
    // factory function for creating TCP socket from already owned fd
    static Socket tcpSocketFromFd(const int fd);

    // delete default, copy constructor and copy assignment
    Socket() = delete;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&)=delete;
    // need to implement move constructor and move assignment

    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&&) noexcept;

    ~Socket() noexcept;
    bool isValid() const;

    void close();

    // function for setting non blocking
    void setNonBlocking(bool nb=true);

    // disable Nagle's algorithms batching
    void setNoDelay(bool no_delay=true);

    // reuse addr for instant restartability of server
    void setReuseAddr(bool enable = true);
    // set buffer sizes
    void setRecvBufSize(int bytes);
    void setSendBufSize(int bytes);
private:

    explicit Socket(const int fd);

    int fd_;
};
}// infra::net namespace ends
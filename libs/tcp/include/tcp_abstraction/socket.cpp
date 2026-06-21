#include "socket.h"
#include <system_error>
#include<cerrno>
#include<unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
namespace infra::net{
Socket Socket::createTcp(){
    int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(fd<0){
        int err=errno;
        throw std::system_error(err, std::generic_category(), "socket()");
    }
    return Socket(fd);
}

Socket Socket::tcpSocketFromFd(const int fd){
    if (fd < 0) throw std::invalid_argument("Socket::tcpSocketFromFd: invalid fd");
    return Socket(fd);
}

Socket::Socket(const int fd): fd_(fd) {}

Socket::Socket(Socket&& other) noexcept{
    fd_=other.fd_;
    other.fd_=-1;
}

Socket& Socket::operator=(Socket&& other) noexcept{
    if(this==&other) return *this;
    // close the current socket
    close();
    fd_=other.fd_;
    other.fd_=-1;
    return *this;
}

Socket::~Socket(){
    close();
}

void Socket::setNonBlocking(bool nb){
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        throw std::system_error(errno,std::generic_category(),"fcntl(F_GETFL)");
    }
    flags = nb ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (::fcntl(fd_, F_SETFL, flags) < 0) {
        throw std::system_error(errno,std::generic_category(),"fcntl(F_SETFL)");
    }
}

bool Socket::isValid() const{
    return fd_ >=0;
}

void Socket::close(){
    if(fd_==-1) return;
    ::close(fd_);
}

void Socket::setNoDelay(bool no_delay){
    int flag = no_delay ? 1 : 0;
    if (::setsockopt(fd_,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(flag)) < 0)
    {
        throw std::system_error(errno,std::generic_category(),"setsockopt(TCP_NODELAY)");
    }
}

void Socket::setReuseAddr(bool enable)
{
    int flag = enable ? 1 : 0;

    if (::setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag)) < 0)
    {
        throw std::system_error(errno,std::generic_category(),"setsockopt(SO_REUSEADDR)");
    }
}

void Socket::setRecvBufSize(int bytes)
{
    if (::setsockopt(fd_,SOL_SOCKET,SO_RCVBUF,&bytes,sizeof(bytes)) == -1)
    {
        throw std::system_error(errno,std::generic_category(),"setsockopt(SO_RCVBUF)");
    }
}

void Socket::setSendBufSize(int bytes)
{
    if (::setsockopt(fd_,SOL_SOCKET,SO_SNDBUF,&bytes,sizeof(bytes)) == -1)
    {
        throw std::system_error(errno,std::generic_category(),"setsockopt(SO_SNDBUF)");
    }
}

}// namespace ends
#include "tcpsocket.h"

#include <vector>
#include <sstream>
#include <string.h>
#include <iomanip>
#include <iostream>

#ifdef _TCPSOCKET_WIN
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <winsock2.h>
#include "pch.h"
#else
#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#ifdef _TCPSOCKET_WIN
bool _wsa_inited = false;

int _wsa_init()
{
    if(!_wsa_inited)
    {
        WSADATA wsaData;
        int wsaRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaRes != 0)
        {
            return wsaRes;
        }
        _wsa_inited = true;
    }
    return 0;
}

int _wsa_uninit()
{
    if(_wsa_inited)
    {
        _wsa_inited = false;
        return WSACleanup();
    }
    return 0;
}
#endif

std::string Int2Str(uint16_t i)
{
    std::stringstream ss;
    ss << i;
    return ss.str();
}

TcpListener::TcpListener()
{
    active = false;
    #ifdef _TCPSOCKET_WIN
    _wsa_init();
    #endif
}

TcpListener::~TcpListener()
{
    if(IsActive())
        Close();
}

bool TcpListener::IsActive() const
{
    return active;
}

bool TcpListener::Start(uint16_t port)
{
    return Start(port, false);
}

bool TcpListener::Start(uint16_t port, bool loopback)
{
    struct sockaddr_in serv_addr;
    int addressFamily = AF_INET;
    socket_id = (socket_t)socket(addressFamily, SOCK_STREAM, IPPROTO_TCP);
    int iSetOption = 1;
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
    if (socket_id < 0)
        return false;

    serv_addr.sin_family = addressFamily;
    serv_addr.sin_addr.s_addr = loopback ? INADDR_LOOPBACK : INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(socket_id, (struct sockaddr*)&serv_addr,
        sizeof(serv_addr)) < 0)
    {
        return false;
    }
    active = true;
    return true;
}

bool TcpListener::Close()
{
    active = false;
    #ifdef _TCPSOCKET_WIN
    int result = closesocket(socket_id);
    #else
    int result = ::close(socket_id);
    #endif
    return result == 0;
}

bool TcpListener::AcceptClient(TcpSocket& client) const
{
    socklen_t clilen;
    sockaddr_in cli_addr;

    if(client.IsConnected())
        return false;

    listen(socket_id, 1);
    clilen = sizeof(cli_addr);
    socket_t client_id = (socket_t)accept(socket_id, (sockaddr*)&cli_addr, &clilen);
    #ifdef _TCPSOCKET_WIN
    if(client_id == INVALID_SOCKET || client_id < 0)
    #else
    if (client_id < 0)
    #endif
    {
        return false;
    }

    client.socket_id = client_id;
    client.connected = true;
    client.storeRemoteAddressInfo(cli_addr.sin_family, &cli_addr);
    return true;
}

TcpSocket::TcpSocket()
{
    this->connected = false;
    #ifdef _TCPSOCKET_WIN
    _wsa_init();
    #endif
}

TcpSocket::~TcpSocket()
{
    if(IsConnected())
        Close();
}

bool TcpSocket::IsConnected() const
{
    return this->connected;
}

bool TcpSocket::Connect(const char* ip, uint16_t port)
{
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = inet_addr(ip);
    int stat = inet_pton(AF_INET, ip, &address.sin_addr);
    if (stat != 1)
        return false;
    address.sin_port = htons(port);
    return connect(address.sin_family, (const sockaddr*)&address, sizeof(address), 
        SOCK_STREAM, 0);
}

bool TcpSocket::Connect6(const char* ip, uint16_t port)
{
    struct sockaddr_in6 address;

    address.sin6_family = AF_INET6;
    int stat = inet_pton(AF_INET6, ip,  &address.sin6_addr);
    if(stat != 1)
        return false;
    address.sin6_port = htons(port);
    return connect(address.sin6_family, (const sockaddr*)&address, sizeof(address), 
        SOCK_STREAM, 0);
}

bool TcpSocket::Connect(const char* host, const char* serviceName)
{
    addrinfo hints, * p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET | AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    int error = getaddrinfo(host, serviceName, &hints, &p);
    if (error != 0 || p == NULL)
    {
        return false;
    }

    return connect(p->ai_family, p->ai_addr, (int)p->ai_addrlen, p->ai_socktype, p->ai_protocol);
}

bool TcpSocket::connect(int addressFamily, const void* address, int addressLength, int socketType, int protocol)
{
    socket_id = (socket_t)socket(addressFamily, socketType, protocol);
    if (socket_id == -1)
    {
        return false;
    }

    int connectR = ::connect(socket_id, (struct sockaddr *)address, addressLength);
    if (connectR == -1)
    {
        #ifdef _TCPSOCKET_WIN
        closesocket(socket_id);
        #else
        ::close(socket_id);
        #endif
        return false;
    }

    connected = true;
    storeRemoteAddressInfo(addressFamily, address);

    return true;
}

void TcpSocket::storeRemoteAddressInfo(int addressFamily, const void* address)
{
#ifdef _TCPSOCKET_WIN
    this->addressFamily = addressFamily;
    if(addressFamily == AF_INET)
    {
        uint32_t ip = (uint32_t)((sockaddr_in*)address)->sin_addr.S_un.S_addr;
        *this->remoteAddress = ip;
        this->remotePort = ((sockaddr_in*)address)->sin_port;
    }
    else if(addressFamily == AF_INET6)
    {
        memcpy(this->remoteAddress, ((sockaddr_in6*)address)->sin6_addr.u.Byte, 16);
        this->remotePort = ((sockaddr_in6*)address)->sin6_port;
    }
#else
#warning Store Remote Address Info not implemented for non windows
#endif
}

int TcpSocket::Send(const void* data, int length)
{
    if(!connected)
        return 0;
#ifdef _TCPSOCKET_WIN
    int error = ::send(socket_id, (const char*)data, length, 0);
#else
    int error = ::send(socket_id, data, length, 0);
#endif
    if(error == 0)
        connected = false;
    return error;
}

int TcpSocket::Recv(void* data, int bufferSize)
{
    if (!connected)
        return 0;
#ifdef _TCPSOCKET_WIN
    int len = (int)::recv(socket_id, (char*)data, bufferSize, 0);
#else
    int len = (int)::recv(socket_id, data, bufferSize, 0);
#endif
    if (len == 0)
    {
        connected = false;
    }
    return len;
}

int TcpSocket::RecvDontWait(void* data, int bufferSize)
{
    if (!connected)
        return 0;

    int len = GetAvailableData();
    if(len > 0)
        return Recv(data, bufferSize);
    return 0;
}

void TcpSocket::Close()
{
    #ifdef _TCPSOCKET_WIN
    closesocket(socket_id);
    #else
    ::close(socket_id);
    #endif
    connected = false;
}

bool TcpSocket::SendString(const std::string& string)
{
    int res = Send(string.c_str(), (int)string.length());
    return res > 0;
}

bool TcpSocket::SendString(const char* string)
{
    int length = (int)strlen(string);
    int res = Send(string, length);
    return res > 0;
}

std::string TcpSocket::ReadString()
{
    char buffer[1024];
    this->Recv(buffer, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    return std::string(buffer);
}

std::string ResolveDns(const char* domain)
{
    #ifdef _TCPSOCKET_WIN
    _wsa_init();
    #endif
    addrinfo hints, * p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    int error = getaddrinfo(domain, "", &hints, &p);
    if (error != 0)
    {
        return std::string();
    }
    std::string string;
    for(int i = 0; p->ai_addr->sa_data[i] != '\0'; i++)
        string += p->ai_addr->sa_data[i];
    return string;
}

int TcpSocket::GetAvailableData()
{
    if(!connected)
        return -1;
    
    #ifdef _TCPSOCKET_WIN
    unsigned long len;
    ioctlsocket(socket_id, FIONREAD, &len);
    return (int)len;
    #else
    uint8_t *buffer = (uint8_t*)malloc(1024);
    int len = (int)::recv(socket_id, buffer, 1024, MSG_DONTWAIT | MSG_PEEK);
    free(buffer);
    if(len == 0)
        connected = false;
    else if(len == -1)
        return 0;
    return len;
    #endif
}

bool TcpSocket::WSAInit()
{
    #ifdef _TCPSOCKET_WIN
    return _wsa_init() == 0;
    #else
    return true;
    #endif
}

bool TcpSocket::WSAUninit()
{
    #ifdef _TCPSOCKET_WIN
    return _wsa_uninit() == 0;
    #else
    return true;
    #endif
}

bool TcpSocket::ConnectTimeout(const char* ip, uint16_t port, int timeoutMillis)
{
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = inet_addr(ip);
    int stat = inet_pton(AF_INET, ip, &address.sin_addr.s_addr);
    if (stat != 1)
        return false;
    address.sin_port = htons(port);
    return connectTimeout(address.sin_family, (const sockaddr*)&address, sizeof(address), 
        SOCK_STREAM, 0, timeoutMillis);
}

bool TcpSocket::ConnectTimeout(const char* host, const char* serviceName, int timeoutMillis)
{
    addrinfo hints, * p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    int error = getaddrinfo(host, serviceName, &hints, &p);
    if (error != 0 || p == NULL)
    {
        std::cerr << "Cant get address information" << std::endl;
        return false;
    }
    return connectTimeout(p->ai_family, p->ai_addr, (int)p->ai_addrlen, 
        p->ai_socktype, p->ai_protocol, timeoutMillis);
}

bool TcpSocket::connectTimeout(int addressFamily, const void* address, int addresslength, 
    int socketType, int protocol, int timeoutMillis)
{
    struct timeval tv;
    fd_set fdset;
    socket_t sock = (socket_t)socket(addressFamily, socketType, protocol);
    #ifdef _TCPSOCKET_WIN
    u_long nonblocking = 1;
    ioctlsocket(sock, FIONBIO, &nonblocking);
    #else
    fcntl(sock, F_SETFL, O_NONBLOCK);
    #endif

    ::connect(sock, (struct sockaddr *)address, sizeof(sockaddr_in));

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    tv.tv_sec = timeoutMillis / 1000;
    tv.tv_usec = (timeoutMillis % 1000) * 1000;

    if (select((int)sock + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        #ifdef _TCPSOCKET_WIN
        char so_error;
        #else
        int so_error;
        #endif
        socklen_t len = sizeof so_error;

        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0) {
#ifdef _TCPSOCKET_WIN
            nonblocking = 0;
            ioctlsocket(sock, FIONBIO, &nonblocking);
#else
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
#endif
            this->socket_id = sock;
            this->connected = true;
            storeRemoteAddressInfo(addressFamily, address);
            return true;
        }
    }
    return false;
}

TcpAddressFamily TcpSocket::GetRemoteAddressFamily() const
{
    switch(this->addressFamily)
    {
        case AF_INET: return TcpAddressFamily::IpV4;
        case AF_INET6: return TcpAddressFamily::IpV6;
        default: return TcpAddressFamily::Other;
    }
}

std::string TcpSocket::GetRemoteAddress() const
{
    switch(this->addressFamily)
    {
        case AF_INET: 
        {
            std::stringstream ss;
            ss << (int)remoteAddress[0] << "." << (int)remoteAddress[1] << "." 
                << (int)remoteAddress[2] << "." << (int)remoteAddress[3];
            return ss.str();
        }
        case AF_INET6: 
        {
            std::stringstream ss;
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)remoteAddress[0];
            for(int i = 1; i < 16; i++)
            {
                if(i%2==0)
                    ss << ":";
                ss << std::setw(2) << std::setfill('0') << (int)remoteAddress[i];
            }
            return ss.str();
        }
        default: return "";
    }
}

uint16_t TcpSocket::GetRemotePort() const
{
    return this->remotePort;
}

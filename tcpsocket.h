#ifndef _TCPSOCKET_H
#define _TCPSOCKET_H

#include <inttypes.h>
#include <string>

#ifdef WIN32
#define _TCPSOCKET_WIN
#endif
#ifdef WINX64
#define _TCPSOCKET_WIN
#endif

#ifdef _TCPSOCKET_WIN
#pragma comment(lib, "Ws2_32.lib")
#endif

typedef int64_t socket_t;

enum TcpAddressFamily
{
    IpV4 = 1,
    IpV6 = 2,
    Other = 0
};

class TcpSocket
{
    public:
        TcpSocket();
        ~TcpSocket();
        bool IsConnected() const;
        bool Connect(const char* ip, uint16_t port);
        bool Connect6(const char* ip, uint16_t port);
        bool Connect(const char* hostname, const char* serviceName);
        bool ConnectTimeout(const char* ip, uint16_t port, int timeoutMillis);
        bool ConnectTimeout(const char* hostname, const char* serviceName, int timeoutMillis);
        void Close();
        int Send(const void* data, int length);
        int Recv(void* data, int bufferSize);
        int RecvDontWait(void* data, int bufferSize);
        bool SendString(const std::string& string);
        bool SendString(const char* string);
        std::string ReadString();
        int GetAvailableData();
        TcpAddressFamily GetRemoteAddressFamily() const;
        std::string GetRemoteAddress() const;
        uint16_t GetRemotePort() const;

        static std::string ResolveDns(const char* domain);
        static bool WSAInit();
        static bool WSAUninit();

    private:
        friend class TcpListener;
        bool connect(int addressFamily, const void* address, int addressLength, int socketType, int protocol);
        bool connectTimeout(int addressFamily, const void* address, int addresslength, int socketType, int protocol, int timeoutMillis);
        void storeRemoteAddressInfo(int addressFamily, const void* address);

        socket_t socket_id;
        bool connected;
        int addressFamily;
        uint8_t remoteAddress[16];
        uint16_t remotePort;
};

class TcpListener
{
    public:
        TcpListener();
        ~TcpListener();
        bool Start(uint16_t port);
        bool Start(uint16_t port, bool loopback);
        bool Close();
        bool IsActive() const;
        bool AcceptClient(TcpSocket& client) const;
    
    private:
        socket_t socket_id;
        bool active;
};

#endif

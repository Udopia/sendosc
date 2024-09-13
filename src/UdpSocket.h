/*************************************************************************************************
sendosc -- Copyright (c) 2021, Markus Iser, KIT - Karlsruhe Institute of Technology

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************************/

#ifndef SRC_UDPSOCKET_H_
#define SRC_UDPSOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <cstring>

namespace OSC {

/**
 * @brief Creates a UDP Socket for Sending
 * Throws runtime_error objects to signal fail states
 */
class UdpSocket {
    int sockfd;

 public:
    UdpSocket(const char* address, uint16_t port) : sockfd(-1) {
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (!inet_aton(address, &addr.sin_addr)) {
            throw std::runtime_error("unable to convert inet address\n");
        }

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            throw std::runtime_error("unable to create socket\n");
        }
        
        // ignore ICMP port unreachable messages
        int rcvbuf = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("unable to connect\n");
        }
    }

    ~UdpSocket() {
        if (sockfd != -1) close(sockfd);
    }

    void Send(const char *data, std::size_t size) {
        if (send(sockfd, data, size, 0) == -1) {
            switch (errno) {
                case ECONNREFUSED:
                    // throw std::runtime_error("The target address is not listening.\n");
                    std::cerr << "The target address is not listening." << std::endl;
                    break;
                case EACCES:
                    throw std::runtime_error("An attempt was made to send to a network/broadcast address as though it was a unicast address.\n");
                case EAGAIN:
                    throw std::runtime_error("The socket is marked nonblocking and the requested operation would block.\n");
                case EBADF:
                    throw std::runtime_error("An invalid descriptor was specified.\n");
                case ECONNRESET:
                    throw std::runtime_error("Connection reset by peer.\n");
                case EDESTADDRREQ:
                    throw std::runtime_error("The socket is not connection-mode, and no peer address is set.\n");
                case EFAULT:
                    throw std::runtime_error("An invalid user space address was specified for an argument.\n");
                case EINTR:
                    throw std::runtime_error("A signal occurred before any data was transmitted.\n");
                case EINVAL:
                    throw std::runtime_error("Invalid argument passed.\n");
                case EISCONN:
                    throw std::runtime_error("The connection-mode socket was connected already but a recipient was specified.\n");
                case EMSGSIZE:
                    throw std::runtime_error("The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible.\n");
                case ENOBUFS:
                    throw std::runtime_error("The output queue for a network interface was full.\n");
                case ENOMEM:
                    throw std::runtime_error("No memory available.\n");
                case ENOTCONN:
                    throw std::runtime_error("The socket is not connected, and no target has been given.\n");
                case ENOTSOCK:
                    throw std::runtime_error("The argument sockfd is not a socket.\n");
                case EOPNOTSUPP:
                    throw std::runtime_error("Some bit in the flags argument is inappropriate for the socket type.\n");
                case EPIPE:
                    throw std::runtime_error("The local end has been shut down on a connection oriented socket.\n");
                default:
                    throw std::runtime_error("Sending failed for unknown reasons: " + std::to_string(errno) + "\n");
            }
        }
    }
};

}  // namespace OSC

#endif  // SRC_UDPSOCKET_H_

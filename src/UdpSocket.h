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

#include <stdexcept>
#include <cstring>

namespace OSC {

/**
 * @brief Creates a UDP Socket for Sending
 * Throws runtime_error objects to signal fail states
 */
class UdpSocket {
    int socket_;

 public:
    UdpSocket(const char* address, uint16_t port) : socket_(-1) {
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (!inet_aton(address, &addr.sin_addr)) {
            throw std::runtime_error("unable to convert inet address\n");
        }

        if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            throw std::runtime_error("unable to create socket\n");
        }

        if (connect(socket_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("unable to connect\n");
        }
    }

    ~UdpSocket() {
        if (socket_ != -1) close(socket_);
    }

    void Send(const char *data, std::size_t size) {
        if (send(socket_, data, size, 0) == -1) {
            throw std::runtime_error("sending failed\n");
        }
    }
};

}  // namespace OSC

#endif  // SRC_UDPSOCKET_H_

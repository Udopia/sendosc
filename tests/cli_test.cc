#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int run(const char* executable, const std::vector<std::string>& arguments) {
    std::vector<char*> argv{const_cast<char*>(executable)};
    for (const auto& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    argv.push_back(nullptr);
    const pid_t child = fork();
    require(child >= 0, "fork failed");
    if (child == 0) {
        execv(executable, argv.data());
        std::cerr << "execv failed: " << std::strerror(errno) << '\n';
        _exit(127);
    }
    int status = 0;
    pid_t result;
    do {
        result = waitpid(child, &status, 0);
    } while (result == -1 && errno == EINTR);
    require(result == child && WIFEXITED(status), "child did not exit normally");
    return WEXITSTATUS(status);
}

void append_int(std::vector<char>& bytes, uint32_t value) {
    const uint32_t network = htonl(value);
    const char* data = reinterpret_cast<const char*>(&network);
    bytes.insert(bytes.end(), data, data + sizeof(network));
}

void append_string(std::vector<char>& bytes, const std::string& value) {
    bytes.insert(bytes.end(), value.begin(), value.end());
    bytes.push_back('\0');
    while (bytes.size() % 4 != 0) bytes.push_back('\0');
}

void append_message(std::vector<char>& bundle, const std::string& address,
                    const std::string& type, const std::vector<char>& value) {
    std::vector<char> message;
    append_string(message, address);
    append_string(message, "," + type);
    message.insert(message.end(), value.begin(), value.end());
    append_int(bundle, message.size());
    bundle.insert(bundle.end(), message.begin(), message.end());
}

std::vector<char> receive(int socket) {
    std::vector<char> packet(4096);
    const ssize_t size = recv(socket, packet.data(), packet.size(), 0);
    require(size >= 16, "expected UDP bundle");
    packet.resize(size);
    require(std::memcmp(packet.data(), "#bundle\0", 8) == 0, "invalid bundle header");
    return std::vector<char>(packet.begin() + 16, packet.end());
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) return 1;
    const int socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket < 0) return 1;
    try {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        require(bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0,
                "bind failed");
        socklen_t length = sizeof(address);
        require(getsockname(socket, reinterpret_cast<sockaddr*>(&address), &length) == 0,
                "getsockname failed");
        const timeval timeout{1, 0};
        require(setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0,
                "setsockopt failed");
        const std::string port = std::to_string(ntohs(address.sin_port));

        require(run(argv[1], {"127.0.0.1", port, "/program/state", "i", "1"}) == 0,
                "example command failed");
        std::vector<char> one;
        std::vector<char> integer;
        append_int(integer, 1);
        append_message(one, "/program/state", "i", integer);
        require(receive(socket) == one, "example packet mismatch");

        require(run(argv[1], {"127.0.0.1", port, "/int", "i", "-2147483648",
                             "/float", "f", "-1.25e0", "/text", "s", "hello world",
                             "/empty", "s", ""}) == 0, "multiple triples failed");
        std::vector<char> expected;
        integer.clear();
        append_int(integer, 0x80000000u);
        append_message(expected, "/int", "i", integer);
        std::vector<char> floating;
        append_int(floating, 0xbfa00000u);
        append_message(expected, "/float", "f", floating);
        std::vector<char> text;
        append_string(text, "hello world");
        append_message(expected, "/text", "s", text);
        text.clear();
        append_string(text, "");
        append_message(expected, "/empty", "s", text);
        require(receive(socket) == expected, "multiple-triple packet mismatch");

        const std::vector<std::vector<std::string>> invalid{
            {"127.0.0.1", port}, {"127.0.0.1", port, "/x", "i"},
            {"127.0.0.1", "0", "/x", "i", "1"},
            {"127.0.0.1", "65536", "/x", "i", "1"},
            {"127.0.0.1", "123abc", "/x", "i", "1"},
            {"invalid-ip", port, "/x", "i", "1"},
            {"127.0.0.1", port, "x", "i", "1"},
            {"127.0.0.1", port, "", "i", "1"},
            {"127.0.0.1", port, "/x", "unknown", "1"},
            {"127.0.0.1", port, "/x", "i", "2147483648"},
            {"127.0.0.1", port, "/x", "i", "-2147483649"},
            {"127.0.0.1", port, "/x", "i", "1x"},
            {"127.0.0.1", port, "/x", "i", ""},
            {"127.0.0.1", port, "/x", "f", "1.2x"},
            {"127.0.0.1", port, "/x", "f", "nan"},
            {"127.0.0.1", port, "/x", "f", "inf"},
            {"127.0.0.1", port, "/x", "f", "1e100"},
            {"127.0.0.1", port, "/ok", "i", "1", "/bad", "i", "no"},
            {"127.0.0.1", port, "/ok", "i", "1", "/big", "s", std::string(2048, 'x')}
        };
        for (const auto& arguments : invalid) {
            require(run(argv[1], arguments) == 1, "invalid command did not fail");
        }
        require(run(argv[1], {"--help"}) == 0, "help failed");
        char byte;
        require(recv(socket, &byte, 1, MSG_DONTWAIT) == -1 &&
                (errno == EAGAIN || errno == EWOULDBLOCK),
                "invalid command or help sent a packet");
        close(socket);
        return 0;
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << '\n';
        close(socket);
        return 1;
    }
}

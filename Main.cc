/*************************************************************************************************
sendosc -- Copyright (c) 2021, Ashlin Iser, KIT - Karlsruhe Institute of Technology

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

#include "sendosc/sendosc.h"

#include <charconv>
#include <cmath>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

void usage(std::ostream& out, const char* program) {
    out << "Usage: " << program << " [IP PORT ADDRESS TYPE VALUE [ADDRESS TYPE VALUE ...]]\n"
        << "No arguments: send /test with string abrakadabra and integer 42 to 127.0.0.1:5000\n"
        << "Types: i = signed 32-bit integer, f = finite float, s = string\n"
        << "Example: " << program << " 127.0.0.1 57120 /program/state i 1\n";
}

template<typename T>
T parse_number(std::string_view text, const char* label) {
    T value{};
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        throw std::runtime_error(std::string("invalid ") + label + ": " + std::string(text));
    }
    return value;
}

struct Message {
    const char* address;
    std::variant<OSC::Int, OSC::Float, OSC::String> value;
};

}  // namespace

int main(int argc, const char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        usage(std::cout, argv[0]);
        return 0;
    }
    if (argc != 1 && (argc < 6 || (argc - 3) % 3 != 0)) {
        usage(std::cerr, argv[0]);
        return 1;
    }

    try {
        if (argc == 1) {
            OSC::Stream osc("127.0.0.1", 5000);
            osc << OSC::Message("/test") << OSC::String("abrakadabra") << OSC::Int(42) << OSC::Flush();
            return 0;
        }
        const int port = parse_number<int>(argv[2], "port");
        if (port < 1 || port > 65535) {
            throw std::runtime_error("port must be between 1 and 65535");
        }

        std::vector<Message> messages;
        for (int index = 3; index < argc; index += 3) {
            const char* address = argv[index];
            if (address[0] != '/') {
                throw std::runtime_error("OSC address must start with '/': " + std::string(address));
            }
            const std::string_view type(argv[index + 1]);
            const char* value = argv[index + 2];
            if (type == "i") {
                messages.push_back({address, OSC::Int(parse_number<int32_t>(value, "integer"))});
            } else if (type == "f") {
                const float number = parse_number<float>(value, "float");
                if (!std::isfinite(number)) {
                    throw std::runtime_error("float must be finite: " + std::string(value));
                }
                messages.push_back({address, OSC::Float(number)});
            } else if (type == "s") {
                messages.push_back({address, OSC::String(value)});
            } else {
                throw std::runtime_error("unsupported OSC type: " + std::string(type));
            }
        }

        OSC::Stream osc(argv[1], port);
        for (const auto& message : messages) {
            osc << OSC::Message(message.address);
            std::visit([&osc](const auto& value) { osc << value; }, message.value);
        }
        osc << OSC::Flush();
    } catch (const std::runtime_error& error) {
        std::cerr << "sendosc: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

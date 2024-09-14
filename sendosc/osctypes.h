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

#ifndef SRC_OSCTYPES_H_
#define SRC_OSCTYPES_H_

#include <bit>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <byteswap.h>


namespace OSC {

/**
 * @brief Generic OSC data type with default implementations for inheritance by concrete types
 * @tparam Q internal type
 * @tparam S type size
 * @tparam T type char
 */
template<typename Q = void, unsigned S = 0, char T = '\0'>
struct AbstractType {
    // @brief Type indicator character used in message serialization
    char type() { return T; }

    // @brief Size of serialized object
    unsigned size() { return S; }

    // @brief Serializes object into given buffer
    void serialize(char*) { }
};

/////////////////
// Control Types

// @brief Start a new message, finalize previous message
struct Message : AbstractType<> {
    const char* uri_;
    explicit Message(const char* uri) : uri_(uri) {}
};

// @brief Send all messages in current bundle and start a new bundle
struct Flush : AbstractType<> {};

/////////////////
// Impulse Types
struct True : AbstractType<void, 0, 'T'> { };
struct False : AbstractType<void, 0, 'F'> { };
struct Null : AbstractType<void, 0, 'N'> { };
struct Impulse : AbstractType<void, 0, 'I'> { };

/////////////////
// Numeric Types

// @brief Base class for Int, Time, and Float
template<typename Q, unsigned S = sizeof(Q), char T = '\0'>
struct NumericType : AbstractType<Q, S, T> {
    Q n_;

    explicit NumericType(Q n) : n_(n) {}

    void serialize(char* buffer) {
        Q value = n_;
        if constexpr (std::endian::native == std::endian::little) {
            if (S == 4) value = bswap_32(value);
            if (S == 8) value = bswap_64(value);
        }
        memcpy(buffer, &value, S);
    }
};

struct Int : NumericType<int32_t, sizeof(int32_t), 'i'> {
    using NumericType::NumericType;
};

struct Float : NumericType<int32_t, sizeof(int32_t), 'f'> {
    explicit Float(float n) : NumericType(0) {
        std::memcpy(&n_, &n, sizeof(float));
    }
};

struct Time : NumericType<uint64_t, sizeof(uint64_t), 't'> {
    Time() : NumericType(0) {
        const uint64_t NTP_EPOCH = 2208988800ULL;  // Seconds between 1900 and 1970 (Unix epoch)
        auto now = std::chrono::system_clock::now().time_since_epoch();
        uint64_t secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(now).count();
        uint64_t nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count() % 1000000000;
        uint32_t ntp_seconds = static_cast<uint32_t>(secondsSinceEpoch + NTP_EPOCH);
        uint32_t ntp_fraction = static_cast<uint32_t>((nanoseconds * (1LL << 32)) / 1000000000);
        n_ = ((uint64_t)ntp_seconds << 32) | ntp_fraction;
    }
};

/////////////////
// Array Types

// @brief Base class for String and Blob
template<typename Q, unsigned S = 0, char T = '\0'>
struct ArrayType : AbstractType<Q, S, T> {
    const Q* a_;
    int32_t size_;

    ArrayType(const Q* a, int32_t size) : a_(a), size_(size) {}
    explicit ArrayType(const Q* a) : ArrayType(a, 0) {}

    // @brief Round length to next multiple of four
    unsigned size() {
        return (size_ + 3) & ~0x3;
    }
};

struct String : ArrayType<char, 0, 's'> {
    String(const char* a, int32_t size) : ArrayType(a, size) {}
    explicit String(const char* a) : String(a, std::strlen(a) + 1) {}

    void serialize(char* buffer) {
        strncpy(buffer, a_, size_);
        memset(buffer + size_, '\0', size() - size_);
    }
};

struct Blob : ArrayType<void, 0, 'b'> {
    Blob(const void* a, int32_t size) : ArrayType(a, size) {}

    void serialize(char* buffer) {
        auto s = Int(size_);
        s.serialize(buffer);
        memcpy(buffer + s.size(), a_, size_);
    }
};


}  // namespace OSC

#endif  // SRC_OSCTYPES_H_

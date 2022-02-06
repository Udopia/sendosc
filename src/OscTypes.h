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

/**
 * @brief Namespace of OSC Data Stream and Structs
 */
namespace OSC {

/**
 * @brief Default Implementations for OSC Type Methods
 * Default implementations for inheritance by OSC Types
 * 
 * @tparam S type size
 * @tparam T type char
 */
template<unsigned S = 0, char T = '\0'>
struct AbstractType {
    /**
     * @brief Type indicator character
     * Used in message serialization
     * @return char Type indicator (default: '\0')
     */
    char type() { return T; }

    /**
     * @brief Size of serialized content
     * @return unsigned Number of bytes occupied by serialized content (default: 0)
     */
    unsigned size() { return S; }

    /**
     * @brief Serialize content into given buffer
     * @param buffer
     */
    void serialize(char*) { }
};


/**
 * @brief Base class for Int, Float and Time
 * 
 * @tparam Q internal type
 * @tparam S type size
 * @tparam T type char
 */
template<typename Q, unsigned S = sizeof(Q), char T = '\0'>
struct NumericType : AbstractType<S, T> {
    Q n_;

    explicit NumericType(Q n) : n_(n) {}

    /**
     * @brief Serialize numeric content into given buffer
     * Conditional conversion of endianness
     * Thanks to oHo at https://stackoverflow.com/questions/809902/64-bit-ntohl-in-c
     * @param buffer
     */
    void serialize(char* buffer) {
        Q value = n_;
        if constexpr (std::endian::native == std::endian::little) {
            char* ptr = reinterpret_cast<char*>(&value);
            std::reverse(ptr, ptr + S);
        }
        memcpy(buffer, &value, S);
    }
};


/**
 * @brief Base class for String and Blob
 * 
 * @tparam Q internal type
 * @tparam S type size
 * @tparam T type char
 */
template<typename Q, unsigned S = 0, char T = '\0'>
struct ArrayType : AbstractType<S, T> {
    const Q* a_;
    int32_t size_;

    ArrayType(const Q* a, int32_t size) : a_(a), size_(size) {}
    explicit ArrayType(const Q* a) : ArrayType(a, 0) {}

    /**
     * @brief Round length to next multiple of four
     * Thanks to Henry S. Warren Jr. in Hacker's Delight
     */
    unsigned size() {
        return (size_ + 3) & ~0x3;
    }
};


/**
 * @brief Start a new message
 * Finalize previous message
 */
struct Message : AbstractType<> {
    const char* uri_;
    explicit Message(const char* uri) : uri_(uri) {}
};

/**
 * @brief Send all messages in current bundle and start a new bundle
 * Also finalizes current message
 */
struct Flush : AbstractType<> {};


// Concrete Impulse Types:

struct True : AbstractType<0, 'T'> { };

struct False : AbstractType<0, 'F'> { };

struct Null : AbstractType<0, 'N'> { };

struct Impulse : AbstractType<0, 'I'> { };


// Concrete Numeric Types:

struct Int : NumericType<int32_t, 4, 'i'> {
    using NumericType::NumericType;  // inherit constructor
};

struct Float : NumericType<float, 4, 'f'> {
    using NumericType::NumericType;  // inherit constructor
};

struct Time : NumericType<uint64_t, 8, 't'> {
    Time() : NumericType(0) {  // time since first use of Time()
        using std::chrono::system_clock, std::chrono::nanoseconds, std::chrono::duration_cast;
        static system_clock::time_point start {
            system_clock::now()
        };
        int64_t nano = duration_cast<nanoseconds>(system_clock::now() - start).count();
        int64_t sec = nano / 1e+9;
        n_ = (sec << 32) + (nano - (sec * 1e+9));
    }

    explicit Time(uint64_t n) : NumericType(n) {}
};


// Concrete Array Types:

struct String : ArrayType<char, 0, 's'> {
    String(const char* a, int32_t size) : ArrayType(a, size) {}
    explicit String(const char* a) : String(a, std::strlen(a) + 1) {}

    void serialize(char* buffer) {
        strncpy(buffer, a_, size_);
    }
};

struct Blob : ArrayType<void, 0, 'b'> {
    Blob(const void* a, int32_t size) : ArrayType(a, size + Int(size).size()) {}

    void serialize(char* buffer) {
        Int(size_).serialize(buffer);
        memcpy(buffer + Int(size_).size(), a_, size_ - Int(size_).size());
    }
};


}  // namespace OSC

#endif  // SRC_OSCTYPES_H_

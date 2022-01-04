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

#ifndef SRC_OSCSTREAM_H_
#define SRC_OSCSTREAM_H_

#include "UdpSocket.h"
#include "OscTypes.h"

/**
 * @brief Namespace of OSC Data Stream and Structs
 */
namespace OSC {

/**
 * @brief OSC Stream Object
 * Creates UDP Socket for given Address and Port
 * Sends incoming OSC Structs via UDP Socket
 */
class Stream {
    char* data_;  // data begin
    char* dend_;  // data end
    char* type_;  // type begin
    char* tend_;  // type end

    char* dcursor_;  // data cursor
    char* tcursor_;  // type cursor

    char* mstart_;  // pointer to start of current message

    UdpSocket sock_;

 public:
    Stream(const char* address, int port, unsigned capacity = 2048) : sock_(address, port) {
        data_ = new char[capacity]();
        type_ = new char[capacity]();
        dend_ = data_ + capacity;
        tend_ = type_ + capacity;
        mstart_ = nullptr;
        clear_type();
        clear_data();
    }

    ~Stream() {
        flush();
        delete[] data_;
        delete[] type_;
    }

    /**
     * @brief Intialize type data
     */
    void clear_type() {
        tcursor_ = type_;
        std::fill(type_, tend_, '\0');
        *tcursor_ = ',';
        ++tcursor_;
    }

    /**
     * @brief Initialize bundle header
     */
    void clear_data() {
        dcursor_ = data_;
        std::fill(data_, dend_, '\0');
        write(String("#bundle"));
        write(Time());
    }

    /**
     * @brief Finalize Message (if possible)
     * Serialize Message Length
     * Insert type string between message uri and message data
     */
    void finalize() {
        if (mstart_ != nullptr) {
            unsigned offset = 4 + String(mstart_ + 4).size();  // size bytes + uri size
            unsigned len = static_cast<int32_t>(dcursor_ - mstart_) - offset;
            unsigned tlen = String(type_).size();
            if (len > 4) {
                Int(len - 4).serialize(mstart_);  // write the first four bytes (= message length)
                std::memmove(mstart_ + offset + tlen, mstart_ + offset, len);  // create space for type info
                dcursor_ += tlen;
            }
            std::memcpy(mstart_ + offset, type_, tlen);  // copy type info
            clear_type();
            mstart_ = nullptr;
        }
    }

    /**
     * @brief Initialize New Message
     * Finalize Previous Message (if necessary)
     * Initialize Message Length to 0
     * Initialize Message URI
     * 
     * @param uri Message URI
     */
    void message(const char* uri) {
        finalize();
        mstart_ = dcursor_;
        write(Int(0));
        write(String(uri));
    }

    /**
     * @brief Flush Message Buffer to Socket
     * Finalize Previous Message (if necessary)
     * Send OSC Bundle
     * Initialize Next OSC Bundle 
     * The Bundle Timestamp encodes the time passed since first flush
     */
    void flush() {
        finalize();
        if (dcursor_ > data_ + 16) {
            sock_.Send(data_, dcursor_ - data_);
            clear_data();
        }
    }

    /**
     * @brief Serializes OSC Structs
     * Serializes type info and shifts the type cursor (if activated)
     * Serializes data fields and shifts the data cursor
     * 
     * @tparam T OSC Struct Type
     * @param msg OSC Struct Object
     * @param write_type activates serialization of type info (default: false)
     */
    template<typename T>
    void write(T msg, bool write_type = false) {
        if (dcursor_ + msg.size() + String(type_).size() >= dend_) {
            throw std::runtime_error("out of buffer memory");
        }
        if (write_type) {
            *tcursor_ = msg.type();
            ++tcursor_;
        }
        msg.serialize(dcursor_);
        dcursor_ += msg.size();
    }

    /**
     * @brief Stream Operator for most OSC Structs
     * 
     * @tparam T OSC Struct Type
     * @param msg OSC Struct Object
     * @return Stream& *this
     */
    template<typename T>
    Stream& operator<<(T msg) {
        write(msg, true);
        return *this;
    }

    /**
     * @brief Stream Operator for Flush Triggers
     * 
     * @param msg Flush Object
     * @return Stream& *this
     */
    Stream& operator<<(const Flush& msg) {
        flush();
        return *this;
    }

    /**
     * @brief Stream Operator for Message Triggers
     * 
     * @param msg Message Object
     * @return Stream& *this
     */
    Stream& operator<<(const Message& msg) {
        message(msg.uri_);
        return *this;
    }
};

}  // namespace OSC

#endif  // SRC_OSCSTREAM_H_

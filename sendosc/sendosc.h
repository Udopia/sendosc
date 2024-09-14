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

#include <cassert>

#include "udpsocket.h"
#include "osctypes.h"

/**
 * This implementation of OSC handles one bundle of messages at a time,
 * on flush it sends the bundle via UDP Socket, and starts a new bundle.
 */
namespace OSC {

/**
 * @brief OSC stream object, creates an UDP socket, 
 * and sends bundles of OSC messages
 */
class Stream {
    // data buffer: begin, end, and cursor
    char* data_;
    char* dend_;
    char* dcursor_;

    // type buffer: begin, end, and cursor
    char* type_;
    char* tend_;
    char* tcursor_;

    // pointer to start of current message
    char* mstart_;

    UDPsock sock_;

 public:
    Stream(const char* address, int port, unsigned capacity = 2048) : sock_(address, port) {
        data_ = new char[capacity]();
        type_ = new char[capacity]();
        dend_ = data_ + capacity;
        tend_ = type_ + capacity;
        init_fresh_bundle();
        init_type_buffer();
    }

    ~Stream() {
        delete[] data_;
        delete[] type_;
    }

    /**
     * @brief Initialize bundle header
     */
    void init_fresh_bundle() {
        std::fill(data_, dend_, '\0');
        dcursor_ = data_;
        write(String("#bundle", 8));
        write(Time());
        mstart_ = dcursor_;
    }

    void init_type_buffer() {
        std::fill(type_, tend_, '\0');
        tcursor_ = type_;
        *tcursor_ = ',';
        ++tcursor_;
    }

    /**
     * @brief Flush bundle and initialize next bundle
     */
    void flush_bundle() {
        end_current_message();
        if (dcursor_ > data_ + 16) {
            std::cerr << "Sending " << dcursor_ - data_ << " bytes" << std::endl;
            std::cerr << "Data: " << data_ << std::endl;
            sock_.Send(data_, dcursor_ - data_);
            init_fresh_bundle();
        }
        else {
            std::cerr << "Warning: suppressing attempt to send empty bundle" << std::endl;
        }
    }

    /**
     * @brief Initialize next message header with length and uri
     * @param uri Message URI
     */
    void start_message(const char* uri) {
        end_current_message();
        mstart_ = dcursor_;
        write(Int(0));
        write(String(uri));
    }

    /**
     * @brief Serialize message: insert type info and message size
     */
    void end_current_message() {
        // if message is not empty, write type info and message size
        if (strlen(type_) > 1) {
            // write type info to header
            auto type_info = String(type_);
            unsigned header_size = 4 + String(mstart_ + 4).size();
            unsigned content_size = dcursor_ - mstart_ - header_size;
            // move data to make space for type info, then copy type info, and advance cursor
            std::memmove(mstart_ + header_size + type_info.size(), mstart_ + header_size, content_size);
            type_info.serialize(mstart_ + header_size);
            dcursor_ += type_info.size();
            // write message size to header
            int32_t message_size = static_cast<int32_t>(dcursor_ - mstart_);
            Int(message_size - 4).serialize(mstart_);
            // reset type buffer to make sure this block is not executed twice
            init_type_buffer();
        }
        // else reset cursor to start of message
        else {
            std::fill(mstart_, dcursor_, '\0');
            dcursor_ = mstart_;
        }
    }

    /**
     * @brief Serialize OSC datatypes, write type info and data fields
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
     * @brief Stream operator for serializing OSC types
     * @tparam T OSC type
     * @param msg OSC object
     * @return Stream& *this
     */
    template<typename T>
    Stream& operator<<(T msg) {
        write(msg, true);
        return *this;
    }

    /**
     * @brief Stream operator for flushing the current message bundle
     * @param msg Flush type object
     * @return Stream& *this
     */
    Stream& operator<<(const Flush&) {
        flush_bundle();
        return *this;
    }

    /**
     * @brief Stream operator for starting a new message
     * @param msg Message type object
     * @return Stream& *this
     */
    Stream& operator<<(const Message& msg) {
        start_message(msg.uri_);
        return *this;
    }
};

}  // namespace OSC

#endif  // SRC_OSCSTREAM_H_

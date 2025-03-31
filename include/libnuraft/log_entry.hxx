/************************************************************************
Modifications Copyright 2017-2019 eBay Inc.
Author/Developer(s): Jung-Sang Ahn

Original Copyright:
See URL: https://github.com/datatechnology/cornerstone

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#ifndef _LOG_ENTRY_HXX_
#define _LOG_ENTRY_HXX_

#include "basic_types.hxx"
#include "buffer.hxx"
#include "buffer_serializer.hxx"
#include "log_val_type.hxx"
#include "ptr.hxx"

#include <iostream>

#ifdef _NO_EXCEPTION
#include <cassert>
#endif
#include <stdexcept>

namespace nuraft {

class log_entry {
public:
    log_entry(ulong term,
              const ptr<buffer>& buff,
              log_val_type value_type = log_val_type::app_log,
              uint64_t log_timestamp = 0)
        : term_(term)
        , value_type_(value_type)
        , buff_(buff)
        , timestamp_us_(log_timestamp) {}

    __nocopy__(log_entry);

public:
    ulong get_term() const { return term_; }

    void set_term(ulong term) { term_ = term; }

    log_val_type get_val_type() const { return value_type_; }

    bool is_buf_null() const { return (buff_.get()) ? false : true; }

    buffer& get_buf() const {
        // We accept nil buffer, but in that case,
        // the get_buf() shouldn't be called, throw runtime exception
        // instead of having segment fault (AV on Windows)
        if (!buff_) {
#ifndef _NO_EXCEPTION
            throw std::runtime_error("get_buf cannot be called for a log_entry "
                                     "with nil buffer");
#else
            assert(0);
#endif
        }

        return *buff_;
    }

    ptr<buffer> get_buf_ptr() const { return buff_; }

    uint64_t get_timestamp() const { return timestamp_us_; }

    void set_timestamp(uint64_t t) { timestamp_us_ = t; }

    //! FORENSICS: serialize prev_pointer and signature
    ptr<buffer> serialize(bool include_timestamp = false) {
        buff_->pos(0);
        ptr<buffer> buf = buffer::alloc(sizeof(ulong) + sizeof(char) + sizeof(int32) + buff_->size());
        buf->put(term_);
        buf->put((static_cast<byte>(value_type_)));
        if (include_timestamp) {
            buf->put(timestamp_us_);
        }

        buf->put((int32)buff_->size());
        buf->put(*buff_);
        buf->pos(0);
        return buf;
    }

    //! FORENSICS: deserialize prev_pointer and signature
    static ptr<log_entry> deserialize(buffer& buf, bool include_timestamp = false) {
        size_t field_size = sizeof(ulong) + sizeof(byte) + sizeof(int32);
        if (include_timestamp) {
            field_size += sizeof(uint64_t);
        }
        if (buf.size() - buf.pos() < field_size) {
            return nullptr; // not enough data to read
        }

        int pos = buf.pos();

        ulong term = buf.get_ulong();
        log_val_type t = static_cast<log_val_type>(buf.get_byte());
        uint64_t timestamp = include_timestamp ? buf.get_ulong() : 0ul;
        size_t data_size = buf.get_int();
        if (buf.size() - buf.pos() < data_size) {
            buf.pos(pos);
            return nullptr; // not enough data to read
        }
        ptr<buffer> data = buffer::alloc(data_size);
        buf.get(data);
        return cs_new<log_entry>(term, data, t, timestamp);
    }

    static ptr<log_entry> deserialize(buffer_serializer& ss, bool include_timestamp = false) {
        size_t field_size = sizeof(ulong) + sizeof(byte) + sizeof(int32);
        if (include_timestamp) {
            field_size += sizeof(uint64_t);
        }
        if ((int64_t)ss.size() - (int64_t)ss.pos() < (int64_t)field_size) {
            std::cerr << "At " << ss.pos() << ", remaining " << ss.size() << ", at least " << field_size << std::endl;
            return nullptr; // not enough data to read
        }

        int pos = ss.pos();

        ulong term = ss.get_u64();
        log_val_type t = static_cast<log_val_type>(ss.get_u8());
        uint64_t timestamp = include_timestamp ? ss.get_u64() : 0ul;
        int data_size = ss.get_i32();
        if ((int64_t)ss.size() - (int64_t)ss.pos() < (int64_t)data_size) {
            std::cerr << "At " << ss.pos() << ", remaining " << ss.size() << ", expecting " << data_size << std::endl;
            ss.pos(pos);
            return nullptr; // not enough data to read
        }
        ptr<buffer> data = buffer::alloc(data_size);
        ss.get_buffer(data);
        return cs_new<log_entry>(term, data, t, timestamp);
    }

    static ulong term_in_buffer(buffer& buf) {
        ulong term = buf.get_ulong();
        buf.pos(0); // reset the position
        return term;
    }

private:
    /**
     * The term number when this log entry was generated.
     */
    ulong term_;

    /**
     * Type of this log entry.
     */
    log_val_type value_type_;

    /**
     * Actual data that this log entry carries.
     */
    ptr<buffer> buff_;

    /**
     * The timestamp (since epoch) when this log entry was generated
     * in microseconds. Used only when `log_entry_timestamp_` in
     * `asio_service_options` is set.
     */
    uint64_t timestamp_us_;
};

} // namespace nuraft

#endif //_LOG_ENTRY_HXX_

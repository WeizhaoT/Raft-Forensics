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

#ifndef _REG_MSG_HXX_
#define _REG_MSG_HXX_

#include "certificate.hxx"
#include "log_entry.hxx"
#include "msg_base.hxx"

#include <vector>

namespace nuraft {

class req_msg : public msg_base {
public:
    //! FORENSICS: Include certificate
    req_msg(ulong term,
            msg_type type,
            int32 src,
            int32 dst,
            ulong last_log_term,
            ulong last_log_idx,
            ulong commit_idx)
        : msg_base(term, type, src, dst)
        , last_log_term_(last_log_term)
        , last_log_idx_(last_log_idx)
        , commit_idx_(commit_idx)
        , log_entries_()
        , cc_() {}

    virtual ~req_msg() __override__ {}

    __nocopy__(req_msg);

public:
    ulong get_last_log_idx() const { return last_log_idx_; }

    ulong get_last_log_term() const { return last_log_term_; }

    ulong get_commit_idx() const { return commit_idx_; }

    std::vector<ptr<log_entry>>& log_entries() { return log_entries_; }

    //! FORENSICS: BEGIN
    void set_certificate(ptr<certificate> cc) { cc_ = cc; }

    ptr<certificate> get_certificate() { return cc_; }

    ptr<buffer> serialize() {
        size_t total_size =
            sizeof(ulong) * 3 + sizeof(ulong) + sizeof(msg_type) + sizeof(int32);
        ptr<buffer> buf = buffer::alloc(total_size);
        buf->put(last_log_term_);
        buf->put(last_log_idx_);
        buf->put(commit_idx_);
        buf->put(msg_base::get_term());
        buf->put(msg_base::get_type());
        buf->put(msg_base::get_src());
        buf->pos(0);
        return buf;
    }

    static ptr<req_msg> deserialize(buffer& buf) {
        ulong last_log_term = buf.get_ulong();
        ulong last_log_idx = buf.get_ulong();
        ulong commit_idx = buf.get_ulong();
        ulong term = buf.get_ulong();
        msg_type type = static_cast<msg_type>(buf.get_byte());
        int32 src = buf.get_int();
        return cs_new<req_msg>(
            term, type, src, 0, last_log_term, last_log_idx, commit_idx);
    }
    //! FORENSICS: END

private:
    // Term of last log below.
    ulong last_log_term_;

    // Last log index that the destination (i.e., follower) node
    // currently has. If below `log_entries_` contains logs,
    // the starting index will be `last_log_idx_ + 1`.
    ulong last_log_idx_;

    // Source (i.e., leader) node's current committed log index.
    // As a pipelining, follower will do commit on this index number
    // after appending given logs.
    ulong commit_idx_;

    // Logs. Can be empty.
    std::vector<ptr<log_entry>> log_entries_;

    //! FORENSICS: certificate
    ptr<certificate> cc_;
};

} // namespace nuraft

#endif //_REG_MSG_HXX_

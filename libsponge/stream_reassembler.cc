#include "stream_reassembler.hh"

#include <iostream>
using namespace std;
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts two substrings (aka two segments) of bytes,
//! and merge them.
StreamReassembler::Segment StreamReassembler::merge(Segment data_left, Segment data_right) {
    // data_left.index need to be less than data_right.index
    if (data_left.index > data_right.index)
        return merge(data_right, data_left);
    // data_left.end >= data_right.end means data_left contains data_right
    // the result of merging is data_left
    // note that both data_left and data_right are unassembled segments
    // so _unassembled = _unassembled + len(merge_data) - len(data_left) - len(data_right)
    if (data_left.end >= data_right.end) {
        _unassembled -= data_right.data.size();
        return data_left;
    } else {
        Segment merge_data;
        // sub-string
        uint64_t start = data_left.end - data_right.index + 1;
        merge_data.data = data_left.data + data_right.data.substr(start);
        merge_data.index = data_left.index;
        merge_data.end = data_right.end;
        _unassembled -= data_left.data.size() + data_right.data.size() - merge_data.data.size();
        return merge_data;
    }
}

//! \details This function accepts one string and its index.
//! In this function, the string will be initialized.
//! For example, data may be "", or _head is 8, but index is 0 and data is "abc".
StreamReassembler::Segment StreamReassembler::init(const string &data, const size_t index) {
    Segment pre_segment;
    // empty string
    pre_segment.end = data.size() == 0 ? 0 : index + data.size() - 1;
    // This situation is ok!
    if (index >= _head) {
        pre_segment.index = index;
        pre_segment.data = data;
    }
    // The end of the segment must be greater than _head.
    else {
        pre_segment.index = _head;
        // useless data
        if (pre_segment.end < _head)
            return pre_segment;
        pre_segment.data = data.substr(_head - index);
    }
    return pre_segment;
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // capacity over
    if (index >= _head + _output.remaining_capacity())
        return;
    // data preprocessing
    string pre_data;
    if (index + data.size() > _head + _output.remaining_capacity()) {
        pre_data = data.substr(0, _head + _output.remaining_capacity() - index);
    } else {
        _eof = _eof ? _eof : eof;
        pre_data = data;
    }
    Segment merge_data = init(pre_data, index);

    // if merge_data.end < merge_data.index, there must be something wrong
    if (merge_data.end >= merge_data.index) {
        // unassembled bytes update
        _unassembled += merge_data.data.size();
        int count = 1;
        std::set<Segment>::iterator it;
        while (count) {
            count--;
            it = _segments.begin();
            for (; it != _segments.end(); it++) {
                // if merge_data intersects with *it
                if ((merge_data.index <= it->end && merge_data.end >= it->index) ||
                    (it->index <= merge_data.end && it->end >= merge_data.index) || it->index - merge_data.end == 1 ||
                    merge_data.index - it->end == 1) {
                    merge_data = merge(*it, merge_data);
                    _segments.erase(it);
                    count++;
                    break;
                }
            }
        }
        _segments.insert(merge_data);
        // write to _output
        it = _segments.begin();
        if (it->index == _head) {
            _output.write(it->data);
            _unassembled -= it->data.size();
            _head = it->end + 1;
            _segments.erase(it);
        }
    }
    // end input
    if (_eof && empty()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled; }

bool StreamReassembler::empty() const { return _unassembled == 0; }

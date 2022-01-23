#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();

    if (!header.syn && !_syn)
        return;

    if (header.syn && !_syn) {
        _syn = true;
        _isn = header.seqno;
    }
    if (header.fin)
        _fin = true;

    uint64_t checkpoint = stream_out().bytes_written();
    uint64_t abs_seqno = unwrap(header.seqno, _isn, checkpoint);
    string data = seg.payload().copy();
    _reassembler.push_substring(data, abs_seqno, _fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn)
        return nullopt;
    uint64_t abs_seqno = stream_out().bytes_written();
    if (_fin && stream_out().input_ended())
        abs_seqno++;
    return optional<WrappingInt32>(wrap(abs_seqno + 1, _isn));
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }

#include "wrapping_integers.hh"
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return isn + static_cast<uint32_t>(n); }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    const uint64_t pow2_32 = 1ul << 32;
    uint32_t offset = n.raw_value() - isn.raw_value();
    // checkpoint = top32(from 32 bit to 63) + down32(from 0 to 31)
    uint64_t top32 = checkpoint & 0xFFFFFFFF00000000;
    uint64_t num1 = (top32 - pow2_32) + offset;
    uint64_t num2 = top32 + offset;
    uint64_t num3 = (top32 + pow2_32) + offset;
    uint64_t diff1 = checkpoint - num1;
    uint64_t diff2 = num2 > checkpoint ? num2 - checkpoint : checkpoint - num2;
    uint64_t diff3 = num3 - checkpoint;
    if (!top32)
        return diff2 < diff3 ? num2 : num3;
    if (diff1 < diff2)
        return diff1 < diff3 ? num1 : num3;
    else
        return diff2 < diff3 ? num2 : num3;
}

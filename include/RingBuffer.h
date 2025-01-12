#pragma once
#include <cstddef>  // for size_t
#include <cstring>

/**
 * A simple ring buffer for items of type T with capacity N.
 * This is a header-only template, so there's no .cpp needed.
 * 
 * Usage example:
 * 
 *   RingBuffer<IMUData, 16> myIMURing;
 *   myIMURing.push(someIMUData);
 *   ...
 *   IMUData out;
 *   if(myIMURing.pop(out)) { ... }
 */
template<typename T, size_t N>
class RingBuffer {
public:
    RingBuffer() 
    : _head(0)
    , _tail(0)
    , _count(0)
    {
        // optionally zero out the buffer
        // std::memset(_buffer, 0, sizeof(_buffer));
    }

    /**
     * Returns true if the buffer is full (cannot accept more).
     */
    bool isFull() const {
        return (_count == N);
    }

    /**
     * Returns true if the buffer is empty (no items to pop).
     */
    bool isEmpty() const {
        return (_count == 0);
    }

    /**
     * Push item into ring buffer if there's space.
     * Returns true if pushed, false if full.
     */
    bool push(const T& item) {
        if(isFull()) {
            return false; // buffer is full
        }
        _buffer[_head] = item;
        _head = (_head + 1) % N;
        _count++;
        return true;
    }

    /**
     * Pop an item if available. Return true if popped, false if empty.
     */
    bool pop(T& outItem) {
        if(isEmpty()) {
            return false;
        }
        outItem = _buffer[_tail];
        _tail = (_tail + 1) % N;
        _count--;
        return true;
    }

    /**
     * Return the number of items currently stored.
     */
    size_t size() const {
        return _count;
    }

    /**
     * Return the capacity (i.e., N).
     */
    size_t capacity() const {
        return N;
    }

private:
    T      _buffer[N];
    size_t _head;
    size_t _tail;
    size_t _count;
};

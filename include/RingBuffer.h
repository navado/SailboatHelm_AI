#pragma once
#include <cstddef> // for size_t

/**
 * A circular ring buffer for items of type T with capacity N.
 * 
 * Key behavior: 
 *   - If the buffer is **full** and you push a new item, 
 *     the **oldest** item is overwritten.
 *   - pop() retrieves the **oldest** item from the buffer 
 *     (FIFO ordering).
 * 
 * Typical usage:
 *   RingBuffer<IMUData, 16> ring;
 *   ring.push(someData);  // If full, it overwrites the oldest
 *   ...
 *   IMUData out;
 *   if(ring.pop(out)) { 
 *       // popped from the oldest
 *   }
 */
template<typename T, size_t N>
class RingBuffer {
public:
    RingBuffer()
        : _head(0)
        , _tail(0)
        , _count(0)
    {
        // Optionally zero out the buffer
        // for (size_t i=0; i < N; i++) {
        //     _buffer[i] = T{};
        // }
    }

    ~RingBuffer() = default;

    /**
     * Return the capacity (constant N).
     */
    constexpr size_t capacity() const { return N; }

    /**
     * Return how many items are currently in the buffer.
     */
    size_t size() const { return _count; }

    /**
     * Return true if the buffer has no items.
     */
    bool isEmpty() const { return (_count == 0); }

    /**
     * Return true if the buffer is at capacity (pushing a new item 
     * will overwrite the oldest).
     */
    bool isFull() const { return (_count == N); }

    /**
     * Push item into the ring buffer, 
     * overwriting the oldest if it's already full.
     *
     * This always returns true now, because we never reject an item.
     */
    bool push(const T& item) {
        _buffer[_head] = item;

        // If the buffer is already full, 
        // move tail to discard the oldest item.
        if(isFull()) {
            _tail = (_tail + 1) % N;
        } else {
            _count++;
        }
        // Move head forward
        _head = (_head + 1) % N;
        return true; // always succeed
    }

    /**
     * Pop the oldest item if available. 
     * Return true if successful, false if empty.
     */
    bool pop(T& out) {
        if(isEmpty()) {
            return false;
        }
        out = _buffer[_tail];
        _tail = (_tail + 1) % N;
        _count--;
        return true;
    }

private:
    T      _buffer[N]; 
    size_t _head;   // next position to write
    size_t _tail;   // next position to read
    size_t _count;  // how many items in buffer
};

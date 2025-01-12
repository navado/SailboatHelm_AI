#pragma once
#include <cstdint>

/**
 * Simple interface to provide time in milliseconds
 * without referencing Arduino's millis().
 * Any platform can implement this (PC using std::chrono, etc.).
 */
class ITimeProvider {
public:
    virtual ~ITimeProvider() = default;

    // Return current time in ms since some epoch
    virtual std::uint64_t getMillis() const = 0;
};

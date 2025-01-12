#pragma once

enum class ButtonId {
    BTN_AUTO,
    BTN_MODE,
    BTN_INC_SMALL,
    BTN_DEC_SMALL,
    BTN_INC_LARGE,
    BTN_DEC_LARGE
};

/**
 * A platform-agnostic interface for reading whether
 * a given button is pressed.
 */
class IInputDevice {
public:
    virtual ~IInputDevice() = default;

    // Return true if the button is pressed, false if not
    virtual bool isPressed(ButtonId btn) const = 0;
};

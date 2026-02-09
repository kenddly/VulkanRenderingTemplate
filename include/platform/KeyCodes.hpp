#pragma once
#include <cstdint>

namespace vks {

enum class Key : uint16_t {
    Unknown = 0,

    // Letters
    A, B, C, D, E, F, G,
    H, I, J, K, L, M, N,
    O, P, Q, R, S, T, U,
    V, W, X, Y, Z,

    // Numbers
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,

    // Function keys
    F1, F2, F3, F4, F5, F6,
    F7, F8, F9, F10, F11, F12,

    // Control keys
    Space,
    Enter,
    Escape,
    Tab,
    Backspace,
    Insert,
    DeleteKey,
    Home,
    End,
    PageUp,
    PageDown,
    Left,
    Right,
    Up,
    Down,

    // Punctuation
    Minus,
    Equal,
    LeftBracket,
    RightBracket,
    Semicolon,
    Apostrophe,
    Comma,
    Period,
    Slash,
    Backslash,
    GraveAccent,

    // Keypad
    KP0, KP1, KP2, KP3, KP4,
    KP5, KP6, KP7, KP8, KP9,
    KPDecimal,
    KPAdd,
    KPSubtract,
    KPMultiply,
    KPDivide,
    KPEnter,

    // Modifier keys
    LeftShift,
    RightShift,
    LeftControl,
    RightControl,
    LeftAlt,
    RightAlt,
    LeftSuper,
    RightSuper,

    COUNT
};

} // namespace vks
#include <SDL3/SDL.h>

namespace Input {

    enum Keys {
        A,
        S,
        D,
        F,
        G,
        H,
        J,
        K,
        L,
        Q,
        W,
        E,
        R,
        T,
        Y,
        U,
        I,
        O,
        P,
        Z,
        X,
        C,
        V,
        B,
        N,
        M,
        Up,
        Down,
        Left,
        Right,
        Space,
        Enter,
        Escape
    };

    enum Mouse {
        LeftButton,
        MiddleButton,
        RightButton,
        MouseX,
        MouseY,
        ScrollX,
        ScrollY
    };

    enum GamepadButtons {
        GP_A,
        GP_B,
        GP_X,
        GP_Y,
        GP_Back,
        GP_Guide,
        GP_Start,
        GP_LeftStick,
        GP_RightStick,
        GP_LeftShoulder,
        GP_RightShoulder,
        GP_Up,
        GP_Down,
        GP_Left,
        GP_Right
    };

    enum GamepadAxes {
        GP_LeftX,
        GP_LeftY,
        GP_RightX,
        GP_RightY,
        GP_TriggerLeft,
        GP_TriggerRight
    };

    enum GamepadVibration {
        GP_VibrationLeft,
        GP_VibrationRight
    };

    enum Touch {
        TouchX,
        TouchY,
        TouchPressure
    };

    bool KeyPressed(Keys Key);
    bool KeyDown(Keys Key);
    bool KeyReleased(Keys Key);
    bool MouseButtonPressed(Mouse Button);
    bool MouseButtonDown(Mouse Button);
    bool MouseButtonReleased(Mouse Button);
    void GetMousePosition(int &x, int &y);
    int GetMouseScrollX();
    int GetMouseScrollY();
    bool GamepadButtonPressed(int gamepadIndex, GamepadButtons Button);
    bool GamepadButtonDown(int gamepadIndex, GamepadButtons Button);
    bool GamepadButtonReleased(int gamepadIndex, GamepadButtons Button);
    float GetGamepadAxis(int gamepadIndex, GamepadAxes Axis);
    void SetGamepadVibration(int gamepadIndex, GamepadVibration Motor, float intensity);
    float GetTouchInput(int touchIndex, Touch inputType);
}
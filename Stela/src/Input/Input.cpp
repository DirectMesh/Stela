#include "Input.h"
#include <vector>

namespace Input
{

    bool KeyPressed(Keys Key)
    {
        // Update SDL keyboard state
        SDL_PumpEvents();

        int numkeys = 0;
        const Uint8 *state = reinterpret_cast<const Uint8 *>(SDL_GetKeyboardState(&numkeys));

        static std::vector<Uint8> prevState;
        if (prevState.size() != (size_t)numkeys)
        {
            prevState.assign(numkeys, 0);
        }

        auto scancodeForKey = [](Keys k) -> int
        {
            switch (k)
            {
            case A:
                return SDL_SCANCODE_A;
            case S:
                return SDL_SCANCODE_S;
            case D:
                return SDL_SCANCODE_D;
            case F:
                return SDL_SCANCODE_F;
            case G:
                return SDL_SCANCODE_G;
            case H:
                return SDL_SCANCODE_H;
            case J:
                return SDL_SCANCODE_J;
            case K:
                return SDL_SCANCODE_K;
            case L:
                return SDL_SCANCODE_L;
            case Q:
                return SDL_SCANCODE_Q;
            case W:
                return SDL_SCANCODE_W;
            case E:
                return SDL_SCANCODE_E;
            case R:
                return SDL_SCANCODE_R;
            case T:
                return SDL_SCANCODE_T;
            case Y:
                return SDL_SCANCODE_Y;
            case U:
                return SDL_SCANCODE_U;
            case I:
                return SDL_SCANCODE_I;
            case O:
                return SDL_SCANCODE_O;
            case P:
                return SDL_SCANCODE_P;
            case Z:
                return SDL_SCANCODE_Z;
            case X:
                return SDL_SCANCODE_X;
            case C:
                return SDL_SCANCODE_C;
            case V:
                return SDL_SCANCODE_V;
            case B:
                return SDL_SCANCODE_B;
            case N:
                return SDL_SCANCODE_N;
            case M:
                return SDL_SCANCODE_M;
            case Up:
                return SDL_SCANCODE_UP;
            case Down:
                return SDL_SCANCODE_DOWN;
            case Left:
                return SDL_SCANCODE_LEFT;
            case Right:
                return SDL_SCANCODE_RIGHT;
            case Space:
                return SDL_SCANCODE_SPACE;
            case Enter:
                return SDL_SCANCODE_RETURN;
            case Escape:
                return SDL_SCANCODE_ESCAPE;
            default:
                return -1;
            }
        };

        int sc = scancodeForKey(Key);
        if (sc < 0 || sc >= numkeys)
            return false;

        // Edge detection: true only on transition from up to down
        bool pressed = state[sc] && !prevState[sc];
        prevState[sc] = state[sc];
        return pressed;
    }

    bool KeyDown(Keys Key)
    {
        // Update SDL keyboard state
        SDL_PumpEvents();

        int numkeys = 0;
        const Uint8 *state = reinterpret_cast<const Uint8 *>(SDL_GetKeyboardState(&numkeys));

        auto scancodeForKey = [](Keys k) -> int
        {
            switch (k)
            {
            case A:
                return SDL_SCANCODE_A;
            case S:
                return SDL_SCANCODE_S;
            case D:
                return SDL_SCANCODE_D;
            case F:
                return SDL_SCANCODE_F;
            case G:
                return SDL_SCANCODE_G;
            case H:
                return SDL_SCANCODE_H;
            case J:
                return SDL_SCANCODE_J;
            case K:
                return SDL_SCANCODE_K;
            case L:
                return SDL_SCANCODE_L;
            case Q:
                return SDL_SCANCODE_Q;
            case W:
                return SDL_SCANCODE_W;
            case E:
                return SDL_SCANCODE_E;
            case R:
                return SDL_SCANCODE_R;
            case T:
                return SDL_SCANCODE_T;
            case Y:
                return SDL_SCANCODE_Y;
            case U:
                return SDL_SCANCODE_U;
            case I:
                return SDL_SCANCODE_I;
            case O:
                return SDL_SCANCODE_O;
            case P:
                return SDL_SCANCODE_P;
            case Z:
                return SDL_SCANCODE_Z;
            case X:
                return SDL_SCANCODE_X;
            case C:
                return SDL_SCANCODE_C;
            case V:
                return SDL_SCANCODE_V;
            case B:
                return SDL_SCANCODE_B;
            case N:
                return SDL_SCANCODE_N;
            case M:
                return SDL_SCANCODE_M;
            case Up:
                return SDL_SCANCODE_UP;
            case Down:
                return SDL_SCANCODE_DOWN;
            case Left:
                return SDL_SCANCODE_LEFT;
            case Right:
                return SDL_SCANCODE_RIGHT;
            case Space:
                return SDL_SCANCODE_SPACE;
            case Enter:
                return SDL_SCANCODE_RETURN;
            case Escape:
                return SDL_SCANCODE_ESCAPE;
            default:
                return -1;
            }
        };

        int sc = scancodeForKey(Key);
        if (sc < 0 || sc >= numkeys)
            return false;

        return state[sc];
    }

    bool KeyReleased(Keys Key)
    {
        // Update SDL keyboard state
        SDL_PumpEvents();

        int numkeys = 0;
        const Uint8 *state = reinterpret_cast<const Uint8 *>(SDL_GetKeyboardState(&numkeys));

        static std::vector<Uint8> prevState;
        if (prevState.size() != (size_t)numkeys)
        {
            prevState.assign(numkeys, 0);
        }

        auto scancodeForKey = [](Keys k) -> int
        {
            switch (k)
            {
            case A:
                return SDL_SCANCODE_A;
            case S:
                return SDL_SCANCODE_S;
            case D:
                return SDL_SCANCODE_D;
            case F:
                return SDL_SCANCODE_F;
            case G:
                return SDL_SCANCODE_G;
            case H:
                return SDL_SCANCODE_H;
            case J:
                return SDL_SCANCODE_J;
            case K:
                return SDL_SCANCODE_K;
            case L:
                return SDL_SCANCODE_L;
            case Q:
                return SDL_SCANCODE_Q;
            case W:
                return SDL_SCANCODE_W;
            case E:
                return SDL_SCANCODE_E;
            case R:
                return SDL_SCANCODE_R;
            case T:
                return SDL_SCANCODE_T;
            case Y:
                return SDL_SCANCODE_Y;
            case U:
                return SDL_SCANCODE_U;
            case I:
                return SDL_SCANCODE_I;
            case O:
                return SDL_SCANCODE_O;
            case P:
                return SDL_SCANCODE_P;
            case Z:
                return SDL_SCANCODE_Z;
            case X:
                return SDL_SCANCODE_X;
            case C:
                return SDL_SCANCODE_C;
            case V:
                return SDL_SCANCODE_V;
            case B:
                return SDL_SCANCODE_B;
            case N:
                return SDL_SCANCODE_N;
            case M:
                return SDL_SCANCODE_M;
            case Up:
                return SDL_SCANCODE_UP;
            case Down:
                return SDL_SCANCODE_DOWN;
            case Left:
                return SDL_SCANCODE_LEFT;
            case Right:
                return SDL_SCANCODE_RIGHT;
            case Space:
                return SDL_SCANCODE_SPACE;
            case Enter:
                return SDL_SCANCODE_RETURN;
            case Escape:
                return SDL_SCANCODE_ESCAPE;
            default:
                return -1;
            }
        };

        int sc = scancodeForKey(Key);
        if (sc < 0 || sc >= numkeys)
            return false;

        // Edge detection: true only on transition from down to up
        bool released = !state[sc] && prevState[sc];
        prevState[sc] = state[sc];
        return released;
    }

    // Mouse helpers
    static Uint32 MousePrevButtons = 0;

    int mouseButtonToSDL(Mouse Button)
    {
        switch (Button)
        {
        case Mouse::LeftButton:
            return SDL_BUTTON_LEFT;
        case Mouse::MiddleButton:
            return SDL_BUTTON_MIDDLE;
        case Mouse::RightButton:
            return SDL_BUTTON_RIGHT;
        default:
            return 0;
        }
    }

    bool MouseButtonPressed(Mouse Button)
    {
        SDL_PumpEvents();
        float x, y;
        Uint32 state = SDL_GetMouseState(&x, &y);
        int btn = mouseButtonToSDL(Button);
        Uint32 mask = (btn > 0) ? (1u << (btn - 1)) : 0;
        bool pressed = (state & mask) && !(MousePrevButtons & mask);
        MousePrevButtons = state;
        return pressed;
    }

    bool MouseButtonDown(Mouse Button)
    {
        SDL_PumpEvents();
        float x, y;
        Uint32 state = SDL_GetMouseState(&x, &y);
        int btn = mouseButtonToSDL(Button);
        Uint32 mask = (btn > 0) ? (1u << (btn - 1)) : 0;
        return (state & mask) != 0;
    }

    bool MouseButtonReleased(Mouse Button)
    {
        SDL_PumpEvents();
        float x, y;
        Uint32 state = SDL_GetMouseState(&x, &y);
        int btn = mouseButtonToSDL(Button);
        Uint32 mask = (btn > 0) ? (1u << (btn - 1)) : 0;
        bool released = !(state & mask) && (MousePrevButtons & mask);
        MousePrevButtons = state;
        return released;
    }

    void GetMousePosition(int &x, int &y)
    {
        SDL_PumpEvents();

        float fx, fy;
        SDL_GetMouseState(&fx, &fy);

        x = static_cast<int>(fx);
        y = static_cast<int>(fy);
    }

    int GetMouseScrollX()
    {
        // SDL provides wheel events via the event queue. If the app processes events elsewhere,
        // returning 0 here is the safest default.
        return 0;
    }

    int GetMouseScrollY()
    {
        return 0;
    }

    // Gamepad helpers
    using GamepadPtr = SDL_Gamepad *;

    static std::vector<GamepadPtr> g_gamepads;
    static std::vector<std::vector<Uint8>> g_prevGamepadButtons;

    static GamepadPtr GetGamepad(int index)
    {
        if (index < 0)
            return nullptr;

        if ((size_t)index >= g_gamepads.size())
        {
            g_gamepads.resize(index + 1, nullptr);
            g_prevGamepadButtons.resize(index + 1);
        }

        if (!g_gamepads[index])
        {
            if (SDL_IsGamepad(index))
            {
                g_gamepads[index] = SDL_OpenGamepad(index);
            }
        }

        if (g_gamepads[index] &&
            g_prevGamepadButtons[index].size() != SDL_GAMEPAD_BUTTON_COUNT)
        {
            g_prevGamepadButtons[index].assign(SDL_GAMEPAD_BUTTON_COUNT, 0);
        }

        return g_gamepads[index];
    }

    static SDL_GamepadButton MapGamepadButton(GamepadButtons b)
    {
        switch (b)
        {
        case GamepadButtons::GP_A:
            return SDL_GAMEPAD_BUTTON_SOUTH;
        case GamepadButtons::GP_B:
            return SDL_GAMEPAD_BUTTON_EAST;
        case GamepadButtons::GP_X:
            return SDL_GAMEPAD_BUTTON_WEST;
        case GamepadButtons::GP_Y:
            return SDL_GAMEPAD_BUTTON_NORTH;
        case GamepadButtons::GP_Back:
            return SDL_GAMEPAD_BUTTON_BACK;
        case GamepadButtons::GP_Guide:
            return SDL_GAMEPAD_BUTTON_GUIDE;
        case GamepadButtons::GP_Start:
            return SDL_GAMEPAD_BUTTON_START;
        case GamepadButtons::GP_LeftStick:
            return SDL_GAMEPAD_BUTTON_LEFT_STICK;
        case GamepadButtons::GP_RightStick:
            return SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        case GamepadButtons::GP_LeftShoulder:
            return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
        case GamepadButtons::GP_RightShoulder:
            return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
        case GamepadButtons::GP_Up:
            return SDL_GAMEPAD_BUTTON_DPAD_UP;
        case GamepadButtons::GP_Down:
            return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        case GamepadButtons::GP_Left:
            return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
        case GamepadButtons::GP_Right:
            return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
        default:
            return SDL_GAMEPAD_BUTTON_INVALID;
        }
    }

    bool GamepadButtonPressed(int index, GamepadButtons button)
    {
        SDL_PumpEvents();
        auto pad = GetGamepad(index);
        if (!pad)
            return false;

        auto b = MapGamepadButton(button);
        if (b == SDL_GAMEPAD_BUTTON_INVALID)
            return false;

        Uint8 cur = SDL_GetGamepadButton(pad, b);
        Uint8 prev = g_prevGamepadButtons[index][b];

        g_prevGamepadButtons[index][b] = cur;
        return cur && !prev;
    }

    bool GamepadButtonDown(int index, GamepadButtons button)
    {
        SDL_PumpEvents();
        auto pad = GetGamepad(index);
        if (!pad)
            return false;

        auto b = MapGamepadButton(button);
        return SDL_GetGamepadButton(pad, b) != 0;
    }

    bool GamepadButtonReleased(int index, GamepadButtons button)
    {
        SDL_PumpEvents();
        auto pad = GetGamepad(index);
        if (!pad)
            return false;

        auto b = MapGamepadButton(button);
        Uint8 cur = SDL_GetGamepadButton(pad, b);
        Uint8 prev = g_prevGamepadButtons[index][b];

        g_prevGamepadButtons[index][b] = cur;
        return !cur && prev;
    }

    float GetGamepadAxis(int index, GamepadAxes axis)
    {
        SDL_PumpEvents();
        auto pad = GetGamepad(index);
        if (!pad)
            return 0.0f;

        SDL_GamepadAxis a;
        switch (axis)
        {
        case GamepadAxes::GP_LeftX:
            a = SDL_GAMEPAD_AXIS_LEFTX;
            break;
        case GamepadAxes::GP_LeftY:
            a = SDL_GAMEPAD_AXIS_LEFTY;
            break;
        case GamepadAxes::GP_RightX:
            a = SDL_GAMEPAD_AXIS_RIGHTX;
            break;
        case GamepadAxes::GP_RightY:
            a = SDL_GAMEPAD_AXIS_RIGHTY;
            break;
        case GamepadAxes::GP_TriggerLeft:
            a = SDL_GAMEPAD_AXIS_LEFT_TRIGGER;
            break;
        case GamepadAxes::GP_TriggerRight:
            a = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
            break;
        default:
            return 0.0f;
        }

        Sint16 v = SDL_GetGamepadAxis(pad, a);
        return (v >= 0) ? (float)v / 32767.0f : (float)v / 32768.0f;
    }

    void SetGamepadVibration(int index, GamepadVibration motor, float intensity)
    {
        auto pad = GetGamepad(index);
        if (!pad)
            return;

        Uint16 mag = (Uint16)(SDL_clamp(intensity, 0.0f, 1.0f) * 0xFFFF);
        Uint16 low = 0, high = 0;

        if (motor == GamepadVibration::GP_VibrationLeft)
            low = mag;
        else if (motor == GamepadVibration::GP_VibrationRight)
            high = mag;
        else
            low = high = mag;

        SDL_RumbleGamepad(pad, low, high, 500);
    }

    float GetTouchInput(int touchIndex, Touch inputType)
    {
        // Touch APIs depend on touch devices and fingers; a minimal portable stub returns 0.
        // A full implementation would query SDL touches/fingers and return X/Y/pressure as requested.
        (void)touchIndex;
        (void)inputType;
        return 0.0f;
    }
}
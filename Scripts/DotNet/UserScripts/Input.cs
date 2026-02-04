using System;
using System.Runtime.InteropServices;

namespace Stela
{
    public enum Keys
    {
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
    }

    public static class Input
    {
        // Function pointers
        private unsafe static delegate* unmanaged<int, bool> _keyPressed;

        public static unsafe void Init(IntPtr keyPressedPtr)
        {
            _keyPressed = (delegate* unmanaged<int, bool>)keyPressedPtr;
        }

        public static unsafe bool KeyPressed(Keys key)
        {
            if (_keyPressed == null) return false;
            return _keyPressed((int)key);
        }
    }
}

using System;
using System.Runtime.InteropServices;

namespace Stela
{
    public static class ScriptAPI
    {
        private static IntPtr _logCallback;

        // C++ will call this to set up the API
        public static void Init(IntPtr logCallback)
        {
            _logCallback = logCallback;
        }

        public static void Log(string message)
        {
            if (_logCallback != IntPtr.Zero)
            {
                // Simple string passing, assuming ASCII/UTF8 handling on C++ side
                // We might need to marshal carefully. 
                // For simplicity, let's assume C++ expects a const char*
                NativeLog(_logCallback, message);
            }
        }

        // Helper to invoke the function pointer
        [DllImport("StelaNative", EntryPoint = "CallLog")] // Dummy name, we invoke pointer manually
        private static extern void NativeLogStub(string msg);

        private delegate void LogDelegate(string message);

        private static void NativeLog(IntPtr fp, string message)
        {
            // Marshal delegate
             var del = Marshal.GetDelegateForFunctionPointer<LogDelegate>(fp);
             del(message);
        }
    }
}

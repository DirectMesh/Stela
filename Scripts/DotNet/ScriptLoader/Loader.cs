using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Stela
{
    public static class Loader
    {
        private static AssemblyLoadContext? _scriptContext;
        private static MethodInfo? _updateMethod;
        private static MethodInfo? _shutdownMethod;
        
        // Callbacks from C++
        private static IntPtr _logCallback;
        private static IntPtr _inputCallback;

        [UnmanagedCallersOnly]
        public static void Init(IntPtr logCallback, IntPtr inputCallback)
        {
            _logCallback = logCallback;
            _inputCallback = inputCallback;
            Console.WriteLine("[Loader] Initialized.");
        }

        [UnmanagedCallersOnly]
        public static int LoadUserScripts(IntPtr pathPtr)
        {
            try
            {
                string? dllPath = Marshal.PtrToStringAnsi(pathPtr);
                if (string.IsNullOrEmpty(dllPath)) return -1;

                Console.WriteLine($"[Loader] Loading assembly from: {dllPath}");

                // Unload previous
                if (_scriptContext != null)
                {
                    try 
                    {
                        _shutdownMethod?.Invoke(null, null);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"[Loader] Shutdown error: {ex}");
                    }
                    
                    _scriptContext.Unload();
                    _scriptContext = null;
                    _updateMethod = null;
                    _shutdownMethod = null;

                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                }

                // Load new
                _scriptContext = new AssemblyLoadContext("UserScriptsContext", isCollectible: true);
                
                // Read bytes first to avoid locking file
                byte[] assemblyBytes = File.ReadAllBytes(dllPath);
                using var ms = new MemoryStream(assemblyBytes);
                var assembly = _scriptContext.LoadFromStream(ms);

                var managerType = assembly.GetType("Stela.ScriptManager");
                if (managerType == null)
                {
                    Console.WriteLine("[Loader] Could not find Stela.ScriptManager type.");
                    return -2;
                }

                var initMethod = managerType.GetMethod("Init", BindingFlags.Public | BindingFlags.Static);
                _updateMethod = managerType.GetMethod("Update", BindingFlags.Public | BindingFlags.Static);
                _shutdownMethod = managerType.GetMethod("Shutdown", BindingFlags.Public | BindingFlags.Static);

                if (initMethod != null)
                {
                    initMethod.Invoke(null, new object[] { _logCallback, _inputCallback });
                }

                return 0;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Loader] Error loading assembly: {ex}");
                return -99;
            }
        }

        [UnmanagedCallersOnly]
        public static void Update(float dt)
        {
            try
            {
                _updateMethod?.Invoke(null, new object[] { dt });
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Loader] Error in Update: {ex}");
            }
        }
        
        [UnmanagedCallersOnly]
        public static void Shutdown()
        {
             try
            {
                _shutdownMethod?.Invoke(null, null);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[Loader] Error in Shutdown: {ex}");
            }
        }
    }
}

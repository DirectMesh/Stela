using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Stela
{
    public class ScriptManager
    {
        private struct ScriptRuntime
        {
            public object Instance;
            public MethodInfo UpdateMethod;
            public bool UpdateTakesDt;
            public MethodInfo ShutdownMethod;
        }

        private static List<ScriptRuntime> _runtimes = new List<ScriptRuntime>();

        // Called by Loader (Managed)
        public static void Init(IntPtr logCallback, IntPtr keyPressedCallback)
        {
            try
            {
                ScriptAPI.Init(logCallback);
                Input.Init(keyPressedCallback);
                _runtimes.Clear();
                ScriptAPI.Log("C# ScriptManager Initialized.");
                LoadScripts();
            }
            catch (Exception ex)
            {
                // Can't log if Init failed, but try
                try { ScriptAPI.Log($"C# Init failed: {ex}"); } catch { }
            }
        }

        private static void LoadScripts()
        {
            try
            {
                var assembly = Assembly.GetExecutingAssembly();
                var types = assembly.GetTypes();

                foreach (var type in types)
                {
                    if (type == typeof(ScriptManager) || type == typeof(ScriptAPI) || type == typeof(Input) || type.IsNestedPrivate) continue;
                    
                    // Simple heuristic: if it has OnStart or OnUpdate, it's a script
                    var onStart = type.GetMethod("OnStart", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
                    var onUpdate = type.GetMethod("OnUpdate", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
                    var onShutdown = type.GetMethod("OnShutdown", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);

                    if (onStart != null || onUpdate != null || onShutdown != null)
                    {
                        try
                        {
                            var instance = Activator.CreateInstance(type);
                            
                            if (onStart != null)
                            {
                                try 
                                {
                                    onStart.Invoke(instance, null);
                                }
                                catch (Exception ex)
                                {
                                    ScriptAPI.Log($"Error in {type.Name}.OnStart: {ex.InnerException?.Message ?? ex.Message}");
                                }
                            }

                            bool takesDt = false;
                            if (onUpdate != null)
                            {
                                var parameters = onUpdate.GetParameters();
                                if (parameters.Length == 1 && parameters[0].ParameterType == typeof(float))
                                {
                                    takesDt = true;
                                }
                                else if (parameters.Length != 0)
                                {
                                    ScriptAPI.Log($"Warning: {type.Name}.OnUpdate has unsupported signature. Expected OnUpdate() or OnUpdate(float dt).");
                                    onUpdate = null; // Disable it
                                }
                            }

                            _runtimes.Add(new ScriptRuntime 
                            { 
                                Instance = instance, 
                                UpdateMethod = onUpdate,
                                UpdateTakesDt = takesDt,
                                ShutdownMethod = onShutdown
                            });
                            
                            ScriptAPI.Log($"Registered script: {type.Name}");
                        }
                        catch (Exception ex)
                        {
                            ScriptAPI.Log($"Failed to register script {type.Name}: {ex.Message}");
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ScriptAPI.Log($"LoadScripts failed: {ex}");
            }
        }

        public static void Update(float dt)
        {
            object[] args = new object[] { dt };
            foreach (var runtime in _runtimes)
            {
                if (runtime.UpdateMethod != null)
                {
                    try
                    {
                        if (runtime.UpdateTakesDt)
                            runtime.UpdateMethod.Invoke(runtime.Instance, args);
                        else
                            runtime.UpdateMethod.Invoke(runtime.Instance, null);
                    }
                    catch (Exception ex)
                    {
                         ScriptAPI.Log($"Error in {runtime.Instance.GetType().Name}.OnUpdate: {ex.InnerException?.Message ?? ex.Message}");
                    }
                }
            }
        }

        public static void Shutdown()
        {
            foreach (var runtime in _runtimes)
            {
                if (runtime.ShutdownMethod != null)
                {
                    try
                    {
                        runtime.ShutdownMethod.Invoke(runtime.Instance, null);
                    }
                    catch (Exception ex)
                    {
                         ScriptAPI.Log($"Error in {runtime.Instance.GetType().Name}.OnShutdown: {ex.InnerException?.Message ?? ex.Message}");
                    }
                }
            }
            _runtimes.Clear();
        }
    }
}

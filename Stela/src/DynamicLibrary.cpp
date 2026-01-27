#include "DynamicLibrary.h"

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <dlfcn.h>
#endif

namespace DynamicLibrary {

    void* Load(const char* path) {
    #if defined(_WIN32)
        return (void*)LoadLibraryA(path);
    #else
        return dlopen(path, RTLD_NOW);
    #endif
    }

    void* GetSymbol(void* library, const char* name) {
    #if defined(_WIN32)
        return (void*)GetProcAddress((HMODULE)library, name);
    #else
        return dlsym(library, name);
    #endif
    }

    void Unload(void* library) {
    #if defined(_WIN32)
        FreeLibrary((HMODULE)library);
    #else
        dlclose(library);
    #endif
    }
}

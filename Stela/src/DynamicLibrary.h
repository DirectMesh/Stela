#pragma once

namespace DynamicLibrary {

    void* Load(const char* path);
    void* GetSymbol(void* library, const char* name);
    void  Unload(void* library);

}

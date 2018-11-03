// platform.cpp
#include "platform.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
	#undef WIN32_LEAN_AND_MEAN
	#undef NOMINMAX
#else
	#include "slang-string.h"
	#include <dlfcn.h>
#endif

namespace Slang
{
	// SharedLibrary

	SharedLibrary SharedLibrary::load(char const* name)
	{
		SharedLibrary result;
#ifdef _WIN32
		{
			HMODULE h = LoadLibraryA(name);
			result.m_handle = (Handle) h;			
		}
#else
		{
			String fullName;
			fullName.append("lib");
			fullName.append(name);
			fullName.append(".so");

			void* h = dlopen(fullName.Buffer(), RTLD_NOW | RTLD_LOCAL);
			if(!h)
			{
				if(auto msg = dlerror())
				{
					fprintf(stderr, "error: %s\n", msg);
				}
			}
			result.m_handle = (Handle) h;
		}
#endif
		return result;
	}

	void SharedLibrary::unload()
	{
        if (m_handle)
        {
#ifdef _WIN32
			FreeLibrary((HMODULE) m_handle);
#else
			dlclose(m_handle);
#endif
            // Mark that it is unloaded
            m_handle = nullptr;
        }
	}

	SharedLibrary::FuncPtr SharedLibrary::findFuncByName(char const* name)
	{
#ifdef _WIN32
	    return (FuncPtr) GetProcAddress((HMODULE) m_handle,	name);
#else
		return (FuncPtr) dlsym((void*) m_handle, name);
#endif
	}
}
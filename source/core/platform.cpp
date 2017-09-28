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
		result.handle = nullptr;

#ifdef _WIN32
		{
			HMODULE h = LoadLibraryA(name);
			result.handle = (Handle) h;			
		}
#else
		{
			String fullName;
			fullName.append("lib");
			fullName.append(name);
			fullName.append(".so");

			void* h = dlopen(fullName.Buffer(), RTLD_NOW|RTLD_LOCAL);
			if(!h)
			{
				if(auto msg = dlerror())
				{
					fprintf(stderr, "error: %s\n", msg);
				}
			}
			result.handle = (Handle) h;

		}
#endif

		return result;
	}

	void SharedLibrary::unload()
	{
#ifdef _WIN32
		{
			FreeLibrary(
				(HMODULE) handle);
		}
#else
		{
			dlclose(handle);
		}
#endif

	}

	SharedLibrary::FuncPtr SharedLibrary::findFuncByName(char const* name)
	{
		FuncPtr funcPtr = nullptr;

#ifdef _WIN32
		{
			funcPtr = (FuncPtr) GetProcAddress(
				(HMODULE) handle,
				name);
		}
#else
		{
			funcPtr = (FuncPtr) dlsym(
				(void*) handle,
				name);
		}
#endif

		return funcPtr;
	}
}
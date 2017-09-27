// platform.h
#ifndef SLANG_CORE_PLATFORM_H_INCLUDED
#define SLANG_CORE_PLATFORM_H_INCLUDED

namespace Slang
{
	// Interface for working with shared libraries
	// in a platfomr-independent fashion.
	struct SharedLibrary
	{
		typedef struct SharedLibraryImpl* Handle;
		Handle handle;

		// Attempt to load a shared library for
		// the current platform.
		static SharedLibrary load(char const* name);

		// If this refers to a valid loaded library,
		// then attempt to unload it
		void unload();

		typedef void (*FuncPtr)(void);

		FuncPtr findFuncByName(char const* name);


		operator Handle() { return handle; }
	};

#ifndef _MSC_VER
	#define _fileno fileno
	#define _isatty isatty
	#define _setmode setmode
	#define _O_BINARY O_BINARY
#endif
}

#endif

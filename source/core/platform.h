// platform.h
#ifndef SLANG_CORE_PLATFORM_H_INCLUDED
#define SLANG_CORE_PLATFORM_H_INCLUDED

namespace Slang
{
	// Interface for working with shared libraries
	// in a platform-independent fashion.
	struct SharedLibrary
	{
		typedef struct SharedLibraryImpl* Handle;
        typedef void(*FuncPtr)(void);

		    // Attempt to load a shared library for
		    // the current platform. Returns an unloaded library on failure.
		static SharedLibrary load(char const* name);

		    // If this refers to a valid loaded library,
		    // then attempt to unload it
		void unload();

            /// True if there is a library loaded
        bool isLoaded() const { return m_handle != nullptr; }

            /// Get a function by name
		FuncPtr findFuncByName(char const* name);

            /// Convert to a handle a handle
		operator Handle() const { return m_handle; }

            /// Ctor
        SharedLibrary():m_handle(nullptr) {}
	
        protected:
        Handle m_handle;
    };

#ifndef _MSC_VER
	#define _fileno fileno
	#define _isatty isatty
	#define _setmode setmode
	#define _O_BINARY O_BINARY
#endif
}

#endif

// platform.h
#ifndef SLANG_CORE_PLATFORM_H_INCLUDED
#define SLANG_CORE_PLATFORM_H_INCLUDED

#include "../../slang.h"
#include "../core/slang-string.h"

namespace Slang
{
	// Interface for working with shared libraries
	// in a platform-independent fashion.
	struct SharedLibrary
	{
		typedef struct SharedLibraryImpl* Handle;
        
        typedef void(*FuncPtr)(void);

            /// Load via an unadorned filename
            /// 
            /// @param the unadorned filename
            /// @return Returns a non null handle for the shared library on success. nullptr indicated failure
        static SlangResult load(const char* filename, Handle& handleOut);

		    /// Attempt to load a shared library for
		    /// the current platform. Returns null handle on failure
            /// The platform specific filename can be generated from a call to appendPlatformFileName
            ///
            /// @param platformFileName the platform specific file name. 
            /// @return Returns a non null handle for the shared library on success. nullptr indicated failure
        static SlangResult loadWithPlatformFilename(char const* platformFileName, Handle& handleOut);

            /// Unload the library that was returned from load as handle
            /// @param The valid handle returned from load 
        static void unload(Handle handle);

            /// Given a shared library handle and a name, return the associated function
            /// Return nullptr if function is not found
            /// @param The shared library handle as returned by loadPlatformLibrary
        static FuncPtr findFuncByName(Handle handle, char const* name);

            /// Append to the end of dst, the name, with any platform specific additions
            /// The input name should be unadorned with any 'lib' prefix or extension
        static void appendPlatformFileName(const UnownedStringSlice& name, StringBuilder& dst);

        private:
            /// Not constructible!
        SharedLibrary();
    };

    struct PlatformUtil
    {
            /// Appends a text interpretation of a result (as defined by supporting OS)
            /// @param res Result to produce a string for
            /// @param builderOut Append the string produced to builderOut
            /// @return SLANG_OK if string is found and appended. Fail otherwise. SLANG_E_NOT_IMPLEMENTED if there is no impl for this platform.
        static SlangResult appendResult(SlangResult res, StringBuilder& builderOut);
    };

#ifndef _MSC_VER
	#define _fileno fileno
	#define _isatty isatty
	#define _setmode setmode
	#define _O_BINARY O_BINARY
#endif
}

#endif

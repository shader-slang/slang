#ifndef SLANG_NVVM_COMPILER_H
#define SLANG_NVVM_COMPILER_H

#include "core/slang-platform.h"
#include "slang-downstream-compiler-util.h"

namespace Slang
{

/// Owns the immutable device-library input selected for one compilation. Returned contents and
/// path are borrowed until this interface is released; neither getter reopens the selected file.
class INVVMCUDADeviceLibrary : public ICastable
{
public:
    SLANG_COM_INTERFACE(
        0x9fc16432,
        0x7bdb,
        0x4d08,
        {0xb6, 0x37, 0x73, 0x56, 0xc3, 0xec, 0xba, 0x19})
    virtual SLANG_NO_THROW ISlangBlob* SLANG_MCALL getContents() = 0;
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() = 0;
};

/// Optional borrowed extension reached through IDownstreamCompiler::castAs. Keep this separate
/// from that interface's vtable and expose it only through castAs, like its path-provider
/// extension.
class INVVMCUDADeviceLibraryProvider
{
public:
    SLANG_COM_INTERFACE(
        0xe44c017b,
        0x44fa,
        0x45c2,
        {0x85, 0x99, 0xc4, 0x08, 0x7f, 0x12, 0x54, 0xe3})
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadCUDADeviceLibrary(INVVMCUDADeviceLibrary** outLibrary, ISlangBlob** outDiagnostics) = 0;
};

/// Locates dynamically loadable libNVVM downstream compilers.
struct NVVMDownstreamCompilerUtil
{
    static SlangResult locateCompilers(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        DownstreamCompilerSet* set);
};

} // namespace Slang

#endif

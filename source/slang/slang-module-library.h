// slang-module-library.h
#ifndef SLANG_MODULE_LIBRARY_H
#define SLANG_MODULE_LIBRARY_H

#include "../compiler-core/slang-artifact.h"

#include "slang-compiler.h" 

namespace Slang
{

// Class to hold information serialized in from a -r slang-lib/slang-module
class ModuleLibrary : public ComObject, public IArtifactInstance
{
public:

    SLANG_COM_OBJECT_IUNKNOWN_ALL

    SLANG_CLASS_GUID(0x2f7412bd, 0x6154, 0x40a9, { 0x89, 0xb3, 0x62, 0xe0, 0x24, 0x17, 0x24, 0xa1 });

    // IArtifactInstance
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL writeToBlob(ISlangBlob** blob) SLANG_OVERRIDE { SLANG_UNUSED(blob); return SLANG_E_NOT_IMPLEMENTED; }
    virtual SLANG_NO_THROW void* SLANG_MCALL queryObject(const Guid& classGuid) SLANG_OVERRIDE { return classGuid == getTypeGuid() ? this : nullptr; }

    List<FrontEndCompileRequest::ExtraEntryPointInfo> m_entryPoints;
    List<RefPtr<IRModule>> m_modules;

    void* getInterface(const Guid& uuid);
};

SlangResult loadModuleLibrary(const Byte* inBytes, size_t bytesCount, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

// Given a product make available as a module
SlangResult loadModuleLibrary(ArtifactKeep keep, IArtifact* artifact, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

} // namespace Slang

#endif

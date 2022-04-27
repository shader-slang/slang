// slang-module-library.h
#ifndef SLANG_MODULE_LIBRARY_H
#define SLANG_MODULE_LIBRARY_H

#include "../compiler-core/slang-artifact.h"

#include "slang-compiler.h" 

namespace Slang
{

// Class to hold information serialized in from a -r slang-lib/slang-module
class ModuleLibrary : public RefObject
{
public:

    List<FrontEndCompileRequest::ExtraEntryPointInfo> m_entryPoints;
    List<RefPtr<IRModule>> m_modules;
};

SlangResult loadModuleLibrary(const Byte* inBytes, size_t bytesCount, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

// Given a product make available as a module
SlangResult loadModuleLibrary(ArtifactKeep keep, Artifact* artifact, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

} // namespace Slang

#endif

// slang-compiler-api.h
#pragma once

//
// This file provides utilities that are needed at the boundary
// between the public Slang API (the interfaces declared in
// `slang.h`) and the code that implements that API.
//

#include "slang-end-to-end-request.h"
#include "slang-global-session.h"
#include "slang-module.h"
#include "slang-session.h"

#include <slang.h>

namespace Slang
{

//
// The following functions are utilties to convert between
// matching "external" (public API) and "internal" (implementation)
// types. They are favored over explicit casts because they
// help avoid making incorrect conversions (e.g., when using
// `reinterpret_cast` or C-style casts), and because they
// abstract over the conversion required for each pair of types.
//

SLANG_FORCE_INLINE slang::IGlobalSession* asExternal(Session* session)
{
    return static_cast<slang::IGlobalSession*>(session);
}

SLANG_FORCE_INLINE ComPtr<Session> asInternal(slang::IGlobalSession* session)
{
    Slang::Session* internalSession = nullptr;
    session->queryInterface(SLANG_IID_PPV_ARGS(&internalSession));
    return ComPtr<Session>(INIT_ATTACH, static_cast<Session*>(internalSession));
}

SLANG_FORCE_INLINE slang::ISession* asExternal(Linkage* linkage)
{
    return static_cast<slang::ISession*>(linkage);
}

SLANG_FORCE_INLINE Module* asInternal(slang::IModule* module)
{
    return static_cast<Module*>(module);
}

SLANG_FORCE_INLINE slang::IModule* asExternal(Module* module)
{
    return static_cast<slang::IModule*>(module);
}

ComponentType* asInternal(slang::IComponentType* inComponentType);

SLANG_FORCE_INLINE slang::IComponentType* asExternal(ComponentType* componentType)
{
    return static_cast<slang::IComponentType*>(componentType);
}

SLANG_FORCE_INLINE slang::ProgramLayout* asExternal(ProgramLayout* programLayout)
{
    return (slang::ProgramLayout*)programLayout;
}

SLANG_FORCE_INLINE Type* asInternal(slang::TypeReflection* type)
{
    return reinterpret_cast<Type*>(type);
}

SLANG_FORCE_INLINE slang::TypeReflection* asExternal(Type* type)
{
    return reinterpret_cast<slang::TypeReflection*>(type);
}

SLANG_FORCE_INLINE DeclRef<Decl> asInternal(slang::GenericReflection* generic)
{
    return DeclRef<Decl>(reinterpret_cast<DeclRefBase*>(generic));
}

SLANG_FORCE_INLINE slang::GenericReflection* asExternal(DeclRef<Decl> generic)
{
    return reinterpret_cast<slang::GenericReflection*>(generic.declRefBase);
}

SLANG_FORCE_INLINE TypeLayout* asInternal(slang::TypeLayoutReflection* type)
{
    return reinterpret_cast<TypeLayout*>(type);
}

SLANG_FORCE_INLINE slang::TypeLayoutReflection* asExternal(TypeLayout* type)
{
    return reinterpret_cast<slang::TypeLayoutReflection*>(type);
}

SLANG_FORCE_INLINE SlangCompileRequest* asExternal(EndToEndCompileRequest* request)
{
    return static_cast<SlangCompileRequest*>(request);
}

SLANG_FORCE_INLINE EndToEndCompileRequest* asInternal(SlangCompileRequest* request)
{
    // Converts to the internal type -- does a runtime type check through queryInterfae
    SLANG_ASSERT(request);
    EndToEndCompileRequest* endToEndRequest = nullptr;
    // NOTE! We aren't using to access an interface, so *doesn't* return with a refcount
    request->queryInterface(SLANG_IID_PPV_ARGS(&endToEndRequest));
    SLANG_ASSERT(endToEndRequest);
    return endToEndRequest;
}

SLANG_FORCE_INLINE SlangCompileTarget asExternal(CodeGenTarget target)
{
    return (SlangCompileTarget)target;
}

SLANG_FORCE_INLINE SlangSourceLanguage asExternal(SourceLanguage sourceLanguage)
{
    return (SlangSourceLanguage)sourceLanguage;
}

} // namespace Slang

// slang-check.h
#pragma once

// This file provides the public interface to the semantic
// checking infrastructure in `slang-check-*`.

#include "slang-syntax.h"

namespace Slang
{
    class DiagnosticSink;
    class EntryPoint;
    class Linkage;
    class Module;
    class ShaderCompiler;
    class ShaderLinkInfo;
    class ShaderSymbol;

    class TranslationUnitRequest;

    bool isGlobalShaderParameter(VarDeclBase* decl);
    bool isFromStdLib(Decl* decl);

    void registerBuiltinDecls(Session* session, Decl* decl);

    OrderedDictionary<GenericTypeParamDecl*, List<Type*>> getCanonicalGenericConstraints(
        ASTBuilder* builder, DeclRef<ContainerDecl> genericDecl);
}

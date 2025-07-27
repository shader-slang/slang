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
bool isFromCoreModule(Decl* decl);

void registerBuiltinDecl(SharedASTBuilder* sharedASTBuilder, Decl* decl);
void registerBuiltinDecl(ASTBuilder* astBuilder, Decl* decl);

void registerBuiltinDecls(Session* session, Decl* decl);

void collectBuiltinDeclsThatNeedRegistration(ModuleDecl* moduleDecl, List<Decl*>& outDecls);

Type* unwrapArrayType(Type* type);
Type* unwrapModifiedType(Type* type);

OrderedDictionary<GenericTypeParamDeclBase*, List<Type*>> getCanonicalGenericConstraints(
    ASTBuilder* builder,
    DeclRef<ContainerDecl> genericDecl);
OrderedDictionary<Type*, List<Type*>> getCanonicalGenericConstraints2(
    ASTBuilder* builder,
    DeclRef<ContainerDecl> genericDecl);

// General utility function to collect all referenced declarations from a value
void collectReferencedDecls(Val* val, HashSet<Decl*>& outDecls);
} // namespace Slang

#ifndef SLANG_LOOKUP_H_INCLUDED
#define SLANG_LOOKUP_H_INCLUDED

#include "slang-syntax.h"

namespace Slang {

struct SemanticsVisitor;

// Take an existing lookup result and refine it to only include
// results that pass the given `LookupMask`.
LookupResult refineLookup(LookupResult const& inResult, LookupMask mask);

// Ensure that the dictionary for name-based member lookup has been
// built for the given container declaration.
void buildMemberDictionary(ContainerDecl* decl);

// Look up a name in the given scope, proceeding up through
// parent scopes as needed.
LookupResult lookUp(
    ASTBuilder*         astBuilder, 
    SemanticsVisitor*   semantics,
    Name*               name,
    RefPtr<Scope>       scope,
    LookupMask          mask = LookupMask::Default);

// Perform member lookup in the context of a type
LookupResult lookUpMember(
    ASTBuilder*         astBuilder,
    SemanticsVisitor*   semantics,
    Name*               name,
    Type*               type,
    LookupMask          mask = LookupMask::Default,
    LookupOptions       options = LookupOptions::None);

    /// Perform "direct" lookup in a container declaration
LookupResult lookUpDirectAndTransparentMembers(
    ASTBuilder*             astBuilder,
    SemanticsVisitor*       semantics,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef,
    LookupMask              mask = LookupMask::Default);

// TODO: this belongs somewhere else

QualType getTypeForDeclRef(
    ASTBuilder*             astBuilder,
    SemanticsVisitor*       sema,
    DiagnosticSink*         sink,
    DeclRef<Decl>           declRef,
    Type**           outTypeResult,
    SourceLoc               loc);

QualType getTypeForDeclRef(
    ASTBuilder*     astBuilder, 
    DeclRef<Decl>   declRef,
    SourceLoc       loc);

    /// Add a found item to a lookup result
void AddToLookupResult(
    LookupResult&		result,
    LookupResultItem	item);

}

#endif

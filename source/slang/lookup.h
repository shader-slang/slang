#ifndef SLANG_LOOKUP_H_INCLUDED
#define SLANG_LOOKUP_H_INCLUDED

#include "Syntax.h"

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
LookupResult LookUp(
    Session*            session,
    SemanticsVisitor*   semantics,
    Name*               name,
    RefPtr<Scope>       scope);

// perform lookup within the context of a particular container declaration,
// and do *not* look further up the chain
LookupResult LookUpLocal(
    Session*                session,
    SemanticsVisitor*       semantics,
    Name*                   name,
    DeclRef<ContainerDecl>  containerDeclRef);

// TODO: this belongs somewhere else

QualType getTypeForDeclRef(
    Session*                session,
    SemanticsVisitor*       sema,
    DiagnosticSink*         sink,
    DeclRef<Decl>           declRef,
    RefPtr<Type>* outTypeResult);

QualType getTypeForDeclRef(
    Session*        session,
    DeclRef<Decl>   declRef);


}

#endif
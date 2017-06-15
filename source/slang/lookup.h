#ifndef SLANG_LOOKUP_H_INCLUDED
#define SLANG_LOOKUP_H_INCLUDED

#include "Syntax.h"

namespace Slang {

// Take an existing lookup result and refine it to only include
// results that pass the given `LookupMask`.
LookupResult refineLookup(LookupResult const& inResult, LookupMask mask);

// Ensure that the dictionary for name-based member lookup has been
// built for the given container declaration.
void buildMemberDictionary(ContainerDecl* decl);

// Look up a name in the given scope, proceeding up through
// parent scopes as needed.
LookupResult LookUp(String const& name, RefPtr<Scope> scope);

// perform lookup within the context of a particular container declaration,
// and do *not* look further up the chain
LookupResult LookUpLocal(String const& name, ContainerDeclRef containerDeclRef);
LookupResult LookUpLocal(String const& name, ContainerDecl* containerDecl);

// TODO: this belongs somewhere else

class SemanticsVisitor;
QualType getTypeForDeclRef(
    SemanticsVisitor*       sema,
    DiagnosticSink*         sink,
    DeclRef                 declRef,
    RefPtr<ExpressionType>* outTypeResult);

QualType getTypeForDeclRef(
    DeclRef                 declRef);


}

#endif
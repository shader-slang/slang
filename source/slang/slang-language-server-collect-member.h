// slang-language-server-collect-member.h
#pragma once

#include "slang-ast-all.h"
#include "slang-syntax.h"
#include "slang-check-impl.h"

namespace Slang
{

struct MemberCollectingContext
{
    ASTBuilder* astBuilder;
    List<Decl*> members;
    SharedSemanticsContext semanticsContext;
    MemberCollectingContext(Linkage* linkage, Module* module, DiagnosticSink* sink)
        : semanticsContext(linkage, module, sink)
    {}
};

void collectMembersInTypeDeclImpl(MemberCollectingContext* context, DeclRef<Decl> declRef);

void collectMembersInType(MemberCollectingContext* context, Type* type);

} // namespace Slang

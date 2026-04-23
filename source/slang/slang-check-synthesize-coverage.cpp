// slang-check-synthesize-coverage.cpp
//
// When `-trace-coverage` is on, the user is asking the compiler to
// instrument their shader with per-statement execution counters.
// The placeholder IR op that records each counter needs a buffer to
// write into at runtime; the *architecturally correct* way to model
// that buffer is as a synthetic global-scope shader parameter, so
// that it flows through the same parameter-binding, reflection, and
// layout pipeline as user-declared globals.
//
// This file runs at semantic-check time, between include/import
// resolution and the main decl-state iteration. It injects a
// `RWStructuredBuffer<uint> __slang_coverage` declaration into the
// module's scope if one isn't already present, marked with a
// `SynthesizedModifier` so downstream tooling can filter it.

#include "slang-check-synthesize-coverage.h"

#include "compiler-core/slang-name.h"
#include "slang-ast-builder.h"
#include "slang-ast-decl.h"
#include "slang-ast-modifier.h"
#include "slang-ast-type.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"

namespace Slang
{

// Well-known buffer name. Matches the IR pass that rewrites
// IncrementCoverageCounter placeholders and the `.slangcov` manifest
// format.
static const char* kCoverageBufferName = "__slang_coverage";

// Find an existing module-scope decl named `__slang_coverage`. If
// present, callers reuse it instead of synthesizing. Returns null if
// not found.
static VarDecl* findExistingCoverageDecl(ModuleDecl* moduleDecl)
{
    for (auto member : moduleDecl->getDirectMemberDecls())
    {
        auto varDecl = as<VarDecl>(member);
        if (!varDecl)
            continue;
        auto name = varDecl->getName();
        if (!name)
            continue;
        if (name->text == kCoverageBufferName)
            return varDecl;
    }
    return nullptr;
}

void maybeSynthesizeCoverageBufferDecl(SemanticsVisitor* visitor, ModuleDecl* moduleDecl)
{
    // Gate on the compile flag. If coverage isn't requested, this
    // entire function is a no-op — we don't want to perturb any
    // normal compile.
    if (!visitor->getLinkage()->m_optionSet.getBoolOption(CompilerOptionName::TraceCoverage))
        return;

    // Don't inject into the core module or other compiler-owned
    // modules; coverage is strictly a user-facing feature.
    if (isFromCoreModule(moduleDecl))
        return;

    // If the user already declared `__slang_coverage` themselves (or
    // a previous compile phase already synthesized it for this
    // module), don't duplicate.
    if (findExistingCoverageDecl(moduleDecl))
        return;

    auto astBuilder = getCurrentASTBuilder();

    // Build the type: `RWStructuredBuffer<uint>`.
    auto uintType = astBuilder->getUIntType();
    auto bufferType = astBuilder->getRWStructuredBufferType(uintType);

    // Build the VarDecl. Parent is the module itself; name matches
    // the well-known reserved identifier. A `SynthesizedModifier`
    // lets downstream tooling filter it out of user-facing views
    // (reflection, IDEs) without affecting correctness.
    auto varDecl = astBuilder->create<VarDecl>();
    varDecl->nameAndLoc.name = visitor->getNamePool()->getName(kCoverageBufferName);
    varDecl->parentDecl = moduleDecl;
    varDecl->type.type = bufferType;
    varDecl->loc = moduleDecl->loc;

    auto synthModifier = astBuilder->create<SynthesizedModifier>();
    addModifier(varDecl, synthModifier);

    // Attach the decl to the module. `addDirectMemberDecl` updates
    // the internal name-lookup table so subsequent phases find it
    // like any other declaration.
    moduleDecl->addDirectMemberDecl(varDecl);
}

} // namespace Slang

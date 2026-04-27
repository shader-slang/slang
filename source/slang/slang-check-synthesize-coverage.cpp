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
#include "compiler-core/slang-token.h"
#include "slang-ast-builder.h"
#include "slang-ast-decl.h"
#include "slang-ast-modifier.h"
#include "slang-ast-type.h"
#include "slang-check-impl.h"
#include "slang-compiler.h"

namespace Slang
{

// Well-known buffer name. Matches the IR pass that rewrites
// IncrementCoverageCounter ops and the coverage metadata / sidecar
// output that describes the buffer to hosts.
static const char* kCoverageBufferName = "__slang_coverage";

// Attach explicit binding modifiers to the synthesized coverage
// VarDecl so that parameter binding pins it to (uN, spaceM) on every
// target instead of auto-allocating. We attach both:
//   - HLSLRegisterSemantic — drives HLSL emit (`register(uN, spaceM)`)
//     and is also picked up by GLSL/SPIR-V parameter binding.
//   - GLSLBindingAttribute — equivalent of `[[vk::binding(N, M)]]`;
//     having it suppresses the "D3D register without Vulkan binding
//     or shift" diagnostic when emitting GLSL/SPIR-V.
// Both encode the same (index, space) pair, so neither target sees
// a conflict.
static void attachExplicitBindingSemantic(
    ASTBuilder* astBuilder,
    NamePool* namePool,
    VarDecl* varDecl,
    int bindingIndex,
    int bindingSpace,
    SourceLoc loc)
{
    auto reg = astBuilder->create<HLSLRegisterSemantic>();
    reg->loc = loc;

    // The Token consumer (extractHLSLLayoutSemanticInfo) splits text
    // like "u7" / "space3" into a class name and digit suffix. UAV
    // class is `u`; RWStructuredBuffer always lands there.
    StringBuilder regName;
    regName << "u" << bindingIndex;
    StringBuilder spaceName;
    spaceName << "space" << bindingSpace;

    // Anchor the Token text in the NamePool so its memory outlives
    // the StringBuilder.
    auto regNameObj = namePool->getName(regName.toString());
    auto spaceNameObj = namePool->getName(spaceName.toString());

    reg->registerName = Token(TokenType::Identifier, regNameObj, loc);
    reg->spaceName = Token(TokenType::Identifier, spaceNameObj, loc);

    addModifier(varDecl, reg);

    auto vkBinding = astBuilder->create<GLSLBindingAttribute>();
    vkBinding->loc = loc;
    vkBinding->binding = bindingIndex;
    vkBinding->set = bindingSpace;
    addModifier(varDecl, vkBinding);
}

// Find an existing module-scope decl named `__slang_coverage` in just
// this module's direct members.
static VarDecl* findExistingCoverageDeclInModule(ModuleDecl* moduleDecl)
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

// Walk this module + all transitively imported modules, looking for an
// existing `__slang_coverage` decl (user-declared or synthesized into
// an earlier-checked module). Imports are checked first so that when
// `import physics; import …` causes `physics.slang` to be checked
// before `simulate.slang`'s synthesizer runs, simulate.slang reuses
// physics's `__slang_coverage` instead of adding a second one whose
// explicit `register(uN, spaceM)` would collide on parameter binding
// (which surfaces as a D3D12 root-signature failure when
// `-trace-coverage-binding` is in use).
static VarDecl* findExistingCoverageDecl(
    ModuleDecl* moduleDecl,
    HashSet<ModuleDecl*>& visited)
{
    if (!visited.add(moduleDecl))
        return nullptr;
    if (auto existing = findExistingCoverageDeclInModule(moduleDecl))
        return existing;
    for (auto importDecl : moduleDecl->getMembersOfType<ImportDecl>())
    {
        if (auto imported = importDecl->importedModuleDecl)
        {
            if (auto existing = findExistingCoverageDecl(imported, visited))
                return existing;
        }
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
    // a previous compile phase already synthesized it for this module
    // or any transitively-imported module), don't duplicate. Imports
    // are processed before this hook (see `checkModule` in
    // slang-check-decl.cpp), so a leaf module like `physics.slang` will
    // already carry its synthesized `__slang_coverage` by the time the
    // importing module's synthesizer runs.
    HashSet<ModuleDecl*> visited;
    if (findExistingCoverageDecl(moduleDecl, visited))
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

    // Apply user-specified explicit binding (`-trace-coverage-binding
    // <index> <space>`) if requested. Without this the parameter
    // binding pass auto-allocates a slot, which is fine for hosts
    // that read the assignment back from the metadata but awkward
    // when the host needs the slot fixed at compile time (e.g. for a
    // pre-built D3D12 root signature).
    auto& opts = visitor->getLinkage()->m_optionSet;
    if (auto values = opts.options.tryGetValue(CompilerOptionName::TraceCoverageBinding))
    {
        if (values->getCount() > 0)
        {
            int bindingIndex = (*values)[0].intValue;
            int bindingSpace = (*values)[0].intValue2;
            attachExplicitBindingSemantic(
                astBuilder,
                visitor->getNamePool(),
                varDecl,
                bindingIndex,
                bindingSpace,
                moduleDecl->loc);
        }
    }

    // Attach the decl to the module. `addDirectMemberDecl` updates
    // the internal name-lookup table so subsequent phases find it
    // like any other declaration.
    moduleDecl->addDirectMemberDecl(varDecl);
}

} // namespace Slang

// unit-test-check-decl.cpp
//
// Tests that inspect the AST produced by the semantic checker.
//
// A `.slang` end-to-end test can only observe what the compiler reports or
// emits: diagnostics, and generated target code. It cannot look at the checked
// AST itself. So a claim like "the checker resolves each struct field to a type
// and preserves declaration order" is only indirectly testable there — you have
// to find some generated output whose shape happens to depend on it.
//
// Running in-process, a unit test can compile a module with the frontend only
// (no target code generation) and then walk the resulting `ModuleDecl`
// directly.

#include "slang/slang-ast-builder.h"
#include "slang/slang-check-impl.h"
#include "slang/slang-mangle.h"
#include "slang/slang-module.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

/// Find the first direct member of `containerDecl` of type `T` whose name matches.
template<typename T>
T* findMemberDecl(ContainerDecl* containerDecl, const char* name)
{
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto decl = as<T>(member);
        if (!decl || !decl->getName())
            continue;
        if (decl->getName()->text == name)
            return decl;
    }
    return nullptr;
}

/// Find the first direct inheritance declaration in `containerDecl`.
InheritanceDecl* findInheritanceDecl(ContainerDecl* containerDecl)
{
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        if (auto inheritanceDecl = as<InheritanceDecl>(member))
            return inheritanceDecl;
    }
    return nullptr;
}

/// Return whether `candidate` is a direct member of `containerDecl`.
bool isDirectMember(ContainerDecl* containerDecl, Decl* candidate)
{
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        if (member == candidate)
            return true;
    }
    return false;
}

} // namespace

// The checker produces a `ModuleDecl` whose direct members are the declarations
// written in the source.
SLANG_UNIT_TEST(checkedModuleExposesTopLevelDeclarations)
{
    StaticUnitTestEnv env(unitTestContext);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "checkedModuleExposesTopLevelDeclarations",
        "struct Point { float x; float y; }\n"
        "int addOne(int value) { return value + 1; }\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);

    ModuleDecl* moduleDecl = module->getModuleDecl();
    SLANG_CHECK_ABORT(moduleDecl != nullptr);

    SLANG_CHECK(findMemberDecl<StructDecl>(moduleDecl, "Point") != nullptr);
    SLANG_CHECK(findMemberDecl<FuncDecl>(moduleDecl, "addOne") != nullptr);
}

// Struct fields keep their source order and each is resolved to a type. Field
// order is observable in memory layout, so a checker that reordered members
// would corrupt any buffer written by a host application.
SLANG_UNIT_TEST(checkedStructPreservesFieldOrderAndTypes)
{
    StaticUnitTestEnv env(unitTestContext);

    Module* module = env.checkModuleFromSource(
        "checkedStructPreservesFieldOrderAndTypes",
        "struct Mixed { float first; int second; float third; }\n");
    SLANG_CHECK_ABORT(module != nullptr);

    StructDecl* structDecl = findMemberDecl<StructDecl>(module->getModuleDecl(), "Mixed");
    SLANG_CHECK_ABORT(structDecl != nullptr);

    ASTBuilder* astBuilder = env.getASTBuilder();
    Type* floatType = astBuilder->getFloatType();
    Type* intType = astBuilder->getIntType();

    List<String> fieldNames;
    List<Type*> fieldTypes;
    for (auto field : structDecl->getDirectMemberDeclsOfType<VarDecl>())
    {
        // Every field must have been resolved to a type by the checker; a null
        // type here means checking silently left the declaration incomplete.
        SLANG_CHECK_ABORT(field->getType() != nullptr);
        fieldTypes.add(field->getType());
        if (field->getName())
            fieldNames.add(field->getName()->text);
    }

    SLANG_CHECK_ABORT(fieldNames.getCount() == 3);
    SLANG_CHECK(fieldNames[0] == "first");
    SLANG_CHECK(fieldNames[1] == "second");
    SLANG_CHECK(fieldNames[2] == "third");

    // The declared types must survive checking in the same order. Asserting the
    // resolved types, rather than only that they are non-null, is what catches a
    // checker that pairs a field with the wrong declaration.
    SLANG_CHECK_ABORT(fieldTypes.getCount() == 3);
    SLANG_CHECK(fieldTypes[0]->equals(floatType));
    SLANG_CHECK(fieldTypes[1]->equals(intType));
    SLANG_CHECK(fieldTypes[2]->equals(floatType));
}

// A checked declaration can be mangled, and overloads that differ only in
// parameter types receive different mangled names. Mangling is what keeps
// overloads distinct across separately-compiled modules, so a collision here
// would let one overload satisfy a reference to the other.
SLANG_UNIT_TEST(checkedOverloadsMangleDistinctly)
{
    StaticUnitTestEnv env(unitTestContext);

    Module* module = env.checkModuleFromSource(
        "checkedOverloadsMangleDistinctly",
        "int overloaded(int value) { return value; }\n"
        "float overloaded(float value) { return value; }\n");
    SLANG_CHECK_ABORT(module != nullptr);

    ModuleDecl* moduleDecl = module->getModuleDecl();
    List<String> mangledNames;
    for (auto member : moduleDecl->getDirectMemberDecls())
    {
        auto funcDecl = as<FuncDecl>(member);
        if (!funcDecl || !funcDecl->getName())
            continue;
        if (funcDecl->getName()->text != "overloaded")
            continue;
        mangledNames.add(getMangledName(env.getASTBuilder(), funcDecl));
    }

    SLANG_CHECK_ABORT(mangledNames.getCount() == 2);
    SLANG_CHECK(mangledNames[0].getLength() > 0);
    SLANG_CHECK(mangledNames[0] != mangledNames[1]);
}

// A generic forwarding witness is intentionally left out of its parent's member list so ordinary
// lookup cannot expose it. The synthesized-declaration registry is the only whole-module path that
// advances both its outer generic signature and inner function through the normal semantic phases
// to `CapabilityChecked`.
SLANG_UNIT_TEST(detachedGenericWitnessReachesFinalCheckState)
{
    StaticUnitTestEnv env(unitTestContext);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "detachedGenericWitnessReachesFinalCheckState",
        "interface IBase {}\n"
        "interface IDerived : IBase {}\n"
        "interface IRequirement { int transform<T : IDerived>(T value); }\n"
        "struct Implementation : IRequirement\n"
        "{\n"
        "    int transform<T : IBase>(T value) { return 0; }\n"
        "}\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);

    auto moduleDecl = module->getModuleDecl();
    auto interfaceDecl = findMemberDecl<InterfaceDecl>(moduleDecl, "IRequirement");
    auto implementationDecl = findMemberDecl<StructDecl>(moduleDecl, "Implementation");
    SLANG_CHECK_ABORT(interfaceDecl != nullptr);
    SLANG_CHECK_ABORT(implementationDecl != nullptr);

    auto requiredGenericDecl = findMemberDecl<GenericDecl>(interfaceDecl, "transform");
    auto inheritanceDecl = findInheritanceDecl(implementationDecl);
    SLANG_CHECK_ABORT(requiredGenericDecl != nullptr);
    auto requirementDecl = as<FuncDecl>(requiredGenericDecl->inner);
    SLANG_CHECK_ABORT(requirementDecl != nullptr);
    SLANG_CHECK_ABORT(inheritanceDecl != nullptr);
    SLANG_CHECK_ABORT(inheritanceDecl->witnessTable != nullptr);

    RequirementWitness witness;
    SLANG_CHECK_ABORT(inheritanceDecl->witnessTable->tryGetRequirementWitness(
        InterfaceRequirementKey(requirementDecl),
        witness));
    SLANG_CHECK_ABORT(witness.getFlavor() == RequirementWitness::Flavor::declRef);

    auto synthesizedDecl = witness.getDeclRef().getDecl();
    SLANG_CHECK_ABORT(synthesizedDecl != nullptr);
    auto synthesizedGenericDecl = as<GenericDecl>(synthesizedDecl->parentDecl);
    SLANG_CHECK_ABORT(synthesizedGenericDecl != nullptr);
    SLANG_CHECK(synthesizedGenericDecl->parentDecl == implementationDecl);
    SLANG_CHECK(!isDirectMember(implementationDecl, synthesizedGenericDecl));
    SLANG_CHECK(synthesizedGenericDecl->isChecked(DeclCheckState::CapabilityChecked));
    SLANG_CHECK(synthesizedDecl->isChecked(DeclCheckState::CapabilityChecked));
}

// Registration is idempotent and persistent across phase drains. A root published after an early
// drain must catch up in the next drain, while a speculative candidate that is never published
// must remain untouched.
SLANG_UNIT_TEST(synthesizedDeclarationRegistryPersistsAcrossPhases)
{
    StaticUnitTestEnv env(unitTestContext);

    Module* module = env.checkModuleFromSource(
        "synthesizedDeclarationRegistryPersistsAcrossPhases",
        "int anchor() { return 0; }\n");
    SLANG_CHECK_ABORT(module != nullptr);

    SharedSemanticsContext shared(module->getLinkage(), module, nullptr);
    SemanticsVisitor visitor(&shared);
    auto astBuilder = env.getASTBuilder();
    auto moduleDecl = module->getModuleDecl();

    auto earlyRoot = astBuilder->create<EmptyDecl>();
    earlyRoot->parentDecl = moduleDecl;
    auto lateRoot = astBuilder->create<EmptyDecl>();
    lateRoot->parentDecl = moduleDecl;
    auto rejectedCandidate = astBuilder->create<EmptyDecl>();
    rejectedCandidate->parentDecl = moduleDecl;

    shared.registerSynthesizedDeclRoot(earlyRoot);
    shared.registerSynthesizedDeclRoot(earlyRoot);
    visitor.ensureRegisteredSynthesizedDecls(DeclCheckState::ReadyForLookup);

    SLANG_CHECK(earlyRoot->isChecked(DeclCheckState::ReadyForLookup));
    SLANG_CHECK(lateRoot->checkState.getState() == DeclCheckState::Unchecked);
    SLANG_CHECK(rejectedCandidate->checkState.getState() == DeclCheckState::Unchecked);

    shared.registerSynthesizedDeclRoot(lateRoot);
    shared.registerSynthesizedDeclRoot(lateRoot);
    visitor.ensureRegisteredSynthesizedDecls(DeclCheckState::DefinitionChecked);

    SLANG_CHECK(earlyRoot->isChecked(DeclCheckState::DefinitionChecked));
    SLANG_CHECK(lateRoot->isChecked(DeclCheckState::DefinitionChecked));
    SLANG_CHECK(rejectedCandidate->checkState.getState() == DeclCheckState::Unchecked);

    visitor.ensureRegisteredSynthesizedDecls(DeclCheckState::CapabilityChecked);

    SLANG_CHECK(earlyRoot->isChecked(DeclCheckState::CapabilityChecked));
    SLANG_CHECK(lateRoot->isChecked(DeclCheckState::CapabilityChecked));
    SLANG_CHECK(rejectedCandidate->checkState.getState() == DeclCheckState::Unchecked);
    SLANG_CHECK(shared.getSynthesizedDeclRootCount() == 2);
    SLANG_CHECK(shared.getSynthesizedDeclRoot(0) == earlyRoot);
    SLANG_CHECK(shared.getSynthesizedDeclRoot(1) == lateRoot);
}

// A registry drain must keep consuming roots appended by declarations it checks. This fixture
// registers a detached struct whose property conformance resolves storage through a transparent
// member and synthesizes another detached declaration. One drain must complete both roots.
SLANG_UNIT_TEST(synthesizedDeclarationRegistryDrainsNewRoots)
{
    StaticUnitTestEnv env(unitTestContext);

    Module* module = env.checkModuleFromSource(
        "synthesizedDeclarationRegistryDrainsNewRoots",
        "interface IStoredValue { property int value { get; set; } }\n"
        "struct StoredValue { int value; }\n");
    SLANG_CHECK_ABORT(module != nullptr);

    auto astBuilder = env.getASTBuilder();
    auto moduleDecl = module->getModuleDecl();
    auto interfaceDecl = findMemberDecl<InterfaceDecl>(moduleDecl, "IStoredValue");
    auto storedValueDecl = findMemberDecl<StructDecl>(moduleDecl, "StoredValue");
    SLANG_CHECK_ABORT(interfaceDecl != nullptr);
    SLANG_CHECK_ABORT(storedValueDecl != nullptr);

    auto synthesizedStruct = astBuilder->create<StructDecl>();
    synthesizedStruct->nameAndLoc.name = astBuilder->getNamePool()->getName("SynthesizedStorage");
    synthesizedStruct->parentDecl = moduleDecl;
    synthesizedStruct->ownedScope = astBuilder->create<Scope>();
    synthesizedStruct->ownedScope->containerDecl = synthesizedStruct;
    synthesizedStruct->ownedScope->parent = moduleDecl->ownedScope;

    auto inheritanceDecl = astBuilder->create<InheritanceDecl>();
    inheritanceDecl->base.type = DeclRefType::create(astBuilder, makeDeclRef(interfaceDecl));
    synthesizedStruct->addMember(inheritanceDecl);

    auto storageDecl = astBuilder->create<VarDecl>();
    storageDecl->nameAndLoc.name = astBuilder->getNamePool()->getName("storage");
    storageDecl->type.type = DeclRefType::create(astBuilder, makeDeclRef(storedValueDecl));
    addModifier(storageDecl, astBuilder->create<TransparentModifier>());
    synthesizedStruct->addMember(storageDecl);

    DiagnosticSink sink(module->getLinkage()->getSourceManager(), nullptr);
    SharedSemanticsContext shared(module->getLinkage(), module, &sink);
    SemanticsVisitor visitor(&shared);
    shared.registerSynthesizedDeclRoot(synthesizedStruct);
    SLANG_CHECK_ABORT(shared.getSynthesizedDeclRootCount() == 1);

    // This is the exact phase that synthesizes and registers the property. Stopping here excludes
    // later conformance-table traversal as a second path that could advance the new root.
    visitor.ensureRegisteredSynthesizedDecls(DeclCheckState::ReadyForConformances);

    SLANG_CHECK_ABORT(shared.getSynthesizedDeclRootCount() == 2);
    auto synthesizedProperty = shared.getSynthesizedDeclRoot(1);
    SLANG_CHECK_ABORT(as<PropertyDecl>(synthesizedProperty) != nullptr);
    SLANG_CHECK(synthesizedProperty->parentDecl == synthesizedStruct);
    SLANG_CHECK(!isDirectMember(synthesizedStruct, synthesizedProperty));
    SLANG_CHECK(synthesizedStruct->checkState.getState() == DeclCheckState::ReadyForConformances);
    SLANG_CHECK(synthesizedProperty->checkState.getState() == DeclCheckState::ReadyForConformances);
    SLANG_CHECK(sink.getErrorCount() == 0);
}

// Source that fails to check reports a diagnostic rather than returning a
// module. This pins the contract the other tests here depend on: a non-null
// module means checking actually succeeded.
SLANG_UNIT_TEST(checkedModuleReportsDiagnosticOnInvalidSource)
{
    StaticUnitTestEnv env(unitTestContext);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "checkedModuleReportsDiagnosticOnInvalidSource",
        "int broken() { return undefinedIdentifier; }\n",
        &diagnostics);

    SLANG_CHECK(module == nullptr);
    SLANG_CHECK(diagnostics.getLength() > 0);
}

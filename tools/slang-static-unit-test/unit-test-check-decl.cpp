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
#include "slang/slang-lookup.h"
#include "slang/slang-mangle.h"
#include "slang/slang-module.h"
#include "slang/slang-syntax.h"
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

/// Checks one `Receiver.method` fixture in isolation, verifies its effective receiver mode, and
/// returns its mangled name.
String checkAndGetReceiverMethodMangledName(
    UnitTestContext* unitTestContext,
    const char* moduleName,
    const char* sourcePath,
    const char* source,
    ParamPassingMode expectedMode)
{
    StaticUnitTestEnv env(unitTestContext);
    String diagnostics;
    Module* module = env.checkModuleFromSourceAtPath(moduleName, sourcePath, source, &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);

    auto receiverDecl = findMemberDecl<StructDecl>(module->getModuleDecl(), "Receiver");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);

    auto methodDecl = findMemberDecl<FuncDecl>(receiverDecl, "method");
    SLANG_CHECK_ABORT(methodDecl != nullptr);

    auto thisParamInfo = methodDecl->findModifier<ThisParamInfoAttribute>();
    SLANG_CHECK_ABORT(thisParamInfo != nullptr);
    SLANG_CHECK(thisParamInfo->info.mode == expectedMode);

    return getMangledName(env.getASTBuilder(), methodDecl);
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

// The parser creates a generic scope before it links the declaration wrapped by that generic.
// That incomplete scope cannot carry a receiver value. Once linked,
// `isReceiverlessDeclarationBoundary` classifies the wrapper from its inner declaration: it
// preserves the enclosing receiver for an instance method, but treats a static method or nested
// `FuncDecl` as a receiverless boundary.
SLANG_UNIT_TEST(receiverlessBoundaryHandlesIncompleteGenericDecl)
{
    StaticUnitTestEnv env(unitTestContext);
    auto astBuilder = env.getASTBuilder();

    auto structDecl = astBuilder->create<StructDecl>();
    auto genericDecl = astBuilder->create<GenericDecl>();
    genericDecl->parentDecl = structDecl;

    SLANG_CHECK(isReceiverlessDeclarationBoundary(genericDecl));

    auto instanceMethodDecl = astBuilder->create<FuncDecl>();
    instanceMethodDecl->parentDecl = genericDecl;
    genericDecl->inner = instanceMethodDecl;
    SLANG_CHECK(!isReceiverlessDeclarationBoundary(genericDecl));

    auto localFunctionDecl = astBuilder->create<FuncDecl>();
    localFunctionDecl->parentDecl = instanceMethodDecl;
    SLANG_CHECK(isReceiverlessDeclarationBoundary(localFunctionDecl));

    auto staticMethodDecl = astBuilder->create<FuncDecl>();
    staticMethodDecl->parentDecl = genericDecl;
    addModifier(staticMethodDecl, astBuilder->create<HLSLStaticModifier>());
    genericDecl->inner = staticMethodDecl;
    SLANG_CHECK(isReceiverlessDeclarationBoundary(genericDecl));

    auto nestedStructDecl = astBuilder->create<StructDecl>();
    nestedStructDecl->parentDecl = genericDecl;
    genericDecl->inner = nestedStructDecl;
    SLANG_CHECK(isReceiverlessDeclarationBoundary(genericDecl));
}

// A lookup result can outlive the expression that initiated lookup, particularly while an overload
// set is being refined. Its `ThisValue` breadcrumb must therefore retain the lexical scope needed
// to reconstruct and check the implicit `this` base.
SLANG_UNIT_TEST(thisValueBreadcrumbPreservesLookupOriginScope)
{
    StaticUnitTestEnv env(unitTestContext);
    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "thisValueBreadcrumbPreservesLookupOriginScope",
        "struct Receiver { int field; int read() { return field; } }\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);

    auto receiverDecl = findMemberDecl<StructDecl>(module->getModuleDecl(), "Receiver");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);
    auto methodDecl = findMemberDecl<FuncDecl>(receiverDecl, "read");
    SLANG_CHECK_ABORT(methodDecl != nullptr);
    SLANG_CHECK_ABORT(methodDecl->ownedScope != nullptr);

    auto astBuilder = env.getASTBuilder();
    auto lookupResult = lookUp(
        astBuilder,
        nullptr,
        astBuilder->getNamePool()->getName("field"),
        methodDecl->ownedScope);
    SLANG_CHECK_ABORT(lookupResult.isValid());
    SLANG_CHECK(!lookupResult.isOverloaded());

    auto breadcrumb = lookupResult.item.breadcrumbs;
    SLANG_CHECK_ABORT(breadcrumb != nullptr);
    SLANG_CHECK(breadcrumb->getKind() == LookupResultItem::Breadcrumb::Kind::ThisValue);
    SLANG_CHECK(breadcrumb->getLookupOriginScope() == methodDecl->ownedScope);
}

// Lookup can be used without a semantic visitor after declarations have been checked. Crossing a
// static method must still remove the receiver value while preserving the enclosing receiver type.
SLANG_UNIT_TEST(nullSemanticsLookupClassifiesStaticMethodReceiverBoundary)
{
    StaticUnitTestEnv env(unitTestContext);
    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "nullSemanticsLookupClassifiesStaticMethodReceiverBoundary",
        "struct Receiver { int field; static int read() { return 0; } }\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);

    auto receiverDecl = findMemberDecl<StructDecl>(module->getModuleDecl(), "Receiver");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);
    auto methodDecl = findMemberDecl<FuncDecl>(receiverDecl, "read");
    SLANG_CHECK_ABORT(methodDecl != nullptr);
    SLANG_CHECK_ABORT(methodDecl->ownedScope != nullptr);

    auto astBuilder = env.getASTBuilder();
    auto lookupResult = lookUp(
        astBuilder,
        nullptr,
        astBuilder->getNamePool()->getName("field"),
        methodDecl->ownedScope);
    SLANG_CHECK_ABORT(lookupResult.isValid());
    SLANG_CHECK(!lookupResult.isOverloaded());

    auto breadcrumb = lookupResult.item.breadcrumbs;
    SLANG_CHECK_ABORT(breadcrumb != nullptr);
    SLANG_CHECK(breadcrumb->getKind() == LookupResultItem::Breadcrumb::Kind::ThisType);
}

// `checkGenericAppWithCheckedArgs()` calls `CompleteOverloadCandidate()` for each distributed
// candidate of an overloaded generic static requirement. Each completion must build an independent
// binding wrapper rather than repurpose one retained by `OverloadedExpr2`. The selected declaration
// reference must retain the binding on which its semantic type depends, while each static-member
// leaf retains only the qualifier's semantic type.
SLANG_UNIT_TEST(selectedStaticMemberDoesNotRewriteSharedQualifierBindings)
{
    StaticUnitTestEnv env(unitTestContext);
    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "selectedStaticMemberDoesNotRewriteSharedQualifierBindings",
        R"(
            interface IFactoryResult
            {
                static int getValue<T>(T value);
                static int getValue<T>(T first, T second);
            }

            IFactoryResult makeResult();

            int useResult()
            {
                return makeResult().getValue<int>(1);
            }
        )",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);

    auto useResultDecl = findMemberDecl<FuncDecl>(module->getModuleDecl(), "useResult");
    SLANG_CHECK_ABORT(useResultDecl != nullptr);
    auto body = as<BlockStmt>(useResultDecl->body);
    SLANG_CHECK_ABORT(body != nullptr);
    auto returnStmt = as<ReturnStmt>(body->body);
    SLANG_CHECK_ABORT(returnStmt != nullptr);
    auto invokeExpr = as<InvokeExpr>(returnStmt->expression);
    SLANG_CHECK_ABORT(invokeExpr != nullptr);

    List<Expr*> selectedWrappers;
    auto selectedMemberExpr = as<StaticMemberExpr>(
        getCallableExprThroughTransparentWrappers(invokeExpr->functionExpr, &selectedWrappers));
    SLANG_CHECK_ABORT(selectedMemberExpr != nullptr);
    LetExpr* selectedBindings = nullptr;
    for (auto wrapper : selectedWrappers)
    {
        if (auto binding = as<LetExpr>(wrapper))
        {
            SLANG_CHECK_ABORT(selectedBindings == nullptr);
            selectedBindings = binding;
        }
    }
    SLANG_CHECK_ABORT(selectedBindings != nullptr);

    auto dependencies =
        selectedMemberExpr->declRef.declRefBase->collectSemanticDeclarationDependencies();
    bool foundSelectedBinding = false;
    for (const auto& dependency : dependencies)
    {
        if (dependency.getDeclaration() == selectedBindings->decl)
        {
            foundSelectedBinding = true;
            break;
        }
    }
    SLANG_CHECK(foundSelectedBinding);

    auto originalOverloads = as<OverloadedExpr2>(invokeExpr->originalFunctionExpr);
    SLANG_CHECK_ABORT(originalOverloads != nullptr);
    SLANG_CHECK(originalOverloads->candidateExprs.getCount() == 2);

    HashSet<LetExpr*> candidateBindings;
    for (auto candidateExpr : originalOverloads->candidateExprs)
    {
        List<Expr*> candidateWrappers;
        auto candidateMemberExpr = as<StaticMemberExpr>(
            getCallableExprThroughTransparentWrappers(candidateExpr, &candidateWrappers));
        SLANG_CHECK_ABORT(candidateMemberExpr != nullptr);

        LetExpr* bindings = nullptr;
        for (auto wrapper : candidateWrappers)
        {
            if (auto binding = as<LetExpr>(wrapper))
            {
                SLANG_CHECK_ABORT(bindings == nullptr);
                bindings = binding;
            }
        }
        SLANG_CHECK_ABORT(bindings != nullptr);
        SLANG_CHECK(bindings != selectedBindings);
        SLANG_CHECK(candidateBindings.add(bindings));
        SLANG_CHECK(bindings->decl == selectedBindings->decl);
        SLANG_CHECK(bindings->decl->initExpr == selectedBindings->decl->initExpr);

        SLANG_CHECK(candidateMemberExpr != selectedMemberExpr);
        auto semanticBaseExpr = as<SharedTypeExpr>(candidateMemberExpr->baseExpression);
        SLANG_CHECK_ABORT(semanticBaseExpr != nullptr);
        SLANG_CHECK(semanticBaseExpr->base.type != nullptr);
        SLANG_CHECK(semanticBaseExpr->base.exp == nullptr);
    }
}

// When lookup finds a single generic static requirement, `checkGenericAppWithCheckedArgs()`
// receives an already-constructed expression instead of an overload-set item.
// `_prepareSelectedDeclRefExpr()` must therefore retain the qualifier binding required by the
// selected inner declaration in the unspecialized generic wrapper.
SLANG_UNIT_TEST(selectedUniqueStaticGenericRetainsQualifierBinding)
{
    StaticUnitTestEnv env(unitTestContext);
    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "selectedUniqueStaticGenericRetainsQualifierBinding",
        R"(
            interface IFactoryResult
            {
                static int getValue<T>(T value);
            }

            IFactoryResult makeResult();

            int useResult()
            {
                return makeResult().getValue<int>(1);
            }
        )",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);

    auto useResultDecl = findMemberDecl<FuncDecl>(module->getModuleDecl(), "useResult");
    SLANG_CHECK_ABORT(useResultDecl != nullptr);
    auto body = as<BlockStmt>(useResultDecl->body);
    SLANG_CHECK_ABORT(body != nullptr);
    auto returnStmt = as<ReturnStmt>(body->body);
    SLANG_CHECK_ABORT(returnStmt != nullptr);
    auto invokeExpr = as<InvokeExpr>(returnStmt->expression);
    SLANG_CHECK_ABORT(invokeExpr != nullptr);

    List<Expr*> selectedWrappers;
    auto selectedMemberExpr = as<StaticMemberExpr>(
        getCallableExprThroughTransparentWrappers(invokeExpr->functionExpr, &selectedWrappers));
    SLANG_CHECK_ABORT(selectedMemberExpr != nullptr);

    LetExpr* selectedBinding = nullptr;
    for (auto wrapper : selectedWrappers)
    {
        if (auto binding = as<LetExpr>(wrapper))
        {
            SLANG_CHECK_ABORT(selectedBinding == nullptr);
            selectedBinding = binding;
        }
    }
    SLANG_CHECK_ABORT(selectedBinding != nullptr);

    bool foundSelectedBinding = false;
    for (const auto& dependency :
         selectedMemberExpr->declRef.declRefBase->collectSemanticDeclarationDependencies())
    {
        if (dependency.getDeclaration() == selectedBinding->decl)
        {
            foundSelectedBinding = true;
            break;
        }
    }
    SLANG_CHECK(foundSelectedBinding);

    auto semanticBaseExpr = as<SharedTypeExpr>(selectedMemberExpr->baseExpression);
    SLANG_CHECK_ABORT(semanticBaseExpr != nullptr);
    SLANG_CHECK(semanticBaseExpr->base.type != nullptr);
    SLANG_CHECK(semanticBaseExpr->base.exp == nullptr);
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

// The HLSL-default change must not rename existing Slang declarations. Programmer-written
// declarations retain the established `[mutating]` and `[__ref]` suffixes; setters, `[constref]`,
// and a non-copyable receiver's borrowing default retain their suffix-free names.
SLANG_UNIT_TEST(checkedThisParamManglingPreservesSlangNames)
{
    StaticUnitTestEnv env(unitTestContext);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "checkedThisParamManglingPreservesSlangNames",
        "struct Receiver\n"
        "{\n"
        "    int inMethod(int value) { return value; }\n"
        "    [constref] int borrowMethod(int value) { return value; }\n"
        "    [mutating] int mutatingMethod(int value) { return value; }\n"
        "    [__ref] int refMethod(int value) { return value; }\n"
        "    property int item { get { return 0; } set {} }\n"
        "}\n"
        "[__NonCopyableType]\n"
        "struct NonCopyableReceiver\n"
        "{\n"
        "    int defaultBorrow() { return 0; }\n"
        "}\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);

    auto moduleDecl = module->getModuleDecl();
    auto receiverDecl = findMemberDecl<StructDecl>(moduleDecl, "Receiver");
    auto nonCopyableReceiverDecl = findMemberDecl<StructDecl>(moduleDecl, "NonCopyableReceiver");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);
    SLANG_CHECK_ABORT(nonCopyableReceiverDecl != nullptr);

    auto inMethod = findMemberDecl<FuncDecl>(receiverDecl, "inMethod");
    auto borrowMethod = findMemberDecl<FuncDecl>(receiverDecl, "borrowMethod");
    auto mutatingMethod = findMemberDecl<FuncDecl>(receiverDecl, "mutatingMethod");
    auto refMethod = findMemberDecl<FuncDecl>(receiverDecl, "refMethod");
    auto propertyDecl = findMemberDecl<PropertyDecl>(receiverDecl, "item");
    auto defaultBorrow = findMemberDecl<FuncDecl>(nonCopyableReceiverDecl, "defaultBorrow");
    SLANG_CHECK_ABORT(inMethod != nullptr);
    SLANG_CHECK_ABORT(borrowMethod != nullptr);
    SLANG_CHECK_ABORT(mutatingMethod != nullptr);
    SLANG_CHECK_ABORT(refMethod != nullptr);
    SLANG_CHECK_ABORT(propertyDecl != nullptr);
    SLANG_CHECK_ABORT(defaultBorrow != nullptr);

    SetterDecl* setterDecl = nullptr;
    for (auto member : propertyDecl->getDirectMemberDecls())
    {
        if (auto setter = as<SetterDecl>(member))
        {
            setterDecl = setter;
            break;
        }
    }
    SLANG_CHECK_ABORT(setterDecl != nullptr);

    auto astBuilder = env.getASTBuilder();
    SLANG_CHECK(getMangledName(astBuilder, inMethod).endsWith("ii"));
    SLANG_CHECK(getMangledName(astBuilder, borrowMethod).endsWith("ii"));
    SLANG_CHECK(getMangledName(astBuilder, mutatingMethod).endsWith("m"));
    SLANG_CHECK(getMangledName(astBuilder, refMethod).endsWith("r"));
    SLANG_CHECK(!getMangledName(astBuilder, setterDecl).endsWith("m"));
    SLANG_CHECK(!getMangledName(astBuilder, defaultBorrow).endsWith("c"));
}

// The new implicit HLSL `inout` default must have the same ABI name as explicit `[mutating]`.
// Existing Slang declarations retain their established names, including the shared suffix-free
// spelling for `in`, `[constref]`, and the read-only form selected by HLSL trailing `const`.
SLANG_UNIT_TEST(hlslDefaultThisManglingMatchesExplicitMutating)
{
    const char* moduleName = "hlslDefaultThisManglingMatchesExplicitMutating";
    const char* hlslSourcePath = "hlslDefaultThisManglingMatchesExplicitMutating.hlsl";
    const char* slangSourcePath = "hlslDefaultThisManglingMatchesExplicitMutating.slang";

    // The test helper selects the dialect for each in-memory program from the input path's
    // extension. Because the helper uses an isolated environment for each fixture, every fixture
    // can use the same module, type, and method names; each comparison therefore measures only
    // receiver semantics.
    String implicitMutatingName = checkAndGetReceiverMethodMangledName(
        unitTestContext,
        moduleName,
        hlslSourcePath,
        "struct Receiver { int method(int value) { return value; } };\n",
        ParamPassingMode::BorrowInOut);

    String explicitMutatingName = checkAndGetReceiverMethodMangledName(
        unitTestContext,
        moduleName,
        hlslSourcePath,
        "struct Receiver { [mutating] int method(int value) { return value; } };\n",
        ParamPassingMode::BorrowInOut);

    String trailingConstName = checkAndGetReceiverMethodMangledName(
        unitTestContext,
        moduleName,
        hlslSourcePath,
        "struct Receiver { int method(int value) const { return value; } };\n",
        ParamPassingMode::In);

    String explicitNonmutatingName = checkAndGetReceiverMethodMangledName(
        unitTestContext,
        moduleName,
        hlslSourcePath,
        "struct Receiver { [nonmutating] int method(int value) { return value; } };\n",
        ParamPassingMode::In);

    String implicitInName = checkAndGetReceiverMethodMangledName(
        unitTestContext,
        moduleName,
        slangSourcePath,
        "struct Receiver { int method(int value) { return value; } };\n",
        ParamPassingMode::In);

    String borrowName = checkAndGetReceiverMethodMangledName(
        unitTestContext,
        moduleName,
        slangSourcePath,
        "struct Receiver { [constref] int method(int value) { return value; } };\n",
        ParamPassingMode::BorrowIn);

    String refName = checkAndGetReceiverMethodMangledName(
        unitTestContext,
        moduleName,
        slangSourcePath,
        "struct Receiver { [__ref] int method(int value) { return value; } };\n",
        ParamPassingMode::Ref);

    SLANG_CHECK(implicitMutatingName == explicitMutatingName);
    SLANG_CHECK(trailingConstName == explicitNonmutatingName);
    SLANG_CHECK(trailingConstName == implicitInName);
    SLANG_CHECK(borrowName == implicitInName);

    String distinctNames[] = {implicitInName, implicitMutatingName, refName};
    for (Index i = 0; i < SLANG_COUNT_OF(distinctNames); ++i)
    {
        for (Index j = i + 1; j < SLANG_COUNT_OF(distinctNames); ++j)
            SLANG_CHECK(distinctNames[i] != distinctNames[j]);
    }
}

// Receiver-mode mangling does not alter receiverless callables or direct function-type
// declarations.
SLANG_UNIT_TEST(thisParamManglingLeavesReceiverlessAndDirectFunctionTypesUnchanged)
{
    StaticUnitTestEnv env(unitTestContext);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "thisParamManglingLeavesReceiverlessAndDirectFunctionTypesUnchanged",
        "struct Receiver\n"
        "{\n"
        "    [NoDiffThis] static int staticNoDiffThis(int value) { return value; }\n"
        "}\n"
        "interface DirectRequirement\n"
        "{\n"
        "    [nonmutating] __associatedfunc functype(int) -> int directIn;\n"
        "    [mutating] __associatedfunc functype(int) -> int directInOut;\n"
        "    [constref] __associatedfunc functype(int) -> int directBorrowIn;\n"
        "    [__ref] __associatedfunc functype(int) -> int directRef;\n"
        "}\n"
        "[NoDiffThis] int freeNoDiffThis(int value) { return value; }\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);

    auto moduleDecl = module->getModuleDecl();
    auto receiverDecl = findMemberDecl<StructDecl>(moduleDecl, "Receiver");
    auto interfaceDecl = findMemberDecl<InterfaceDecl>(moduleDecl, "DirectRequirement");
    auto freeNoDiffThis = findMemberDecl<FuncDecl>(moduleDecl, "freeNoDiffThis");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);
    SLANG_CHECK_ABORT(interfaceDecl != nullptr);
    SLANG_CHECK_ABORT(freeNoDiffThis != nullptr);

    auto staticNoDiffThis = findMemberDecl<FuncDecl>(receiverDecl, "staticNoDiffThis");
    auto directIn = findMemberDecl<FuncDecl>(interfaceDecl, "directIn");
    auto directInOut = findMemberDecl<FuncDecl>(interfaceDecl, "directInOut");
    auto directBorrowIn = findMemberDecl<FuncDecl>(interfaceDecl, "directBorrowIn");
    auto directRef = findMemberDecl<FuncDecl>(interfaceDecl, "directRef");
    SLANG_CHECK_ABORT(staticNoDiffThis != nullptr);
    SLANG_CHECK_ABORT(directIn != nullptr);
    SLANG_CHECK_ABORT(directInOut != nullptr);
    SLANG_CHECK_ABORT(directBorrowIn != nullptr);
    SLANG_CHECK_ABORT(directRef != nullptr);

    auto astBuilder = env.getASTBuilder();
    struct DirectFunctionTypeCase
    {
        FuncDecl* decl;
        ParamPassingMode receiverMode;
    };
    DirectFunctionTypeCase directFunctionTypeCases[] = {
        {directIn, ParamPassingMode::In},
        {directInOut, ParamPassingMode::BorrowInOut},
        {directBorrowIn, ParamPassingMode::BorrowIn},
        {directRef, ParamPassingMode::Ref},
    };
    for (const auto& directFunctionTypeCase : directFunctionTypeCases)
    {
        auto thisParamInfo = findEffectiveThisParamInfo(
            astBuilder,
            makeDeclRef(directFunctionTypeCase.decl));
        SLANG_CHECK_ABORT(thisParamInfo.has_value());
        SLANG_CHECK(thisParamInfo->mode == directFunctionTypeCase.receiverMode);
        SLANG_CHECK(getMangledName(astBuilder, directFunctionTypeCase.decl).endsWith("B"));
    }
    SLANG_CHECK(getMangledName(astBuilder, freeNoDiffThis).endsWith("n"));
    SLANG_CHECK(getMangledName(astBuilder, staticNoDiffThis).endsWith("n"));
}

// Synthesized witness wrappers use the checked effective `this` parameter mode as part of their
// internal symbol identity. This keeps wrappers with different calling conventions distinct when
// linking coalesces declarations with the same mangled name.
SLANG_UNIT_TEST(synthesizedEffectiveThisManglingEncodesMode)
{
    StaticUnitTestEnv env(unitTestContext);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "synthesizedEffectiveThisManglingEncodesMode",
        "struct Receiver { int method() { return 0; } }\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);

    auto receiverDecl = findMemberDecl<StructDecl>(module->getModuleDecl(), "Receiver");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);
    auto methodDecl = findMemberDecl<FuncDecl>(receiverDecl, "method");
    SLANG_CHECK_ABORT(methodDecl != nullptr);
    auto thisParamInfo = methodDecl->findModifier<ThisParamInfoAttribute>();
    SLANG_CHECK_ABORT(thisParamInfo != nullptr);

    auto astBuilder = env.getASTBuilder();
    auto unsynthesizedName = getMangledName(astBuilder, methodDecl);
    auto synthesizedMode = astBuilder->create<SynthesizedParamPassingModeModifier>();
    addModifier(methodDecl, synthesizedMode);

    struct ModeCase
    {
        ParamPassingMode mode;
        char const* suffix;
    };
    ModeCase cases[] = {
        {ParamPassingMode::In, "ti_"},
        {ParamPassingMode::BorrowInOut, "tio_"},
        {ParamPassingMode::BorrowIn, "tc_"},
        {ParamPassingMode::Ref, "tr_"},
    };

    List<String> mangledNames;
    for (auto modeCase : cases)
    {
        synthesizedMode->mode = modeCase.mode;
        thisParamInfo->info.mode = modeCase.mode;
        auto mangledName = getMangledName(astBuilder, methodDecl);
        SLANG_CHECK(mangledName.endsWith(modeCase.suffix));
        SLANG_CHECK(mangledName != unsynthesizedName);
        for (auto previousName : mangledNames)
            SLANG_CHECK(mangledName != previousName);
        mangledNames.add(mangledName);
    }
}

// `ParamInfo` and a type carrying a parameter-passing-mode wrapper are two encodings of the same
// information. The wrapper represents only the mode; semantic modifiers such as `no_diff` remain
// part of the value type in both directions.
SLANG_UNIT_TEST(paramInfoWrappedTypeRoundTrips)
{
    StaticUnitTestEnv env(unitTestContext);
    auto astBuilder = env.getASTBuilder();
    auto noDiffIntType =
        astBuilder->getModifiedType(astBuilder->getIntType(), astBuilder->getNoDiffModifierVal());

    ParamPassingMode modes[] = {
        ParamPassingMode::In,
        ParamPassingMode::Out,
        ParamPassingMode::BorrowInOut,
        ParamPassingMode::BorrowIn,
        ParamPassingMode::Ref,
    };
    for (auto mode : modes)
    {
        ParamInfo original;
        original.type = noDiffIntType;
        original.mode = mode;

        auto wrappedType = getParamTypeWithModeWrapper(astBuilder, original);
        auto decoded = getParamInfoFromTypeWithModeWrapper(wrappedType);

        SLANG_CHECK(decoded.mode == original.mode);
        SLANG_CHECK(decoded.type->equals(original.type));
        SLANG_CHECK(doesTypeHaveNoDiffModifier(decoded.type));
    }
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

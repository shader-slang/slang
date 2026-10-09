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

// `SynthesizedParamPassingModeModifier` is a complete source of receiver-mode policy. An ad hoc
// semantics context used by specialization has no translation unit from which to infer a
// source-language default, so receiver checking must consume that fixed mode without asking for a
// fallback first.
SLANG_UNIT_TEST(synthesizedEffectiveThisUsesFixedModeWithoutTranslationUnit)
{
    StaticUnitTestEnv env(unitTestContext);

    Module* module = env.checkModuleFromSource(
        "synthesizedEffectiveThisUsesFixedModeWithoutTranslationUnit",
        "struct Receiver {}\n");
    SLANG_CHECK_ABORT(module != nullptr);

    auto receiverDecl = findMemberDecl<StructDecl>(module->getModuleDecl(), "Receiver");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);

    auto astBuilder = env.getASTBuilder();
    auto synthesizedMethod = astBuilder->create<FuncDecl>();
    synthesizedMethod->parentDecl = receiverDecl;
    auto synthesizedMode = astBuilder->create<SynthesizedParamPassingModeModifier>();
    synthesizedMode->mode = ParamPassingMode::BorrowInOut;
    addModifier(synthesizedMethod, synthesizedMode);

    DiagnosticSink sink(module->getLinkage()->getSourceManager(), nullptr);
    SharedSemanticsContext shared(
        module->getLinkage(),
        module->getModuleDecl()->languageVersion,
        &sink);
    SemanticsVisitor visitor(&shared);
    visitor.checkAndAttachEffectiveThisParamInfo(synthesizedMethod);

    auto thisParamInfo = synthesizedMethod->findModifier<ThisParamInfoAttribute>();
    SLANG_CHECK_ABORT(thisParamInfo != nullptr);
    SLANG_CHECK(thisParamInfo->info.mode == ParamPassingMode::BorrowInOut);
    SLANG_CHECK(sink.getErrorCount() == 0);
}

// Source mangling preserves the legacy receiver-mode suffixes: only methods explicitly marked
// `[mutating]` or `[__ref]` had suffixes. Borrowed methods, writable setters, and direct
// function-type declarations remain unsuffixed. Neither HLSL's writable default nor trailing
// `const` adds a suffix because receiver mode is not source overload identity.
SLANG_UNIT_TEST(checkedEffectiveThisManglingPreservesLegacySuffixes)
{
    StaticUnitTestEnv env(unitTestContext);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "checkedEffectiveThisManglingPreservesLegacySuffixes",
        "struct Receiver\n"
        "{\n"
        "    [constref] int ordinaryBorrow(int value) { return value; }\n"
        "    [mutating] int ordinaryMutating(int value) { return value; }\n"
        "    [__ref] int ordinaryRef(int value) { return value; }\n"
        "    [NoDiffThis] static int staticNoDiffThis(int value) { return value; }\n"
        "    property int item { get { return 0; } set {} }\n"
        "}\n"
        "[__NonCopyableType]\n"
        "struct NonCopyableReceiver\n"
        "{\n"
        "    int defaultBorrow() { return 0; }\n"
        "}\n"
        "interface DirectRequirement\n"
        "{\n"
        "    [constref] __associatedfunc functype(int) -> int directBorrow;\n"
        "    [mutating] __associatedfunc functype(int) -> int directMutating;\n"
        "    [__ref] __associatedfunc functype(int) -> int directRef;\n"
        "}\n"
        "[NoDiffThis] int freeNoDiffThis(int value) { return value; }\n",
        &diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);

    auto moduleDecl = module->getModuleDecl();
    auto receiverDecl = findMemberDecl<StructDecl>(moduleDecl, "Receiver");
    auto nonCopyableReceiverDecl = findMemberDecl<StructDecl>(moduleDecl, "NonCopyableReceiver");
    auto interfaceDecl = findMemberDecl<InterfaceDecl>(moduleDecl, "DirectRequirement");
    auto freeNoDiffThis = findMemberDecl<FuncDecl>(moduleDecl, "freeNoDiffThis");
    SLANG_CHECK_ABORT(receiverDecl != nullptr);
    SLANG_CHECK_ABORT(nonCopyableReceiverDecl != nullptr);
    SLANG_CHECK_ABORT(interfaceDecl != nullptr);
    SLANG_CHECK_ABORT(freeNoDiffThis != nullptr);

    auto ordinaryBorrow = findMemberDecl<FuncDecl>(receiverDecl, "ordinaryBorrow");
    auto ordinaryMutating = findMemberDecl<FuncDecl>(receiverDecl, "ordinaryMutating");
    auto ordinaryRef = findMemberDecl<FuncDecl>(receiverDecl, "ordinaryRef");
    auto staticNoDiffThis = findMemberDecl<FuncDecl>(receiverDecl, "staticNoDiffThis");
    auto propertyDecl = findMemberDecl<PropertyDecl>(receiverDecl, "item");
    auto defaultBorrow = findMemberDecl<FuncDecl>(nonCopyableReceiverDecl, "defaultBorrow");
    auto directBorrow = findMemberDecl<FuncDecl>(interfaceDecl, "directBorrow");
    auto directMutating = findMemberDecl<FuncDecl>(interfaceDecl, "directMutating");
    auto directRef = findMemberDecl<FuncDecl>(interfaceDecl, "directRef");
    SLANG_CHECK_ABORT(ordinaryBorrow != nullptr);
    SLANG_CHECK_ABORT(ordinaryMutating != nullptr);
    SLANG_CHECK_ABORT(ordinaryRef != nullptr);
    SLANG_CHECK_ABORT(staticNoDiffThis != nullptr);
    SLANG_CHECK_ABORT(propertyDecl != nullptr);
    SLANG_CHECK_ABORT(defaultBorrow != nullptr);
    SLANG_CHECK_ABORT(directBorrow != nullptr);
    SLANG_CHECK_ABORT(directMutating != nullptr);
    SLANG_CHECK_ABORT(directRef != nullptr);

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
    SLANG_CHECK(getMangledName(astBuilder, freeNoDiffThis).endsWith("n"));
    SLANG_CHECK(getMangledName(astBuilder, ordinaryBorrow).endsWith("ii"));
    SLANG_CHECK(getMangledName(astBuilder, ordinaryMutating).endsWith("m"));
    SLANG_CHECK(getMangledName(astBuilder, ordinaryRef).endsWith("r"));
    SLANG_CHECK(getMangledName(astBuilder, staticNoDiffThis).endsWith("n"));
    SLANG_CHECK(!getMangledName(astBuilder, setterDecl).endsWith("m"));
    SLANG_CHECK(!getMangledName(astBuilder, defaultBorrow).endsWith("c"));
    SLANG_CHECK(getMangledName(astBuilder, directBorrow).endsWith("B"));
    SLANG_CHECK(getMangledName(astBuilder, directMutating).endsWith("B"));
    SLANG_CHECK(getMangledName(astBuilder, directRef).endsWith("B"));

    // `loadModuleFromSourceString` uses the session language option for in-memory source whose
    // path still ends in `.slang`. The `Language` session option selects the HLSL dialect, so the
    // first member function exercises its writable default and the second exercises trailing
    // `const`.
    slang::CompilerOptionEntry sourceLanguageOption = {};
    sourceLanguageOption.name = slang::CompilerOptionName::Language;
    sourceLanguageOption.value.kind = slang::CompilerOptionValueKind::Int;
    sourceLanguageOption.value.intValue0 = SLANG_SOURCE_LANGUAGE_HLSL;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &sourceLanguageOption;

    ComPtr<slang::ISession> hlslSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, hlslSession.writeRef())));

    ComPtr<slang::IBlob> hlslDiagnostics;
    ComPtr<slang::IModule> hlslModuleInterface(hlslSession->loadModuleFromSourceString(
        "checkedHlslEffectiveThisManglingPreservesLegacySuffixes",
        "checked-hlsl-effective-this-mangling.slang",
        "struct HlslReceiver\n"
        "{\n"
        "    int defaultWritable(int value) { return value; }\n"
        "    int trailingConst(int value) const { return value; }\n"
        "};\n",
        hlslDiagnostics.writeRef()));
    SLANG_CHECK_ABORT(hlslModuleInterface != nullptr);

    auto hlslModule = static_cast<Module*>(hlslModuleInterface.get());
    auto hlslReceiverDecl = findMemberDecl<StructDecl>(hlslModule->getModuleDecl(), "HlslReceiver");
    SLANG_CHECK_ABORT(hlslReceiverDecl != nullptr);

    auto defaultWritable = findMemberDecl<FuncDecl>(hlslReceiverDecl, "defaultWritable");
    auto trailingConst = findMemberDecl<FuncDecl>(hlslReceiverDecl, "trailingConst");
    SLANG_CHECK_ABORT(defaultWritable != nullptr);
    SLANG_CHECK_ABORT(trailingConst != nullptr);

    auto defaultWritableInfo = defaultWritable->findModifier<ThisParamInfoAttribute>();
    auto trailingConstInfo = trailingConst->findModifier<ThisParamInfoAttribute>();
    SLANG_CHECK_ABORT(defaultWritableInfo != nullptr);
    SLANG_CHECK_ABORT(trailingConstInfo != nullptr);
    SLANG_CHECK(defaultWritableInfo->info.mode == ParamPassingMode::BorrowInOut);
    SLANG_CHECK(trailingConstInfo->info.mode == ParamPassingMode::In);

    // The final `ii` encodes the ordinary `int` parameter and result. Any receiver-mode suffix
    // appended for either HLSL case would make these checks fail.
    auto hlslAstBuilder = hlslModule->getASTBuilder();
    SLANG_CHECK(getMangledName(hlslAstBuilder, defaultWritable).endsWith("ii"));
    SLANG_CHECK(getMangledName(hlslAstBuilder, trailingConst).endsWith("ii"));
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
    auto legacyName = getMangledName(astBuilder, methodDecl);
    auto synthesizedMode = astBuilder->create<SynthesizedParamPassingModeModifier>();
    addModifier(methodDecl, synthesizedMode);

    struct ModeCase
    {
        ParamPassingMode mode;
        char const* suffix;
    };
    ModeCase cases[] = {
        {ParamPassingMode::In, "ti_"},
        {ParamPassingMode::Out, "to_"},
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
        SLANG_CHECK(mangledName != legacyName);
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

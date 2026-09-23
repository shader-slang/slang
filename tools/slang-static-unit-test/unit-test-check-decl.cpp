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

// Moving mangling to the checked effective `this` parameter information must not change the
// established names. In particular, only explicitly `[mutating]` and `[__ref]` ordinary methods
// had mode suffixes; a borrowed method, a setter's writable default, and a direct function-type
// declaration did not.
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
        "    [mutating] [__ref] int overlappingModes(int value) { return value; }\n"
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
    auto overlappingModes = findMemberDecl<FuncDecl>(receiverDecl, "overlappingModes");
    auto staticNoDiffThis = findMemberDecl<FuncDecl>(receiverDecl, "staticNoDiffThis");
    auto propertyDecl = findMemberDecl<PropertyDecl>(receiverDecl, "item");
    auto defaultBorrow = findMemberDecl<FuncDecl>(nonCopyableReceiverDecl, "defaultBorrow");
    auto directBorrow = findMemberDecl<FuncDecl>(interfaceDecl, "directBorrow");
    auto directMutating = findMemberDecl<FuncDecl>(interfaceDecl, "directMutating");
    auto directRef = findMemberDecl<FuncDecl>(interfaceDecl, "directRef");
    SLANG_CHECK_ABORT(ordinaryBorrow != nullptr);
    SLANG_CHECK_ABORT(ordinaryMutating != nullptr);
    SLANG_CHECK_ABORT(ordinaryRef != nullptr);
    SLANG_CHECK_ABORT(overlappingModes != nullptr);
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
    SLANG_CHECK(getMangledName(astBuilder, overlappingModes).endsWith("mr"));
    SLANG_CHECK(getMangledName(astBuilder, staticNoDiffThis).endsWith("n"));
    SLANG_CHECK(!getMangledName(astBuilder, setterDecl).endsWith("m"));
    SLANG_CHECK(!getMangledName(astBuilder, defaultBorrow).endsWith("c"));
    SLANG_CHECK(getMangledName(astBuilder, directBorrow).endsWith("B"));
    SLANG_CHECK(getMangledName(astBuilder, directMutating).endsWith("B"));
    SLANG_CHECK(getMangledName(astBuilder, directRef).endsWith("B"));
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

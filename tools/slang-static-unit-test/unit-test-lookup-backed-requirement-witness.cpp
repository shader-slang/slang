// unit-test-lookup-backed-requirement-witness.cpp
//
// Tests requirement lookup through the checked AST witness shape from issue #12751.

#include "slang/slang-ast-builder.h"
#include "slang/slang-module.h"
#include "slang/slang-syntax.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

/// Find the first direct member of `containerDecl` of type `T` whose name matches.
template<typename T>
T* findMemberDecl(ContainerDecl* containerDecl, const char* name = nullptr)
{
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto typedMember = as<T>(member);
        if (!typedMember)
            continue;
        if (!name)
            return typedMember;
        if (typedMember->getName() && typedMember->getName()->text == name)
            return typedMember;
    }
    return nullptr;
}

/// Find the direct constraint whose subtype names `associatedTypeDecl`.
GenericTypeConstraintDecl* findAssociatedTypeConstraint(
    InterfaceDecl* interfaceDecl,
    AssocTypeDecl* associatedTypeDecl)
{
    for (auto constraint : interfaceDecl->getDirectMemberDeclsOfType<GenericTypeConstraintDecl>())
    {
        auto subType = as<DeclRefType>(constraint->sub.type);
        if (subType && subType->getDeclRef().getDecl() == associatedTypeDecl)
            return constraint;
    }
    return nullptr;
}

} // namespace

SLANG_UNIT_TEST(lookupBackedRequirementWitness)
{
    StaticUnitTestEnv env(unitTestContext);
    ASTBuilder* astBuilder = env.getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    String diagnostics;
    Module* module = env.checkModuleFromSource(
        "lookupBackedRequirementWitness",
        R"(
            interface IPrimitive
            {
                associatedtype Attributes;
            }

            interface ICustomPrimitive : IPrimitive {}

            interface IContext
            {
                associatedtype Primitive : IPrimitive;
            }

            struct Attributes {}

            struct Primitive<T> : ICustomPrimitive
            {
                typealias Attributes = T;
            }

            struct Context : IContext
            {
                typealias Primitive = ::Primitive<::Attributes>;
            }
        )",
        &diagnostics);
    if (!module && diagnostics.getLength())
        getTestReporter()->message(TestMessageType::Info, diagnostics.getBuffer());
    SLANG_CHECK_ABORT(module != nullptr);

    ModuleDecl* moduleDecl = module->getModuleDecl();
    auto primitiveInterface = findMemberDecl<InterfaceDecl>(moduleDecl, "IPrimitive");
    auto customPrimitiveInterface = findMemberDecl<InterfaceDecl>(moduleDecl, "ICustomPrimitive");
    auto contextInterface = findMemberDecl<InterfaceDecl>(moduleDecl, "IContext");
    auto attributesDecl = findMemberDecl<StructDecl>(moduleDecl, "Attributes");
    auto contextDecl = findMemberDecl<StructDecl>(moduleDecl, "Context");
    SLANG_CHECK_ABORT(primitiveInterface != nullptr);
    SLANG_CHECK_ABORT(customPrimitiveInterface != nullptr);
    SLANG_CHECK_ABORT(contextInterface != nullptr);
    SLANG_CHECK_ABORT(attributesDecl != nullptr);
    SLANG_CHECK_ABORT(contextDecl != nullptr);

    auto attributesRequirement = findMemberDecl<AssocTypeDecl>(primitiveInterface, "Attributes");
    auto primitiveRequirement = findMemberDecl<AssocTypeDecl>(contextInterface, "Primitive");
    SLANG_CHECK_ABORT(primitiveRequirement != nullptr);
    auto primitiveConstraint = findAssociatedTypeConstraint(contextInterface, primitiveRequirement);
    auto customPrimitiveInheritance = findMemberDecl<InheritanceDecl>(customPrimitiveInterface);
    auto contextInheritance = findMemberDecl<InheritanceDecl>(contextDecl);
    SLANG_CHECK_ABORT(attributesRequirement != nullptr);
    SLANG_CHECK_ABORT(primitiveConstraint != nullptr);
    SLANG_CHECK_ABORT(customPrimitiveInheritance != nullptr);
    SLANG_CHECK_ABORT(contextInheritance != nullptr);

    // Consider `Context` above. Its `IContext` table stores both the selected `Primitive` type and
    // the proof that this selected type implements `IPrimitive`. Select that proof by its exact
    // constraint key; witness-table entries are keyed data, so their order and representation are
    // not a valid way to identify one.
    WitnessTable* contextWitnessTable = contextInheritance->witnessTable;
    SLANG_CHECK_ABORT(contextWitnessTable != nullptr);
    RequirementWitness rawPrimitiveRequirement;
    SLANG_CHECK_ABORT(contextWitnessTable->getRequirementDictionary().tryGetValue(
        primitiveConstraint,
        rawPrimitiveRequirement));
    SLANG_CHECK_ABORT(rawPrimitiveRequirement.getFlavor() == RequirementWitness::Flavor::val);
    auto primitiveWitness = as<DeclaredSubtypeWitness>(rawPrimitiveRequirement.getVal());
    SLANG_CHECK_ABORT(primitiveWitness != nullptr);
    auto primitiveLookup = as<LookupDeclRef>(primitiveWitness->getDeclRef().declRefBase);
    SLANG_CHECK_ABORT(primitiveLookup != nullptr);
    SLANG_CHECK_ABORT(primitiveLookup->getDecl() == customPrimitiveInheritance);

    auto parentPrimitiveWitness = as<DeclaredSubtypeWitness>(primitiveLookup->getWitness());
    SLANG_CHECK_ABORT(parentPrimitiveWitness != nullptr);
    SLANG_CHECK_ABORT(
        as<LookupDeclRef>(parentPrimitiveWitness->getDeclRef().declRefBase) == nullptr);
    SLANG_CHECK_ABORT(parentPrimitiveWitness->getDeclRef().as<InheritanceDecl>());

    // The lookup declaration records the exact parent witness and requirement key used to reach
    // the nested conformance table. Querying that pair proves the shared direct-inheritance path
    // still supplies the intermediate witness table needed by the recursive lookup.
    auto intermediateWitness = tryLookUpRequirementWitness(
        astBuilder,
        primitiveLookup->getWitness(),
        primitiveLookup->getDecl());
    SLANG_CHECK_ABORT(intermediateWitness.getFlavor() == RequirementWitness::Flavor::witnessTable);
    auto intermediateTable = intermediateWitness.getWitnessTable();
    SLANG_CHECK(intermediateTable->witnessedType->equals(primitiveWitness->getSub()));
    SLANG_CHECK(intermediateTable->baseType->equals(primitiveWitness->getSup()));

    auto attributesRequirementWitness =
        tryLookUpRequirementWitness(astBuilder, primitiveWitness, attributesRequirement);
    SLANG_CHECK_ABORT(attributesRequirementWitness.getFlavor() == RequirementWitness::Flavor::val);
    auto attributesType = as<DeclRefType>(attributesRequirementWitness.getVal());
    SLANG_CHECK_ABORT(attributesType != nullptr);
    SLANG_CHECK(attributesType->getDeclRef().getDecl() == attributesDecl);

    // `ThisType` and `ThisTypeConstraint` are synthesized requirements rather than table entries.
    // A failed ordinary lookup must reach these shared fallbacks instead of returning `none` from
    // the lookup-backed path.
    auto thisTypeRequirement = primitiveInterface->getThisTypeDecl();
    SLANG_CHECK_ABORT(thisTypeRequirement != nullptr);
    auto thisTypeConstraintRequirement =
        findMemberDecl<ThisTypeConstraintDecl>(thisTypeRequirement);
    SLANG_CHECK_ABORT(thisTypeConstraintRequirement != nullptr);

    auto thisTypeWitness =
        tryLookUpRequirementWitness(astBuilder, primitiveWitness, thisTypeRequirement);
    SLANG_CHECK_ABORT(thisTypeWitness.getFlavor() == RequirementWitness::Flavor::val);
    SLANG_CHECK(thisTypeWitness.getVal() == primitiveWitness->getSub());

    auto thisTypeConstraintWitness =
        tryLookUpRequirementWitness(astBuilder, primitiveWitness, thisTypeConstraintRequirement);
    SLANG_CHECK_ABORT(thisTypeConstraintWitness.getFlavor() == RequirementWitness::Flavor::val);
    SLANG_CHECK(thisTypeConstraintWitness.getVal() == primitiveWitness);
}

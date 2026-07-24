// slang-check-shader.cpp
#include "slang-check-impl.h"

// This file encapsulates semantic checking logic primarily
// related to shaders, including validating entry points,
// enumerating specialization parameters, and validating
// attempts to specialize shader code.

#include "../core/slang-char-util.h"
#include "../core/slang-type-text-util.h"
#include "slang-lookup.h"
#include "slang-parameter-binding.h"
#include "slang-profile.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

namespace Slang
{

static constexpr char const* kNodeLaunchModeBroadcasting = "broadcasting";
static constexpr char const* kNodeLaunchModeThread = "thread";

// Direction of a semantic value (input from previous stage, or output to next stage)
enum class SemanticDirection
{
    Input,
    Output,
};

// Maximum nesting depth when recursively walking a declaration's type for system-value
// semantics. Shared by validateSystemValueSemantic and collectDepthOutputSemantics so the two
// walks provably use the same bound: collectDepthOutputSemantics can return silently at the
// limit precisely because validateSystemValueSemantic runs first with this same bound and has
// already reported MaximumTypeNestingLevelExceeded for anything deeper.
static constexpr UInt kMaxSystemValueSemanticRecursionDepth = 128;

static bool isValidThreadDispatchIDType(Type* type)
{
    // Can accept a single int/unit
    {
        auto basicType = as<BasicExpressionType>(type);
        if (basicType)
        {
            return (
                basicType->getBaseType() == BaseType::Int ||
                basicType->getBaseType() == BaseType::UInt);
        }
    }
    // Can be an int/uint vector from size 1 to 3
    {
        auto vectorType = as<VectorExpressionType>(type);
        if (!vectorType)
        {
            return false;
        }
        auto elemCount = as<ConstantIntVal>(vectorType->getElementCount());
        if (elemCount->getValue() < 1 || elemCount->getValue() > 3)
        {
            return false;
        }
        // Must be a basic type
        auto basicType = as<BasicExpressionType>(vectorType->getElementType());
        if (!basicType)
        {
            return false;
        }

        // Must be integral
        auto baseType = basicType->getBaseType();
        return (baseType == BaseType::Int || baseType == BaseType::UInt);
    }
}

// Unwrap Conditional<T, hasValue> types to T.
// If the type is Conditional<T, hasValue> (where hasValue can be true or false),
// returns the inner type T. Otherwise, returns the original type unchanged.
static Type* unwrapConditionalType(Type* type)
{
    if (auto conditionalType = as<ConditionalType>(type))
        return conditionalType->getValueType();
    return type;
}

// Extract the scalar element type and element count from a type.
// For a scalar BasicExpressionType, returns the type itself with count 1.
// For a VectorExpressionType, returns the element type and count.
// Returns nullptr if the type is neither scalar nor vector.
static BasicExpressionType* getScalarElementType(Type* type, IntegerLiteralValue& outCount)
{
    if (auto basicType = as<BasicExpressionType>(type))
    {
        outCount = 1;
        return basicType;
    }
    if (auto vecType = as<VectorExpressionType>(type))
    {
        if (auto countVal = as<ConstantIntVal>(vecType->getElementCount()))
        {
            outCount = countVal->getValue();
            return as<BasicExpressionType>(vecType->getElementType());
        }
    }
    return nullptr;
}

// Check if two types are compatible for system value semantics.
// Two types are compatible when they have the same shape (both scalar, or both
// vectors of the same element count) and their scalar element types belong to
// the same type category (integer, floating-point, or bool). This allows sign
// coercions like int3 for a uint3 semantic while rejecting cross-category
// coercions like float for a uint semantic.
static bool isSemanticTypeCompatible(Type* expectedType, Type* type)
{
    // Unwrap Conditional<T, hasValue> to T
    type = unwrapConditionalType(type);

    IntegerLiteralValue expectedCount = 0, typeCount = 0;
    auto expectedElem = getScalarElementType(expectedType, expectedCount);
    auto typeElem = getScalarElementType(type, typeCount);

    // Both types must be scalar or vector (no matrices, arrays, structs, etc.)
    if (!expectedElem || !typeElem)
        return false;

    // Shapes must match: same element count (1 for scalar, N for vectorN)
    if (expectedCount != typeCount)
        return false;

    // Scalar element types must be in the same category.
    // BaseTypeInfo tracks FloatingPoint and Integer flags; bool has neither.
    // Comparing the masked flags ensures int/uint match each other, float/half/double
    // match each other, and bool only matches bool.
    using Flag = BaseTypeInfo::Flag;
    constexpr BaseTypeInfo::Flags categoryMask = Flag::FloatingPoint | Flag::Integer;
    const auto& expectedInfo = BaseTypeInfo::getInfo(expectedElem->getBaseType());
    const auto& typeInfo = BaseTypeInfo::getInfo(typeElem->getBaseType());
    return (expectedInfo.flags & categoryMask) == (typeInfo.flags & categoryMask);
}

// Look up a SemanticDecl by name in the given scope.
// Semantic names in core.meta.slang are stored lowercase for case-insensitive matching.
static SemanticDecl* lookUpSemanticDecl(
    ASTBuilder* astBuilder,
    SemanticsVisitor* visitor,
    const String& semanticName,
    Scope* scope)
{
    auto namePool = astBuilder->getGlobalSession()->getNamePool();

    // Lowercase the name for lookup (semantics in core.meta.slang are lowercase)
    String lowerName = semanticName.toLower();
    auto name = namePool->getName(lowerName);
    auto lookupResult = lookUp(astBuilder, visitor, name, scope, LookupMask::Semantic);

    if (!lookupResult.isValid())
        return nullptr;

    return as<SemanticDecl>(lookupResult.item.declRef.getDecl());
}

// A requirement that adds nothing beyond `stage` (e.g. a bare `[require(fragment)]`) is already
// implied by the profile; propagating it would only make the "profile implicitly upgraded"
// diagnostic list a redundant stage atom.
static bool isStageOnlyRequirement(const CapabilitySetVal* capSet, Stage stage)
{
    if (!capSet)
        return true;
    CapabilityAtom stageAtom = getAtomFromStage(stage);
    if (stageAtom == CapabilityAtom::Invalid)
        return false;
    return CapabilitySet((CapabilityName)stageAtom).implies(CapabilitySet{capSet});
}

// General capability inference does not traverse from a semantic to the accessor it resolves to,
// so a requirement like `fragmentshaderbarycentric` on the `SV_Barycentrics` getter would be lost.
// Join the matched accessor's whole `[require]` set into `*outCaps`, but only when it adds
// something beyond the entry point's stage (see `isStageOnlyRequirement`).
static void collectSemanticAccessorRequirement(Decl* member, Stage stage, CapabilitySet* outCaps)
{
    if (!outCaps)
        return;
    auto requireAttr = member->findModifier<RequireCapabilityAttribute>();
    if (!requireAttr || !requireAttr->capabilitySet)
        return;
    if (isStageOnlyRequirement(requireAttr->capabilitySet, stage))
        return;
    outCaps->nonDestructiveJoin(requireAttr->capabilitySet);
}

// Validate that type being used for a system value semantic is compatible with the semantic.
static void validateSystemValueSemanticForType(
    SemanticsVisitor* visitor,
    DiagnosticSink* sink,
    SourceLoc loc,
    Type* type,
    HLSLSimpleSemantic* semantic,
    Stage stage,
    SemanticDirection direction,
    Scope* scope,
    CapabilitySet* outInferredCaps = nullptr)
{
    if (!semantic || !type)
        return;

    auto semanticNameSlice = semantic->name.getContent();

    // Only validate SV_ semantics
    if (!semanticNameSlice.startsWithCaseInsensitive(toSlice("sv_")))
        return;

    auto astBuilder = visitor->getASTBuilder();

    // Split name and index (e.g., "SV_Target0" -> "SV_Target" + "0")
    UnownedStringSlice baseNameSlice;
    UnownedStringSlice indexSlice;
    splitNameAndIndex(semanticNameSlice, baseNameSlice, indexSlice);
    String baseName = String(baseNameSlice);

    // Look up the SemanticDecl
    auto semanticDecl = lookUpSemanticDecl(astBuilder, visitor, baseName, scope);

    // If no SemanticDecl found, the semantic is not defined in core.meta.slang
    if (!semanticDecl)
    {
        diagnoseCapabilityErrors(
            sink,
            visitor->getOptionSet(),
            Diagnostics::UnknownSystemValueSemantic{.semanticName = baseName, .location = loc});
        return;
    }

    // If the semantic has no accessors defined, it accepts any type (e.g., ray tracing payloads)
    bool hasAnyAccessors = false;
    for (auto member : semanticDecl->getMembers())
    {
        if (as<SemanticGetterDecl>(member) || as<SemanticSetterDecl>(member))
        {
            hasAnyAccessors = true;
            break;
        }
    }

    if (!hasAnyAccessors)
        return;

    bool isOutput = (direction == SemanticDirection::Output);
    const char* directionStr = isOutput ? "output" : "input";
    const char* stageStr = getStageName(stage);

    // Look for matching accessor (getter for input, setter for output)
    bool foundMatchingAccessor = false;
    bool foundAccessorForDirection = false;
    List<Type*> validTypes;

    for (auto member : semanticDecl->getMembers())
    {
        // Check for getter (input) or setter (output)
        bool isGetter = as<SemanticGetterDecl>(member) != nullptr;
        bool isSetter = as<SemanticSetterDecl>(member) != nullptr;

        if (!isGetter && !isSetter)
            continue;

        // Direction check: getter = input, setter = output
        bool accessorIsOutput = isSetter;
        if (accessorIsOutput != isOutput)
            continue;

        // Check if the accessor's stage requirement matches the current stage
        // Multiple [require(stage)] attributes are merged into a single capabilitySet
        // using union (OR), so we check if the current stage is compatible
        if (auto requireAttr = member->findModifier<RequireCapabilityAttribute>())
        {
            if (requireAttr->capabilitySet)
            {
                CapabilityAtom currentStage = getAtomFromStage(stage);
                // Use !isIncompatibleWith because the capabilitySet is a union of stages
                // (e.g., compute | mesh | amplification), and we want to check if the
                // current stage is ANY of the allowed stages
                if (requireAttr->capabilitySet->isIncompatibleWith(currentStage))
                    continue;
            }
        }

        foundAccessorForDirection = true;

        // Get the accessor's type
        Type* accessorType = nullptr;
        if (auto getter = as<SemanticGetterDecl>(member))
            accessorType = getter->type.type;
        else if (auto setter = as<SemanticSetterDecl>(member))
            accessorType = setter->type.type;

        if (!accessorType)
        {
            // Type not resolved - this shouldn't happen after semantic checking
            continue;
        }

        if (isSemanticTypeCompatible(accessorType, type))
        {
            foundMatchingAccessor = true;
            collectSemanticAccessorRequirement(member, stage, outInferredCaps);
            break;
        }

        // Special case: if accessor is unsized array and type is sized array with same element type
        if (auto accessorArrayType = as<ArrayExpressionType>(accessorType))
        {
            if (auto typeArrayType = as<ArrayExpressionType>(type))
            {
                // Accessor has unsized array and type has any array - check element types
                if (accessorArrayType->isUnsized())
                {
                    if (isSemanticTypeCompatible(
                            accessorArrayType->getElementType(),
                            typeArrayType->getElementType()))
                    {
                        foundMatchingAccessor = true;
                        collectSemanticAccessorRequirement(member, stage, outInferredCaps);
                        break;
                    }
                }
            }
        }

        // Collect valid types for error message
        validTypes.add(accessorType);
    }

    if (!foundAccessorForDirection)
    {
        // No accessor defined for this stage+direction combination
        diagnoseCapabilityErrors(
            sink,
            visitor->getOptionSet(),
            Diagnostics::SystemValueSemanticInvalidDirection{
                .semantic = baseName,
                .direction = directionStr,
                .stage = stageStr,
                .location = loc});
    }
    else if (!foundMatchingAccessor)
    {
        // Type mismatch - build string of valid types
        StringBuilder validTypesStr;
        for (Index validTypeIndex = 0; validTypeIndex < validTypes.getCount(); validTypeIndex++)
        {
            if (validTypeIndex > 0)
                validTypesStr << "' or '";
            validTypesStr << validTypes[validTypeIndex];
        }

        diagnoseCapabilityErrors(
            sink,
            visitor->getOptionSet(),
            Diagnostics::SystemValueSemanticInvalidType{
                .type = unwrapConditionalType(type),
                .semantic = baseName,
                .expectedTypes = validTypesStr,
                .location = loc});
    }
}

// Check if a system value semantic name is per-primitive in mesh shaders.
// These semantics must only appear in OutputPrimitives (or 'out primitives') parameters.
static bool isPerPrimitiveMeshSemantic(UnownedStringSlice semanticName)
{
    return semanticName.caseInsensitiveEquals(toSlice("sv_cullprimitive")) ||
           semanticName.caseInsensitiveEquals(toSlice("sv_primitiveid")) ||
           semanticName.caseInsensitiveEquals(toSlice("sv_rendertargetarrayindex")) ||
           semanticName.caseInsensitiveEquals(toSlice("sv_viewportarrayindex")) ||
           semanticName.caseInsensitiveEquals(toSlice("sv_shadingrate"));
}

// Recursively check struct fields for per-primitive mesh shader semantics.
// Emits a diagnostic if any per-primitive semantic is found in a vertex/index output.
static void validateNoPerPrimitiveSemanticsInType(
    DiagnosticSink* sink,
    Type* type,
    ASTBuilder* astBuilder,
    HashSet<Type*>& seenTypes)
{
    if (!type)
        return;
    if (seenTypes.contains(type))
        return;
    seenTypes.add(type);

    auto declRefType = as<DeclRefType>(type);
    if (!declRefType)
        return;

    auto structDeclRef = declRefType->getDeclRef().as<StructDecl>();
    if (!structDeclRef)
        return;

    for (auto fieldDeclRef : getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance))
    {
        auto fieldDecl = fieldDeclRef.getDecl();

        // Recurse into nested struct types, unwrapping conditional and array wrappers first
        if (auto fieldVarDecl = as<VarDeclBase>(fieldDecl))
        {
            Type* fieldType = unwrapConditionalType(fieldVarDecl->getType());
            while (auto arrayType = as<ArrayExpressionType>(fieldType))
                fieldType = unwrapConditionalType(arrayType->getElementType());
            validateNoPerPrimitiveSemanticsInType(sink, fieldType, astBuilder, seenTypes);
        }

        // Check if this field has a per-primitive system value semantic
        auto semantic = fieldDecl->findModifier<HLSLSimpleSemantic>();
        if (!semantic)
            continue;

        auto name = semantic->name.getContent();
        if (isPerPrimitiveMeshSemantic(name))
        {
            sink->diagnose(Diagnostics::PerPrimitiveSemanticInVertexOutput{
                .semantic = String(name),
                .location = fieldDecl->loc});
        }
    }
}


// Validate `decl`'s SV_ semantics against the SemanticDecl definitions in the core module,
// recursing through struct fields. When `outInferredCaps` is non-null, also fold each matched
// accessor's capability requirement into it (non-destructive join), so a caller can enforce a
// requirement the semantic carries but the general capability inference misses — e.g.
// `fragmentshaderbarycentric` for `SV_Barycentrics`.
static void validateSystemValueSemantic(
    SemanticsVisitor* visitor,
    DiagnosticSink* sink,
    Decl* decl,
    Stage stage,
    SemanticDirection direction,
    Scope* scope,
    CapabilitySet* outInferredCaps = nullptr,
    UInt recursionDepth = 0)
{
    if (!decl)
        return;

    if (recursionDepth >= kMaxSystemValueSemanticRecursionDepth)
    {
        if (sink)
        {
            Diagnostics::MaximumTypeNestingLevelExceeded diag = {};
            diag.location = _getTypeNestingDiagnosticPosForDecl(decl);
            sink->diagnose(diag);
        }
        return;
    }

    // Get the type from the declaration
    Type* type = nullptr;
    if (auto varDecl = as<VarDeclBase>(decl))
        type = varDecl->getType();
    else if (auto funcDecl = as<FuncDecl>(decl))
        type = funcDecl->returnType.type;
    else
        return;

    if (!type)
        return;

    // Unwrap Conditional<T, hasValue> to T before checking for wrapper types
    type = unwrapConditionalType(type);

    // Mesh shader output types (OutputVertices, OutputPrimitives, OutputIndices) and
    // geometry shader stream types (PointStream, LineStream, TriangleStream) are
    // implicitly outputs - they don't require the 'out' keyword.
    // They need to be unwrapped to the element type before validating semantics,
    // and they could contain Conditional<T, hasValue> types, so we need to unwrap them again.
    if (auto meshOutputType = as<MeshOutputType>(type))
    {
        auto elementType = meshOutputType->getElementType();
        // If this is a vertex or index output, validate that no per-primitive semantics are used.
        // Per-primitive semantics must only appear in OutputPrimitives / 'out primitives'.
        if (stage == Stage::Mesh && !as<PrimitivesType>(meshOutputType))
        {
            if (auto semantic = decl->findModifier<HLSLSimpleSemantic>())
            {
                if (isPerPrimitiveMeshSemantic(semantic->name.getContent()))
                {
                    sink->diagnose(Diagnostics::PerPrimitiveSemanticInVertexOutput{
                        .semantic = String(semantic->name.getContent()),
                        .location = decl->loc});
                }
            }

            HashSet<Type*> seenTypes;
            validateNoPerPrimitiveSemanticsInType(
                sink,
                unwrapConditionalType(elementType),
                visitor->getASTBuilder(),
                seenTypes);
        }
        type = unwrapConditionalType(elementType);
        direction = SemanticDirection::Output;
    }
    else if (auto streamOutputType = as<HLSLStreamOutputType>(type))
    {
        auto elementType = streamOutputType->getElementType();
        type = unwrapConditionalType(elementType);
        direction = SemanticDirection::Output;
    }

    auto astBuilder = visitor->getASTBuilder();

    // If the type is a struct, recursively validate semantics on all fields
    if (auto declRefType = as<DeclRefType>(type))
    {
        if (auto structDeclRef = declRefType->getDeclRef().as<StructDecl>())
        {
            for (auto fieldDeclRef :
                 getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance))
            {
                auto fieldDecl = fieldDeclRef.getDecl();
                validateSystemValueSemantic(
                    visitor,
                    sink,
                    fieldDecl,
                    stage,
                    direction,
                    scope,
                    outInferredCaps,
                    recursionDepth + 1);
            }
        }
    }

    // Check if this decl has a system value semantic to validate
    auto semantic = decl->findModifier<HLSLSimpleSemantic>();
    if (!semantic)
        return;

    validateSystemValueSemanticForType(
        visitor,
        sink,
        decl->loc,
        type,
        semantic,
        stage,
        direction,
        scope,
        outInferredCaps);
}

// Return true if `semanticName` is one of the fragment depth-output system values
// (SV_Depth / SV_DepthGreaterEqual / SV_DepthLessEqual). A trailing numeric index is
// stripped first, matching validateSystemValueSemanticForType above, because Slang treats
// an indexed spelling like "SV_Depth0" as the same depth output (it lowers to gl_FragDepth /
// DepthReplacing just as "SV_Depth" does), so it must be classified identically here.
static bool isDepthOutputSemantic(UnownedStringSlice semanticName)
{
    UnownedStringSlice baseName;
    UnownedStringSlice indexSlice;
    splitNameAndIndex(semanticName, baseName, indexSlice);
    return baseName.caseInsensitiveEquals(toSlice("sv_depth")) ||
           baseName.caseInsensitiveEquals(toSlice("sv_depthgreaterequal")) ||
           baseName.caseInsensitiveEquals(toSlice("sv_depthlessequal"));
}

// Append to `ioDepthSemantics` every depth-output system-value semantic that `decl` contributes
// as a fragment output. `decl` is an `out`/`inout` parameter or the entry-point function itself
// (whose return type is examined). A fragment output can only be a scalar, a struct, or an array
// of those — never a mesh/stream output wrapper, which belong to non-fragment stages — so
// unwrapping Conditional and array wrappers and recursing into struct fields reaches every place
// a depth semantic can appear. Each depth semantic is recorded independently (a decl may
// contribute both its own semantic and those of its fields), so a collected count greater than
// one means the fragment entry point genuinely declares more than one depth output.
static void collectDepthOutputSemantics(
    ASTBuilder* astBuilder,
    Decl* decl,
    List<HLSLSimpleSemantic*>& ioDepthSemantics,
    UInt recursionDepth = 0)
{
    if (!decl || recursionDepth >= kMaxSystemValueSemanticRecursionDepth)
        return;

    // Get the type from the declaration (parameter type or function return type).
    Type* type = nullptr;
    if (auto varDecl = as<VarDeclBase>(decl))
        type = varDecl->getType();
    else if (auto funcDecl = as<FuncDecl>(decl))
        type = funcDecl->returnType.type;

    if (type)
    {
        // Unwrap Conditional<T> and any array wrappers, matching the sibling aggregate walk
        // validateNoPerPrimitiveSemanticsInType, so a depth semantic on a field of an
        // array-of-struct output (e.g. `out DepthOut a[1]`) is still reached.
        type = unwrapConditionalType(type);
        while (auto arrayType = as<ArrayExpressionType>(type))
            type = unwrapConditionalType(arrayType->getElementType());
        if (auto declRefType = as<DeclRefType>(type))
        {
            if (auto structDeclRef = declRefType->getDeclRef().as<StructDecl>())
            {
                for (auto fieldDeclRef :
                     getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance))
                {
                    collectDepthOutputSemantics(
                        astBuilder,
                        fieldDeclRef.getDecl(),
                        ioDepthSemantics,
                        recursionDepth + 1);
                }
            }
        }
    }

    // Record a depth semantic declared directly on this decl.
    if (auto semantic = decl->findModifier<HLSLSimpleSemantic>())
    {
        if (isDepthOutputSemantic(semantic->name.getContent()))
            ioDepthSemantics.add(semantic);
    }
}

/// Recursively walk `paramDeclRef` and add any existential/interface specialization parameters to
/// `ioSpecializationParams`.
static bool _collectExistentialSpecializationParamsRec(
    ASTBuilder* astBuilder,
    SpecializationParams& ioSpecializationParams,
    DeclRef<VarDeclBase> paramDeclRef,
    DiagnosticSink* sink,
    UInt recursionDepth);

/// Recursively walk `type` and add any existential/interface specialization parameters to
/// `ioSpecializationParams`.
static bool _collectExistentialSpecializationParamsRec(
    ASTBuilder* astBuilder,
    SpecializationParams& ioSpecializationParams,
    Type* type,
    SourceLoc loc,
    DiagnosticSink* sink,
    UInt recursionDepth)
{
    if (recursionDepth >= kMaxTypeNestingDepth)
    {
        if (sink)
        {
            Diagnostics::MaximumTypeNestingLevelExceeded diag = {};
            diag.location = loc;
            sink->diagnose(diag);
        }
        return false;
    }

    // Whether or not something is an array does not affect
    // the number of existential slots it introduces.
    //
    while (auto arrayType = as<ArrayExpressionType>(type))
    {
        type = arrayType->getElementType();
    }

    if (auto parameterGroupType = as<ParameterGroupType>(type))
    {
        return _collectExistentialSpecializationParamsRec(
            astBuilder,
            ioSpecializationParams,
            parameterGroupType->getElementType(),
            loc,
            sink,
            recursionDepth + 1);
    }

    if (auto declRefType = as<DeclRefType>(type))
    {
        auto typeDeclRef = declRefType->getDeclRef();
        if (auto interfaceDeclRef = typeDeclRef.as<InterfaceDecl>())
        {
            // Each leaf parameter of interface type adds a specialization
            // parameter, which determines the concrete type(s) that may
            // be provided as arguments for that parameter.
            //
            SpecializationParam specializationParam;
            specializationParam.flavor = SpecializationParam::Flavor::ExistentialType;
            specializationParam.loc = loc;
            specializationParam.object = type;
            ioSpecializationParams.add(specializationParam);
        }
        else if (auto structDeclRef = typeDeclRef.as<StructDecl>())
        {
            // A structure type should recursively introduce
            // existential slots for its fields.
            //
            for (auto fieldDeclRef :
                 getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance))
            {
                if (!_collectExistentialSpecializationParamsRec(
                        astBuilder,
                        ioSpecializationParams,
                        fieldDeclRef,
                        sink,
                        recursionDepth + 1))
                {
                    return false;
                }
            }
        }
    }

    // TODO: We eventually need to handle cases like constant
    // buffers and parameter blocks that may have existential
    // element types.
    return true;
}

static bool _collectExistentialSpecializationParamsRec(
    ASTBuilder* astBuilder,
    SpecializationParams& ioSpecializationParams,
    DeclRef<VarDeclBase> paramDeclRef,
    DiagnosticSink* sink,
    UInt recursionDepth)
{
    return _collectExistentialSpecializationParamsRec(
        astBuilder,
        ioSpecializationParams,
        getType(astBuilder, paramDeclRef),
        paramDeclRef.getLoc(),
        sink,
        recursionDepth);
}


/// Collect any interface/existential specialization parameters for `paramDeclRef` into
/// `ioParamInfo` and `ioSpecializationParams`
static bool _collectExistentialSpecializationParamsForShaderParam(
    ASTBuilder* astBuilder,
    ShaderParamInfo& ioParamInfo,
    SpecializationParams& ioSpecializationParams,
    DeclRef<VarDeclBase> paramDeclRef,
    DiagnosticSink* sink)
{
    Index beginParamIndex = ioSpecializationParams.getCount();
    if (!_collectExistentialSpecializationParamsRec(
            astBuilder,
            ioSpecializationParams,
            paramDeclRef,
            sink,
            0))
    {
        return false;
    }
    Index endParamIndex = ioSpecializationParams.getCount();

    ioParamInfo.firstSpecializationParamIndex = beginParamIndex;
    ioParamInfo.specializationParamCount = endParamIndex - beginParamIndex;
    return true;
}

void EntryPoint::_collectGenericSpecializationParamsRec(Decl* decl)
{
    if (!decl)
        return;

    _collectGenericSpecializationParamsRec(decl->parentDecl);

    auto genericDecl = as<GenericDecl>(decl);
    if (!genericDecl)
        return;

    for (auto m : genericDecl->getDirectMemberDecls())
    {
        if (auto genericTypeParam = as<GenericTypeParamDecl>(m))
        {
            SpecializationParam param;
            param.flavor = SpecializationParam::Flavor::GenericType;
            param.loc = genericTypeParam->loc;
            param.object = genericTypeParam;
            m_genericSpecializationParams.add(param);
        }
        else if (auto genericValParam = as<GenericValueParamDecl>(m))
        {
            SpecializationParam param;
            param.flavor = SpecializationParam::Flavor::GenericValue;
            param.loc = genericValParam->loc;
            param.object = genericValParam;
            m_genericSpecializationParams.add(param);
        }
        else if (auto genericValPackParam = as<GenericValuePackParamDecl>(m))
        {
            SpecializationParam param;
            param.flavor = SpecializationParam::Flavor::GenericValue;
            param.loc = genericValPackParam->loc;
            param.object = genericValPackParam;
            m_genericSpecializationParams.add(param);
        }
    }
}

/// Enumerate the existential-type parameters of an `EntryPoint`.
///
/// Any parameters found will be added to the list of existential slots on `this`.
///
void EntryPoint::_collectShaderParams()
{
    // We don't currently treat an entry point as having any
    // *global* shader parameters.
    //
    // TODO: We could probably clean up the code a bit by treating
    // an entry point as introducing a global shader parameter
    // that is based on the implicit "parameters struct" type
    // of the entry point itself.

    // We collect the generic parameters of the entry point,
    // along with those of any outer generics first.
    //
    _collectGenericSpecializationParamsRec(getFuncDecl());

    // After geneic specialization parameters have been collected,
    // we look through the value parameters of the entry point
    // function and see if any of them introduce existential/interface
    // specialization parameters.
    //
    // Note: we defensively test whether there is a function decl-ref
    // because this routine gets called from the constructor, and
    // a "dummy" entry point will have a null pointer for the function.
    //
    if (auto funcDeclRef = getFuncDeclRef())
    {
        for (auto paramDeclRef : getParameters(getLinkage()->getASTBuilder(), funcDeclRef))
        {
            ShaderParamInfo shaderParamInfo;
            shaderParamInfo.paramDeclRef = paramDeclRef;

            if (!_collectExistentialSpecializationParamsForShaderParam(
                    getLinkage()->getASTBuilder(),
                    shaderParamInfo,
                    m_existentialSpecializationParams,
                    paramDeclRef,
                    nullptr))
            {
                return;
            }

            m_shaderParams.add(shaderParamInfo);
        }
    }
}

bool isPrimaryDecl(CallableDecl* decl)
{
    SLANG_ASSERT(decl);
    return (!decl->primaryDecl) || (decl == decl->primaryDecl);
}

DeclRef<FuncDecl> findFunctionDeclByName(Module* translationUnit, Name* name, DiagnosticSink* sink)
{
    DeclRef<FuncDecl> entryPointFuncDeclRef;

    auto expr = translationUnit->findDeclFromString(getText(name), sink);
    if (auto declRefExpr = as<DeclRefExpr>(expr))
    {
        entryPointFuncDeclRef = declRefExpr->declRef.as<FuncDecl>();

        if (!entryPointFuncDeclRef)
        {
            if (auto genDeclRef = as<GenericDecl>(declRefExpr->declRef))
            {
                SharedSemanticsContext context(
                    translationUnit->getLinkage(),
                    translationUnit,
                    sink);
                SemanticsVisitor visitor(&context);
                entryPointFuncDeclRef = createDefaultSubstitutionsIfNeeded(
                                            translationUnit->getASTBuilder(),
                                            &visitor,
                                            translationUnit->getASTBuilder()->getMemberDeclRef(
                                                genDeclRef,
                                                genDeclRef.getDecl()->inner))
                                            .as<FuncDecl>();
            }
        }

        if (entryPointFuncDeclRef && getModule(entryPointFuncDeclRef.getDecl()) != translationUnit)
            entryPointFuncDeclRef = DeclRef<FuncDecl>();
    }

    if (!entryPointFuncDeclRef)
    {
        auto translationUnitSyntax = translationUnit->getModuleDecl();
        sink->diagnose(Diagnostics::EntryPointFunctionNotFound{
            .name = name->text,
            .location = translationUnitSyntax->loc});
    }
    return entryPointFuncDeclRef;
}

// Is a entry pointer parmaeter of `type` always a uniform parameter?
bool isUniformParameterType(Type* type)
{
    if (as<ResourceType>(type))
        return true;
    if (as<SubpassInputType>(type))
        return true;
    if (as<HLSLStructuredBufferTypeBase>(type))
        return true;
    if (as<UntypedBufferResourceType>(type))
        return true;
    if (as<UniformParameterGroupType>(type))
        return true;
    if (as<GLSLShaderStorageBufferType>(type))
        return true;
    if (as<SamplerStateType>(type))
        return true;
    if (as<PtrType>(type))
        return true;
    if (auto arrayType = as<ArrayExpressionType>(type))
        return isUniformParameterType(arrayType->getElementType());
    if (auto modType = as<ModifiedType>(type))
        return isUniformParameterType(modType->getBase());
    return false;
}

// Return whether `type`, used as an entry-point parameter, can actually have its
// binding placed by a `[[vk::binding(...)]]` annotation, i.e. it consumes a
// descriptor-shaped resource. This gates the "attribute ignored" diagnostic: the
// warning is suppressed only for parameters we can honor, and still fires for
// parameter kinds (e.g. plain varying scalars) where the annotation has no effect.
// Arrays and modified types defer to their element/base type.
//
// A `struct` is compatible iff at least one of its fields — transitively, and
// including fields inherited from a base struct — is compatible. This mirrors the
// binder, which decomposes an aggregate through its computed type layout: a struct
// whose fields include a texture/sampler accumulates a `DescriptorTableSlot` and so
// the binder positions those fields. Consider `struct Resources { Texture2D tex;
// SamplerState samp; }` used as `[[vk::binding(2,1)]] uniform Resources r`: the
// binder places `r.tex` at (binding 2, set 1) and `r.samp` at (binding 3, set 1),
// so the annotation is honored and the E38010 "ignored" warning must not fire.
// Field types are substituted through the struct's `DeclRef` (so a generic field
// such as `T tex` with `T = Texture2D` is recognized), and a nested struct field is
// handled by the recursive call.
//
// The recursion needs no explicit cycle/depth guard: it only ever descends a finite,
// acyclic type structure. A by-value struct cycle (`struct S { S next; }`) has
// unbounded size and is rejected earlier by the front-end nesting limit
// (`E39997`, `kMaxTypeNestingDepth`), and a cyclic inheritance graph is rejected by
// `E39999` — both fire before `validateEntryPoint` runs this predicate. So every
// field/base reached here resolves to a finite, non-recursive type.
//
// This list must stay in sync with the binder's contract: an explicit
// `[[vk::binding(...)]]` can only position a parameter that consumes a
// `DescriptorTableSlot` or `SubElementRegisterSpace` (see
// `isVkBindingEntryPointParameterResourceKind` in slang-parameter-binding.cpp).
// Unlike the near-identical `isUniformParameterType` above, do NOT list `PtrType`
// here: a raw pointer is a buffer-device-address value in push-constant/uniform
// storage with no descriptor slot to position, so the binder never honors a
// binding on it. Listing it would silently suppress the E38010 diagnostic
// (regression #11857). The struct case relies on that same subset property: it
// returns `true` only when a genuine descriptor-consuming leaf is found, so a
// struct of only pointers or plain data still (correctly) warns.
static bool isVkBindingCompatibleEntryPointParameterType(ASTBuilder* astBuilder, Type* type)
{
    if (as<ResourceType>(type))
        return true;
    if (as<SubpassInputType>(type))
        return true;
    if (as<HLSLStructuredBufferTypeBase>(type))
        return true;
    if (as<UntypedBufferResourceType>(type))
        return true;
    if (as<UniformParameterGroupType>(type))
        return true;
    if (as<GLSLShaderStorageBufferType>(type))
        return true;
    if (as<SamplerStateType>(type))
        return true;
    if (as<DynamicResourceType>(type))
        return true;
    if (auto arrayType = as<ArrayExpressionType>(type))
        return isVkBindingCompatibleEntryPointParameterType(
            astBuilder,
            arrayType->getElementType());
    if (auto modType = as<ModifiedType>(type))
        return isVkBindingCompatibleEntryPointParameterType(astBuilder, modType->getBase());
    if (auto declRefType = as<DeclRefType>(type))
    {
        if (auto structDeclRef = declRefType->getDeclRef().as<StructDecl>())
        {
            // `MemberFilterStyle::Instance` selects instance (non-`static`) members, not
            // own-vs-inherited: `getFields` returns only fields declared directly in this
            // struct, which is why inheritance needs the separate `findBaseStructType`
            // branch below rather than being folded in here. `static` members are excluded
            // deliberately — a static resource is a global, not part of this parameter
            // value's descriptor layout, so it must not make the struct look bindable.
            for (auto fieldDeclRef :
                 getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance))
            {
                if (isVkBindingCompatibleEntryPointParameterType(
                        astBuilder,
                        getType(astBuilder, fieldDeclRef)))
                    return true;
            }
            // Inherited fields also participate in the layout, so a base struct that
            // consumes a descriptor makes the derived type compatible too.
            if (auto baseStructType = findBaseStructType(astBuilder, structDeclRef))
                return isVkBindingCompatibleEntryPointParameterType(astBuilder, baseStructType);
        }
    }
    return false;
}

bool isBuiltinParameterType(Type* type)
{
    if (!as<BuiltinType>(type))
        return false;
    if (as<BasicExpressionType>(type))
        return false;
    if (as<VectorExpressionType>(type))
        return false;
    if (as<MatrixExpressionType>(type))
        return false;
    if (auto arrayType = as<ArrayExpressionType>(type))
        return isBuiltinParameterType(arrayType->getElementType());
    return true;
}

// Returns true if `type` is declared with `__intrinsic_type(op)` for the given
// IR opcode. Used to detect intrinsic types such as `CoopMat` and `CoopVec`
// which have no dedicated AST type class but carry an `IntrinsicTypeModifier`.
static bool isIntrinsicTypeWithOp(Type* type, IROp op)
{
    SLANG_ASSERT(type);
    type = as<Type>(type->resolve());
    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();

    auto declRefType = as<DeclRefType>(type);
    if (!declRefType)
        return false;
    auto decl = declRefType->getDeclRef().getDecl();
    if (!decl)
        return false;

    auto modifier = decl->findModifier<IntrinsicTypeModifier>();
    if (!modifier)
        return false;
    return IROp(modifier->irOp) == op;
}

// Describes a rule for types that are invalid as entry-point varying parameters/return types.
struct EntryPointVaryingTypeRule
{
    // Returns true if this type matches the rule (i.e., is invalid).
    bool (*matches)(Type* type);

    // Human-readable reason string for the diagnostic.
    const char* reason;

    // If non-null, this rule only applies when the target matches this predicate.
    // When null, the rule applies to all targets.
    bool (*targetPredicate)(CodeGenTarget target);

    // If non-null, this rule only applies when the stage matches this predicate.
    // When null, the rule applies to all stages for which the parameter/return
    // can be a varying (see `canHaveVaryingInput` in `validateEntryPoint`).
    bool (*stagePredicate)(Stage stage);
};

static bool _matchDifferentialPairType(Type* type)
{
    // Match both `DifferentialPair<T>` and `DifferentialPtrPair<T>` (the
    // backward-mode autodiff ptr-pair type). They are represented by
    // distinct AST classes but share the same varying-type restriction.
    return as<DifferentialPairType>(type) != nullptr ||
           as<DifferentialPtrPairType>(type) != nullptr;
}

static bool _matchAtomicType(Type* type)
{
    return as<AtomicType>(type) != nullptr;
}

static bool _matchCoopVectorType(Type* type)
{
    // `CoopVec` is declared in the core module as a struct carrying
    // `__intrinsic_type(kIROp_CoopVectorType)`. Detect it via that modifier
    // to stay consistent with how `_matchCoopMatrixType` handles `CoopMat`.
    return isIntrinsicTypeWithOp(type, kIROp_CoopVectorType);
}

static bool _matchCoopMatrixType(Type* type)
{
    return isIntrinsicTypeWithOp(type, kIROp_CoopMatrixType);
}

static bool _matchVectorBoolType(Type* type)
{
    auto vecType = as<VectorExpressionType>(type);
    if (!vecType)
        return false;
    auto elemType = as<BasicExpressionType>(vecType->getElementType());
    if (!elemType)
        return false;
    return elemType->getBaseType() == BaseType::Bool;
}

static bool _matchHLSLStreamOutputType(Type* type)
{
    return as<HLSLStreamOutputType>(type) != nullptr;
}

static bool _matchMeshOutputType(Type* type)
{
    return as<MeshOutputType>(type) != nullptr;
}

static bool _isNonGeometryStage(Stage stage)
{
    return stage != Stage::Geometry && stage != Stage::Unknown;
}

static bool _isNonMeshStage(Stage stage)
{
    return stage != Stage::Mesh && stage != Stage::Unknown;
}

// Returns true if `type` is `OutputIndices<T, N>` (`IndicesType`) whose
// element type `T` is anything other than the three valid mesh-output
// index shapes: `uint` for point indices, `uint2` for line indices, or
// `uint3` for triangle indices. Any other element type makes downstream
// codegen crash: non-integral scalars (e.g. `float`) cause a null
// `IRIntLit` dereference in GLSL legalization, while wrong-width vectors
// (e.g. `uint4`) and struct types hit `SLANG_UNREACHABLE` (issue #9435).
static bool _matchInvalidIndicesElementType(Type* type)
{
    auto indicesType = as<IndicesType>(type);
    if (!indicesType)
        return false;
    auto elementType = indicesType->getElementType();

    // If the element type is an error (unresolved name, failed generic, etc.)
    // a diagnostic has already been emitted — don't pile on a second one.
    if (!elementType || as<ErrorType>(elementType))
        return false;

    // Unwrap typedef / type-alias sugar so that e.g.
    // `typedef uint3 Triangle; OutputIndices<Triangle, N>` is accepted.
    elementType = elementType->getCanonicalType();

    if (auto basicType = as<BasicExpressionType>(elementType))
    {
        // Point indices: scalar `uint`.
        return basicType->getBaseType() != BaseType::UInt;
    }
    if (auto vectorType = as<VectorExpressionType>(elementType))
    {
        // Line/triangle indices: `uint2`/`uint3`.
        auto basicElem = as<BasicExpressionType>(vectorType->getElementType());
        if (!basicElem || basicElem->getBaseType() != BaseType::UInt)
            return true;
        auto count = as<ConstantIntVal>(vectorType->getElementCount());
        if (!count)
            return true;
        auto n = count->getValue();
        return n != 2 && n != 3;
    }
    return true;
}

static bool _matchMatrixWithNonFloatElementType(Type* type)
{
    // SPIR-V's `OpTypeMatrix` requires column vectors to have a floating-point
    // scalar component type (half/float/double). Matrices with integer or bool
    // element types are legalized to arrays in IR, but that legalization does
    // not produce valid SPIR-V when the matrix appears in an interface block
    // (entry-point varyings). Diagnose them up front (issue #9451).
    auto matType = as<MatrixExpressionType>(type);
    if (!matType)
        return false;
    auto elemType = as<BasicExpressionType>(matType->getElementType());
    if (!elemType)
        return false;
    switch (elemType->getBaseType())
    {
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
        return false; // valid floating-point element types
    default:
        return true; // integer, bool, etc. — not valid for SPIR-V matrices
    }
}

static bool _matchMatrixWithOutOfRangeDimensions(Type* type)
{
    // Row and column counts outside the 1..4 range break downstream codegen
    // (issue #9450). Only the entry-point varying site is restricted here;
    // the constructed type itself is not rejected, so targets that can
    // support larger matrices internally are unaffected.
    auto matType = as<MatrixExpressionType>(type);
    if (!matType)
        return false;
    auto isOutOfRange = [](IntVal* v)
    {
        if (auto cv = as<ConstantIntVal>(v))
        {
            auto n = cv->getValue();
            return n < 1 || n > 4;
        }
        return false;
    };
    return isOutOfRange(matType->getRowCount()) || isOutOfRange(matType->getColumnCount());
}

// True for stages that use interface-block-style varyings in SPIR-V/GLSL
// (i.e. the stages where the failure modes in issues #9446/#9448/#9449/#9452
// were reproduced). Ray-tracing payload/attribute stages and the compute
// stages do not use interface blocks and place fewer restrictions on the
// varying type, so we avoid diagnosing those stages.
//
// Note: `Amplification` (task) shaders output via `TaskPayloadWorkgroupEXT`
// rather than interface blocks, so they are not included here. `Mesh` shader
// varying outputs *do* lower to interface blocks in SPIR-V, so it is.
static bool _isInterfaceBlockVaryingStage(Stage stage)
{
    switch (stage)
    {
    case Stage::Vertex:
    case Stage::Fragment:
    case Stage::Geometry:
    case Stage::Hull:
    case Stage::Domain:
    case Stage::Mesh:
        return true;
    default:
        return false;
    }
}

static const EntryPointVaryingTypeRule kEntryPointVaryingTypeRules[] = {
    // `DifferentialPair`/`DifferentialPtrPair` are autodiff wrapper types with
    // no defined meaning at an inter-stage interface. Issue #9429 shows they
    // crash the backend on every stage where they participate as varyings.
    {_matchDifferentialPairType,
     "DifferentialPair/DifferentialPtrPair is not a valid varying type",
     nullptr,
     nullptr},

    // `Atomic<T>` is a storage-class wrapper, not a value type that can be
    // passed across a shader interface. Issue #9443.
    {_matchAtomicType, "Atomic is not a valid varying type", nullptr, nullptr},

    // `CoopVec` and `CoopMat` generate invalid SPIR-V when placed into an
    // interface block. Ray-tracing payload/attribute stages tolerate them,
    // so we restrict the rule to interface-block varying stages
    // (issues #9446, #9448, #9449).
    {_matchCoopVectorType,
     "CoopVec is not a valid varying type for this stage",
     nullptr,
     _isInterfaceBlockVaryingStage},
    {_matchCoopMatrixType,
     "CoopMat is not a valid varying type for this stage",
     nullptr,
     _isInterfaceBlockVaryingStage},

    // `vector<bool>` produces invalid SPIR-V on interface-block stages
    // (issue #9452). Compute and ray-tracing payload stages accept it in
    // practice; only diagnose where the failure actually occurs.
    {_matchVectorBoolType,
     "vector<bool> is not a valid SPIR-V varying type for this stage",
     isSPIRV,
     _isInterfaceBlockVaryingStage},

    // `matrix<T, R, C>` where T is not a floating-point type (half/float/double)
    // generates invalid SPIR-V because `OpTypeMatrix` requires floating-point
    // column vectors. Integer and bool matrices are legalized to arrays in IR
    // but that legalization does not cover interface-block varyings (issue #9451).
    {_matchMatrixWithNonFloatElementType,
     "matrix element type must be a floating-point type (half, float, or double) for "
     "SPIR-V entry-point varyings",
     isSPIRV,
     _isInterfaceBlockVaryingStage},

    // `matrix<T, R, C>` with row/column count outside 1..4 breaks downstream
    // codegen on the interface-block stages (issue #9450).
    {_matchMatrixWithOutOfRangeDimensions,
     "matrix row and column counts must be between 1 and 4 inclusive",
     nullptr,
     _isInterfaceBlockVaryingStage},

    // Geometry-shader stream output wrappers
    // (`PointStream`/`LineStream`/`TriangleStream<T>`) only make sense on
    // a `[shader("geometry")]` entry point. Using them on any other stage
    // segfaults during code generation (issue #9430).
    {_matchHLSLStreamOutputType,
     "stream output types are only valid on a geometry shader entry point",
     nullptr,
     _isNonGeometryStage},

    // Mesh-shader output wrappers
    // (`OutputVertices`/`OutputIndices`/`OutputPrimitives<T>`) only make
    // sense on a `[shader("mesh")]` entry point. The mesh-side counterpart
    // of #9430 — without this rule the SPIR-V generator produces invalid
    // or crashing output.
    {_matchMeshOutputType,
     "mesh output types are only valid on a mesh shader entry point",
     nullptr,
     _isNonMeshStage},

    // `OutputIndices<T, N>` requires `T` to be `uint`/`uint2`/`uint3`.
    // Other element types crash downstream codegen (issue #9435).
    {_matchInvalidIndicesElementType,
     "OutputIndices element type must be uint, uint2, or uint3",
     nullptr,
     _isInterfaceBlockVaryingStage},
};

struct VaryingTypeValidationContext
{
    ASTBuilder* astBuilder;
    DiagnosticSink* sink;
    Name* entryPointName;
    SourceLoc loc;
    const char* direction;
    const char* context;
    ArrayView<CodeGenTarget> targets;
    Stage stage;
    HashSet<Type*> seenTypes;
    UInt recursionDepth = 0;
    bool reportedNestingLimit = false;
};

// Recursively walks a type and checks it against the varying type rules.
// Returns true if any error was found.
static bool validateVaryingType(VaryingTypeValidationContext& ctx, Type* type)
{
    if (!type)
        return false;

    if (as<ErrorType>(type))
        return false;

    // Guard against deeply / infinitely nested generic struct types. The
    // `seenTypes` set catches self-referential types that reuse the same
    // `Type*` pointer, but generic instantiations such as
    // `struct LoopField<each T> { LoopField<T, int> next; }` produce a fresh
    // `Type*` at every level and would otherwise recurse unboundedly.
    if (ctx.recursionDepth >= kMaxTypeNestingDepth)
    {
        if (!ctx.reportedNestingLimit)
        {
            ctx.reportedNestingLimit = true;
            Diagnostics::MaximumTypeNestingLevelExceeded diag = {};
            diag.location = ctx.loc;
            ctx.sink->diagnose(diag);
        }
        return true;
    }
    struct DepthGuard
    {
        UInt& depth;
        DepthGuard(UInt& d)
            : depth(d)
        {
            ++depth;
        }
        ~DepthGuard() { --depth; }
    } depthGuard(ctx.recursionDepth);

    // Unwrap ModifiedType
    if (auto modType = as<ModifiedType>(type))
        return validateVaryingType(ctx, modType->getBase());

    for (const auto& rule : kEntryPointVaryingTypeRules)
    {
        if (!rule.matches(type))
            continue;

        // A rule may scope itself to a subset of stages (e.g. only
        // interface-block stages). If so, skip when the entry point's stage
        // is outside that set.
        if (rule.stagePredicate && !rule.stagePredicate(ctx.stage))
            continue;

        if (rule.targetPredicate)
        {
            bool anyTargetMatches = false;
            String matchedTargetName;
            for (auto target : ctx.targets)
            {
                if (rule.targetPredicate(target))
                {
                    anyTargetMatches = true;
                    matchedTargetName =
                        TypeTextUtil::getCompileTargetName(SlangCompileTarget(target));
                    break;
                }
            }
            if (!anyTargetMatches)
                continue;

            ctx.sink->diagnose(Diagnostics::InvalidEntryPointVaryingTypeForTarget{
                .type = type,
                .direction = ctx.direction,
                .context = ctx.context,
                .entryPoint = ctx.entryPointName,
                .target = matchedTargetName,
                .reason = rule.reason,
                .location = ctx.loc});
        }
        else
        {
            ctx.sink->diagnose(Diagnostics::InvalidEntryPointVaryingType{
                .type = type,
                .direction = ctx.direction,
                .context = ctx.context,
                .entryPoint = ctx.entryPointName,
                .reason = rule.reason,
                .location = ctx.loc});
        }
        return true;
    }

    // Recurse into array element type.
    // Note: `ArrayExpressionType` inherits from `DeclRefType` (its decl is the
    // builtin `Array` struct), so this check must come before the struct-field
    // recursion below to avoid iterating the internal fields of `Array`.
    if (auto arrayType = as<ArrayExpressionType>(type))
        return validateVaryingType(ctx, arrayType->getElementType());

    // Recurse through geometry-shader stream output wrappers
    // (`PointStream`/`LineStream`/`TriangleStream<T>`) and mesh-shader output
    // wrappers (`Vertices`/`Indices`/`Primitives<T>`). Like arrays, these are
    // `DeclRefType`s whose decl is an empty `struct`, so without an explicit
    // unwrap the struct-field recursion below would walk zero fields and miss
    // an invalid inner varying type such as `TriangleStream<BadStruct>`.
    if (auto streamType = as<HLSLStreamOutputType>(type))
        return validateVaryingType(ctx, streamType->getElementType());
    if (auto meshOutputType = as<MeshOutputType>(type))
        return validateVaryingType(ctx, meshOutputType->getElementType());

    // Recurse into struct fields
    if (auto declRefType = as<DeclRefType>(type))
    {
        auto structDeclRef = declRefType->getDeclRef().as<StructDecl>();
        if (structDeclRef)
        {
            if (ctx.seenTypes.contains(type))
                return false;
            ctx.seenTypes.add(type);

            bool foundError = false;
            // Iterate the struct's fields through the DeclRef so that generic
            // type parameter substitutions are applied to each field's type.
            // Without this, `struct Wrapper<T> { T x; }` used as
            // `Wrapper<DifferentialPair<float>>` would not be caught.
            for (auto fieldDeclRef :
                 getFields(ctx.astBuilder, structDeclRef, MemberFilterStyle::Instance))
            {
                auto fieldType = getType(ctx.astBuilder, fieldDeclRef);
                if (validateVaryingType(ctx, fieldType))
                    foundError = true;
            }
            return foundError;
        }
    }

    // We deliberately do not recurse into vector/matrix element types: the
    // type system requires those element types to be scalar, and any scalar
    // element restriction (e.g. `vector<bool>`, `matrix<uint64_t, ...>`) is
    // expressed as a whole-type rule in `kEntryPointVaryingTypeRules` above.

    return false;
}

bool doStructFieldsHaveSemanticImpl(Type* type, HashSet<Type*>& seenTypes)
{
    auto declRefType = as<DeclRefType>(type);
    if (!declRefType)
        return false;
    auto structDecl = as<StructDecl>(declRefType->getDeclRef().getDecl());
    if (!structDecl)
        return false;
    seenTypes.add(type);
    bool hasFields = false;
    for (auto field : structDecl->getFields())
    {
        hasFields = true;
        if (!field->findModifier<HLSLSemantic>())
        {
            if (!seenTypes.contains(field->getType()))
            {
                if (!doStructFieldsHaveSemanticImpl(field->getType(), seenTypes))
                    return false;
            }
        }
    }
    return hasFields;
}

bool doStructFieldsHaveSemantic(Type* type)
{
    HashSet<Type*> seenTypes;
    return doStructFieldsHaveSemanticImpl(type, seenTypes);
}

// Returns the base portion of a semantic name with any trailing decimal
// digits stripped, so that e.g. `SV_Position`, `SV_Position0` and
// `SV_Position1` all yield `SV_Position`. HLSL semantics are
// indexed by an optional integer suffix and the index isn't relevant
// for "is the semantic present" questions.
static UnownedStringSlice _semanticBaseName(UnownedStringSlice name)
{
    auto end = name.end();
    while (end != name.begin() && CharUtil::isDigit(end[-1]))
        --end;
    return UnownedStringSlice(name.begin(), end);
}

// Returns true if `decl` has a semantic whose base name (any trailing
// decimal index dropped) matches `baseName`, case-insensitively. `decl`
// may be null, in which case false is returned.
static bool _declHasSemantic(Decl* decl, UnownedStringSlice baseName)
{
    if (!decl)
        return false;
    if (auto semantic = decl->findModifier<HLSLSimpleSemantic>())
    {
        if (_semanticBaseName(semantic->name.getContent()).caseInsensitiveEquals(baseName))
            return true;
    }
    return false;
}

// Returns true if any declaration reachable from `type` (transitively
// through structs, arrays, conditional/wrapper types, modified types
// and stream/mesh output wrappers) carries a semantic whose base name
// matches `baseName`.
//
// A recursion depth bound is enforced alongside `seenTypes`: generic
// instantiations can produce a fresh `Type*` at each level and would
// otherwise recurse unboundedly.
static bool _typeHasSemanticImpl(
    ASTBuilder* astBuilder,
    Type* type,
    UnownedStringSlice baseName,
    HashSet<Type*>& seenTypes,
    UInt recursionDepth = 0)
{
    if (recursionDepth >= kMaxTypeNestingDepth)
        return false;
    if (!type)
        return false;
    type = unwrapConditionalType(type);
    if (!type)
        return false;
    if (seenTypes.contains(type))
        return false;
    seenTypes.add(type);

    const auto next = recursionDepth + 1;

    if (auto modType = as<ModifiedType>(type))
        return _typeHasSemanticImpl(astBuilder, modType->getBase(), baseName, seenTypes, next);

    if (auto arrayType = as<ArrayExpressionType>(type))
        return _typeHasSemanticImpl(
            astBuilder,
            arrayType->getElementType(),
            baseName,
            seenTypes,
            next);
    if (auto streamType = as<HLSLStreamOutputType>(type))
        return _typeHasSemanticImpl(
            astBuilder,
            streamType->getElementType(),
            baseName,
            seenTypes,
            next);
    if (auto meshOutputType = as<MeshOutputType>(type))
        return _typeHasSemanticImpl(
            astBuilder,
            meshOutputType->getElementType(),
            baseName,
            seenTypes,
            next);

    auto declRefType = as<DeclRefType>(type);
    if (!declRefType)
        return false;
    auto structDeclRef = declRefType->getDeclRef().as<StructDecl>();
    if (!structDeclRef)
        return false;

    for (auto fieldDeclRef : getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance))
    {
        auto fieldDecl = fieldDeclRef.getDecl();
        if (_declHasSemantic(fieldDecl, baseName))
            return true;
        auto fieldType = getType(astBuilder, fieldDeclRef);
        if (_typeHasSemanticImpl(astBuilder, fieldType, baseName, seenTypes, next))
            return true;
    }
    return false;
}

// Convenience wrapper: check whether `decl` (and the type it carries)
// transitively expose a semantic whose base name matches `baseName`.
static bool _outputDeclHasSemantic(
    ASTBuilder* astBuilder,
    Decl* decl,
    Type* type,
    UnownedStringSlice baseName)
{
    if (_declHasSemantic(decl, baseName))
        return true;
    HashSet<Type*> seenTypes;
    return _typeHasSemanticImpl(astBuilder, type, baseName, seenTypes);
}

static bool _allTargetsSupportVkBindingOnEntryPointParameters(Linkage* linkage)
{
    for (auto targetReq : linkage->targets)
    {
        if (!doesTargetSupportVkBindingOnEntryPointParameters(targetReq))
            return false;
    }
    return true;
}


// A user-defined generic struct found in an entry-point signature type, paired
// with the source location of its use.
struct GenericStructTypeUse
{
    StructDecl* structDecl;
    SourceLoc useLoc;
};

// Collect every user-defined generic struct (e.g. `Foo<int>`) reachable from an
// entry-point signature type `type`, recursing through wrapper/composite types so
// that `Foo<int>`, `Foo<int>[N]`, `Optional<Foo<int>>`, and
// `ConstantBuffer<Foo<int>>` are all found. Results are appended to `outUses` and
// `visited` guards against cycles in the `Val` graph.
//
// This is needed because the general capability-inference walk
// (`SemanticsDeclReferenceVisitor`) records a type's requirements only when its
// decl-ref is a `DirectDeclRef`; a generic specialization uses a
// `GenericAppDeclRef` and is skipped, so a `[require(...)]` on a generic struct
// used in an entry-point signature is otherwise dropped. The non-generic spelling
// `Foo` is already handled by that walk, so only the generic case is collected
// here (to avoid duplicate reporting).
//
// This deliberately lives in entry-point validation rather than in the general
// inference walk: inferring a generic struct type's requirements for *every*
// function that names such a type would require many core-module library
// functions (e.g. the cooperative vector/matrix/tensor `Load`/`Store` helpers,
// which take `CoopVec<T,N>` etc.) to redeclare those capabilities. Restricting
// the check to entry-point signatures matches the reported defect without
// changing library-function inference.
//
// Only the struct decl itself is filtered for `MagicTypeModifier`/
// `IntrinsicTypeModifier`: builtin generic types (e.g. `LineStream<T>`,
// `OutputPatch<T,N>`) already have dedicated, more specific entry-point
// diagnostics, so reporting a generic capability error for them would only
// duplicate those. Wrapper builtins are still recursed *through* so that a
// user-defined `Foo<int>` nested inside them is found.
static void collectGenericStructTypeUses(
    ASTBuilder* astBuilder,
    Val* type,
    SourceLoc useLoc,
    HashSet<Val*>& visited,
    List<GenericStructTypeUse>& outUses,
    UInt recursionDepth = 0)
{
    if (!type || !visited.add(type))
        return;

    // Bound the recursion to avoid overflowing the stack on a legitimately deep
    // acyclic chain (e.g. `Wrap<Wrap<...<Foo<int>>...>>`), where each level is a
    // distinct hash-consed `Val` that the visited set does not collapse. This
    // mirrors the `kMaxTypeNestingDepth` guard used by the other type walks in
    // this file; a type nested past that limit is already diagnosed with
    // "maximum type nesting level exceeded" by `validateVaryingType`, which runs
    // earlier in `validateEntryPoint`, so we simply stop descending here.
    if (recursionDepth >= kMaxTypeNestingDepth)
        return;

    if (auto declRefType = as<DeclRefType>(type))
    {
        auto structDeclRef = declRefType->getDeclRef().as<StructDecl>();
        if (structDeclRef && as<GenericAppDeclRef>(declRefType->getDeclRefBase()) &&
            !structDeclRef.getDecl()->findModifier<MagicTypeModifier>() &&
            !structDeclRef.getDecl()->findModifier<IntrinsicTypeModifier>())
        {
            // Only contribute structs that actually carry a requirement; this keeps
            // both the aggregation and the diagnostic loop free of null/empty sets.
            auto* caps = structDeclRef.getDecl()->inferredCapabilityRequirements;
            if (caps && !caps->isEmpty())
                outUses.add({structDeclRef.getDecl(), useLoc});
        }
        // Recurse through the struct's fields *with substitutions applied*, so a
        // wrapper like `struct Wrapper<T> { Foo<T> f; }` used as `Wrapper<int>`,
        // or a non-generic `struct Wrapper { Foo<int> f; }`, still reaches
        // `Foo<int>` (which the `Val`-operand walk below alone would miss, since
        // the field type is not an operand of the wrapper type).
        if (structDeclRef)
        {
            for (auto fieldDeclRef :
                 getFields(astBuilder, structDeclRef, MemberFilterStyle::Instance))
                collectGenericStructTypeUses(
                    astBuilder,
                    getType(astBuilder, fieldDeclRef),
                    useLoc,
                    visited,
                    outUses,
                    recursionDepth + 1);
        }
    }

    // Recurse into the type's `Val` operands (generic arguments, element types,
    // etc.) so nested user generic structs inside wrappers/arrays are found.
    for (Index i = 0; i < type->getOperandCount(); i++)
    {
        if (type->m_operands[i].kind == ValNodeOperandKind::ValNode)
            collectGenericStructTypeUses(
                astBuilder,
                type->getOperand(i),
                useLoc,
                visited,
                outUses,
                recursionDepth + 1);
    }
}

// Validate that an entry point function conforms to any additional
// constraints based on the stage (and profile?) it specifies.
void validateEntryPoint(EntryPoint* entryPoint, DiagnosticSink* sink)
{
    auto entryPointFuncDecl = entryPoint->getFuncDecl();
    auto stage = entryPoint->getStage();

    // TODO: We currently do minimal checking here, but this is the
    // right place to perform the following validation checks:
    //

    // * Are the function input/output parameters and result type
    //   all valid for the chosen stage? (e.g., there shouldn't be
    //   an `OutputStream<X>` type in a vertex shader signature)
    //
    // * For any varying input/output, are there semantics specified
    //   (Note: this potentially overlaps with layout logic...), and
    //   are the system-value semantics valid for the given stage?
    //
    //   There's actually a lot of detail to semantic checking, in
    //   that the AST-level code should probably be validating the
    //   use of system-value semantics by linking them to explicit
    //   declarations in the core module. We should also be
    //   using profile information on those declarations to infer
    //   appropriate profile restrictions on the entry point.
    //
    // * Is the entry point actually usable on the given stage/profile?
    //   E.g., if we have a vertex shader that (transitively) calls
    //   `Texture2D.Sample`, then that should produce an error because
    //   that function is specific to the fragment profile/stage.
    //

    auto entryPointName = entryPointFuncDecl->getName();

    auto module = getModule(entryPointFuncDecl);
    auto linkage = entryPoint->getLinkage();

    // Check if the return type is valid for a shader entry point
    auto returnType = entryPointFuncDecl->returnType.type;
    if (returnType)
    {
        // Use the existing getTypeTags functionality to check for resource types
        // Create a temporary SemanticsVisitor to access getTypeTags
        SharedSemanticsContext shared(linkage, module, sink);
        SemanticsVisitor visitor(&shared);

        auto typeTags = visitor.getTypeTags(returnType);
        bool hasResourceOrUnsizedTypes = (((int)typeTags & (int)TypeTag::Opaque) != 0) ||
                                         (((int)typeTags & (int)TypeTag::Unsized) != 0);

        if (hasResourceOrUnsizedTypes)
        {
            sink->diagnose(Diagnostics::EntryPointCannotReturnResourceType{
                .entryPoint = entryPointName,
                .returnType = returnType,
                .location = entryPointFuncDecl->loc});
        }

        if (as<ArrayExpressionType>(returnType))
        {
            sink->diagnose(Diagnostics::EntryPointCannotReturnArrayType{
                .entryPoint = entryPointName,
                .returnType = returnType,
                .location = entryPointFuncDecl->loc});
        }
    }

    // NOTE: Varying-parameter / return-type validation happens *after* the
    // auto-uniform classification below, so that parameters auto-marked
    // `uniform` on non-varying stages (e.g. `compute`) are skipped rather
    // than being diagnosed as if they were varyings.

    // Every entry point needs to have a stage specified either via
    // command-line/API options, or via an explicit `[shader("...")]` attribute.
    //
    if (stage == Stage::Unknown)
    {
        sink->diagnose(Diagnostics::EntryPointHasNoStage{
            .entryPoint = entryPointName->text,
            .location = entryPointFuncDecl->loc});
    }

    if (stage == Stage::Hull)
    {
        // TODO: We could consider *always* checking any `[patchconsantfunc("...")]`
        // attributes, so that they need to resolve to a function.

        auto attr = entryPointFuncDecl->findModifier<PatchConstantFuncAttribute>();

        if (attr)
        {
            if (attr->args.getCount() != 1)
            {
                sink->diagnose(Diagnostics::BadlyDefinedPatchConstantFunc{
                    .entryPointName = entryPointName,
                    .location = attr});
                return;
            }

            Expr* expr = attr->args[0];
            StringLiteralExpr* stringLit = as<StringLiteralExpr>(expr);

            if (!stringLit)
            {
                sink->diagnose(Diagnostics::BadlyDefinedPatchConstantFunc{
                    .entryPointName = entryPointName,
                    .location = attr});
                return;
            }

            // We look up the patch-constant function by its name in the module
            // scope of the translation unit that declared the HS entry point.
            //
            // TODO: Eventually we probably want to do the lookup in the scope
            // of the parent declarations of the entry point. E.g., if the entry
            // point is a member function of a `struct`, then its patch-constant
            // function should be allowed to be another member function of
            // the same `struct`.
            //
            // In the extremely long run we may want to support an alternative to
            // this attribute-based linkage between the two functions that
            // make up the entry point.
            //
            Name* name = linkage->getNamePool()->getName(stringLit->value);
            DeclRef<FuncDecl> patchConstantFuncDeclRef = findFunctionDeclByName(module, name, sink);
            if (!patchConstantFuncDeclRef)
            {
                sink->diagnose(Diagnostics::AttributeFunctionNotFound{
                    .funcName = name,
                    .attrName = "patchconstantfunc",
                    .location = expr});
                return;
            }

            attr->patchConstantFuncDecl = patchConstantFuncDeclRef.getDecl();
        }
    }
    else if (stage == Stage::Geometry)
    {
        bool hasOutputStream = false;
        for (const auto& param : entryPointFuncDecl->getParameters())
        {
            if (as<HLSLStreamOutputType>(param->getType()))
            {
                hasOutputStream = true;
                break;
            }
        }
        if (!hasOutputStream)
        {
            sink->diagnose(Diagnostics::GeometryShaderMissingOutputStream{
                .entryPoint = entryPointName,
                .location = entryPointFuncDecl->loc});
        }
        if (!entryPointFuncDecl->findModifier<MaxVertexCountAttribute>())
        {
            sink->diagnose(Diagnostics::GeometryShaderMissingMaxVertexCount{
                .entryPoint = entryPointName,
                .location = entryPointFuncDecl->loc});
        }
    }
    else if (stage == Stage::Mesh)
    {
        // A mesh shader must declare both an output topology and the
        // pair of mesh outputs (vertices + indices); otherwise the
        // generated SPIR-V is invalid (issue #9444). The geometry-shader
        // checks above are the equivalent precedent.
        if (!entryPointFuncDecl->findModifier<OutputTopologyAttribute>())
        {
            sink->diagnose(Diagnostics::MeshShaderMissingOutputTopology{
                .entryPoint = entryPointName,
                .location = entryPointFuncDecl->loc});
        }
        bool hasVerticesOutput = false;
        bool hasIndicesOutput = false;
        for (const auto& param : entryPointFuncDecl->getParameters())
        {
            auto meshOutputType = as<MeshOutputType>(param->getType());
            if (!meshOutputType)
                continue;
            if (as<VerticesType>(meshOutputType))
                hasVerticesOutput = true;
            else if (as<IndicesType>(meshOutputType))
                hasIndicesOutput = true;
        }
        if (!hasVerticesOutput || !hasIndicesOutput)
        {
            sink->diagnose(Diagnostics::MeshShaderMissingOutputs{
                .entryPoint = entryPointName,
                .location = entryPointFuncDecl->loc});
        }
    }
    else if (stage == Stage::Compute)
    {
        for (const auto& param : entryPointFuncDecl->getParameters())
        {
            if (auto semantic = param->findModifier<HLSLSimpleSemantic>())
            {
                const auto& semanticToken = semantic->name;

                String lowerName = String(semanticToken.getContent()).toLower();

                if (lowerName == "sv_dispatchthreadid")
                {
                    Type* paramType = param->getType();

                    if (!isValidThreadDispatchIDType(paramType))
                    {
                        sink->diagnose(Diagnostics::InvalidDispatchThreadIdType{
                            .type = paramType->toString(),
                            .location = param->loc});
                        return;
                    }
                }
            }
        }
    }

    // Accumulated here so the SV-semantic walk (just below) and the generic-struct pass (further
    // below) can both add to it before the per-target profile check consumes it.
    CapabilitySet entryPointInferredCaps{entryPointFuncDecl->inferredCapabilityRequirements};

    // Validate system value semantics, and collect the capabilities their accessors require:
    // general capability inference does not look through a semantic to the `[require]` on the
    // accessor it resolves to, so a need like `fragmentshaderbarycentric` on `SV_Barycentrics`
    // must be gathered here.
    {
        SharedSemanticsContext shared(linkage, module, sink);
        SemanticsVisitor visitor(&shared);

        // Use the session's coreLanguageScope which contains the SemanticDecl definitions
        // The module's own scope may not include the core module (e.g., when loading from
        // serialized module)
        if (auto scope = linkage->getSessionImpl()->coreLanguageScope)
        {
            // Validate system value semantics for entry point parameters
            for (const auto& param : entryPointFuncDecl->getParameters())
            {
                if (param->hasModifier<InOutModifier>())
                {
                    validateSystemValueSemantic(
                        &visitor,
                        sink,
                        param,
                        stage,
                        SemanticDirection::Input,
                        scope,
                        &entryPointInferredCaps);
                    validateSystemValueSemantic(
                        &visitor,
                        sink,
                        param,
                        stage,
                        SemanticDirection::Output,
                        scope,
                        &entryPointInferredCaps);
                }
                else if (param->hasModifier<OutModifier>())
                {
                    validateSystemValueSemantic(
                        &visitor,
                        sink,
                        param,
                        stage,
                        SemanticDirection::Output,
                        scope,
                        &entryPointInferredCaps);
                }
                else
                {
                    validateSystemValueSemantic(
                        &visitor,
                        sink,
                        param,
                        stage,
                        SemanticDirection::Input,
                        scope,
                        &entryPointInferredCaps);
                }
            }

            // Validate the return type semantic
            validateSystemValueSemantic(
                &visitor,
                sink,
                entryPointFuncDecl,
                stage,
                SemanticDirection::Output,
                scope,
                &entryPointInferredCaps);
        }
    }

    // HLSL allows a fragment shader to write at most one depth output. Slang previously
    // accepted multiple depth output semantics (e.g. both SV_Depth and SV_DepthGreaterEqual)
    // and silently produced a semantically wrong shader: the GLSL emitter assumes a single
    // directional depth qualifier on gl_FragDepth, and the SPIR-V emitter collapses the
    // conflicting per-variable depth execution modes to DepthReplacing (dropping the
    // directional hint). The per-parameter validation above checks each semantic in isolation
    // and never aggregates, so detect the conflict here across all of the entry point's
    // outputs and reject it uniformly on every target.
    if (stage == Stage::Fragment)
    {
        auto astBuilder = getCurrentASTBuilder();
        List<HLSLSimpleSemantic*> depthOutputSemantics;
        for (const auto& param : entryPointFuncDecl->getParameters())
        {
            // Depth semantics are output-only (setter-only in the core module; any input use
            // is already rejected), so only `out`/`inout` parameters can carry one.
            // InOutModifier derives from OutModifier, so this catches both.
            if (param->hasModifier<OutModifier>())
                collectDepthOutputSemantics(astBuilder, param, depthOutputSemantics);
        }
        // The return value is also an output of the entry point.
        collectDepthOutputSemantics(astBuilder, entryPointFuncDecl, depthOutputSemantics);

        if (depthOutputSemantics.getCount() > 1)
        {
            // Report at the second collected depth output — the one that makes the count
            // exceed one — and name it as conflicting with the first. Collection order is
            // parameters (in declaration order) then the return value, so the "second" is the
            // later contributor, not necessarily the lexically-later one.
            sink->diagnose(Diagnostics::MultipleDepthOutputSemantics{
                .conflictingSemantic = String(depthOutputSemantics[1]->name.getContent()),
                .earlierSemantic = String(depthOutputSemantics[0]->name.getContent()),
                .location = depthOutputSemantics[1]->loc});
        }
    }

    // For compute, mesh, and amplification (task) entry points using GLSL
    // syntax, the thread group size is specified via layout(local_size_x = N)
    // on a sibling EmptyDecl rather than via [numthreads] on the entry point
    // itself. GLSL allows each axis to be specified in a separate declaration,
    // so we merge all GLSLLayoutLocalSizeAttribute values into a single
    // NumThreadsAttribute.
    // Node shaders in thread-launch mode do not require [numthreads].
    bool isThreadLaunchNode = false;
    bool hasUncheckedNodeLaunchAttr = false;
    if (stage == Stage::Node)
    {
        auto launchAttr = entryPointFuncDecl->findModifier<NodeLaunchAttribute>();
        for (auto modifier = entryPointFuncDecl->modifiers.first; modifier;
             modifier = modifier->next)
        {
            auto uncheckedAttr = as<UncheckedAttribute>(modifier);
            if (!uncheckedAttr || !uncheckedAttr->keywordName)
                continue;

            if (uncheckedAttr->keywordName->text.getUnownedSlice() == toSlice("NodeLaunch"))
            {
                hasUncheckedNodeLaunchAttr = true;
                break;
            }
        }
        if (!launchAttr && !hasUncheckedNodeLaunchAttr)
        {
            sink->diagnose(Diagnostics::NodeLaunchAttributeRequired{.decl = entryPointFuncDecl});
        }
        isThreadLaunchNode = launchAttr && launchAttr->mode == kNodeLaunchModeThread;
    }

    if (isThreadLaunchNode)
    {
        if (auto numThreadsAttr = entryPointFuncDecl->findModifier<NumThreadsAttribute>())
        {
            sink->diagnose(
                Diagnostics::NumThreadsDisallowedOnThreadLaunchNode{.attr = numThreadsAttr});
        }
    }

    bool needsNumThreads = stage == Stage::Compute || stage == Stage::Mesh ||
                           stage == Stage::Amplification || stage == Stage::Node;
    if (needsNumThreads && !isThreadLaunchNode && !hasUncheckedNodeLaunchAttr &&
        !entryPointFuncDecl->findModifier<NumThreadsAttribute>())
    {
        auto parentDecl = entryPointFuncDecl->parentDecl;
        if (parentDecl)
        {
            NumThreadsAttribute* numThreads = nullptr;
            for (auto emptyDecl : parentDecl->getMembersOfType<EmptyDecl>())
            {
                auto glslAttr = emptyDecl->findModifier<GLSLLayoutLocalSizeAttribute>();
                if (!glslAttr)
                    continue;

                if (!numThreads)
                {
                    numThreads = getCurrentASTBuilder()->create<NumThreadsAttribute>();
                    for (int i = 0; i < 3; ++i)
                    {
                        numThreads->extents[i] = glslAttr->extents[i];
                        numThreads->specConstExtents[i] = glslAttr->specConstExtents[i];
                    }
                    // We attribute the location of the new NumThreadsAttribute
                    // to the location of the first GLSLLayoutLocalSizeAttribute,
                    // just to have something there (even if multiple
                    // attributes get merged to this NumThreadsAttribute).
                    numThreads->loc = glslAttr->loc;
                }
                else
                {
                    // Merge: for each axis, take the non-default value.
                    // GLSL defaults unspecified axes to 1.
                    for (int i = 0; i < 3; ++i)
                    {
                        if (glslAttr->specConstExtents[i])
                        {
                            numThreads->extents[i] = nullptr;
                            numThreads->specConstExtents[i] = glslAttr->specConstExtents[i];
                        }
                        else if (glslAttr->extents[i])
                        {
                            if (auto cint = as<ConstantIntVal>(glslAttr->extents[i]))
                            {
                                if (cint->getValue() != 1)
                                    numThreads->extents[i] = glslAttr->extents[i];
                            }
                            else
                            {
                                numThreads->extents[i] = glslAttr->extents[i];
                            }
                        }
                    }
                }
            }
            if (numThreads)
                addModifier(entryPointFuncDecl, numThreads);
        }
        if (stage == Stage::Node && !entryPointFuncDecl->findModifier<NumThreadsAttribute>())
        {
            sink->diagnose(
                Diagnostics::NodeNumThreadsAttributeRequired{.decl = entryPointFuncDecl});
        }
    }

    bool canHaveVaryingInput = false;
    bool shouldWarnOnNonUniformParam = true;
    switch (stage)
    {
    case Stage::Vertex:
    case Stage::Fragment:
    case Stage::Miss:
    case Stage::AnyHit:
    case Stage::ClosestHit:
    case Stage::Callable:
    case Stage::Geometry:
    case Stage::Mesh:
    case Stage::Hull:
    case Stage::Domain:
        canHaveVaryingInput = true;
        break;
    case Stage::Dispatch:
        shouldWarnOnNonUniformParam = false;
        break;
    case Stage::Node:
        {
            canHaveVaryingInput = true;
            auto hasMaxGrid = entryPointFuncDecl->findModifier<NodeMaxDispatchGridAttribute>();
            auto hasFixedGrid = entryPointFuncDecl->findModifier<NodeDispatchGridAttribute>();
            auto launchAttr = entryPointFuncDecl->findModifier<NodeLaunchAttribute>();
            // Fixed and maximum dispatch-grid attributes are valid only on broadcasting nodes,
            // e.g. `[NodeLaunch("broadcasting")] [NodeDispatchGrid(1, 1, 1)]`.
            if ((hasMaxGrid || hasFixedGrid) && launchAttr &&
                launchAttr->mode != kNodeLaunchModeBroadcasting)
            {
                sink->diagnose(
                    Diagnostics::NodeGridAttributeRequiresBroadcasting{.decl = entryPointFuncDecl});
            }
            break;
        }
    default:
        break;
    }

    for (const auto& param : entryPointFuncDecl->getParameters())
    {
        if (auto allowSparseNodesAttr = param->findModifier<AllowSparseNodesAttribute>())
        {
            // `[AllowSparseNodes]` is valid on node output arrays, e.g.
            // `[AllowSparseNodes] NodeOutputArray<MyRecord> outputs` or
            // `[AllowSparseNodes] EmptyNodeOutputArray outputs`.
            auto paramType = param->getType();
            if (!isIntrinsicTypeWithOp(paramType, kIROp_NodeOutputArrayType) &&
                !isIntrinsicTypeWithOp(paramType, kIROp_EmptyNodeOutputArrayType))
            {
                sink->diagnose(Diagnostics::AllowSparseNodesRequiresNodeOutputArray{
                    .attr = allowSparseNodesAttr});
            }
        }

        if (isUniformParameterType(param->getType()))
        {
            // Automatically add `uniform` modifier to entry point parameters.
            if (!param->hasModifier<HLSLUniformModifier>())
            {
                addModifier(param, getCurrentASTBuilder()->create<HLSLUniformModifier>());
                continue;
            }
        }

        if (canHaveVaryingInput)
            continue;

        // If the stage doesn't allow varying input/output,
        // we require the parameter to be associated with a system value semantic.
        if (param->hasModifier<HLSLUniformModifier>())
            continue;
        if (param->findModifier<HLSLSemantic>())
            continue;

        bool isBuiltinType = isBuiltinParameterType(param->getType());
        if (isBuiltinType)
            continue;

        if (doStructFieldsHaveSemantic(param->getType()))
            continue;

        // The user is defining a parameter with no 'uniform' modifier for a stage that doesn't
        // support varying input/output. We will automatically convert it to a 'uniform' parameter,
        // and diagnose a warning.
        addModifier(param, getCurrentASTBuilder()->create<HLSLUniformModifier>());
        if (shouldWarnOnNonUniformParam)
        {
            sink->diagnose(
                Diagnostics::NonUniformEntryPointParameterTreatedAsUniform{.param = param});
        }
    }

    // Validate that varying parameter/return types are legal. Runs after the
    // auto-uniform classification above so that parameters that will end up
    // being treated as uniform on non-varying stages are not diagnosed.
    {
        List<CodeGenTarget> targets;
        for (auto target : linkage->targets)
            targets.add(target->getTarget());

        auto astBuilder = linkage->getASTBuilder();

        // Validate return type as varying output
        if (returnType)
        {
            VaryingTypeValidationContext ctx;
            ctx.astBuilder = astBuilder;
            ctx.sink = sink;
            ctx.entryPointName = entryPointName;
            ctx.loc = entryPointFuncDecl->loc;
            ctx.direction = "output";
            ctx.context = "return type";
            ctx.targets = targets.getArrayView();
            ctx.stage = stage;
            validateVaryingType(ctx, returnType);
        }

        // Validate each parameter that would be treated as varying. Parameters
        // that were auto-marked uniform by the loop above are skipped here.
        for (const auto& param : entryPointFuncDecl->getParameters())
        {
            if (param->hasModifier<HLSLUniformModifier>())
                continue;
            if (isUniformParameterType(param->getType()))
                continue;

            VaryingTypeValidationContext ctx;
            ctx.astBuilder = astBuilder;
            ctx.sink = sink;
            ctx.entryPointName = entryPointName;
            ctx.loc = param->loc;
            // Note: `InOutModifier` inherits from `OutModifier`, so it must be
            // checked first to avoid mislabeling `inout` parameters as "output".
            ctx.direction = param->hasModifier<InOutModifier>() ? "input/output"
                            : param->hasModifier<OutModifier>() ? "output"
                            : param->hasModifier<RefModifier>() ? "input/output"
                                                                : "input";

            StringBuilder contextSb;
            auto paramName = param->getName();
            if (paramName)
                contextSb << "parameter '" << paramName->text << "'";
            else
                contextSb << "parameter";
            String contextStr = contextSb.produceString();
            ctx.context = contextStr.getBuffer();
            ctx.targets = targets.getArrayView();
            ctx.stage = stage;
            validateVaryingType(ctx, param->getType());
        }
    }

    // For vertex shaders, warn when an output has been declared but none
    // of them carry the `SV_Position` semantic. This is almost always a
    // bug: the rasterizer needs an output position from the last
    // vertex-processing stage. Cases where the vertex shader is
    // intentionally producing no position (e.g. it feeds a
    // tessellation/geometry/mesh stage that supplies SV_Position itself,
    // or rasterizer-discard / transform feedback is in use) are rare;
    // users who hit this can add the semantic to a vertex output, or
    // silence the warning explicitly.
    //
    // We deliberately skip the check when the entry point declares no
    // outputs at all (void return type, no `out`/`inout` parameters).
    // GLSL-style entry points write `gl_Position` via a global rather
    // than as a returned member, so we cannot tell whether SV_Position
    // is missing just by looking at the signature.
    if (stage == Stage::Vertex)
    {
        auto astBuilder = linkage->getASTBuilder();
        const auto svPosition = UnownedStringSlice::fromLiteral("sv_position");

        auto returnBasicType = as<BasicExpressionType>(returnType);
        bool returnIsVoid = returnBasicType && returnBasicType->getBaseType() == BaseType::Void;
        bool hasOutputs = returnType && !returnIsVoid;
        bool hasSvPosition =
            _outputDeclHasSemantic(astBuilder, entryPointFuncDecl, returnType, svPosition);

        if (!hasSvPosition)
        {
            for (const auto& param : entryPointFuncDecl->getParameters())
            {
                // Only outputs (or in/out) of the entry point can carry
                // SV_Position for the rasterizer.
                if (!param->hasModifier<OutModifier>() && !param->hasModifier<InOutModifier>())
                    continue;
                hasOutputs = true;
                if (_outputDeclHasSemantic(astBuilder, param, param->getType(), svPosition))
                {
                    hasSvPosition = true;
                    break;
                }
            }
        }

        if (hasOutputs && !hasSvPosition)
        {
            sink->diagnose(Diagnostics::VertexShaderMissingSvPosition{
                .entryPoint = entryPointName,
                .location = entryPointFuncDecl->loc});
        }
    }

    // Attribute and keyword diagnostics. Check for ignored [[vk::binding]] and
    // [[vk::push_constant]] attributes, and the register() and packoffset() keywords on entry
    // point parameters. Slang currently ignores these in the cases diagnosed below, which can lead
    // to user confusion whenever the output does not correspond to what was requested. Conversely,
    // Slang silently generating output that just happens to align with what's requested can also
    // lead to user confusion, with the user mistakenly believing that the modifiers are working as
    // intended.
    //
    // Note that this only checks when they're used on entry point parameters.
    bool supportsVkBindingOnEntryPointParameters =
        _allTargetsSupportVkBindingOnEntryPointParameters(linkage);
    for (const auto& param : entryPointFuncDecl->getParameters())
    {
        auto astBuilder = linkage->getASTBuilder();
        bool supportsVkBindingOnParameter =
            supportsVkBindingOnEntryPointParameters &&
            isVkBindingCompatibleEntryPointParameterType(astBuilder, param->getType());
        if (!supportsVkBindingOnParameter && param->findModifier<GLSLBindingAttribute>())
        {
            sink->diagnose(Diagnostics::UnhandledModOnEntryPointParameter{
                .modifier = "attribute '[[vk::binding(...)]]'",
                .param = param->getName(),
                .location = param->loc});
        }
        if (param->findModifier<PushConstantAttribute>())
        {
            sink->diagnose(Diagnostics::UnhandledModOnEntryPointParameter{
                .modifier = "attribute '[[vk::push_constant]]'",
                .param = param->getName(),
                .location = param->loc});
        }
        if (param->findModifier<HLSLRegisterSemantic>())
        {
            sink->diagnose(Diagnostics::UnhandledModOnEntryPointParameter{
                .modifier = "keyword 'register'",
                .param = param->getName(),
                .location = param->loc});
        }
        if (param->findModifier<HLSLPackOffsetSemantic>())
        {
            sink->diagnose(Diagnostics::UnhandledModOnEntryPointParameter{
                .modifier = "keyword 'packoffset'",
                .param = param->getName(),
                .location = param->loc});
        }
    }

    // Augment the entry point's inferred requirements with the capability
    // requirements of the *generic* struct types in its signature (parameters and
    // return type). The general inference walk records a type's requirements only
    // when its decl-ref is a `DirectDeclRef`, so a non-generic struct such as
    // `Foo a` is already covered there, but a generic one such as `Foo<int> a`
    // (whose decl-ref is a `GenericAppDeclRef`) is not. We gather the missing
    // generic-struct requirements here so a `[require(...)]` on `Foo` is enforced
    // for both spellings. `signatureStructUses` keeps each contributing struct and
    // its use location so we can point the diagnostic at the exact use site (the
    // non-generic case is reported by `diagnoseMissingCapabilityProvenance`).
    List<GenericStructTypeUse> signatureStructUses;
    {
        auto astBuilder = linkage->getASTBuilder();
        // Use a fresh `visited` set per signature position. `Val` nodes are
        // hash-consed, so the same specialization `Foo<int>` on two parameters is
        // the identical `Val*`; a shared set would drop the second use site and the
        // user would see only one "see using of 'Foo'" note. The per-position set
        // still guards against cycles within a single type.
        for (auto param : entryPointFuncDecl->getParameters())
        {
            // Prefer the written type-expression location (the `Foo<int>` use
            // site); fall back to the parameter location if no type syntax was
            // retained.
            SourceLoc useLoc = (param->type.exp) ? param->type.exp->loc : param->loc;
            HashSet<Val*> visited;
            collectGenericStructTypeUses(
                astBuilder,
                param->getType(),
                useLoc,
                visited,
                signatureStructUses);
        }
        // The return type has the same silent-compile bug as parameters: a
        // `Foo<int> main()` whose `Foo` requires an unavailable capability must be
        // diagnosed too.
        SourceLoc returnLoc = (entryPointFuncDecl->returnType.exp)
                                  ? entryPointFuncDecl->returnType.exp->loc
                                  : entryPointFuncDecl->loc;
        HashSet<Val*> visited;
        collectGenericStructTypeUses(
            astBuilder,
            entryPointFuncDecl->returnType.type,
            returnLoc,
            visited,
            signatureStructUses);
    }
    // Every collected use carries a non-empty requirement (filtered in the
    // collector), so this join is unconditional.
    for (auto& use : signatureStructUses)
        entryPointInferredCaps.nonDestructiveJoin(use.structDecl->inferredCapabilityRequirements);

    for (auto target : linkage->targets)
    {
        auto targetCaps = target->getTargetCaps();
        auto stageCapabilitySet = entryPoint->getProfile().getCapabilityName();
        targetCaps.join(stageCapabilitySet);
        if (targetCaps.isIncompatibleWith(entryPointInferredCaps))
        {
            // Incompatable means we don't support a set of abstract atoms.
            // Diagnose that we lack support for 'stage' and 'target' atoms with our provided
            // entry-point
            auto compileTarget = target->getTargetCaps().getCompileTarget();
            auto stageTarget = stageCapabilitySet.getTargetStage();
            maybeDiagnose(
                sink,
                linkage->m_optionSet,
                DiagnosticCategory::Capability,
                Diagnostics::EntryPointUsesUnavailableCapability{
                    .stage = capabilityNameToString((CapabilityName)stageTarget),
                    .target = capabilityNameToString((CapabilityName)compileTarget),
                    .decl = entryPointFuncDecl});

            // Find out what is incompatible (ancestor missing a super set of 'target+stage')
            CapabilitySet failedSet({(CapabilityName)compileTarget, (CapabilityName)stageTarget});
            diagnoseMissingCapabilityProvenance(
                linkage->m_optionSet,
                sink,
                entryPointFuncDecl,
                failedSet);

            // The provenance walk above follows `capabilityRequirementProvenance`,
            // which does not record generic struct signature types. Point at any
            // such struct whose requirement is itself incompatible with the
            // target, mirroring the notes emitted for non-generic structs.
            for (auto& use : signatureStructUses)
            {
                if (!targetCaps.isIncompatibleWith(
                        CapabilitySet{use.structDecl->inferredCapabilityRequirements}))
                    continue;
                maybeDiagnose(
                    sink,
                    linkage->m_optionSet,
                    DiagnosticCategory::Capability,
                    Diagnostics::SeeUsingOf{.decl = use.structDecl, .location = use.useLoc});
                maybeDiagnose(
                    sink,
                    linkage->m_optionSet,
                    DiagnosticCategory::Capability,
                    Diagnostics::SeeDefinitionOf{.decl = use.structDecl});
                if (auto requireAttr = use.structDecl->findModifier<RequireCapabilityAttribute>())
                    maybeDiagnose(
                        sink,
                        linkage->m_optionSet,
                        DiagnosticCategory::Capability,
                        Diagnostics::SeeDeclarationOfModifier{.modifier = requireAttr});
            }
        }
        else
        {
            auto& targetOptionSet = target->getOptionSet();
            bool specificProfileRequested =
                targetOptionSet.hasOption(CompilerOptionName::Profile) &&
                (targetOptionSet.getIntOption(CompilerOptionName::Profile) !=
                 SLANG_PROFILE_UNKNOWN);
            bool specificCapabilityRequested = false;
            for (auto atomVal : targetOptionSet.getArray(CompilerOptionName::Capability))
            {
                switch (atomVal.kind)
                {
                case CompilerOptionValueKind::Int:
                    if (atomVal.intValue != SLANG_CAPABILITY_UNKNOWN)
                        specificCapabilityRequested = true;
                    break;
                case CompilerOptionValueKind::String:
                    // User made a specific capability request
                    specificCapabilityRequested = true;
                    break;
                }
                if (specificCapabilityRequested)
                    break;
            }

            if (auto declaredCapsMod =
                    entryPointFuncDecl->findModifier<ExplicitlyDeclaredCapabilityModifier>())
            {
                // If the entry point has an explicitly declared capability, then we
                // will merge that with the target capability set before checking if
                // there is an implicit upgrade.
                targetCaps.nonDestructiveJoin(declaredCapsMod->declaredCapabilityRequirements);
            }

            // Only attempt to error if a specific profile or capability is requested
            if ((specificCapabilityRequested || specificProfileRequested) &&
                targetCaps.atLeastOneSetImpliedInOther(entryPointInferredCaps) ==
                    CapabilitySet::ImpliesReturnFlags::NotImplied)
            {
                CapabilitySet combinedSets = targetCaps;
                combinedSets.join(entryPointInferredCaps);
                CapabilityAtomSet addedAtoms{};
                if (auto targetCapSet = targetCaps.getAtomSets())
                {
                    if (auto combinedSet = combinedSets.getAtomSets())
                    {
                        CapabilityAtomSet::calcSubtract(
                            addedAtoms,
                            (*combinedSet),
                            (*targetCapSet));
                    }
                }
                StringBuilder entryPointNameSb;
                printDiagnosticArg(entryPointNameSb, entryPointFuncDecl);
                auto atoms = addedAtoms.getElements<CapabilityAtom>();
                StringBuilder capsSb;
                printDiagnosticArg(capsSb, atoms);
                maybeDiagnoseWarningOrError(
                    sink,
                    target->getOptionSet(),
                    DiagnosticCategory::Capability,
                    Diagnostics::ProfileImplicitlyUpgraded{
                        .entryPoint = entryPointNameSb.produceString(),
                        .profile = target->getOptionSet().getProfile().getName(),
                        .capabilities = capsSb.produceString(),
                        .location = entryPointFuncDecl->loc,
                    },
                    Diagnostics::ProfileImplicitlyUpgradedRestrictive{
                        .entryPoint = entryPointNameSb.produceString(),
                        .profile = target->getOptionSet().getProfile().getName(),
                        .capabilities = capsSb.produceString(),
                        .location = entryPointFuncDecl->loc,
                    });
            }
        }
    }
}

bool resolveStageOfProfileWithEntryPoint(
    Profile& entryPointProfile,
    CompilerOptionSet& optionSet,
    const List<RefPtr<TargetRequest>>& targets,
    FuncDecl* entryPointFuncDecl,
    DiagnosticSink* sink)
{
    if (auto entryPointAttr = entryPointFuncDecl->findModifier<EntryPointAttribute>())
    {
        auto entryPointProfileStage = entryPointProfile.getStage();
        auto entryPointStage =
            getStageFromAtom(CapabilitySet{entryPointAttr->capabilitySet}.getTargetStage());

        // Ensure every target is specifying the same stage as an entry-point
        // if a profile+stage was set, else user will not be aware that their
        // code is requiring `fragment` on a `vertex` shader
        for (auto target : targets)
        {
            auto targetProfile = target->getOptionSet().getProfile();
            auto profileStage = targetProfile.getStage();
            if (profileStage != Stage::Unknown && profileStage != entryPointStage)
                maybeDiagnose(
                    sink,
                    optionSet,
                    DiagnosticCategory::Capability,
                    Diagnostics::EntryPointAndProfileAreIncompatible{
                        .decl = entryPointFuncDecl,
                        .stage = getStageName(entryPointStage),
                        .profile = targetProfile.getName(),
                        .location = entryPointAttr->loc});
        }
        if (entryPointProfileStage == Stage::Unknown)
            entryPointProfile = Profile(entryPointStage);
        else if (
            entryPointProfileStage != Stage::Unknown && entryPointProfileStage != entryPointStage)
            maybeDiagnose(
                sink,
                optionSet,
                DiagnosticCategory::Capability,
                Diagnostics::SpecifiedStageDoesntMatchAttribute{
                    .entryPoint = entryPointFuncDecl->getName()->text,
                    .compiledStage = getStageName(entryPointProfileStage),
                    .attributeStage = getStageName(entryPointStage),
                    .location = entryPointFuncDecl->loc});
        entryPointProfile.additionalCapabilities.add(CapabilitySet{entryPointAttr->capabilitySet});
        return true;
    }
    return false;
}

// Given an entry point specified via API or command line options,
// attempt to find a matching AST declaration that implements the specified
// entry point. If such a function is found, then validate that it actually
// meets the requirements for the selected stage/profile.
//
// Returns an `EntryPoint` object representing the (unspecialized)
// entry point if it is found and validated, and null otherwise.
//
RefPtr<EntryPoint> findAndValidateEntryPoint(FrontEndEntryPointRequest* entryPointReq)
{
    // The first step in validating the entry point is to find
    // the (unique) function declaration that matches its name.
    //
    // TODO: We may eventually want/need to extend this to
    // account for nested names like `SomeStruct.vsMain`, or
    // indeed even to handle generics.
    //
    auto compileRequest = entryPointReq->getCompileRequest();
    auto translationUnit = entryPointReq->getTranslationUnit();
    auto linkage = compileRequest->getLinkage();
    auto sink = compileRequest->getSink();

    auto entryPointName = entryPointReq->getName();
    DeclRef<FuncDecl> entryPointFuncDeclRef =
        findFunctionDeclByName(translationUnit->getModule(), entryPointName, sink);

    // Did we find a function declaration in our search?
    if (!entryPointFuncDeclRef)
    {
        return nullptr;
    }

    // TODO: it is possible that the entry point was declared with
    // profile or target overloading. Is there anything that we need
    // to do at this point to filter out declarations that aren't
    // relevant to the selected profile for the entry point?

    // We found something, and can start doing some basic checking.
    //
    // If the entry point specifies a stage via a `[shader("...")]` attribute,
    // then we might be able to infer a stage for the entry point request if
    // it didn't have one, *or* issue a diagnostic if there is a mismatch with the profile.

    auto entryPointProfile = entryPointReq->getProfile();
    resolveStageOfProfileWithEntryPoint(
        entryPointProfile,
        linkage->m_optionSet,
        linkage->targets,
        entryPointFuncDeclRef.getDecl(),
        sink);
    // TODO: Should we attach a `[shader(...)]` attribute to an
    // entry point that didn't have one, so that we can have
    // a more uniform representation in the AST?

    RefPtr<EntryPoint> entryPoint =
        EntryPoint::create(linkage, entryPointFuncDeclRef, entryPointProfile);

    // Now that we've *found* the entry point, it is time to validate
    // that it actually meets the constraints for the chosen stage/profile.
    //
    validateEntryPoint(entryPoint, sink);

    // We should return nullptr if entry point fails to validate
    if (sink->getErrorCount())
    {
        return nullptr;
    }

    return entryPoint;
}

/// Get the name a variable will use for reflection purposes
Name* getReflectionName(VarDeclBase* varDecl)
{
    if (auto reflectionNameModifier = varDecl->findModifier<ParameterGroupReflectionName>())
        return reflectionNameModifier->nameAndLoc.name;

    return varDecl->getName();
}

Type* getParamValueType(ASTBuilder* astBuilder, DeclRef<ParamDecl> paramDeclRef)
{
    auto paramType = getType(astBuilder, paramDeclRef);
    if (paramDeclRef.getDecl()->findModifier<NoDiffModifier>())
    {
        auto modifierVal = static_cast<Val*>(astBuilder->getOrCreate<NoDiffModifierVal>());
        paramType = astBuilder->getModifiedType(paramType, 1, &modifierVal);
    }
    return paramType;
}

Type* getParamTypeWithModeWrapper(ASTBuilder* astBuilder, DeclRef<ParamDecl> paramDeclRef)
{
    auto paramValueType = getParamValueType(astBuilder, paramDeclRef);
    auto paramMode = getParamPassingMode(paramDeclRef.getDecl());
    return getParamTypeWithModeWrapper(astBuilder, paramValueType, paramMode);
}

Type* getParamTypeWithModeWrapper(
    ASTBuilder* astBuilder,
    Type* paramValueType,
    ParamPassingMode paramMode)
{
    switch (paramMode)
    {
    case ParamPassingMode::In:
        return paramValueType;
    case ParamPassingMode::BorrowIn:
        return astBuilder->getConstRefParamType(paramValueType);
    case ParamPassingMode::Out:
        return astBuilder->getOutParamType(paramValueType);
    case ParamPassingMode::BorrowInOut:
        return astBuilder->getBorrowInOutParamType(paramValueType);
    case ParamPassingMode::Ref:
        return astBuilder->getRefParamType(paramValueType);
    default:
        SLANG_UNEXPECTED("unhandled parameter-passing mode");
        UNREACHABLE_RETURN(paramValueType);
    }
}

void Module::_collectShaderParams(DiagnosticSink* sink)
{
    // We are going to walk the global declarations in the body of the
    // module, and use those to build up our lists of:
    //
    // * Global shader parameters
    // * Specialization parameters (both generic and interface/existential)
    // * Requirements (`import`ed modules)
    //
    // For requirements, we want to be careful to only
    // add each required module once (in case the same
    // module got `import`ed multiple times), so we
    // will keep a set of the modules we've already
    // seen and processed.
    //

    // We need to use a work list to traverse through all global scopes,
    // including the top level `moduleDecl` and all the included `FileDecl`s.

    List<ContainerDecl*> workList;
    workList.add(m_moduleDecl);

    HashSet<Module*> requiredModuleSet;
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto moduleDecl = workList[i];
        for (auto globalDecl : moduleDecl->getDirectMemberDecls())
        {
            if (auto globalVar = as<VarDecl>(globalDecl))
            {
                // We do not want to consider global variable declarations
                // that don't represents shader parameters. This includes
                // things like `static` globals and `groupshared` variables.
                //
                if (!isGlobalShaderParameter(globalVar))
                {
                    bool isVarying = false;
                    for (auto m : globalVar->modifiers)
                    {
                        if (as<InModifier>(m) || as<OutModifier>(m))
                        {
                            isVarying = true;
                            break;
                        }
                    }
                    if (!isVarying)
                        continue;
                }

                // At this point we know we have a global shader parameter.

                ShaderParamInfo shaderParamInfo;
                shaderParamInfo.paramDeclRef = makeDeclRef(globalVar);

                // We need to consider what specialization parameters
                // are introduced by this shader parameter. This step
                // fills in fields on `shaderParamInfo` so that we
                // can assocaite specialization arguments supplied later
                // with the correct parameter.
                //
                if (!_collectExistentialSpecializationParamsForShaderParam(
                        getLinkage()->getASTBuilder(),
                        shaderParamInfo,
                        m_specializationParams,
                        makeDeclRef(globalVar),
                        sink))
                {
                    return;
                }

                m_shaderParams.add(shaderParamInfo);
            }
            else if (auto globalGenericParam = as<GlobalGenericParamDecl>(globalDecl))
            {
                // A global generic type parameter declaration introduces
                // a suitable specialization parameter.
                //
                SpecializationParam specializationParam;
                specializationParam.flavor = SpecializationParam::Flavor::GenericType;
                specializationParam.loc = globalGenericParam->loc;
                specializationParam.object = globalGenericParam;
                m_specializationParams.add(specializationParam);
            }
            else if (auto globalGenericValueParam = as<GlobalGenericValueParamDecl>(globalDecl))
            {
                // A global generic type parameter declaration introduces
                // a suitable specialization parameter.
                //
                SpecializationParam specializationParam;
                specializationParam.flavor = SpecializationParam::Flavor::GenericValue;
                specializationParam.loc = globalGenericValueParam->loc;
                specializationParam.object = globalGenericValueParam;
                m_specializationParams.add(specializationParam);
            }
            else if (auto importDecl = as<ImportDecl>(globalDecl))
            {
                // An `import` declaration creates a requirement dependency
                // from this module to another module.
                //
                auto importedModule = getModule(importDecl->importedModuleDecl);
                if (!requiredModuleSet.contains(importedModule))
                {
                    requiredModuleSet.add(importedModule);
                    m_requirements.add(importedModule);
                }
            }
            else if (auto fileDecl = as<FileDecl>(globalDecl))
            {
                // If we see a `FileDecl`, we need to recursively look into its
                // scope.
                workList.add(fileDecl);
            }
            else if (auto namespaceDecl = as<NamespaceDecl>(globalDecl))
            {
                workList.add(namespaceDecl);
            }
        }
    }
}

Index Module::getRequirementCount()
{
    return m_requirements.getCount();
}

RefPtr<ComponentType> Module::getRequirement(Index index)
{
    return m_requirements[index];
}

void Module::acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
{
    visitor->visitModule(this, as<ModuleSpecializationInfo>(specializationInfo));
}


/// Create a new component type based on `inComponentType`, but with all its requiremetns filled.
RefPtr<ComponentType> fillRequirements(ComponentType* inComponentType)
{
    auto linkage = inComponentType->getLinkage();

    // We are going to simplify things by solving the problem iteratively.
    // If the current `componentType` has requirements for `A`, `B`, ... etc.
    // then we will create a composite of `componentType`, `A`, `B`, ...
    // and then see if the resulting composite has any requirements.
    //
    // This avoids the problem of trying to compute teh transitive closure
    // of the requirements relationship (while dealing with deduplication,
    // etc.)

    RefPtr<ComponentType> componentType = inComponentType;
    for (;;)
    {
        auto requirementCount = componentType->getRequirementCount();
        if (requirementCount == 0)
            break;

        List<RefPtr<ComponentType>> allComponents;
        allComponents.add(componentType);

        for (Index rr = 0; rr < requirementCount; ++rr)
        {
            auto requirement = componentType->getRequirement(rr);
            allComponents.add(requirement);
        }

        componentType = CompositeComponentType::create(linkage, allComponents);
    }
    return componentType;
}

bool parseTypeConformanceArgString(
    UnownedStringSlice optionString,
    UnownedStringSlice& outTypeName,
    UnownedStringSlice& outInterfaceName,
    Index& outSequentialId)
{
    // The expected format for the type conformance argument is:
    // `TypeName:InterfaceName[=SequentialId]`
    //
    // Where `TypeName` is the name of a concrete type, `InterfaceName`
    // is the name of an interface type, and `SequentialId` is an optional
    // integer that specifies a sequential ID for the conformance.
    //
    // If the string does not match this format, we will return false.

    outTypeName = UnownedStringSlice();
    outInterfaceName = UnownedStringSlice();
    outSequentialId = -1;
    auto colonPos = optionString.indexOf(':');
    if (colonPos < 0)
    {
        // If there is no colon, then the string is invalid.
        return false;
    }
    outTypeName = optionString.head(colonPos);
    auto interfaceNameStart = colonPos + 1;
    auto equalsPos = optionString.indexOf('=');
    if (equalsPos < interfaceNameStart)
    {
        // If there is no equals sign, then the interface name goes to the end of the string.
        outInterfaceName = optionString.tail(interfaceNameStart);
    }
    else
    {
        // If there is an equals sign, then the interface name goes up to that point.
        outInterfaceName =
            optionString.subString(interfaceNameStart, equalsPos - interfaceNameStart);
        // The sequential ID is the part after the equals sign.
        auto sequentialIdString = optionString.tail(equalsPos + 1);
        if (SLANG_FAILED(StringUtil::parseInt(sequentialIdString, outSequentialId)))
            return false;
    }
    return true;
}

/// Create a component type to represent the "global scope" of a compile request.
///
/// This component type will include all the modules and their global
/// parameters from the compile request, but not anything specific
/// to any entry point functions.
///
/// The layout for this component type will thus represent the things that
/// a user is likely to want to have stay the same across all compiled
/// entry points.
///
/// The component type that this function creates is unspecialized, in
/// that it doesn't take into account any specialization arguments
/// that might have been supplied as part of the compile request.
///
RefPtr<ComponentType> createUnspecializedGlobalComponentType(FrontEndCompileRequest* compileRequest)
{
    // We want our resulting program to depend on
    // all the translation units the user specified,
    // even if some of them don't contain entry points
    // (this is important for parameter layout/binding).
    //
    // We also want to ensure that the modules for the
    // translation units comes first in the enumerated
    // order for dependencies, to match the pre-existing
    // compiler behavior (at least for now).
    //
    auto linkage = compileRequest->getLinkage();

    RefPtr<ComponentType> globalComponentType;
    if (compileRequest->translationUnits.getCount() == 1)
    {
        // The common case is that a compilation only uses
        // a single translation unit, and thus results in
        // a single `Module`. We can then use that module
        // as the component type that represents the global scope.
        //
        globalComponentType = compileRequest->translationUnits[0]->getModule();
    }
    else
    {
        List<RefPtr<ComponentType>> translationUnitComponentTypes;
        for (auto tu : compileRequest->translationUnits)
        {
            translationUnitComponentTypes.add(tu->getModule());
        }

        globalComponentType =
            CompositeComponentType::create(linkage, translationUnitComponentTypes);
    }

    List<RefPtr<ComponentType>> conformanceComponents;

    // Find and include all type conformances specified through compiler options.
    for (auto conformances :
         compileRequest->optionSet.getArray(CompilerOptionName::TypeConformance))
    {
        auto stringValue = conformances.stringValue.getUnownedSlice();
        UnownedStringSlice typeName, interfaceName;
        Index sequentialId = -1;
        if (!parseTypeConformanceArgString(stringValue, typeName, interfaceName, sequentialId))
        {
            compileRequest->getSink()->diagnose(
                Diagnostics::InvalidTypeConformanceOptionString{.option = stringValue});
            continue;
        }
        DiagnosticSink typeLookupSink(linkage->getSourceManager(), nullptr);
        auto concreteType =
            globalComponentType->getTypeFromString(String(typeName).getBuffer(), &typeLookupSink);
        if (!concreteType || as<ErrorType>(concreteType))
        {
            compileRequest->getSink()->diagnose(Diagnostics::InvalidTypeConformanceOptionNoType{
                .option = stringValue,
                .typeName = typeName});
            continue;
        }
        auto interfaceType = globalComponentType->getTypeFromString(
            String(interfaceName).getBuffer(),
            &typeLookupSink);
        if (!interfaceType || as<ErrorType>(interfaceType))
        {
            compileRequest->getSink()->diagnose(Diagnostics::InvalidTypeConformanceOptionNoType{
                .option = stringValue,
                .typeName = interfaceName});
            continue;
        }
        ComPtr<slang::ITypeConformance> conformanceComponent;
        ComPtr<ISlangBlob> diagnostics;
        compileRequest->getLinkage()->createTypeConformanceComponentType(
            (slang::TypeReflection*)concreteType,
            (slang::TypeReflection*)interfaceType,
            conformanceComponent.writeRef(),
            sequentialId,
            diagnostics.writeRef());
        if (!conformanceComponent)
        {
            // If we failed to create the conformance component, then
            // we should report the diagnostics that were generated.
            //
            compileRequest->getSink()->diagnose(
                Diagnostics::CannotCreateTypeConformance{.conformance = stringValue});
            if (diagnostics)
            {
                compileRequest->getSink()->diagnoseRaw(
                    Severity::Error,
                    UnownedStringSlice((char*)diagnostics->getBufferPointer()));
            }
            continue;
        }
        conformanceComponents.add(static_cast<TypeConformance*>(conformanceComponent.get()));
    }

    if (conformanceComponents.getCount() > 0)
    {
        // If we found any type conformances, then we will
        // create a composite component type that includes
        // the global component type and the conformance components.
        //
        conformanceComponents.add(globalComponentType);
        globalComponentType = CompositeComponentType::create(linkage, conformanceComponents);
    }

    return fillRequirements(globalComponentType);
}

void FrontEndCompileRequest::checkEntryPoints()
{
    auto linkage = getLinkage();
    SLANG_AST_BUILDER_RAII(linkage->getASTBuilder());

    auto sink = getSink();

    // The validation of entry points here will be modal, and controlled
    // by whether the user specified any entry points directly via
    // API or command-line options.
    //
    // TODO: We may want to make this choice explicit rather than implicit.
    //
    // First, check if the user requested any entry points explicitly via
    // the API or command line.
    //
    bool anyExplicitEntryPoints = getEntryPointReqCount() != 0;

    if (anyExplicitEntryPoints)
    {
        // If there were any explicit requests for entry points to be
        // checked, then we will *only* check those.
        //
        for (auto entryPointReq : getEntryPointReqs())
        {
            auto entryPoint = findAndValidateEntryPoint(entryPointReq);
            if (entryPoint)
            {
                // TODO: We need to implement an explicit policy
                // for what should happen if the user specified
                // entry points via the command-line (or API),
                // but didn't specify any groups (since the current
                // compilation API doesn't allow for grouping).
                //
                entryPointReq->getTranslationUnit()->module->_addEntryPoint(entryPoint);
            }
        }

        // TODO: We should consider always processing both categories,
        // and just making sure to only check each entry point function
        // declaration once...
    }
    else
    {
        // Otherwise, scan for any `[shader(...)]` attributes in
        // the user's code, and construct `EntryPoint`s to
        // represent them.
        //
        // This ensures that downstream code only has to consider
        // the central list of entry point requests, and doesn't
        // have to know where they came from.

        // TODO: A comprehensive approach here would need to search
        // recursively for entry points, because they might appear
        // as, e.g., member function of a `struct` type.
        //
        // For now we'll start with an extremely basic approach that
        // should work for typical HLSL code.
        //
        Index translationUnitCount = translationUnits.getCount();
        for (Index tt = 0; tt < translationUnitCount; ++tt)
        {
            auto translationUnit = translationUnits[tt];
            translationUnit->getModule()->_discoverEntryPoints(sink, this->getLinkage()->targets);
        }
    }
}


/// Create a component type that represents the global scope for a compile request,
/// along with any entry point functions.
///
/// The resulting component type will include the global-scope information
/// first, so its layout will be compatible with the result of
/// `createUnspecializedGlobalComponentType`.
///
/// The new component type will also add on any entry-point functions
/// that were requested and will thus include space for their `uniform` parameters.
/// If multiple entry points were requested then they will be given non-overlapping
/// parameter bindings, consistent with them being used together in
/// a single pipeline state, hit group, etc.
///
/// The result of this function is unspecialized and doesn't take into
/// account any specialization arguments the user might have supplied.
///
RefPtr<ComponentType> createUnspecializedGlobalAndEntryPointsComponentType(
    FrontEndCompileRequest* compileRequest,
    List<RefPtr<ComponentType>>& outUnspecializedEntryPoints)
{
    auto linkage = compileRequest->getLinkage();

    auto globalComponentType = compileRequest->getGlobalComponentType();

    List<RefPtr<ComponentType>> allComponentTypes;
    allComponentTypes.add(globalComponentType);

    Index translationUnitCount = compileRequest->translationUnits.getCount();
    for (Index tt = 0; tt < translationUnitCount; ++tt)
    {
        auto translationUnit = compileRequest->translationUnits[tt];
        auto module = translationUnit->getModule();

        for (auto entryPoint : module->getEntryPoints())
        {
            outUnspecializedEntryPoints.add(entryPoint);
            allComponentTypes.add(entryPoint);
        }
    }

    // Also consider entry points that were introduced via adding
    // a library reference...
    //
    for (auto extraEntryPoint : compileRequest->m_extraEntryPoints)
    {
        auto entryPoint = EntryPoint::createDummyForDeserialize(
            linkage,
            extraEntryPoint.name,
            extraEntryPoint.profile,
            extraEntryPoint.mangledName);
        allComponentTypes.add(entryPoint);
    }

    if (allComponentTypes.getCount() > 1)
    {
        auto composite = CompositeComponentType::create(linkage, allComponentTypes);
        return composite;
    }
    else
    {
        return globalComponentType;
    }
}

RefPtr<ComponentType::SpecializationInfo> Module::_validateSpecializationArgsImpl(
    SpecializationArg const* args,
    Index argCount,
    Index& outConsumedArgCount,
    DiagnosticSink* sink)
{
    if (argCount < getSpecializationParamCount())
    {
        sink->diagnose(Diagnostics::MismatchSpecializationArguments{
            .expected = (int64_t)getSpecializationParamCount(),
            .provided = (int64_t)argCount});
        return nullptr;
    }
    outConsumedArgCount = getSpecializationParamCount();
    argCount = outConsumedArgCount;

    SharedSemanticsContext semanticsContext(getLinkage(), this, sink);
    SemanticsVisitor visitor(&semanticsContext);

    RefPtr<Module::ModuleSpecializationInfo> specializationInfo =
        new Module::ModuleSpecializationInfo();

    for (Index ii = 0; ii < argCount; ++ii)
    {
        auto& arg = args[ii];
        auto& param = m_specializationParams[ii];

        switch (param.flavor)
        {
        case SpecializationParam::Flavor::GenericType:
            {
                auto genericTypeParamDecl = as<GlobalGenericParamDecl>(param.object);
                SLANG_ASSERT(genericTypeParamDecl);

                Type* argType = as<Type>(arg.val);
                if (!argType)
                {
                    sink->diagnose(Diagnostics::ExpectedTypeForSpecializationArg{
                        .param = genericTypeParamDecl->getName()->text,
                        .location = param.loc});
                    argType = getLinkage()->getASTBuilder()->getErrorType();
                }

                // TODO: There is a serious flaw to this checking logic if we ever have cases where
                // the constraints on one `type_param` can depend on another `type_param`, e.g.:
                //
                //      type_param A;
                //      type_param B : ISidekick<A>;
                //
                // In that case, if a user tries to set `B` to `Robin` and `Robin` conforms to
                // `ISidekick<Batman>`, then the compiler needs to know whether `A` is being
                // set to `Batman` to know whether the setting for `B` is valid. In this limit
                // the constraints can be mutually recursive (so `A : IMentor<B>`).
                //
                // The only way to check things correctly is to validate each conformance under
                // a set of assumptions (substitutions) that includes all the type substitutions,
                // and possibly also all the other constraints *except* the one to be validated.
                //
                // We will punt on this for now, and just check each constraint in isolation.

                // As a quick sanity check, see if the argument that is being supplied for a
                // global generic type parameter is a reference to *another* global generic
                // type parameter, since that should always be an error.
                //
                if (auto argDeclRefType = as<DeclRefType>(argType))
                {
                    auto argDeclRef = argDeclRefType->getDeclRef();
                    if (auto argGenericParamDeclRef = argDeclRef.as<GlobalGenericParamDecl>())
                    {
                        if (argGenericParamDeclRef.getDecl() == genericTypeParamDecl)
                        {
                            // We are trying to specialize a generic parameter using itself.
                            sink->diagnose(Diagnostics::CannotSpecializeGlobalGenericToItself{
                                .param = genericTypeParamDecl->getName(),
                                .location = genericTypeParamDecl->loc});
                            continue;
                        }
                        else
                        {
                            // We are trying to specialize a generic parameter using a *different*
                            // global generic type parameter.
                            sink->diagnose(
                                Diagnostics::CannotSpecializeGlobalGenericToAnotherGenericParam{
                                    .param = genericTypeParamDecl->getName(),
                                    .otherParam = argGenericParamDeclRef.getName(),
                                    .location = genericTypeParamDecl->loc});
                            continue;
                        }
                    }
                }

                ModuleSpecializationInfo::GenericArgInfo genericArgInfo;
                genericArgInfo.paramDecl = genericTypeParamDecl;
                genericArgInfo.argVal = argType;
                specializationInfo->genericArgs.add(genericArgInfo);

                // Walk through the declared constraints for the parameter,
                // and check that the argument actually satisfies them.
                for (auto constraintDecl :
                     genericTypeParamDecl->getMembersOfType<GenericTypeConstraintDecl>())
                {
                    // Get the type that the constraint is enforcing conformance to
                    auto interfaceType = getSup(
                        getLinkage()->getASTBuilder(),
                        DeclRef<GenericTypeConstraintDecl>(constraintDecl));

                    // Use our semantic-checking logic to search for a witness to the required
                    // conformance
                    auto witness =
                        visitor.isSubtype(argType, interfaceType, IsSubTypeOptions::None);
                    if (!witness)
                    {
                        // If no witness was found, then we will be unable to satisfy
                        // the conformances required.
                        sink->diagnose(
                            Diagnostics::TypeArgumentForGenericParameterDoesNotConformToInterface{
                                .typeArg = argType,
                                .param = genericTypeParamDecl->nameAndLoc.name,
                                .interface = interfaceType,
                                .location = genericTypeParamDecl->loc});
                    }

                    ModuleSpecializationInfo::GenericArgInfo constraintArgInfo;
                    constraintArgInfo.paramDecl = constraintDecl;
                    constraintArgInfo.argVal = witness;
                    specializationInfo->genericArgs.add(constraintArgInfo);
                }
            }
            break;

        case SpecializationParam::Flavor::ExistentialType:
            {
                auto interfaceType = as<Type>(param.object);
                SLANG_ASSERT(interfaceType);

                Type* argType = as<Type>(arg.val);
                if (!argType)
                {
                    sink->diagnose(Diagnostics::ExpectedTypeForSpecializationArg{
                        .param = interfaceType ? interfaceType->toString() : "<unknown type>",
                        .location = param.loc});
                    argType = getLinkage()->getASTBuilder()->getErrorType();
                }

                auto witness = visitor.isSubtype(argType, interfaceType, IsSubTypeOptions::None);
                if (!witness)
                {
                    // If no witness was found, then we will be unable to satisfy
                    // the conformances required.
                    sink->diagnose(Diagnostics::TypeArgumentDoesNotConformToInterface{
                        .typeArg = argType,
                        .interface = interfaceType});
                }

                ExpandedSpecializationArg expandedArg;
                expandedArg.val = argType;
                expandedArg.witness = witness;

                specializationInfo->existentialArgs.add(expandedArg);
            }
            break;

        case SpecializationParam::Flavor::GenericValue:
            {
                auto paramDecl = as<GlobalGenericValueParamDecl>(param.object);
                SLANG_ASSERT(paramDecl);

                // Now we need to check that the argument `Val` has the
                // appropriate type expected by the parameter.

                IntVal* intVal = as<IntVal>(arg.val);
                if (!intVal)
                {
                    sink->diagnose(Diagnostics::ExpectedValueOfTypeForSpecializationArg{
                        .type = paramDecl->getType(),
                        .param = paramDecl->getName()->text,
                        .location = param.loc});
                    intVal =
                        getLinkage()->getASTBuilder()->getIntVal(m_astBuilder->getIntType(), 0);
                }

                ModuleSpecializationInfo::GenericArgInfo expandedArg;
                expandedArg.paramDecl = paramDecl;
                expandedArg.argVal = intVal;

                specializationInfo->genericArgs.add(expandedArg);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled specialization parameter flavor");
        }
    }

    return specializationInfo;
}


static void _extractSpecializationArgs(
    ComponentType* componentType,
    List<Expr*> const& argExprs,
    List<SpecializationArg>& outArgs,
    DiagnosticSink* sink)
{
    auto linkage = componentType->getLinkage();

    SharedSemanticsContext semanticsContext(linkage, nullptr, sink);
    SemanticsVisitor semanticsVisitor(&semanticsContext);

    auto argCount = argExprs.getCount();
    for (Index ii = 0; ii < argCount; ++ii)
    {
        auto argExpr = argExprs[ii];

        SpecializationArg arg;
        arg.val = semanticsVisitor.ExtractGenericArgVal(argExpr);
        outArgs.add(arg);
    }
}

RefPtr<ComponentType::SpecializationInfo> EntryPoint::_validateSpecializationArgsImpl(
    SpecializationArg const* inArgs,
    Index inArgCount,
    Index& outConsumedArgCount,
    DiagnosticSink* sink)
{
    SLANG_AST_BUILDER_RAII(getLinkage()->getASTBuilder());

    auto args = inArgs;
    auto argCount = inArgCount;

    // Validating a specialization argument means checking that the argument
    // type conforms to the entry point's generic constraints, and that check
    // needs to see every `extension` that could supply a conformance witness.
    //
    // Scope the checking session to the entry point's own module rather than
    // leaving it module-less. A module-less (`m_module == nullptr`) context
    // resolves extensions from the linkage's `loadedModulesList`, which never
    // contains the primary command-line translation unit -- so a conformance
    // provided by an `extension` in that primary source (e.g. specializing a
    // `T : IFoo` entry point to a type whose `T : IFoo` witness comes from an
    // `extension T : IFoo` in the same file) was invisible and failed with
    // E38029, even though the identical call resolves fine in the module body.
    //
    // With `m_module` set, `getCandidateExtensionsForTypeDecl` instead consults
    // `importedModulesList`, so we seed it from the entry point's module
    // dependency closure. `getModuleDependencies()` self-includes the owning
    // module (see `Module::Module`'s `addModuleDependency(this)`), so the
    // primary module's own extensions come along -- this is the same
    // point-of-view an in-body generic call has.
    //
    // When the entry point has no owning module (`getModule()` is null) we pass
    // `nullptr` and fall back to the prior module-less behavior.
    auto entryPointModule = getModule();
    SharedSemanticsContext sharedSemanticsContext(getLinkage(), entryPointModule, sink);
    if (entryPointModule)
    {
        for (auto module : getModuleDependencies())
        {
            auto moduleDecl = module->getModuleDecl();
            if (sharedSemanticsContext.importedModulesSet.add(moduleDecl))
                sharedSemanticsContext.importedModulesList.add(moduleDecl);
        }
    }
    SemanticsVisitor visitor(&sharedSemanticsContext);

    // The last N arguments will be for the implicit existential arguments
    // of the entry point (if it has any).
    //
    auto existentialSpecializationParamCount = getExistentialSpecializationParamCount();
    auto genericSpecializationParamCount = getGenericSpecializationParamCount();

    RefPtr<EntryPointSpecializationInfo> info = new EntryPointSpecializationInfo();

    DeclRef<FuncDecl> specializedFuncDeclRef = m_funcDeclRef;
    Index genericArgCount = genericSpecializationParamCount;
    if (genericSpecializationParamCount)
    {
        // We need to construct a generic application and use
        // the semantic checking machinery to expand out
        // the rest of the arguments via inference...

        auto genericDeclRef = m_funcDeclRef.getParent().as<GenericDecl>();
        SLANG_ASSERT(genericDeclRef); // otherwise we wouldn't have generic parameters

        bool isVariadic =
            (genericDeclRef.getDecl()->getMembersOfType<GenericTypePackParamDecl>().getCount() !=
             0) ||
            (genericDeclRef.getDecl()->getMembersOfType<GenericValuePackParamDecl>().getCount() !=
             0);

        // If function is variadic generic, it will consume all the provided arguments.
        if (isVariadic)
            genericArgCount = argCount - existentialSpecializationParamCount;

        if (genericArgCount < 0)
        {
            sink->diagnose(Diagnostics::MismatchSpecializationArguments{
                .expected = (int64_t)(genericSpecializationParamCount +
                                      existentialSpecializationParamCount),
                .provided = (int64_t)argCount});
            return nullptr;
        }

        List<Expr*> genericArgs;

        auto astBuilder = getLinkage()->getASTBuilder();
        for (Index ii = 0; ii < genericArgCount; ++ii)
        {
            auto specializationArg = args[ii];
            if (specializationArg.expr)
            {
                genericArgs.add(specializationArg.expr);
                continue;
            }
            if (auto typeVal = as<Type>(specializationArg.val))
            {
                auto typeExpr = astBuilder->create<SharedTypeExpr>();
                typeExpr->type = astBuilder->getTypeType(typeVal);
                genericArgs.add(typeExpr);
            }
            else if (auto intVal = as<ConstantIntVal>(specializationArg.val))
            {
                if (intVal->getType() == astBuilder->getBoolType())
                {
                    auto intExpr = astBuilder->create<BoolLiteralExpr>();
                    intExpr->type = intVal->getType();
                    intExpr->value = intVal->getValue() != 0;
                    genericArgs.add(intExpr);
                }
                else
                {
                    auto intExpr = astBuilder->create<IntegerLiteralExpr>();
                    intExpr->type = intVal->getType();
                    intExpr->value = intVal->getValue();
                    genericArgs.add(intExpr);
                }
            }
            else
            {
                sink->diagnose(Diagnostics::InvalidFormOfSpecializationArg{.index = ii + 1});
            }
        }
        auto genAppExpr = astBuilder->create<GenericAppExpr>();
        auto genExpr = astBuilder->create<VarExpr>();
        genExpr->declRef = genericDeclRef;
        genExpr->type = astBuilder->getOrCreate<GenericDeclRefType>();
        genExpr->checked = true;
        genAppExpr->functionExpr = genExpr;
        genAppExpr->arguments = _Move(genericArgs);
        auto checkedExpr = visitor.CheckTerm(genAppExpr);
        if (auto partiallyAppliedExpr = as<PartiallyAppliedGenericExpr>(checkedExpr))
        {
            // Entry-point specialization can leave a generic partially applied
            // after parsing the explicit specialization arguments. The generic
            // solver completes that decl-ref from the provided ordinary
            // arguments, declaration-time defaults, and witness constraints. An
            // otherwise empty inference context is enough here because there are
            // no value-level call arguments to unify against entry-point
            // parameters.
            SemanticsVisitor::GenericInferenceContext inferenceContext;
            inferenceContext.genericDecl = genericDeclRef.getDecl();
            ConversionCost outCost;
            specializedFuncDeclRef =
                visitor
                    .trySolveGenericArguments(
                        _Move(inferenceContext),
                        genericDeclRef,
                        partiallyAppliedExpr->providedOrdinaryArgs.getArrayView(),
                        outCost)
                    .as<FuncDecl>();
        }
        else if (auto declRefExpr = as<DeclRefExpr>(checkedExpr))
        {
            specializedFuncDeclRef = declRefExpr->declRef.as<FuncDecl>();
        }

        if (!specializedFuncDeclRef)
            return nullptr;
    }

    info->specializedFuncDeclRef = specializedFuncDeclRef;

    // Once the generic parameters (if any) have been dealt with,
    // any remaining specialization arguments are for existential/interface
    // specialization parameters, attached to the value parameters
    // of the entry point.
    //
    args += genericArgCount;
    argCount -= genericArgCount;
    outConsumedArgCount = genericArgCount + existentialSpecializationParamCount;

    if (argCount < existentialSpecializationParamCount)
    {
        sink->diagnose(Diagnostics::MismatchSpecializationArguments{
            .expected =
                (int64_t)(genericSpecializationParamCount + existentialSpecializationParamCount),
            .provided = (int64_t)argCount});
        return nullptr;
    }

    for (Index ii = 0; ii < existentialSpecializationParamCount; ++ii)
    {
        auto& param = m_existentialSpecializationParams[ii];
        auto& specializationArg = args[ii];

        // TODO: We need to handle all the cases of "flavor" for the `param`s (not just types)

        auto paramType = as<Type>(param.object);
        auto argType = as<Type>(specializationArg.val);

        auto witness = visitor.isSubtype(argType, paramType, IsSubTypeOptions::None);
        if (!witness)
        {
            // If no witness was found, then we will be unable to satisfy
            // the conformances required.
            sink->diagnose(Diagnostics::TypeArgumentDoesNotConformToInterface{
                .typeArg = argType,
                .interface = paramType});
            continue;
        }

        ExpandedSpecializationArg expandedArg;
        expandedArg.val = specializationArg.val;
        expandedArg.witness = witness;
        info->existentialSpecializationArgs.add(expandedArg);
    }

    return info;
}

/// Create a specialization an existing entry point based on specialization argument expressions.
RefPtr<ComponentType> createSpecializedEntryPoint(
    EntryPoint* unspecializedEntryPoint,
    List<Expr*> const& argExprs,
    DiagnosticSink* sink)
{
    // We need to convert all of the `Expr` arguments
    // into `SpecializationArg`s, so that we can bottleneck
    // through the shared logic.
    //
    List<SpecializationArg> args;
    _extractSpecializationArgs(unspecializedEntryPoint, argExprs, args, sink);
    if (sink->getErrorCount())
        return nullptr;

    return ((ComponentType*)unspecializedEntryPoint)
        ->specialize(args.getBuffer(), args.getCount(), sink);
}

Scope* ComponentType::_getOrCreateScopeForLegacyLookup(ASTBuilder* astBuilder)
{
    // The shape of this logic is dictated by the legacy
    // behavior for name-based lookup/parsing of types
    // specified via the API or command line.
    //
    // We begin with a dummy scope that has as its parent
    // the scope that provides the "base" language
    // definitions (that scope is necessary because
    // it defines keywords like `true` and `false`).
    //
    if (m_lookupScope)
        return m_lookupScope;

    Scope* scope = astBuilder->create<Scope>();
    scope->parent = getLinkage()->getSessionImpl()->slangLanguageScope;
    //
    // Next, the scope needs to include all of the
    // modules in the program as peers, as if they
    // were `import`ed into the scope.
    //
    for (auto module : getModuleDependencies())
    {
        for (auto srcScope = module->getModuleDecl()->ownedScope; srcScope;
             srcScope = srcScope->nextSibling)
        {
            if (srcScope->containerDecl != module->getModuleDecl() &&
                srcScope->containerDecl->parentDecl != module->getModuleDecl())
                continue; // Skip scopes that is not part of current module.

            Scope* moduleScope = astBuilder->create<Scope>();
            moduleScope->containerDecl = srcScope->containerDecl;

            moduleScope->nextSibling = scope->nextSibling;
            scope->nextSibling = moduleScope;
        }
    }
    m_lookupScope = scope;
    return scope;
}

/// Parse an array of strings as specialization arguments.
///
/// Names in the strings will be parsed in the context of
/// the code loaded into the given compile request.
///
void parseSpecializationArgStrings(
    EndToEndCompileRequest* endToEndReq,
    List<String> const& genericArgStrings,
    List<Expr*>& outGenericArgs)
{
    auto unspecialiedProgram = endToEndReq->getUnspecializedGlobalComponentType();

    // TODO(JS):
    //
    // We create the scopes on the linkages ASTBuilder. We might want to create a temporary
    // ASTBuilder, and let that memory get freed, but is like this because it's not clear if the
    // scopes in ASTNode members will dangle if we do.
    Scope* scope = unspecialiedProgram->_getOrCreateScopeForLegacyLookup(
        endToEndReq->getLinkage()->getASTBuilder());

    // We are going to do some semantic checking, so we need to
    // set up a `SemanticsVistitor` that we can use.
    //
    auto linkage = endToEndReq->getLinkage();
    auto sink = endToEndReq->getSink();

    SharedSemanticsContext sharedSemanticsContext(linkage, nullptr, sink);
    SemanticsVisitor semantics(&sharedSemanticsContext);

    // We will be looping over the generic argument strings
    // that the user provided via the API (or command line),
    // and parsing+checking each into an `Expr`.
    //
    // This loop will *not* handle coercing the arguments
    // to be types.
    //
    for (auto name : genericArgStrings)
    {
        Expr* argExpr = linkage->parseTermString(name, scope);
        argExpr = semantics.CheckTerm(argExpr);

        if (!argExpr)
        {
            sink->diagnose(Diagnostics::InternalCompilerError{});
            return;
        }

        outGenericArgs.add(argExpr);
    }
}

Type* Linkage::specializeType(
    Type* unspecializedType,
    Int argCount,
    Type* const* args,
    DiagnosticSink* sink)
{
    SLANG_ASSERT(unspecializedType);

    // TODO: We should cache and re-use specialized types
    // when the exact same arguments are provided again later.

    SharedSemanticsContext sharedSemanticsContext(this, nullptr, sink);
    SemanticsVisitor visitor(&sharedSemanticsContext);

    SpecializationParams specializationParams;
    if (!_collectExistentialSpecializationParamsRec(
            getASTBuilder(),
            specializationParams,
            unspecializedType,
            SourceLoc(),
            sink,
            0))
    {
        return nullptr;
    }

    SLANG_ASSERT(specializationParams.getCount() == argCount);
    if (specializationParams.getCount() != argCount)
        return nullptr;

    ExpandedSpecializationArgs specializationArgs;
    for (Int aa = 0; aa < argCount; ++aa)
    {
        auto paramType = as<Type>(specializationParams[aa].object);
        auto argType = args[aa];

        ExpandedSpecializationArg arg;
        arg.val = argType;
        arg.witness = visitor.isSubtype(argType, paramType, IsSubTypeOptions::None);
        specializationArgs.add(arg);
    }

    ExistentialSpecializedType* specializedType =
        m_astBuilder->getOrCreate<ExistentialSpecializedType>(
            unspecializedType,
            specializationArgs);

    m_specializedTypes.add(specializedType);

    return specializedType;
}

/// Shared implementation logic for the `_createSpecializedProgram*` entry points.
static RefPtr<ComponentType> _createSpecializedProgramImpl(
    Linkage* linkage,
    ComponentType* unspecializedProgram,
    List<Expr*> const& specializationArgExprs,
    DiagnosticSink* sink)
{
    // If there are no specialization arguments,
    // then the the result of specialization should
    // be the same as the input.
    //
    auto specializationArgCount = specializationArgExprs.getCount();
    if (specializationArgCount == 0)
    {
        return unspecializedProgram;
    }

    auto specializationParamCount = unspecializedProgram->getSpecializationParamCount();
    if (specializationArgCount != specializationParamCount)
    {
        sink->diagnose(Diagnostics::MismatchSpecializationArguments{
            .expected = (int64_t)specializationParamCount,
            .provided = (int64_t)specializationArgCount});
        return nullptr;
    }

    // We have an appropriate number of arguments for the global specialization parameters,
    // and now we need to check that the arguments conform to the declared constraints.
    //
    SharedSemanticsContext visitor(linkage, nullptr, sink);

    List<SpecializationArg> specializationArgs;
    _extractSpecializationArgs(
        unspecializedProgram,
        specializationArgExprs,
        specializationArgs,
        sink);
    if (sink->getErrorCount())
        return nullptr;

    auto specializedProgram = unspecializedProgram->specialize(
        specializationArgs.getBuffer(),
        specializationArgs.getCount(),
        sink);

    return specializedProgram;
}

/// Specialize an entry point that was checked by the front-end, based on specialization arguments.
///
/// If the end-to-end compile request included specialization argument strings
/// for this entry point, then they will be parsed, checked, and used
/// as arguments to the generic entry point.
///
/// Returns a specialized entry point if everything worked as expected.
/// Returns null and diagnoses errors if anything goes wrong.
///
RefPtr<ComponentType> createSpecializedEntryPoint(
    EndToEndCompileRequest* endToEndReq,
    EntryPoint* unspecializedEntryPoint,
    EndToEndCompileRequest::EntryPointInfo const& entryPointInfo)
{
    auto sink = endToEndReq->getSink();

    // If the user specified generic arguments for the entry point,
    // then we will need to parse the arguments first.
    //
    List<Expr*> specializationArgExprs;
    parseSpecializationArgStrings(
        endToEndReq,
        entryPointInfo.specializationArgStrings,
        specializationArgExprs);

    // Next we specialize the entry point function given the parsed
    // generic argument expressions.
    //
    auto entryPoint =
        createSpecializedEntryPoint(unspecializedEntryPoint, specializationArgExprs, sink);

    return entryPoint;
}

/// Create a specialized component type for the global scope of the given compile request.
///
/// The specialized program will be consistent with that created by
/// `createUnspecializedGlobalComponentType`, and will simply fill in
/// its specialization parameters with the arguments (if any) supllied
/// as part fo the end-to-end compile request.
///
/// The layout of the new component type will be consistent with that
/// of the original *if* there are no global generic type parameters
/// (only interface/existential parameters).
///
RefPtr<ComponentType> createSpecializedGlobalComponentType(EndToEndCompileRequest* endToEndReq)
{
    // The compile request must have already completed front-end processing,
    // so that we have an unspecialized program available, and now only need
    // to parse and check any generic arguments that are being supplied for
    // global or entry-point generic parameters.
    //
    auto unspecializedProgram = endToEndReq->getUnspecializedGlobalComponentType();
    auto linkage = endToEndReq->getLinkage();
    auto sink = endToEndReq->getSink();

    // First, let's parse the specialization argument strings that were
    // provided via the API, so that we can match them
    // against what was declared in the program.
    //
    List<Expr*> globalSpecializationArgs;
    parseSpecializationArgStrings(
        endToEndReq,
        endToEndReq->m_globalSpecializationArgStrings,
        globalSpecializationArgs);

    // Don't proceed further if anything failed to parse.
    if (sink->getErrorCount())
        return nullptr;

    // Now we create the initial specialized program by
    // applying the global generic arguments (if any) to the
    // unspecialized program.
    //
    auto specializedProgram = _createSpecializedProgramImpl(
        linkage,
        unspecializedProgram,
        globalSpecializationArgs,
        sink);

    // If anything went wrong with the global generic
    // arguments, then bail out now.
    //
    if (!specializedProgram)
        return nullptr;

    // Next we will deal with the entry points for the
    // new specialized program.
    //
    // If the user specified explicit entry points as part of the
    // end-to-end request, then we only want to process those (and
    // ignore any other `[shader(...)]`-attributed entry points).
    //
    // However, if the user specified *no* entry points as part
    // of the end-to-end request, then we would like to go
    // ahead and consider all the entry points that were found
    // by the front-end.
    //
    Index entryPointCount = endToEndReq->m_entryPoints.getCount();
    if (entryPointCount == 0)
    {
        entryPointCount = unspecializedProgram->getEntryPointCount();
        endToEndReq->m_entryPoints.setCount(entryPointCount);
    }

    return specializedProgram;
}

/// Diagnose an unspecialized generic entry point, reporting it against the
/// entry-point function's source location.
///
/// A generic entry point is only legal if it is specialized with concrete
/// generic arguments (via `-specialize` / `addEntryPointEx`). An
/// *unspecialized* generic entry point cannot be lowered: it would produce an
/// `IRGeneric` rather than an `IRFunc` and crash IR linking (issue #10209).
static void diagnoseGenericEntryPoint(EntryPoint* entryPoint, DiagnosticSink* sink)
{
    auto funcDecl = entryPoint->getFuncDecl();
    sink->diagnose(Diagnostics::EntryPointCannotBeGeneric{
        .entryPoint = funcDecl->getName(),
        .location = funcDecl->loc});
}

/// Create a specialized program based on the given compile request.
///
/// The specialized program created here includes both the global
/// scope for all the translation units involved and all the entry
/// points, and it also includes any specialization arguments
/// that were supplied.
///
/// It is important to note that this function specializes
/// the global scope and the entry points in isolation and then
/// composes them, and that this can lead to different layout
/// from the result of `createUnspecializedGlobalAndEntryPointsComponentType`.
///
/// If we have a module `M` with entry point `E`, and each has one
/// specialization parameter, then `createUnspecialized...` will yield:
///
///     compose(M,E)
///
/// That composed type will have two specialization parameters (the one
/// from `M` plus the one from `E`) and so we might specialize it to get:
///
///     specialize(compose(M,E), X, Y)
///
/// while if we use `createSpecialized...` we will get:
///
///     compose(specialize(M,X), specialize(E,Y))
///
/// While these options are semantically equivalent, they would not lay
/// out the same way in memory.
///
/// There are many reasons why an application might prefer one over the
/// other, and an application that cares should use the more explicit
/// APIs to construct what they want. The behavior of this function
/// is just to provide a reasonable default for use by end-to-end
/// compilation (e.g., from the command line).
///
RefPtr<ComponentType> createSpecializedGlobalAndEntryPointsComponentType(
    EndToEndCompileRequest* endToEndReq,
    List<RefPtr<ComponentType>>& outSpecializedEntryPoints)
{
    auto specializedGlobalComponentType = endToEndReq->getSpecializedGlobalComponentType();

    List<RefPtr<ComponentType>> allComponentTypes;
    allComponentTypes.add(specializedGlobalComponentType);

    auto unspecializedGlobalAndEntryPointsComponentType =
        endToEndReq->getUnspecializedGlobalAndEntryPointsComponentType();

    // It is possible that there were entry points other than those specified
    // vai the original end-to-end compile request. In particular:
    //
    // * It is possible to compile with *no* entry points specified, in which
    //   case the current compiler behavior is to use any entry points marked
    //   via `[shader(...)]` attributes in the AST.
    //
    // * It is possible for entry points to come into play via serialized libraries
    //   loaded with `-r` on the command line (or the equivalent API).
    //
    // We will thus draw a distinction between the "specified" entry points,
    // and the "found" entry points.
    //
    auto specifiedEntryPointCount = endToEndReq->m_entryPoints.getCount();
    auto foundEntryPointCount =
        unspecializedGlobalAndEntryPointsComponentType->getEntryPointCount();

    SLANG_ASSERT(foundEntryPointCount >= specifiedEntryPointCount);

    // For any entry points that were specified, we can use the specialization
    // argument information provided via API or command line.
    //
    for (Index ii = 0; ii < specifiedEntryPointCount; ++ii)
    {
        auto& entryPointInfo = endToEndReq->m_entryPoints[ii];
        auto unspecializedEntryPoint =
            unspecializedGlobalAndEntryPointsComponentType->getEntryPoint(ii);

        // A generic entry point must have concrete generic arguments. They can
        // arrive either bound into the entry-point name (`-entry foo<int>`,
        // reflected in the func declRef) or as separate specialization-arg
        // strings (`-specialize`/`addEntryPointEx`, applied below by
        // `createSpecializedEntryPoint`). If neither is present, the generic is
        // unspecialized and would lower to an `IRGeneric` rather than an
        // `IRFunc`, crashing IR linking (issue #10209); reject it here.
        //
        // `isSpecialized` walks the whole enclosing decl chain and compares the
        // declRef's generic args against the defaults, so it correctly accepts
        // name-bound args (and nested-generic entry points) while still flagging
        // a genuinely unbound generic.
        if (!endToEndReq->getLinkage()->isSpecialized(unspecializedEntryPoint->getFuncDeclRef()) &&
            entryPointInfo.specializationArgStrings.getCount() == 0)
        {
            diagnoseGenericEntryPoint(unspecializedEntryPoint, endToEndReq->getSink());
            continue;
        }

        auto specializedEntryPoint =
            createSpecializedEntryPoint(endToEndReq, unspecializedEntryPoint, entryPointInfo);
        allComponentTypes.add(specializedEntryPoint);

        outSpecializedEntryPoints.add(specializedEntryPoint);
    }

    // There might have been errors during the specialization above,
    // so we will bail out early if anything went wrong, rather
    // then try to create a composite where some of the constituent
    // component types might be null.
    //
    if (endToEndReq->getSink()->getErrorCount() != 0)
        return nullptr;

    // Any entry points beyond those that were specified up front will be
    // assumed to not need/want specialization.
    //
    for (Index ii = specifiedEntryPointCount; ii < foundEntryPointCount; ++ii)
    {
        auto unspecializedEntryPoint =
            unspecializedGlobalAndEntryPointsComponentType->getEntryPoint(ii);

        // These entry points (e.g. discovered via `[shader(...)]`) carry no
        // specialization arguments, so an unspecialized generic one can never be
        // specialized and must be rejected (#10209). `isSpecialized` is false
        // only when the generic args are still the defaults (a discovered
        // `[shader]` entry point can't bind args via its name either).
        if (!endToEndReq->getLinkage()->isSpecialized(unspecializedEntryPoint->getFuncDeclRef()))
        {
            diagnoseGenericEntryPoint(unspecializedEntryPoint, endToEndReq->getSink());
            continue;
        }

        allComponentTypes.add(unspecializedEntryPoint);
        outSpecializedEntryPoints.add(unspecializedEntryPoint);
    }

    // Bail out if rejecting a generic entry point above raised an error,
    // rather than composing a program with a missing entry point.
    if (endToEndReq->getSink()->getErrorCount() != 0)
        return nullptr;

    RefPtr<ComponentType> composed =
        CompositeComponentType::create(endToEndReq->getLinkage(), allComponentTypes);
    return composed;
}

SourceLoc _getTypeNestingDiagnosticPosForDecl(Decl* decl)
{
    if (auto varDecl = as<VarDeclBase>(decl))
    {
        auto loc = getDiagnosticPos(varDecl->type);
        if (loc.isValid())
            return loc;
    }
    else if (auto callableDecl = as<CallableDecl>(decl))
    {
        auto loc = getDiagnosticPos(callableDecl->returnType);
        if (loc.isValid())
            return loc;
    }

    return getDiagnosticPos(decl);
}

} // namespace Slang

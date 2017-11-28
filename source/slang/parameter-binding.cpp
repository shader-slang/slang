// parameter-binding.cpp
#include "parameter-binding.h"

#include "lookup.h"
#include "compiler.h"
#include "type-layout.h"

#include "../../slang.h"

namespace Slang {

struct ParameterInfo;

// Information on ranges of registers already claimed/used
struct UsedRange
{
    // What parameter has claimed this range?
    ParameterInfo*  parameter = nullptr;

    // Begin/end of the range (half-open interval)
    UInt begin;
    UInt end;
};
bool operator<(UsedRange left, UsedRange right)
{
    if (left.begin != right.begin)
        return left.begin < right.begin;
    if (left.end != right.end)
        return left.end < right.end;
    return false;
}

static bool rangesOverlap(UsedRange const& x, UsedRange const& y)
{
    SLANG_ASSERT(x.begin <= x.end);
    SLANG_ASSERT(y.begin <= y.end);

    // If they don't overlap, then one must be earlier than the other,
    // and that one must therefore *end* before the other *begins*

    if (x.end <= y.begin) return false;
    if (y.end <= x.begin) return false;

    // Otherwise they must overlap
    return true;
}


struct UsedRanges
{
    List<UsedRange> ranges;

    // Add a range to the set, either by extending
    // an existing range, or by adding a new one...
    //
    // If we find that the new range overlaps with
    // an existing range for a *different* parameter
    // then we return that parameter so that the
    // caller can issue an error.
    ParameterInfo* Add(UsedRange const& range)
    {
        ParameterInfo* newParam = range.parameter;
        ParameterInfo* existingParam = nullptr;
        for (auto& rr : ranges)
        {
            if (rangesOverlap(rr, range)
                && rr.parameter
                && rr.parameter != newParam)
            {
                // there was an overlap!
                existingParam = rr.parameter;
            }
        }

        for (auto& rr : ranges)
        {
            if (rr.begin == range.end)
            {
                rr.begin = range.begin;
                return existingParam;
            }
            else if (rr.end == range.begin)
            {
                rr.end = range.end;
                return existingParam;
            }
        }
        ranges.Add(range);
        ranges.Sort();
        return existingParam;
    }

    ParameterInfo* Add(ParameterInfo* param, UInt begin, UInt end)
    {
        UsedRange range;
        range.parameter = param;
        range.begin = begin;
        range.end = end;
        return Add(range);
    }


    // Try to find space for `count` entries
    UInt Allocate(ParameterInfo* param, UInt count)
    {
        UInt begin = 0;

        UInt rangeCount = ranges.Count();
        for (UInt rr = 0; rr < rangeCount; ++rr)
        {
            // try to fit in before this range...

            UInt end = ranges[rr].begin;

            // If there is enough space...
            if (end >= begin + count)
            {
                // ... then claim it and be done
                Add(param, begin, begin + count);
                return begin;
            }

            // ... otherwise, we need to look at the
            // space between this range and the next
            begin = ranges[rr].end;
        }

        // We've run out of ranges to check, so we
        // can safely go after the last one!
        Add(param, begin, begin + count);
        return begin;
    }
};

struct ParameterBindingInfo
{
    size_t              space;
    size_t              index;
    size_t              count;
};

enum
{
    kLayoutResourceKindCount = SLANG_PARAMETER_CATEGORY_COUNT,
};

struct UsedRangeSet : RefObject
{
    // Information on what ranges of "registers" have already
    // been claimed, for each resource type
    UsedRanges usedResourceRanges[kLayoutResourceKindCount];
};

// Information on a single parameter
struct ParameterInfo : RefObject
{
    // Layout info for the concrete variables that will make up this parameter
    List<RefPtr<VarLayout>> varLayouts;

    ParameterBindingInfo    bindingInfo[kLayoutResourceKindCount];

    // The next parameter that has the same name...
    ParameterInfo* nextOfSameName;

    // The translation unit this parameter is specific to, if any
    TranslationUnitRequest* translationUnit = nullptr;

    ParameterInfo()
    {
        // Make sure we aren't claiming any resources yet
        for( int ii = 0; ii < kLayoutResourceKindCount; ++ii )
        {
            bindingInfo[ii].count = 0;
        }
    }
};

struct EntryPointParameterBindingContext
{
    // What ranges of resources bindings are already claimed for this translation unit
    UsedRangeSet usedRangeSet;
};

// State that is shared during parameter binding,
// across all translation units
struct SharedParameterBindingContext
{
    // The base compile request
    CompileRequest* compileRequest;

    // The target request that is triggering layout
    //
    // TODO: We should eventually strip this down to
    // just the subset of fields on the target that
    // can influence layout decisions.
    TargetRequest*  targetRequest;

    LayoutRulesFamilyImpl* defaultLayoutRules;

    // All shader parameters we've discovered so far, and started to lay out...
    List<RefPtr<ParameterInfo>> parameters;

    // The program layout we are trying to construct
    RefPtr<ProgramLayout> programLayout;

    // What ranges of resources bindings are already claimed at the global scope?
    // We store one of these for each declared binding space/set.
    //
    Dictionary<UInt, RefPtr<UsedRangeSet>> globalSpaceUsedRangeSets;

    // What ranges of resource bindings are claimed for particular translation unit?
    // This is only used for varying input/output.
    //
    Dictionary<TranslationUnitRequest*, RefPtr<UsedRangeSet>> translationUnitUsedRangeSets;

    // Which register spaces have been claimed so far?
    UsedRanges usedSpaces;

    TargetRequest* getTargetRequest() { return targetRequest; }
};

static DiagnosticSink* getSink(SharedParameterBindingContext* shared)
{
    return &shared->compileRequest->mSink;
}

// State that might be specific to a single translation unit
// or event to an entry point.
struct ParameterBindingContext
{
    // The translation unit we are processing right now
    TranslationUnitRequest* translationUnit;

    // All the shared state needs to be available
    SharedParameterBindingContext* shared;

    // The type layout context to use when computing
    // the resource usage of shader parameters.
    TypeLayoutContext layoutContext;

    // A dictionary to accellerate looking up parameters by name
    Dictionary<Name*, ParameterInfo*> mapNameToParameterInfo;

    // What stage (if any) are we compiling for?
    Stage stage;

    // The source language we are trying to use
    SourceLanguage sourceLanguage;

    TargetRequest* getTargetRequest() { return shared->getTargetRequest(); }
    LayoutRulesFamilyImpl* getRulesFamily() { return layoutContext.getRulesFamily(); }
};

static DiagnosticSink* getSink(ParameterBindingContext* context)
{
    return getSink(context->shared);
}


struct LayoutSemanticInfo
{
    LayoutResourceKind  kind; // the register kind
    UInt                space;
    UInt                index;

    // TODO: need to deal with component-granularity binding...
};

LayoutSemanticInfo ExtractLayoutSemanticInfo(
    ParameterBindingContext*    /*context*/,
    HLSLLayoutSemantic*         semantic)
{
    LayoutSemanticInfo info;
    info.space = 0;
    info.index = 0;
    info.kind = LayoutResourceKind::None;

    auto registerName = semantic->registerName.Content;
    if (registerName.Length() == 0)
        return info;

    LayoutResourceKind kind = LayoutResourceKind::None;
    switch (registerName[0])
    {
    case 'b':
        kind = LayoutResourceKind::ConstantBuffer;
        break;

    case 't':
        kind = LayoutResourceKind::ShaderResource;
        break;

    case 'u':
        kind = LayoutResourceKind::UnorderedAccess;
        break;

    case 's':
        kind = LayoutResourceKind::SamplerState;
        break;

    default:
        // TODO: issue an error here!
        return info;
    }

    // TODO: need to parse and handle `space` binding
    int space = 0;

    UInt index = 0;
    for (UInt ii = 1; ii < registerName.Length(); ++ii)
    {
        int c = registerName[ii];
        if (c >= '0' && c <= '9')
        {
            index = index * 10 + (c - '0');
        }
        else
        {
            // TODO: issue an error here!
            return info;
        }
    }

    // TODO: handle component mask part of things...

    info.kind = kind;
    info.index = (int) index;
    info.space = space;
    return info;
}

// This function is supposed to determine if two global shader
// parameter declarations represent the same logical parameter
// (so that they should get the exact same binding(s) allocated).
//
static bool doesParameterMatch(
    ParameterBindingContext*,
    RefPtr<VarLayout>   varLayout,
    ParameterInfo*)
{
    // Any "varying" parameter should automatically be excluded
    //
    // Note that we use the `typeLayout` field rather than
    // looking at resource information on the variable directly,
    // because this may be called when binding hasn't been performed.
    for (auto rr : varLayout->typeLayout->resourceInfos)
    {
        switch (rr.kind)
        {
        case LayoutResourceKind::VertexInput:
        case LayoutResourceKind::FragmentOutput:
            return false;

        default:
            break;
        }
    }

    // TODO: this is where we should apply a more detailed
    // matching process, to check that the existing
    // declarations conform to the same basic layout.

    return true;
}

//

// Given a GLSL `layout` modifier, we need to be able to check for
// a particular sub-argument and extract its value if present.
template<typename T>
static bool findLayoutArg(
    RefPtr<ModifiableSyntaxNode>    syntax,
    UInt*                           outVal)
{
    for( auto modifier : syntax->GetModifiersOfType<T>() )
    {
        if( modifier )
        {
            *outVal = (UInt) strtoull(modifier->valToken.Content.Buffer(), nullptr, 10);
            return true;
        }
    }
    return false;
}

template<typename T>
static bool findLayoutArg(
    DeclRef<Decl>   declRef,
    UInt*           outVal)
{
    return findLayoutArg<T>(declRef.getDecl(), outVal);
}

//

static Name* getReflectionName(VarDeclBase* varDecl)
{
    if (auto reflectionNameModifier = varDecl->FindModifier<ParameterGroupReflectionName>())
        return reflectionNameModifier->nameAndLoc.name;

    return varDecl->getName();
}

static bool isGLSLBuiltinName(VarDeclBase* varDecl)
{
    return getText(getReflectionName(varDecl)).StartsWith("gl_");
}

RefPtr<Type> tryGetEffectiveTypeForGLSLVaryingInput(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl)
{
    if (isGLSLBuiltinName(varDecl))
        return nullptr;

    auto type = varDecl->getType();
    if( varDecl->HasModifier<InModifier>() || type->As<GLSLInputParameterGroupType>())
    {
        // Special case to handle "arrayed" shader inputs, as used
        // for Geometry and Hull input
        switch( context->stage )
        {
        case Stage::Geometry:
        case Stage::Hull:
        case Stage::Domain:
            // Tessellation `patch` variables should stay as written
            if( !varDecl->HasModifier<GLSLPatchModifier>() )
            {
                // Unwrap array type, if prsent
                if( auto arrayType = type->As<ArrayExpressionType>() )
                {
                    type = arrayType->baseType.Ptr();
                }
            }
            break;

        default:
            break;
        }

        return type;
    }

    return nullptr;
}

RefPtr<Type> tryGetEffectiveTypeForGLSLVaryingOutput(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl)
{
    if (isGLSLBuiltinName(varDecl))
        return nullptr;

    auto type = varDecl->getType();
    if( varDecl->HasModifier<OutModifier>() || type->As<GLSLOutputParameterGroupType>())
    {
        // Special case to handle "arrayed" shader outputs, as used
        // for Hull Shader output
        //
        // Note(tfoley): there is unfortunate code duplication
        // with the `in` case above.
        switch( context->stage )
        {
        case Stage::Hull:
            // Tessellation `patch` variables should stay as written
            if( !varDecl->HasModifier<GLSLPatchModifier>() )
            {
                // Unwrap array type, if prsent
                if( auto arrayType = type->As<ArrayExpressionType>() )
                {
                    type = arrayType->baseType.Ptr();
                }
            }
            break;

        default:
            break;
        }

        return type;
    }

    return nullptr;
}

RefPtr<TypeLayout>
getTypeLayoutForGlobalShaderParameter_GLSL(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl)
{
    auto layoutContext = context->layoutContext;
    auto rules = layoutContext.getRulesFamily();
    auto type = varDecl->getType();

    // A GLSL shader parameter will be marked with
    // a qualifier to match the boundary it uses
    //
    // In the case of a parameter block, we will have
    // consumed this qualifier as part of parsing,
    // so that it won't be present on the declaration
    // any more. As such we also inspect the type
    // of the variable.

    // We want to check for a constant-buffer type with a `push_constant` layout
    // qualifier before we move on to anything else.
    if( varDecl->HasModifier<GLSLPushConstantLayoutModifier>() && type->As<ConstantBufferType>() )
    {
        return CreateTypeLayout(
            layoutContext.with(rules->getPushConstantBufferRules()),
            type);
    }

    // TODO(tfoley): We have multiple variations of
    // the `uniform` modifier right now, and that
    // needs to get fixed...
    if( varDecl->HasModifier<HLSLUniformModifier>() || type->As<ConstantBufferType>() )
    {
        return CreateTypeLayout(
            layoutContext.with(rules->getConstantBufferRules()),
            type);
    }

    if( varDecl->HasModifier<GLSLBufferModifier>() || type->As<GLSLShaderStorageBufferType>() )
    {
        return CreateTypeLayout(
            layoutContext.with(rules->getShaderStorageBufferRules()),
            type);
    }

    if (auto effectiveVaryingInputType = tryGetEffectiveTypeForGLSLVaryingInput(context, varDecl))
    {
        // We expect to handle these elsewhere
        SLANG_DIAGNOSE_UNEXPECTED(getSink(context), varDecl, "GLSL varying input");
        return CreateTypeLayout(
            layoutContext.with(rules->getVaryingInputRules()),
            effectiveVaryingInputType);
    }

    if (auto effectiveVaryingOutputType = tryGetEffectiveTypeForGLSLVaryingOutput(context, varDecl))
    {
        // We expect to handle these elsewhere
        SLANG_DIAGNOSE_UNEXPECTED(getSink(context), varDecl, "GLSL varying output");
        return CreateTypeLayout(
            layoutContext.with(rules->getVaryingOutputRules()),
            effectiveVaryingOutputType);
    }

    // A `const` global with a `layout(constant_id = ...)` modifier
    // is a declaration of a specialization constant.
    if( varDecl->HasModifier<GLSLConstantIDLayoutModifier>() )
    {
        return CreateTypeLayout(
            layoutContext.with(rules->getSpecializationConstantRules()),
            type);
    }

    // GLSL says that an "ordinary" global  variable
    // is just a (thread local) global and not a
    // parameter
    return nullptr;
}

RefPtr<TypeLayout>
getTypeLayoutForGlobalShaderParameter_HLSL(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl)
{
    auto layoutContext = context->layoutContext;
    auto rules = layoutContext.getRulesFamily();
    auto type = varDecl->getType();

    // HLSL `static` modifier indicates "thread local"
    if(varDecl->HasModifier<HLSLStaticModifier>())
        return nullptr;

    // HLSL `groupshared` modifier indicates "thread-group local"
    if(varDecl->HasModifier<HLSLGroupSharedModifier>())
        return nullptr;

    // TODO(tfoley): there may be other cases that we need to handle here

    // An "ordinary" global variable is implicitly a uniform
    // shader parameter.
    return CreateTypeLayout(
        layoutContext.with(rules->getConstantBufferRules()),
        type);
}

// Determine how to lay out a global variable that might be
// a shader parameter.
// Returns `nullptr` if the declaration does not represent
// a shader parameter.

RefPtr<TypeLayout>
getTypeLayoutForGlobalShaderParameter(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl)
{
    switch( context->sourceLanguage )
    {
    case SourceLanguage::Slang:
    case SourceLanguage::HLSL:
        return getTypeLayoutForGlobalShaderParameter_HLSL(context, varDecl);

    case SourceLanguage::GLSL:
        return getTypeLayoutForGlobalShaderParameter_GLSL(context, varDecl);

    default:
        SLANG_UNEXPECTED("unhandled source language");
        UNREACHABLE_RETURN(nullptr);
    }
}


//

enum EntryPointParameterDirection
{
    kEntryPointParameterDirection_Input  = 0x1,
    kEntryPointParameterDirection_Output = 0x2,
};
typedef unsigned int EntryPointParameterDirectionMask;

struct EntryPointParameterState
{
    String*                             optSemanticName = nullptr;
    int*                                ioSemanticIndex = nullptr;
    EntryPointParameterDirectionMask    directionMask;
    int                                 semanticSlotCount;
};


static RefPtr<TypeLayout> processEntryPointParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout);

static void collectGlobalScopeGLSLVaryingParameter(
    ParameterBindingContext*        context,
    RefPtr<VarDeclBase>             varDecl,
    RefPtr<Type>          effectiveType,
    EntryPointParameterDirection    direction)
{
    int defaultSemanticIndex = 0;

    EntryPointParameterState state;
    state.directionMask = direction;
    state.ioSemanticIndex = &defaultSemanticIndex;

    RefPtr<VarLayout> varLayout = new VarLayout();
    varLayout->varDecl = makeDeclRef(varDecl.Ptr());

    varLayout->typeLayout = processEntryPointParameter(
        context,
        effectiveType,
        state,
        varLayout);

    // Now add it to our list of reflection parameters, so
    // that it can get a location assigned later...

    ParameterInfo* parameterInfo = new ParameterInfo();
    parameterInfo->translationUnit = context->translationUnit;
    context->shared->parameters.Add(parameterInfo);
    parameterInfo->varLayouts.Add(varLayout);
}

// Collect a single declaration into our set of parameters
static void collectGlobalGenericParameter(
    ParameterBindingContext*    context,
    RefPtr<GlobalGenericParamDecl>         paramDecl)
{
    RefPtr<GenericParamLayout> layout = new GenericParamLayout();
    layout->decl = paramDecl;
    layout->index = (int)context->shared->programLayout->globalGenericParams.Count();
    context->shared->programLayout->globalGenericParams.Add(layout);
}

// Collect a single declaration into our set of parameters
static void collectGlobalScopeParameter(
    ParameterBindingContext*    context,
    RefPtr<VarDeclBase>         varDecl)
{
    // HACK: We need to intercept GLSL varying `in` and `out` here, way earlier
    // in the process, so that we can avoid all kinds of nastiness that would
    // otherwise be applied to them.
    if (context->sourceLanguage == SourceLanguage::GLSL)
    {
        if (auto effectiveVaryingInputType = tryGetEffectiveTypeForGLSLVaryingInput(context, varDecl))
        {
            collectGlobalScopeGLSLVaryingParameter(context, varDecl, effectiveVaryingInputType, kEntryPointParameterDirection_Input);
            return;
        }

        if (auto effectiveVaryingOutputType = tryGetEffectiveTypeForGLSLVaryingOutput(context, varDecl))
        {
            collectGlobalScopeGLSLVaryingParameter(context, varDecl, effectiveVaryingOutputType, kEntryPointParameterDirection_Output);
            return;
        }
    }


    // We use a single operation to both check whether the
    // variable represents a shader parameter, and to compute
    // the layout for that parameter's type.
    auto typeLayout = getTypeLayoutForGlobalShaderParameter(
        context,
        varDecl.Ptr());

    // If we did not find appropriate layout rules, then it
    // must mean that this global variable is *not* a shader
    // parameter.
    if(!typeLayout)
        return;

    // Now create a variable layout that we can use
    RefPtr<VarLayout> varLayout = new VarLayout();
    varLayout->typeLayout = typeLayout;
    varLayout->varDecl = DeclRef<Decl>(varDecl.Ptr(), nullptr).As<VarDeclBase>();

    // This declaration may represent the same logical parameter
    // as a declaration that came from a different translation unit.
    // If that is the case, we want to re-use the same `VarLayout`
    // across both parameters.
    //
    // First we look for an existing entry matching the name
    // of this parameter:
    auto parameterName = getReflectionName(varDecl);
    ParameterInfo* parameterInfo = nullptr;
    if( context->mapNameToParameterInfo.TryGetValue(parameterName, parameterInfo) )
    {
        // If the parameters have the same name, but don't "match" according to some reasonable rules,
        // then we need to bail out.
        if( !doesParameterMatch(context, varLayout, parameterInfo) )
        {
            parameterInfo = nullptr;
        }
    }

    // If we didn't find a matching parameter, then we need to create one here
    if( !parameterInfo )
    {
        parameterInfo = new ParameterInfo();
        context->shared->parameters.Add(parameterInfo);
        context->mapNameToParameterInfo.AddIfNotExists(parameterName, parameterInfo);
    }
    else
    {
        varLayout->flags |= VarLayoutFlag::IsRedeclaration;
    }

    // Add this variable declaration to the list of declarations for the parameter
    parameterInfo->varLayouts.Add(varLayout);
}

static RefPtr<UsedRangeSet> findUsedRangeSetForSpace(
    ParameterBindingContext*    context,
    UInt                        space)
{
    RefPtr<UsedRangeSet> usedRangeSet;
    if (context->shared->globalSpaceUsedRangeSets.TryGetValue(space, usedRangeSet))
        return usedRangeSet;

    usedRangeSet = new UsedRangeSet();
    context->shared->globalSpaceUsedRangeSets.Add(space, usedRangeSet);
    return usedRangeSet;
}

// Record that a particular register space (or set, in the GLSL case)
// has been used in at least one binding, and so it should not
// be used by auto-generated bindings that need to claim entire
// spaces.
static void markSpaceUsed(
    ParameterBindingContext*    context,
    UInt                        space)
{
    context->shared->usedSpaces.Add(nullptr, space, space+1);
}

static UInt allocateUnusedSpaces(
    ParameterBindingContext*    context,
    UInt                        count)
{
    return context->shared->usedSpaces.Allocate(nullptr, count);
}

static RefPtr<UsedRangeSet> findUsedRangeSetForTranslationUnit(
    ParameterBindingContext*    context,
    TranslationUnitRequest*     translationUnit)
{
    if (!translationUnit)
        return findUsedRangeSetForSpace(context, 0);

    RefPtr<UsedRangeSet> usedRangeSet;
    if (context->shared->translationUnitUsedRangeSets.TryGetValue(translationUnit, usedRangeSet))
        return usedRangeSet;

    usedRangeSet = new UsedRangeSet();
    context->shared->translationUnitUsedRangeSets.Add(translationUnit, usedRangeSet);
    return usedRangeSet;
}

static void addExplicitParameterBinding(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    VarDeclBase*                varDecl,
    LayoutSemanticInfo const&   semanticInfo,
    UInt                        count,
    RefPtr<UsedRangeSet>        usedRangeSet = nullptr)
{
    auto kind = semanticInfo.kind;

    auto& bindingInfo = parameterInfo->bindingInfo[(int)kind];
    if( bindingInfo.count != 0 )
    {
        // We already have a binding here, so we want to
        // confirm that it matches the new one that is
        // incoming...
        if( bindingInfo.count != count
            || bindingInfo.index != semanticInfo.index
            || bindingInfo.space != semanticInfo.space )
        {
            getSink(context)->diagnose(varDecl, Diagnostics::conflictingExplicitBindingsForParameter, getReflectionName(varDecl));

            auto firstVarDecl = parameterInfo->varLayouts[0]->varDecl.getDecl();
            if( firstVarDecl != varDecl )
            {
                getSink(context)->diagnose(firstVarDecl, Diagnostics::seeOtherDeclarationOf, getReflectionName(firstVarDecl));
            }
        }

        // TODO(tfoley): `register` semantics can technically be
        // profile-specific (not sure if anybody uses that)...
    }
    else
    {
        bindingInfo.count = count;
        bindingInfo.index = semanticInfo.index;
        bindingInfo.space = semanticInfo.space;

        if (!usedRangeSet)
        {
            usedRangeSet = findUsedRangeSetForSpace(context, semanticInfo.space);

            // Record that the particular binding space was
            // used by an explicit binding, so that we don't
            // claim it for auto-generated bindings that
            // need to grab a full space
            markSpaceUsed(context, semanticInfo.space);
        }
        auto overlappedParameterInfo = usedRangeSet->usedResourceRanges[(int)semanticInfo.kind].Add(
            parameterInfo,
            semanticInfo.index,
            semanticInfo.index + count);

        if (overlappedParameterInfo)
        {
            auto paramA = parameterInfo->varLayouts[0]->varDecl.getDecl();
            auto paramB = overlappedParameterInfo->varLayouts[0]->varDecl.getDecl();

            getSink(context)->diagnose(paramA, Diagnostics::parameterBindingsOverlap,
                getReflectionName(paramA),
                getReflectionName(paramB));

            getSink(context)->diagnose(paramB, Diagnostics::seeDeclarationOf, getReflectionName(paramB));
        }
    }
}

static void addExplicitParameterBindings_HLSL(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    RefPtr<VarLayout>           varLayout)
{
    auto typeLayout = varLayout->typeLayout;
    auto varDecl = varLayout->varDecl;

    // If the declaration has explicit binding modifiers, then
    // here is where we want to extract and apply them...

    // Look for HLSL `register` or `packoffset` semantics.
    for (auto semantic : varDecl.getDecl()->GetModifiersOfType<HLSLLayoutSemantic>())
    {
        // Need to extract the information encoded in the semantic
        LayoutSemanticInfo semanticInfo = ExtractLayoutSemanticInfo(context, semantic);
        auto kind = semanticInfo.kind;
        if (kind == LayoutResourceKind::None)
            continue;

        // TODO: need to special-case when this is a `c` register binding...

        // Find the appropriate resource-binding information
        // inside the type, to see if we even use any resources
        // of the given kind.

        auto typeRes = typeLayout->FindResourceInfo(kind);
        int count = 0;
        if (typeRes)
        {
            count = (int) typeRes->count;
        }
        else
        {
            // TODO: warning here!
        }

        addExplicitParameterBinding(context, parameterInfo, varDecl, semanticInfo, count);
    }
}

static void addExplicitParameterBindings_GLSL(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    RefPtr<VarLayout>           varLayout)
{
    auto typeLayout = varLayout->typeLayout;
    auto varDecl = varLayout->varDecl;

    // The catch in GLSL is that the expected resource type
    // is implied by the parameter declaration itself, and
    // the `layout` modifier is only allowed to adjust
    // the index/offset/etc.
    //

    // We also may need to store explicit binding info in a different place,
    // in the case of varying input/output, since we don't want to collect
    // things globally;
    RefPtr<UsedRangeSet> usedRangeSet;

    TypeLayout::ResourceInfo* resInfo = nullptr;
    LayoutSemanticInfo semanticInfo;
    semanticInfo.index = 0;
    semanticInfo.space = 0;
    if( (resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::DescriptorTableSlot)) != nullptr )
    {
        // Try to find `binding` and `set`
        if(!findLayoutArg<GLSLBindingLayoutModifier>(varDecl, &semanticInfo.index))
            return;

        findLayoutArg<GLSLSetLayoutModifier>(varDecl, &semanticInfo.space);
    }
    else if( (resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::VertexInput)) != nullptr )
    {
        // Try to find `location` binding
        if(!findLayoutArg<GLSLLocationLayoutModifier>(varDecl, &semanticInfo.index))
            return;

        usedRangeSet = findUsedRangeSetForTranslationUnit(context, parameterInfo->translationUnit);
    }
    else if( (resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::FragmentOutput)) != nullptr )
    {
        // Try to find `location` binding
        if(!findLayoutArg<GLSLLocationLayoutModifier>(varDecl, &semanticInfo.index))
            return;

        usedRangeSet = findUsedRangeSetForTranslationUnit(context, parameterInfo->translationUnit);
    }
    else if( (resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::SpecializationConstant)) != nullptr )
    {
        // Try to find `constant_id` binding
        if(!findLayoutArg<GLSLConstantIDLayoutModifier>(varDecl, &semanticInfo.index))
            return;
    }

    // If we didn't find any matches, then bail
    if(!resInfo)
        return;

    auto kind = resInfo->kind;
    auto count = resInfo->count;
    semanticInfo.kind = kind;

    addExplicitParameterBinding(context, parameterInfo, varDecl, semanticInfo, int(count), usedRangeSet);
}

// Given a single parameter, collect whatever information we have on
// how it has been explicitly bound, which may come from multiple declarations
void generateParameterBindings(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo)
{
    // There must be at least one declaration for the parameter.
    SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);

    // Iterate over all declarations looking for explicit binding information.
    for( auto& varLayout : parameterInfo->varLayouts )
    {
        // Handle HLSL `register` and `packoffset` modifiers
        addExplicitParameterBindings_HLSL(context, parameterInfo, varLayout);


        // Handle GLSL `layout` modifiers
        addExplicitParameterBindings_GLSL(context, parameterInfo, varLayout);
    }
}

// Generate the binding information for a shader parameter.
static void completeBindingsForParameter(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo)
{
    // For any resource kind used by the parameter
    // we need to update its layout information
    // to include a binding for that resource kind.
    //
    // We will use the first declaration of the parameter as
    // a stand-in for all the declarations, so it is important
    // that earlier code has validated that the declarations
    // "match".

    SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);
    auto firstVarLayout = parameterInfo->varLayouts.First();
    auto firstTypeLayout = firstVarLayout->typeLayout;

    for(auto typeRes : firstTypeLayout->resourceInfos)
    {
        // Did we already apply some explicit binding information
        // for this resource kind?
        auto kind = typeRes.kind;
        auto& bindingInfo = parameterInfo->bindingInfo[(int)kind];
        if( bindingInfo.count != 0 )
        {
            // If things have already been bound, our work is done.
            continue;
        }

        auto count = typeRes.count;

        // We need to special-case the scenario where
        // a parameter wants to claim an entire register
        // space to itself (for a parameter block), since
        // that can't be handled like other resources.
        if (kind == LayoutResourceKind::RegisterSpace)
        {
            // We need to snag a register space of our own.

            UInt space = allocateUnusedSpaces(context, count);

            bindingInfo.count = count;
            bindingInfo.index = space;

            // TODO: what should we store as the "space" for
            // an allocation of register spaces? Either zero
            // or `space` makes sense, but it isn't clear
            // which is a better choice.
            bindingInfo.space = 0;

            continue;
        }
        else if (kind == LayoutResourceKind::GenericResource)
        {
            bindingInfo.space = 0;
            bindingInfo.count = 0;
            bindingInfo.index = 0;
            continue;
        }

        // For now we only auto-generate bindings in space zero
        //
        // TODO: we may want to support searching for a space with
        // capacity for our resource, just in case somebody has
        // claimed the entire range...
        UInt space = 0;

        RefPtr<UsedRangeSet> usedRangeSet;
        switch (kind)
        {
        default:
            usedRangeSet = findUsedRangeSetForSpace(context, space);
            break;

        case LayoutResourceKind::VertexInput:
        case LayoutResourceKind::FragmentOutput:
            usedRangeSet = findUsedRangeSetForTranslationUnit(context, parameterInfo->translationUnit);
            break;
        }

        bindingInfo.count = count;
        bindingInfo.index = usedRangeSet->usedResourceRanges[(int)kind].Allocate(parameterInfo, (int) count);

        bindingInfo.space = space;
    }

    if (firstTypeLayout->FindResourceInfo(LayoutResourceKind::GenericResource))
    {

    }

    // At this point we should have explicit binding locations chosen for
    // all the relevant resource kinds, so we can apply these to the
    // declarations:

    for(auto& varLayout : parameterInfo->varLayouts)
    {
        for(auto k = 0; k < kLayoutResourceKindCount; ++k)
        {
            auto kind = LayoutResourceKind(k);
            auto& bindingInfo = parameterInfo->bindingInfo[k];

            // skip resources we aren't consuming
            if(bindingInfo.count == 0)
                continue;

            // Add a record to the variable layout
            auto varRes = varLayout->AddResourceInfo(kind);
            varRes->space = (int) bindingInfo.space;
            varRes->index = (int) bindingInfo.index;
        }
    }
}

static void collectGlobalScopeParameters(
    ParameterBindingContext*    context,
    ModuleDecl*          program)
{
    // First enumerate parameters at global scope
    // We collect two things here:
    // 1. A shader parameter, which is always a variable
    // 2. A global entry-point generic parameter type (`__generic_param`),
    //    which is a GlobalGenericParamDecl
    // We collect global generic type parameters in the first pass,
    // So we can fill in the correct index into ordinary type layouts 
    // for generic types in the second pass.
    for (auto decl : program->Members)
    {
        if (auto genParamDecl = decl.As<GlobalGenericParamDecl>())
            collectGlobalGenericParameter(context, genParamDecl);
    }
    for (auto decl : program->Members)
    {
        if (auto varDecl = decl.As<VarDeclBase>())
            collectGlobalScopeParameter(context, varDecl);
    }

    // Next, we need to enumerate the parameters of
    // each entry point (which requires knowing what the
    // entry points *are*)

    // TODO(tfoley): Entry point functions should be identified
    // by looking for a generated modifier that is attached
    // to global-scope function declarations.
}

struct SimpleSemanticInfo
{
    String  name;
    int     index;
};

SimpleSemanticInfo decomposeSimpleSemantic(
    HLSLSimpleSemantic* semantic)
{
    auto composedName = semantic->name.Content;

    // look for a trailing sequence of decimal digits
    // at the end of the composed name
    UInt length = composedName.Length();
    UInt indexLoc = length;
    while( indexLoc > 0 )
    {
        auto c = composedName[indexLoc-1];
        if( c >= '0' && c <= '9' )
        {
            indexLoc--;
            continue;
        }
        else
        {
            break;
        }
    }

    SimpleSemanticInfo info;

    // 
    if( indexLoc == length )
    {
        // No index suffix
        info.name = composedName;
        info.index = 0;
    }
    else
    {
        // The name is everything before the digits
        info.name = composedName.SubString(0, indexLoc);
        info.index = strtol(composedName.SubString(indexLoc, length - indexLoc).begin(), nullptr, 10);
    }
    return info;
}

static RefPtr<TypeLayout> processSimpleEntryPointParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>          type,
    EntryPointParameterState const& inState,
    RefPtr<VarLayout>               varLayout,
    int                             semanticSlotCount = 1)
{
    EntryPointParameterState state = inState;
    state.semanticSlotCount = semanticSlotCount;

    auto optSemanticName    =  state.optSemanticName;
    auto semanticIndex      = *state.ioSemanticIndex;

    String semanticName = optSemanticName ? *optSemanticName : "";
    String sn = semanticName.ToLower();

    RefPtr<TypeLayout> typeLayout =  new TypeLayout();
    if (sn.StartsWith("sv_")
        || sn.StartsWith("nv_"))
    {
        // System-value semantic.

        if (state.directionMask & kEntryPointParameterDirection_Output)
        {
            // Note: I'm just doing something expedient here and detecting `SV_Target`
            // outputs and claiming the appropriate register range right away.
            //
            // TODO: we should really be building up some representation of all of this,
            // once we've gone to the trouble of looking it all up...
            if( sn == "sv_target" )
            {
                // TODO: construct a `ParameterInfo` we can use here so that
                // overlapped layout errors get reported nicely.

                auto usedResourceSet = findUsedRangeSetForSpace(context, 0);
                usedResourceSet->usedResourceRanges[int(LayoutResourceKind::UnorderedAccess)].Add(nullptr, semanticIndex, semanticIndex + semanticSlotCount);


                // We also need to track this as an ordinary varying output from the stage,
                // since that is how GLSL will want to see it.
                auto rules = context->getRulesFamily()->getVaryingOutputRules();
                SimpleLayoutInfo layout = GetLayout(
                    context->layoutContext.with(rules),
                    type);
                typeLayout->addResourceUsage(layout.kind, layout.size);
            }
        }

        // Remember the system-value semantic so that we can query it later
        if (varLayout)
        {
            varLayout->systemValueSemantic = semanticName;
            varLayout->systemValueSemanticIndex = semanticIndex;
        }

        // TODO: add some kind of usage information for system input/output
    }
    else
    {
        // user-defined semantic

        if (state.directionMask & kEntryPointParameterDirection_Input)
        {
            auto rules = context->getRulesFamily()->getVaryingInputRules();
            SimpleLayoutInfo layout = GetLayout(
                context->layoutContext.with(rules),
                type);
            typeLayout->addResourceUsage(layout.kind, layout.size);
        }

        if (state.directionMask & kEntryPointParameterDirection_Output)
        {
            auto rules = context->getRulesFamily()->getVaryingOutputRules();
            SimpleLayoutInfo layout = GetLayout(
                context->layoutContext.with(rules),
                type);
            typeLayout->addResourceUsage(layout.kind, layout.size);
        }
    }

    *state.ioSemanticIndex += state.semanticSlotCount;
    typeLayout->type = type;

    return typeLayout;
}

static RefPtr<TypeLayout> processEntryPointParameterWithPossibleSemantic(
    ParameterBindingContext*        context,
    Decl*                           declForSemantic,
    RefPtr<Type>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout)
{
    // If there is no explicit semantic already in effect, *and* we find an explicit
    // semantic on the associated declaration, then we'll use it.
    if( !state.optSemanticName )
    {
        if( auto semantic = declForSemantic->FindModifier<HLSLSimpleSemantic>() )
        {
            auto semanticInfo = decomposeSimpleSemantic(semantic);
            int semanticIndex = semanticInfo.index;

            EntryPointParameterState subState = state;
            subState.optSemanticName = &semanticInfo.name;
            subState.ioSemanticIndex = &semanticIndex;

            return processEntryPointParameter(context, type, subState, varLayout);
        }
    }

    // Default case: either there was an explicit semantic in effect already,
    // *or* we couldn't find an explicit semantic to apply on the given
    // declaration, so we will just recursive with whatever we have at
    // the moment.
    return processEntryPointParameter(context, type, state, varLayout);
}


static RefPtr<TypeLayout> processEntryPointParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout)
{
    // If there is an available semantic name and index,
    // then we should apply it to this parameter unconditionally
    // (that is, not just if it is a leaf parameter).
    auto optSemanticName    =  state.optSemanticName;
    if (optSemanticName && varLayout)
    {
        // Always store semantics in upper-case for
        // reflection information, since they are
        // supposed to be case-insensitive and
        // upper-case is the dominant convention.
        String semanticName = *optSemanticName;
        String sn = semanticName.ToUpper();

        auto semanticIndex      = *state.ioSemanticIndex;

        varLayout->semanticName = sn;
        varLayout->semanticIndex = semanticIndex;
        varLayout->flags |= VarLayoutFlag::HasSemantic;
    }


    // Scalar and vector types are treated as outputs directly
    if(auto basicType = type->As<BasicExpressionType>())
    {
        return processSimpleEntryPointParameter(context, basicType, state, varLayout);
    }
    else if(auto vectorType = type->As<VectorExpressionType>())
    {
        return processSimpleEntryPointParameter(context, vectorType, state, varLayout);
    }
    // A matrix is processed as if it was an array of rows
    else if( auto matrixType = type->As<MatrixExpressionType>() )
    {
        auto rowCount = GetIntVal(matrixType->getRowCount());
        return processSimpleEntryPointParameter(context, matrixType, state, varLayout, (int) rowCount);
    }
    else if( auto arrayType = type->As<ArrayExpressionType>() )
    {
        // Note: Bad Things will happen if we have an array input
        // without a semantic already being enforced.
        
        auto elementCount = (UInt) GetIntVal(arrayType->ArrayLength);

        // We use the first element to derive the layout for the element type
        auto elementTypeLayout = processEntryPointParameter(context, arrayType->baseType, state, varLayout);

        // We still walk over subsequent elements to make sure they consume resources
        // as needed
        for( UInt ii = 1; ii < elementCount; ++ii )
        {
            processEntryPointParameter(context, arrayType->baseType, state, nullptr);
        }

        RefPtr<ArrayTypeLayout> arrayTypeLayout = new ArrayTypeLayout();
        arrayTypeLayout->elementTypeLayout = elementTypeLayout;
        arrayTypeLayout->type = arrayType;

        for (auto rr : elementTypeLayout->resourceInfos)
        {
            arrayTypeLayout->findOrAddResourceInfo(rr.kind)->count = rr.count * elementCount;
        }

        return arrayTypeLayout;
    }
    // Ignore a bunch of types that don't make sense here...
    else if (auto textureType = type->As<TextureType>()) { return nullptr;  }
    else if(auto samplerStateType = type->As<SamplerStateType>()) { return nullptr;  }
    else if(auto constantBufferType = type->As<ConstantBufferType>()) { return nullptr;  }
    // Catch declaration-reference types late in the sequence, since
    // otherwise they will include all of the above cases...
    else if( auto declRefType = type->As<DeclRefType>() )
    {
        auto declRef = declRefType->declRef;

        if (auto structDeclRef = declRef.As<StructDecl>())
        {
            RefPtr<StructTypeLayout> structLayout = new StructTypeLayout();
            structLayout->type = type;

            // Need to recursively walk the fields of the structure now...
            for( auto field : GetFields(structDeclRef) )
            {
                RefPtr<VarLayout> fieldVarLayout = new VarLayout();
                fieldVarLayout->varDecl = field;

                auto fieldTypeLayout = processEntryPointParameterWithPossibleSemantic(
                    context,
                    field.getDecl(),
                    GetType(field),
                    state,
                    fieldVarLayout);

                if(fieldTypeLayout)
                {
                    fieldVarLayout->typeLayout = fieldTypeLayout;

                    for (auto rr : fieldTypeLayout->resourceInfos)
                    {
                        SLANG_RELEASE_ASSERT(rr.count != 0);

                        auto structRes = structLayout->findOrAddResourceInfo(rr.kind);
                        fieldVarLayout->findOrAddResourceInfo(rr.kind)->index = structRes->count;
                        structRes->count += rr.count;
                    }
                }

                structLayout->fields.Add(fieldVarLayout);
                structLayout->mapVarToLayout.Add(field.getDecl(), fieldVarLayout);
            }

            return structLayout;
        }
        else if (auto globalGenericParam = declRef.As<GlobalGenericParamDecl>())
        {
            auto genParamTypeLayout = new GenericParamTypeLayout();
            // we should have already populated ProgramLayout::genericEntryPointParams list at this point,
            // so we can find the index of this generic param decl in the list
            genParamTypeLayout->type = type;
            genParamTypeLayout->paramIndex = findGenericParam(context->shared->programLayout->globalGenericParams, globalGenericParam.getDecl());
            genParamTypeLayout->findOrAddResourceInfo(LayoutResourceKind::GenericResource)->count++;
            return genParamTypeLayout;
        }
        else
        {
            SLANG_UNEXPECTED("unhandled type kind");
        }
    }
    // If we ran into an error in checking the user's code, then skip this parameter
    else if( auto errorType = type->As<ErrorType>() )
    {
        return nullptr;
    }

    SLANG_UNEXPECTED("unhandled type kind");
    UNREACHABLE_RETURN(nullptr);
}

static void collectEntryPointParameters(
    ParameterBindingContext*        context,
    EntryPointRequest*              entryPoint,
    Substitutions*                  typeSubst)
{
    FuncDecl* entryPointFuncDecl = entryPoint->decl;
    if (!entryPointFuncDecl)
    {
        // Something must have failed earlier, so that
        // we didn't find a declaration to match this
        // entry point request.
        return;
    }

    // Create the layout object here
    auto entryPointLayout = new EntryPointLayout();
    entryPointLayout->profile = entryPoint->profile;
    entryPointLayout->entryPoint = entryPointFuncDecl;


    context->shared->programLayout->entryPoints.Add(entryPointLayout);

    // Okay, we seemingly have an entry-point function, and now we need to collect info on its parameters too
    //
    // TODO: Long-term we probably want complete information on all inputs/outputs of an entry point,
    // but for now we are really just trying to scrape information on fragment outputs, so lets do that:
    //
    // TODO: check whether we should enumerate the parameters before the return type, or vice versa

    int defaultSemanticIndex = 0;

    EntryPointParameterState state;
    state.ioSemanticIndex = &defaultSemanticIndex;
    state.optSemanticName = nullptr;
    state.semanticSlotCount = 0;

    for( auto m : entryPointFuncDecl->Members )
    {
        auto paramDecl = m.As<VarDeclBase>();
        if(!paramDecl)
            continue;

        // We have an entry-point parameter, and need to figure out what to do with it.

        // TODO: need to handle `uniform`-qualified parameters here
        if (paramDecl->HasModifier<HLSLUniformModifier>())
            continue;

        state.directionMask = 0;

        // If it appears to be an input, process it as such.
        if( paramDecl->HasModifier<InModifier>() || paramDecl->HasModifier<InOutModifier>() || !paramDecl->HasModifier<OutModifier>() )
        {
            state.directionMask |= kEntryPointParameterDirection_Input;
        }

        // If it appears to be an output, process it as such.
        if(paramDecl->HasModifier<OutModifier>() || paramDecl->HasModifier<InOutModifier>())
        {
            state.directionMask |= kEntryPointParameterDirection_Output;
        }

        RefPtr<VarLayout> paramVarLayout = new VarLayout();
        paramVarLayout->varDecl = makeDeclRef(paramDecl.Ptr());

        auto paramTypeLayout = processEntryPointParameterWithPossibleSemantic(
            context,
            paramDecl.Ptr(),
            paramDecl->type.type->Substitute(typeSubst).As<Type>(),
            state,
            paramVarLayout);

        // Skip parameters for which we could not compute a layout
        if(!paramTypeLayout)
            continue;

        paramVarLayout->typeLayout = paramTypeLayout;

        for (auto rr : paramTypeLayout->resourceInfos)
        {
            auto entryPointRes = entryPointLayout->findOrAddResourceInfo(rr.kind);
            paramVarLayout->findOrAddResourceInfo(rr.kind)->index = entryPointRes->count;
            entryPointRes->count += rr.count;
        }

        entryPointLayout->fields.Add(paramVarLayout);
        entryPointLayout->mapVarToLayout.Add(paramDecl, paramVarLayout);
    }

    // If we can find an output type for the entry point, then process it as
    // an output parameter.
    if( auto resultType = entryPointFuncDecl->ReturnType.type )
    {
        state.directionMask = kEntryPointParameterDirection_Output;

        RefPtr<VarLayout> resultLayout = new VarLayout();

        auto resultTypeLayout = processEntryPointParameterWithPossibleSemantic(
            context,
            entryPointFuncDecl,
            resultType->Substitute(typeSubst).As<Type>(),
            state,
            resultLayout);

        if( resultTypeLayout )
        {
            resultLayout->typeLayout = resultTypeLayout;

            for (auto rr : resultTypeLayout->resourceInfos)
            {
                auto entryPointRes = entryPointLayout->findOrAddResourceInfo(rr.kind);
                resultLayout->findOrAddResourceInfo(rr.kind)->index = entryPointRes->count;
                entryPointRes->count += rr.count;
            }
        }

        entryPointLayout->resultLayout = resultLayout;
    }
}

// When doing parameter binding for global-scope stuff in GLSL,
// we may need to know what stage we are compiling for, so that
// we can handle special cases appropriately (e.g., "arrayed"
// inputs and outputs).
static Stage
inferStageForTranslationUnit(
    TranslationUnitRequest* translationUnit)
{
    // In the specific case where we are compiling GLSL input,
    // and have only a single entry point, use the stage
    // of the entry point.
    //
    // TODO: can we generalize this at all?
    if( translationUnit->sourceLanguage == SourceLanguage::GLSL )
    {
        if( translationUnit->entryPoints.Count() == 1 )
        {
            return translationUnit->entryPoints[0]->profile.GetStage();
        }
    }

    return Stage::Unknown;
}

static void collectModuleParameters(
    ParameterBindingContext*    inContext,
    ModuleDecl*          module)
{
    // Each loaded module provides a separate (logical) namespace for
    // parameters, so that two parameters with the same name, in
    // distinct modules, should yield different bindings.
    //
    ParameterBindingContext contextData = *inContext;
    auto context = &contextData;

    context->translationUnit = nullptr;

    context->stage = Stage::Unknown;

    // All imported modules are implicitly Slang code
    context->sourceLanguage = SourceLanguage::Slang;

    // A loaded module cannot define entry points that
    // we'll expose (for now), so we just need to
    // consider global-scope parameters.
    collectGlobalScopeParameters(context, module);
}

static void collectParameters(
    ParameterBindingContext*        inContext,
    CompileRequest*                 request)
{
    // All of the parameters in translation units directly
    // referenced in the compile request are part of one
    // logical namespace/"linkage" so that two parameters
    // with the same name should represent the same
    // parameter, and get the same binding(s)
    ParameterBindingContext contextData = *inContext;
    auto context = &contextData;

    for( auto& translationUnit : request->translationUnits )
    {
        context->translationUnit = translationUnit;
        context->stage = inferStageForTranslationUnit(translationUnit.Ptr());
        context->sourceLanguage = translationUnit->sourceLanguage;

        // First look at global-scope parameters
        collectGlobalScopeParameters(context, translationUnit->SyntaxNode.Ptr());

        // Next consider parameters for entry points
        for( auto& entryPoint : translationUnit->entryPoints )
        {
            context->stage = entryPoint->profile.GetStage();
            collectEntryPointParameters(context, entryPoint.Ptr(), nullptr);
        }
    }

    // Now collect parameters from loaded modules
    for (auto& loadedModule : request->loadedModulesList)
    {
        collectModuleParameters(context, loadedModule->moduleDecl.Ptr());
    }
}

static bool isGLSLCrossCompilerNeeded(
    TargetRequest*  targetReq)
{
    auto compileReq = targetReq->compileRequest;

    // We only need cross-compilation if we
    // are targetting something GLSL-based.
    switch (targetReq->target)
    {
    default:
        return false;

    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        break;
    }

    // If we `import`ed any Slang code, then the
    // cross compiler is definitely needed, to
    // translate that Slang over to GLSL.
    if (compileReq->loadedModulesList.Count() != 0)
        return true;

    // If there are any non-GLSL translation units,
    // then we need to cross compile those...
    for (auto tu : compileReq->translationUnits)
    {
        if (tu->sourceLanguage != SourceLanguage::GLSL)
            return true;
    }

    // If we get to this point, then we have plain vanilla
    // GLSL input, with no `import` declarations, so we
    // are able to output GLSL without cross compilation.
    return false;
}

void generateParameterBindings(
    TargetRequest*     targetReq)
{
    CompileRequest* compileReq = targetReq->compileRequest;

    // Try to find rules based on the selected code-generation target
    auto layoutContext = getInitialLayoutContextForTarget(targetReq);

    // If there was no target, or there are no rules for the target,
    // then bail out here.
    if (!layoutContext.rules)
        return;

    RefPtr<ProgramLayout> programLayout = new ProgramLayout();
    targetReq->layout = programLayout;

    // Create a context to hold shared state during the process
    // of generating parameter bindings
    SharedParameterBindingContext sharedContext;
    sharedContext.compileRequest = compileReq;
    sharedContext.defaultLayoutRules = layoutContext.getRulesFamily();
    sharedContext.programLayout = programLayout;

    // Create a sub-context to collect parameters that get
    // declared into the global scope
    ParameterBindingContext context;
    context.shared = &sharedContext;
    context.translationUnit = nullptr;
    context.layoutContext = layoutContext;
    // Walk through AST to discover all the parameters
    collectParameters(&context, compileReq);

    // Now walk through the parameters to generate initial binding information
    for( auto& parameter : sharedContext.parameters )
    {
        generateParameterBindings(&context, parameter);
    }

    bool anyGlobalUniforms = false;
    for( auto& parameterInfo : sharedContext.parameters )
    {
        SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);
        auto firstVarLayout = parameterInfo->varLayouts.First();

        // Does the field have any uniform data?
        if( firstVarLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
        {
            anyGlobalUniforms = true;
            break;
        }
    }

    // If there are any global-scope uniforms, then we need to
    // allocate a constant-buffer binding for them here.
    ParameterBindingInfo globalConstantBufferBinding;
    globalConstantBufferBinding.index = 0;
    if( anyGlobalUniforms )
    {
        // TODO: this logic is only correct for D3D targets, where
        // global-scope uniforms get wrapped into a constant buffer.

        UInt space = 0;
        auto usedRangeSet = findUsedRangeSetForSpace(&context, space);

        globalConstantBufferBinding.index =
            usedRangeSet->usedResourceRanges[
                (int)LayoutResourceKind::ConstantBuffer].Allocate(nullptr, 1);

        // For now we only auto-generate bindings in space zero
        globalConstantBufferBinding.space = space;
    }


    // Now walk through again to actually give everything
    // ranges of registers...
    for( auto& parameter : sharedContext.parameters )
    {
        completeBindingsForParameter(&context, parameter);
    }

    // TODO: need to deal with parameters declared inside entry-point
    // parameter lists at some point...


    // Next we need to create a type layout to reflect the information
    // we have collected.

    // We will lay out any bare uniforms at the global scope into
    // a single constant buffer. This is appropriate for HLSL global-scope
    // uniforms, and Vulkan GLSL doesn't allow uniforms at global scope,
    // so it should work out.
    //
    // For legacy GLSL targets, we'd probably need a distinct resource
    // kind and set of rules here, since legacy uniforms are not the
    // same as the contents of a constant buffer.
    auto globalScopeRules = context.getRulesFamily()->getConstantBufferRules();

    RefPtr<StructTypeLayout> globalScopeStructLayout = new StructTypeLayout();
    globalScopeStructLayout->rules = globalScopeRules;

    UniformLayoutInfo structLayoutInfo = globalScopeRules->BeginStructLayout();
    for( auto& parameterInfo : sharedContext.parameters )
    {
        SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);
        auto firstVarLayout = parameterInfo->varLayouts.First();

        // Does the field have any uniform data?
        auto layoutInfo = firstVarLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform);
        size_t uniformSize = layoutInfo ? layoutInfo->count : 0;
        if( uniformSize != 0 )
        {
            // Make sure uniform fields get laid out properly...

            UniformLayoutInfo fieldInfo(
                uniformSize,
                firstVarLayout->typeLayout->uniformAlignment);

            size_t uniformOffset = globalScopeRules->AddStructField(
                &structLayoutInfo,
                fieldInfo);

            for( auto& varLayout : parameterInfo->varLayouts )
            {
                varLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset;
            }
        }

        globalScopeStructLayout->fields.Add(firstVarLayout);

        for( auto& varLayout : parameterInfo->varLayouts )
        {
            globalScopeStructLayout->mapVarToLayout.Add(varLayout->varDecl.getDecl(), varLayout);
        }
    }
    globalScopeRules->EndStructLayout(&structLayoutInfo);

    RefPtr<TypeLayout> globalScopeLayout = globalScopeStructLayout;

    // If there are global-scope uniforms, then we need to wrap
    // up a global constant buffer type layout to hold them
    if( anyGlobalUniforms )
    {
        auto globalConstantBufferLayout = createParameterGroupTypeLayout(
            layoutContext,
            nullptr,
            globalScopeRules,
            globalScopeRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer),
            globalScopeStructLayout);

        globalScopeLayout = globalConstantBufferLayout;
    }

    // Final final step: pick a binding for the "hack sampler", if needed...
    //
    // We only want to do this if the GLSL cross-compilation support is
    // being invoked, so that we don't gum up other shaders.
    if(isGLSLCrossCompilerNeeded(targetReq))
    {
        UInt space = 0;
        auto hackSamplerUsedRanges = findUsedRangeSetForSpace(&context, space);

        UInt binding = hackSamplerUsedRanges->usedResourceRanges[(int)LayoutResourceKind::DescriptorTableSlot].Allocate(nullptr, 1);

        programLayout->bindingForHackSampler = (int)binding;

        RefPtr<Variable> var = new Variable();
        var->nameAndLoc.name = compileReq->getNamePool()->getName("SLANG_hack_samplerForTexelFetch");
        var->type.type = getSamplerStateType(compileReq->mSession);

        auto typeLayout = new TypeLayout();
        typeLayout->type = var->type.type;
        typeLayout->addResourceUsage(LayoutResourceKind::DescriptorTableSlot, 1);

        auto varLayout = new VarLayout();
        varLayout->varDecl = makeDeclRef(var.Ptr());
        varLayout->typeLayout = typeLayout;
        auto resInfo = varLayout->AddResourceInfo(LayoutResourceKind::DescriptorTableSlot);
        resInfo->index = binding;
        resInfo->space = space;

        programLayout->hackSamplerVar = var;

        globalScopeStructLayout->fields.Add(varLayout);
    }

    // We now have a bunch of layout information, which we should
    // record into a suitable object that represents the program
    RefPtr<VarLayout> globalVarLayout = new VarLayout();
    globalVarLayout->typeLayout = globalScopeLayout;
    if (anyGlobalUniforms)
    {
        auto cbInfo = globalVarLayout->findOrAddResourceInfo(LayoutResourceKind::ConstantBuffer);
        cbInfo->space = 0;
        cbInfo->index = globalConstantBufferBinding.index;
    }
    programLayout->globalScopeLayout = globalVarLayout;
}

StructTypeLayout* getGlobalStructLayout(
    ProgramLayout*  programLayout);

RefPtr<ProgramLayout> specializeProgramLayout(
    TargetRequest * targetReq,
    ProgramLayout* programLayout, 
    Substitutions * typeSubst)
{
    RefPtr<ProgramLayout> newProgramLayout;
    newProgramLayout = new ProgramLayout();
    newProgramLayout->bindingForHackSampler = programLayout->bindingForHackSampler;
    newProgramLayout->hackSamplerVar = programLayout->hackSamplerVar;
    newProgramLayout->globalGenericParams = programLayout->globalGenericParams;

    List<RefPtr<TypeLayout>> paramTypeLayouts;
    auto globalStructLayout = getGlobalStructLayout(programLayout);
    SLANG_ASSERT(globalStructLayout);
    RefPtr<StructTypeLayout> structLayout = new StructTypeLayout();
    RefPtr<TypeLayout> globalScopeLayout = structLayout;
    structLayout->uniformAlignment = globalStructLayout->uniformAlignment;
    
    // Try to find rules based on the selected code-generation target
    auto layoutContext = getInitialLayoutContextForTarget(targetReq);

    // If there was no target, or there are no rules for the target,
    // then bail out here.
    if (!layoutContext.rules)
        return newProgramLayout;

 
    // we need to initialize a layout context to mark used registers
    SharedParameterBindingContext sharedContext;
    sharedContext.compileRequest = targetReq->compileRequest;
    sharedContext.defaultLayoutRules = layoutContext.getRulesFamily();
    sharedContext.programLayout = newProgramLayout;

    // Create a sub-context to collect parameters that get
    // declared into the global scope
    ParameterBindingContext context;
    context.shared = &sharedContext;
    context.translationUnit = nullptr;
    context.layoutContext = layoutContext;
    
    
    for (auto & translationUnit : targetReq->compileRequest->translationUnits)
    {
        for (auto & entryPoint : translationUnit->entryPoints)
        {
            collectEntryPointParameters(&context, entryPoint, typeSubst);
        }
    }

    auto constantBufferRules = context.getRulesFamily()->getConstantBufferRules();
    structLayout->rules = constantBufferRules;

    UniformLayoutInfo structLayoutInfo;
    structLayoutInfo.alignment = globalStructLayout->uniformAlignment;
    structLayoutInfo.size = 0;
    bool anyUniforms = false;
    Dictionary<RefPtr<VarLayout>, RefPtr<VarLayout>> varLayoutMapping;
    for (auto & varLayout : globalStructLayout->fields)
    {
        // To recover layout context, we skip generic resources in the first pass
        // If the var is a generic resource, its resourceInfos will be empty.
        if (varLayout->resourceInfos.Count() == 0)
            continue;
        SLANG_ASSERT(varLayout->resourceInfos.Count() == varLayout->typeLayout->resourceInfos.Count());
        auto uniformInfo = varLayout->FindResourceInfo(LayoutResourceKind::Uniform);
        auto tUniformInfo = varLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform);
        if (uniformInfo)
        {
            anyUniforms = true;
            SLANG_ASSERT(tUniformInfo);
            structLayoutInfo.size = Math::Max(structLayoutInfo.size, uniformInfo->index + tUniformInfo->count);
        }
        for (UInt i = 0; i < varLayout->resourceInfos.Count(); i++)
        {
            auto resInfo = varLayout->resourceInfos[i];
            auto tresInfo = varLayout->typeLayout->resourceInfos[i];
            SLANG_ASSERT(resInfo.kind == tresInfo.kind);
            auto usedRangeSet = findUsedRangeSetForSpace(&context, resInfo.space);
            markSpaceUsed(&context, resInfo.space);
            usedRangeSet->usedResourceRanges[(int)resInfo.kind].Add(
                nullptr, // we don't need to track parameter info here
                resInfo.index,
                resInfo.index + varLayout->typeLayout->resourceInfos[0].count);
        }
        structLayout->fields.Add(varLayout);
        varLayoutMapping[varLayout] = varLayout;
    }
    auto originalGlobalCBufferInfo = programLayout->globalScopeLayout->FindResourceInfo(LayoutResourceKind::ConstantBuffer);
    VarLayout::ResourceInfo globalCBufferInfo;
    globalCBufferInfo.kind = LayoutResourceKind::None;
    globalCBufferInfo.space = 0;
    globalCBufferInfo.index = 0;
    if (originalGlobalCBufferInfo)
    {
        globalCBufferInfo.kind = LayoutResourceKind::ConstantBuffer;
        globalCBufferInfo.space = originalGlobalCBufferInfo->space;
        globalCBufferInfo.index = originalGlobalCBufferInfo->index;
    }
    // we have the context restored, can continue to layout the generic variables now
    for (auto & varLayout : globalStructLayout->fields)
    {
        if (varLayout->typeLayout->FindResourceInfo(LayoutResourceKind::GenericResource))
        {
            RefPtr<Type> newType = varLayout->typeLayout->type->Substitute(typeSubst).As<Type>();
            RefPtr<TypeLayout> newTypeLayout = CreateTypeLayout(
                layoutContext.with(constantBufferRules),
                newType);
            auto layoutInfo = newTypeLayout->FindResourceInfo(LayoutResourceKind::Uniform);
            size_t uniformSize = layoutInfo ? layoutInfo->count : 0;
            if (uniformSize)
            {
                if (globalCBufferInfo.kind == LayoutResourceKind::None)
                {
                    // user defined a uniform via a global generic type argument
                    // but we have not reserved a binding for the global uniform buffer
                    UInt space = 0;
                    auto usedRangeSet = findUsedRangeSetForSpace(&context, space);
                    globalCBufferInfo.kind = LayoutResourceKind::ConstantBuffer;
                    globalCBufferInfo.index =
                        usedRangeSet->usedResourceRanges[
                            (int)LayoutResourceKind::ConstantBuffer].Allocate(nullptr, 1);
                    globalCBufferInfo.space = space;
                }
            }
            RefPtr<VarLayout> newVarLayout = new VarLayout();
            RefPtr<ParameterInfo> paramInfo = new ParameterInfo();
            newVarLayout->varDecl = varLayout->varDecl;
            newVarLayout->typeLayout = newTypeLayout;
            paramInfo->varLayouts.Add(newVarLayout);
            completeBindingsForParameter(&context, paramInfo);
            // update uniform layout
            
            if (uniformSize != 0)
            {
                // Make sure uniform fields get laid out properly...
                UniformLayoutInfo fieldInfo(
                    uniformSize,
                    newTypeLayout->uniformAlignment);
                size_t uniformOffset = layoutContext.getRulesFamily()->getConstantBufferRules()->AddStructField(
                    &structLayoutInfo,
                    fieldInfo);
                newVarLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset;
                anyUniforms = true;
            }
            structLayout->fields.Add(newVarLayout);
            varLayoutMapping[varLayout] = newVarLayout;
        }
    }
    for (auto mapping : globalStructLayout->mapVarToLayout)
    {
        RefPtr<VarLayout> updatedVarLayout = mapping.Value;
        varLayoutMapping.TryGetValue(updatedVarLayout, updatedVarLayout);
        structLayout->mapVarToLayout[mapping.Key] = updatedVarLayout;
    }

    // If there are global-scope uniforms, then we need to wrap
    // up a global constant buffer type layout to hold them
    RefPtr<VarLayout> globalVarLayout = new VarLayout();
    if (anyUniforms)
    {
        auto globalConstantBufferLayout = createParameterGroupTypeLayout(
            layoutContext,
            nullptr,
            constantBufferRules,
            constantBufferRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer),
            structLayout);

        globalScopeLayout = globalConstantBufferLayout;
        auto cbInfo = globalVarLayout->findOrAddResourceInfo(LayoutResourceKind::ConstantBuffer);
        *cbInfo = globalCBufferInfo;
    }
    globalVarLayout->typeLayout = globalScopeLayout;
    programLayout->globalScopeLayout = globalVarLayout;
    newProgramLayout->globalScopeLayout = globalVarLayout;
    return newProgramLayout;
}
}

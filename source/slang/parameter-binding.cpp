// parameter-binding.cpp
#include "parameter-binding.h"

#include "lookup.h"
#include "compiler.h"
#include "type-layout.h"

#include "../../slang.h"

#define SLANG_EXHAUSTIVE_SWITCH() default: assert(!"unexpected"); break;

namespace Slang {

// Information on ranges of registers already claimed/used
struct UsedRange
{
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

struct UsedRanges
{
    List<UsedRange> ranges;

    // Add a range to the set, either by extending
    // an existing range, or by adding a new one...
    void Add(UsedRange const& range)
    {
        for (auto& rr : ranges)
        {
            if (rr.begin == range.end)
            {
                rr.begin = range.begin;
                return;
            }
            else if (rr.end == range.begin)
            {
                rr.end = range.end;
                return;
            }
        }
        ranges.Add(range);
        ranges.Sort();
    }

    void Add(UInt begin, UInt end)
    {
        UsedRange range;
        range.begin = begin;
        range.end = end;
        Add(range);
    }


    // Try to find space for `count` entries
    UInt Allocate(UInt count)
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
                Add(begin, begin + count);
                return begin;
            }

            // ... otherwise, we need to look at the
            // space between this range and the next
            begin = ranges[rr].end;
        }

        // We've run out of ranges to check, so we
        // can safely go after the last one!
        Add(begin, begin + count);
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

// Information on a single parameter
struct ParameterInfo : RefObject
{
    // Layout info for the concrete variables that will make up this parameter
    List<RefPtr<VarLayout>> varLayouts;

    ParameterBindingInfo    bindingInfo[kLayoutResourceKindCount];

    // The next parameter that has the same name...
    ParameterInfo* nextOfSameName;

    ParameterInfo()
    {
        // Make sure we aren't claiming any resources yet
        for( int ii = 0; ii < kLayoutResourceKindCount; ++ii )
        {
            bindingInfo[ii].count = 0;
        }
    }
};

// State that is shared during parameter binding,
// across all translation units
struct SharedParameterBindingContext
{
    LayoutRulesFamilyImpl* defaultLayoutRules;

    // All shader parameters we've discovered so far, and started to lay out...
    List<RefPtr<ParameterInfo>> parameters;

    // The program layout we are trying to construct
    RefPtr<ProgramLayout> programLayout;

    // Information on what ranges of "registers" have already
    // been claimed, for each resource type
    UsedRanges usedResourceRanges[kLayoutResourceKindCount];
};

// State that might be specific to a single translation unit
// or event to an entry point.
struct ParameterBindingContext
{
    // All the shared state needs to be available
    SharedParameterBindingContext* shared;

    // The layout rules to use while computing usage...
    LayoutRulesFamilyImpl* layoutRules;

    // A dictionary to accellerate looking up parameters by name
    Dictionary<String, ParameterInfo*> mapNameToParameterInfo;

    // What stage (if any) are we compiling for?
    Stage stage;

    // The source language we are trying to use
    SourceLanguage sourceLanguage;
};

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

static bool doesParameterMatch(
    ParameterBindingContext*,
    RefPtr<VarLayout>,
    ParameterInfo*)
{
    // TODO: need to implement this eventually
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

RefPtr<TypeLayout>
getTypeLayoutForGlobalShaderParameter_GLSL(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl)
{
    auto rules = context->layoutRules;
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
    if (varDecl->HasModifier<GLSLPushConstantLayoutModifier>() && type->As<ConstantBufferType>())
        return CreateTypeLayout(type, rules->getPushConstantBufferRules());

    // TODO(tfoley): We have multiple variations of
    // the `uniform` modifier right now, and that
    // needs to get fixed...
    if(varDecl->HasModifier<HLSLUniformModifier>() || type->As<ConstantBufferType>())
        return CreateTypeLayout(type, rules->getConstantBufferRules());

    if(varDecl->HasModifier<GLSLBufferModifier>() || type->As<GLSLShaderStorageBufferType>())
        return CreateTypeLayout(type, rules->getShaderStorageBufferRules());

    if( varDecl->HasModifier<InModifier>() || type->As<GLSLInputParameterBlockType>())
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
                    type = arrayType->BaseType.Ptr();
                }
            }
            break;

        default:
            break;
        }

        return CreateTypeLayout(type, rules->getVaryingInputRules());
    }

    if( varDecl->HasModifier<OutModifier>() || type->As<GLSLOutputParameterBlockType>())
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
                    type = arrayType->BaseType.Ptr();
                }
            }
            break;

        default:
            break;
        }

        return CreateTypeLayout(type, rules->getVaryingOutputRules());
    }

    // A `const` global with a `layout(constant_id = ...)` modifier
    // is a declaration of a specialization constant.
    if(varDecl->HasModifier<GLSLConstantIDLayoutModifier>())
        return CreateTypeLayout(type, rules->getSpecializationConstantRules());

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
    auto rules = context->layoutRules;
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
    return CreateTypeLayout(type, rules->getConstantBufferRules());
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
        assert(false);
        return nullptr;
    }
}


//



// Collect a single declaration into our set of parameters
static void collectGlobalScopeParameter(
    ParameterBindingContext*    context,
    RefPtr<VarDeclBase>         varDecl)
{
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
    auto parameterName = varDecl->Name.Content;
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
        context->mapNameToParameterInfo.Add(parameterName, parameterInfo);
    }
    else
    {
        varLayout->flags |= VarLayoutFlag::IsRedeclaration;
    }

    // Add this variable declaration to the list of declarations for the parameter
    parameterInfo->varLayouts.Add(varLayout);
}

static void addExplicitParameterBinding(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    LayoutSemanticInfo const&   semanticInfo,
    UInt                        count)
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
            // TODO: diagnose!
        }

        // TODO(tfoley): `register` semantics can technically be
        // profile-specific (not sure if anybody uses that)...
    }
    else
    {
        bindingInfo.count = count;
        bindingInfo.index = semanticInfo.index;
        bindingInfo.space = semanticInfo.space;

        // If things are bound in `space0` (the default), then we need
        // to lay claim to the register range used, so that automatic
        // assignment doesn't go and use the same registers.
        if (semanticInfo.space == 0)
        {
            context->shared->usedResourceRanges[(int)semanticInfo.kind].Add(
                semanticInfo.index,
                semanticInfo.index + count);
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

        addExplicitParameterBinding(context, parameterInfo, semanticInfo, count);
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
    }
    else if( (resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::FragmentOutput)) != nullptr )
    {
        // Try to find `location` binding
        if(!findLayoutArg<GLSLLocationLayoutModifier>(varDecl, &semanticInfo.index))
            return;
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

    addExplicitParameterBinding(context, parameterInfo, semanticInfo, int(count));
}

// Given a single parameter, collect whatever information we have on
// how it has been explicitly bound, which may come from multiple declarations
void generateParameterBindings(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo)
{
    // There must be at least one declaration for the parameter.
    assert(parameterInfo->varLayouts.Count() != 0);

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

    assert(parameterInfo->varLayouts.Count() != 0);
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
        bindingInfo.count = count;
        bindingInfo.index = context->shared->usedResourceRanges[(int)kind].Allocate((int) count);

        // For now we only auto-generate bindings in space zero
        bindingInfo.space = 0;
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
    ProgramSyntaxNode*          program)
{
    // First enumerate parameters at global scope
    for( auto decl : program->Members )
    {
        // A shader parameter is always a variable,
        // so skip declarations that aren't variables.
        auto varDecl = decl.As<VarDeclBase>();
        if (!varDecl)
            continue;

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

enum EntryPointParameterDirection
{
    kEntryPointParameterDirection_Input  = 0x1,
    kEntryPointParameterDirection_Output = 0x2,
};
typedef unsigned int EntryPointParameterDirectionMask;

struct EntryPointParameterState
{
    String*                             optSemanticName;
    int*                                ioSemanticIndex;
    EntryPointParameterDirectionMask    directionMask;
    int                                 semanticSlotCount;
};

static RefPtr<TypeLayout> processSimpleEntryPointParameter(
    ParameterBindingContext*        context,
    RefPtr<ExpressionType>          type,
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
    if (sn.StartsWith("sv_"))
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
                context->shared->usedResourceRanges[int(LayoutResourceKind::UnorderedAccess)].Add(semanticIndex, semanticIndex + semanticSlotCount);
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
            auto rules = context->layoutRules->getVaryingInputRules();
            SimpleLayoutInfo layout = GetLayout(type, rules);
            typeLayout->addResourceUsage(layout.kind, layout.size);
        }

        if (state.directionMask & kEntryPointParameterDirection_Output)
        {
            auto rules = context->layoutRules->getVaryingOutputRules();
            SimpleLayoutInfo layout = GetLayout(type, rules);
            typeLayout->addResourceUsage(layout.kind, layout.size);
        }
    }

    *state.ioSemanticIndex += state.semanticSlotCount;
    typeLayout->type = type;

    return typeLayout;
}

static RefPtr<TypeLayout> processEntryPointParameter(
    ParameterBindingContext*        context,
    RefPtr<ExpressionType>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout);

static RefPtr<TypeLayout> processEntryPointParameterWithPossibleSemantic(
    ParameterBindingContext*        context,
    Decl*                           declForSemantic,
    RefPtr<ExpressionType>          type,
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

            processEntryPointParameter(context, type, subState, varLayout);
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
    RefPtr<ExpressionType>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout)
{
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
        auto elementTypeLayout = processEntryPointParameter(context, arrayType->BaseType, state, varLayout);

        // We still walk over subsequent elements to make sure they consume resources
        // as needed
        for( UInt ii = 1; ii < elementCount; ++ii )
        {
            processEntryPointParameter(context, arrayType->BaseType, state, nullptr);
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

        if (auto structDeclRef = declRef.As<StructSyntaxNode>())
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

                fieldVarLayout->typeLayout = fieldTypeLayout;

                for (auto rr : fieldTypeLayout->resourceInfos)
                {
                    assert(rr.count != 0);

                    auto structRes = structLayout->findOrAddResourceInfo(rr.kind);
                    fieldVarLayout->findOrAddResourceInfo(rr.kind)->index = structRes->count;
                    structRes->count += rr.count;
                }

                structLayout->fields.Add(fieldVarLayout);
                structLayout->mapVarToLayout.Add(field.getDecl(), fieldVarLayout);
            }

            return structLayout;
        }
        else
        {
            assert(!"unimplemented");
        }
    }
    else
    {
        assert(!"unimplemented");
    }

    assert(!"unexpected");
    return nullptr;
}

static void collectEntryPointParameters(
    ParameterBindingContext*        context,
    EntryPointRequest*              entryPoint,
    ProgramSyntaxNode*              translationUnitSyntax)
{
    // First, look for the entry point with the specified name

    // Make sure we've got a query-able member dictionary
    buildMemberDictionary(translationUnitSyntax);

    Decl* entryPointDecl;
    if( !translationUnitSyntax->memberDictionary.TryGetValue(entryPoint->name, entryPointDecl) )
    {
        // No such entry point!
        return;
    }
    if( entryPointDecl->nextInContainerWithSameName )
    {
        // Not the only decl of that name!
        return;
    }

    FunctionSyntaxNode* entryPointFuncDecl = dynamic_cast<FunctionSyntaxNode*>(entryPointDecl);
    if( !entryPointFuncDecl )
    {
        // Not a function!
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
            paramDecl->Type.type,
            state,
            paramVarLayout);

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
            resultType,
            state,
            resultLayout);

        resultLayout->typeLayout = resultTypeLayout;

        for (auto rr : resultTypeLayout->resourceInfos)
        {
            auto entryPointRes = entryPointLayout->findOrAddResourceInfo(rr.kind);
            resultLayout->findOrAddResourceInfo(rr.kind)->index = entryPointRes->count;
            entryPointRes->count += rr.count;
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
    ProgramSyntaxNode*          module)
{
    // Each loaded module provides a separate (logical) namespace for
    // parameters, so that two parameters with the same name, in
    // distinct modules, should yield different bindings.
    //
    ParameterBindingContext contextData = *inContext;
    auto context = &contextData;

    context->stage = Stage::Unknown;

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
        context->stage = inferStageForTranslationUnit(translationUnit.Ptr());
        context->sourceLanguage = translationUnit->sourceLanguage;

        // First look at global-scope parameters
        collectGlobalScopeParameters(context, translationUnit->SyntaxNode.Ptr());

        // Next consider parameters for entry points
        for( auto& entryPoint : translationUnit->entryPoints )
        {
            context->stage = entryPoint->profile.GetStage();
            collectEntryPointParameters(context, entryPoint.Ptr(), translationUnit->SyntaxNode.Ptr());
        }
    }

    // Now collect parameters from loaded modules
    for (auto& module : request->loadedModulesList)
    {
        collectModuleParameters(context, module.Ptr());
    }
}

void generateParameterBindings(
    CompileRequest*                 request)
{
    // Try to find rules based on the selected code-generation target
    auto rules = GetLayoutRulesFamilyImpl(request->Target);

    // If there was no target, or there are no rules for the target,
    // then bail out here.
    if (!rules)
        return;

    RefPtr<ProgramLayout> programLayout = new ProgramLayout;

    // Create a context to hold shared state during the process
    // of generating parameter bindings
    SharedParameterBindingContext sharedContext;
    sharedContext.defaultLayoutRules = rules;
    sharedContext.programLayout = programLayout;

    // Create a sub-context to collect parameters that get
    // declared into the global scope
    ParameterBindingContext context;
    context.shared = &sharedContext;
    context.layoutRules = sharedContext.defaultLayoutRules;

    // Walk through AST to discover all the parameters
    collectParameters(&context, request);

    // Now walk through the parameters to generate initial binding information
    for( auto& parameter : sharedContext.parameters )
    {
        generateParameterBindings(&context, parameter);
    }

    bool anyGlobalUniforms = false;
    for( auto& parameterInfo : sharedContext.parameters )
    {
        assert(parameterInfo->varLayouts.Count() != 0);
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
    if( anyGlobalUniforms )
    {
        globalConstantBufferBinding.index =
            context.shared->usedResourceRanges[
                (int)LayoutResourceKind::ConstantBuffer].Allocate(1);

        // For now we only auto-generate bindings in space zero
        globalConstantBufferBinding.space = 0;
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
    auto globalScopeRules = context.layoutRules->getConstantBufferRules();

    RefPtr<StructTypeLayout> globalScopeStructLayout = new StructTypeLayout();
    globalScopeStructLayout->rules = globalScopeRules;

    UniformLayoutInfo structLayoutInfo = globalScopeRules->BeginStructLayout();
    for( auto& parameterInfo : sharedContext.parameters )
    {
        assert(parameterInfo->varLayouts.Count() != 0);
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
        auto globalConstantBufferLayout = createParameterBlockTypeLayout(
            nullptr,
            globalScopeRules,
            globalScopeRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer),
            globalScopeStructLayout);

        globalScopeLayout = globalConstantBufferLayout;
    }

    // We now have a bunch of layout information, which we should
    // record into a suitable object that represents the program
    programLayout->globalScopeLayout = globalScopeLayout;
    request->layout = programLayout;
}

}

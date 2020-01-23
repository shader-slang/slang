// slang-parameter-binding.cpp
#include "slang-parameter-binding.h"

#include "slang-lookup.h"
#include "slang-compiler.h"
#include "slang-type-layout.h"

#include "slang-ir-string-hash.h"

#include "../../slang.h"

namespace Slang {

struct ParameterInfo;

// Information on ranges of registers already claimed/used
struct UsedRange
{
    // What parameter has claimed this range?
    VarLayout* parameter;

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
    // The `ranges` array maintains a sorted list of `UsedRange`
    // objects such that the `end` of a range is <= the `begin`
    // of any range that comes after it.
    //
    // The values covered by each `[begin,end)` range are marked
    // as used, and anything not in such an interval is implicitly
    // free.
    //
    // TODO: if it ever starts to matter for performance, we
    // could encode this information as a tree instead of an array.
    //
    List<UsedRange> ranges;

    // Add a range to the set, either by extending
    // existing range(s), or by adding a new one.
    //
    // If we find that the new range overlaps with
    // an existing range for a *different* parameter
    // then we return that parameter so that the
    // caller can issue an error.
    //
    VarLayout* Add(UsedRange range)
    {
        // The invariant on entry to this
        // function is that the `ranges` array
        // is sorted and no two entries in the
        // array intersect. We must preserve
        // that property as a postcondition.
        //
        // The other postcondition is that the
        // interval covered by the input `range`
        // must be marked as consumed.

        // We will try track any parameter associated
        // with an overlapping range that doesn't
        // match the parameter on `range`, so that
        // the compiler can issue useful diagnostics.
        //
        VarLayout* newParam = range.parameter;
        VarLayout* existingParam = nullptr;

        // A clever algorithm might use a binary
        // search to identify the first entry in `ranges`
        // that might overlap `range`, but we are going
        // to settle for being less clever for now, in
        // the hopes that we can at least be correct.
        //
        // Note: we are going to iterate over `ranges`
        // using indices, because we may actually modify
        // the array as we go.
        //
        Int rangeCount = ranges.getCount();
        for(Int rr = 0; rr < rangeCount; ++rr)
        {
            auto existingRange = ranges[rr];

            // The invariant on entry to each loop
            // iteration will be that `range` does
            // *not* intersect any preceding entry
            // in the array.
            //
            // Note that this invariant might be
            // true only because we modified
            // `range` along the way.
            //
            // If `range` does not intertsect `existingRange`
            // then our invariant will be trivially
            // true for the next iteration.
            //
            if(!rangesOverlap(existingRange, range))
            {
                continue;
            }

            // We now know that `range` and `existingRange`
            // intersect. The first thing to do
            // is to check if we have a parameter
            // associated with `existingRange`, so
            // that we can use it for emitting diagnostics
            // about the overlap:
            //
            if( existingRange.parameter
                && existingRange.parameter != newParam)
            {
                // There was an overlap with a range that
                // had a parameter specified, so we will
                // use that parameter in any subsequent
                // diagnostics.
                //
                existingParam = existingRange.parameter;
            }

            // Before we can move on in our iteration,
            // we need to re-establish our invariant by modifying
            // `range` so that it doesn't overlap with `existingRange`.
            // Of course we also want to end up with a correct
            // result for the overall operation, so we can't just
            // throw away intervals.
            //
            // We first note that if `range` starts before `existingRange`,
            // then the interval from `range.begin` to `existingRange.begin`
            // needs to be accounted for in the final result. Furthermore,
            // the interval `[range.begin, existingRange.begin)` could not
            // intersect with any range already in the `ranges` array,
            // because it comes strictly before `existingRange`, and our
            // invariant says there is no intersection with preceding ranges.
            //
            if(range.begin < existingRange.begin)
            {
                UsedRange prefix;
                prefix.begin = range.begin;
                prefix.end = existingRange.begin;
                prefix.parameter = range.parameter;
                ranges.add(prefix);
            }
            //
            // Now we know that the interval `[range.begin, existingRange.begin)`
            // is claimed, if it exists, and clearly the interval
            // `[existingRange.begin, existingRange.end)` is already claimed,
            // so the only interval left to consider would be
            // `[existingRange.end, range.end)`, if it is non-empty.
            // That range might intersect with others in the array, so
            // we will need to continue iterating to deal with that
            // possibility.
            //
            range.begin = existingRange.end;

            // If the range would be empty, then of course we have nothing
            // left to do.
            //
            if(range.begin >= range.end)
                break;

            // Otherwise, have can be sure that `range` now comes
            // strictly *after* `existingRange`, and thus our invariant
            // is preserved.
        }

        // If we manage to exit the loop, then we have resolved
        // an intersection with existing entries - possibly by
        // adding some new entries.
        //
        // If the `range` we are left with is still non-empty,
        // then we should go ahead and add it.
        //
        if(range.begin < range.end)
        {
            ranges.add(range);
        }

        // Any ranges that got added along the way might not
        // be in the proper sorted order, so we'll need to
        // sort the array to restore our global invariant.
        //
        ranges.sort();

        // We end by returning an overlapping parameter that
        // we found along the way, if any.
        //
        return existingParam;
    }

    VarLayout* Add(VarLayout* param, UInt begin, UInt end)
    {
        UsedRange range;
        range.parameter = param;
        range.begin = begin;
        range.end = end;
        return Add(range);
    }

    VarLayout* Add(VarLayout* param, UInt begin, LayoutSize end)
    {
        UsedRange range;
        range.parameter = param;
        range.begin = begin;
        range.end = end.isFinite() ? end.getFiniteValue() : UInt(-1);
        return Add(range);
    }

    bool contains(UInt index)
    {
        for (auto rr : ranges)
        {
            if (index < rr.begin)
                return false;

            if (index >= rr.end)
                continue;

            return true;
        }

        return false;
    }


    // Try to find space for `count` entries
    UInt Allocate(VarLayout* param, UInt count)
    {
        UInt begin = 0;

        UInt rangeCount = ranges.getCount();
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
    size_t              space = 0;
    size_t              index = 0;
    LayoutSize          count;
};

struct ParameterBindingAndKindInfo : ParameterBindingInfo
{
    LayoutResourceKind kind = LayoutResourceKind::None;
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
    // Layout info for the variable that represents this parameter
    RefPtr<VarLayout> varLayout;

    ParameterBindingInfo    bindingInfo[kLayoutResourceKindCount];

    // The translation unit this parameter is specific to, if any
//    TranslationUnitRequest* translationUnit = nullptr;

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
    SharedParameterBindingContext(
        LayoutRulesFamilyImpl*  defaultLayoutRules,
        ProgramLayout*          programLayout,
        TargetRequest*          targetReq,
        DiagnosticSink*         sink)
        : defaultLayoutRules(defaultLayoutRules)
        , programLayout(programLayout)
        , targetRequest(targetReq)
        , m_sink(sink)
    {
    }

    DiagnosticSink* m_sink = nullptr;

    // The program that we are laying out
//    Program* program = nullptr;

    // The target request that is triggering layout
    //
    // TODO: We should eventually strip this down to
    // just the subset of fields on the target that
    // can influence layout decisions.
    TargetRequest*  targetRequest = nullptr;

    LayoutRulesFamilyImpl* defaultLayoutRules;

    // All shader parameters we've discovered so far, and started to lay out...
    List<RefPtr<ParameterInfo>> parameters;

    // The program layout we are trying to construct
    RefPtr<ProgramLayout> programLayout;

    // What ranges of resources bindings are already claimed at the global scope?
    // We store one of these for each declared binding space/set.
    //
    Dictionary<UInt, RefPtr<UsedRangeSet>> globalSpaceUsedRangeSets;

    // Which register spaces have been claimed so far?
    UsedRanges usedSpaces;

    // The space to use for auto-generated bindings.
    UInt defaultSpace = 0;

    TargetRequest* getTargetRequest() { return targetRequest; }
    DiagnosticSink* getSink() { return m_sink; }
    Linkage* getLinkage() { return targetRequest->getLinkage(); }
};

static DiagnosticSink* getSink(SharedParameterBindingContext* shared)
{
    return shared->getSink();
}

// State that might be specific to a single translation unit
// or event to an entry point.
struct ParameterBindingContext
{
    // All the shared state needs to be available
    SharedParameterBindingContext* shared;

    // The type layout context to use when computing
    // the resource usage of shader parameters.
    TypeLayoutContext layoutContext;

    // What stage (if any) are we compiling for?
    Stage stage;

    // The entry point that is being processed right now.
    EntryPointLayout*   entryPointLayout = nullptr;

    TargetRequest* getTargetRequest() { return shared->getTargetRequest(); }
    LayoutRulesFamilyImpl* getRulesFamily() { return layoutContext.getRulesFamily(); }

    Linkage* getLinkage() { return shared->getLinkage(); }
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

static bool isDigit(char c)
{
    return (c >= '0') && (c <= '9');
}

/// Given a string that specifies a name and index (e.g., `COLOR0`),
/// split it into slices for the name part and the index part.
static void splitNameAndIndex(
    UnownedStringSlice const&       text,
    UnownedStringSlice& outName,
    UnownedStringSlice& outDigits)
{
    char const* nameBegin = text.begin();
    char const* digitsEnd = text.end();

    char const* nameEnd = digitsEnd;
    while( nameEnd != nameBegin && isDigit(*(nameEnd - 1)) )
    {
        nameEnd--;
    }
    char const* digitsBegin = nameEnd;

    outName = UnownedStringSlice(nameBegin, nameEnd);
    outDigits = UnownedStringSlice(digitsBegin, digitsEnd);
}

LayoutResourceKind findRegisterClassFromName(UnownedStringSlice const& registerClassName)
{
    switch( registerClassName.size() )
    {
    case 1:
        switch (*registerClassName.begin())
        {
        case 'b': return LayoutResourceKind::ConstantBuffer;
        case 't': return LayoutResourceKind::ShaderResource;
        case 'u': return LayoutResourceKind::UnorderedAccess;
        case 's': return LayoutResourceKind::SamplerState;

        default:
            break;
        }
        break;

    case 5:
        if( registerClassName == "space" )
        {
            return LayoutResourceKind::RegisterSpace;
        }
        break;

    default:
        break;
    }
    return LayoutResourceKind::None;
}

LayoutSemanticInfo ExtractLayoutSemanticInfo(
    ParameterBindingContext*    context,
    HLSLLayoutSemantic*         semantic)
{
    LayoutSemanticInfo info;
    info.space = 0;
    info.index = 0;
    info.kind = LayoutResourceKind::None;

    UnownedStringSlice registerName = semantic->registerName.Content;
    if (registerName.size() == 0)
        return info;

    // The register name is expected to be in the form:
    //
    //      identifier-char+ digit+
    //
    // where the identifier characters name a "register class"
    // and the digits identify a register index within that class.
    //
    // We are going to split the string the user gave us
    // into these constituent parts:
    //
    UnownedStringSlice registerClassName;
    UnownedStringSlice registerIndexDigits;
    splitNameAndIndex(registerName, registerClassName, registerIndexDigits);

    LayoutResourceKind kind = findRegisterClassFromName(registerClassName);
    if(kind == LayoutResourceKind::None)
    {
        getSink(context)->diagnose(semantic->registerName, Diagnostics::unknownRegisterClass, registerClassName);
        return info;
    }

    // For a `register` semantic, the register index is not optional (unlike
    // how it works for varying input/output semantics).
    if( registerIndexDigits.size() == 0 )
    {
        getSink(context)->diagnose(semantic->registerName, Diagnostics::expectedARegisterIndex, registerClassName);
    }

    UInt index = 0;
    for(auto c : registerIndexDigits)
    {
        SLANG_ASSERT(isDigit(c));
        index = index * 10 + (c - '0');
    }


    UInt space = 0;
    if( auto registerSemantic = as<HLSLRegisterSemantic>(semantic) )
    {
        auto const& spaceName = registerSemantic->spaceName.Content;
        if(spaceName.size() != 0)
        {
            UnownedStringSlice spaceSpelling;
            UnownedStringSlice spaceDigits;
            splitNameAndIndex(spaceName, spaceSpelling, spaceDigits);

            if( kind == LayoutResourceKind::RegisterSpace )
            {
                getSink(context)->diagnose(registerSemantic->spaceName, Diagnostics::unexpectedSpecifierAfterSpace, spaceName);
            }
            else if( spaceSpelling != UnownedTerminatedStringSlice("space") )
            {
                getSink(context)->diagnose(registerSemantic->spaceName, Diagnostics::expectedSpace, spaceSpelling);
            }
            else if( spaceDigits.size() == 0 )
            {
                getSink(context)->diagnose(registerSemantic->spaceName, Diagnostics::expectedSpaceIndex);
            }
            else
            {
                for(auto c : spaceDigits)
                {
                    SLANG_ASSERT(isDigit(c));
                    space = space * 10 + (c - '0');
                }
            }
        }
    }

    // TODO: handle component mask part of things...
    if( semantic->componentMask.Content.size() != 0 )
    {
        getSink(context)->diagnose(semantic->componentMask, Diagnostics::componentMaskNotSupported);
    }

    info.kind = kind;
    info.index = (int) index;
    info.space = space;
    return info;
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
            *outVal = (UInt) strtoull(String(modifier->valToken.Content).getBuffer(), nullptr, 10);
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

    /// Determine how to lay out a global variable that might be a shader parameter.
    ///
    /// Returns `nullptr` if the declaration does not represent a shader parameter.
RefPtr<TypeLayout> getTypeLayoutForGlobalShaderParameter(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl,
    Type*                       type)
{
    auto layoutContext = context->layoutContext;
    auto rules = layoutContext.getRulesFamily();

    if(varDecl->HasModifier<ShaderRecordAttribute>() && as<ConstantBufferType>(type))
    {
        return createTypeLayout(
            layoutContext.with(rules->getShaderRecordConstantBufferRules()),
            type);
    }


    // We want to check for a constant-buffer type with a `push_constant` layout
    // qualifier before we move on to anything else.
    if( varDecl->HasModifier<PushConstantAttribute>() && as<ConstantBufferType>(type) )
    {
        return createTypeLayout(
            layoutContext.with(rules->getPushConstantBufferRules()),
            type);
    }

    // TODO: The cases below for detecting globals that aren't actually
    // shader parameters should be redundant now that the semantic
    // checking logic is responsible for populating the list of
    // parameters on a `Program`. We should be able to clean up
    // the code by removing these two cases, and the related null
    // pointer checks in the code that calls this.

    // HLSL `static` modifier indicates "thread local"
    if(varDecl->HasModifier<HLSLStaticModifier>())
        return nullptr;

    // HLSL `groupshared` modifier indicates "thread-group local"
    if(varDecl->HasModifier<HLSLGroupSharedModifier>())
        return nullptr;

    // TODO(tfoley): there may be other cases that we need to handle here

    // An "ordinary" global variable is implicitly a uniform
    // shader parameter.
    return createTypeLayout(
        layoutContext.with(rules->getConstantBufferRules()),
        type);
}

//

struct EntryPointParameterState
{
    String*                             optSemanticName = nullptr;
    int*                                ioSemanticIndex = nullptr;
    EntryPointParameterDirectionMask    directionMask;
    int                                 semanticSlotCount;
    Stage                               stage = Stage::Unknown;
    bool                                isSampleRate = false;
    SourceLoc                           loc;
};


static RefPtr<TypeLayout> processEntryPointVaryingParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout);

static RefPtr<VarLayout> _createVarLayout(
    TypeLayout*             typeLayout,
    DeclRef<VarDeclBase>    varDeclRef)
{
    RefPtr<VarLayout> varLayout = new VarLayout();
    varLayout->typeLayout = typeLayout;
    varLayout->varDecl = varDeclRef;

    if(auto pendingDataTypeLayout = typeLayout->pendingDataTypeLayout)
    {
        RefPtr<VarLayout> pendingVarLayout = new VarLayout();
        pendingVarLayout->typeLayout = pendingDataTypeLayout;
        varLayout->pendingVarLayout = pendingVarLayout;
    }

    return varLayout;
}

// Collect a single declaration into our set of parameters
static void collectGlobalScopeParameter(
    ParameterBindingContext*    context,
    ShaderParamInfo const&      shaderParamInfo,
    SubstitutionSet             globalGenericSubst)
{
    auto varDeclRef = shaderParamInfo.paramDeclRef;

    // We apply any substitutions for global generic parameters here.
    auto type = GetType(varDeclRef)->Substitute(globalGenericSubst).as<Type>();

    // We use a single operation to both check whether the
    // variable represents a shader parameter, and to compute
    // the layout for that parameter's type.
    auto typeLayout = getTypeLayoutForGlobalShaderParameter(
        context,
        varDeclRef.getDecl(),
        type);

    // If we did not find appropriate layout rules, then it
    // must mean that this global variable is *not* a shader
    // parameter.
    if(!typeLayout)
        return;

    // Now create a variable layout that we can use
    RefPtr<VarLayout> varLayout = _createVarLayout(typeLayout, varDeclRef);

    // The logic in `check.cpp` that created the `ShaderParamInfo`
    // will have identified any cases where there might be multiple
    // global variables that logically represent the same shader parameter.
    //
    // We will track the same basic information during layout using
    // the `ParameterInfo` type.
    //
    // TODO: `ParameterInfo` should probably become `LayoutParamInfo`.
    //
    ParameterInfo* parameterInfo = new ParameterInfo();
    context->shared->parameters.add(parameterInfo);

    // Add the created var layout to the parameter information structure,
    // so that we can update it as we proceed with parameter binding.
    //
    parameterInfo->varLayout = varLayout;
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
static VarLayout* markSpaceUsed(
    ParameterBindingContext*    context,
    VarLayout*                  varLayout,
    UInt                        space)
{
    return context->shared->usedSpaces.Add(varLayout, space, space+1);
}

static UInt allocateUnusedSpaces(
    ParameterBindingContext*    context,
    UInt                        count)
{
    return context->shared->usedSpaces.Allocate(nullptr, count);
}

static bool shouldDisableDiagnostic(
    Decl*                   decl,
    DiagnosticInfo const&   diagnosticInfo)
{
    for( auto dd = decl; dd; dd = dd->ParentDecl )
    {
        for( auto modifier : dd->modifiers )
        {
            auto allowAttr = as<AllowAttribute>(modifier);
            if(!allowAttr)
                continue;

            if(allowAttr->diagnostic == &diagnosticInfo)
                return true;
        }
    }
    return false;
}

static void addExplicitParameterBinding(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    VarDeclBase*                varDecl,
    LayoutSemanticInfo const&   semanticInfo,
    LayoutSize                  count)
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
        }

        // TODO(tfoley): `register` semantics can technically be
        // profile-specific (not sure if anybody uses that)...
    }
    else
    {
        bindingInfo.count = count;
        bindingInfo.index = semanticInfo.index;
        bindingInfo.space = semanticInfo.space;

        VarLayout* overlappedVarLayout = nullptr;
        if( kind == LayoutResourceKind::RegisterSpace )
        {
            // Parameter is being bound to an entire space, so we
            // need to mark the given space as used and report
            // an error if another parameter was already allocated
            // there.
            //
            overlappedVarLayout = markSpaceUsed(context, parameterInfo->varLayout, semanticInfo.index);
        }
        else
        {
            auto usedRangeSet = findUsedRangeSetForSpace(context, semanticInfo.space);

            // Record that the particular binding space was
            // used by an explicit binding, so that we don't
            // claim it for auto-generated bindings that
            // need to grab a full space
            markSpaceUsed(context, parameterInfo->varLayout, semanticInfo.space);

            overlappedVarLayout = usedRangeSet->usedResourceRanges[(int)semanticInfo.kind].Add(
                parameterInfo->varLayout,
                semanticInfo.index,
                semanticInfo.index + count);
        }

        if (overlappedVarLayout)
        {
            auto paramA = parameterInfo->varLayout->varDecl.getDecl();
            auto paramB = overlappedVarLayout->varDecl.getDecl();

            auto& diagnosticInfo = Diagnostics::parameterBindingsOverlap;

            // If *both* of the shader parameters declarations agree
            // that overlapping bindings should be allowed, then we
            // will not emit a diagnostic. Otherwise, we will warn
            // the user because such overlapping bindings are likely
            // to indicate a programming error.
            //
            if(shouldDisableDiagnostic(paramA, diagnosticInfo)
                && shouldDisableDiagnostic(paramB, diagnosticInfo))
            {
            }
            else
            {
                getSink(context)->diagnose(paramA, diagnosticInfo,
                    getReflectionName(paramA),
                    getReflectionName(paramB));

                getSink(context)->diagnose(paramB, Diagnostics::seeDeclarationOf, getReflectionName(paramB));
            }
        }
    }
}

static void addExplicitParameterBindings_HLSL(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    RefPtr<VarLayout>           varLayout)
{
    // We only want to apply D3D `register` modifiers when compiling for
    // D3D targets.
    //
    // TODO: Nominally, the `register` keyword allows for a shader
    // profile to be specified, so that a given binding only
    // applies for a specific profile:
    //
    //      https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-variable-register
    //
    // We might want to consider supporting that syntax in the
    // long run, in order to handle bindings for multiple targets
    // in a more consistent fashion (whereas using `register` for D3D
    // and `[[vk::binding(...)]]` for Vulkan creates a lot of
    // visual noise).
    //
    // For now we do the filtering on target in a very direct fashion:
    //
    if(!isD3DTarget(context->getTargetRequest()))
        return;

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
        LayoutSize count = 0;
        if (typeRes)
        {
            count = typeRes->count;
        }
        else
        {
            // TODO: warning here!
        }

        addExplicitParameterBinding(context, parameterInfo, varDecl, semanticInfo, count);
    }
}

static void maybeDiagnoseMissingVulkanLayoutModifier(
    ParameterBindingContext*    context,
    DeclRef<VarDeclBase> const& varDecl)
{
    // If the user didn't specify a `binding` (and optional `set`) for Vulkan,
    // but they *did* specify a `register` for D3D, then that is probably an
    // oversight on their part.
    if( auto registerModifier = varDecl.getDecl()->FindModifier<HLSLRegisterSemantic>() )
    {
        getSink(context)->diagnose(registerModifier, Diagnostics::registerModifierButNoVulkanLayout, varDecl.GetName());
    }
}

static void addExplicitParameterBindings_GLSL(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    RefPtr<VarLayout>           varLayout)
{

    // We only want to apply GLSL-style layout modifers
    // when compiling for a Khronos-related target.
    //
    // TODO: This should have some finer granularity
    // so that we are able to distinguish between
    // Vulkan and OpenGL as targets.
    //
    if(!isKhronosTarget(context->getTargetRequest()))
        return;

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
        auto attr = varDecl.getDecl()->FindModifier<GLSLBindingAttribute>();
        if (!attr)
        {
            maybeDiagnoseMissingVulkanLayoutModifier(context, varDecl);
            return;
        }
        semanticInfo.index = attr->binding;
        semanticInfo.space = attr->set;
    }
    else if( (resInfo = typeLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace)) != nullptr )
    {
        // Try to find `set`
        auto attr = varDecl.getDecl()->FindModifier<GLSLBindingAttribute>();
        if (!attr)
        {
            maybeDiagnoseMissingVulkanLayoutModifier(context, varDecl);
            return;
        }
        if( attr->binding != 0)
        {
            getSink(context)->diagnose(attr, Diagnostics::wholeSpaceParameterRequiresZeroBinding, varDecl.GetName(), attr->binding);
        }
        semanticInfo.index = attr->set;
        semanticInfo.space = 0;
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

    addExplicitParameterBinding(context, parameterInfo, varDecl, semanticInfo, count);
}

// Given a single parameter, collect whatever information we have on
// how it has been explicitly bound, which may come from multiple declarations
void generateParameterBindings(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo)
{
    // There must have been a declaration for the parameter.
    SLANG_RELEASE_ASSERT(parameterInfo->varLayout);

    // We will look for explicit binding information on the declaration.
    auto varLayout = parameterInfo->varLayout;

    // Handle HLSL `register` and `packoffset` modifiers
    addExplicitParameterBindings_HLSL(context, parameterInfo, varLayout);


    // Handle GLSL `layout` modifiers and `[vk::...]` attributes.
    //
    // TODO: We should deprecate the support for `layout` and then rename
    // these `_HLSL` and `_GLSL` functions to be more explicit and clear
    // about the fact that they are specific to the *target* and not to
    // the *source language* (as they were at one point).
    //
    addExplicitParameterBindings_GLSL(context, parameterInfo, varLayout);
}

// Generate the binding information for a shader parameter.
static void completeBindingsForParameterImpl(
    ParameterBindingContext*    context,
    RefPtr<VarLayout>           firstVarLayout,
    ParameterBindingInfo        bindingInfos[kLayoutResourceKindCount],
    RefPtr<ParameterInfo>       parameterInfo)
{
    // For any resource kind used by the parameter
    // we need to update its layout information
    // to include a binding for that resource kind.
    //
    auto firstTypeLayout = firstVarLayout->typeLayout;

    // We need to deal with allocation of full register spaces first,
    // since that is the most complicated bit of logic.
    //
    // We will compute how many full register spaces the parameter
    // needs to allocate, across all the kinds of resources it
    // consumes, so that we can allocate a contiguous range of
    // spaces.
    //
    UInt spacesToAllocateCount = 0;
    for(auto typeRes : firstTypeLayout->resourceInfos)
    {
        auto kind = typeRes.kind;

        // We want to ignore resource kinds for which the user
        // has specified an explicit binding, since those won't
        // go into our contiguously allocated range.
        //
        auto& bindingInfo = bindingInfos[(int)kind];
        if( bindingInfo.count != 0 )
        {
            continue;
        }

        // Now we inspect the kind of resource to figure out
        // its space requirements:
        //
        switch( kind )
        {
        default:
            // An unbounded-size array will need its own space.
            //
            if( typeRes.count.isInfinite() )
            {
                spacesToAllocateCount++;
            }
            break;

        case LayoutResourceKind::RegisterSpace:
            // If the parameter consumes any full spaces (e.g., it
            // is a `struct` type with one or more unbounded arrays
            // for fields), then we will include those spaces in
            // our allocaiton.
            //
            // We assume/require here that we never end up needing
            // an unbounded number of spaces.
            // TODO: we should enforce that somewhere with an error.
            //
            spacesToAllocateCount += typeRes.count.getFiniteValue();
            break;

        case LayoutResourceKind::Uniform:
            // We want to ignore uniform data for this calculation,
            // since any uniform data in top-level shader parameters
            // needs to go into a global constant buffer.
            //
            break;

        case LayoutResourceKind::GenericResource:
            // This is more of a marker case, and shouldn't ever
            // need a space allocated to it.
            break;
        }
    }

    // If we compute that the parameter needs some number of full
    // spaces allocated to it, then we will go ahead and allocate
    // contiguous spaces here.
    //
    UInt firstAllocatedSpace = 0;
    if(spacesToAllocateCount)
    {
        firstAllocatedSpace = allocateUnusedSpaces(context, spacesToAllocateCount);
    }

    // We'll then dole the allocated spaces (if any) out to the resource
    // categories that need them.
    //
    UInt currentAllocatedSpace = firstAllocatedSpace;

    for(auto typeRes : firstTypeLayout->resourceInfos)
    {
        // Did we already apply some explicit binding information
        // for this resource kind?
        auto kind = typeRes.kind;
        auto& bindingInfo = bindingInfos[(int)kind];
        if( bindingInfo.count != 0 )
        {
            // If things have already been bound, our work is done.
            //
            // TODO: it would be good to handle the case where a
            // binding specified a space, but not an offset/index
            // for some kind of resource.
            //
            continue;
        }

        auto count = typeRes.count;

        // Certain resource kinds require special handling.
        //
        // Note: This `switch` statement should have a `case` for
        // all of the special cases above that affect the computation of
        // `spacesToAllocateCount`.
        //
        switch( kind )
        {
        case LayoutResourceKind::RegisterSpace:
            {
                // The parameter's type needs to consume some number of whole
                // register spaces, and we have already allocated a contiguous
                // range of spaces above.
                //
                // As always, we can't handle the case of a parameter that needs
                // an infinite number of spaces.
                //
                SLANG_ASSERT(count.isFinite());
                bindingInfo.count = count;

                // We will use the spaces we've allocated, and bump
                // the variable tracking the "current" space by
                // the number of spaces consumed.
                //
                bindingInfo.index = currentAllocatedSpace;
                currentAllocatedSpace += count.getFiniteValue();

                // TODO: what should we store as the "space" for
                // an allocation of register spaces? Either zero
                // or `space` makes sense, but it isn't clear
                // which is a better choice.
                bindingInfo.space = 0;

                continue;
            }

        case LayoutResourceKind::GenericResource:
            {
                // `GenericResource` is somewhat confusingly named,
                // but simply indicates that the type of this parameter
                // in some way depends on a generic parameter that has
                // not been bound to a concrete value, so that asking
                // specific questions about its resource usage isn't
                // really possible.
                //
                bindingInfo.space = 0;
                bindingInfo.count = 1;
                bindingInfo.index = 0;
                continue;
            }

        case LayoutResourceKind::Uniform:
            // TODO: we don't currently handle global-scope uniform parameters.
            break;
        }

        // At this point, we know the parameter consumes some resource
        // (e.g., D3D `t` registers or Vulkan `binding`s), and the user
        // didn't specify an explicit binding, so we will have to
        // assign one for them.
        //
        // If we are consuming an infinite amount of the given resource
        // (e.g., an unbounded array of `Texure2D` requires an infinite
        // number of `t` regisers in D3D), then we will go ahead
        // and assign a full space:
        //
        if( count.isInfinite() )
        {
            bindingInfo.count = count;
            bindingInfo.index = 0;
            bindingInfo.space = currentAllocatedSpace;
            currentAllocatedSpace++;
        }
        else
        {
            // If we have a finite amount of resources, then
            // we will go ahead and allocate from the "default"
            // space.

            UInt space = context->shared->defaultSpace;
            RefPtr<UsedRangeSet> usedRangeSet = findUsedRangeSetForSpace(context, space);

            bindingInfo.count = count;
            bindingInfo.index = usedRangeSet->usedResourceRanges[(int)kind].Allocate(firstVarLayout, count.getFiniteValue());
            bindingInfo.space = space;
        }
    }
}

static void applyBindingInfoToParameter(
    RefPtr<VarLayout>       varLayout,
    ParameterBindingInfo    bindingInfos[kLayoutResourceKindCount])
{
    for(auto k = 0; k < kLayoutResourceKindCount; ++k)
    {
        auto kind = LayoutResourceKind(k);
        auto& bindingInfo = bindingInfos[k];

        // skip resources we aren't consuming
        if(bindingInfo.count == 0)
            continue;

        // Add a record to the variable layout
        auto varRes = varLayout->AddResourceInfo(kind);
        varRes->space = (int) bindingInfo.space;
        varRes->index = (int) bindingInfo.index;
    }
}

// Generate the binding information for a shader parameter.
static void completeBindingsForParameter(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo)
{
    auto varLayout = parameterInfo->varLayout;
    SLANG_RELEASE_ASSERT(varLayout);

    completeBindingsForParameterImpl(
        context,
        varLayout,
        parameterInfo->bindingInfo,
        parameterInfo);

    // At this point we should have explicit binding locations chosen for
    // all the relevant resource kinds, so we can apply these to the
    // declarations:

    applyBindingInfoToParameter(varLayout, parameterInfo->bindingInfo);
}

static void completeBindingsForParameter(
    ParameterBindingContext*    context,
    RefPtr<VarLayout>           varLayout)
{
    ParameterBindingInfo bindingInfos[kLayoutResourceKindCount];
    completeBindingsForParameterImpl(
        context,
        varLayout,
        bindingInfos,
        nullptr);
    applyBindingInfoToParameter(varLayout, bindingInfos);
}

    /// Allocate binding location for any "pending" data in a shader parameter.
    ///
    /// When a parameter contains interface-type fields (recursively), we might
    /// not have included them in the base layout for the parameter, and instead
    /// need to allocate space for them after all other shader parameters have
    /// been laid out.
    ///
    /// This function should be called on the `pendingVarLayout` field of an
    /// existing `VarLayout` to ensure that its pending data has been properly
    /// assigned storage. It handles the case where the `pendingVarLayout`
    /// field is null.
    ///
static void _allocateBindingsForPendingData(
    ParameterBindingContext*    context,
    RefPtr<VarLayout>           pendingVarLayout)
{
    if(!pendingVarLayout) return;

    completeBindingsForParameter(context, pendingVarLayout);
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
    UInt length = composedName.size();
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
        String stringComposedName(composedName);

        info.name = stringComposedName.subString(0, indexLoc);
        info.index = strtol(stringComposedName.begin() + indexLoc, nullptr, 10);
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
    String sn = semanticName.toLower();

    RefPtr<TypeLayout> typeLayout;
    if (sn.startsWith("sv_")
        || sn.startsWith("nv_"))
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
                //
                typeLayout = getSimpleVaryingParameterTypeLayout(
                    context->layoutContext,
                    type,
                    kEntryPointParameterDirection_Output);
            }
        }

        if (state.directionMask & kEntryPointParameterDirection_Input)
        {
            if (sn == "sv_sampleindex")
            {
                state.isSampleRate = true;
            }
        }

        if( !typeLayout )
        {
            // If we didn't compute a special-case layout for the
            // system-value parameter (e.g., because it was an
            // `SV_Target` output), then create a default layout
            // that consumes no input/output varying slots.
            // (since system parameters are distinct from
            // user-defined parameters for layout purposes)
            //
            typeLayout = getSimpleVaryingParameterTypeLayout(
                context->layoutContext,
                type,
                0);
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
        // In this case we have a user-defined semantic, which means
        // an ordinary input and/or output varying parameter.
        //
        typeLayout = getSimpleVaryingParameterTypeLayout(
                context->layoutContext,
                type,
                state.directionMask);
    }

    if (state.isSampleRate
        && (state.directionMask & kEntryPointParameterDirection_Input)
        && (context->stage == Stage::Fragment))
    {
        if (auto entryPointLayout = context->entryPointLayout)
        {
            entryPointLayout->flags |= EntryPointLayout::Flag::usesAnySampleRateInput;
        }
    }

    *state.ioSemanticIndex += state.semanticSlotCount;
    typeLayout->type = type;

    return typeLayout;
}

static RefPtr<TypeLayout> processEntryPointVaryingParameterDecl(
    ParameterBindingContext*        context,
    Decl*                           decl,
    RefPtr<Type>                    type,
    EntryPointParameterState const& inState,
    RefPtr<VarLayout>               varLayout)
{
    SimpleSemanticInfo semanticInfo;
    int semanticIndex = 0;

    EntryPointParameterState state = inState;

    // If there is no explicit semantic already in effect, *and* we find an explicit
    // semantic on the associated declaration, then we'll use it.
    if( !state.optSemanticName )
    {
        if( auto semantic = decl->FindModifier<HLSLSimpleSemantic>() )
        {
            semanticInfo = decomposeSimpleSemantic(semantic);
            semanticIndex = semanticInfo.index;

            state.optSemanticName = &semanticInfo.name;
            state.ioSemanticIndex = &semanticIndex;
        }
    }

    if (decl)
    {
        if (decl->FindModifier<HLSLSampleModifier>())
        {
            state.isSampleRate = true;
        }
    }

    // Default case: either there was an explicit semantic in effect already,
    // *or* we couldn't find an explicit semantic to apply on the given
    // declaration, so we will just recursive with whatever we have at
    // the moment.
    return processEntryPointVaryingParameter(context, type, state, varLayout);
}

static RefPtr<TypeLayout> processEntryPointVaryingParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>                    type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout)
{
    // Make sure to associate a stage with every
    // varying parameter (including sub-fields of
    // `struct`-type parameters), since downstream
    // code generation will need to look at the
    // stage (possibly on individual leaf fields) to
    // decide when to emit things like the `flat`
    // interpolation modifier.
    //
    if( varLayout )
    {
        varLayout->stage = state.stage;
    }

    // The default handling of varying parameters should not apply
    // to geometry shader output streams; they have their own special rules.
    if( auto gsStreamType = as<HLSLStreamOutputType>(type) )
    {
        //

        auto elementType = gsStreamType->getElementType();

        int semanticIndex = 0;

        EntryPointParameterState elementState;
        elementState.directionMask = kEntryPointParameterDirection_Output;
        elementState.ioSemanticIndex = &semanticIndex;
        elementState.isSampleRate = false;
        elementState.optSemanticName = nullptr;
        elementState.semanticSlotCount = 0;
        elementState.stage = state.stage;
        elementState.loc = state.loc;

        auto elementTypeLayout = processEntryPointVaryingParameter(context, elementType, elementState, nullptr);

        RefPtr<StreamOutputTypeLayout> typeLayout = new StreamOutputTypeLayout();
        typeLayout->type = type;
        typeLayout->rules = elementTypeLayout->rules;
        typeLayout->elementTypeLayout = elementTypeLayout;

        for(auto resInfo : elementTypeLayout->resourceInfos)
            typeLayout->addResourceUsage(resInfo);

        return typeLayout;
    }

    // Raytracing shaders have a slightly different interpretation of their
    // "varying" input/output parameters, since they don't have the same
    // idea of previous/next stage as the rasterization shader types.
    //
    if( state.directionMask & kEntryPointParameterDirection_Output )
    {
        // Note: we are silently treating `out` parameters as if they
        // were `in out` for this test, under the assumption that
        // an `out` parameter represents a write-only payload.

        switch(state.stage)
        {
        default:
            // Not a raytracing shader.
            break;

        case Stage::Intersection:
        case Stage::RayGeneration:
            // Don't expect this case to have any `in out` parameters.
            getSink(context)->diagnose(state.loc, Diagnostics::dontExpectOutParametersForStage, getStageName(state.stage));
            break;

        case Stage::AnyHit:
        case Stage::ClosestHit:
        case Stage::Miss:
            // `in out` or `out` parameter is payload
            return createTypeLayout(context->layoutContext.with(
                context->getRulesFamily()->getRayPayloadParameterRules()),
                type);

        case Stage::Callable:
            // `in out` or `out` parameter is payload
            return createTypeLayout(context->layoutContext.with(
                context->getRulesFamily()->getCallablePayloadParameterRules()),
                type);

        }
    }
    else
    {
        switch(state.stage)
        {
        default:
            // Not a raytracing shader.
            break;

        case Stage::Intersection:
        case Stage::RayGeneration:
        case Stage::Miss:
        case Stage::Callable:
            // Don't expect this case to have any `in` parameters.
            //
            // TODO: For a miss or callable shader we could interpret
            // an `in` parameter as indicating a payload that the
            // programmer doesn't intend to write to.
            //
            getSink(context)->diagnose(state.loc, Diagnostics::dontExpectInParametersForStage, getStageName(state.stage));
            break;

        case Stage::AnyHit:
        case Stage::ClosestHit:
            // `in` parameter is hit attributes
            return createTypeLayout(context->layoutContext.with(
                context->getRulesFamily()->getHitAttributesParameterRules()),
                type);
        }
    }

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
        String sn = semanticName.toUpper();

        auto semanticIndex      = *state.ioSemanticIndex;

        varLayout->semanticName = sn;
        varLayout->semanticIndex = semanticIndex;
        varLayout->flags |= VarLayoutFlag::HasSemantic;
    }

    // Scalar and vector types are treated as outputs directly
    if(auto basicType = as<BasicExpressionType>(type))
    {
        return processSimpleEntryPointParameter(context, basicType, state, varLayout);
    }
    else if(auto vectorType = as<VectorExpressionType>(type))
    {
        return processSimpleEntryPointParameter(context, vectorType, state, varLayout);
    }
    // A matrix is processed as if it was an array of rows
    else if( auto matrixType = as<MatrixExpressionType>(type) )
    {
        auto rowCount = GetIntVal(matrixType->getRowCount());
        return processSimpleEntryPointParameter(context, matrixType, state, varLayout, (int) rowCount);
    }
    else if( auto arrayType = as<ArrayExpressionType>(type) )
    {
        // Note: Bad Things will happen if we have an array input
        // without a semantic already being enforced.
        
        auto elementCount = (UInt) GetIntVal(arrayType->ArrayLength);

        // We use the first element to derive the layout for the element type
        auto elementTypeLayout = processEntryPointVaryingParameter(context, arrayType->baseType, state, varLayout);

        // We still walk over subsequent elements to make sure they consume resources
        // as needed
        for( UInt ii = 1; ii < elementCount; ++ii )
        {
            processEntryPointVaryingParameter(context, arrayType->baseType, state, nullptr);
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
    else if (auto textureType = as<TextureType>(type)) { return nullptr;  }
    else if(auto samplerStateType = as<SamplerStateType>(type)) { return nullptr;  }
    else if(auto constantBufferType = as<ConstantBufferType>(type)) { return nullptr;  }
    // Catch declaration-reference types late in the sequence, since
    // otherwise they will include all of the above cases...
    else if( auto declRefType = as<DeclRefType>(type) )
    {
        auto declRef = declRefType->declRef;

        if (auto structDeclRef = declRef.as<StructDecl>())
        {
            RefPtr<StructTypeLayout> structLayout = new StructTypeLayout();
            structLayout->type = type;

            // Need to recursively walk the fields of the structure now...
            for( auto field : GetFields(structDeclRef) )
            {
                RefPtr<VarLayout> fieldVarLayout = new VarLayout();
                fieldVarLayout->varDecl = field;

                auto fieldTypeLayout = processEntryPointVaryingParameterDecl(
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
                        fieldVarLayout->findOrAddResourceInfo(rr.kind)->index = structRes->count.getFiniteValue();
                        structRes->count += rr.count;
                    }
                }

                structLayout->fields.add(fieldVarLayout);
                structLayout->mapVarToLayout.Add(field.getDecl(), fieldVarLayout);
            }

            return structLayout;
        }
        else if (auto globalGenericParamDecl = declRef.as<GlobalGenericParamDecl>())
        {
            auto& layoutContext = context->layoutContext;

            if( auto concreteType = findGlobalGenericSpecializationArg(
                layoutContext,
                globalGenericParamDecl) )
            {
                // If we know what concrete type has been used to specialize
                // the global generic type parameter, then we should use
                // the concrete type instead.
                //
                // Note: it should be illegal for the user to use a generic
                // type parameter in a varying parameter list without giving
                // it an explicit user-defined semantic. Otherwise, it would be possible
                // that the concrete type that gets plugged in is a user-defined
                // `struct` that uses some `SV_` semantics in its definition,
                // so that any static information about what system values
                // the entry point uses would be incorrect.
                //
                return processEntryPointVaryingParameter(context, concreteType, state, varLayout);
            }
            else
            {
                // If we don't know a concrete type, then we aren't generating final
                // code, so the reflection information should show the generic
                // type parameter.
                //
                // We don't make any attempt to assign varying parameter resources
                // to the generic type, since we can't know how many "slots"
                // of varying input/output it would consume.
                //
                return createTypeLayoutForGlobalGenericTypeParam(layoutContext, type, globalGenericParamDecl);
            }
        }
        else if (auto associatedTypeParam = declRef.as<AssocTypeDecl>())
        {
            RefPtr<TypeLayout> assocTypeLayout = new TypeLayout();
            assocTypeLayout->type = type;
            return assocTypeLayout;
        }
        else
        {
            SLANG_UNEXPECTED("unhandled type kind");
        }
    }
    // If we ran into an error in checking the user's code, then skip this parameter
    else if( auto errorType = as<ErrorType>(type) )
    {
        return nullptr;
    }

    SLANG_UNEXPECTED("unhandled type kind");
    UNREACHABLE_RETURN(nullptr);
}

    /// Compute the type layout for a parameter declared directly on an entry point.
static RefPtr<TypeLayout> computeEntryPointParameterTypeLayout(
    ParameterBindingContext*        context,
    DeclRef<VarDeclBase>            paramDeclRef,
    RefPtr<VarLayout>               paramVarLayout,
    EntryPointParameterState&       state)
{
    auto paramType = GetType(paramDeclRef);
    SLANG_ASSERT(paramType);

    if( paramDeclRef.getDecl()->HasModifier<HLSLUniformModifier>() )
    {
        // An entry-point parameter that is explicitly marked `uniform` represents
        // a uniform shader parameter passed via the implicitly-defined
        // constant buffer (e.g., the `$Params` constant buffer seen in fxc/dxc output).
        //
        return createTypeLayout(
            context->layoutContext.with(context->getRulesFamily()->getConstantBufferRules()),
            paramType);
    }
    else
    {
        // The default case is a varying shader parameter, which could be used for
        // input, output, or both.
        //
        // The varying case needs to not only compute a layout, but also assocaite
        // "semantic" strings/indices with the varying parameters by recursively
        // walking their structure.

        state.directionMask = 0;

        // If it appears to be an input, process it as such.
        if( paramDeclRef.getDecl()->HasModifier<InModifier>()
            || paramDeclRef.getDecl()->HasModifier<InOutModifier>()
            || !paramDeclRef.getDecl()->HasModifier<OutModifier>() )
        {
            state.directionMask |= kEntryPointParameterDirection_Input;
        }

        // If it appears to be an output, process it as such.
        if(paramDeclRef.getDecl()->HasModifier<OutModifier>()
            || paramDeclRef.getDecl()->HasModifier<InOutModifier>())
        {
            state.directionMask |= kEntryPointParameterDirection_Output;
        }

        return processEntryPointVaryingParameterDecl(
            context,
            paramDeclRef.getDecl(),
            paramType,
            state,
            paramVarLayout);
    }
}

// There are multiple places where we need to compute the layout
// for a "scope" such as the global scope or an entry point.
// The `ScopeLayoutBuilder` encapsulates the logic around:
//
// * Doing layout for the ordinary/uniform fields, which involves
//   using the `struct` layout rules for constant buffers on
//   the target.
//
// * Creating a final type/var layout that reflects whether the
//   scope needs a constant buffer to be allocated to it.
//
struct ScopeLayoutBuilder
{
    ParameterBindingContext*    m_context = nullptr;
    LayoutRulesImpl*            m_rules = nullptr;
    RefPtr<StructTypeLayout>    m_structLayout;
    UniformLayoutInfo           m_structLayoutInfo;

    // We need to compute a layout for any "pending" data inside
    // of the parameters being added to the scope, to facilitate
    // later allocating space for all the pending parameters after
    // the primary shader parameters.
    //
    StructTypeLayoutBuilder     m_pendingDataTypeLayoutBuilder;

    void beginLayout(
        ParameterBindingContext* context)
    {
        m_context = context;
        m_rules = context->getRulesFamily()->getConstantBufferRules();
        m_structLayout = new StructTypeLayout();
        m_structLayout->rules = m_rules;

        m_structLayoutInfo = m_rules->BeginStructLayout();
    }

    void _addParameter(
        RefPtr<VarLayout>   varLayout)
    {
        // Does the parameter have any uniform data?
        auto layoutInfo = varLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform);
        LayoutSize uniformSize = layoutInfo ? layoutInfo->count : 0;
        if( uniformSize != 0 )
        {
            // Make sure uniform fields get laid out properly...

            UniformLayoutInfo fieldInfo(
                uniformSize,
                varLayout->typeLayout->uniformAlignment);

            LayoutSize uniformOffset = m_rules->AddStructField(
                &m_structLayoutInfo,
                fieldInfo);

            varLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset.getFiniteValue();
        }

        m_structLayout->fields.add(varLayout);

        m_structLayout->mapVarToLayout.Add(varLayout->varDecl.getDecl(), varLayout);
    }

    void addParameter(
        RefPtr<VarLayout> varLayout)
    {
        _addParameter(varLayout);

        // Any "pending" items on a field type become "pending" items
        // on the overall `struct` type layout.
        //
        // TODO: This logic ends up duplicated between here and the main
        // `struct` layout logic in `type-layout.cpp`. If this gets any
        // more complicated we should see if there is a way to share it.
        //
        if( auto fieldPendingDataTypeLayout = varLayout->typeLayout->pendingDataTypeLayout )
        {
            m_pendingDataTypeLayoutBuilder.beginLayoutIfNeeded(nullptr, m_rules);
            auto fieldPendingDataVarLayout = m_pendingDataTypeLayoutBuilder.addField(varLayout->varDecl, fieldPendingDataTypeLayout);

            m_structLayout->pendingDataTypeLayout = m_pendingDataTypeLayoutBuilder.getTypeLayout();

            varLayout->pendingVarLayout = fieldPendingDataVarLayout;
        }
    }

    void addParameter(
        ParameterInfo* parameterInfo)
    {
        auto varLayout = parameterInfo->varLayout;
        SLANG_RELEASE_ASSERT(varLayout);

        _addParameter(varLayout);

        // Global parameters will have their non-orindary/uniform
        // pending data handled by the main parameter binding
        // logic, but we still need to construct a layout
        // that includes any pending data.
        //
        if(auto fieldPendingVarLayout = varLayout->pendingVarLayout)
        {
            auto fieldPendingTypeLayout = fieldPendingVarLayout->typeLayout;

            m_pendingDataTypeLayoutBuilder.beginLayoutIfNeeded(nullptr, m_rules);
            m_structLayout->pendingDataTypeLayout = m_pendingDataTypeLayoutBuilder.getTypeLayout();

            auto fieldUniformLayoutInfo = fieldPendingTypeLayout->FindResourceInfo(LayoutResourceKind::Uniform);
            LayoutSize fieldUniformSize = fieldUniformLayoutInfo ? fieldUniformLayoutInfo->count : 0;
            if( fieldUniformSize != 0 )
            {
                // Make sure uniform fields get laid out properly...

                UniformLayoutInfo fieldInfo(
                    fieldUniformSize,
                    fieldPendingTypeLayout->uniformAlignment);

                LayoutSize uniformOffset = m_rules->AddStructField(
                    m_pendingDataTypeLayoutBuilder.getStructLayoutInfo(),
                    fieldInfo);

                fieldPendingVarLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset.getFiniteValue();
            }

            m_pendingDataTypeLayoutBuilder.getTypeLayout()->fields.add(fieldPendingVarLayout);
        }

    }

        // Add a "simple" parameter that cannot have any user-defined
        // register or binding modifiers, so that its layout computation
        // can be simplified greatly.
        //
    void addSimpleParameter(
        RefPtr<VarLayout> varLayout)
    {
        // The main `addParameter` logic will deal with any ordinary/uniform data,
        // and with the "pending" part of the layout.
        //
        addParameter(varLayout);

        // That leaves us to deal with the resource usage that isn't
        // handled by `addParameter`.
        //
        auto paramTypeLayout = varLayout->getTypeLayout();
        for (auto paramTypeResInfo : paramTypeLayout->resourceInfos)
        {
            // We need to skip ordinary/uniform data because it was
            // handled by `addParameter`.
            //
            if(paramTypeResInfo.kind == LayoutResourceKind::Uniform)
                continue;

            // Whatever resources the parameter uses, we need to
            // assign the parameter's location/register/binding offset to
            // be the sum of everything added so far.
            //
            auto scopeResInfo = m_structLayout->findOrAddResourceInfo(paramTypeResInfo.kind);
            varLayout->findOrAddResourceInfo(paramTypeResInfo.kind)->index = scopeResInfo->count.getFiniteValue();

            // We then need to add the resources consumed by the parameter
            // to those consumed by the scope.
            //
            scopeResInfo->count += paramTypeResInfo.count;
        }
    }

    RefPtr<VarLayout> endLayout()
    {
        // Finish computing the layout for the ordindary data (if any).
        //
        m_rules->EndStructLayout(&m_structLayoutInfo);
        m_pendingDataTypeLayoutBuilder.endLayout();

        // Copy the final layout information computed for ordinary data
        // over to the struct type layout for the scope.
        //
        m_structLayout->addResourceUsage(LayoutResourceKind::Uniform, m_structLayoutInfo.size);
        m_structLayout->uniformAlignment = m_structLayout->uniformAlignment;

        RefPtr<TypeLayout> scopeTypeLayout = m_structLayout;

        // If a constant buffer is needed (because there is a non-zero
        // amount of uniform data), then we need to wrap up the layout
        // to reflect the constant buffer that will be generated.
        //
        scopeTypeLayout = createConstantBufferTypeLayoutIfNeeded(
            m_context->layoutContext,
            scopeTypeLayout);

        // We now have a bunch of layout information, which we should
        // record into a suitable object that represents the scope
        RefPtr<VarLayout> scopeVarLayout = new VarLayout();
        scopeVarLayout->typeLayout = scopeTypeLayout;

        if( auto pendingTypeLayout = scopeTypeLayout->pendingDataTypeLayout )
        {
            RefPtr<VarLayout> pendingVarLayout = new VarLayout();
            pendingVarLayout->typeLayout = pendingTypeLayout;
            scopeVarLayout->pendingVarLayout = pendingVarLayout;
        }

        return scopeVarLayout;
    }
};

    /// Helper routine to allocate a constant buffer binding if one is needed.
    ///
    /// This function primarily exists to encapsulate the logic for allocating
    /// the resources required for a constant buffer in the appropriate
    /// target-specific fashion.
    ///
static ParameterBindingAndKindInfo maybeAllocateConstantBufferBinding(
    ParameterBindingContext*    context,
    bool                        needConstantBuffer)
{
    if( !needConstantBuffer ) return ParameterBindingAndKindInfo();

    UInt space = context->shared->defaultSpace;
    auto usedRangeSet = findUsedRangeSetForSpace(context, space);

    auto layoutInfo = context->getRulesFamily()->getConstantBufferRules()->GetObjectLayout(
        ShaderParameterKind::ConstantBuffer);

    ParameterBindingAndKindInfo info;
    info.kind = layoutInfo.kind;
    info.count = layoutInfo.size;
    info.index = usedRangeSet->usedResourceRanges[(int)layoutInfo.kind].Allocate(nullptr, layoutInfo.size.getFiniteValue());
    info.space = space;
    return info;
}

    /// Remove resource usage from `typeLayout` that should only be stored per-entry-point.
    ///
    /// This is used when constructing the overall layout for an entry point, to make sure
    /// that certain kinds of resource usage from the entry point don't "leak" into
    /// the resource usage of the overall program.
    ///
static void removePerEntryPointParameterKinds(
    TypeLayout* typeLayout)
{
    typeLayout->removeResourceUsage(LayoutResourceKind::VaryingInput);
    typeLayout->removeResourceUsage(LayoutResourceKind::VaryingOutput);
    typeLayout->removeResourceUsage(LayoutResourceKind::ShaderRecord);
    typeLayout->removeResourceUsage(LayoutResourceKind::HitAttributes);
    typeLayout->removeResourceUsage(LayoutResourceKind::ExistentialObjectParam);
    typeLayout->removeResourceUsage(LayoutResourceKind::ExistentialTypeParam);
}

    /// Iterate over the parameters of an entry point to compute its requirements.
    ///
static RefPtr<EntryPointLayout> collectEntryPointParameters(
    ParameterBindingContext*                    context,
    EntryPoint*                                 entryPoint,
    EntryPoint::EntryPointSpecializationInfo*   specializationInfo)
{
    // We will take responsibility for creating and filling in
    // the `EntryPointLayout` object here.
    //
    RefPtr<EntryPointLayout> entryPointLayout = new EntryPointLayout();
    entryPointLayout->profile = entryPoint->getProfile();
    entryPointLayout->name = entryPoint->getName();

    // The entry point layout must be added to the output
    // program layout so that it can be accessed by reflection.
    //
    context->shared->programLayout->entryPoints.add(entryPointLayout);

    DeclRef<FuncDecl> entryPointFuncDeclRef = entryPoint->getFuncDeclRef();

    // HACK: We might have an `EntryPoint` that has been deserialized, in
    // which case we don't currently have access to its AST-level information,
    // and as a result we cannot collect parameter information from it.
    //
    if( !entryPointFuncDeclRef )
    {
        // TODO: figure out what fields we absolutely need to fill in.

        RefPtr<StructTypeLayout> paramsTypeLayout = new StructTypeLayout();

        RefPtr<VarLayout> paramsLayout = new VarLayout();
        paramsLayout->typeLayout = paramsTypeLayout;

        entryPointLayout->parametersLayout = paramsLayout;

        return entryPointLayout;
    }

    // If specialization was applied to the entry point, then the side-band
    // information that was generated will have a more specialized reference
    // to the entry point with generic parameters filled in. We should
    // use that version if it is available.
    //
    if(specializationInfo)
        entryPointFuncDeclRef = specializationInfo->specializedFuncDeclRef;

    auto entryPointType = DeclRefType::Create(context->getLinkage()->getSessionImpl(), entryPointFuncDeclRef);

    entryPointLayout->entryPoint = entryPointFuncDeclRef;

    // For the duration of our parameter collection work we will
    // establish this entry point as the current one in the context.
    //
    context->entryPointLayout = entryPointLayout;

    // We are going to iterate over the entry-point parameters,
    // and while we do so we will go ahead and perform layout/binding
    // assignment for two cases:
    //
    // First, the varying parameters of the entry point will have
    // their semantics and locations assigned, so we set up state
    // for tracking that layout.
    //
    int defaultSemanticIndex = 0;
    EntryPointParameterState state;
    state.ioSemanticIndex = &defaultSemanticIndex;
    state.optSemanticName = nullptr;
    state.semanticSlotCount = 0;
    state.stage = entryPoint->getStage();

    // Second, we will compute offsets for any "ordinary" data
    // in the parameter list (e.g., a `uniform float4x4 mvp` parameter),
    // which is what the `ScopeLayoutBuilder` is designed to help with.
    //
    ScopeLayoutBuilder scopeBuilder;
    scopeBuilder.beginLayout(context);
    auto paramsStructLayout = scopeBuilder.m_structLayout;
    paramsStructLayout->type = entryPointType;

    for( auto& shaderParamInfo : entryPoint->getShaderParams() )
    {
        auto paramDeclRef = shaderParamInfo.paramDeclRef;

        // Any generic specialization applied to the entry-point function
        // must also be applied to its parameters.
        paramDeclRef.substitutions = entryPointFuncDeclRef.substitutions;

        // When computing layout for an entry-point parameter,
        // we want to make sure that the layout context has access
        // to the existential type arguments (if any) that were
        // provided for the entry-point existential type parameters (if any).
        //
        if(specializationInfo)
        {
            auto& existentialSpecializationArgs = specializationInfo->existentialSpecializationArgs;
            auto genericSpecializationParamCount = entryPoint->getGenericSpecializationParamCount();

            context->layoutContext = context->layoutContext
                .withSpecializationArgs(
                    existentialSpecializationArgs.getBuffer(),
                    existentialSpecializationArgs.getCount())
                .withSpecializationArgsOffsetBy(
                    shaderParamInfo.firstSpecializationParamIndex - genericSpecializationParamCount);
        }

        // Any error messages we emit during the process should
        // refer to the location of this parameter.
        //
        state.loc = paramDeclRef.getLoc();

        // We are going to construct the variable layout for this
        // parameter *before* computing the type layout, because
        // the type layout computation is also determining the effective
        // semantic of the parameter, which needs to be stored
        // back onto the `VarLayout`.
        //
        RefPtr<VarLayout> paramVarLayout = new VarLayout();
        paramVarLayout->varDecl = paramDeclRef;
        paramVarLayout->stage = state.stage;

        auto paramTypeLayout = computeEntryPointParameterTypeLayout(
            context,
            paramDeclRef,
            paramVarLayout,
            state);
        paramVarLayout->typeLayout = paramTypeLayout;

        // We expect to always be able to compute a layout for
        // entry-point parameters, but to be defensive we will
        // skip parameters that couldn't have a layout computed
        // when assertions are disabled.
        //
        SLANG_ASSERT(paramTypeLayout);
        if(!paramTypeLayout)
            continue;

        // Now that we've computed the layout to use for the parameter,
        // we need to add its resource usage to that of the entry
        // point as a whole.
        //
        scopeBuilder.addSimpleParameter(paramVarLayout);
    }

    // We don't want certain kinds of resource usage within an entry
    // point to "leak" into the overall resource usage of the entry
    // point and thus lead to offsetting of successive entry points.
    //
    // For example if we have a vertex and a fragment entry point
    // in the some program, and each has one varying input, then
    // the both the vertex and fragment varying outputs should have
    // a location/index of zero. It would be bad if the fragment
    // input (or whichever entry point comes second in the global
    // ordering) started at location one, because then it wouldn't
    // line up correctly with any vertex stage outputs.
    //
    // We handle this with a bit of a kludge, by removing the
    // particular `LayoutResourceKind`s that are susceptible to
    // this problem from the overall resource usage of the entry
    // point.
    //
    removePerEntryPointParameterKinds(scopeBuilder.m_structLayout);

    entryPointLayout->parametersLayout = scopeBuilder.endLayout();

    // For an entry point with a non-`void` return type, we need to process the
    // return type as a varying output parameter.
    //
    // TODO: Ideally we should make the layout process more robust to empty/void
    // types and apply this logic unconditionally.
    //
    auto resultType = GetResultType(entryPointFuncDeclRef);
    SLANG_ASSERT(resultType);

    if( !resultType->Equals(resultType->getSession()->getVoidType()) )
    {
        state.loc = entryPointFuncDeclRef.getLoc();
        state.directionMask = kEntryPointParameterDirection_Output;

        RefPtr<VarLayout> resultLayout = new VarLayout();
        resultLayout->stage = state.stage;

        auto resultTypeLayout = processEntryPointVaryingParameterDecl(
            context,
            entryPointFuncDeclRef.getDecl(),
            resultType,
            state,
            resultLayout);

        if( resultTypeLayout )
        {
            resultLayout->typeLayout = resultTypeLayout;

            for (auto rr : resultTypeLayout->resourceInfos)
            {
                auto entryPointRes = paramsStructLayout->findOrAddResourceInfo(rr.kind);
                resultLayout->findOrAddResourceInfo(rr.kind)->index = entryPointRes->count.getFiniteValue();
                entryPointRes->count += rr.count;
            }
        }

        entryPointLayout->resultLayout = resultLayout;
    }

    return entryPointLayout;
}

    /// Visitor used by `collectGlobalGenericArguments`
struct CollectGlobalGenericArgumentsVisitor : ComponentTypeVisitor
{
    CollectGlobalGenericArgumentsVisitor(
        ParameterBindingContext*    context)
        : m_context(context)
    {}

    ParameterBindingContext* m_context;

    void visitEntryPoint(EntryPoint* entryPoint, EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        SLANG_UNUSED(entryPoint);
        SLANG_UNUSED(specializationInfo);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        SLANG_UNUSED(module);

        if(!specializationInfo)
            return;

        for(auto& globalGenericArg : specializationInfo->genericArgs)
        {
            if(auto globalGenericTypeParamDecl = as<GlobalGenericParamDecl>(globalGenericArg.paramDecl))
            {
                m_context->shared->programLayout->globalGenericArgs.Add(globalGenericTypeParamDecl, globalGenericArg.argVal);
            }
        }
    }

    void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        specialized->getBaseComponentType()->acceptVisitor(this, specialized->getSpecializationInfo());
    }
};

    /// Collect an ordered list of all the specialization arguments given for global generic specialization parameters in `program`.
    ///
    /// This information is used to accelerate the process of mapping a global generic type
    /// to its definition during type layout.
    ///
static void collectGlobalGenericArguments(
    ParameterBindingContext*    context,
    ComponentType*              program)
{
    CollectGlobalGenericArgumentsVisitor visitor(context);
    program->acceptVisitor(&visitor, nullptr);
}

    /// Collect information about the (unspecialized) specialization parameters of `program` into `context`.
    ///
    /// This function computes the reflection/layout for for the specialization parameters, so
    /// that they can be exposed to the API user.
    ///
static void collectSpecializationParams(
    ParameterBindingContext*    context,
    ComponentType*              program)
{
    auto specializationParamCount = program->getSpecializationParamCount();
    for(Index ii = 0; ii < specializationParamCount; ++ii)
    {
        auto specializationParam = program->getSpecializationParam(ii);
        switch(specializationParam.flavor)
        {
        case SpecializationParam::Flavor::GenericType:
        case SpecializationParam::Flavor::GenericValue:
            {
                RefPtr<GenericSpecializationParamLayout> paramLayout = new GenericSpecializationParamLayout();
                paramLayout->decl = specializationParam.object.as<Decl>();
                context->shared->programLayout->specializationParams.add(paramLayout);
            }
            break;

        case SpecializationParam::Flavor::ExistentialType:
        case SpecializationParam::Flavor::ExistentialValue:
            {
                RefPtr<ExistentialSpecializationParamLayout> paramLayout = new ExistentialSpecializationParamLayout();
                paramLayout->type = specializationParam.object.as<Type>();
                context->shared->programLayout->specializationParams.add(paramLayout);
            }
            break;

        default:
            SLANG_UNEXPECTED("unhandled specialization parameter flavor");
            break;
        }
    }
}

    /// Visitor used by `collectParameters()`
struct CollectParametersVisitor : ComponentTypeVisitor
{
    CollectParametersVisitor(
        ParameterBindingContext*    context)
        : m_context(context)
    {}

    ParameterBindingContext* m_context;

    void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        // The parameters of a composite component type can
        // be determined by just visiting its children in order.
        //
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        // The parameters of a specialized component type
        // are just those of its base component type, with
        // appropriate specialization information passed
        // along.
        //
        visitChildren(specialized);

        // While we are at it, we will also make note of any
        // tagged-union types that were used as part of the
        // specialization arguments, since we need to make
        // sure that their layout information is computed
        // and made available for IR code generation.
        //
        // Note: this isn't really the best place for this logic to sit,
        // but it is the simplest place where we can collect all the tagged
        // union types that get referenced by a program.
        //
        for( auto taggedUnionType : specialized->getTaggedUnionTypes() )
        {
            SLANG_ASSERT(taggedUnionType);
            auto substType = taggedUnionType;
            auto typeLayout = createTypeLayout(m_context->layoutContext, substType);
            m_context->shared->programLayout->taggedUnionTypeLayouts.add(typeLayout);
        }
    }


    void visitEntryPoint(EntryPoint* entryPoint, EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        // An entry point is a leaf case.
        //
        // In our current model an entry point does not introduce
        // any global shader parameters, but in practice it effectively
        // acts a lot like a single global shader parameter named after
        // the entry point and with a `struct` type that combines
        // all the `uniform` entry point parameters.
        //
        // Later passes will need to make sure that the entry point
        // gets enumerated in the right order relative to any global
        // shader parameters.
        //

        ParameterBindingContext contextData = *m_context;
        auto context = &contextData;
        context->stage = entryPoint->getStage();

        collectEntryPointParameters(context, entryPoint, specializationInfo);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        // A single module represents a leaf case for layout.
        //
        // We will enumerate the (global) shader parameters declared
        // in the module and add each to our canonical ordering.
        //
        auto paramCount = module->getShaderParamCount();

       ExpandedSpecializationArg* specializationArgs = specializationInfo
           ? specializationInfo->existentialArgs.getBuffer()
           : nullptr;

        for(Index pp = 0; pp < paramCount; ++pp)
        {
            auto shaderParamInfo = module->getShaderParam(pp);
            if(specializationArgs)
            {
                m_context->layoutContext = m_context->layoutContext.withSpecializationArgs(
                    specializationArgs,
                    shaderParamInfo.specializationParamCount);
                specializationArgs += shaderParamInfo.specializationParamCount;
            }

            collectGlobalScopeParameter(m_context, shaderParamInfo, SubstitutionSet());
        }
    }

};

    /// Recursively collect the global shader parameters and entry points in `program`.
    ///
    /// This function is used to establish the global ordering of parameters and
    /// entry points used for layout.
    ///
static void collectParameters(
    ParameterBindingContext*            inContext,
    ComponentType*                      program)
{
    // All of the parameters in translation units directly
    // referenced in the compile request are part of one
    // logical namespace/"linkage" so that two parameters
    // with the same name should represent the same
    // parameter, and get the same binding(s)

    ParameterBindingContext contextData = *inContext;
    auto context = &contextData;
    context->stage = Stage::Unknown;

    CollectParametersVisitor visitor(context);
    program->acceptVisitor(&visitor, nullptr);
}

    /// Emit a diagnostic about a uniform parameter at global scope.
void diagnoseGlobalUniform(
    SharedParameterBindingContext*  sharedContext,
    VarDeclBase*                    varDecl)
{
    // It is entirely possible for Slang to support uniform parameters at the global scope,
    // by bundling them into an implicit constant buffer, and indeed the layout algorithm
    // implemented in this file computes a layout *as if* the Slang compiler does just that.
    //
    // The missing link is the downstream IR and code generation steps, where we would need
    // to collect all of the global-scope uniforms into a common `struct` type and then
    // create a new constant buffer parameter over that type.
    //
    // For now it is easier to simply ban this case, since most shader authors have
    // switched to modern HLSL/GLSL style with `cbuffer` or `uniform` block declarations.
    //
    // TODO: In the long run it may be best to require *all* global-scope shader parameters
    // to be marked with a keyword (e.g., `uniform`) so that ordinary global variable syntax can be
    // used safely.
    //
    getSink(sharedContext)->diagnose(varDecl, Diagnostics::globalUniformsNotSupported, varDecl->getName());
}

static int _calcTotalNumUsedRegistersForLayoutResourceKind(ParameterBindingContext* bindingContext, LayoutResourceKind kind)
{
    int numUsed = 0;
    for (auto& pair : bindingContext->shared->globalSpaceUsedRangeSets)
    {
        UsedRangeSet* rangeSet = pair.Value;
        const auto& usedRanges = rangeSet->usedResourceRanges[kind];
        for (const auto& usedRange : usedRanges.ranges)
        {
            numUsed += int(usedRange.end - usedRange.begin);
        }
    }
    return numUsed;
}

static bool _isCPUTarget(CodeGenTarget target)
{
    switch (target)
    {
        case CodeGenTarget::CPPSource:
        case CodeGenTarget::CSource:
        case CodeGenTarget::Executable:
        case CodeGenTarget::SharedLibrary:
        case CodeGenTarget::HostCallable:
        {
            return true;
        }
        default: return false;
    }
}

static bool _isPTXTarget(CodeGenTarget target)
{
    switch (target)
    {
        case CodeGenTarget::CUDASource:
        case CodeGenTarget::PTX:
        {
            return true;
        }
        default: return false;
    }
}

    /// Keep track of the running global counter for entry points and global parameters visited.
    ///
    /// Because of explicit `register` and `[[vk::binding(...)]]` support, parameter binding
    /// needs to proceed in multiple passes, and each pass must both visit the things that
    /// need layout (parameters and entry points) in the same order in each pass, and must
    /// also be able to look up the side-band information that flows between passes.
    ///
    /// Currently the `ParameterBindingContext` keeps separate arrays for global shader
    /// parameters and entry points, but in the global ordering for layout they can be
    /// interleaved. There is also no simple tracking structure that relates a global
    /// parameter or entry point to its index in those arrays. Instead, we just keep
    /// running counters during our passes over the program so that we can easily
    /// compute the linear index of each entry point and global parameter as it
    /// is encountered.
    ///
struct ParameterBindingVisitorCounters
{
    Index entryPointCounter = 0;
    Index globalParamCounter = 0;
};

    /// Recursive routine to "complete" all binding for parameters and entry points in `componentType`.
    ///
    /// This includes allocation of as-yet-unused register/binding ranges to parameters (which
    /// will then affect the ranges of registers/bindings that are available to subsequent
    /// parameters), and imporantly *also* includes allocate of space to any "pending"
    /// data for interface/existential type parameters/fields.
    ///
static void _completeBindings(
    ParameterBindingContext*            context,
    ComponentType*                      componentType,
    ParameterBindingVisitorCounters*    ioCounters);

    /// A visitor used by `_completeBindings`.
    ///
    /// This visitor walks the structure of a `ComponentType` to ensure that
    /// any shader parameters (and entry points) it contains that *don't*
    /// have explicit bindings on them get allocated registers/bindings
    /// as appropriate.
    ///
    /// The main complication of this visitor is how it handles the
    /// `SpecializedComponentType` case, because a specialized component
    /// type needs to be handled as an atomic unit that lays out the
    /// same in all contexts.
    ///
struct CompleteBindingsVisitor : ComponentTypeVisitor
{
    CompleteBindingsVisitor(ParameterBindingContext* context, ParameterBindingVisitorCounters* counters)
        : m_context(context)
        , m_counters(counters)
    {}

    ParameterBindingContext* m_context;
    ParameterBindingVisitorCounters* m_counters;

    void visitEntryPoint(EntryPoint* entryPoint, EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        SLANG_UNUSED(entryPoint);
        SLANG_UNUSED(specializationInfo);

        // We compute the index of the entry point in the global ordering,
        // so we can look up the tracking data in our context. As a result
        // we don't actually make use of the parameters that were passed in.
        //
        auto globalEntryPointIndex = m_counters->entryPointCounter++;
        auto globalEntryPointInfo = m_context->shared->programLayout->entryPoints[globalEntryPointIndex];


        // We mostly treat an entry point like a single shader parameter that
        // uses its `parametersLayout`.
        //
        completeBindingsForParameter(m_context, globalEntryPointInfo->parametersLayout);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        SLANG_UNUSED(specializationInfo);
        // A module is a leaf case: we just want to visit each parameter.
        visitLeafParams(module);
    }

    void visitLeafParams(ComponentType* componentType)
    {
        auto paramCount = componentType->getShaderParamCount();
        for(Index ii = 0; ii < paramCount; ++ii)
        {
            auto globalParamIndex = m_counters->globalParamCounter++;
            auto globalParamInfo = m_context->shared->parameters[globalParamIndex];

            completeBindingsForParameter(m_context, globalParamInfo);
        }
    }

    void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        // We just wnat to recurse on the children of the composite in order.
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        // The handling of a specialized component type here is subtle.
        //
        // We do *not* simply recurse on the base component type.
        // Doing so would ensure that the parameters would get
        // registers/bindings allocated to them, but it wouldn't
        // allocate space for the "pending" data related to
        // existential/interface parameters.
        //
        // Instead, we recursive through `_completeBindings`,
        // which has the job of allocating space for the parameters,
        // and then for any "pending" data required.
        //
        // Handling things this way ensures that a particular
        // `SpecializedComponentType` gets laid out exactly
        // the same wherever it gets used, rather than
        // getting laid out differently when it is placed
        // into different compositions.
        //
        auto base = specialized->getBaseComponentType();
        _completeBindings(m_context, base, m_counters);
    }
};

    /// A visitor used by `_completeBindings`.
    ///
    /// This visitor is used to follow up after the `CompleteBindingsVisitor`
    /// any ensure that any "pending" data required by the parameters that
    /// got laid out now gets a location.
    ///
    /// To make a concrete example:
    ///
    ///     Texture2D a;
    ///     IThing    b;
    ///     Texture2D c;
    ///
    /// If these parameters were laid out with `b` specialized to a type
    /// that contains a single `Texture2D`, then the `CompleteBindingsVisitor`
    /// would visit `a`, `b`, and then `c` in order. It would give `a` the
    /// first register/binding available (say, `t0`). It would then make
    /// a note that due to specialization, `b`, needs a `t` register as well,
    /// but it *cannot* be allocated just yet, because doing so would change
    /// the location of `c`, so it is marked as "pending." Then `c` would
    /// be visited and get `t1`. As a result the registers given to `a`
    /// and `c` are independent of how `b` gets specialized.
    ///
    /// Next, the `FlushPendingDataVisitor` comes through and applies to
    /// the parameters again. For `a` there is no pending data, but for
    /// `b` there is a pending request for a `t` register, so it gets allocated
    /// now (getting `t2`). The `c` parameter then has no pending data, so
    /// we are done.
    ///
    /// *When* the pending data gets flushed is then significant. In general,
    /// the order in which modules get composed an specialized is signficaint.
    /// The module above (let's call it `M`) has one specialization parameter
    /// (for `b`), and if we want to compose it with another module `N` that
    /// has no specialization parameters, we could compute either:
    ///
    ///     compose(specialize(M, SomeType), N)
    ///
    /// or:
    ///
    ///     specialize(compose(M,N), SomeType)
    ///
    /// In the first case, the "pending" data for `M` gets flushed right after `M`,
    /// so that `specialize(M,SomeType)` can have a consistent layout
    /// regardless of how it is used. In the second case, the pending data for
    /// `M` only gets flushed after `N`'s parameters are allocated, thus guaranteeing
    /// that the `compose(M,N)` part has a consistent layout regardless of what
    /// type gets plugged in during specialization.
    ///
    /// There are trade-offs to be made by an application about which approach
    /// to prefer, and the compiler supports either policy choice.
    ///
struct FlushPendingDataVisitor : ComponentTypeVisitor
{
    FlushPendingDataVisitor(ParameterBindingContext* context, ParameterBindingVisitorCounters* counters)
        : m_context(context)
        , m_counters(counters)
    {}

    ParameterBindingContext* m_context;
    ParameterBindingVisitorCounters* m_counters;

    void visitEntryPoint(EntryPoint* entryPoint, EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        SLANG_UNUSED(entryPoint);
        SLANG_UNUSED(specializationInfo);

        auto globalEntryPointIndex = m_counters->entryPointCounter++;
        auto globalEntryPointInfo = m_context->shared->programLayout->entryPoints[globalEntryPointIndex];

        // We need to allocate space for any "pending" data that
        // appeared in the entry-point parameter list.
        //
        _allocateBindingsForPendingData(m_context, globalEntryPointInfo->parametersLayout->pendingVarLayout);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        SLANG_UNUSED(specializationInfo);
        visitLeafParams(module);
    }

    void visitLeafParams(ComponentType* componentType)
    {
        // In the "leaf" case we just allocate space for any
        // pending data in the parameters, in order.
        //
        auto paramCount = componentType->getShaderParamCount();
        for(Index ii = 0; ii < paramCount; ++ii)
        {
            auto globalParamIndex = m_counters->globalParamCounter++;
            auto globalParamInfo = m_context->shared->parameters[globalParamIndex];
            auto varLayout = globalParamInfo->varLayout;

            _allocateBindingsForPendingData(m_context, varLayout->pendingVarLayout);
        }
    }

    void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        // Because `SpecializedComponentType` was a special case for `CompleteBindingsVisitor`,
        // it ends up being a special case here too.
        //
        // The `CompleteBindings...` pass treated a `SpecializedComponentType`
        // as an atomic unit. Any "pending" data that came from its parameters
        // will already have been dealt with, so it would be incorrect for
        // us to recurse into `specialized`.
        //
        // Instead, we just need to *skip* `specialized`, since it was
        // completely handled already. This isn't quite as simple
        // as just doing nothing, because our passes are using
        // some global counters to find the absolute/linear index
        // of each parameter and entry point as it is encountered.
        // We will simply bump those counters by the number of
        // parameters and entry points contained under `specialized`,
        // which is luckily provided by the `ComponentType` API.
        // 
        m_counters->globalParamCounter += specialized->getShaderParamCount();
        m_counters->entryPointCounter += specialized->getEntryPointCount();
    }

};

static void _completeBindings(
    ParameterBindingContext*            context,
    ComponentType*                      componentType,
    ParameterBindingVisitorCounters*    ioCounters)
{
    ParameterBindingVisitorCounters savedCounters = *ioCounters;

    CompleteBindingsVisitor completeBindingsVisitor(context, ioCounters);
    componentType->acceptVisitor(&completeBindingsVisitor, nullptr);

    FlushPendingDataVisitor flushVisitor(context, &savedCounters);
    componentType->acceptVisitor(&flushVisitor, nullptr);
}

    /// "Complete" binding of parametesr in the given `program`.
    ///
    /// Completing binding involves both assigning registers/bindings
    /// to an parameters that didn't get explicit locations, and then
    /// also providing locations to any "pending" data that needed
    /// space allocated (used for existential/interface type parameters).
    ///
static void _completeBindings(
    ParameterBindingContext*    context,
    ComponentType*              program)
{
    // The process of completing binding has a recursive structure,
    // so we will immediately delegate to a subroutine that handles
    // the recursion.
    //
    ParameterBindingVisitorCounters counters;
    _completeBindings(context, program, &counters);
}

RefPtr<ProgramLayout> generateParameterBindings(
    TargetProgram*  targetProgram,
    DiagnosticSink* sink)
{
    auto program = targetProgram->getProgram();
    auto targetReq = targetProgram->getTargetReq();

    RefPtr<ProgramLayout> programLayout = new ProgramLayout();
    programLayout->targetProgram = targetProgram;

    {
        auto& pool = programLayout->hashedStringLiteralPool;
        program->enumerateIRModules([&](IRModule* module) { findGlobalHashedStringLiterals(module, pool); });
    }

    // Try to find rules based on the selected code-generation target
    auto layoutContext = getInitialLayoutContextForTarget(targetReq, programLayout);

    // If there was no target, or there are no rules for the target,
    // then bail out here.
    if (!layoutContext.rules)
        return nullptr;

    // Create a context to hold shared state during the process
    // of generating parameter bindings
    SharedParameterBindingContext sharedContext(
        layoutContext.getRulesFamily(),
        programLayout,
        targetReq,
        sink);

    // Create a sub-context to collect parameters that get
    // declared into the global scope
    ParameterBindingContext context;
    context.shared = &sharedContext;
    context.layoutContext = layoutContext;

    // We want to start by finding out what (if anything) has
    // been bound to the global generic parameters of the
    // program, since we need to know these types to compute
    // layout for parameters that use the generic type parameters.
    //
    collectGlobalGenericArguments(&context, program);

    // Next we want to collect a full listing of all the shader
    // parameters that need to be considered for layout, along
    // with all of the entry points, which also need their
    // parameters laid out and thus act pretty much like global
    // parameters themselves.
    //
    collectParameters(&context, program);

    // We will also collect basic information on the specialization
    // parameters exposed by the program.
    //
    // Whereas `collectGlobalGenericArguments` was collecting the
    // concrete types that have been plugged into specialization
    // parameters, this step is about collecting the *unspecialized*
    // parameters (if any) for the purposes of reflection.
    //
    collectSpecializationParams(&context, program);

    // Once we have a canonical list of all the shader parameters
    // (and entry points) in need of layout, we will walk through
    // the parameters that might have explicit binding annotations,
    // and "reserve" the registers/bindings/etc. that those parameters
    // declare so that subequent automatic layout steps do not try to
    // overlap them.
    //
    // Along the way we will issue diagnostics if there appear to
    // be overlapping, conflicting, or inconsistent explicit bindings.
    //
    // Note that we do *not* support explicit binding annotations
    // on entry point parameters, so we only consider global shader
    // parameters here.
    //
    // (Also note that explicit bindings end up being the main
    // source of complexity in the layout system, and we could greatly
    // simplify this file by eliminating support for explicit
    // binding in the future)
    //
    for( auto& parameter : sharedContext.parameters )
    {
        generateParameterBindings(&context, parameter);
    }

    // Once we have a canonical list of all the parameters, we can
    // detect if there are any global-scope parameters that make use
    // of `LayoutResourceKind::Uniform`, since such parameters would
    // need to be packaged into a "default" constant buffer.
    // The fxc/dxc compilers support this step, and in reflection
    // refer to the generated constant buffer as `$Globals`.
    //
    // Note that this logic doesn't account for the existance of
    // "legacy" (non-buffer-bound) uniforms in GLSL for OpenGL.
    // If we wanted to support legaqcy uniforms we would probably
    // want to do so through a different feature.
    //
    bool needDefaultConstantBuffer = false;

    // On a CPU target, it's okay to have global scope parameters that use Uniform resources (because on CPU
    // all resources are 'Uniform')
    // TODO(JS): We'll just assume the same with CUDA target for now..
    if (!_isCPUTarget(targetReq->target) && !_isPTXTarget(targetReq->target))
    {
        for( auto& parameterInfo : sharedContext.parameters )
        {
            auto varLayout = parameterInfo->varLayout;
            SLANG_RELEASE_ASSERT(varLayout);

            // Does the field have any uniform data?
            if( varLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
            {
                needDefaultConstantBuffer = true;
                diagnoseGlobalUniform(&sharedContext, varLayout->varDecl);
            }
        }
    }

    // Next, we want to determine if there are any global-scope parameters
    // that don't just allocate a whole register space to themselves; these
    // parameters will need to go into a "default" space, which should always
    // be the first space we allocate.
    //
    // As a starting point, we will definitely need a "default" space if
    // we are creating a default constant buffer, since it should get
    // a binding in that "default" space.
    //
    bool needDefaultSpace = needDefaultConstantBuffer;
    if (!needDefaultSpace)
    {
        // Next we will look at the global-scope parameters and see if
        // any of them requires a `register` or `binding` that will
        // thus need to land in a default space.
        //
        for (auto& parameterInfo : sharedContext.parameters)
        {
            auto varLayout = parameterInfo->varLayout;
            SLANG_RELEASE_ASSERT(varLayout);

            // For each parameter, we will look at each resource it consumes.
            //
            for (auto resInfo : varLayout->typeLayout->resourceInfos)
            {
                // We don't care about whole register spaces/sets, since
                // we don't need to allocate a default space/set for a parameter
                // that itself consumes a whole space/set.
                //
                if( resInfo.kind == LayoutResourceKind::RegisterSpace )
                    continue;

                // We also don't want to consider resource kinds for which
                // the variable already has an (explicit) binding, since
                // the space from the explicit binding will be used, so
                // that a default space isn't needed.
                //
                if( parameterInfo->bindingInfo[resInfo.kind].count != 0 )
                    continue;

                // Otherwise, we have a shader parameter that will need
                // a default space or set to live in.
                //
                needDefaultSpace = true;
                break;
            }
        }

        // We also need a default space for any entry-point parameters
        // that consume appropriate resource kinds.
        //
        for(auto& entryPoint : sharedContext.programLayout->entryPoints)
        {
            auto paramsLayout = entryPoint->parametersLayout;
            for(auto resInfo : paramsLayout->resourceInfos )
            {
                switch(resInfo.kind)
                {
                default:
                    break;

                case LayoutResourceKind::RegisterSpace:
                case LayoutResourceKind::VaryingInput:
                case LayoutResourceKind::VaryingOutput:
                case LayoutResourceKind::HitAttributes:
                case LayoutResourceKind::RayPayload:
                    continue;
                }

                needDefaultSpace = true;
                break;
            }
        }
    }

    // If we need a space for default bindings, then allocate it here.
    if (needDefaultSpace)
    {
        UInt defaultSpace = 0;

        // Check if space #0 has been allocated yet. If not, then we'll
        // want to use it.
        if (sharedContext.usedSpaces.contains(0))
        {
            // Somebody has already put things in space zero.
            //
            // TODO: There are two cases to handle here:
            //
            // 1) If there is any free register ranges in space #0,
            // then we should keep using it as the default space.
            //
            // 2) If somebody went and put an HLSL unsized array into space #0,
            // *or* if they manually placed something like a paramter block
            // there (which should consume whole spaces), then we need to
            // allocate an unused space instead.
            //
            // For now we don't deal with the concept of unsized arrays, or
            // manually assigning parameter blocks to spaces, so we punt
            // on this and assume case (1).

            defaultSpace = 0;
        }
        else
        {
            // Nobody has used space zero yet, so we need
            // to make sure to reserve it for defaults.
            defaultSpace = allocateUnusedSpaces(&context, 1);

            // The result of this allocation had better be that
            // we got space #0, or else something has gone wrong.
            SLANG_ASSERT(defaultSpace == 0);
        }

        sharedContext.defaultSpace = defaultSpace;
    }

    // If there are any global-scope uniforms, then we need to
    // allocate a constant-buffer binding for them here.
    //
    ParameterBindingAndKindInfo globalConstantBufferBinding = maybeAllocateConstantBufferBinding(
        &context,
        needDefaultConstantBuffer);

    // Now that all of the explicit bindings have been dealt with
    // and we've also allocate any space/buffer that is required
    // for global-scope parameters, we will go through the
    // shader parameters and entry points yet again, in order
    // to actually allocate specific bindings/registers to
    // parameters and entry points that need them.
    //
    _completeBindings(&context, program);

    // Next we need to create a type layout to reflect the information
    // we have collected, and we will use the `ScopeLayoutBuilder`
    // to encapsulate the logic that can be shared with the entry-point
    // case.
    //
    ScopeLayoutBuilder globalScopeLayoutBuilder;
    globalScopeLayoutBuilder.beginLayout(&context);
    for( auto& parameterInfo : sharedContext.parameters )
    {
        globalScopeLayoutBuilder.addParameter(parameterInfo);
    }

    auto globalScopeVarLayout = globalScopeLayoutBuilder.endLayout();
    if( globalConstantBufferBinding.count != 0 )
    {
        auto cbInfo = globalScopeVarLayout->findOrAddResourceInfo(globalConstantBufferBinding.kind);
        cbInfo->space = globalConstantBufferBinding.space;
        cbInfo->index = globalConstantBufferBinding.index;
    }

    programLayout->parametersLayout = globalScopeVarLayout;

    {
        const int numShaderRecordRegs = _calcTotalNumUsedRegistersForLayoutResourceKind(&context, LayoutResourceKind::ShaderRecord);
        if (numShaderRecordRegs > 1)
        {
           sink->diagnose(SourceLoc(), Diagnostics::tooManyShaderRecordConstantBuffers, numShaderRecordRegs);
        }
    }

    return programLayout;
}

ProgramLayout* TargetProgram::getOrCreateLayout(DiagnosticSink* sink)
{
    if( !m_layout )
    {
        m_layout = generateParameterBindings(this, sink);
        if( m_layout )
        {
            m_irModuleForLayout = createIRModuleForLayout(sink);
        }
    }
    return m_layout;
}

void generateParameterBindings(
    ComponentType*  program,
    TargetRequest*  targetReq,
    DiagnosticSink* sink)
{
    program->getTargetProgram(targetReq)->getOrCreateLayout(sink);
}

} // namespace Slang

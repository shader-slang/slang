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
        Int rangeCount = ranges.Count();
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
                ranges.Add(prefix);
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
            ranges.Add(range);
        }

        // Any ranges that got added along the way might not
        // be in the proper sorted order, so we'll need to
        // sort the array to restore our global invariant.
        //
        ranges.Sort();

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
    // Layout info for the concrete variables that will make up this parameter
    List<RefPtr<VarLayout>> varLayouts;

    ParameterBindingInfo    bindingInfo[kLayoutResourceKindCount];

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

    // A dictionary to accelerate looking up parameters by name
    Dictionary<Name*, ParameterInfo*> mapNameToParameterInfo;

    // What stage (if any) are we compiling for?
    Stage stage;

    // The entry point that is being processed right now.
    EntryPointLayout*   entryPointLayout = nullptr;

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

static Name* getReflectionName(VarDeclBase* varDecl)
{
    if (auto reflectionNameModifier = varDecl->FindModifier<ParameterGroupReflectionName>())
        return reflectionNameModifier->nameAndLoc.name;

    return varDecl->getName();
}

// Information tracked when doing a structural
// match of types.
struct StructuralTypeMatchStack
{
    DeclRef<VarDeclBase>        leftDecl;
    DeclRef<VarDeclBase>        rightDecl;
    StructuralTypeMatchStack*   parent;
};

static void diagnoseParameterTypeMismatch(
    ParameterBindingContext*    context,
    StructuralTypeMatchStack*   inStack)
{
    SLANG_ASSERT(inStack);

    // The bottom-most entry in the stack should represent
    // the shader parameters that kicked things off
    auto stack = inStack;
    while(stack->parent)
        stack = stack->parent;

    getSink(context)->diagnose(stack->leftDecl, Diagnostics::shaderParameterDeclarationsDontMatch, getReflectionName(stack->leftDecl));
    getSink(context)->diagnose(stack->rightDecl, Diagnostics::seeOtherDeclarationOf, getReflectionName(stack->rightDecl));
}

// Two types that were expected to match did not.
// Inform the user with a suitable message.
static void diagnoseTypeMismatch(
    ParameterBindingContext*    context,
    StructuralTypeMatchStack*   inStack)
{
    auto stack = inStack;
    SLANG_ASSERT(stack);
    diagnoseParameterTypeMismatch(context, stack);

    auto leftType = GetType(stack->leftDecl);
    auto rightType = GetType(stack->rightDecl);

    if( stack->parent )
    {
        getSink(context)->diagnose(stack->leftDecl, Diagnostics::fieldTypeMisMatch, getReflectionName(stack->leftDecl), leftType, rightType);
        getSink(context)->diagnose(stack->rightDecl, Diagnostics::seeOtherDeclarationOf, getReflectionName(stack->rightDecl));

        stack = stack->parent;
        if( stack )
        {
            while( stack->parent )
            {
                getSink(context)->diagnose(stack->leftDecl, Diagnostics::usedInDeclarationOf, getReflectionName(stack->leftDecl));
                stack = stack->parent;
            }
        }
    }
    else
    {
        getSink(context)->diagnose(stack->leftDecl, Diagnostics::shaderParameterTypeMismatch, leftType, rightType);
    }
}

// Two types that were expected to match did not.
// Inform the user with a suitable message.
static void diagnoseTypeFieldsMismatch(
    ParameterBindingContext*    context,
    DeclRef<Decl> const&        left,
    DeclRef<Decl> const&        right,
    StructuralTypeMatchStack*   stack)
{
    diagnoseParameterTypeMismatch(context, stack);

    getSink(context)->diagnose(left, Diagnostics::fieldDeclarationsDontMatch, left.GetName());
    getSink(context)->diagnose(right, Diagnostics::seeOtherDeclarationOf, right.GetName());

    if( stack )
    {
        while( stack->parent )
        {
            getSink(context)->diagnose(stack->leftDecl, Diagnostics::usedInDeclarationOf, getReflectionName(stack->leftDecl));
            stack = stack->parent;
        }
    }
}

static void collectFields(
    DeclRef<AggTypeDecl>    declRef,
    List<DeclRef<VarDecl>>& outFields)
{
    for( auto fieldDeclRef : getMembersOfType<VarDecl>(declRef) )
    {
        if(fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
            continue;

        outFields.Add(fieldDeclRef);
    }
}

static bool validateTypesMatch(
    ParameterBindingContext*    context,
    Type*                       left,
    Type*                       right,
    StructuralTypeMatchStack*   stack);

static bool validateIntValuesMatch(
    ParameterBindingContext*    context,
    IntVal*                     left,
    IntVal*                     right,
    StructuralTypeMatchStack*   stack)
{
    if(left->EqualsVal(right))
        return true;

    // TODO: are there other cases we need to handle here?

    diagnoseTypeMismatch(context, stack);
    return false;
}


static bool validateValuesMatch(
    ParameterBindingContext*    context,
    Val*                        left,
    Val*                        right,
    StructuralTypeMatchStack*   stack)
{
    if( auto leftType = dynamicCast<Type>(left) )
    {
        if( auto rightType = dynamicCast<Type>(right) )
        {
            return validateTypesMatch(context, leftType, rightType, stack);
        }
    }

    if( auto leftInt = dynamicCast<IntVal>(left) )
    {
        if( auto rightInt = dynamicCast<IntVal>(right) )
        {
            return validateIntValuesMatch(context, leftInt, rightInt, stack);
        }
    }

    if( auto leftWitness = dynamicCast<SubtypeWitness>(left) )
    {
        if( auto rightWitness = dynamicCast<SubtypeWitness>(right) )
        {
            return true;
        }
    }

    diagnoseTypeMismatch(context, stack);
    return false;
}

static bool validateGenericSubstitutionsMatch(
    ParameterBindingContext*    context,
    GenericSubstitution*        left,
    GenericSubstitution*        right,
    StructuralTypeMatchStack*   stack)
{
    if( !left )
    {
        if( !right )
        {
            return true;
        }

        diagnoseTypeMismatch(context, stack);
        return false;
    }



    UInt argCount = left->args.Count();
    if( argCount != right->args.Count() )
    {
        diagnoseTypeMismatch(context, stack);
        return false;
    }

    for( UInt aa = 0; aa < argCount; ++aa )
    {
        auto leftArg = left->args[aa];
        auto rightArg = right->args[aa];

        if(!validateValuesMatch(context, leftArg, rightArg, stack))
            return false;
    }

    return true;
}

static bool validateThisTypeSubstitutionsMatch(
    ParameterBindingContext*    /*context*/,
    ThisTypeSubstitution*       /*left*/,
    ThisTypeSubstitution*       /*right*/,
    StructuralTypeMatchStack*   /*stack*/)
{
    // TODO: actual checking.
    return true;
}

static bool validateSpecializationsMatch(
    ParameterBindingContext*    context,
    SubstitutionSet             left,
    SubstitutionSet             right,
    StructuralTypeMatchStack*   stack)
{
    auto ll = left.substitutions;
    auto rr = right.substitutions;
    for(;;)
    {
        // Skip any global generic substitutions.
        if(auto leftGlobalGeneric = as<GlobalGenericParamSubstitution>(ll))
        {
            ll = leftGlobalGeneric->outer;
            continue;
        }
        if(auto rightGlobalGeneric = as<GlobalGenericParamSubstitution>(rr))
        {
            rr = rightGlobalGeneric->outer;
            continue;
        }

        // If either ran out, then we expect both to have run out.
        if(!ll || !rr)
            return !ll && !rr;

        auto leftSubst = ll;
        auto rightSubst = rr;

        ll = ll->outer;
        rr = rr->outer;

        if(auto leftGeneric = as<GenericSubstitution>(leftSubst))
        {
            if(auto rightGeneric = as<GenericSubstitution>(rightSubst))
            {
                if(validateGenericSubstitutionsMatch(context, leftGeneric, rightGeneric, stack))
                {
                    continue;
                }
            }
        }
        else if(auto leftThisType = as<ThisTypeSubstitution>(leftSubst))
        {
            if(auto rightThisType = as<ThisTypeSubstitution>(rightSubst))
            {
                if(validateThisTypeSubstitutionsMatch(context, leftThisType, rightThisType, stack))
                {
                    continue;
                }
            }
        }

        return false;
    }

    return true;
}

// Determine if two types "match" for the purposes of `cbuffer` layout rules.
//
static bool validateTypesMatch(
    ParameterBindingContext*    context,
    Type*                       left,
    Type*                       right,
    StructuralTypeMatchStack*   stack)
{
    if(left->Equals(right))
        return true;

    // It is possible that the types don't match exactly, but
    // they *do* match structurally.

    // Note: the following code will lead to infinite recursion if there
    // are ever recursive types. We'd need a more refined system to
    // cache the matches we've already found.

    if( auto leftDeclRefType = as<DeclRefType>(left) )
    {
        if( auto rightDeclRefType = as<DeclRefType>(right) )
        {
            // Are they references to matching decl refs?
            auto leftDeclRef = leftDeclRefType->declRef;
            auto rightDeclRef = rightDeclRefType->declRef;

            // Do the reference the same declaration? Or declarations
            // with the same name?
            //
            // TODO: we should only consider the same-name case if the
            // declarations come from translation units being compiled
            // (and not an imported module).
            if( leftDeclRef.getDecl() == rightDeclRef.getDecl()
                || leftDeclRef.GetName() == rightDeclRef.GetName() )
            {
                // Check that any generic arguments match
                if( !validateSpecializationsMatch(
                    context,
                    leftDeclRef.substitutions,
                    rightDeclRef.substitutions,
                    stack) )
                {
                    return false;
                }

                // Check that any declared fields match too.
                if( auto leftStructDeclRef = leftDeclRef.as<AggTypeDecl>() )
                {
                    if( auto rightStructDeclRef = rightDeclRef.as<AggTypeDecl>() )
                    {
                        List<DeclRef<VarDecl>> leftFields;
                        List<DeclRef<VarDecl>> rightFields;

                        collectFields(leftStructDeclRef, leftFields);
                        collectFields(rightStructDeclRef, rightFields);

                        UInt leftFieldCount = leftFields.Count();
                        UInt rightFieldCount = rightFields.Count();

                        if( leftFieldCount != rightFieldCount )
                        {
                            diagnoseTypeFieldsMismatch(context, leftDeclRef, rightDeclRef, stack);
                            return false;
                        }

                        for( UInt ii = 0; ii < leftFieldCount; ++ii )
                        {
                            auto leftField = leftFields[ii];
                            auto rightField = rightFields[ii];

                            if( leftField.GetName() != rightField.GetName() )
                            {
                                diagnoseTypeFieldsMismatch(context, leftDeclRef, rightDeclRef, stack);
                                return false;
                            }

                            auto leftFieldType = GetType(leftField);
                            auto rightFieldType = GetType(rightField);

                            StructuralTypeMatchStack subStack;
                            subStack.parent = stack;
                            subStack.leftDecl = leftField;
                            subStack.rightDecl = rightField;

                            if(!validateTypesMatch(context, leftFieldType,rightFieldType, &subStack))
                                return false;
                        }
                    }
                }

                // Everything seemed to match recursively.
                return true;
            }
        }
    }

    // If we are looking at `T[N]` and `U[M]` we want to check that
    // `T` is structurally equivalent to `U` and `N` is the same as `M`.
    else if( auto leftArrayType = as<ArrayExpressionType>(left) )
    {
        if( auto rightArrayType = as<ArrayExpressionType>(right) )
        {
            if(!validateTypesMatch(context, leftArrayType->baseType, rightArrayType->baseType, stack) )
                return false;

            if(!validateValuesMatch(context, leftArrayType->ArrayLength, rightArrayType->ArrayLength, stack))
                return false;

            return true;
        }
    }

    diagnoseTypeMismatch(context, stack);
    return false;
}

// This function is supposed to determine if two global shader
// parameter declarations represent the same logical parameter
// (so that they should get the exact same binding(s) allocated).
//
static bool doesParameterMatch(
    ParameterBindingContext* context,
    RefPtr<VarLayout>   varLayout,
    ParameterInfo* parameterInfo)
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

    StructuralTypeMatchStack stack;
    stack.parent = nullptr;
    stack.leftDecl = varLayout->varDecl;
    stack.rightDecl = parameterInfo->varLayouts[0]->varDecl;

    validateTypesMatch(context, varLayout->typeLayout->type, parameterInfo->varLayouts[0]->typeLayout->type, &stack);

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
            *outVal = (UInt) strtoull(String(modifier->valToken.Content).Buffer(), nullptr, 10);
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
    if( varDecl->HasModifier<InModifier>() || as<GLSLInputParameterGroupType>(type))
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
                // Unwrap array type, if present
                if( auto arrayType = as<ArrayExpressionType>(type) )
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
    if( varDecl->HasModifier<OutModifier>() || as<GLSLOutputParameterGroupType>(type))
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
                // Unwrap array type, if present
                if( auto arrayType = as<ArrayExpressionType>(type) )
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

    if( varDecl->HasModifier<ShaderRecordAttribute>() && as<ConstantBufferType>(type) )
    {
        return CreateTypeLayout(
            layoutContext.with(rules->getShaderRecordConstantBufferRules()),
            type);
    }


    // We want to check for a constant-buffer type with a `push_constant` layout
    // qualifier before we move on to anything else.
    if (varDecl->HasModifier<PushConstantAttribute>() && as<ConstantBufferType>(type))
    {
        return CreateTypeLayout(
            layoutContext.with(rules->getPushConstantBufferRules()),
            type);
    }

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

RefPtr<TypeLayout> getTypeLayoutForGlobalShaderParameter(
    ParameterBindingContext*    context,
    VarDeclBase*                varDecl)
{
    return getTypeLayoutForGlobalShaderParameter(context, varDecl, varDecl->getType());
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
    Stage                               stage = Stage::Unknown;
    bool                                isSampleRate = false;
    SourceLoc                           loc;
};


static RefPtr<TypeLayout> processEntryPointVaryingParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout);

// Collect a single declaration into our set of parameters
static void collectGlobalGenericParameter(
    ParameterBindingContext*    context,
    RefPtr<GlobalGenericParamDecl>         paramDecl)
{
    RefPtr<GenericParamLayout> layout = new GenericParamLayout();
    layout->decl = paramDecl;
    layout->index = (int)context->shared->programLayout->globalGenericParams.Count();
    context->shared->programLayout->globalGenericParams.Add(layout);
    context->shared->programLayout->globalGenericParamsMap[layout->decl->getName()->text] = layout.Ptr();
}

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
    varLayout->varDecl = DeclRef<Decl>(varDecl.Ptr(), nullptr).as<VarDeclBase>();

    // This declaration may represent the same logical parameter
    // as a declaration that came from a different translation unit.
    // If that is the case, we want to re-use the same `VarLayout`
    // across both parameters.
    //
    // TODO: This logic currently detects *any* global-scope parameters
    // with matching names, but it should eventually be narrowly
    // scoped so that it only applies to parameters from unnamed modules.
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

static void addExplicitParameterBinding(
    ParameterBindingContext*    context,
    RefPtr<ParameterInfo>       parameterInfo,
    VarDeclBase*                varDecl,
    LayoutSemanticInfo const&   semanticInfo,
    LayoutSize                  count,
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
        auto overlappedVarLayout = usedRangeSet->usedResourceRanges[(int)semanticInfo.kind].Add(
            parameterInfo->varLayouts[0],
            semanticInfo.index,
            semanticInfo.index + count);

        if (overlappedVarLayout)
        {
            auto paramA = parameterInfo->varLayouts[0]->varDecl.getDecl();
            auto paramB = overlappedVarLayout->varDecl.getDecl();

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

    addExplicitParameterBinding(context, parameterInfo, varDecl, semanticInfo, count, usedRangeSet);
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
    // We will use the first declaration of the parameter as
    // a stand-in for all the declarations, so it is important
    // that earlier code has validated that the declarations
    // "match".

    SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);
    auto firstVarLayout = parameterInfo->varLayouts.First();

    completeBindingsForParameterImpl(
        context,
        firstVarLayout,
        parameterInfo->bindingInfo,
        parameterInfo);

    // At this point we should have explicit binding locations chosen for
    // all the relevant resource kinds, so we can apply these to the
    // declarations:

    for(auto& varLayout : parameterInfo->varLayouts)
    {
        applyBindingInfoToParameter(varLayout, parameterInfo->bindingInfo);
    }
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



static void collectGlobalScopeParameters(
    ParameterBindingContext*    context,
    ModuleDecl*          program)
{
    // First enumerate parameters at global scope
    // We collect two things here:
    // 1. A shader parameter, which is always a variable
    // 2. A global entry-point generic parameter type (`type_param`),
    //    which is a GlobalGenericParamDecl
    // We collect global generic type parameters in the first pass,
    // So we can fill in the correct index into ordinary type layouts 
    // for generic types in the second pass.
    for (auto decl : program->Members)
    {
        if (auto genParamDecl = as<GlobalGenericParamDecl>(decl))
            collectGlobalGenericParameter(context, genParamDecl);
    }
    for (auto decl : program->Members)
    {
        if (auto varDecl = as<VarDeclBase>(decl))
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

        info.name = stringComposedName.SubString(0, indexLoc);
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

        if (state.directionMask & kEntryPointParameterDirection_Input)
        {
            if (sn == "sv_sampleindex")
            {
                state.isSampleRate = true;
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
            return CreateTypeLayout(context->layoutContext.with(
                context->getRulesFamily()->getRayPayloadParameterRules()),
                type);

        case Stage::Callable:
            // `in out` or `out` parameter is payload
            return CreateTypeLayout(context->layoutContext.with(
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
            return CreateTypeLayout(context->layoutContext.with(
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
        String sn = semanticName.ToUpper();

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

                structLayout->fields.Add(fieldVarLayout);
                structLayout->mapVarToLayout.Add(field.getDecl(), fieldVarLayout);
            }

            return structLayout;
        }
        else if (auto globalGenericParam = declRef.as<GlobalGenericParamDecl>())
        {
            auto genParamTypeLayout = new GenericParamTypeLayout();
            // we should have already populated ProgramLayout::genericEntryPointParams list at this point,
            // so we can find the index of this generic param decl in the list
            genParamTypeLayout->type = type;
            genParamTypeLayout->paramIndex = findGenericParam(context->shared->programLayout->globalGenericParams, globalGenericParam.getDecl());
            genParamTypeLayout->findOrAddResourceInfo(LayoutResourceKind::GenericResource)->count += 1;
            return genParamTypeLayout;
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
    SubstitutionSet                 typeSubst,
    DeclRef<ParamDecl>              paramDeclRef,
    RefPtr<VarLayout>               paramVarLayout,
    EntryPointParameterState&       state)
{
    auto paramDeclRefType = GetType(paramDeclRef);
    SLANG_ASSERT(paramDeclRefType);

    auto paramType = paramDeclRefType->Substitute(typeSubst).as<Type>();

    if( paramDeclRef.getDecl()->HasModifier<HLSLUniformModifier>() )
    {
        // An entry-point parameter that is explicitly marked `uniform` represents
        // a uniform shader parameter passed via the implicitly-defined
        // constant buffer (e.g., the `$Params` constant buffer seen in fxc/dxc output).
        //
        return CreateTypeLayout(
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
    bool                        m_needConstantBuffer = false;

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
        RefPtr<VarLayout>   firstVarLayout,
        ParameterInfo*      parameterInfo)
    {
        // Does the parameter have any uniform data?
        auto layoutInfo = firstVarLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform);
        LayoutSize uniformSize = layoutInfo ? layoutInfo->count : 0;
        if( uniformSize != 0 )
        {
            m_needConstantBuffer = true;

            // Make sure uniform fields get laid out properly...

            UniformLayoutInfo fieldInfo(
                uniformSize,
                firstVarLayout->typeLayout->uniformAlignment);

            LayoutSize uniformOffset = m_rules->AddStructField(
                &m_structLayoutInfo,
                fieldInfo);

            if( parameterInfo )
            {
                for( auto& varLayout : parameterInfo->varLayouts )
                {
                    varLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset.getFiniteValue();
                }
            }
            else
            {
                firstVarLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset.getFiniteValue();
            }
        }

        m_structLayout->fields.Add(firstVarLayout);

        if( parameterInfo )
        {
            for( auto& varLayout : parameterInfo->varLayouts )
            {
                m_structLayout->mapVarToLayout.Add(varLayout->varDecl.getDecl(), varLayout);
            }
        }
        else
        {
            m_structLayout->mapVarToLayout.Add(firstVarLayout->varDecl.getDecl(), firstVarLayout);
        }
    }

    void addParameter(
        RefPtr<VarLayout> varLayout)
    {
        _addParameter(varLayout, nullptr);
    }

    void addParameter(
        ParameterInfo* parameterInfo)
    {
        SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);
        auto firstVarLayout = parameterInfo->varLayouts.First();

        _addParameter(firstVarLayout, parameterInfo);
    }

    RefPtr<VarLayout> endLayout()
    {
        m_rules->EndStructLayout(&m_structLayoutInfo);

        RefPtr<TypeLayout> scopeTypeLayout = m_structLayout;

        // If the caller decided to allocate a constant buffer for
        // the ordinary data, then we need to wrap up the structure
        // type (layout) in a constant buffer type (layout).
        //
        if( m_needConstantBuffer )
        {
            auto constantBufferLayout = createParameterGroupTypeLayout(
                m_context->layoutContext,
                nullptr,
                m_rules,
                m_rules->GetObjectLayout(ShaderParameterKind::ConstantBuffer),
                m_structLayout);

            scopeTypeLayout = constantBufferLayout;
        }

        // We now have a bunch of layout information, which we should
        // record into a suitable object that represents the scope
        RefPtr<VarLayout> scopeVarLayout = new VarLayout();
        scopeVarLayout->typeLayout = scopeTypeLayout;
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

    /// Iterate over the parameters of an entry point to compute its requirements.
    ///
static void collectEntryPointParameters(
    ParameterBindingContext*        context,
    EntryPoint*                     entryPoint,
    SubstitutionSet                 typeSubst)
{
    DeclRef<FuncDecl> entryPointFuncDeclRef = entryPoint->getFuncDeclRef();

    // We will take responsibility for creating and filling in
    // the `EntryPointLayout` object here.
    //
    RefPtr<EntryPointLayout> entryPointLayout = new EntryPointLayout();
    entryPointLayout->profile = entryPoint->getProfile();
    entryPointLayout->entryPoint = entryPointFuncDeclRef.getDecl();

    // The entry point layout must be added to the output
    // program layout so that it can be accessed by reflection.
    //
    context->shared->programLayout->entryPoints.Add(entryPointLayout);

    // For the duration of our parameter collection work we will
    // establish this entry point as the current one in the context.
    //
    context->entryPointLayout = entryPointLayout;

    // Note: this isn't really the best place for this logic to sit,
    // but it is the simplest place where we have a direct correspondence
    // between a single `EntryPoint` and its matching `EntryPointLayout`,
    // so we'll use it.
    //
    for( auto taggedUnionType : entryPoint->getTaggedUnionTypes() )
    {
        SLANG_ASSERT(taggedUnionType);
        auto substType = taggedUnionType->Substitute(typeSubst).as<Type>();
        auto typeLayout = CreateTypeLayout(context->layoutContext, substType);
        entryPointLayout->taggedUnionTypeLayouts.Add(typeLayout);
    }

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

    for( auto paramDeclRef : getMembersOfType<ParamDecl>(entryPointFuncDeclRef) )
    {
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
            typeSubst,
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
        // Any "ordinary" data (e.g., a `float4x4`) needs to be accounted
        // for using the `ScopeLayoutBuilder`, since it will handle
        // the details of target-specific `struct` type layout.
        //
        scopeBuilder.addParameter(paramVarLayout);

        // All of the other resources types will be handled in a
        // simpler loop that just increments the relevant counters.
        //
        for (auto paramTypeResInfo : paramTypeLayout->resourceInfos)
        {
            // We need to skip ordinary data because it is being
            // handled by the `scopeBuilder`.
            //
            if(paramTypeResInfo.kind == LayoutResourceKind::Uniform)
                continue;

            // Whatever resources the parameter uses, we need to
            // assign the parameter's location/register/binding offset to
            // be the sum of everything added so far.
            //
            auto entryPointResInfo = paramsStructLayout->findOrAddResourceInfo(paramTypeResInfo.kind);
            paramVarLayout->findOrAddResourceInfo(paramTypeResInfo.kind)->index = entryPointResInfo->count.getFiniteValue();

            // We then need to add the resources consumed by the parameter
            // to those consumed by the entry point.
            //
            entryPointResInfo->count += paramTypeResInfo.count;
        }
    }
    entryPointLayout->parametersLayout = scopeBuilder.endLayout();

    // For an entry point with a non-`void` return type, we need to process the
    // return type as a varying output parameter.
    //
    // TODO: Ideally we should make the layout process more robust to empty/void
    // types and apply this logic unconditionally.
    //
    auto resultType = GetResultType(entryPointFuncDeclRef)->Substitute(typeSubst).as<Type>();
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
            resultType->Substitute(typeSubst).as<Type>(),
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
}

static void collectParameters(
    ParameterBindingContext*        inContext,
    Program*                        program)
{
    // All of the parameters in translation units directly
    // referenced in the compile request are part of one
    // logical namespace/"linkage" so that two parameters
    // with the same name should represent the same
    // parameter, and get the same binding(s)
    ParameterBindingContext contextData = *inContext;
    auto context = &contextData;

    for(RefPtr<Module> module : program->getModuleDependencies())
    {
        context->stage = Stage::Unknown;

        // First look at global-scope parameters
        collectGlobalScopeParameters(context, module->getModuleDecl());
    }

    // Next consider parameters for entry points
    for(auto entryPoint : program->getEntryPoints())
    {
        context->stage = entryPoint->getStage();
        collectEntryPointParameters(context, entryPoint, SubstitutionSet());
    }
    context->entryPointLayout = nullptr;
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

RefPtr<ProgramLayout> generateParameterBindings(
    TargetProgram*  targetProgram,
    DiagnosticSink* sink)
{
    auto program = targetProgram->getProgram();
    auto targetReq = targetProgram->getTargetReq();

    RefPtr<ProgramLayout> programLayout = new ProgramLayout();
    programLayout->targetProgram = targetProgram;

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

    // Walk through AST to discover all the parameters
    collectParameters(&context, program);

    // Now walk through the parameters to generate initial binding information
    for( auto& parameter : sharedContext.parameters )
    {
        generateParameterBindings(&context, parameter);
    }

    // Determine if there are any global-scope parameters that use `Uniform`
    // resources, and thus need to get packaged into a constant buffer.
    //
    // Note: this doesn't account for GLSL's support for "legacy" uniforms
    // at global scope, which don't get assigned a CB.
    bool needDefaultConstantBuffer = false;
    for( auto& parameterInfo : sharedContext.parameters )
    {
        SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);
        auto firstVarLayout = parameterInfo->varLayouts.First();

        // Does the field have any uniform data?
        if( firstVarLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
        {
            needDefaultConstantBuffer = true;
            diagnoseGlobalUniform(&sharedContext, firstVarLayout->varDecl);
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
    bool needDefaultSpace = needDefaultConstantBuffer;
    if (!needDefaultSpace)
    {
        for (auto& parameterInfo : sharedContext.parameters)
        {
            SLANG_RELEASE_ASSERT(parameterInfo->varLayouts.Count() != 0);
            auto firstVarLayout = parameterInfo->varLayouts.First();

            // Does the parameter have any resource usage that isn't just
            // allocating a whole register space?
            for (auto resInfo : firstVarLayout->typeLayout->resourceInfos)
            {
                if (resInfo.kind != LayoutResourceKind::RegisterSpace)
                {
                    needDefaultSpace = true;
                    break;
                }
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

    // Now walk through again to actually give everything
    // ranges of registers...
    for( auto& parameter : sharedContext.parameters )
    {
        completeBindingsForParameter(&context, parameter);
    }

    // After we have allocated registers/bindings to everything
    // in the global scope we will process the parameters
    // of each entry point in order.
    //
    // Note: the effect of the current implementation is to
    // allocate non-overlapping registers/bindings between all
    // the entry points in the compile request (e.g., if you
    // have a vertex and fragment shader being compiled together,
    // we will allocate distinct constant buffer registers for
    // their uniform parameters).
    //
    // TODO: We probably need to provide some more nuanced control
    // over whether entry points get overlapping or non-overlapping
    // bindings. It seems clear that if we were compiling multiple
    // compute kernels in one invocation we'd want them to get
    // overlapping bindings, because we cannot ever have them bound
    // together in a single pipeline state.
    //
    // Similarly, entry point parameters of DirectX Raytracing (DXR)
    // shaders should probably be allowed to overlap by default,
    // since those parameters should really go into the "local root signature."
    // (Note: there is a bit more subtlety around ray tracing
    // shaders that will be assembled into a "hit group")
    //
    // For now we are just doing the simplest thing, which will be
    // appropriate for:
    //
    // * Compiling a single compute shader in a compile request.
    // * Compiling some number of rasterization shader entry points
    //   in a single request, to be used together.
    // * Compiling a single ray-tracing shader in a compile request.
    //
    for( auto entryPoint : sharedContext.programLayout->entryPoints )
    {
        auto entryPointParamsLayout = entryPoint->parametersLayout;
        completeBindingsForParameter(&context, entryPointParamsLayout);
    }

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
    }
    return m_layout;
}

void generateParameterBindings(
    Program*        program,
    TargetRequest*  targetReq,
    DiagnosticSink* sink)
{
    program->getTargetProgram(targetReq)->getOrCreateLayout(sink);
}

RefPtr<ProgramLayout> specializeProgramLayout(
    TargetRequest*  targetReq,
    ProgramLayout*  oldProgramLayout, 
    SubstitutionSet typeSubst,
    DiagnosticSink* sink)
{
    // The goal of the layout specialization step is to take an existing `ProgramLayout`,
    // and add a layout to any parameter(s) that could not be laid out previously, because
    // they had a dependence on generic type parameters that made layout impossible at
    // the time.
    //
    // TODO: It would be far simpler to just "re-do" the entire layout process, just
    // with knowledge of what the global type substitution is, but that would mean that
    // global parameters that come after a generic-dependent parameter might change
    // their location/binding/register depending on what types are plugged in.
    // Our current design preserves the layout for any global parameter that was placed during
    // the initial layout of a program (before the generic arguments were know).
    // It isn't clear that this design choice pays off in practice, since there is  lot
    // of complexity in this function.

    RefPtr<ProgramLayout> newProgramLayout;
    newProgramLayout = new ProgramLayout();
    newProgramLayout->targetProgram = oldProgramLayout->targetProgram;
    newProgramLayout->globalGenericParams = oldProgramLayout->globalGenericParams;

    // The basic idea will be to iterate over the parameters in the old layout,
    // and "pick up where we left off" in terms of allocating registers to things.
    //
    // That means we will look at the existing parameters (that were laid out already)
    // and mark any registers/bytes/bindings/etc. that they occupy as "used" so
    // that the subsequent layout of the generic-dependency parameters will not
    // collide with them.
    //
    // We will use the same kind of context type as the original parameter binding
    // step did, so we initialize its state here:

    auto layoutContext = getInitialLayoutContextForTarget(targetReq, newProgramLayout);
    SLANG_ASSERT(layoutContext.rules);

    SharedParameterBindingContext sharedContext(
        layoutContext.getRulesFamily(),
        newProgramLayout,
        targetReq,
        sink);

    ParameterBindingContext context;
    context.shared = &sharedContext;
    context.layoutContext = layoutContext;

    // We will also need state for laying out any global-scope parameters
    // that include ordinary/uniform data.
    //
    auto oldGlobalStructLayout = getGlobalStructLayout(oldProgramLayout);
    SLANG_ASSERT(oldGlobalStructLayout);

    ScopeLayoutBuilder newGlobalScopeLayoutBuilder;
    newGlobalScopeLayoutBuilder.beginLayout(&context);
    auto& newGlobalStructLayoutInfo = newGlobalScopeLayoutBuilder.m_structLayoutInfo;
    auto newGlobalStructLayout = newGlobalScopeLayoutBuilder.m_structLayout;

    // The initial state for uniform layout will be based on whatever
    // global-scope ordinary/uniform parameters were laid out before.
    // The alignment can be read directly from the old global layout.
    //
    newGlobalStructLayoutInfo.alignment = oldGlobalStructLayout->uniformAlignment;
    newGlobalStructLayoutInfo.size = 0;

    // The remaining information needs to be collected by looking at
    // the individual parameters in the existing layout.
    //
    bool oldAnyUniforms = false;
    for(auto oldVarLayout : oldGlobalStructLayout->fields)
    {
        // If a parameter made use of a global generic parameter, then we would
        // have skipped applying layout to it in the original layout process,
        // and so we should skip it for the process of recovering the existing
        // layout information.
        //
        if (oldVarLayout->FindResourceInfo(LayoutResourceKind::GenericResource))
            continue;

        // Otherwise, we will "reserve" any resources that the parameter was
        // determined to consume.
        //
        // The easy case is any registers/bindings used for textures/sampler/etc.
        // We iterate over the kinds of resources consumed by teh parameter.
        //
        for( auto varResInfo : oldVarLayout->resourceInfos )
        {
            // For each kind of resource consumed the `varResInfo` will tell us
            // the start of the consumed range, whle the type will be needed
            // to tell us the amount of resources consumed.
            //
            if( auto typeResInfo = oldVarLayout->typeLayout->FindResourceInfo(varResInfo.kind) )
            {
                // We will mark the range of resources consumed by theis parameter
                // as "used" so that it cannot be claimed by later parameters.
                //
                auto usedRangeSet = findUsedRangeSetForSpace(&context, varResInfo.space);
                markSpaceUsed(&context, varResInfo.space);
                usedRangeSet->usedResourceRanges[(int)varResInfo.kind].Add(
                    nullptr, // we don't need to track parameter info here
                    varResInfo.index,
                    varResInfo.index + typeResInfo->count);
            }
        }

        // The more subtle case is when the parameter consumes ordinary bytes
        // of uniform (constant buffer) memory, because we do not use the
        // same "used range" model to allocate space for ordinary data.
        //
        // Instead, we simply track the highest byte offset covered by any parameter.
        //
        if (auto varUniformInfo = oldVarLayout->FindResourceInfo(LayoutResourceKind::Uniform))
        {
            oldAnyUniforms = true;

            if( auto typeUniformInfo = oldVarLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
            {
                newGlobalStructLayoutInfo.size = maximum(
                    newGlobalStructLayoutInfo.size,
                    varUniformInfo->index + typeUniformInfo->count);
            }
        }
    }

    // Rather than attempt to re-use the entry-point layout information
    // that was collected in the first pass, we will re-collect the
    // information for entry points from scratch.
    //
    // This ensures that when an entry point makes use of a generic type
    // parameter, the layout of its parameter list strictly follows
    // the declaration order.
    //
    for( auto entryPoint : oldProgramLayout->getProgram()->getEntryPoints() )
    {
        collectEntryPointParameters(&context, entryPoint, typeSubst);
        context.entryPointLayout = nullptr;
    }

    // Now that we've marked thing as being used, we can make a second
    // sweep to compute the requirements of any generic-dependent parameters.
    //
    // Along the way we will build up the new layout for the global-scope
    // structure type, including the offsets of all ordinary/uniform fields.
    //

    bool newAnyUniforms = oldAnyUniforms;
    List<RefPtr<VarLayout>> newVarLayouts;
    Dictionary<RefPtr<VarLayout>, RefPtr<VarLayout>> mapOldLayoutToNew;
    for(auto oldVarLayout : oldGlobalStructLayout->fields)
    {
        // In this pass, the variables that *don't* depend on generic parameters
        // are the easy ones to handle. We can just copy them over to the new layout.
        //
        if(!oldVarLayout->FindResourceInfo(LayoutResourceKind::GenericResource))
        {
            newGlobalStructLayout->fields.Add(oldVarLayout);
            continue;
        }

        // In the case where things are generic-dependent, we need to re-do
        // the type layout process on the type that results from doing
        // substitution with the global generic arguments.
        //
        RefPtr<Type> oldType = oldVarLayout->getTypeLayout()->getType();
        SLANG_ASSERT(oldType);
        RefPtr<Type> newType = oldType->Substitute(typeSubst).as<Type>();

        RefPtr<TypeLayout> newTypeLayout = getTypeLayoutForGlobalShaderParameter(
            &context,
            oldVarLayout->varDecl,
            newType);

        RefPtr<VarLayout> newVarLayout = new VarLayout();
        newVarLayout->varDecl = oldVarLayout->varDecl;
        newVarLayout->stage = oldVarLayout->stage;
        newVarLayout->typeLayout = newTypeLayout;

        newGlobalScopeLayoutBuilder.addParameter(newVarLayout);
        newVarLayouts.Add(newVarLayout);
        mapOldLayoutToNew.Add(oldVarLayout, newVarLayout);

        if(auto uniformInfo = newTypeLayout->FindResourceInfo(LayoutResourceKind::Uniform))
        {
            if(uniformInfo->count != 0)
            {
                newAnyUniforms = true;
                diagnoseGlobalUniform(&sharedContext, newVarLayout->varDecl);
            }
        }

    }
    auto newGlobalScopeVarLayout = newGlobalScopeLayoutBuilder.endLayout();

    // We had better have made a copy of every field in the original layout.
    //
    SLANG_ASSERT(oldGlobalStructLayout->fields.Count() == newGlobalStructLayout->fields.Count());

    // If there were no global-scope uniforms before, but there
    // are now that we've done global substitution, then we
    // need to allocate a global constant buffer to hold them.
    //
    auto newGlobalConstantBufferBinding = maybeAllocateConstantBufferBinding(&context, newAnyUniforms && !oldAnyUniforms);

    // Now we need to "complete" finding for each of the new parameters,
    // which is the step that actually allocates resource to them.
    //
    // Note: we don't support generic-dependent parameters with explicit bindings,
    // so we should probably emit an error message about that in the original
    // layout step.
    //
    for(auto newVarLayout : newVarLayouts)
    {
        completeBindingsForParameter(&context, newVarLayout);
    }

    // One remaining missing step is that the `StructLayout` type maintains
    // a map from variable declarations to their layouts, and in some cases
    // multiple declarations will map to the same layout (because, e.g., the
    // same `cbuffer` was declared in both a vertex and fragment shader file).
    //
    // We need to clone that remapping information over from the old program
    // layout. This is why we created the `mapOldLayoutToNew` mapping in
    // the preceding loop.
    //
    // TODO: This step would be easier if the `StructLayout::mapVarToLayout`
    // dictionary were instead a mapping from variable declaration to the
    // *index* of the corresponding layout in the `fields` array.
    //
    for(auto entry : oldGlobalStructLayout->mapVarToLayout)
    {
        RefPtr<VarLayout> varLayout = entry.Value;
        mapOldLayoutToNew.TryGetValue(varLayout, varLayout);
        newGlobalStructLayout->mapVarToLayout[entry.Key] = varLayout;
    }

    // Just as for the initial computation of layout, we will complete
    // binding for entry-point parameters *after* we have laid out
    // all the global-scope parameters.
    //
    // Note that this includes layout of generic-dependent global scope
    // parameters, so it is possible for entry point uniform parameters
    // to end up with a different register/binding after generic specialization.
    // (There really isn't a great way around that)
    //
    for( auto entryPoint : sharedContext.programLayout->entryPoints )
    {
        auto entryPointParamsLayout = entryPoint->parametersLayout;
        completeBindingsForParameter(&context, entryPointParamsLayout);
    }

    // As a last step we need to set up the binding/offset information
    // for the global scope itself.
    //
    // We will start by copying whatever information was in the old layout.
    //
    {
        auto oldGlobalScopeVarLayout = oldProgramLayout->parametersLayout;
        for( auto oldResInfo : oldGlobalScopeVarLayout->resourceInfos )
        {
            auto newResInfo = newGlobalScopeVarLayout->findOrAddResourceInfo(oldResInfo.kind);
            newResInfo->space = oldResInfo.space;
            newResInfo->kind = oldResInfo.kind;
        }
    }

    // If we had to create a constant buffer to house the global-scope
    // ordinary/uniform data, then we need to make sure to set that
    // information on the global scope.
    //
    if(newGlobalConstantBufferBinding.kind != LayoutResourceKind::None )
    {
        auto resInfo = newGlobalScopeVarLayout->findOrAddResourceInfo(newGlobalConstantBufferBinding.kind);
        resInfo->space = newGlobalConstantBufferBinding.space;
        resInfo->index = newGlobalConstantBufferBinding.index;
    }

    newProgramLayout->parametersLayout = newGlobalScopeVarLayout;
    return newProgramLayout;
}

} // namespace Slang

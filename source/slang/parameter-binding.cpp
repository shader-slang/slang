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
    ParameterInfo* Add(UsedRange range)
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
        ParameterInfo* newParam = range.parameter;
        ParameterInfo* existingParam = nullptr;

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

    ParameterInfo* Add(ParameterInfo* param, UInt begin, UInt end)
    {
        UsedRange range;
        range.parameter = param;
        range.begin = begin;
        range.end = end;
        return Add(range);
    }

    ParameterInfo* Add(ParameterInfo* param, UInt begin, LayoutSize end)
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
    LayoutSize          count;
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

    // What ranges of resource bindings are claimed for particular translation unit?
    // This is only used for varying input/output.
    //
    Dictionary<TranslationUnitRequest*, RefPtr<UsedRangeSet>> translationUnitUsedRangeSets;

    // Which register spaces have been claimed so far?
    UsedRanges usedSpaces;

    // The space to use for auto-generated bindings.
    UInt defaultSpace = 0;

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

    // The entry point that is being processed right now.
    EntryPointLayout*   entryPointLayout = nullptr;

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
    if( auto registerSemantic = dynamic_cast<HLSLRegisterSemantic*>(semantic) )
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
    DeclRef<AggTypeDecl>        declRef,
    List<DeclRef<StructField>>& outFields)
{
    for( auto fieldDeclRef : getMembersOfType<StructField>(declRef) )
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
    if( auto leftType = dynamic_cast<Type*>(left) )
    {
        if( auto rightType = dynamic_cast<Type*>(right) )
        {
            return validateTypesMatch(context, leftType, rightType, stack);
        }
    }

    if( auto leftInt = dynamic_cast<IntVal*>(left) )
    {
        if( auto rightInt = dynamic_cast<IntVal*>(right) )
        {
            return validateIntValuesMatch(context, leftInt, rightInt, stack);
        }
    }

    if( auto leftWitness = dynamic_cast<SubtypeWitness*>(left) )
    {
        if( auto rightWitness = dynamic_cast<SubtypeWitness*>(right) )
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
        if(auto leftGlobalGeneric = ll.As<GlobalGenericParamSubstitution>())
        {
            ll = leftGlobalGeneric->outer;
            continue;
        }
        if(auto rightGlobalGeneric = rr.As<GlobalGenericParamSubstitution>())
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

        if(auto leftGeneric = leftSubst.As<GenericSubstitution>())
        {
            if(auto rightGeneric = rightSubst.As<GenericSubstitution>())
            {
                if(validateGenericSubstitutionsMatch(context, leftGeneric, rightGeneric, stack))
                {
                    continue;
                }
            }
        }
        else if(auto leftThisType = leftSubst.As<ThisTypeSubstitution>())
        {
            if(auto rightThisType = rightSubst.As<ThisTypeSubstitution>())
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

    if( auto leftDeclRefType = left->As<DeclRefType>() )
    {
        if( auto rightDeclRefType = right->As<DeclRefType>() )
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
                if( auto leftStructDeclRef = leftDeclRef.As<AggTypeDecl>() )
                {
                    if( auto rightStructDeclRef = rightDeclRef.As<AggTypeDecl>() )
                    {
                        List<DeclRef<StructField>> leftFields;
                        List<DeclRef<StructField>> rightFields;

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
    else if( auto leftArrayType = left->As<ArrayExpressionType>() )
    {
        if( auto rightArrayType = right->As<ArrayExpressionType>() )
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
    if( varDecl->HasModifier<PushConstantAttribute>() && type->As<ConstantBufferType>() )
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

    if( varDecl->HasModifier<ShaderRecordNVLayoutModifier>() && type->As<ConstantBufferType>() )
    {
        return CreateTypeLayout(
            layoutContext.with(rules->getShaderRecordConstantBufferRules()),
            type);
    }

    // We want to check for a constant-buffer type with a `push_constant` layout
    // qualifier before we move on to anything else.
    if (varDecl->HasModifier<PushConstantAttribute>() && type->As<ConstantBufferType>())
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
    Stage                               stage = Stage::Unknown;
    bool                                isSampleRate = false;
    SourceLoc                           loc;
};


static RefPtr<TypeLayout> processEntryPointParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>          type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout);

static void collectGlobalScopeGLSLVaryingParameter(
    ParameterBindingContext*        context,
    RefPtr<VarDeclBase>             varDecl,
    RefPtr<Type>                    effectiveType,
    EntryPointParameterDirection    direction)
{
    int defaultSemanticIndex = 0;

    EntryPointParameterState state;
    state.directionMask = direction;
    state.ioSemanticIndex = &defaultSemanticIndex;
    state.stage = context->stage;
    state.loc = varDecl->loc;

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
    context->shared->programLayout->globalGenericParamsMap[layout->decl->getName()->text] = layout.Ptr();
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
        auto& bindingInfo = parameterInfo->bindingInfo[(int)kind];
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
        auto& bindingInfo = parameterInfo->bindingInfo[(int)kind];
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
            bindingInfo.index = usedRangeSet->usedResourceRanges[(int)kind].Allocate(parameterInfo, count.getFiniteValue());

            bindingInfo.space = space;
        }
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

static RefPtr<TypeLayout> processEntryPointParameterDecl(
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
    return processEntryPointParameter(context, type, state, varLayout);
}

static RefPtr<TypeLayout> processEntryPointParameter(
    ParameterBindingContext*        context,
    RefPtr<Type>                    type,
    EntryPointParameterState const& state,
    RefPtr<VarLayout>               varLayout)
{
    if (varLayout)
    {
        varLayout->stage = state.stage;
    }

    // The default handling of varying parameters should not apply
    // to geometry shader output streams; they have their own special rules.
    if( auto gsStreamType = type->As<HLSLStreamOutputType>() )
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

        auto elementTypeLayout = processEntryPointParameter(context, elementType, elementState, nullptr);

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

                auto fieldTypeLayout = processEntryPointParameterDecl(
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
        else if (auto globalGenericParam = declRef.As<GlobalGenericParamDecl>())
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
    SubstitutionSet                 typeSubst)
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

    context->entryPointLayout = entryPointLayout;


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
    state.stage = entryPoint->getStage();

    for( auto m : entryPointFuncDecl->Members )
    {
        auto paramDecl = m.As<VarDeclBase>();
        if(!paramDecl)
            continue;

        // We have an entry-point parameter, and need to figure out what to do with it.
        state.loc = paramDecl->loc;

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

        auto paramTypeLayout = processEntryPointParameterDecl(
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
            paramVarLayout->findOrAddResourceInfo(rr.kind)->index = entryPointRes->count.getFiniteValue();
            entryPointRes->count += rr.count;
        }

        entryPointLayout->fields.Add(paramVarLayout);
        entryPointLayout->mapVarToLayout.Add(paramDecl, paramVarLayout);
    }

    // If we have a non-`void` output type for the entry point, then process it as
    // an output parameter.
    auto resultType = entryPointFuncDecl->ReturnType.type;
    if( !resultType->Equals(resultType->getSession()->getVoidType()) )
    {
        state.loc = entryPointFuncDecl->loc;
        state.directionMask = kEntryPointParameterDirection_Output;

        RefPtr<VarLayout> resultLayout = new VarLayout();

        auto resultTypeLayout = processEntryPointParameterDecl(
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
                resultLayout->findOrAddResourceInfo(rr.kind)->index = entryPointRes->count.getFiniteValue();
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
    // TODO: now that we've dropped official GLSL support,
    // we probably should drop this as well.
    //
    if( translationUnit->sourceLanguage == SourceLanguage::GLSL )
    {
        if( translationUnit->entryPoints.Count() == 1 )
        {
            return translationUnit->entryPoints[0]->getStage();
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
            context->stage = entryPoint->getStage();
            collectEntryPointParameters(context, entryPoint.Ptr(), SubstitutionSet());
        }
        context->entryPointLayout = nullptr;
    }

    // Now collect parameters from loaded modules
    for (auto& loadedModule : request->loadedModulesList)
    {
        collectModuleParameters(context, loadedModule->moduleDecl.Ptr());
    }
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
    programLayout->targetRequest = targetReq;

    targetReq->layout = programLayout;

    // Create a context to hold shared state during the process
    // of generating parameter bindings
    SharedParameterBindingContext sharedContext;
    sharedContext.compileRequest = compileReq;
    sharedContext.defaultLayoutRules = layoutContext.getRulesFamily();
    sharedContext.programLayout = programLayout;
    sharedContext.targetRequest = targetReq;

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
            break;
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
    ParameterBindingInfo globalConstantBufferBinding;
    globalConstantBufferBinding.index = 0;
    globalConstantBufferBinding.space = 0;
    if( needDefaultConstantBuffer )
    {
        // TODO: this logic is only correct for D3D targets, where
        // global-scope uniforms get wrapped into a constant buffer.

        UInt space = sharedContext.defaultSpace;
        auto usedRangeSet = findUsedRangeSetForSpace(&context, space);

        globalConstantBufferBinding.index =
            usedRangeSet->usedResourceRanges[
                (int)LayoutResourceKind::ConstantBuffer].Allocate(nullptr, 1);

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
        LayoutSize uniformSize = layoutInfo ? layoutInfo->count : 0;
        if( uniformSize != 0 )
        {
            // Make sure uniform fields get laid out properly...

            UniformLayoutInfo fieldInfo(
                uniformSize,
                firstVarLayout->typeLayout->uniformAlignment);

            LayoutSize uniformOffset = globalScopeRules->AddStructField(
                &structLayoutInfo,
                fieldInfo);

            for( auto& varLayout : parameterInfo->varLayouts )
            {
                varLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset.getFiniteValue();
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
    if( needDefaultConstantBuffer )
    {
        auto globalConstantBufferLayout = createParameterGroupTypeLayout(
            layoutContext,
            nullptr,
            globalScopeRules,
            globalScopeRules->GetObjectLayout(ShaderParameterKind::ConstantBuffer),
            globalScopeStructLayout);

        globalScopeLayout = globalConstantBufferLayout;
    }

    // We now have a bunch of layout information, which we should
    // record into a suitable object that represents the program
    RefPtr<VarLayout> globalVarLayout = new VarLayout();
    globalVarLayout->typeLayout = globalScopeLayout;
    if (needDefaultConstantBuffer)
    {
        auto cbInfo = globalVarLayout->findOrAddResourceInfo(LayoutResourceKind::ConstantBuffer);
        cbInfo->space = globalConstantBufferBinding.space;
        cbInfo->index = globalConstantBufferBinding.index;
    }
    programLayout->globalScopeLayout = globalVarLayout;
}

StructTypeLayout* getGlobalStructLayout(
    ProgramLayout*  programLayout);

RefPtr<ProgramLayout> specializeProgramLayout(
    TargetRequest * targetReq,
    ProgramLayout* programLayout, 
    SubstitutionSet typeSubst)
{
    RefPtr<ProgramLayout> newProgramLayout;
    newProgramLayout = new ProgramLayout();
    newProgramLayout->targetRequest = targetReq;
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
    sharedContext.targetRequest = targetReq;

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
        context.entryPointLayout = nullptr;
    }

    auto constantBufferRules = context.getRulesFamily()->getConstantBufferRules();
    structLayout->rules = constantBufferRules;
    structLayout->fields.SetSize(globalStructLayout->fields.Count());
    UniformLayoutInfo structLayoutInfo;
    structLayoutInfo.alignment = globalStructLayout->uniformAlignment;
    structLayoutInfo.size = 0;
    bool anyUniforms = false;
    Dictionary<RefPtr<VarLayout>, RefPtr<VarLayout>> varLayoutMapping;
    for (uint32_t varId = 0; varId < globalStructLayout->fields.Count(); varId++)
    {
        auto &varLayout = globalStructLayout->fields[varId];
        // To recover layout context, we skip generic resources in the first pass
        if (varLayout->FindResourceInfo(LayoutResourceKind::GenericResource))
            continue;

        if (auto uniformInfo = varLayout->FindResourceInfo(LayoutResourceKind::Uniform))
        {
            anyUniforms = true;

            if( auto tUniformInfo = varLayout->typeLayout->FindResourceInfo(LayoutResourceKind::Uniform) )
            {
                structLayoutInfo.size = maximum(structLayoutInfo.size, uniformInfo->index + tUniformInfo->count);
            }
        }
        for( auto resInfo : varLayout->resourceInfos )
        {
            if( auto tresInfo = varLayout->typeLayout->FindResourceInfo(resInfo.kind) )
            {
                auto usedRangeSet = findUsedRangeSetForSpace(&context, resInfo.space);
                markSpaceUsed(&context, resInfo.space);
                usedRangeSet->usedResourceRanges[(int)resInfo.kind].Add(
                    nullptr, // we don't need to track parameter info here
                    resInfo.index,
                    resInfo.index + tresInfo->count);
            }
        }

        structLayout->fields[varId] = varLayout;
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
    for (uint32_t varId = 0; varId < globalStructLayout->fields.Count(); varId++)
    {
        auto &varLayout = globalStructLayout->fields[varId];
        if (varLayout->typeLayout->FindResourceInfo(LayoutResourceKind::GenericResource))
        {
            RefPtr<Type> newType = varLayout->typeLayout->type->Substitute(typeSubst).As<Type>();
            RefPtr<TypeLayout> newTypeLayout = CreateTypeLayout(
                layoutContext.with(constantBufferRules),
                newType);
            auto layoutInfo = newTypeLayout->FindResourceInfo(LayoutResourceKind::Uniform);
            LayoutSize uniformSize = layoutInfo ? layoutInfo->count : 0;
            if (uniformSize != 0)
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
            newVarLayout->stage = varLayout->stage;
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
                LayoutSize uniformOffset = layoutContext.getRulesFamily()->getConstantBufferRules()->AddStructField(
                    &structLayoutInfo,
                    fieldInfo);
                newVarLayout->findOrAddResourceInfo(LayoutResourceKind::Uniform)->index = uniformOffset.getFiniteValue();
                anyUniforms = true;
            }
            structLayout->fields[varId] = newVarLayout;
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

// slang-check-conversion.cpp
#include "slang-check-impl.h"

// This file contains semantic-checking logic for dealing
// with conversion (both implicit and explicit) of expressions
// from one type to another.
//
// Type conversion is also the point at which a C-style initializer
// list (e.g., `float4 a = { 1, 2, 3, 4 };`) is validated against
// the desired type, so this file also contains all of the logic
// associated with validating initializer lists.

namespace Slang
{
    ConversionCost SemanticsVisitor::getImplicitConversionCost(
        Decl* decl)
    {
        if(auto modifier = decl->findModifier<ImplicitConversionModifier>())
        {
            return modifier->cost;
        }

        return kConversionCost_Explicit;
    }

    bool SemanticsVisitor::isEffectivelyScalarForInitializerLists(
        RefPtr<Type>    type)
    {
        if(as<ArrayExpressionType>(type)) return false;
        if(as<VectorExpressionType>(type)) return false;
        if(as<MatrixExpressionType>(type)) return false;

        if(as<BasicExpressionType>(type))
        {
            return true;
        }

        if(as<ResourceType>(type))
        {
            return true;
        }
        if(as<UntypedBufferResourceType>(type))
        {
            return true;
        }
        if(as<SamplerStateType>(type))
        {
            return true;
        }

        if(auto declRefType = as<DeclRefType>(type))
        {
            if(as<StructDecl>(declRefType->declRef))
                return false;
        }

        return true;
    }

    bool SemanticsVisitor::shouldUseInitializerDirectly(
        RefPtr<Type>    toType,
        RefPtr<Expr>    fromExpr)
    {
        // A nested initializer list should always be used directly.
        //
        if(as<InitializerListExpr>(fromExpr))
        {
            return true;
        }

        // If the desired type is a scalar, then we should always initialize
        // directly, since it isn't an aggregate.
        //
        if(isEffectivelyScalarForInitializerLists(toType))
            return true;

        // If the type we are initializing isn't effectively scalar,
        // but the initialization expression *is*, then it doesn't
        // seem like direct initialization is intended.
        //
        if(isEffectivelyScalarForInitializerLists(fromExpr->type))
            return false;

        // Once the above cases are handled, the main thing
        // we want to check for is whether a direct initialization
        // is possible (a type conversion exists).
        //
        return canCoerce(toType, fromExpr->type);
    }

    bool SemanticsVisitor::_readValueFromInitializerList(
        RefPtr<Type>                toType,
        RefPtr<Expr>*               outToExpr,
        RefPtr<InitializerListExpr> fromInitializerListExpr,
        UInt                       &ioInitArgIndex)
    {
        // First, we will check if we have run out of arguments
        // on the initializer list.
        //
        UInt initArgCount = fromInitializerListExpr->args.getCount();
        if(ioInitArgIndex >= initArgCount)
        {
            // If we are at the end of the initializer list,
            // then our ability to read an argument depends
            // on whether the type we are trying to read
            // is default-initializable.
            //
            // For now, we will just pretend like everything
            // is default-initializable and move along.
            return true;
        }

        // Okay, we have at least one initializer list expression,
        // so we will look at the next expression and decide
        // whether to use it to initialize the desired type
        // directly (possibly via casts), or as the first sub-expression
        // for aggregate initialization.
        //
        auto firstInitExpr = fromInitializerListExpr->args[ioInitArgIndex];
        if(shouldUseInitializerDirectly(toType, firstInitExpr))
        {
            ioInitArgIndex++;
            return _coerce(
                toType,
                outToExpr,
                firstInitExpr->type,
                firstInitExpr,
                nullptr);
        }

        // If there is somehow an error in one of the initialization
        // expressions, then everything could be thrown off and we
        // shouldn't keep trying to read arguments.
        //
        if( IsErrorExpr(firstInitExpr) )
        {
            // Stop reading arguments, as if we'd reached
            // the end of the list.
            //
            ioInitArgIndex = initArgCount;
            return true;
        }

        // The fallback case is to recursively read the
        // type from the same list as an aggregate.
        //
        return _readAggregateValueFromInitializerList(
            toType,
            outToExpr,
            fromInitializerListExpr,
            ioInitArgIndex);
    }

    bool SemanticsVisitor::_readAggregateValueFromInitializerList(
        RefPtr<Type>                inToType,
        RefPtr<Expr>*               outToExpr,
        RefPtr<InitializerListExpr> fromInitializerListExpr,
        UInt                       &ioArgIndex)
    {
        auto toType = inToType;
        UInt argCount = fromInitializerListExpr->args.getCount();

        // In the case where we need to build a result expression,
        // we will collect the new arguments here
        List<RefPtr<Expr>> coercedArgs;

        if(isEffectivelyScalarForInitializerLists(toType))
        {
            // For any type that is effectively a non-aggregate,
            // we expect to read a single value from the initializer list
            //
            if(ioArgIndex < argCount)
            {
                auto arg = fromInitializerListExpr->args[ioArgIndex++];
                return _coerce(
                    toType,
                    outToExpr,
                    arg->type,
                    arg,
                    nullptr);
            }
            else
            {
                // If there wasn't an initialization
                // expression to be found, then we need
                // to perform default initialization here.
                //
                // We will let this case come through the front-end
                // as an `InitializerListExpr` with zero arguments,
                // and then have the IR generation logic deal with
                // synthesizing default values.
            }
        }
        else if (auto toVecType = as<VectorExpressionType>(toType))
        {
            auto toElementCount = toVecType->elementCount;
            auto toElementType = toVecType->elementType;

            UInt elementCount = 0;
            if (auto constElementCount = as<ConstantIntVal>(toElementCount))
            {
                elementCount = (UInt) constElementCount->value;
            }
            else
            {
                // We don't know the element count statically,
                // so what are we supposed to be doing?
                //
                if(outToExpr)
                {
                    getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForVectorOfUnknownSize, toElementCount);
                }
                return false;
            }

            for(UInt ee = 0; ee < elementCount; ++ee)
            {
                RefPtr<Expr> coercedArg;
                bool argResult = _readValueFromInitializerList(
                    toElementType,
                    outToExpr ? &coercedArg : nullptr,
                    fromInitializerListExpr,
                    ioArgIndex);

                // No point in trying further if any argument fails
                if(!argResult)
                    return false;

                if( coercedArg )
                {
                    coercedArgs.add(coercedArg);
                }
            }
        }
        else if(auto toArrayType = as<ArrayExpressionType>(toType))
        {
            // TODO(tfoley): If we can compute the size of the array statically,
            // then we want to check that there aren't too many initializers present

            auto toElementType = toArrayType->baseType;

            if(auto toElementCount = toArrayType->arrayLength)
            {
                // In the case of a sized array, we need to check that the number
                // of elements being initialized matches what was declared.
                //
                UInt elementCount = 0;
                if (auto constElementCount = as<ConstantIntVal>(toElementCount))
                {
                    elementCount = (UInt) constElementCount->value;
                }
                else
                {
                    // We don't know the element count statically,
                    // so what are we supposed to be doing?
                    //
                    if(outToExpr)
                    {
                        getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForArrayOfUnknownSize, toElementCount);
                    }
                    return false;
                }

                for(UInt ee = 0; ee < elementCount; ++ee)
                {
                    RefPtr<Expr> coercedArg;
                    bool argResult = _readValueFromInitializerList(
                        toElementType,
                        outToExpr ? &coercedArg : nullptr,
                        fromInitializerListExpr,
                        ioArgIndex);

                    // No point in trying further if any argument fails
                    if(!argResult)
                        return false;

                    if( coercedArg )
                    {
                        coercedArgs.add(coercedArg);
                    }
                }
            }
            else
            {
                // In the case of an unsized array type, we will use the
                // number of arguments to the initializer to determine
                // the element count.
                //
                UInt elementCount = 0;
                while(ioArgIndex < argCount)
                {
                    RefPtr<Expr> coercedArg;
                    bool argResult = _readValueFromInitializerList(
                        toElementType,
                        outToExpr ? &coercedArg : nullptr,
                        fromInitializerListExpr,
                        ioArgIndex);

                    // No point in trying further if any argument fails
                    if(!argResult)
                        return false;

                    elementCount++;

                    if( coercedArg )
                    {
                        coercedArgs.add(coercedArg);
                    }
                }

                // We have a new type for the conversion, based on what
                // we learned.
                toType = getSession()->getArrayType(
                    toElementType,
                    new ConstantIntVal(elementCount));
            }
        }
        else if(auto toMatrixType = as<MatrixExpressionType>(toType))
        {
            // In the general case, the initializer list might comprise
            // both vectors and scalars.
            //
            // The traditional HLSL compilers treat any vectors in
            // the initializer list exactly equivalent to their sequence
            // of scalar elements, and don't care how this might, or
            // might not, align with the rows of the matrix.
            //
            // We will draw a line in the sand and say that an initializer
            // list for a matrix will act as if the matrix type were an
            // array of vectors for the rows.


            UInt rowCount = 0;
            auto toRowType = createVectorType(
                toMatrixType->getElementType(),
                toMatrixType->getColumnCount());

            if (auto constRowCount = as<ConstantIntVal>(toMatrixType->getRowCount()))
            {
                rowCount = (UInt) constRowCount->value;
            }
            else
            {
                // We don't know the element count statically,
                // so what are we supposed to be doing?
                //
                if(outToExpr)
                {
                    getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForMatrixOfUnknownSize, toMatrixType->getRowCount());
                }
                return false;
            }

            for(UInt rr = 0; rr < rowCount; ++rr)
            {
                RefPtr<Expr> coercedArg;
                bool argResult = _readValueFromInitializerList(
                    toRowType,
                    outToExpr ? &coercedArg : nullptr,
                    fromInitializerListExpr,
                    ioArgIndex);

                // No point in trying further if any argument fails
                if(!argResult)
                    return false;

                if( coercedArg )
                {
                    coercedArgs.add(coercedArg);
                }
            }
        }
        else if(auto toDeclRefType = as<DeclRefType>(toType))
        {
            auto toTypeDeclRef = toDeclRefType->declRef;
            if(auto toStructDeclRef = toTypeDeclRef.as<StructDecl>())
            {
                // Trying to initialize a `struct` type given an initializer list.
                // We will go through the fields in order and try to match them
                // up with initializer arguments.
                //
                for(auto fieldDeclRef : getMembersOfType<VarDecl>(toStructDeclRef, MemberFilterStyle::Instance))
                {
                    RefPtr<Expr> coercedArg;
                    bool argResult = _readValueFromInitializerList(
                        GetType(fieldDeclRef),
                        outToExpr ? &coercedArg : nullptr,
                        fromInitializerListExpr,
                        ioArgIndex);

                    // No point in trying further if any argument fails
                    if(!argResult)
                        return false;

                    if( coercedArg )
                    {
                        coercedArgs.add(coercedArg);
                    }
                }
            }
        }
        else
        {
            // We shouldn't get to this case in practice,
            // but just in case we'll consider an initializer
            // list invalid if we are trying to read something
            // off of it that wasn't handled by the cases above.
            //
            if(outToExpr)
            {
                getSink()->diagnose(fromInitializerListExpr, Diagnostics::cannotUseInitializerListForType, inToType);
            }
            return false;
        }

        // We were able to coerce all the arguments given, and so
        // we need to construct a suitable expression to remember the result
        //
        if(outToExpr)
        {
            auto toInitializerListExpr = new InitializerListExpr();
            toInitializerListExpr->loc = fromInitializerListExpr->loc;
            toInitializerListExpr->type = QualType(toType);
            toInitializerListExpr->args = coercedArgs;

            *outToExpr = toInitializerListExpr;
        }

        return true;
    }

    bool SemanticsVisitor::_coerceInitializerList(
        RefPtr<Type>                toType,
        RefPtr<Expr>*               outToExpr,
        RefPtr<InitializerListExpr> fromInitializerListExpr)
    {
        UInt argCount = fromInitializerListExpr->args.getCount();
        UInt argIndex = 0;

        // TODO: we should handle the special case of `{0}` as an initializer
        // for arbitrary `struct` types here.

        if(!_readAggregateValueFromInitializerList(toType, outToExpr, fromInitializerListExpr, argIndex))
            return false;

        if(argIndex != argCount)
        {
            if( outToExpr )
            {
                getSink()->diagnose(fromInitializerListExpr, Diagnostics::tooManyInitializers, argIndex, argCount);
            }
        }

        return true;
    }

    bool SemanticsVisitor::_failedCoercion(
        RefPtr<Type>    toType,
        RefPtr<Expr>*   outToExpr,
        RefPtr<Expr>    fromExpr)
    {
        if(outToExpr)
        {
            // As a special case, if the expression we are trying to convert
            // from is overloaded (implying an ambiguous reference), then we
            // will try to produce a more appropriately tailored error message.
            //
            auto fromType = fromExpr->type.type;
            if( as<OverloadGroupType>(fromType) )
            {
                diagnoseAmbiguousReference(fromExpr);
            }
            else
            {
                getSink()->diagnose(fromExpr->loc, Diagnostics::typeMismatch, toType, fromExpr->type);
            }
        }
        return false;
    }

    bool SemanticsVisitor::_coerce(
        RefPtr<Type>    toType,
        RefPtr<Expr>*   outToExpr,
        RefPtr<Type>    fromType,
        RefPtr<Expr>    fromExpr,
        ConversionCost* outCost)
    {
        // An important and easy case is when the "to" and "from" types are equal.
        //
        if( toType->equals(fromType) )
        {
            if(outToExpr)
                *outToExpr = fromExpr;
            if(outCost)
                *outCost = kConversionCost_None;
            return true;
        }

        // Another important case is when either the "to" or "from" type
        // represents an error. In such a case we must have already
        // reporeted the error, so it is better to allow the conversion
        // to pass than to report a "cascading" error that might not
        // make any sense.
        //
        if(as<ErrorType>(toType) || as<ErrorType>(fromType))
        {
            if(outToExpr)
                *outToExpr = CreateImplicitCastExpr(toType, fromExpr);
            if(outCost)
                *outCost = kConversionCost_None;
            return true;
        }

        // Coercion from an initializer list is allowed for many types,
        // so we will farm that out to its own subroutine.
        //
        if( auto fromInitializerListExpr = as<InitializerListExpr>(fromExpr))
        {
            if( !_coerceInitializerList(
                toType,
                outToExpr,
                fromInitializerListExpr) )
            {
                return false;
            }

            // For now, we treat coercion from an initializer list
            // as having  no cost, so that all conversions from initializer
            // lists are equally valid. This is fine given where initializer
            // lists are allowed to appear now, but might need to be made
            // more strict if we allow for initializer lists in more
            // places in the language (e.g., as function arguments).
            //
            if(outCost)
            {
                *outCost = kConversionCost_None;
            }

            return true;
        }

        // If we are casting to an interface type, then that will succeed
        // if the "from" type conforms to the interface.
        //
        if (auto toDeclRefType = as<DeclRefType>(toType))
        {
            auto toTypeDeclRef = toDeclRefType->declRef;
            if (auto interfaceDeclRef = toTypeDeclRef.as<InterfaceDecl>())
            {
                if(auto witness = tryGetInterfaceConformanceWitness(fromType, interfaceDeclRef))
                {
                    if (outToExpr)
                        *outToExpr = createCastToInterfaceExpr(toType, fromExpr, witness);
                    if (outCost)
                        *outCost = kConversionCost_CastToInterface;
                    return true;
                }
            }
        }

        // We allow implicit conversion of a parameter group type like
        // `ConstantBuffer<X>` or `ParameterBlock<X>` to its element
        // type `X`.
        //
        if(auto fromParameterGroupType = as<ParameterGroupType>(fromType))
        {
            auto fromElementType = fromParameterGroupType->getElementType();

            // If we convert, e.g., `ConstantBuffer<A> to `A`, we will allow
            // subsequent conversion of `A` to `B` if such a conversion
            // is possible.
            //
            ConversionCost subCost = kConversionCost_None;

            RefPtr<DerefExpr> derefExpr;
            if(outToExpr)
            {
                derefExpr = new DerefExpr();
                derefExpr->base = fromExpr;
                derefExpr->type = QualType(fromElementType);
            }

            if(!_coerce(
                toType,
                outToExpr,
                fromElementType,
                derefExpr,
                &subCost))
            {
                return false;
            }

            if(outCost)
                *outCost = subCost + kConversionCost_ImplicitDereference;
            return true;
        }

        // The main general-purpose approach for conversion is
        // using suitable marked initializer ("constructor")
        // declarations on the target type.
        //
        // This is treated as a form of overload resolution,
        // since we are effectively forming an overloaded
        // call to one of the initializers in the target type.

        OverloadResolveContext overloadContext;
        overloadContext.disallowNestedConversions = true;
        overloadContext.argCount = 1;
        overloadContext.argTypes = &fromType;

        overloadContext.originalExpr = nullptr;
        if(fromExpr)
        {
            overloadContext.loc = fromExpr->loc;
            overloadContext.funcLoc = fromExpr->loc;
            overloadContext.args = &fromExpr;
        }

        overloadContext.baseExpr = nullptr;
        overloadContext.mode = OverloadResolveContext::Mode::JustTrying;

        AddTypeOverloadCandidates(toType, overloadContext);

        // After all of the overload candidates have been added
        // to the context and processed, we need to see whether
        // there was one best overload or not.
        //
        if(overloadContext.bestCandidates.getCount() != 0)
        {
            // In this case there were multiple equally-good candidates to call.
            //
            // We will start by checking if the candidates are
            // even applicable, because if not, then we shouldn't
            // consider the conversion as possible.
            //
            if(overloadContext.bestCandidates[0].status != OverloadCandidate::Status::Applicable)
                return _failedCoercion(toType, outToExpr, fromExpr);

            // If all of the candidates in `bestCandidates` are applicable,
            // then we have an ambiguity.
            //
            // We will compute a nominal conversion cost as the minimum over
            // all the conversions available.
            //
            ConversionCost bestCost = kConversionCost_Explicit;
            for(auto candidate : overloadContext.bestCandidates)
            {
                ConversionCost candidateCost = getImplicitConversionCost(
                    candidate.item.declRef.getDecl());

                if(candidateCost < bestCost)
                    bestCost = candidateCost;
            }

            // Conceptually, we want to treat the conversion as
            // possible, but report it as ambiguous if we actually
            // need to reify the result as an expression.
            //
            if(outToExpr)
            {
                getSink()->diagnose(fromExpr, Diagnostics::ambiguousConversion, fromType, toType);

                *outToExpr = CreateErrorExpr(fromExpr);
            }

            if(outCost)
                *outCost = bestCost;

            return true;
        }
        else if(overloadContext.bestCandidate)
        {
            // If there is a single best candidate for conversion,
            // then we want to use it.
            //
            // It is possible that there was a single best candidate,
            // but it wasn't actually usable, so we will check for
            // that case first.
            //
            if(overloadContext.bestCandidate->status != OverloadCandidate::Status::Applicable)
                return _failedCoercion(toType, outToExpr, fromExpr);

            // Next, we need to look at the implicit conversion
            // cost associated with the initializer we are invoking.
            //
            ConversionCost cost = getImplicitConversionCost(
                    overloadContext.bestCandidate->item.declRef.getDecl());;

            // If the cost is too high to be usable as an
            // implicit conversion, then we will report the
            // conversion as possible (so that an overload involving
            // this conversion will be selected over one without),
            // but then emit a diagnostic when actually reifying
            // the result expression.
            //
            if( cost >= kConversionCost_Explicit )
            {
                if( outToExpr )
                {
                    getSink()->diagnose(fromExpr, Diagnostics::typeMismatch, toType, fromType);
                    getSink()->diagnose(fromExpr, Diagnostics::noteExplicitConversionPossible, fromType, toType);
                }
            }

            if(outCost)
                *outCost = cost;

            if(outToExpr)
            {
                // The logic here is a bit ugly, to deal with the fact that
                // `CompleteOverloadCandidate` will, left to its own devices,
                // construct a vanilla `InvokeExpr` to represent the call
                // to the initializer we found, while we *want* it to
                // create some variety of `ImplicitCastExpr`.
                //
                // Now, it just so happens that `CompleteOverloadCandidate`
                // will use the "original" expression if one is available,
                // so we'll create one and initialize it here.
                // We fill in the location and arguments, but not the
                // base expression (the callee), since that will come
                // from the selected overload candidate.
                //
                auto castExpr = createImplicitCastExpr();
                castExpr->loc = fromExpr->loc;
                castExpr->arguments.add(fromExpr);
                //
                // Next we need to set our cast expression as the "original"
                // expression and then complete the overload process.
                //
                overloadContext.originalExpr = castExpr;
                *outToExpr = CompleteOverloadCandidate(overloadContext, *overloadContext.bestCandidate);
                //
                // However, the above isn't *quite* enough, because
                // the process of completing the overload candidate
                // might overwrite the argument list that was passed
                // in to overload resolution, and in this case that
                // "argument list" was just a pointer to `fromExpr`.
                //
                // That means we need to clear the argument list and
                // reload it from `fromExpr` to make sure that we
                // got the arguments *after* any transformations
                // were applied.
                // For right now this probably doesn't matter,
                // because we don't allow nested implicit conversions,
                // but I'd rather play it safe.
                //
                castExpr->arguments.clear();
                castExpr->arguments.add(fromExpr);
            }

            return true;
        }

        return _failedCoercion(toType, outToExpr, fromExpr);
    }

    bool SemanticsVisitor::canCoerce(
        RefPtr<Type>    toType,
        RefPtr<Type>    fromType,
        ConversionCost* outCost)
    {
        // As an optimization, we will maintain a cache of conversion results
        // for basic types such as scalars and vectors.
        //
        
        bool shouldAddToCache = false;
        ConversionCost cost;
        TypeCheckingCache* typeCheckingCache = getSession()->getTypeCheckingCache();

        BasicTypeKeyPair cacheKey;
        cacheKey.type1 = makeBasicTypeKey(toType.Ptr());
        cacheKey.type2 = makeBasicTypeKey(fromType.Ptr());
    
        if( cacheKey.isValid())
        {
            if (typeCheckingCache->conversionCostCache.TryGetValue(cacheKey, cost))
            {
                if (outCost)
                    *outCost = cost;
                return cost != kConversionCost_Impossible;
            }
            else
                shouldAddToCache = true;
        }

        // If there was no suitable entry in the cache,
        // then we fall back to the general-purpose
        // conversion checking logic.
        //
        // Note that we are passing in `nullptr` as
        // the output expression to be constructed,
        // which suppresses emission of any diagnostics
        // during the coercion process.
        //
        bool rs = _coerce(
            toType,
            nullptr,
            fromType,
            nullptr,
            &cost);

        if (outCost)
            *outCost = cost;

        if (shouldAddToCache)
        {
            if (!rs)
                cost = kConversionCost_Impossible;
            typeCheckingCache->conversionCostCache[cacheKey] = cost;
        }

        return rs;
    }

    RefPtr<TypeCastExpr> SemanticsVisitor::createImplicitCastExpr()
    {
        return new ImplicitCastExpr();
    }

    RefPtr<Expr> SemanticsVisitor::CreateImplicitCastExpr(
        RefPtr<Type>	toType,
        RefPtr<Expr>	fromExpr)
    {
        RefPtr<TypeCastExpr> castExpr = createImplicitCastExpr();

        auto typeType = getTypeType(toType);

        auto typeExpr = new SharedTypeExpr();
        typeExpr->type.type = typeType;
        typeExpr->base.type = toType;

        castExpr->loc = fromExpr->loc;
        castExpr->functionExpr = typeExpr;
        castExpr->type = QualType(toType);
        castExpr->arguments.add(fromExpr);
        return castExpr;
    }

    RefPtr<Expr> SemanticsVisitor::createCastToInterfaceExpr(
        RefPtr<Type>    toType,
        RefPtr<Expr>    fromExpr,
        RefPtr<Val>     witness)
    {
        RefPtr<CastToInterfaceExpr> expr = new CastToInterfaceExpr();
        expr->loc = fromExpr->loc;
        expr->type = QualType(toType);
        expr->valueArg = fromExpr;
        expr->witnessArg = witness;
        return expr;
    }

    RefPtr<Expr> SemanticsVisitor::coerce(
        RefPtr<Type>    toType,
        RefPtr<Expr>    fromExpr)
    {
        RefPtr<Expr> expr;
        if (!_coerce(
            toType,
            &expr,
            fromExpr->type.Ptr(),
            fromExpr.Ptr(),
            nullptr))
        {
            // Note(tfoley): We don't call `CreateErrorExpr` here, because that would
            // clobber the type on `fromExpr`, and an invariant here is that coercion
            // really shouldn't *change* the expression that is passed in, but should
            // introduce new AST nodes to coerce its value to a different type...
            return CreateImplicitCastExpr(
                getSession()->getErrorType(),
                fromExpr);
        }
        return expr;
    }

    bool SemanticsVisitor::canConvertImplicitly(
        RefPtr<Type> toType,
        RefPtr<Type> fromType)
    {
        // Can we convert at all?
        ConversionCost conversionCost;
        if(!canCoerce(toType, fromType, &conversionCost))
            return false;

        // Is the conversion cheap enough to be done implicitly?
        if(conversionCost >= kConversionCost_GeneralConversion)
            return false;

        return true;
    }
}

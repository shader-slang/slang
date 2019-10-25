// slang-check-shader.cpp
#include "slang-check-impl.h"

// This file encapsulates semantic checking logic primarily
// related to shaders, including validating entry points,
// enumerating specialization parameters, and validating
// attempts to specialize shader code.

#include "slang-lookup.h"

namespace Slang
{
    static bool isValidThreadDispatchIDType(Type* type)
    {
        // Can accept a single int/unit
        {
            auto basicType = as<BasicExpressionType>(type);
            if (basicType)
            {
                return (basicType->baseType == BaseType::Int || basicType->baseType == BaseType::UInt);
            }
        }
        // Can be an int/uint vector from size 1 to 3
        {
            auto vectorType = as<VectorExpressionType>(type);
            if (!vectorType)
            {
                return false;
            }
            auto elemCount = as<ConstantIntVal>(vectorType->elementCount);
            if (elemCount->value < 1 || elemCount->value > 3)
            {
                return false;
            }
            // Must be a basic type
            auto basicType = as<BasicExpressionType>(vectorType->elementType);
            if (!basicType)
            {
                return false;
            }

            // Must be integral
            return (basicType->baseType == BaseType::Int || basicType->baseType == BaseType::UInt);
        }
    }

        /// Recursively walk `paramDeclRef` and add any existential/interface specialization parameters to `ioSpecializationParams`.
    static void _collectExistentialSpecializationParamsRec(
        SpecializationParams&   ioSpecializationParams,
        DeclRef<VarDeclBase>    paramDeclRef);

        /// Recursively walk `type` and add any existential/interface specialization parameters to `ioSpecializationParams`.
    static void _collectExistentialSpecializationParamsRec(
        SpecializationParams&   ioSpecializationParams,
        Type*                   type)
    {
        // Whether or not something is an array does not affect
        // the number of existential slots it introduces.
        //
        while( auto arrayType = as<ArrayExpressionType>(type) )
        {
            type = arrayType->baseType;
        }

        if( auto parameterGroupType = as<ParameterGroupType>(type) )
        {
            _collectExistentialSpecializationParamsRec(
                ioSpecializationParams,
                parameterGroupType->getElementType());
            return;
        }

        if( auto declRefType = as<DeclRefType>(type) )
        {
            auto typeDeclRef = declRefType->declRef;
            if( auto interfaceDeclRef = typeDeclRef.as<InterfaceDecl>() )
            {
                // Each leaf parameter of interface type adds a specialization
                // parameter, which determines the concrete type(s) that may
                // be provided as arguments for that parameter.
                //
                SpecializationParam specializationParam;
                specializationParam.flavor = SpecializationParam::Flavor::ExistentialType;
                specializationParam.object = type;
                ioSpecializationParams.add(specializationParam);
            }
            else if( auto structDeclRef = typeDeclRef.as<StructDecl>() )
            {
                // A structure type should recursively introduce
                // existential slots for its fields.
                //
                for( auto fieldDeclRef : GetFields(structDeclRef) )
                {
                    if(fieldDeclRef.getDecl()->HasModifier<HLSLStaticModifier>())
                        continue;

                    _collectExistentialSpecializationParamsRec(
                        ioSpecializationParams,
                        fieldDeclRef);
                }
            }
        }

        // TODO: We eventually need to handle cases like constant
        // buffers and parameter blocks that may have existential
        // element types.
    }

    static void _collectExistentialSpecializationParamsRec(
        SpecializationParams&   ioSpecializationParams,
        DeclRef<VarDeclBase>    paramDeclRef)
    {
        _collectExistentialSpecializationParamsRec(
            ioSpecializationParams,
            GetType(paramDeclRef));
    }


        /// Collect any interface/existential specialization parameters for `paramDeclRef` into `ioParamInfo` and `ioSpecializationParams`
    static void _collectExistentialSpecializationParamsForShaderParam(
        ShaderParamInfo&        ioParamInfo,
        SpecializationParams&   ioSpecializationParams,
        DeclRef<VarDeclBase>    paramDeclRef)
    {
        Index beginParamIndex = ioSpecializationParams.getCount();
        _collectExistentialSpecializationParamsRec(ioSpecializationParams, paramDeclRef);
        Index endParamIndex = ioSpecializationParams.getCount();
        
        ioParamInfo.firstSpecializationParamIndex = beginParamIndex;
        ioParamInfo.specializationParamCount = endParamIndex - beginParamIndex;
    }

    void EntryPoint::_collectGenericSpecializationParamsRec(Decl* decl)
    {
        if(!decl)
            return;

        _collectGenericSpecializationParamsRec(decl->ParentDecl);

        auto genericDecl = as<GenericDecl>(decl);
        if(!genericDecl)
            return;

        for(auto m : genericDecl->Members)
        {
            if(auto genericTypeParam = as<GenericTypeParamDecl>(m))
            {
                SpecializationParam param;
                param.flavor = SpecializationParam::Flavor::GenericType;
                param.object = genericTypeParam;
                m_genericSpecializationParams.add(param);
            }
            else if(auto genericValParam = as<GenericValueParamDecl>(m))
            {
                SpecializationParam param;
                param.flavor = SpecializationParam::Flavor::GenericValue;
                param.object = genericValParam;
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
        if( auto funcDeclRef = getFuncDeclRef() )
        {
            for( auto paramDeclRef : GetParameters(funcDeclRef) )
            {
                ShaderParamInfo shaderParamInfo;
                shaderParamInfo.paramDeclRef = paramDeclRef;

                _collectExistentialSpecializationParamsForShaderParam(
                    shaderParamInfo,
                    m_existentialSpecializationParams,
                    paramDeclRef);

                m_shaderParams.add(shaderParamInfo);
            }
        }
    }

    bool isPrimaryDecl(
        CallableDecl*   decl)
    {
        SLANG_ASSERT(decl);
        return (!decl->primaryDecl) || (decl == decl->primaryDecl);
    }

    FuncDecl* findFunctionDeclByName(
        Module*                 translationUnit,
        Name*                   name,
        DiagnosticSink*         sink)
    {
        auto translationUnitSyntax = translationUnit->getModuleDecl();

        // Make sure we've got a query-able member dictionary
        buildMemberDictionary(translationUnitSyntax);

        // We will look up any global-scope declarations in the translation
        // unit that match the name of our entry point.
        Decl* firstDeclWithName = nullptr;
        if (!translationUnitSyntax->memberDictionary.TryGetValue(name, firstDeclWithName))
        {
            // If there doesn't appear to be any such declaration, then we are done.

            sink->diagnose(translationUnitSyntax, Diagnostics::entryPointFunctionNotFound, name);

            return nullptr;
        }

        // We found at least one global-scope declaration with the right name,
        // but (1) it might not be a function, and (2) there might be
        // more than one function.
        //
        // We'll walk the linked list of declarations with the same name,
        // to see what we find. Along the way we'll keep track of the
        // first function declaration we find, if any:
        FuncDecl* entryPointFuncDecl = nullptr;
        for (auto ee = firstDeclWithName; ee; ee = ee->nextInContainerWithSameName)
        {
            // Is this declaration a function?
            if (auto funcDecl = as<FuncDecl>(ee))
            {
                // Skip non-primary declarations, so that
                // we don't give an error when an entry
                // point is forward-declared.
                if (!isPrimaryDecl(funcDecl))
                    continue;

                // is this the first one we've seen?
                if (!entryPointFuncDecl)
                {
                    // If so, this is a candidate to be
                    // the entry point function.
                    entryPointFuncDecl = funcDecl;
                }
                else
                {
                    // Uh-oh! We've already seen a function declaration with this
                    // name before, so the whole thing is ambiguous. We need
                    // to diagnose and bail out.

                    sink->diagnose(translationUnitSyntax, Diagnostics::ambiguousEntryPoint, name);

                    // List all of the declarations that the user *might* mean
                    for (auto ff = firstDeclWithName; ff; ff = ff->nextInContainerWithSameName)
                    {
                        if (auto candidate = as<FuncDecl>(ff))
                        {
                            sink->diagnose(candidate, Diagnostics::entryPointCandidate, candidate->getName());
                        }
                    }

                    // Bail out.
                    return nullptr;
                }
            }
        }

        return entryPointFuncDecl;
    }

    // Validate that an entry point function conforms to any additional
    // constraints based on the stage (and profile?) it specifies.
    void validateEntryPoint(
        EntryPoint*     entryPoint,
        DiagnosticSink* sink)
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
        //   declarations in the standard library. We should also be
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
        auto linkage = module->getLinkage();


        // Every entry point needs to have a stage specified either via
        // command-line/API options, or via an explicit `[shader("...")]` attribute.
        //
        if( stage == Stage::Unknown )
        {
            sink->diagnose(entryPointFuncDecl, Diagnostics::entryPointHasNoStage, entryPointName);
        }

        if( stage == Stage::Hull )
        {
            // TODO: We could consider *always* checking any `[patchconsantfunc("...")]`
            // attributes, so that they need to resolve to a function.

            auto attr = entryPointFuncDecl->FindModifier<PatchConstantFuncAttribute>();

            if (attr)
            {
                if (attr->args.getCount() != 1)
                {
                    sink->diagnose(attr, Diagnostics::badlyDefinedPatchConstantFunc, entryPointName);
                    return;
                }

                Expr* expr = attr->args[0];
                StringLiteralExpr* stringLit = as<StringLiteralExpr>(expr);

                if (!stringLit)
                {
                    sink->diagnose(expr, Diagnostics::badlyDefinedPatchConstantFunc, entryPointName);
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
                FuncDecl* patchConstantFuncDecl = findFunctionDeclByName(
                    module,
                    name,
                    sink);
                if (!patchConstantFuncDecl)
                {
                    sink->diagnose(expr, Diagnostics::attributeFunctionNotFound, name, "patchconstantfunc");
                    return;
                }

                attr->patchConstantFuncDecl = patchConstantFuncDecl;
            }
        }
        else if(stage == Stage::Compute)
        {
            for(const auto& param : entryPointFuncDecl->GetParameters())
            {
                if(auto semantic = param->FindModifier<HLSLSimpleSemantic>())
                {
                    const auto& semanticToken = semantic->name;

                    String lowerName = String(semanticToken.Content).toLower();

                    if(lowerName == "sv_dispatchthreadid")
                    {
                        Type* paramType = param->getType();

                        if(!isValidThreadDispatchIDType(paramType))
                        {
                            String typeString = paramType->ToString();
                            sink->diagnose(param->loc, Diagnostics::invalidDispatchThreadIDType, typeString);
                            return;
                        }
                    }
                }
            }
        }
    }

    // Given an entry point specified via API or command line options,
    // attempt to find a matching AST declaration that implements the specified
    // entry point. If such a function is found, then validate that it actually
    // meets the requirements for the selected stage/profile.
    //
    // Returns an `EntryPoint` object representing the (unspecialized)
    // entry point if it is found and validated, and null otherwise.
    //
    RefPtr<EntryPoint> findAndValidateEntryPoint(
        FrontEndEntryPointRequest*  entryPointReq)
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
        auto translationUnitSyntax = translationUnit->getModuleDecl();

        auto entryPointName = entryPointReq->getName();

        // Make sure we've got a query-able member dictionary
        buildMemberDictionary(translationUnitSyntax);

        // We will look up any global-scope declarations in the translation
        // unit that match the name of our entry point.
        Decl* firstDeclWithName = nullptr;
        if( !translationUnitSyntax->memberDictionary.TryGetValue(entryPointName, firstDeclWithName) )
        {
            // If there doesn't appear to be any such declaration, then
            // we need to diagnose it as an error, and then bail out.
            sink->diagnose(translationUnitSyntax, Diagnostics::entryPointFunctionNotFound, entryPointName);
            return nullptr;
        }

        // We found at least one global-scope declaration with the right name,
        // but (1) it might not be a function, and (2) there might be
        // more than one function.
        //
        // We'll walk the linked list of declarations with the same name,
        // to see what we find. Along the way we'll keep track of the
        // first function declaration we find, if any:
        //
        FuncDecl* entryPointFuncDecl = nullptr;
        for(auto ee = firstDeclWithName; ee; ee = ee->nextInContainerWithSameName)
        {
            // We want to support the case where the declaration is
            // a generic function, so we will automatically
            // unwrap any outer `GenericDecl` we find here.
            //
            auto decl = ee;
            if(auto genericDecl = as<GenericDecl>(decl))
                decl = genericDecl->inner;

            // Is this declaration a function?
            if (auto funcDecl = as<FuncDecl>(decl))
            {
                // Skip non-primary declarations, so that
                // we don't give an error when an entry
                // point is forward-declared.
                if (!isPrimaryDecl(funcDecl))
                    continue;

                // is this the first one we've seen?
                if (!entryPointFuncDecl)
                {
                    // If so, this is a candidate to be
                    // the entry point function.
                    entryPointFuncDecl = funcDecl;
                }
                else
                {
                    // Uh-oh! We've already seen a function declaration with this
                    // name before, so the whole thing is ambiguous. We need
                    // to diagnose and bail out.

                    sink->diagnose(translationUnitSyntax, Diagnostics::ambiguousEntryPoint, entryPointName);

                    // List all of the declarations that the user *might* mean
                    for (auto ff = firstDeclWithName; ff; ff = ff->nextInContainerWithSameName)
                    {
                        if (auto candidate = as<FuncDecl>(ff))
                        {
                            sink->diagnose(candidate, Diagnostics::entryPointCandidate, candidate->getName());
                        }
                    }

                    // Bail out.
                    return nullptr;
                }
            }
        }

        // Did we find a function declaration in our search?
        if(!entryPointFuncDecl)
        {
            // If not, then we need to diagnose the error.
            // For convenience, we will point to the first
            // declaration with the right name, that wasn't a function.
            sink->diagnose(firstDeclWithName, Diagnostics::entryPointSymbolNotAFunction, entryPointName);
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
        // it didn't have one, *or* issue a diagnostic if there is a mismatch.
        //
        auto entryPointProfile = entryPointReq->getProfile();
        if( auto entryPointAttribute = entryPointFuncDecl->FindModifier<EntryPointAttribute>() )
        {
            auto entryPointStage = entryPointProfile.GetStage();
            if( entryPointStage == Stage::Unknown )
            {
                entryPointProfile.setStage(entryPointAttribute->stage);
            }
            else if( entryPointAttribute->stage != entryPointStage )
            {
                sink->diagnose(entryPointFuncDecl, Diagnostics::specifiedStageDoesntMatchAttribute, entryPointName, entryPointStage, entryPointAttribute->stage);
            }
        }
        else
        {
            // TODO: Should we attach a `[shader(...)]` attribute to an
            // entry point that didn't have one, so that we can have
            // a more uniform representation in the AST?
        }

        RefPtr<EntryPoint> entryPoint = EntryPoint::create(
            linkage,
            makeDeclRef(entryPointFuncDecl),
            entryPointProfile);

        // Now that we've *found* the entry point, it is time to validate
        // that it actually meets the constraints for the chosen stage/profile.
        //
        validateEntryPoint(entryPoint, sink);

        return entryPoint;
    }

    /// Get the name a variable will use for reflection purposes
Name* getReflectionName(VarDeclBase* varDecl)
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
    DiagnosticSink*             sink,
    StructuralTypeMatchStack*   inStack)
{
    SLANG_ASSERT(inStack);

    // The bottom-most entry in the stack should represent
    // the shader parameters that kicked things off
    auto stack = inStack;
    while(stack->parent)
        stack = stack->parent;

    sink->diagnose(stack->leftDecl, Diagnostics::shaderParameterDeclarationsDontMatch, getReflectionName(stack->leftDecl));
    sink->diagnose(stack->rightDecl, Diagnostics::seeOtherDeclarationOf, getReflectionName(stack->rightDecl));
}

// Two types that were expected to match did not.
// Inform the user with a suitable message.
static void diagnoseTypeMismatch(
    DiagnosticSink*             sink,
    StructuralTypeMatchStack*   inStack)
{
    auto stack = inStack;
    SLANG_ASSERT(stack);
    diagnoseParameterTypeMismatch(sink, stack);

    auto leftType = GetType(stack->leftDecl);
    auto rightType = GetType(stack->rightDecl);

    if( stack->parent )
    {
        sink->diagnose(stack->leftDecl, Diagnostics::fieldTypeMisMatch, getReflectionName(stack->leftDecl), leftType, rightType);
        sink->diagnose(stack->rightDecl, Diagnostics::seeOtherDeclarationOf, getReflectionName(stack->rightDecl));

        stack = stack->parent;
        if( stack )
        {
            while( stack->parent )
            {
                sink->diagnose(stack->leftDecl, Diagnostics::usedInDeclarationOf, getReflectionName(stack->leftDecl));
                stack = stack->parent;
            }
        }
    }
    else
    {
        sink->diagnose(stack->leftDecl, Diagnostics::shaderParameterTypeMismatch, leftType, rightType);
    }
}

// Two types that were expected to match did not.
// Inform the user with a suitable message.
static void diagnoseTypeFieldsMismatch(
    DiagnosticSink*             sink,
    DeclRef<Decl> const&        left,
    DeclRef<Decl> const&        right,
    StructuralTypeMatchStack*   stack)
{
    diagnoseParameterTypeMismatch(sink, stack);

    sink->diagnose(left, Diagnostics::fieldDeclarationsDontMatch, left.GetName());
    sink->diagnose(right, Diagnostics::seeOtherDeclarationOf, right.GetName());

    if( stack )
    {
        while( stack->parent )
        {
            sink->diagnose(stack->leftDecl, Diagnostics::usedInDeclarationOf, getReflectionName(stack->leftDecl));
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

        outFields.add(fieldDeclRef);
    }
}

static bool validateTypesMatch(
    DiagnosticSink*             sink,
    Type*                       left,
    Type*                       right,
    StructuralTypeMatchStack*   stack);

static bool validateIntValuesMatch(
    DiagnosticSink*             sink,
    IntVal*                     left,
    IntVal*                     right,
    StructuralTypeMatchStack*   stack)
{
    if(left->EqualsVal(right))
        return true;

    // TODO: are there other cases we need to handle here?

    diagnoseTypeMismatch(sink, stack);
    return false;
}


static bool validateValuesMatch(
    DiagnosticSink*             sink,
    Val*                        left,
    Val*                        right,
    StructuralTypeMatchStack*   stack)
{
    if( auto leftType = dynamicCast<Type>(left) )
    {
        if( auto rightType = dynamicCast<Type>(right) )
        {
            return validateTypesMatch(sink, leftType, rightType, stack);
        }
    }

    if( auto leftInt = dynamicCast<IntVal>(left) )
    {
        if( auto rightInt = dynamicCast<IntVal>(right) )
        {
            return validateIntValuesMatch(sink, leftInt, rightInt, stack);
        }
    }

    if( auto leftWitness = dynamicCast<SubtypeWitness>(left) )
    {
        if( auto rightWitness = dynamicCast<SubtypeWitness>(right) )
        {
            return true;
        }
    }

    diagnoseTypeMismatch(sink, stack);
    return false;
}

static bool validateGenericSubstitutionsMatch(
    DiagnosticSink*             sink,
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

        diagnoseTypeMismatch(sink, stack);
        return false;
    }



    Index argCount = left->args.getCount();
    if( argCount != right->args.getCount() )
    {
        diagnoseTypeMismatch(sink, stack);
        return false;
    }

    for( Index aa = 0; aa < argCount; ++aa )
    {
        auto leftArg = left->args[aa];
        auto rightArg = right->args[aa];

        if(!validateValuesMatch(sink, leftArg, rightArg, stack))
            return false;
    }

    return true;
}

static bool validateThisTypeSubstitutionsMatch(
    DiagnosticSink*             /*sink*/,
    ThisTypeSubstitution*       /*left*/,
    ThisTypeSubstitution*       /*right*/,
    StructuralTypeMatchStack*   /*stack*/)
{
    // TODO: actual checking.
    return true;
}

static bool validateSpecializationsMatch(
    DiagnosticSink*             sink,
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
                if(validateGenericSubstitutionsMatch(sink, leftGeneric, rightGeneric, stack))
                {
                    continue;
                }
            }
        }
        else if(auto leftThisType = as<ThisTypeSubstitution>(leftSubst))
        {
            if(auto rightThisType = as<ThisTypeSubstitution>(rightSubst))
            {
                if(validateThisTypeSubstitutionsMatch(sink, leftThisType, rightThisType, stack))
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
    DiagnosticSink*             sink,
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
                    sink,
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

                        Index leftFieldCount = leftFields.getCount();
                        Index rightFieldCount = rightFields.getCount();

                        if( leftFieldCount != rightFieldCount )
                        {
                            diagnoseTypeFieldsMismatch(sink, leftDeclRef, rightDeclRef, stack);
                            return false;
                        }

                        for( Index ii = 0; ii < leftFieldCount; ++ii )
                        {
                            auto leftField = leftFields[ii];
                            auto rightField = rightFields[ii];

                            if( leftField.GetName() != rightField.GetName() )
                            {
                                diagnoseTypeFieldsMismatch(sink, leftDeclRef, rightDeclRef, stack);
                                return false;
                            }

                            auto leftFieldType = GetType(leftField);
                            auto rightFieldType = GetType(rightField);

                            StructuralTypeMatchStack subStack;
                            subStack.parent = stack;
                            subStack.leftDecl = leftField;
                            subStack.rightDecl = rightField;

                            if(!validateTypesMatch(sink, leftFieldType,rightFieldType, &subStack))
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
            if(!validateTypesMatch(sink, leftArrayType->baseType, rightArrayType->baseType, stack) )
                return false;

            if(!validateValuesMatch(sink, leftArrayType->ArrayLength, rightArrayType->ArrayLength, stack))
                return false;

            return true;
        }
    }

    diagnoseTypeMismatch(sink, stack);
    return false;
}

// This function is supposed to determine if two global shader
// parameter declarations represent the same logical parameter
// (so that they should get the exact same binding(s) allocated).
//
static bool doesParameterMatch(
    DiagnosticSink*         sink,
    DeclRef<VarDeclBase>    varDeclRef,
    DeclRef<VarDeclBase>    existingVarDeclRef)
{
    StructuralTypeMatchStack stack;
    stack.parent = nullptr;
    stack.leftDecl = varDeclRef;
    stack.rightDecl = existingVarDeclRef;

    validateTypesMatch(sink, GetType(varDeclRef), GetType(existingVarDeclRef), &stack);

    return true;
}

    void Module::_collectShaderParams()
    {
        auto moduleDecl = m_moduleDecl;

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
        HashSet<Module*> requiredModuleSet;

        for( auto globalDecl : moduleDecl->Members )
        {
            if(auto globalVar = globalDecl.as<VarDecl>())
            {
                // We do not want to consider global variable declarations
                // that don't represents shader parameters. This includes
                // things like `static` globals and `groupshared` variables.
                //
                if(!isGlobalShaderParameter(globalVar))
                    continue;

                // At this point we know we have a global shader parameter.

                GlobalShaderParamInfo shaderParamInfo;
                shaderParamInfo.paramDeclRef = makeDeclRef(globalVar.Ptr());

                // We need to consider what specialization parameters
                // are introduced by this shader parameter. This step
                // fills in fields on `shaderParamInfo` so that we
                // can assocaite specialization arguments supplied later
                // with the correct parameter.
                //
                _collectExistentialSpecializationParamsForShaderParam(
                    shaderParamInfo,
                    m_specializationParams,
                    makeDeclRef(globalVar.Ptr()));

                m_shaderParams.add(shaderParamInfo);
            }
            else if( auto globalGenericParam = as<GlobalGenericParamDecl>(globalDecl) )
            {
                // A global generic type parameter declaration introduces
                // a suitable specialization parameter.
                //
                SpecializationParam specializationParam;
                specializationParam.flavor = SpecializationParam::Flavor::GenericType;
                specializationParam.object = globalGenericParam;
                m_specializationParams.add(specializationParam);
            }
            else if( auto importDecl = as<ImportDecl>(globalDecl) )
            {
                // An `import` declaration creates a requirement dependency
                // from this module to another module.
                //
                auto importedModule = getModule(importDecl->importedModuleDecl);
                if(!requiredModuleSet.Contains(importedModule))
                {
                    requiredModuleSet.Add(importedModule);
                    m_requirements.add(importedModule);
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


        /// Enumerate the parameters of a `LegacyProgram`.
    void LegacyProgram::_collectShaderParams(DiagnosticSink* sink)
    {
        // We need to collect all of the global shader parameters
        // referenced by the compile request, and for each we
        // need to do a few things:
        //
        // * We need to determine if the parameter is a duplicate/redeclaration
        //   of the "same" parameter in another translation unit, and collapse
        //   those into one logical shader parameter if so.
        //
        // * We need to determine what existential type slots are introduced
        //   by the parameter, and associate that information with the parameter.
        //
        // To deal with the first issue, we will maintain a map from a parameter
        // name to the index of an existing parameter with that name.
        //
        // TODO: Eventually we should deprecate support for the
        // deduplication feature of `LegaqcyProgram`, at which point
        // this entire type and all its complications can be eliminated
        // from the code (that includes a lot of support in the "parameter
        // binding" step for shader parameters with multiple declarations).
        // Until that point this type will have a fair amount of duplication
        // with stuff in `Module` and `CompositeComponentType`.

        // We use a dictionary to keep track of any shader parameter
        // we've alrady collected with a given name.
        //
        Dictionary<Name*, Int> mapNameToParamIndex;

        for( auto translationUnit : m_translationUnits )
        {
            auto module = translationUnit->getModule();
            auto moduleDecl = module->getModuleDecl();
            for( auto globalVar : moduleDecl->getMembersOfType<VarDecl>() )
            {
                // We do not want to consider global variable declarations
                // that don't represents shader parameters. This includes
                // things like `static` globals and `groupshared` variables.
                //
                if(!isGlobalShaderParameter(globalVar))
                    continue;

                // This declaration may represent the same logical parameter
                // as a declaration that came from a different translation unit.
                // If that is the case, we want to re-use the same `ShaderParamInfo`
                // across both parameters.
                //
                // TODO: This logic currently detects *any* global-scope parameters
                // with matching names, but it should eventually be narrowly
                // scoped so that it only applies to parameters from unnamed modules
                // (that is, modules that represent directly-compiled shader files
                // and not `import`ed code).
                //
                // First we look for an existing entry matching the name
                // of this parameter:
                //
                auto paramName = getReflectionName(globalVar);
                Int existingParamIndex = -1;
                if( mapNameToParamIndex.TryGetValue(paramName, existingParamIndex) )
                {
                    // If the parameters have the same name, but don't "match" according to some reasonable rules,
                    // then we will treat them as distinct global parameters.
                    //
                    // Note: all of the mismatch cases currently report errors, so that
                    // compilation will fail on a mismatch.
                    //
                    auto& existingParam = m_shaderParams[existingParamIndex];
                    if( doesParameterMatch(sink, makeDeclRef(globalVar.Ptr()), existingParam.paramDeclRef) )
                    {
                        // If we hit this case, then we had a match, and we should
                        // consider the new variable to be a redclaration of
                        // the existing one.

                        existingParam.additionalParamDeclRefs.add(
                            makeDeclRef(globalVar.Ptr()));
                        continue;
                    }
                }

                Int newParamIndex = Int(m_shaderParams.getCount());
                mapNameToParamIndex.Add(paramName, newParamIndex);

                GlobalShaderParamInfo shaderParamInfo;
                shaderParamInfo.paramDeclRef = makeDeclRef(globalVar.Ptr());

                _collectExistentialSpecializationParamsForShaderParam(
                    shaderParamInfo,
                    m_specializationParams,
                    makeDeclRef(globalVar.Ptr()));

                m_shaderParams.add(shaderParamInfo);
            }
        }
    }

        /// Create a new component type based on `inComponentType`, but with all its requiremetns filled.
    RefPtr<ComponentType> fillRequirements(
        ComponentType* inComponentType)
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
        for(;;)
        {
            auto requirementCount = componentType->getRequirementCount();
            if(requirementCount == 0)
                break;

            List<RefPtr<ComponentType>> allComponents;
            allComponents.add(componentType);

            for(Index rr = 0; rr < requirementCount; ++rr)
            {
                auto requirement = componentType->getRequirement(rr);
                allComponents.add(requirement);
            }

            componentType = CompositeComponentType::create(
                linkage,
                allComponents);
        }
        return componentType;
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
    RefPtr<ComponentType> createUnspecializedGlobalComponentType(
        FrontEndCompileRequest* compileRequest)
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
        auto sink = compileRequest->getSink();

        RefPtr<ComponentType> globalComponentType;
        if(compileRequest->translationUnits.getCount() == 1)
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
            globalComponentType = new LegacyProgram(
                linkage,
                compileRequest->translationUnits,
                sink);
        }

        return fillRequirements(globalComponentType);
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
        FrontEndCompileRequest*         compileRequest,
        List<RefPtr<ComponentType>>&    outUnspecializedEntryPoints)
    {
        auto linkage = compileRequest->getLinkage();
        auto sink = compileRequest->getSink();

        auto globalComponentType = compileRequest->getGlobalComponentType();

        // The validation of entry points here will be modal, and controlled
        // by whether the user specified any entry points directly via
        // API or command-line options.
        //
        // TODO: We may want to make this choice explicit rather than implicit.
        //
        // First, check if the user requested any entry points explicitly via
        // the API or command line.
        //
        bool anyExplicitEntryPoints = compileRequest->getEntryPointReqCount() != 0;

        List<RefPtr<ComponentType>> allComponentTypes;
        allComponentTypes.add(globalComponentType);

        if( anyExplicitEntryPoints )
        {
            // If there were any explicit requests for entry points to be
            // checked, then we will *only* check those.
            //
            for(auto entryPointReq : compileRequest->getEntryPointReqs())
            {
                auto entryPoint = findAndValidateEntryPoint(
                    entryPointReq);
                if( entryPoint )
                {
                    // TODO: We need to implement an explicit policy
                    // for what should happen if the user specified
                    // entry points via the command-line (or API),
                    // but didn't specify any groups (since the current
                    // compilation API doesn't allow for grouping).
                    //
                    entryPointReq->getTranslationUnit()->entryPoints.add(entryPoint);

                    outUnspecializedEntryPoints.add(entryPoint);
                    allComponentTypes.add(entryPoint);
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
            Index translationUnitCount = compileRequest->translationUnits.getCount();
            for(Index tt = 0; tt < translationUnitCount; ++tt)
            {
                auto translationUnit = compileRequest->translationUnits[tt];
                for( auto globalDecl : translationUnit->getModuleDecl()->Members )
                {
                    auto maybeFuncDecl = globalDecl;
                    if( auto genericDecl = as<GenericDecl>(maybeFuncDecl) )
                    {
                        maybeFuncDecl = genericDecl->inner;
                    }

                    auto funcDecl = as<FuncDecl>(maybeFuncDecl);
                    if(!funcDecl)
                        continue;

                    auto entryPointAttr = funcDecl->FindModifier<EntryPointAttribute>();
                    if(!entryPointAttr)
                        continue;

                    // We've discovered a valid entry point. It is a function (possibly
                    // generic) that has a `[shader(...)]` attribute to mark it as an
                    // entry point.
                    //
                    // We will now register that entry point as an `EntryPoint`
                    // with an appropriately chosen profile.
                    //
                    // The profile will only include a stage, so that the profile "family"
                    // and "version" are left unspecified. Downstream code will need
                    // to be able to handle this case.
                    //
                    Profile profile;
                    profile.setStage(entryPointAttr->stage);

                    RefPtr<EntryPoint> entryPoint = EntryPoint::create(
                        linkage,
                        makeDeclRef(funcDecl),
                        profile);

                    validateEntryPoint(entryPoint, sink);

                    // Note: in the case that the user didn't explicitly
                    // specify entry points and we are instead compiling
                    // a shader "library," then we do not want to automatically
                    // combine the entry points into groups in the generated
                    // `Program`, since that would be slightly too magical.
                    //
                    // Instead, each entry point will end up in a singleton
                    // group, so that its entry-point parameters lay out
                    // independent of the others.
                    //
                    translationUnit->entryPoints.add(entryPoint);

                    outUnspecializedEntryPoints.add(entryPoint);
                    allComponentTypes.add(entryPoint);
                }
            }
        }

        if(allComponentTypes.getCount() > 1)
        {
            auto composite = CompositeComponentType::create(
                linkage,
                allComponentTypes);
            return composite;
        }
        else
        {
            return globalComponentType;
        }
    }

    RefPtr<ComponentType::SpecializationInfo> Module::_validateSpecializationArgsImpl(
        SpecializationArg const*    args,
        Index                       argCount,
        DiagnosticSink*             sink)
    {
        SLANG_ASSERT(argCount == getSpecializationParamCount());

        SharedSemanticsContext semanticsContext(getLinkage(), sink);
        SemanticsVisitor visitor(&semanticsContext);

        RefPtr<Module::ModuleSpecializationInfo> specializationInfo = new Module::ModuleSpecializationInfo();

        for( Index ii = 0; ii < argCount; ++ii )
        {
            auto& arg = args[ii];
            auto& param = m_specializationParams[ii];

            auto argType = arg.val.as<Type>();
            SLANG_ASSERT(argType);

            switch( param.flavor )
            {
            case SpecializationParam::Flavor::GenericType:
                {
                    auto genericTypeParamDecl = param.object.as<GlobalGenericParamDecl>();
                    SLANG_ASSERT(genericTypeParamDecl);

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
                    if( auto argDeclRefType = argType.as<DeclRefType>() )
                    {
                        auto argDeclRef = argDeclRefType->declRef;
                        if(auto argGenericParamDeclRef = argDeclRef.as<GlobalGenericParamDecl>())
                        {
                            if(argGenericParamDeclRef.getDecl() == genericTypeParamDecl)
                            {
                                // We are trying to specialize a generic parameter using itself.
                                sink->diagnose(genericTypeParamDecl,
                                    Diagnostics::cannotSpecializeGlobalGenericToItself,
                                    genericTypeParamDecl->getName());
                                continue;
                            }
                            else
                            {
                                // We are trying to specialize a generic parameter using a *different*
                                // global generic type parameter.
                                sink->diagnose(genericTypeParamDecl,
                                    Diagnostics::cannotSpecializeGlobalGenericToAnotherGenericParam,
                                    genericTypeParamDecl->getName(),
                                    argGenericParamDeclRef.GetName());
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
                    for(auto constraintDecl : genericTypeParamDecl->getMembersOfType<GenericTypeConstraintDecl>())
                    {
                        // Get the type that the constraint is enforcing conformance to
                        auto interfaceType = GetSup(DeclRef<GenericTypeConstraintDecl>(constraintDecl, nullptr));

                        // Use our semantic-checking logic to search for a witness to the required conformance
                        auto witness = visitor.tryGetSubtypeWitness(argType, interfaceType);
                        if (!witness)
                        {
                            // If no witness was found, then we will be unable to satisfy
                            // the conformances required.
                            sink->diagnose(genericTypeParamDecl,
                                Diagnostics::typeArgumentForGenericParameterDoesNotConformToInterface,
                                argType,
                                genericTypeParamDecl->nameAndLoc.name,
                                interfaceType);
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
                    auto interfaceType = param.object.as<Type>();
                    SLANG_ASSERT(interfaceType);

                    auto witness = visitor.tryGetSubtypeWitness(argType, interfaceType);
                    if (!witness)
                    {
                            // If no witness was found, then we will be unable to satisfy
                            // the conformances required.
                            sink->diagnose(SourceLoc(),
                                Diagnostics::typeArgumentDoesNotConformToInterface,
                                argType,
                                interfaceType);
                    }

                    ExpandedSpecializationArg expandedArg;
                    expandedArg.val = argType;
                    expandedArg.witness = witness;

                    specializationInfo->existentialArgs.add(expandedArg);
                }
                break;

            default:
                SLANG_UNEXPECTED("unhandled specialization parameter flavor");
            }
        }

        return specializationInfo;
    }


    static void _extractSpecializationArgs(
        ComponentType*              componentType,
        List<RefPtr<Expr>> const&   argExprs,
        List<SpecializationArg>&    outArgs,
        DiagnosticSink*             sink)
    {
        auto linkage = componentType->getLinkage();

        auto argCount = argExprs.getCount();
        for(Index ii = 0; ii < argCount; ++ii )
        {
            auto argExpr = argExprs[ii];
            auto paramInfo = componentType->getSpecializationParam(ii);

            // TODO: We should support non-type arguments here

            auto argType = checkProperType(linkage, TypeExp(argExpr), sink);
            if( !argType )
            {
                // If no witness was found, then we will be unable to satisfy
                // the conformances required.
                sink->diagnose(argExpr,
                    Diagnostics::expectedAType,
                    argExpr->type);
                continue;
            }

            SpecializationArg arg;
            arg.val = argType;
            outArgs.add(arg);
        }
    }

    RefPtr<ComponentType::SpecializationInfo> EntryPoint::_validateSpecializationArgsImpl(
        SpecializationArg const*    inArgs,
        Index                       inArgCount,
        DiagnosticSink*             sink)
    {
        auto args = inArgs;
        auto argCount = inArgCount;

        SharedSemanticsContext sharedSemanticsContext(getLinkage(), sink);
        SemanticsVisitor visitor(&sharedSemanticsContext);

        // The first N arguments will be for the explicit generic parameters
        // of the entry point (if it has any).
        //
        auto genericSpecializationParamCount = getGenericSpecializationParamCount();
        SLANG_ASSERT(argCount >= genericSpecializationParamCount);

        Result result = SLANG_OK;

        RefPtr<EntryPointSpecializationInfo> info = new EntryPointSpecializationInfo();

        DeclRef<FuncDecl> specializedFuncDeclRef = m_funcDeclRef;
        if(genericSpecializationParamCount)
        {
            // We need to construct a generic application and use
            // the semantic checking machinery to expand out
            // the rest of the arguments via inference...

            auto genericDeclRef = m_funcDeclRef.GetParent().as<GenericDecl>();
            SLANG_ASSERT(genericDeclRef); // otherwise we wouldn't have generic parameters

            RefPtr<GenericSubstitution> genericSubst = new GenericSubstitution();
            genericSubst->outer = genericDeclRef.substitutions.substitutions;
            genericSubst->genericDecl = genericDeclRef.getDecl();

            for(Index ii = 0; ii < genericSpecializationParamCount; ++ii)
            {
                auto specializationArg = args[ii];
                genericSubst->args.add(specializationArg.val);
            }

            for( auto constraintDecl : genericDeclRef.getDecl()->getMembersOfType<GenericTypeConstraintDecl>() )
            {
                auto constraintSubst = genericDeclRef.substitutions;
                constraintSubst.substitutions = genericSubst;

                DeclRef<GenericTypeConstraintDecl> constraintDeclRef(
                    constraintDecl, constraintSubst);

                auto sub = GetSub(constraintDeclRef);
                auto sup = GetSup(constraintDeclRef);

                auto subTypeWitness = visitor.tryGetSubtypeWitness(sub, sup);
                if(subTypeWitness)
                {
                    genericSubst->args.add(subTypeWitness);
                }
                else
                {
                    // TODO: diagnose a problem here
                    sink->diagnose(constraintDecl, Diagnostics::typeArgumentDoesNotConformToInterface, sub, sup);
                    result = SLANG_FAIL;
                    continue;
                }
            }

            specializedFuncDeclRef.substitutions.substitutions = genericSubst;
        }

        info->specializedFuncDeclRef = specializedFuncDeclRef;

        // Once the generic parameters (if any) have been dealt with,
        // any remaining specialization arguments are for existential/interface
        // specialization parameters, attached to the value parameters
        // of the entry point.
        //
        args += genericSpecializationParamCount;
        argCount -= genericSpecializationParamCount;

        auto existentialSpecializationParamCount = getExistentialSpecializationParamCount();
        SLANG_ASSERT(argCount == existentialSpecializationParamCount);

        for( Index ii = 0; ii < existentialSpecializationParamCount; ++ii )
        {
            auto& param = m_existentialSpecializationParams[ii];
            auto& specializationArg = args[ii];

            // TODO: We need to handle all the cases of "flavor" for the `param`s (not just types)

            auto paramType = param.object.as<Type>();
            auto argType = specializationArg.val.as<Type>();

            auto witness = visitor.tryGetSubtypeWitness(argType, paramType);
            if (!witness)
            {
                // If no witness was found, then we will be unable to satisfy
                // the conformances required.
                sink->diagnose(SourceLoc(), Diagnostics::typeArgumentDoesNotConformToInterface, argType, paramType);
                result = SLANG_FAIL;
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
        EntryPoint*                 unspecializedEntryPoint,
        List<RefPtr<Expr>> const&   argExprs,
        DiagnosticSink*             sink)
    {
        // We need to convert all of the `Expr` arguments
        // into `SpecializationArg`s, so that we can bottleneck
        // through the shared logic.
        //
        List<SpecializationArg> args;
        _extractSpecializationArgs(unspecializedEntryPoint, argExprs, args, sink);
        if(sink->GetErrorCount())
            return nullptr;

        return unspecializedEntryPoint->specialize(
            args.getBuffer(),
            args.getCount(),
            sink);
    }

        /// Parse an array of strings as specialization arguments.
        ///
        /// Names in the strings will be parsed in the context of
        /// the code loaded into the given compile request.
        ///
    void parseSpecializationArgStrings(
        EndToEndCompileRequest* endToEndReq,
        List<String> const&     genericArgStrings,
        List<RefPtr<Expr>>&     outGenericArgs)
    {
        auto unspecialiedProgram = endToEndReq->getUnspecializedGlobalComponentType();

        // TODO: Building a list of `scopesToTry` here shouldn't
        // be required, since the `Scope` type itself has the ability
        // for form chains for lookup purposes (e.g., the way that
        // `import` is handled by modifying a scope).
        //
        List<RefPtr<Scope>> scopesToTry;
        for( auto module : unspecialiedProgram->getModuleDependencies() )
            scopesToTry.add(module->getModuleDecl()->scope);

        // We are going to do some semantic checking, so we need to
        // set up a `SemanticsVistitor` that we can use.
        //
        auto linkage = endToEndReq->getLinkage();
        auto sink = endToEndReq->getSink();

        SharedSemanticsContext sharedSemanticsContext(
            linkage,
            sink);
        SemanticsVisitor semantics(&sharedSemanticsContext);

        // We will be looping over the generic argument strings
        // that the user provided via the API (or command line),
        // and parsing+checking each into an `Expr`.
        //
        // This loop will *not* handle coercing the arguments
        // to be types.
        //
        for(auto name : genericArgStrings)
        {
            RefPtr<Expr> argExpr;
            for (auto & s : scopesToTry)
            {
                argExpr = linkage->parseTypeString(name, s);
                argExpr = semantics.CheckTerm(argExpr);
                if( argExpr )
                {
                    break;
                }
            }

            if(!argExpr)
            {
                sink->diagnose(SourceLoc(), Diagnostics::internalCompilerError, "couldn't parse specialization argument");
                return;
            }

            outGenericArgs.add(argExpr);
        }
    }

    Type* Linkage::specializeType(
        Type*           unspecializedType,
        Int             argCount,
        Type* const*    args,
        DiagnosticSink* sink)
    {
        SLANG_ASSERT(unspecializedType);

        // TODO: We should cache and re-use specialized types
        // when the exact same arguments are provided again later.

        SharedSemanticsContext sharedSemanticsContext(this, sink);
        SemanticsVisitor visitor(&sharedSemanticsContext);

        SpecializationParams specializationParams;
        _collectExistentialSpecializationParamsRec(specializationParams, unspecializedType);

        assert(specializationParams.getCount() == argCount);

        ExpandedSpecializationArgs specializationArgs;
        for( Int aa = 0; aa < argCount; ++aa )
        {
            auto paramType = specializationParams[aa].object.as<Type>();
            auto argType = args[aa];

            ExpandedSpecializationArg arg;
            arg.val = argType;
            arg.witness = visitor.tryGetSubtypeWitness(argType, paramType);
            specializationArgs.add(arg);
        }

        RefPtr<ExistentialSpecializedType> specializedType = new ExistentialSpecializedType();
        specializedType->baseType = unspecializedType;
        specializedType->args = specializationArgs;

        m_specializedTypes.add(specializedType);

        return specializedType;
    }

        /// Shared implementation logic for the `_createSpecializedProgram*` entry points.
    static RefPtr<ComponentType> _createSpecializedProgramImpl(
        Linkage*                    linkage,
        ComponentType*              unspecializedProgram,
        List<RefPtr<Expr>> const&   specializationArgExprs,
        DiagnosticSink*             sink)
    {
        // If there are no specialization arguments,
        // then the the result of specialization should
        // be the same as the input.
        //
        auto specializationArgCount = specializationArgExprs.getCount();
        if( specializationArgCount == 0 )
        {
            return unspecializedProgram;
        }

        auto specializationParamCount = unspecializedProgram->getSpecializationParamCount();
        if(specializationArgCount != specializationParamCount )
        {
            sink->diagnose(SourceLoc(), Diagnostics::mismatchSpecializationArguments,
                specializationParamCount,
                specializationArgCount);
            return nullptr;
        }

        // We have an appropriate number of arguments for the global specialization parameters,
        // and now we need to check that the arguments conform to the declared constraints.
        //
        SharedSemanticsContext visitor(linkage, sink);

        List<SpecializationArg> specializationArgs;
        _extractSpecializationArgs(unspecializedProgram, specializationArgExprs, specializationArgs, sink);
        if(sink->GetErrorCount())
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
        EndToEndCompileRequest*                         endToEndReq,
        EntryPoint*                                     unspecializedEntryPoint,
        EndToEndCompileRequest::EntryPointInfo const&   entryPointInfo)
    {
        auto sink = endToEndReq->getSink();
        auto entryPointFuncDecl = unspecializedEntryPoint->getFuncDecl();

        // If the user specified generic arguments for the entry point,
        // then we will need to parse the arguments first.
        //
        List<RefPtr<Expr>> specializationArgExprs;
        parseSpecializationArgStrings(
            endToEndReq,
            entryPointInfo.specializationArgStrings,
            specializationArgExprs);

        // Next we specialize the entry point function given the parsed
        // generic argument expressions.
        //
        auto entryPoint = createSpecializedEntryPoint(
            unspecializedEntryPoint,
            specializationArgExprs,
            sink);

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
    RefPtr<ComponentType> createSpecializedGlobalComponentType(
        EndToEndCompileRequest* endToEndReq)
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
        List<RefPtr<Expr>> globalSpecializationArgs;
        parseSpecializationArgStrings(
            endToEndReq,
            endToEndReq->globalSpecializationArgStrings,
            globalSpecializationArgs);

        // Don't proceed further if anything failed to parse.
        if(sink->GetErrorCount())
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
        if(!specializedProgram)
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
        Index entryPointCount = endToEndReq->entryPoints.getCount();
        if( entryPointCount == 0 )
        {
            entryPointCount = unspecializedProgram->getEntryPointCount();
            endToEndReq->entryPoints.setCount(entryPointCount);
        }

        return specializedProgram;
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
        EndToEndCompileRequest*         endToEndReq,
        List<RefPtr<ComponentType>>&    outSpecializedEntryPoints)
    {
        auto specializedGlobalComponentType = endToEndReq->getSpecializedGlobalComponentType();

        List<RefPtr<ComponentType>> allComponentTypes;
        allComponentTypes.add(specializedGlobalComponentType);

        auto unspecializedGlobalAndEntryPointsComponentType = endToEndReq->getUnspecializedGlobalAndEntryPointsComponentType();
        auto entryPointCount = unspecializedGlobalAndEntryPointsComponentType->getEntryPointCount();

        for(Index ii = 0; ii < entryPointCount; ++ii)
        {
            auto& entryPointInfo = endToEndReq->entryPoints[ii];
            auto unspecializedEntryPoint = unspecializedGlobalAndEntryPointsComponentType->getEntryPoint(ii);

            auto specializedEntryPoint = createSpecializedEntryPoint(endToEndReq, unspecializedEntryPoint, entryPointInfo);
            allComponentTypes.add(specializedEntryPoint);

            outSpecializedEntryPoints.add(specializedEntryPoint);
        }

        RefPtr<ComponentType> composed = CompositeComponentType::create(endToEndReq->getLinkage(), allComponentTypes);
        return composed;
    }


}

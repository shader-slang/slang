// slang-check-modifier.cpp
#include "slang-check-impl.h"

// This file implements semantic checking behavior for
// modifiers.
//
// At present, the semantic checking we do on modifiers is primarily
// focused on `[attributes]`.

#include "slang-lookup.h"

namespace Slang
{
    RefPtr<ConstantIntVal> SemanticsVisitor::checkConstantIntVal(
        RefPtr<Expr>    expr)
    {
        // First type-check the expression as normal
        expr = CheckExpr(expr);

        auto intVal = CheckIntegerConstantExpression(expr.Ptr());
        if(!intVal)
            return nullptr;

        auto constIntVal = as<ConstantIntVal>(intVal);
        if(!constIntVal)
        {
            getSink()->diagnose(expr->loc, Diagnostics::expectedIntegerConstantNotLiteral);
            return nullptr;
        }
        return constIntVal;
    }

    RefPtr<ConstantIntVal> SemanticsVisitor::checkConstantEnumVal(
        RefPtr<Expr>    expr)
    {
        // First type-check the expression as normal
        expr = CheckExpr(expr);

        auto intVal = CheckEnumConstantExpression(expr.Ptr());
        if(!intVal)
            return nullptr;

        auto constIntVal = as<ConstantIntVal>(intVal);
        if(!constIntVal)
        {
            getSink()->diagnose(expr->loc, Diagnostics::expectedIntegerConstantNotLiteral);
            return nullptr;
        }
        return constIntVal;
    }

    // Check an expression, coerce it to the `String` type, and then
    // ensure that it has a literal (not just compile-time constant) value.
    bool SemanticsVisitor::checkLiteralStringVal(
        RefPtr<Expr>    expr,
        String*         outVal)
    {
        // TODO: This should actually perform semantic checking, etc.,
        // but for now we are just going to look for a direct string
        // literal AST node.

        if(auto stringLitExpr = as<StringLiteralExpr>(expr))
        {
            if(outVal)
            {
                *outVal = stringLitExpr->value;
            }
            return true;
        }

        getSink()->diagnose(expr, Diagnostics::expectedAStringLiteral);

        return false;
    }

    void SemanticsVisitor::visitModifier(Modifier*)
    {
        // Do nothing with modifiers for now
    }

    AttributeDecl* SemanticsVisitor::lookUpAttributeDecl(Name* attributeName, Scope* scope)
    {
        // Look up the name and see what we find.
        //
        // TODO: This needs to have some special filtering or naming
        // rules to keep us from seeing shadowing variable declarations.
        auto lookupResult = lookUp(getSession(), this, attributeName, scope, LookupMask::Attribute);

        // If the result was overloaded,
        // then we aren't going to be able to extract a single decl.
        if(lookupResult.isOverloaded())
            return nullptr;

        if (lookupResult.isValid())
        {
            auto decl = lookupResult.item.declRef.getDecl();
            if (auto attributeDecl = as<AttributeDecl>(decl))
            {
                return attributeDecl;
            }
            else
            {
                return nullptr;
            }
        }

        // If we couldn't find a system attribute, try looking up as a user defined attribute
        // A user defined attribute class is defined as a struct type with a "UserDefinedAttributeAttribute" modifier
        lookupResult = lookUp(getSession(), this, getSession()->getNameObj(attributeName->text + "Attribute"), scope, LookupMask::type);
        if (lookupResult.isOverloaded())
        {
            // see if we have already created an AttributeDecl for this attribute struct
            for (auto alt : lookupResult.items)
            {
                if (auto adecl = alt.declRef.as<AttributeDecl>())
                    return adecl.getDecl();
            }
        }
        // If we still cannot find any thing, quit
        if (!lookupResult.isValid() || lookupResult.isOverloaded())
            return nullptr;
        // Now construct an AttributeDecl for this user defined attribute class for future lookup
        auto userDefAttribAttrib = lookupResult.item.declRef.decl->FindModifier<AttributeUsageAttribute>();
        if (!userDefAttribAttrib)
            return nullptr;
        // create an AttributeDecl for the user defined attribute
        auto structAttribDef = lookupResult.item.declRef.as<StructDecl>().getDecl();
        RefPtr<AttributeDecl> attribDecl = new AttributeDecl();
        attribDecl->nameAndLoc = structAttribDef->nameAndLoc;
        attribDecl->loc = structAttribDef->loc;
        attribDecl->nextInContainerWithSameName = structAttribDef->nextInContainerWithSameName;
        // create a __attributeTarget modifier for the attribute class definition
        RefPtr<AttributeTargetModifier> targetModifier = new AttributeTargetModifier();
        targetModifier->syntaxClass = userDefAttribAttrib->targetSyntaxClass;
        targetModifier->loc = structAttribDef->loc;
        targetModifier->next = attribDecl->modifiers.first;
        attribDecl->modifiers.first = targetModifier;
        structAttribDef->nextInContainerWithSameName = attribDecl.Ptr();
        // we should create UserDefinedAttribute nodes for all user defined attribute instances
        attribDecl->syntaxClass = getSession()->findSyntaxClass(getSession()->getNameObj("UserDefinedAttribute"));
        for (auto member : structAttribDef->Members)
        {
            if (auto varMember = as<VarDecl>(member))
            {
                RefPtr<ParamDecl> param = new ParamDecl();
                param->nameAndLoc = member->nameAndLoc;
                param->type = varMember->type;
                param->loc = member->loc;
                attribDecl->Members.add(param);
            }
        }
        // add the attribute class definition to the syntax tree, so it can be found
        structAttribDef->ParentDecl->Members.add(attribDecl.Ptr());
        structAttribDef->ParentDecl->memberDictionaryIsValid = false;
        // do necessary checks on this newly constructed node
        checkDecl(attribDecl.Ptr());
        return attribDecl.Ptr();
    }

    bool SemanticsVisitor::hasIntArgs(Attribute* attr, int numArgs)
    {
        if (int(attr->args.getCount()) != numArgs)
        {
            return false;
        }
        for (int i = 0; i < numArgs; ++i)
        {
            if (!as<IntegerLiteralExpr>(attr->args[i]))
            {
                return false;
            }
        }
        return true;
    }

    bool SemanticsVisitor::hasStringArgs(Attribute* attr, int numArgs)
    {
        if (int(attr->args.getCount()) != numArgs)
        {
            return false;
        }
        for (int i = 0; i < numArgs; ++i)
        {
            if (!as<StringLiteralExpr>(attr->args[i]))
            {
                return false;
            }
        }
        return true;
    }

    bool SemanticsVisitor::getAttributeTargetSyntaxClasses(SyntaxClass<RefObject> & cls, uint32_t typeFlags)
    {
        if (typeFlags == (int)UserDefinedAttributeTargets::Struct)
        {
            cls = getSession()->findSyntaxClass(getSession()->getNameObj("StructDecl"));
            return true;
        }
        if (typeFlags == (int)UserDefinedAttributeTargets::Var)
        {
            cls = getSession()->findSyntaxClass(getSession()->getNameObj("VarDecl"));
            return true;
        }
        if (typeFlags == (int)UserDefinedAttributeTargets::Function)
        {
            cls = getSession()->findSyntaxClass(getSession()->getNameObj("FuncDecl"));
            return true;
        }
        return false;
    }

    bool SemanticsVisitor::validateAttribute(RefPtr<Attribute> attr, AttributeDecl* attribClassDecl)
    {
        if(auto numThreadsAttr = as<NumThreadsAttribute>(attr))
        {
            SLANG_ASSERT(attr->args.getCount() == 3);

            int32_t values[3];

            for (int i = 0; i < 3; ++i)
            {
                int32_t value = 1;

                auto arg = attr->args[i];
                if (arg)
                {
                    auto intValue = checkConstantIntVal(arg);
                    if (!intValue)
                    {
                        return false;
                    }
                    if (intValue->value < 1)
                    {
                        getSink()->diagnose(attr, Diagnostics::nonPositiveNumThreads, intValue->value);
                        return false;
                    }
                    value = int32_t(intValue->value);
                }
                values[i] = value;
            }

            numThreadsAttr->x          = values[0];
            numThreadsAttr->y          = values[1];
            numThreadsAttr->z          = values[2]; 
        }
        else if (auto bindingAttr = as<GLSLBindingAttribute>(attr))
        {
            // This must be vk::binding or gl::binding (as specified in core.meta.slang under vk_binding/gl_binding)
            // Must have 2 int parameters. Ideally this would all be checked from the specification
            // in core.meta.slang, but that's not completely implemented. So for now we check here.
            if (attr->args.getCount() != 2)
            {
                return false;
            }

            // TODO(JS): Prior validation currently doesn't ensure both args are ints (as specified in core.meta.slang), so check here
            // to make sure they both are
            auto binding = checkConstantIntVal(attr->args[0]);
            auto set = checkConstantIntVal(attr->args[1]);

            if (binding == nullptr || set == nullptr)
            {
                return false;
            }
                    
            bindingAttr->binding = int32_t(binding->value);
            bindingAttr->set = int32_t(set->value);
        }
        else if (auto maxVertexCountAttr = as<MaxVertexCountAttribute>(attr))
        {
            SLANG_ASSERT(attr->args.getCount() == 1);
            auto val = checkConstantIntVal(attr->args[0]);

            if(!val) return false;

            maxVertexCountAttr->value = (int32_t)val->value;
        }
        else if(auto instanceAttr = as<InstanceAttribute>(attr))
        {
            SLANG_ASSERT(attr->args.getCount() == 1);
            auto val = checkConstantIntVal(attr->args[0]);

            if(!val) return false;

            instanceAttr->value = (int32_t)val->value;
        }
        else if(auto entryPointAttr = as<EntryPointAttribute>(attr))
        {
            SLANG_ASSERT(attr->args.getCount() == 1);

            String stageName;
            if(!checkLiteralStringVal(attr->args[0], &stageName))
            {
                return false;
            }

            auto stage = findStageByName(stageName);
            if(stage == Stage::Unknown)
            {
                getSink()->diagnose(attr->args[0], Diagnostics::unknownStageName, stageName);
            }

            entryPointAttr->stage = stage;
        }
        else if ((as<DomainAttribute>(attr)) ||
                    (as<MaxTessFactorAttribute>(attr)) ||
                    (as<OutputTopologyAttribute>(attr)) ||
                    (as<PartitioningAttribute>(attr)) ||
                    (as<PatchConstantFuncAttribute>(attr)))
        {
            // Let it go thru iff single string attribute
            if (!hasStringArgs(attr, 1))
            {
                getSink()->diagnose(attr, Diagnostics::expectedSingleStringArg, attr->name);
            }
        }
        else if (as<OutputControlPointsAttribute>(attr))
        {
            // Let it go thru iff single integral attribute
            if (!hasIntArgs(attr, 1))
            {
                getSink()->diagnose(attr, Diagnostics::expectedSingleIntArg, attr->name);
            }
        }
        else if (as<PushConstantAttribute>(attr))
        {
            // Has no args
            SLANG_ASSERT(attr->args.getCount() == 0);
        }
        else if (as<ShaderRecordAttribute>(attr))
        {
            // Has no args
            SLANG_ASSERT(attr->args.getCount() == 0);
        }
        else if (as<EarlyDepthStencilAttribute>(attr))
        {
            // Has no args
            SLANG_ASSERT(attr->args.getCount() == 0);
        }
        else if (auto attrUsageAttr = as<AttributeUsageAttribute>(attr))
        {
            uint32_t targetClassId = (uint32_t)UserDefinedAttributeTargets::None;
            if (attr->args.getCount() == 1)
            {
                RefPtr<IntVal> outIntVal;
                if (auto cInt = checkConstantEnumVal(attr->args[0]))
                {
                    targetClassId = (uint32_t)(cInt->value);
                }
                else
                {
                    getSink()->diagnose(attr, Diagnostics::expectedSingleIntArg, attr->name);
                    return false;
                }
            }
            if (!getAttributeTargetSyntaxClasses(attrUsageAttr->targetSyntaxClass, targetClassId))
            {
                getSink()->diagnose(attr, Diagnostics::invalidAttributeTarget);
                return false;
            }
        }
        else if (auto unrollAttr = as<UnrollAttribute>(attr))
        {
            // Check has an argument. We need this because default behavior is to give an error
            // if an attribute has arguments, but not handled explicitly (and the default param will come through
            // as 1 arg if nothing is specified)
            SLANG_ASSERT(attr->args.getCount() == 1);
        }
        else if (auto userDefAttr = as<UserDefinedAttribute>(attr))
        {
            // check arguments against attribute parameters defined in attribClassDecl
            Index paramIndex = 0;
            auto params = attribClassDecl->getMembersOfType<ParamDecl>();
            for (auto paramDecl : params)
            {
                if (paramIndex < attr->args.getCount())
                {
                    auto & arg = attr->args[paramIndex];
                    bool typeChecked = false;
                    if (auto basicType = as<BasicExpressionType>(paramDecl->getType()))
                    {
                        if (basicType->baseType == BaseType::Int)
                        {
                            if (auto cint = checkConstantIntVal(arg))
                            {
                                attr->intArgVals[(uint32_t)paramIndex] = cint;
                            }
                            typeChecked = true;
                        }
                    }
                    if (!typeChecked)
                    {
                        arg = CheckExpr(arg);
                        arg = coerce(paramDecl->getType(), arg);
                    }
                }
                paramIndex++;
            }
            if (params.getCount() < attr->args.getCount())
            {
                getSink()->diagnose(attr, Diagnostics::tooManyArguments, attr->args.getCount(), params.getCount());
            }
            else if (params.getCount() > attr->args.getCount())
            {
                getSink()->diagnose(attr, Diagnostics::notEnoughArguments, attr->args.getCount(), params.getCount());
            }
        }
        else if (auto formatAttr = as<FormatAttribute>(attr))
        {
            SLANG_ASSERT(attr->args.getCount() == 1);

            String formatName;
            if(!checkLiteralStringVal(attr->args[0], &formatName))
            {
                return false;
            }

            ImageFormat format = ImageFormat::unknown;
            if(!findImageFormatByName(formatName.getBuffer(), &format))
            {
                getSink()->diagnose(attr->args[0], Diagnostics::unknownImageFormatName, formatName);
            }

            formatAttr->format = format;
        }
        else if (auto allowAttr = as<AllowAttribute>(attr))
        {
            SLANG_ASSERT(attr->args.getCount() == 1);

            String diagnosticName;
            if(!checkLiteralStringVal(attr->args[0], &diagnosticName))
            {
                return false;
            }

            auto diagnosticInfo = findDiagnosticByName(diagnosticName.getUnownedSlice());
            if(!diagnosticInfo)
            {
                getSink()->diagnose(attr->args[0], Diagnostics::unknownDiagnosticName, diagnosticName);
            }

            allowAttr->diagnostic = diagnosticInfo;
        }
        else
        {
            if(attr->args.getCount() == 0)
            {
                // If the attribute took no arguments, then we will
                // assume it is valid as written.
            }
            else
            {
                // We should be special-casing the checking of any attribute
                // with a non-zero number of arguments.
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), attr, "unhandled attribute");
                return false;
            }
        }

        return true;
    }

    RefPtr<AttributeBase> SemanticsVisitor::checkAttribute(
        UncheckedAttribute*     uncheckedAttr,
        ModifiableSyntaxNode*   attrTarget)
    {
        auto attrName = uncheckedAttr->getName();
        auto attrDecl = lookUpAttributeDecl(
            attrName,
            uncheckedAttr->scope);

        if(!attrDecl)
        {
            getSink()->diagnose(uncheckedAttr, Diagnostics::unknownAttributeName, attrName);
            return uncheckedAttr;
        }

        if(!attrDecl->syntaxClass.isSubClassOf<Attribute>())
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), attrDecl, "attribute declaration does not reference an attribute class");
            return uncheckedAttr;
        }

        // Manage scope
        RefPtr<RefObject> attrInstance = attrDecl->syntaxClass.createInstance();
        auto attr = attrInstance.as<Attribute>();
        if(!attr)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), attrDecl, "attribute class did not yield an attribute object");
            return uncheckedAttr;
        }

        // We are going to replace the unchecked attribute with the checked one.

        // First copy all of the state over from the original attribute.
        attr->name  = uncheckedAttr->name;
        attr->args  = uncheckedAttr->args;
        attr->loc   = uncheckedAttr->loc;

        // We will start with checking steps that can be applied independent
        // of the concrete attribute type that was selected. These only need
        // us to look at the attribute declaration itself.
        //
        // Start by doing argument/parameter matching
        UInt argCount = attr->args.getCount();
        UInt paramCounter = 0;
        bool mismatch = false;
        for(auto paramDecl : attrDecl->getMembersOfType<ParamDecl>())
        {
            UInt paramIndex = paramCounter++;
            if( paramIndex < argCount )
            {
                // TODO: support checking the argument against the declared
                // type for the parameter.
            }
            else
            {
                // We didn't have enough arguments for the
                // number of parameters declared.
                if(auto defaultArg = paramDecl->initExpr)
                {
                    // The attribute declaration provided a default,
                    // so we should use that.
                    //
                    // TODO: we need to figure out how to hook up
                    // default arguments as needed.
                    // For now just copy the expression over.

                    attr->args.add(paramDecl->initExpr);
                }
                else
                {
                    mismatch = true;
                }
            }
        }
        UInt paramCount = paramCounter;

        if(mismatch)
        {
            getSink()->diagnose(attr, Diagnostics::attributeArgumentCountMismatch, attrName, paramCount, argCount);
            return uncheckedAttr;
        }

        // The next bit of validation that we can apply semi-generically
        // is to validate that the target for this attribute is a valid
        // one for the chosen attribute.
        //
        // The attribute declaration will have one or more `AttributeTargetModifier`s
        // that each specify a syntax class that the attribute can be applied to.
        // If any of these match `attrTarget`, then we are good.
        //
        bool validTarget = false;
        for(auto attrTargetMod : attrDecl->GetModifiersOfType<AttributeTargetModifier>())
        {
            if(attrTarget->getClass().isSubClassOf(attrTargetMod->syntaxClass))
            {
                validTarget = true;
                break;
            }
        }
        if(!validTarget)
        {
            getSink()->diagnose(attr, Diagnostics::attributeNotApplicable, attrName);
            return uncheckedAttr;
        }

        // Now apply type-specific validation to the attribute.
        if(!validateAttribute(attr, attrDecl))
        {
            return uncheckedAttr;
        }


        return attr;
    }

    RefPtr<Modifier> SemanticsVisitor::checkModifier(
        RefPtr<Modifier>        m,
        ModifiableSyntaxNode*   syntaxNode)
    {
        if(auto hlslUncheckedAttribute = as<UncheckedAttribute>(m))
        {
            // We have an HLSL `[name(arg,...)]` attribute, and we'd like
            // to check that it is provides all the expected arguments
            //
            // First, look up the attribute name in the current scope to find
            // the right syntax class to instantiate.
            //

            return checkAttribute(hlslUncheckedAttribute, syntaxNode);
        }
        // Default behavior is to leave things as they are,
        // and assume that modifiers are mostly already checked.
        //
        // TODO: This would be a good place to validate that
        // a modifier is actually valid for the thing it is
        // being applied to, and potentially to check that
        // it isn't in conflict with any other modifiers
        // on the same declaration.

        return m;
    }


    void SemanticsVisitor::checkModifiers(ModifiableSyntaxNode* syntaxNode)
    {
        // TODO(tfoley): need to make sure this only
        // performs semantic checks on a `SharedModifier` once...

        // The process of checking a modifier may produce a new modifier in its place,
        // so we will build up a new linked list of modifiers that will replace
        // the old list.
        RefPtr<Modifier> resultModifiers;
        RefPtr<Modifier>* resultModifierLink = &resultModifiers;

        RefPtr<Modifier> modifier = syntaxNode->modifiers.first;
        while(modifier)
        {
            // Because we are rewriting the list in place, we need to extract
            // the next modifier here (not at the end of the loop).
            auto next = modifier->next;

            // We also go ahead and clobber the `next` field on the modifier
            // itself, so that the default behavior of `checkModifier()` can
            // be to return a single unlinked modifier.
            modifier->next = nullptr;

            auto checkedModifier = checkModifier(modifier, syntaxNode);
            if(checkedModifier)
            {
                // If checking gave us a modifier to add, then we
                // had better add it.

                // Just in case `checkModifier` ever returns multiple
                // modifiers, lets advance to the end of the list we
                // are building.
                while(*resultModifierLink)
                    resultModifierLink = &(*resultModifierLink)->next;

                // attach the new modifier at the end of the list,
                // and now set the "link" to point to its `next` field
                *resultModifierLink = checkedModifier;
                resultModifierLink = &checkedModifier->next;
            }

            // Move along to the next modifier
            modifier = next;
        }

        // Whether we actually re-wrote anything or note, lets
        // install the new list of modifiers on the declaration
        syntaxNode->modifiers.first = resultModifiers;
    }



}

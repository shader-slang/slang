// slang-ast-print.cpp
#include "slang-ast-print.h"

#include "slang-check-impl.h"

namespace Slang {

ASTPrinter::Part::Kind ASTPrinter::Part::getKind(ASTPrinter::Part::Type type)
{
    typedef ASTPrinter::Part::Kind Kind;
    typedef ASTPrinter::Part::Type Type;

    switch (type)
    {
        case Type::ParamType:           return Kind::Type;
        case Type::ParamName:           return Kind::Name;
        case Type::ReturnType:          return Kind::Type;
        case Type::DeclPath:            return Kind::Name;
        case Type::GenericParamType:    return Kind::Type;
        case Type::GenericParamValue:   return Kind::Value; 
        case Type::GenericParamValueType:    return Kind::Type;
        default: break;
    }
    return Kind::None;
}

void ASTPrinter::addType(Type* type)
{
    if (!type)
    {
        m_builder << "<error>";
        return;
    }
    type = type->getCanonicalType();
    if (m_optionFlags & OptionFlag::SimplifiedBuiltinType)
    {
        if (auto vectorType = as<VectorExpressionType>(type))
        {
            if (as<BasicExpressionType>(vectorType->getElementType()))
            {
                vectorType->getElementType()->toText(m_builder);
                if (as<ConstantIntVal>(vectorType->getElementCount()))
                {
                    m_builder << vectorType->getElementCount();
                    return;
                }
            }
        }
        else if (auto matrixType = as<MatrixExpressionType>(type))
        {
            auto elementType = matrixType->getElementType();
            if (as<BasicExpressionType>(elementType))
            {
                matrixType->getElementType()->toText(m_builder);
                if (as<ConstantIntVal>(matrixType->getRowCount()) &&
                    as<ConstantIntVal>(matrixType->getColumnCount()))
                {
                    m_builder << matrixType->getRowCount() << "x" << matrixType->getColumnCount();
                    return;
                }
            }
        }
    }
    type->toText(m_builder);
}

void ASTPrinter::addVal(Val* val)
{
    val->toText(m_builder);
}

/* static */void ASTPrinter::appendDeclName(Decl* decl, StringBuilder& out)
{
    if (as<ConstructorDecl>(decl))
    {
        out << "init";
    }
    else if (as<SubscriptDecl>(decl))
    {
        out << "subscript";
    }
    else
    {
        out << getText(decl->getName());
    }
}

void ASTPrinter::_addDeclName(Decl* decl)
{
    appendDeclName(decl, m_builder);
}

void ASTPrinter::addOverridableDeclPath(const DeclRef<Decl>& declRef)
{
    ScopePart scopePart(this, Part::Type::DeclPath);
    _addDeclPathRec(declRef, 0);
}

void ASTPrinter::addDeclPath(const DeclRef<Decl>& declRef)
{
    ScopePart scopePart(this, Part::Type::DeclPath);
    _addDeclPathRec(declRef, 1);
}

void ASTPrinter::_addDeclPathRec(const DeclRef<Decl>& declRef, Index depth)
{
    auto& sb = m_builder;

    // Find the parent declaration
    auto parentDeclRef = declRef.getParent();

    // If the immediate parent is a generic, then we probably
    // want the declaration above that...
    auto parentGenericDeclRef = parentDeclRef.as<GenericDecl>();
    if (parentGenericDeclRef)
    {
        parentDeclRef = parentGenericDeclRef.getParent();
    }

    // Depending on what the parent is, we may want to format things specially
    if (auto aggTypeDeclRef = parentDeclRef.as<AggTypeDecl>())
    {
        _addDeclPathRec(aggTypeDeclRef, depth + 1);
        sb << toSlice(".");
    }
    else if (auto namespaceDeclRef = parentDeclRef.as<NamespaceDecl>())
    {
        _addDeclPathRec(namespaceDeclRef, depth + 1);
        // Hmm, it could be argued that we follow the . as seen in AggType as is followed in some other languages
        // like Java.
        // That it is useful to have a distinction between something that is a member/method and something that is
        // in a scope (such as a namespace), and is something that has returned to later languages probably for that
        // reason (Slang accepts . or ::). So for now this is follows the :: convention.
        //
        // It could be argued them that the previous '.' use should vary depending on that distinction.
        
        sb << toSlice("::");
    }
    else if (auto extensionDeclRef = parentDeclRef.as<ExtensionDecl>())
    {
        ExtensionDecl* extensionDecl = as<ExtensionDecl>(parentDeclRef.getDecl());
        Type* type = extensionDecl->targetType.type;
        addType(type);
        sb << toSlice(".");
    }
    else if (auto moduleDecl = as<ModuleDecl>(parentDeclRef.getDecl()))
    {
        Name* moduleName = moduleDecl->getName();
        if ((m_optionFlags & OptionFlag::ModuleName) && moduleName)
        {
            // Use to say in modules scope
            sb << moduleName->text << toSlice("::");
        }
    }

    // If this decl is the module, we only output it's name if that feature is enabled
    if (ModuleDecl* moduleDecl = as<ModuleDecl>(declRef.getDecl()))
    {
        Name* moduleName = moduleDecl->getName();
        if ((m_optionFlags & OptionFlag::ModuleName) && moduleName)
        {
            sb << moduleName->text; 
        }
        return;
    }

    _addDeclName(declRef.getDecl());

    // If the parent declaration is a generic, then we need to print out its
    // signature
    if (parentGenericDeclRef && 
        !declRef.as<GenericValueParamDecl>() &&
        !declRef.as<GenericTypeParamDecl>())
    {
        auto substArgs = tryGetGenericArguments(SubstitutionSet(declRef), parentGenericDeclRef.getDecl());
        if (substArgs.getCount())
        {
            // If the name we printed previously was an operator
            // that ends with `<`, then immediately printing the
            // generic arguments inside `<...>` may cause it to
            // be hard to parse the operator name visually.
            //
            // We thus include a space between the declaration name
            // and its generic arguments in this case.
            //
            if (sb.endsWith("<"))
            {
                sb << " ";
            }

            sb << "<";
            bool first = true;
            for (auto arg : substArgs)
            {
                // When printing the representation of a specialized
                // generic declaration we don't want to include the
                // argument values for subtype witnesses since these
                // do not correspond to parameters of the generic
                // as the user sees it.
                //
                if (as<Witness>(arg))
                    continue;

                if (!first) sb << ", ";
                addVal(arg);
                first = false;
            }
            sb << ">";
        }
        else if (depth > 0)
        {
            // Write out the generic parameters (only if the depth allows it)
            addGenericParams(parentGenericDeclRef);
        }
    }
}

void ASTPrinter::addGenericParams(const DeclRef<GenericDecl>& genericDeclRef)
{
    auto& sb = m_builder;

    sb << "<";
    bool first = true;
    for (auto paramDeclRef : getMembers(m_astBuilder, genericDeclRef))
    {
        if (auto genericTypeParam = paramDeclRef.as<GenericTypeParamDecl>())
        {
            if (!first) sb << ", ";
            first = false;

            {
                ScopePart scopePart(this, Part::Type::GenericParamType);
                sb << getText(genericTypeParam.getName());
            }
        }
        else if (auto genericValParam = paramDeclRef.as<GenericValueParamDecl>())
        {
            if (!first) sb << ", ";
            first = false;

            {
                ScopePart scopePart(this, Part::Type::GenericParamValue);
                sb << getText(genericValParam.getName());
            }

            sb << ":";

            {
                ScopePart scopePart(this, Part::Type::GenericParamValueType);
                addType(getType(m_astBuilder, genericValParam));
            }
        }
        else
        {
        }
    }
    sb << ">";
}

void ASTPrinter::addDeclParams(const DeclRef<Decl>& declRef, List<Range<Index>>* outParamRange)
{
    auto& sb = m_builder;

    if (auto funcDeclRef = declRef.as<CallableDecl>())
    {
        // This is something callable, so we need to also print parameter types for overloading
        sb << "(";

        bool first = true;
        for (auto paramDeclRef : getParameters(m_astBuilder, funcDeclRef))
        {
            if (!first) sb << ", ";

            auto rangeStart = sb.getLength();

            ParamDecl* paramDecl = paramDeclRef.getDecl();

            {
                ScopePart scopePart(this, Part::Type::ParamType);

                // Seems these apply to parameters/VarDeclBase and are not part of the 'type'
                // but seems more appropriate to put in the Type Part

                if (paramDecl->hasModifier<InOutModifier>())
                {
                    sb << toSlice("inout ");
                }
                else if (paramDecl->hasModifier<OutModifier>())
                {
                    sb << toSlice("out ");
                }
                else if (paramDecl->hasModifier<InModifier>())
                {
                    sb << toSlice("in ");
                }

                // And this to params/variables (not the type)
                if (paramDecl->hasModifier<ConstModifier>())
                {
                    sb << toSlice("const ");
                }

                addType(getType(m_astBuilder, paramDeclRef));
            }

            // Output the parameter name if there is one, and it's enabled in the options
            if (m_optionFlags & OptionFlag::ParamNames && paramDecl->getName())
            {
                sb << " ";

                {
                    ScopePart scopePart(this, Part::Type::ParamName);
                    sb << paramDecl->getName()->text;
                }
            }

            auto rangeEnd = sb.getLength();

            if (outParamRange)
                outParamRange->add(makeRange<Index>(rangeStart, rangeEnd));

            first = false;
        }

        sb << ")";
    }
    else if (auto genericDeclRef = declRef.as<GenericDecl>())
    {
        addGenericParams(genericDeclRef);

        addDeclParams(m_astBuilder->getMemberDeclRef(genericDeclRef, genericDeclRef.getDecl()->inner), outParamRange);
    }
    else
    {
    }
}

void ASTPrinter::addDeclKindPrefix(Decl* decl)
{
    if (auto genericDecl = as<GenericDecl>(decl))
    {
        decl = genericDecl->inner;
    }
    for (auto modifier : decl->modifiers)
    {
        if (modifier->getKeywordName())
        {
            if (m_optionFlags & OptionFlag::NoInternalKeywords)
            {
                if (as<TargetIntrinsicModifier>(modifier))
                    continue;
                if (as<MagicTypeModifier>(modifier))
                    continue;
                if (as<IntrinsicOpModifier>(modifier))
                    continue;
                if (as<IntrinsicTypeModifier>(modifier))
                    continue;
                if (as<BuiltinModifier>(modifier))
                    continue;
                if (as<BuiltinRequirementModifier>(modifier))
                    continue;
                if (as<BuiltinTypeModifier>(modifier))
                    continue;
                if (as<SpecializedForTargetModifier>(modifier))
                    continue;
                if (as<AttributeTargetModifier>(modifier))
                    continue;
                if (as<RequiredCUDASMVersionModifier>(modifier))
                    continue;
                if (as<RequiredSPIRVVersionModifier>(modifier))
                    continue;
                if (as<RequiredGLSLVersionModifier>(modifier))
                    continue;
                if (as<RequiredGLSLExtensionModifier>(modifier))
                    continue;
                if (as<GLSLLayoutModifier>(modifier))
                    continue;
                if (as<GLSLLayoutModifierGroupMarker>(modifier))
                    continue;
                if (as<HLSLLayoutSemantic>(modifier))
                    continue;
            }
            // Don't print out attributes.
            if (as<AttributeBase>(modifier))
                continue;
            m_builder << modifier->getKeywordName()->text << " ";
        }
    }
    if (as<FuncDecl>(decl))
    {
        m_builder << "func ";
    }
    else if (as<StructDecl>(decl))
    {
        m_builder << "struct ";
    }
    else if (as<InterfaceDecl>(decl))
    {
        m_builder << "interface ";
    }
    else if (as<ClassDecl>(decl))
    {
        m_builder << "class ";
    }
    else if (auto typedefDecl = as<TypeDefDecl>(decl))
    {
        m_builder << "typedef ";
        if (typedefDecl->type.type)
        {
            addType(typedefDecl->type.type);
            m_builder << " ";
        }
    }
    else if (const auto propertyDecl = as<PropertyDecl>(decl))
    {
        m_builder << "property ";
    }
    else if (as<NamespaceDecl>(decl))
    {
        m_builder << "namespace ";
    }
    else if (auto varDecl = as<VarDeclBase>(decl))
    {
        if (varDecl->getType())
        {
            addType(varDecl->getType());
            m_builder << " ";
        }
    }
    else if (as<EnumDecl>(decl))
    {
        m_builder << "enum ";
    }
    else if (auto enumCase = as<EnumCaseDecl>(decl))
    {
        if (enumCase->getType())
        {
            addType(enumCase->getType());
            m_builder << " ";
        }
    }
    else if (const auto assocType = as<AssocTypeDecl>(decl))
    {
        m_builder << "associatedtype ";
    }
    else if (const auto attribute = as<AttributeDecl>(decl))
    {
        m_builder << "attribute ";
    }
}

void ASTPrinter::addDeclResultType(const DeclRef<Decl>& inDeclRef)
{
    DeclRef<Decl> declRef = inDeclRef;
    if (auto genericDeclRef = declRef.as<GenericDecl>())
    {
        declRef = m_astBuilder->getMemberDeclRef<Decl>(genericDeclRef, genericDeclRef.getDecl()->inner);
    }

    if (declRef.as<ConstructorDecl>())
    {
    }
    else if (auto callableDeclRef = declRef.as<CallableDecl>())
    {
        m_builder << " -> ";

        {
            ScopePart scopePart(this, Part::Type::ReturnType);
            addType(getResultType(m_astBuilder, callableDeclRef));
        }
    }
    else if (auto propertyDecl = declRef.as<PropertyDecl>())
    {
        if (propertyDecl.getDecl()->type.type)
        {
            m_builder << " : ";
            addType(declRef.substitute(m_astBuilder, propertyDecl.getDecl()->type.type));
        }
    }
}

/* static */void ASTPrinter::addDeclSignature(const DeclRef<Decl>& declRef)
{
    addDeclKindPrefix(declRef.getDecl());
    addDeclPath(declRef);
    addDeclParams(declRef);
    addDeclResultType(declRef);
}

/* static */String ASTPrinter::getDeclSignatureString(DeclRef<Decl> declRef, ASTBuilder* astBuilder)
{
    ASTPrinter astPrinter(
        astBuilder,
        ASTPrinter::OptionFlag::NoInternalKeywords | ASTPrinter::OptionFlag::SimplifiedBuiltinType);
    astPrinter.addDeclSignature(declRef);
    return astPrinter.getString();
}

/* static */String ASTPrinter::getDeclSignatureString(const LookupResultItem& item, ASTBuilder* astBuilder)
{
    return getDeclSignatureString(item.declRef, astBuilder);
}

/* static */UnownedStringSlice ASTPrinter::getPart(Part::Type partType, const UnownedStringSlice& slice, const List<Part>& parts)
{
    const Index index = parts.findFirstIndex([&](const Part& part) -> bool { return part.type == partType; });
    return index >= 0 ? getPart(slice, parts[index]) : UnownedStringSlice();
}

UnownedStringSlice ASTPrinter::getPartSlice(Part::Type partType) const
{
    return m_parts ? getPart(partType, getSlice(), *m_parts) : UnownedStringSlice();
}


} // namespace Slang

// slang-ast-print.cpp
#include "slang-ast-print.h"

#include "slang-check-impl.h"

namespace Slang {

void ASTPrinter::addType(Type* type)
{
    m_builder << type->toString();
}

void ASTPrinter::addVal(Val* val)
{
    m_builder << val->toString();
}

void ASTPrinter::addDeclName(Decl* decl)
{
    if (as<ConstructorDecl>(decl))
    {
        m_builder << "init";
    }
    else if (as<SubscriptDecl>(decl))
    {
        m_builder << "subscript";
    }
    else
    {
        m_builder << getText(decl->getName());
    }
}

void ASTPrinter::addDeclPath(const DeclRef<Decl>& declRef)
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
        addDeclPath(aggTypeDeclRef);
        sb << ".";
    }

    addDeclName(declRef.getDecl());

    // If the parent declaration is a generic, then we need to print out its
    // signature
    if (parentGenericDeclRef)
    {
        auto genSubst = as<GenericSubstitution>(declRef.substitutions.substitutions);
        SLANG_RELEASE_ASSERT(genSubst);
        SLANG_RELEASE_ASSERT(genSubst->genericDecl == parentGenericDeclRef.getDecl());

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
        for (auto arg : genSubst->args)
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
}

void ASTPrinter::addDeclParams(const DeclRef<Decl>& declRef)
{
    auto& sb = m_builder;

    if (auto funcDeclRef = declRef.as<CallableDecl>())
    {
        // This is something callable, so we need to also print parameter types for overloading
        sb << "(";

        bool first = true;
        for (auto paramDeclRef : getParameters(funcDeclRef))
        {
            if (!first) sb << ", ";

            addType(getType(m_astBuilder, paramDeclRef));

            first = false;
        }

        sb << ")";
    }
    else if (auto genericDeclRef = declRef.as<GenericDecl>())
    {
        sb << "<";
        bool first = true;
        for (auto paramDeclRef : getMembers(genericDeclRef))
        {
            if (auto genericTypeParam = paramDeclRef.as<GenericTypeParamDecl>())
            {
                if (!first) sb << ", ";
                first = false;

                sb << getText(genericTypeParam.getName());
            }
            else if (auto genericValParam = paramDeclRef.as<GenericValueParamDecl>())
            {
                if (!first) sb << ", ";
                first = false;

                sb << getText(genericValParam.getName());
                sb << ":";
                addType(getType(m_astBuilder, genericValParam));
            }
            else
            {
            }
        }
        sb << ">";

        addDeclParams(DeclRef<Decl>(getInner(genericDeclRef), genericDeclRef.substitutions));
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
    if (as<FuncDecl>(decl))
    {
        m_builder << "func ";
    }
}

void ASTPrinter::addDeclResultType(const DeclRef<Decl>& inDeclRef)
{
    DeclRef<Decl> declRef = inDeclRef;
    if (auto genericDeclRef = declRef.as<GenericDecl>())
    {
        declRef = DeclRef<Decl>(getInner(genericDeclRef), genericDeclRef.substitutions);
    }

    if (as<ConstructorDecl>(declRef))
    {
    }
    else if (auto callableDeclRef = declRef.as<CallableDecl>())
    {
        m_builder << " -> ";
        addType(getResultType(m_astBuilder, callableDeclRef));
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
    ASTPrinter astPrinter(astBuilder);
    astPrinter.addDeclSignature(declRef);
    return astPrinter.getString();
}

/* static */String ASTPrinter::getDeclSignatureString(const LookupResultItem& item, ASTBuilder* astBuilder)
{
    return getDeclSignatureString(item.declRef, astBuilder);
}

} // namespace Slang

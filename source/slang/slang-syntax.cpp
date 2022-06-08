#include "slang-syntax.h"

#include "slang-compiler.h"
#include "slang-visitor.h"

#include <typeinfo>
#include <assert.h>

namespace Slang
{

/* static */const TypeExp TypeExp::empty;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! DiagnosticSink impls !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void printDiagnosticArg(StringBuilder& sb, Decl* decl)
{
    sb << getText(decl->getName());
}

void printDiagnosticArg(StringBuilder& sb, Type* type)
{
    type->toText(sb);
}

void printDiagnosticArg(StringBuilder& sb, Val* val)
{
    val->toText(sb);
}

void printDiagnosticArg(StringBuilder& sb, TypeExp const& type)
{
    if (type.type)
        type.type->toText(sb);
    else
        sb << "<null>";
}

void printDiagnosticArg(StringBuilder& sb, QualType const& type)
{
    if (type.type)
        type.type->toText(sb);
    else
        sb << "<null>";
}

SourceLoc const& getDiagnosticPos(SyntaxNode const* syntax)
{
    return syntax->loc;
}

SourceLoc const& getDiagnosticPos(TypeExp const& typeExp)
{
    return typeExp.exp->loc;
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!  Free functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Decl*const* adjustFilterCursorImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end)
{
    switch (filterStyle)
    {
        default:
        case MemberFilterStyle::All:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo))
                {
                    return ptr;
                }
            }
            break;
        }
        case MemberFilterStyle::Instance:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && !decl->hasModifier<HLSLStaticModifier>())
                {
                    return ptr;
                }
            }
            break;
        }
        case MemberFilterStyle::Static:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && decl->hasModifier<HLSLStaticModifier>())
                {
                    return ptr;
                }
            }
            break;
        }
    }
    return end;
}

Decl*const* getFilterCursorByIndexImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end, Index index)
{
    switch (filterStyle)
    {
        default:
        case MemberFilterStyle::All:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo))
                {
                    if (index <= 0)
                    {
                        return ptr;
                    }
                    index--;
                }
            }
            break;
        }
        case MemberFilterStyle::Instance:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && !decl->hasModifier<HLSLStaticModifier>())
                {
                    if (index <= 0)
                    {
                        return ptr;
                    }
                    index--;
                }
            }
            break;
        }
        case MemberFilterStyle::Static:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                if (decl->getClassInfo().isSubClassOf(clsInfo) && decl->hasModifier<HLSLStaticModifier>())
                {
                    if (index <= 0)
                    {
                        return ptr;
                    }
                    index--;
                }
            }
            break;
        }
    }
    return nullptr;
}

Index getFilterCountImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end)
{
    Index count = 0;
    switch (filterStyle)
    {
        default:
        case MemberFilterStyle::All:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                count += Index(decl->getClassInfo().isSubClassOf(clsInfo));
            }
            break;
        }
        case MemberFilterStyle::Instance:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                count += Index(decl->getClassInfo().isSubClassOf(clsInfo)&& !decl->hasModifier<HLSLStaticModifier>());
            }
            break;
        }
        case MemberFilterStyle::Static:
        {
            for (; ptr != end; ptr++)
            {
                Decl* decl = *ptr;
                count += Index(decl->getClassInfo().isSubClassOf(clsInfo) && decl->hasModifier<HLSLStaticModifier>());
            }
            break;
        }
    }
    return count;
}

    // TypeExp

    bool TypeExp::equals(Type* other)
    {
        return type->equals(other);
    }

    //
    // RequirementWitness
    //

    RequirementWitness::RequirementWitness(Val* val)
        : m_flavor(Flavor::val)
        , m_val(val)
    {}


    RequirementWitness::RequirementWitness(RefPtr<WitnessTable> witnessTable)
        : m_flavor(Flavor::witnessTable)
        , m_obj(witnessTable)
    {}

    RefPtr<WitnessTable> RequirementWitness::getWitnessTable()
    {
        SLANG_ASSERT(getFlavor() == Flavor::witnessTable);
        return m_obj.as<WitnessTable>();
    }


    RequirementWitness RequirementWitness::specialize(ASTBuilder* astBuilder, SubstitutionSet const& subst)
    {
        switch(getFlavor())
        {
        default:
            SLANG_UNEXPECTED("unknown requirement witness flavor");
        case RequirementWitness::Flavor::none:
            return RequirementWitness();

        case RequirementWitness::Flavor::declRef:
            {
                int diff = 0;
                return RequirementWitness(
                    getDeclRef().substituteImpl(astBuilder, subst, &diff));
            }

        case RequirementWitness::Flavor::val:
            {
                auto val = getVal();
                SLANG_ASSERT(val);

                return RequirementWitness(
                    val->substitute(astBuilder, subst));
            }
        }
    }

    RequirementWitness tryLookUpRequirementWitness(
        ASTBuilder*     astBuilder,
        SubtypeWitness* subtypeWitness,
        Decl*           requirementKey)
    {
        if(auto declaredSubtypeWitness = as<DeclaredSubtypeWitness>(subtypeWitness))
        {
            if(auto inheritanceDeclRef = declaredSubtypeWitness->declRef.as<InheritanceDecl>())
            {
                // A conformance that was declared as part of an inheritance clause
                // will have built up a dictionary of the satisfying declarations
                // for each of its requirements.
                RequirementWitness requirementWitness;
                auto witnessTable = inheritanceDeclRef.getDecl()->witnessTable;
                if(witnessTable && witnessTable->requirementDictionary.TryGetValue(requirementKey, requirementWitness))
                {
                    // The `inheritanceDeclRef` has substitutions applied to it that
                    // *aren't* present in the `requirementWitness`, because it was
                    // derived by the front-end when looking at the `InheritanceDecl` alone.
                    //
                    // We need to apply these substitutions here for the result to make sense.
                    //
                    // E.g., if we have a case like:
                    //
                    //      interface ISidekick { associatedtype Hero; void follow(Hero hero); }
                    //      struct Sidekick<H> : ISidekick { typedef H Hero; void follow(H hero) {} };
                    //
                    //      void followHero<S : ISidekick>(S s, S.Hero h)
                    //      {
                    //          s.follow(h);
                    //      }
                    //
                    //      Batman batman;
                    //      Sidekick<Batman> robin;
                    //      followHero<Sidekick<Batman>>(robin, batman);
                    //
                    // The second argument to `followHero` is `batman`, which has type `Batman`.
                    // The parameter declaration lists the type `S.Hero`, which is a reference
                    // to an associated type. The front  end will expand this into something
                    // like `S.{S:ISidekick}.Hero` - that is, we'll end up with a declaration
                    // reference to `ISidekick.Hero` with a this-type substitution that references
                    // the `{S:ISidekick}` declaration as a witness.
                    //
                    // The front-end will expand the generic application `followHero<Sidekick<Batman>>`
                    // to `followHero<Sidekick<Batman>, {Sidekick<H>:ISidekick}[H->Batman]>`
                    // (that is, the hidden second parameter will reference the inheritance
                    // clause on `Sidekick<H>`, with a substitution to map `H` to `Batman`.
                    //
                    // This step should map the `{S:ISidekick}` declaration over to the
                    // concrete `{Sidekick<H>:ISidekick}[H->Batman]` inheritance declaration.
                    // At that point `tryLookupRequirementWitness` might be called, because
                    // we want to look up the witness for the key `ISidekick.Hero` in the
                    // inheritance decl-ref that is `{Sidekick<H>:ISidekick}[H->Batman]`.
                    //
                    // That lookup will yield us a reference to the typedef `Sidekick<H>.Hero`,
                    // *without* any substitution for `H` (or rather, with a default one that
                    // maps `H` to `H`.
                    //
                    // So, in order to get the *right* end result, we need to apply
                    // the substitutions from the inheritance decl-ref to the witness.
                    //
                    requirementWitness = requirementWitness.specialize(astBuilder, inheritanceDeclRef.substitutions);

                    return requirementWitness;
                }
            }
        }

        // TODO: should handle the transitive case here too

        return RequirementWitness();
    }

    //
    // WitnessTable
    //

    void WitnessTable::add(Decl* decl, RequirementWitness const& witness)
    {
        SLANG_ASSERT(!requirementDictionary.ContainsKey(decl));

        requirementDictionary.Add(decl, witness);
        requirementList.add(KeyValuePair<Decl*, RequirementWitness>(decl, witness));
    }

    //

    static Type* ExtractGenericArgType(Val* val)
    {
        auto type = as<Type>(val);
        SLANG_RELEASE_ASSERT(type);
        return type;
    }

    static IntVal* ExtractGenericArgInteger(Val* val)
    {
        auto intVal = as<IntVal>(val);
        SLANG_RELEASE_ASSERT(intVal);
        return intVal;
    }

    DeclRef<Decl> createDefaultSubstitutionsIfNeeded(
        ASTBuilder*     astBuilder,
        DeclRef<Decl>   declRef)
    {
        // It is possible that `declRef` refers to a generic type,
        // but does not specify arguments for its generic parameters.
        // (E.g., this happens when referring to a generic type from
        // within its own member functions). To handle this case,
        // we will construct a default specialization at the use
        // site if needed.
        //
        // This same logic should also apply to declarations nested
        // more than one level inside of a generic (e.g., a `typdef`
        // inside of a generic `struct`).
        //
        // Similarly, it needs to work for multiple levels of
        // nested generics.
        //

        // We are going to build up a list of substitutions that need
        // to be applied to the decl-ref to make it specialized.
        Substitutions* substsToApply = nullptr;
        Substitutions** link = &substsToApply;

        Decl* dd = declRef.getDecl();
        for(;;)
        {
            Decl* childDecl = dd;
            Decl* parentDecl = dd->parentDecl;
            if(!parentDecl)
                break;

            dd = parentDecl;

            if(auto genericParentDecl = as<GenericDecl>(parentDecl))
            {
                // Don't specialize any parameters of a generic.
                if(childDecl != genericParentDecl->inner)
                    break;

                // We have a generic ancestor, but do we have an substitutions for it?
                GenericSubstitution* foundSubst = nullptr;
                for(auto s = declRef.substitutions.substitutions; s; s = s->outer)
                {
                    auto genSubst = as<GenericSubstitution>(s);
                    if(!genSubst)
                        continue;

                    if(genSubst->genericDecl != genericParentDecl)
                        continue;

                    // Okay, we found a matching substitution,
                    // so there is nothing to be done.
                    foundSubst = genSubst;
                    break;
                }

                if(!foundSubst)
                {
                    Substitutions* newSubst = createDefaultSubstitutionsForGeneric(
                        astBuilder, 
                        genericParentDecl,
                        nullptr);

                    *link = newSubst;
                    link = &newSubst->outer;
                }
            }
        }

        if(!substsToApply)
            return declRef;

        int diff = 0;
        return declRef.substituteImpl(astBuilder, substsToApply, &diff);
    }

    // TODO: need to figure out how to unify this with the logic
    // in the generic case...
    DeclRefType* DeclRefType::create(
        ASTBuilder*     astBuilder,
        DeclRef<Decl>   declRef)
    {
        declRef = createDefaultSubstitutionsIfNeeded(astBuilder, declRef);

        if (auto builtinMod = declRef.getDecl()->findModifier<BuiltinTypeModifier>())
        {
            auto type = astBuilder->create<BasicExpressionType>(builtinMod->tag);
            type->declRef = declRef;
            return type;
        }
        else if (auto magicMod = declRef.getDecl()->findModifier<MagicTypeModifier>())
        {
            GenericSubstitution* subst = nullptr;
            for(auto s = declRef.substitutions.substitutions; s; s = s->outer)
            {
                if(auto genericSubst = as<GenericSubstitution>(s))
                {
                    subst = genericSubst;
                    break;
                }
            }

            if (magicMod->magicName == "SamplerState")
            {
                auto type = astBuilder->create<SamplerStateType>();
                type->declRef = declRef;
                type->flavor = SamplerStateFlavor(magicMod->tag);
                return type;
            }
            else if (magicMod->magicName == "Vector")
            {
                SLANG_ASSERT(subst && subst->args.getCount() == 2);
                auto vecType = astBuilder->create<VectorExpressionType>();
                vecType->declRef = declRef;
                vecType->elementType = ExtractGenericArgType(subst->args[0]);
                vecType->elementCount = ExtractGenericArgInteger(subst->args[1]);
                return vecType;
            }
            else if (magicMod->magicName == "Matrix")
            {
                SLANG_ASSERT(subst && subst->args.getCount() == 3);
                auto matType = astBuilder->create<MatrixExpressionType>();
                matType->declRef = declRef;
                return matType;
            }
            else if (magicMod->magicName == "Texture")
            {
                SLANG_ASSERT(subst && subst->args.getCount() >= 1);
                auto textureType = astBuilder->create<TextureType>(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->magicName == "TextureSampler")
            {
                SLANG_ASSERT(subst && subst->args.getCount() >= 1);
                auto textureType = astBuilder->create<TextureSamplerType>(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->magicName == "GLSLImageType")
            {
                SLANG_ASSERT(subst && subst->args.getCount() >= 1);
                auto textureType = astBuilder->create<GLSLImageType>(
                    TextureFlavor(magicMod->tag),
                    ExtractGenericArgType(subst->args[0]));
                textureType->declRef = declRef;
                return textureType;
            }
            else if (magicMod->magicName == "FeedbackType")
            {
                SLANG_ASSERT(subst == nullptr);
                auto type = astBuilder->create<FeedbackType>();
                type->declRef = declRef;
                type->kind = FeedbackType::Kind(magicMod->tag);
                return type;
            }

            // TODO: eventually everything should follow this pattern,
            // and we can drive the dispatch with a table instead
            // of this ridiculously slow `if` cascade.

        #define CASE(n,T)													\
            else if(magicMod->magicName == #n) {									\
                auto type = astBuilder->create<T>();						\
                type->declRef = declRef;									\
                return type;												\
            }

            CASE(HLSLInputPatchType, HLSLInputPatchType)
            CASE(HLSLOutputPatchType, HLSLOutputPatchType)

        #undef CASE

            #define CASE(n,T)													\
                else if(magicMod->magicName == #n) {									\
                    SLANG_ASSERT(subst && subst->args.getCount() == 1);			\
                    auto type = astBuilder->create<T>();						\
                    type->elementType = ExtractGenericArgType(subst->args[0]);	\
                    type->declRef = declRef;									\
                    return type;												\
                }

            CASE(ConstantBuffer, ConstantBufferType)
            CASE(TextureBuffer, TextureBufferType)
            CASE(ParameterBlockType, ParameterBlockType)
            CASE(GLSLInputParameterGroupType, GLSLInputParameterGroupType)
            CASE(GLSLOutputParameterGroupType, GLSLOutputParameterGroupType)
            CASE(GLSLShaderStorageBufferType, GLSLShaderStorageBufferType)

            CASE(HLSLStructuredBufferType, HLSLStructuredBufferType)
            CASE(HLSLRWStructuredBufferType, HLSLRWStructuredBufferType)
            CASE(HLSLRasterizerOrderedStructuredBufferType, HLSLRasterizerOrderedStructuredBufferType)
            CASE(HLSLAppendStructuredBufferType, HLSLAppendStructuredBufferType)
            CASE(HLSLConsumeStructuredBufferType, HLSLConsumeStructuredBufferType)

            CASE(HLSLPointStreamType, HLSLPointStreamType)
            CASE(HLSLLineStreamType, HLSLLineStreamType)
            CASE(HLSLTriangleStreamType, HLSLTriangleStreamType)

            #undef CASE

            // "magic" builtin types which have no generic parameters
            #define CASE(n,T)													\
                else if(magicMod->magicName == #n) {									\
                    auto type = astBuilder->create<T>();						\
                    type->declRef = declRef;									\
                    return type;												\
                }

            CASE(HLSLByteAddressBufferType, HLSLByteAddressBufferType)
            CASE(HLSLRWByteAddressBufferType, HLSLRWByteAddressBufferType)
            CASE(HLSLRasterizerOrderedByteAddressBufferType, HLSLRasterizerOrderedByteAddressBufferType)
            CASE(UntypedBufferResourceType, UntypedBufferResourceType)

            CASE(GLSLInputAttachmentType, GLSLInputAttachmentType)

            #undef CASE

            else
            {
                auto classInfo = astBuilder->findSyntaxClass(magicMod->magicName.getUnownedSlice());
                if (!classInfo.classInfo)
                {
                    SLANG_UNEXPECTED("unhandled type");
                }

                NodeBase* type = classInfo.createInstance(astBuilder);
                if (!type)
                {
                    SLANG_UNEXPECTED("constructor failure");
                }

                auto declRefType = dynamicCast<DeclRefType>(type);
                if (!declRefType)
                {
                    SLANG_UNEXPECTED("expected a declaration reference type");
                }
                declRefType->declRef = declRef;
                return declRefType;
            }
        }
        else
        {
            return astBuilder->create<DeclRefType>(declRef);
        }
    }

    //

    GenericSubstitution* findInnerMostGenericSubstitution(Substitutions* subst)
    {
        for(Substitutions* s = subst; s; s = s->outer)
        {
            if(auto genericSubst = as<GenericSubstitution>(s))
                return genericSubst;
        }
        return nullptr;
    }

   
    // DeclRefBase

    Type* DeclRefBase::substitute(ASTBuilder* astBuilder, Type* type) const
    {
        // Note that type can be nullptr, and so this function can return nullptr (although only correctly when no substitutions) 

        // No substitutions? Easy.
        if (!substitutions)
            return type;

        SLANG_ASSERT(type);

        // Otherwise we need to recurse on the type structure
        // and apply substitutions where it makes sense
        return Slang::as<Type>(type->substitute(astBuilder, substitutions));
    }

    DeclRefBase DeclRefBase::substitute(ASTBuilder* astBuilder, DeclRefBase declRef) const
    {
        if(!substitutions)
            return declRef;

        int diff = 0;
        return declRef.substituteImpl(astBuilder, substitutions, &diff);
    }

    SubstExpr<Expr> DeclRefBase::substitute(ASTBuilder* /* astBuilder*/, Expr* expr) const
    {
        return SubstExpr<Expr>(expr, substitutions);
    }

    SubstExpr<Expr> substituteExpr(SubstitutionSet const& substs, Expr* expr)
    {
        return SubstExpr<Expr>(expr, substs);
    }

    DeclRef<Decl> substituteDeclRef(SubstitutionSet const& substs, ASTBuilder* astBuilder, DeclRef<Decl> const& declRef)
    {
        if(!substs)
            return declRef;

        int diff = 0;
        auto declRefBase = declRef.substituteImpl(astBuilder, substs, &diff);
        return DeclRef<Decl>(declRefBase.decl, declRefBase.substitutions);
    }

    Type* substituteType(SubstitutionSet const& substs, ASTBuilder* astBuilder, Type* type)
    {
        if(!type) return nullptr;
        if(!substs) return type;

        SLANG_ASSERT(type);

        return Slang::as<Type>(type->substitute(astBuilder, substs));
    }


    void buildMemberDictionary(ContainerDecl* decl);

    InterfaceDecl* findOuterInterfaceDecl(Decl* decl)
    {
        Decl* dd = decl;
        while(dd)
        {
            if(auto interfaceDecl = as<InterfaceDecl>(dd))
                return interfaceDecl;

            dd = dd->parentDecl;
        }
        return nullptr;
    }

    Substitutions* specializeSubstitutionsShallow(
        ASTBuilder*             astBuilder, 
        Substitutions*   substToSpecialize,
        Substitutions*   substsToApply,
        Substitutions*   restSubst,
        int*                    ioDiff)
    {
        SLANG_ASSERT(substToSpecialize);
        return substToSpecialize->applySubstitutionsShallow(astBuilder, substsToApply, restSubst, ioDiff);
    }

    // Construct new substitutions to apply to a declaration,
    // based on a provided substitution set to be applied
    Substitutions* specializeSubstitutions(
        ASTBuilder*             astBuilder,
        Decl*                   declToSpecialize,
        Substitutions*   substsToSpecialize,
        Substitutions*   substsToApply,
        int*                    ioDiff)
    {
        // No declaration? Then nothing to specialize.
        if(!declToSpecialize)
            return nullptr;

        // No (remaining) substitutions to apply? Then we are done.
        if(!substsToApply)
            return substsToSpecialize;

        // Walk the hierarchy of the declaration to determine what specializations might apply.
        // We assume that the `substsToSpecialize` must be aligned with the ancestor
        // hierarchy of `declToSpecialize` such that if, e.g., the `declToSpecialize` is
        // nested directly in a generic, then `substToSpecialize` will either start with
        // the corresponding `GenericSubstitution` or there will be *no* generic substitutions
        // corresponding to that decl.
        for(Decl* ancestorDecl = declToSpecialize; ancestorDecl; ancestorDecl = ancestorDecl->parentDecl)
        {
            if(auto ancestorGenericDecl = as<GenericDecl>(ancestorDecl))
            {
                // The declaration is nested inside a generic.
                // Does it already have a specialization for that generic?
                if(auto specGenericSubst = as<GenericSubstitution>(substsToSpecialize))
                {
                    if(specGenericSubst->genericDecl == ancestorGenericDecl)
                    {
                        // Yes. We have an existing specialization, so we will
                        // keep one matching it in place.
                        int diff = 0;
                        auto restSubst = specializeSubstitutions(
                            astBuilder,
                            ancestorGenericDecl->parentDecl,
                            specGenericSubst->outer,
                            substsToApply,
                            &diff);

                        auto firstSubst = specializeSubstitutionsShallow(
                            astBuilder,
                            specGenericSubst,
                            substsToApply,
                            restSubst,
                            &diff);

                        *ioDiff += diff;
                        return firstSubst;
                    }
                }

                // If the declaration is not already specialized
                // for the given generic, then see if we are trying
                // to *apply* such specializations to it.
                //
                // TODO: The way we handle things right now with
                // "default" specializations, this case shouldn't
                // actually come up.
                //
                for(auto s = substsToApply; s; s = s->outer)
                {
                    auto appGenericSubst = as<GenericSubstitution>(s);
                    if(!appGenericSubst)
                        continue;

                    if(appGenericSubst->genericDecl != ancestorGenericDecl)
                        continue;

                    // The substitutions we are applying are trying
                    // to specialize this generic, but we don't already
                    // have a generic substitution in place.
                    // We will need to create one.

                    int diff = 0;
                    auto restSubst = specializeSubstitutions(
                        astBuilder,
                        ancestorGenericDecl->parentDecl,
                        substsToSpecialize,
                        substsToApply,
                        &diff);

                    GenericSubstitution* firstSubst = astBuilder->create<GenericSubstitution>();
                    firstSubst->genericDecl = ancestorGenericDecl;
                    firstSubst->args = appGenericSubst->args;
                    firstSubst->outer = restSubst;

                    (*ioDiff)++;
                    return firstSubst;
                }
            }
            else if(auto ancestorInterfaceDecl = as<InterfaceDecl>(ancestorDecl))
            {
                // The task is basically the same as for the generic case:
                // We want to see if there is any existing substitution that
                // applies to this declaration, and use that if possible.

                // The declaration is nested inside a generic.
                // Does it already have a specialization for that generic?
                if(auto specThisTypeSubst = as<ThisTypeSubstitution>(substsToSpecialize))
                {
                    if(specThisTypeSubst->interfaceDecl == ancestorInterfaceDecl)
                    {
                        // Yes. We have an existing specialization, so we will
                        // keep one matching it in place.
                        int diff = 0;
                        auto restSubst = specializeSubstitutions(
                            astBuilder,
                            ancestorInterfaceDecl->parentDecl,
                            specThisTypeSubst->outer,
                            substsToApply,
                            &diff);

                        auto firstSubst = specializeSubstitutionsShallow(
                            astBuilder,
                            specThisTypeSubst,
                            substsToApply,
                            restSubst,
                            &diff);

                        *ioDiff += diff;
                        return firstSubst;
                    }
                }

                // Otherwise, check if we are trying to apply
                // a this-type substitution to the given interface
                //
                for(auto s = substsToApply; s; s = s->outer)
                {
                    auto appThisTypeSubst = as<ThisTypeSubstitution>(s);
                    if(!appThisTypeSubst)
                        continue;

                    if(appThisTypeSubst->interfaceDecl != ancestorInterfaceDecl)
                        continue;

                    int diff = 0;
                    auto restSubst = specializeSubstitutions(
                        astBuilder,
                        ancestorInterfaceDecl->parentDecl,
                        substsToSpecialize,
                        substsToApply,
                        &diff);

                    ThisTypeSubstitution* firstSubst = astBuilder->create<ThisTypeSubstitution>();
                    firstSubst->interfaceDecl = ancestorInterfaceDecl;
                    firstSubst->witness = appThisTypeSubst->witness;
                    firstSubst->outer = restSubst;

                    (*ioDiff)++;
                    return firstSubst;
                }
            }
        }

        // If we reach here then we've walked the full hierarchy up from
        // `declToSpecialize` and either didn't run into an generic/interface
        // declarations, or we didn't find any attempt to specialize them
        // in either substitution.
        //
        // As an invariant, there should *not* be any generic or this-type
        // substitutions in `substToSpecialize`, because otherwise they
        // would be specializations that don't actually apply to the given
        // declaration.
        //
        // Note: this does *not* mean that `substsToApply` doesn't have
        // any generic or this-type substitutions; it just means that none
        // of them were applicable.
        //
        return nullptr;
    }

    DeclRefBase DeclRefBase::substituteImpl(ASTBuilder* astBuilder, SubstitutionSet substSet, int* ioDiff) const
    {
        // Nothing to do when we have no declaration.
        if(!decl)
            return *this;

        // Apply the given substitutions to any specializations
        // that have already been applied to this declaration.
        int diff = 0;

        auto substSubst = specializeSubstitutions(
            astBuilder,
            decl,
            substitutions.substitutions,
            substSet.substitutions,
            &diff);

        if (!diff)
            return *this;

        *ioDiff += diff;

        DeclRefBase substDeclRef;
        substDeclRef.decl = decl;
        substDeclRef.substitutions = substSubst;

        // TODO: The old code here used to try to translate a decl-ref
        // to an associated type in a decl-ref for the concrete type
        // in a particular implementation.
        //
        // I have only kept that logic in `DeclRefType::SubstituteImpl`,
        // but it may turn out it is needed here too.

        return substDeclRef;
    }


    // Check if this is an equivalent declaration reference to another
    bool DeclRefBase::equals(DeclRefBase const& declRef) const
    {
        if (decl != declRef.decl)
            return false;
        if (!substitutions.equals(declRef.substitutions))
            return false;

        return true;
    }

    // Convenience accessors for common properties of declarations
    Name* DeclRefBase::getName() const
    {
        return decl->nameAndLoc.name;
    }
    SourceLoc DeclRefBase::getNameLoc() const
    {
        return decl->nameAndLoc.loc;
    }
    SourceLoc DeclRefBase::getLoc() const
    {
        return decl->loc;
    }

    DeclRefBase DeclRefBase::getParent() const
    {
        // Want access to the free function (the 'as' method by default gets priority)
        // Can access as method with this->as because it removes any ambiguity.
        using Slang::as;

        auto parentDecl = decl->parentDecl;
        if (!parentDecl)
            return DeclRefBase();

        // Default is to apply the same set of substitutions/specializations
        // to the parent declaration as were applied to the child.
        Substitutions* substToApply = substitutions.substitutions;

        if(auto interfaceDecl = as<InterfaceDecl>(decl))
        {
            // The declaration being referenced is an `interface` declaration,
            // and there might be a this-type substitution in place.
            // A reference to the parent of the interface declaration
            // should not include that substitution.
            if(auto thisTypeSubst = as<ThisTypeSubstitution>(substToApply))
            {
                if(thisTypeSubst->interfaceDecl == interfaceDecl)
                {
                    // Strip away that specializations that apply to the interface.
                    substToApply = thisTypeSubst->outer;
                }
            }
        }

        if (auto parentGenericDecl = as<GenericDecl>(parentDecl))
        {
            // The parent of this declaration is a generic, which means
            // that the decl-ref to the current declaration might include
            // substitutions that specialize the generic parameters.
            // A decl-ref to the parent generic should *not* include
            // those substitutions.
            //
            if(auto genericSubst = as<GenericSubstitution>(substToApply))
            {
                if(genericSubst->genericDecl == parentGenericDecl)
                {
                    // Strip away the specializations that were applied to the parent.
                    substToApply = genericSubst->outer;
                }
            }
        }

        return DeclRefBase(parentDecl, substToApply);
    }

    HashCode DeclRefBase::getHashCode() const
    {
        return combineHash(PointerHash<1>::getHashCode(decl), substitutions.getHashCode());
    }

    // IntVal

    IntegerLiteralValue getIntVal(IntVal* val)
    {
        if (auto constantVal = as<ConstantIntVal>(val))
        {
            return constantVal->value;
        }
        SLANG_UNEXPECTED("needed a known integer value");
        //return 0;
    }

    //

    // HLSLPatchType

    Type* HLSLPatchType::getElementType()
    {
        return as<Type>(findInnerMostGenericSubstitution(declRef.substitutions)->args[0]);
    }

    IntVal* HLSLPatchType::getElementCount()
    {
        return as<IntVal>(findInnerMostGenericSubstitution(declRef.substitutions)->args[1]);
    }

    // Constructors for types

    ArrayExpressionType* getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType,
        IntVal*         elementCount)
    {
        auto arrayType = astBuilder->create<ArrayExpressionType>();
        arrayType->baseType = elementType;
        arrayType->arrayLength = elementCount;
        return arrayType;
    }

    ArrayExpressionType* getArrayType(
        ASTBuilder* astBuilder,
        Type* elementType)
    {
        auto arrayType = astBuilder->create<ArrayExpressionType>();
        arrayType->baseType = elementType;
        return arrayType;
    }

    NamedExpressionType* getNamedType(
        ASTBuilder*                 astBuilder,
        DeclRef<TypeDefDecl> const& declRef)
    {
        DeclRef<TypeDefDecl> specializedDeclRef = createDefaultSubstitutionsIfNeeded(astBuilder, declRef).as<TypeDefDecl>();

        return astBuilder->create<NamedExpressionType>(specializedDeclRef);
    }

    
    FuncType* getFuncType(
        ASTBuilder*                     astBuilder,
        DeclRef<CallableDecl> const&    declRef)
    {
        FuncType* funcType = astBuilder->create<FuncType>();

        funcType->resultType = getResultType(astBuilder, declRef);
        funcType->errorType = getErrorCodeType(astBuilder, declRef);
        for (auto paramDeclRef : getParameters(declRef))
        {
            auto paramDecl = paramDeclRef.getDecl();
            auto paramType = getType(astBuilder, paramDeclRef);
            if( paramDecl->findModifier<RefModifier>() )
            {
                paramType = astBuilder->getRefType(paramType);
            }
            else if( paramDecl->findModifier<OutModifier>() )
            {
                if(paramDecl->findModifier<InOutModifier>() || paramDecl->findModifier<InModifier>())
                {
                    paramType = astBuilder->getInOutType(paramType);
                }
                else
                {
                    paramType = astBuilder->getOutType(paramType);
                }
            }
            funcType->paramTypes.add(paramType);
        }

        return funcType;
    }

    GenericDeclRefType* getGenericDeclRefType(
        ASTBuilder*                 astBuilder,
        DeclRef<GenericDecl> const& declRef)
    {
        return astBuilder->create<GenericDeclRefType>(declRef);
    }

    NamespaceType* getNamespaceType(
        ASTBuilder*                         astBuilder,
        DeclRef<NamespaceDeclBase> const&   declRef)
    {
        auto type = astBuilder->create<NamespaceType>();
        type->declRef = declRef;
        return type;
    }

    SamplerStateType* getSamplerStateType(
        ASTBuilder*     astBuilder)
    {
        return astBuilder->create<SamplerStateType>();
    }

    ThisTypeSubstitution* findThisTypeSubstitution(
        Substitutions*  substs,
        InterfaceDecl*  interfaceDecl)
    {
        for(Substitutions* s = substs; s; s = s->outer)
        {
            auto thisTypeSubst = as<ThisTypeSubstitution>(s);
            if(!thisTypeSubst)
                continue;

            if(thisTypeSubst->interfaceDecl != interfaceDecl)
                continue;

            return thisTypeSubst;
        }

        return nullptr;
    }

    //

    String DeclRefBase::toString() const
    {
        StringBuilder builder;
        toText(builder);
        return builder;
    }

    // Prints a partially qualified type name with generic substitutions.
    static void _printNestedDecl(const Substitutions* substitutions, Decl* decl, StringBuilder& out)
    {
        // If there is a parent scope for the declaration, print it first.
        // Exclude top-level namespaces like `tu0` or `core`.
        if (decl->parentDecl && !Slang::as<ModuleDecl>(decl->parentDecl))
        {
            auto parentGeneric = Slang::as<GenericDecl>(decl->parentDecl);

            // Exclude function or operator names.
            // Avoids excessively verbose messages like `func<T>(func::T)`
            if (!(parentGeneric && Slang::as<CallableDecl>(parentGeneric->inner)))
            {
                _printNestedDecl(substitutions, decl->parentDecl, out);

                // If the parent is a generic for this type, skip *this* type.
                // Avoids duplicate types like `MyType<T>::MyType`
                if (parentGeneric && parentGeneric->inner == decl)
                    return;

                out << ".";
            }
        }
        
        // Print this type's name.
        auto name = decl->getName();
        if (name)
        {
            out << name->text;
        }

        // Look for generic substitutions on this type.
        for (const Substitutions* subst = substitutions; subst; subst = subst->outer)
        {
            auto genericSubstitution = Slang::as<GenericSubstitution>(subst);
            if (!genericSubstitution)
                continue;

            // If the substitution is for this type, print it.
            if (genericSubstitution->genericDecl == decl)
            {
                out << "<";
                bool isFirst = true;
                for (const auto& it : genericSubstitution->args)
                {
                    if (!isFirst)
                        out << ", ";
                    isFirst = false;
                    it->toText(out);
                }
                out << ">";

                break;
            }
        }
    }

    void DeclRefBase::toText(StringBuilder& out) const
    {
        if (decl)
        {
            _printNestedDecl(substitutions, decl, out);
        }
    }

    bool SubstitutionSet::equals(const SubstitutionSet& substSet) const
    {
        if (substitutions == substSet.substitutions)
        {
            return true;
        }
        if (substitutions == nullptr || substSet.substitutions == nullptr)
        {
            return false;
        }
        return substitutions->equals(substSet.substitutions);
    }

    HashCode SubstitutionSet::getHashCode() const
    {
        HashCode rs = 0;
        if (substitutions)
            rs = combineHash(rs, substitutions->getHashCode());
        return rs;
    }


ModuleDecl* getModuleDecl(Decl* decl)
{
    for( auto dd = decl; dd; dd = dd->parentDecl )
    {
        if(auto moduleDecl = as<ModuleDecl>(dd))
            return moduleDecl;
    }
    return nullptr;
}

Module* getModule(Decl* decl)
{
    auto moduleDecl = getModuleDecl(decl);
    if(!moduleDecl)
        return nullptr;

    return moduleDecl->module;
}

static const ImageFormatInfo kImageFormatInfos[] =
{
#define SLANG_IMAGE_FORMAT_INFO(TYPE, COUNT, SIZE) SLANG_SCALAR_TYPE_##TYPE, uint8_t(COUNT), uint8_t(SIZE)
#define FORMAT(NAME, OTHER) \
    { SLANG_IMAGE_FORMAT_INFO OTHER, UnownedStringSlice::fromLiteral(#NAME) },
#include "slang-image-format-defs.h"
#undef FORMAT
#undef SLANG_IMAGE_FORMAT_INFO
};

bool findImageFormatByName(char const* inName, ImageFormat* outFormat)
{
    const UnownedStringSlice name(inName);

    for (Index i = 0; i < SLANG_COUNT_OF(kImageFormatInfos); ++i)
    {
        const auto& info = kImageFormatInfos[i];
        if (info.name == name)
        {
            *outFormat = ImageFormat(i);
            return true;
        }
    }
    return false;
}

char const* getGLSLNameForImageFormat(ImageFormat format)
{
    return kImageFormatInfos[Index(format)].name.begin();
}


const ImageFormatInfo& getImageFormatInfo(ImageFormat format)
{
    return kImageFormatInfos[Index(format)];
}

char const* getTryClauseTypeName(TryClauseType c)
{
    switch (c)
    {
    case TryClauseType::None:
        return "None";
    case TryClauseType::Standard:
        return "Standard";
    case TryClauseType::Optional:
        return "Optional";
    case TryClauseType::Assert:
        return "Assert";
    default:
        return "Unknown";
    }
}

} // namespace Slang

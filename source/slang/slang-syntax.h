#ifndef SLANG_SYNTAX_H
#define SLANG_SYNTAX_H

#include "../core/slang-basic.h"
#include "slang-lexer.h"
#include "slang-profile.h"
#include "slang-type-system-shared.h"
#include "../../slang.h"

#include "../core/slang-semantic-version.h"

#include "slang-name.h"

#include <assert.h>

namespace Slang
{
    class Module;
    class Name;
    class Session;
    class Substitutions;
    class SyntaxVisitor;
    class FuncDecl;
    class Layout;

    struct IExprVisitor;
    struct IDeclVisitor;
    struct IModifierVisitor;
    struct IStmtVisitor;
    struct ITypeVisitor;
    struct IValVisitor;

    class Parser;
    class SyntaxNode;

    typedef RefPtr<RefObject> (*SyntaxParseCallback)(Parser* parser, void* userData);

    typedef unsigned int ConversionCost;
    enum : ConversionCost
    {
        // No conversion at all
        kConversionCost_None = 0,

        // Conversion from a buffer to the type it carries needs to add a minimal
        // extra cost, just so we can distinguish an overload on `ConstantBuffer<Foo>`
        // from one on `Foo`
        kConversionCost_ImplicitDereference = 10,

        // Conversions based on explicit sub-typing relationships are the cheapest
        //
        // TODO(tfoley): We will eventually need a discipline for ranking
        // when two up-casts are comparable.
        kConversionCost_CastToInterface = 50,

        // Conversion that is lossless and keeps the "kind" of the value the same
        kConversionCost_RankPromotion = 150,

        // Conversions that are lossless, but change "kind"
        kConversionCost_UnsignedToSignedPromotion = 200,

        // Conversion from signed->unsigned integer of same or greater size
        kConversionCost_SignedToUnsignedConversion = 300,

        // Cost of converting an integer to a floating-point type
        kConversionCost_IntegerToFloatConversion = 400,

        // Default case (usable for user-defined conversions)
        kConversionCost_Default = 500,

        // Catch-all for conversions that should be discouraged
        // (i.e., that really shouldn't be made implicitly)
        //
        // TODO: make these conversions not be allowed implicitly in "Slang mode"
        kConversionCost_GeneralConversion = 900,

        // This is the cost of an explicit conversion, which should
        // not actually be performed.
        kConversionCost_Explicit = 90000,

        // Additional conversion cost to add when promoting from a scalar to
        // a vector (this will be added to the cost, if any, of converting
        // the element type of the vector)
        kConversionCost_ScalarToVector = 1,

        // Conversion is impossible
        kConversionCost_Impossible = 0xFFFFFFFF,
    };

    enum class ImageFormat
    {
#define FORMAT(NAME) NAME,
#include "slang-image-format-defs.h"
    };

    bool findImageFormatByName(char const* name, ImageFormat* outFormat);
    char const* getGLSLNameForImageFormat(ImageFormat format);

    // TODO(tfoley): We should ditch this enumeration
    // and just use the IR opcodes that represent these
    // types directly. The one major complication there
    // is that the order of the enum values currently
    // matters, since it determines promotion rank.
    // We either need to keep that restriction, or
    // look up promotion rank by some other means.
    //

    class Decl;
    class Val;

    // Forward-declare all syntax classes
#define SYNTAX_CLASS(NAME, BASE, ...) class NAME;
#include "slang-object-meta-begin.h"
#include "slang-syntax-defs.h"
#include "slang-object-meta-end.h"

    // Helper type for pairing up a name and the location where it appeared
    struct NameLoc
    {
        Name*       name;
        SourceLoc   loc;

        NameLoc()
            : name(nullptr)
        {}

        explicit NameLoc(Name* inName)
            : name(inName)
        {}


        NameLoc(Name* inName, SourceLoc inLoc)
            : name(inName)
            , loc(inLoc)
        {}

        NameLoc(Token const& token)
            : name(token.getNameOrNull())
            , loc(token.getLoc())
        {}
    };

    struct StringSliceLoc
    {
        UnownedStringSlice name;
        SourceLoc loc;

        StringSliceLoc()
            : name(nullptr)
        {}
        explicit StringSliceLoc(const UnownedStringSlice& inName)
            : name(inName)
        {}
        StringSliceLoc(const UnownedStringSlice& inName, SourceLoc inLoc)
            : name(inName)
            , loc(inLoc)
        {}
        StringSliceLoc(Token const& token)
            : loc(token.getLoc())
        {
            Name* tokenName = token.getNameOrNull();
            if (tokenName)
            {
                name = tokenName->text.getUnownedSlice();
            }
        }
    };

    // Helper class for iterating over a list of heap-allocated modifiers
    struct ModifierList
    {
        struct Iterator
        {
            Modifier* current;

            Modifier* operator*()
            {
                return current;
            }

            void operator++();
#if 0
            {
                current = current->next.Ptr();
            }
#endif

            bool operator!=(Iterator other)
            {
                return current != other.current;
            };

            Iterator()
                : current(nullptr)
            {}

            Iterator(Modifier* modifier)
                : current(modifier)
            {}
        };

        ModifierList()
            : modifiers(nullptr)
        {}

        ModifierList(Modifier* modifiers)
            : modifiers(modifiers)
        {}

        Iterator begin() { return Iterator(modifiers); }
        Iterator end() { return Iterator(nullptr); }

        Modifier* modifiers;
    };

    // Helper class for iterating over heap-allocated modifiers
    // of a specific type.
    template<typename T>
    struct FilteredModifierList
    {
        struct Iterator
        {
            Modifier* current;

            T* operator*()
            {
                return (T*)current;
            }

            void operator++();
            #if 0
            {
                current = Adjust(current->next.Ptr());
            }
            #endif

            bool operator!=(Iterator other)
            {
                return current != other.current;
            };

            Iterator()
                : current(nullptr)
            {}

            Iterator(Modifier* modifier)
                : current(modifier)
            {}
        };

        FilteredModifierList()
            : modifiers(nullptr)
        {}

        FilteredModifierList(Modifier* modifiers)
            : modifiers(Adjust(modifiers))
        {}

        Iterator begin() { return Iterator(modifiers); }
        Iterator end() { return Iterator(nullptr); }

        static Modifier* Adjust(Modifier* modifier);
        #if 0
        {
            Modifier* m = modifier;
            for (;;)
            {
                if (!m) return m;
                if (dynamicCast<T>(m)) return m;
                m = m->next.Ptr();
            }
        }
        #endif

        Modifier* modifiers;
    };

    // A set of modifiers attached to a syntax node
    struct Modifiers
    {
        // The first modifier in the linked list of heap-allocated modifiers
        RefPtr<Modifier> first;

        template<typename T>
        FilteredModifierList<T> getModifiersOfType() { return FilteredModifierList<T>(first.Ptr()); }

        // Find the first modifier of a given type, or return `nullptr` if none is found.
        template<typename T>
        T* findModifier()
        {
            return *getModifiersOfType<T>().begin();
        }

        template<typename T>
        bool hasModifier() { return findModifier<T>() != nullptr; }

        FilteredModifierList<Modifier>::Iterator begin() { return FilteredModifierList<Modifier>::Iterator(first.Ptr()); }
        FilteredModifierList<Modifier>::Iterator end() { return FilteredModifierList<Modifier>::Iterator(nullptr); }
    };

    class NamedExpressionType;
    class GenericDecl;
    class ContainerDecl;

    // Try to extract a simple integer value from an `IntVal`.
    // This fill assert-fail if the object doesn't represent a literal value.
    IntegerLiteralValue GetIntVal(RefPtr<IntVal> val);

        /// Represents how much checking has been applied to a declaration.
    enum class DeclCheckState : uint8_t
    {
            /// The declaration has been parsed, but
            /// is otherwise completely unchecked.
            ///
        Unchecked,

            /// Basic checks on the modifiers of the declaration have been applied.
            ///
            /// For example, when a declaration has attributes, the transformation
            /// of an attribute from the parsed-but-unchecked form into a checked
            /// form (in which it has the appropriate C++ subclass) happens here.
            ///
        ModifiersChecked,

            /// The type/signature of the declaration has been checked.
            ///
            /// For a value declaration like a variable or function, this means that
            /// the type of the declaration can be queried.
            ///
            /// For a type declaration like a `struct` or `typedef` this means
            /// that a `Type` referring to that declaration can be formed.
            ///
        SignatureChecked,

            /// The declaration's basic signature has been checked to the point that
            /// it is ready to be referenced in other places.
            ///
            /// For a function, this means that it has been organized into a
            /// "redeclration group" if there are multiple functions with the
            /// same name in a scope.
            ///
        ReadyForReference,

            /// The declaration is ready for lookup operations to be performed.
            ///
            /// For type declarations (e.g., aggregate types, generic type parameters)
            /// this means that any base type or constraint clauses have been
            /// sufficiently checked so that we can enumerate the inheritance
            /// hierarchy of the type and discover all its members.
            ///
        ReadyForLookup,

            /// Any conformance declared on the declaration have been validated.
            ///
            /// In particular, this step means that a "witness table" has been
            /// created to show  how a type satisfies the requirements of any
            /// interfaces it conforms to.
            ///
        ReadyForConformances,

            /// The declaration is fully checked.
            ///
            /// This step includes any validation of the declaration that is
            /// immaterial to clients code using the declaration, but that is
            /// nonetheless relevant to checking correctness.
            ///
            /// The canonical example here is checking the body of functions.
            /// Client code cannot depend on *how* a function is implemented,
            /// but we still need to (eventually) check the bodies of all
            /// functions, so it belongs in the last phase of checking.
            ///
        Checked,

        // For convenience at sites that call `ensureDecl()`, we define
        // some aliases for the above states that are expressed in terms
        // of what client code needs to be able to do with a declaration.
        //
        // These aliases can be changed over time if we decide to add
        // more phases to semantic checking.

        CanEnumerateBases = ReadyForLookup,
        CanUseBaseOfInheritanceDecl = ReadyForLookup,
        CanUseTypeOfValueDecl = ReadyForReference,
        CanUseExtensionTargetType = ReadyForLookup,
        CanUseAsType = ReadyForReference,
        CanUseFuncSignature = ReadyForReference,
        CanSpecializeGeneric = ReadyForReference,
        CanReadInterfaceRequirements = ReadyForLookup,
    };

        /// A `DeclCheckState` plus a bit to track whether a declaration is currently being checked.
    struct DeclCheckStateExt
    {
    public:
        DeclCheckStateExt() {}
        DeclCheckStateExt(DeclCheckState state)
            : m_raw(uint8_t(state))
        {}

        enum : uint8_t
        {
                /// A flag to indicate that a declaration is being checked.
                ///
                /// The value of this flag is chosen so that it can be
                /// represented in the bits of a `DeclCheckState` without
                /// colliding with the bits that represent actual states.
                ///
            kBeingCheckedBit = 0x80,
        };

        DeclCheckState getState() const { return DeclCheckState(m_raw & ~kBeingCheckedBit); }
        void setState(DeclCheckState state)
        {
            m_raw = (m_raw & kBeingCheckedBit) | uint8_t(state);
        }

        bool isBeingChecked() const { return (m_raw & kBeingCheckedBit) != 0; }

        void setIsBeingChecked(bool isBeingChecked)
        {
            m_raw = (m_raw & ~kBeingCheckedBit)
                | (isBeingChecked ? kBeingCheckedBit : 0);
        }

        bool operator>=(DeclCheckState state) const
        {
            return getState() >= state;
        }

    private:
        uint8_t m_raw = 0;
    };

    void addModifier(
        RefPtr<ModifiableSyntaxNode>    syntax,
        RefPtr<Modifier>                modifier);

    struct QualType
    {
        RefPtr<Type>	type;
        bool	        IsLeftValue;

        QualType()
            : IsLeftValue(false)
        {}

        QualType(Type* type)
            : type(type)
            , IsLeftValue(false)
        {}

        Type* Ptr() { return type.Ptr(); }

        operator Type*() { return type; }
        operator RefPtr<Type>() { return type; }
        RefPtr<Type> operator->() { return type; }
    };

    // A reference to a class of syntax node, that can be
    // used to create instances on the fly
    struct SyntaxClassBase
    {
        typedef void* (*CreateFunc)();

        // Run-time type representation for syntax nodes
        struct ClassInfo
        {
            typedef ClassInfo ThisType;

                /// A constant time implementation of isSubClassOf
            SLANG_FORCE_INLINE bool isSubClassOf(const ThisType& super) const
            {
                // We include super.m_classId, because it's a subclass of itself.
                return m_classId >= super.m_classId && m_classId < super.m_childrenEndClassId;
            }
                /// Will produce the same result as isSubClassOf, but more slowly by traversing the m_superClass
                /// Works without initRange being called. 
            bool isSubClassOfSlow(const ThisType& super) const;

                /// This function must have been called before any dynamic casting will work (via the m_classId/m_childrenEndClassId.
                /// It sets up the m_rangeStart/m_rangeEnd values to make.
                /// Is called within the creation of Session
            static SlangResult initRanges();

                /// Ctor
            ClassInfo(const char* name, CreateFunc createFunc, const ClassInfo* superClass);

                /// The id for this class. The children of this class are in the range of m_classId + 1 to (but not including) m_childrenEndId.
            mutable uint32_t m_classId;
                /// Non inclusive end of range of children.
            mutable uint32_t m_childrenEndClassId;        

            const ClassInfo* m_superClass;          ///< The super class of this class, or nullptr if has no super class. 
            const char* m_name;                     ///< Textual class name, for debugging 
            CreateFunc m_createFunc;                ///< Callback to use when creating instances
            
            ClassInfo* m_next;                      ///< Next in list starting from s_first
            static ClassInfo* s_first;
        };

        SyntaxClassBase()
        {}

        SyntaxClassBase(ClassInfo const* classInfoIn)
            : classInfo(classInfoIn)
        {}

        void* createInstanceImpl() const
        {
            auto ci = classInfo;
            if (!ci) return nullptr;

            auto cf = ci->m_createFunc;
            if (!cf) return nullptr;

            return cf();
        }

        SLANG_FORCE_INLINE bool isSubClassOfImpl(SyntaxClassBase const& super) const { return classInfo->isSubClassOf(*super.classInfo); }

        ClassInfo const* classInfo = nullptr;

        template<typename T>
        struct Impl
        {
            static void* createFunc();
            static const ClassInfo kClassInfo;
        };
    };

    template<typename T>
    struct SyntaxClass : SyntaxClassBase
    {
        SyntaxClass()
        {}

        template <typename U>
        SyntaxClass(SyntaxClass<U> const& other,
            typename EnableIf<IsConvertible<T*, U*>::Value, void>::type* = 0)
            : SyntaxClassBase(other.classInfo)
        {
        }

        T* createInstance() const
        {
            return (T*)createInstanceImpl();
        }

        SyntaxClass(const ClassInfo* classInfoIn):
            SyntaxClassBase(classInfoIn) 
        {}

        static SyntaxClass<T> getClass()
        {
            return SyntaxClass<T>(&SyntaxClassBase::Impl<T>::kClassInfo);
        }

        template<typename U>
        bool isSubClassOf(SyntaxClass<U> super)
        {
            return isSubClassOfImpl(super);
        }

        template<typename U>
        bool isSubClassOf()
        {
            return isSubClassOf(SyntaxClass<U>::getClass());
        }
    };

    template<typename T>
    SyntaxClass<T> getClass()
    {
        return SyntaxClass<T>::getClass();
    }

    template<typename T>
    SLANG_FORCE_INLINE T* dynamicCast(NodeBase* node);

    template<typename T>
    SLANG_FORCE_INLINE const T* dynamicCast(const NodeBase* node);

    template<typename T>
    SLANG_FORCE_INLINE T* as(NodeBase* node);

    template<typename T>
    SLANG_FORCE_INLINE const T* as(const NodeBase* node);
    
    struct SubstitutionSet
    {
        RefPtr<Substitutions> substitutions;
        operator Substitutions*() const
        {
            return substitutions;
        }

        SubstitutionSet() {}
        SubstitutionSet(RefPtr<Substitutions> subst)
            : substitutions(subst)
        {
        }
        bool Equals(const SubstitutionSet& substSet) const;
        int GetHashCode() const;
    };

    template<typename T>
    struct DeclRef;

    // A reference to a declaration, which may include
    // substitutions for generic parameters.
    struct DeclRefBase
    {
        typedef Decl DeclType;

        // The underlying declaration
        Decl* decl = nullptr;
        Decl* getDecl() const { return decl; }

        // Optionally, a chain of substitutions to perform
        SubstitutionSet substitutions;

        DeclRefBase()
        {}
        
        DeclRefBase(Decl* decl)
            :decl(decl)
        {}

        DeclRefBase(Decl* decl, SubstitutionSet subst)
            :decl(decl),
            substitutions(subst)
        {}

        DeclRefBase(Decl* decl, RefPtr<Substitutions> subst)
            : decl(decl)
            , substitutions(subst)
        {}

        // Apply substitutions to a type or declaration
        RefPtr<Type> Substitute(RefPtr<Type> type) const;

        DeclRefBase Substitute(DeclRefBase declRef) const;

        // Apply substitutions to an expression
        RefPtr<Expr> Substitute(RefPtr<Expr> expr) const;

        // Apply substitutions to this declaration reference
        DeclRefBase SubstituteImpl(SubstitutionSet subst, int* ioDiff);

        // Returns true if 'as' will return a valid cast
        template <typename T>
        bool is() const { return Slang::as<T>(decl) != nullptr; }

        // "dynamic cast" to a more specific declaration reference type
        template<typename T>
        DeclRef<T> as() const;

        // Check if this is an equivalent declaration reference to another
        bool Equals(DeclRefBase const& declRef) const;
        bool operator == (const DeclRefBase& other) const
        {
            return Equals(other);
        }

        // Convenience accessors for common properties of declarations
        Name* GetName() const;
        SourceLoc getLoc() const;
        DeclRefBase GetParent() const;

        int GetHashCode() const;

        // Debugging:
        String toString() const;
    };

    template<typename T>
    struct DeclRef : DeclRefBase
    {
        typedef T DeclType;

        DeclRef()
        {}
        
        DeclRef(T* decl, SubstitutionSet subst)
            : DeclRefBase(decl, subst)
        {}

        DeclRef(T* decl, RefPtr<Substitutions> subst)
            : DeclRefBase(decl, SubstitutionSet(subst))
        {}

        template <typename U>
        DeclRef(DeclRef<U> const& other,
            typename EnableIf<IsConvertible<T*, U*>::Value, void>::type* = 0)
            : DeclRefBase(other.decl, other.substitutions)
        {
        }

        T* getDecl() const
        {
            return (T*)decl;
        }

        operator T*() const
        {
            return getDecl();
        }

        //
        static DeclRef<T> unsafeInit(DeclRefBase const& declRef)
        {
            return DeclRef<T>((T*) declRef.decl, declRef.substitutions);
        }

        RefPtr<Type> Substitute(RefPtr<Type> type) const
        {
            return DeclRefBase::Substitute(type);
        }
        RefPtr<Expr> Substitute(RefPtr<Expr> expr) const
        {
            return DeclRefBase::Substitute(expr);
        }

        // Apply substitutions to a type or declaration
        template<typename U>
        DeclRef<U> Substitute(DeclRef<U> declRef) const
        {
            return DeclRef<U>::unsafeInit(DeclRefBase::Substitute(declRef));
        }

        // Apply substitutions to this declaration reference
        DeclRef<T> SubstituteImpl(SubstitutionSet subst, int* ioDiff)
        {
            return DeclRef<T>::unsafeInit(DeclRefBase::SubstituteImpl(subst, ioDiff));
        }

        DeclRef<ContainerDecl> GetParent() const
        {
            return DeclRef<ContainerDecl>::unsafeInit(DeclRefBase::GetParent());
        }
    };

    template<typename T>
    DeclRef<T> DeclRefBase::as() const
    {
        DeclRef<T> result;
        result.decl = Slang::as<T>(decl);
        result.substitutions = substitutions;
        return result;
    }

    template<typename T>
    inline DeclRef<T> makeDeclRef(T* decl)
    {
        return DeclRef<T>(decl, nullptr);
    }

    enum class MemberFilterStyle
    {
        All,                        ///< All members               
        Instance,                   ///< Only instance members
        Static,                     ///< Only static (ie non instance) members
    };

    const RefPtr<Decl>* adjustFilterCursorImpl(const SyntaxClassBase::ClassInfo& clsInfo, MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end);
    const RefPtr<Decl>* getFilterCursorByIndexImpl(const SyntaxClassBase::ClassInfo& clsInfo, MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end, Index index);
    Index getFilterCountImpl(const SyntaxClassBase::ClassInfo& clsInfo, MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end);


    template <typename T>
    const RefPtr<Decl>* adjustFilterCursor(MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end)
    {
        return adjustFilterCursorImpl(SyntaxClassBase::Impl<T>::kClassInfo, filterStyle, ptr, end);
    }

        /// Finds the element at index. If there is no element at the index (for example has too few elements), returns nullptr.
    template <typename T>
    const RefPtr<Decl>* getFilterCursorByIndex(MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end, Index index)
    {
        return getFilterCursorByIndexImpl(SyntaxClassBase::Impl<T>::kClassInfo, filterStyle, ptr, end, index);
    }
     
    template <typename T>
    Index getFilterCount(MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end)
    {
        return getFilterCountImpl(SyntaxClassBase::Impl<T>::kClassInfo, filterStyle, ptr, end);
    }
     
    template <typename T>
    bool isFilterNonEmpty(MemberFilterStyle filterStyle, const RefPtr<Decl>* ptr, const RefPtr<Decl>* end)
    {
        return adjustFilterCursorImpl(SyntaxClassBase::Impl<T>::kClassInfo, filterStyle, ptr, end) != end;
    }

    template<typename T>
    struct FilteredMemberList
    {
        typedef RefPtr<Decl> Element;

        FilteredMemberList()
            : m_begin(nullptr)
            , m_end(nullptr)
        {}

        explicit FilteredMemberList(
            List<Element> const& list,
            MemberFilterStyle filterStyle = MemberFilterStyle::All)
            : m_begin(adjustFilterCursor<T>(filterStyle, list.begin(), list.end()))
            , m_end(list.end())
            , m_filterStyle(filterStyle)
        {}

        struct Iterator
        {
            const Element* m_cursor;
            const Element* m_end;
            MemberFilterStyle m_filterStyle;

            bool operator!=(Iterator const& other) const { return m_cursor != other.m_cursor; }

            void operator++() { m_cursor = adjustFilterCursor<T>(m_filterStyle, m_cursor + 1, m_end); }

            const RefPtr<T>& operator*() { return  *(RefPtr<T>*)(m_cursor); }
        };

        Iterator begin()
        {
            Iterator iter = { m_begin, m_end, m_filterStyle };
            return iter;
        }

        Iterator end()
        {
            Iterator iter = { m_end, m_end, m_filterStyle };
            return iter;
        }

        // TODO(tfoley): It is ugly to have these.
        // We should probably fix the call sites instead.
        const RefPtr<T>& getFirst() { return *begin(); }
        Index getCount() { return getFilterCount<T>(m_filterStyle, m_begin, m_end); }

        RefPtr<T> operator[](Index index) const
        {
            const RefPtr<Decl>* ptr = getFilterCursorByIndex<T>(m_filterStyle, m_begin, m_end, index);
            SLANG_ASSERT(ptr);
            return  *(RefPtr<T>*)(ptr);
        }

            /// Returns true if empty (equivalent to getCount() == 0)
        bool isEmpty() const
        {
            /// Note we don't have to scan, because m_begin has already been adjusted, when the FilteredMemberList is constructed
            return m_begin == m_end;
        }
            /// Returns true if non empty (equivalent to getCount() != 0 but faster)
        bool isNonEmpty() const { return !isEmpty(); }

        List<RefPtr<T>> toArray()
        {
            List<RefPtr<T>> result;
            for (auto element : (*this))
            {
                result.add(element);
            }
            return result;
        }
        
        const Element* m_begin;             ///< Is either equal to m_end, or points to first *valid* filtered member
        const Element* m_end;
        MemberFilterStyle m_filterStyle;
    };

    struct TransparentMemberInfo
    {
        // The declaration of the transparent member
        Decl*	decl;
    };

    template<typename T>
    struct FilteredMemberRefList
    {
        List<RefPtr<Decl>> const&	m_decls;
        SubstitutionSet		m_substitutions;
        MemberFilterStyle   m_filterStyle;

        FilteredMemberRefList(
            List<RefPtr<Decl>> const&	decls,
            SubstitutionSet		substitutions,
            MemberFilterStyle   filterStyle = MemberFilterStyle::All)
            : m_decls(decls)
            , m_substitutions(substitutions)
            , m_filterStyle(filterStyle)
        {}

        Index getCount() const { return getFilterCount<T>(m_filterStyle, m_decls.begin(), m_decls.end()); }
    
            /// True if empty (equivalent to getCount == 0, but faster)
        bool isEmpty() const { return !isNonEmpty(); }
            /// True if non empty (equivalent to getCount() != 0 but faster)
        bool isNonEmpty() const { return isFilterNonEmpty<T>(m_filterStyle, m_decls.begin(), m_decls.end()); }

        DeclRef<T> operator[](Index index) const
        {
             const RefPtr<Decl>* decl = getFilterCursorByIndex<T>(m_filterStyle, m_decls.begin(), m_decls.end(), index);
             SLANG_ASSERT(decl);
             return DeclRef<T>((T*) decl->Ptr(), m_substitutions);
        }

        List<DeclRef<T>> toArray() const
        {
            List<DeclRef<T>> result;
            for (auto d : *this)
                result.add(d);
            return result;
        }

        struct Iterator
        {
            FilteredMemberRefList const* m_list;
            const RefPtr<Decl>* m_ptr;
            const RefPtr<Decl>* m_end;
            MemberFilterStyle m_filterStyle;

            Iterator() : m_list(nullptr), m_ptr(nullptr), m_filterStyle(MemberFilterStyle::All) {}
            Iterator(
                FilteredMemberRefList const* list,
                const RefPtr<Decl>* ptr,
                const RefPtr<Decl>* end,
                MemberFilterStyle filterStyle
                )
                : m_list(list)
                , m_ptr(ptr)
                , m_end(end)
                , m_filterStyle(filterStyle)
            {}

            bool operator!=(const Iterator& other) const  { return m_ptr != other.m_ptr; }

            void operator++() { m_ptr = adjustFilterCursor<T>(m_filterStyle, m_ptr + 1, m_end); }

            DeclRef<T> operator*() { return DeclRef<T>((T*) m_ptr->Ptr(), m_list->m_substitutions); }
        };

        Iterator begin() const { return Iterator(this, adjustFilterCursor<T>(m_filterStyle, m_decls.begin(), m_decls.end()), m_decls.end(), m_filterStyle); }
        Iterator end() const { return Iterator(this, m_decls.end(), m_decls.end(), m_filterStyle); }
    };

    //
    // type Expressions
    //

    // A "type expression" is a term that we expect to resolve to a type during checking.
    // We store both the original syntax and the resolved type here.
    struct TypeExp
    {
        TypeExp() {}
        TypeExp(TypeExp const& other)
            : exp(other.exp)
            , type(other.type)
        {}
        explicit TypeExp(RefPtr<Expr> exp)
            : exp(exp)
        {}
        explicit TypeExp(RefPtr<Type> type)
            : type(type)
        {}
        TypeExp(RefPtr<Expr> exp, RefPtr<Type> type)
            : exp(exp)
            , type(type)
        {}

        RefPtr<Expr> exp;
        RefPtr<Type> type;

        bool Equals(Type* other);
#if 0
        {
            return type->Equals(other);
        }
#endif
        bool Equals(RefPtr<Type> other);
#if 0
        {
            return type->Equals(other.Ptr());
        }
#endif
        Type* Ptr() { return type.Ptr(); }
        operator Type*()
        {
            return type;
        }
        Type* operator->() { return Ptr(); }

        TypeExp Accept(SyntaxVisitor* visitor);
    };



    struct Scope : public RefObject
    {
        // The parent of this scope (where lookup should go if nothing is found locally)
        RefPtr<Scope>           parent;

        // The next sibling of this scope (a peer for lookup)
        RefPtr<Scope>           nextSibling;

        // The container to use for lookup
        //
        // Note(tfoley): This is kept as an unowned pointer
        // so that a scope can't keep parts of the AST alive,
        // but the opposite it allowed.
        ContainerDecl*          containerDecl;
    };

    // Masks to be applied when lookup up declarations
    enum class LookupMask : uint8_t
    {
        type = 0x1,
        Function = 0x2,
        Value = 0x4,
        Attribute = 0x8,

        Default = type | Function | Value,
    };

    // Represents one item found during lookup
    struct LookupResultItem
    {
        // Sometimes lookup finds an item, but there were additional
        // "hops" taken to reach it. We need to remember these steps
        // so that if/when we consturct a full expression we generate
        // appropriate AST nodes for all the steps.
        //
        // We build up a list of these "breadcrumbs" while doing
        // lookup, and store them alongside each item found.
        //
        // As an example, suppose we have an HLSL `cbuffer` declaration:
        //
        //     cbuffer C { float4 f; }
        //
        // This is syntax sugar for a global-scope variable of
        // type `ConstantBuffer<T>` where `T` is a `struct` containing
        // all the members:
        //
        //     struct Anon0 { float4 f; };
        //     __transparent ConstantBuffer<Anon0> anon1;
        //
        // The `__transparent` modifier there captures the fact that
        // when somebody writes `f` in their code, they expect it to
        // "see through" the `cbuffer` declaration (or the global variable,
        // in this case) and find the member inside.
        //
        // But when the user writes `f` we can't just create a simple
        // `VarExpr` that refers directly to that field, because that
        // doesn't actually reflect the required steps in a way that
        // code generation can use.
        //
        // Instead we need to construct an expression like `(*anon1).f`,
        // where there is are two additional steps in the process:
        //
        // 1. We needed to dereference the pointer-like type `ConstantBuffer<Anon0>`
        //    to get at a value of type `Anon0`
        // 2. We needed to access a sub-field of the aggregate type `Anon0`
        //
        // We *could* just create these full-formed expressions during
        // lookup, but this might mean creating a large number of
        // AST nodes in cases where the user calls an overloaded function.
        // At the very least we'd rather not heap-allocate in the common
        // case where no "extra" steps need to be performed to get to
        // the declarations.
        //
        // This is where "breadcrumbs" come in. A breadcrumb represents
        // an extra "step" that must be performed to turn a declaration
        // found by lookup into a valid expression to splice into the
        // AST. Most of the time lookup result items don't have any
        // breadcrumbs, so that no extra heap allocation takes place.
        // When an item does have breadcrumbs, and it is chosen as
        // the unique result (perhaps by overload resolution), then
        // we can walk the list of breadcrumbs to create a full
        // expression.
        class Breadcrumb : public RefObject
        {
        public:
            enum class Kind : uint8_t
            {
                // The lookup process looked "through" an in-scope
                // declaration to the fields inside of it, so that
                // even if lookup started with a simple name `f`,
                // it needs to result in a member expression `obj.f`.
                Member,

                // The lookup process took a pointer(-like) value, and then
                // proceeded to derefence it and look at the thing(s)
                // it points to instead, so that the final expression
                // needs to have `(*obj)`
                Deref,

                // The lookup process saw a value `obj` of type `T` and
                // took into account an in-scope constraint that says
                // `T` is a subtype of some other type `U`, so that
                // lookup was able to find a member through type `U`
                // instead.
                Constraint,

                // The lookup process considered a member of an
                // enclosing type as being in scope, so that any
                // reference to that member needs to use a `this`
                // expression as appropriate.
                This,
            };

            // The kind of lookup step that was performed
            Kind kind;

            // For the `Kind::This` case, is the `this` parameter
            // mutable or not?
            enum class ThisParameterMode : uint8_t
            {
                Default,
                Mutating,
            };
            ThisParameterMode thisParameterMode = ThisParameterMode::Default;


            // As needed, a reference to the declaration that faciliated
            // the lookup step.
            //
            // For a `Member` lookup step, this is the declaration whose
            // members were implicitly pulled into scope.
            //
            // For a `Constraint` lookup step, this is the `ConstraintDecl`
            // that serves to witness the subtype relationship.
            //
            DeclRef<Decl> declRef;

            // The next implicit step that the lookup process took to
            // arrive at a final value.
            RefPtr<Breadcrumb> next;

            Breadcrumb(
                Kind                kind,
                DeclRef<Decl>       declRef,
                RefPtr<Breadcrumb>  next,
                ThisParameterMode   thisParameterMode = ThisParameterMode::Default)
                : kind(kind)
                , thisParameterMode(thisParameterMode)
                , declRef(declRef)
                , next(next)
            {}
        };

        // A properly-specialized reference to the declaration that was found.
        DeclRef<Decl> declRef;

        // Any breadcrumbs needed in order to turn that declaration
        // reference into a well-formed expression.
        //
        // This is unused in the simple case where a declaration
        // is being referenced directly (rather than through
        // transparent members).
        RefPtr<Breadcrumb> breadcrumbs;

        LookupResultItem() = default;
        explicit LookupResultItem(DeclRef<Decl> declRef)
            : declRef(declRef)
        {}
        LookupResultItem(DeclRef<Decl> declRef, RefPtr<Breadcrumb> breadcrumbs)
            : declRef(declRef)
            , breadcrumbs(breadcrumbs)
        {}
    };


    // Result of looking up a name in some lexical/semantic environment.
    // Can be used to enumerate all the declarations matching that name,
    // in the case where the result is overloaded.
    struct LookupResult
    {
        // The one item that was found, in the smple case
        LookupResultItem item;

        // All of the items that were found, in the complex case.
        // Note: if there was no overloading, then this list isn't
        // used at all, to avoid allocation.
        List<LookupResultItem> items;

        // Was at least one result found?
        bool isValid() const { return item.declRef.getDecl() != nullptr; }

        bool isOverloaded() const { return items.getCount() > 1; }

        Name* getName() const
        {
            return items.getCount() > 1 ? items[0].declRef.GetName() : item.declRef.GetName();
        }
        LookupResultItem* begin()
        {
            if (isValid())
            {
                if (isOverloaded())
                    return items.begin();
                else
                    return &item;
            }
            else
                return nullptr;
        }
        LookupResultItem* end()
        {
            if (isValid())
            {
                if (isOverloaded())
                    return items.end();
                else
                    return &item + 1;
            }
            else
                return nullptr;
        }
    };

    struct SemanticsVisitor;

    struct LookupRequest
    {
        SemanticsVisitor*   semantics   = nullptr;
        RefPtr<Scope>       scope       = nullptr;
        RefPtr<Scope>       endScope    = nullptr;

        LookupMask          mask        = LookupMask::Default;
    };

    struct WitnessTable;

    // A value that witnesses the satisfaction of an interface
    // requirement by a particular declaration or value.
    struct RequirementWitness
    {
        RequirementWitness()
            : m_flavor(Flavor::none)
        {}

        RequirementWitness(DeclRef<Decl> declRef)
            : m_flavor(Flavor::declRef)
            , m_declRef(declRef)
        {}

        RequirementWitness(RefPtr<Val> val);

        RequirementWitness(RefPtr<WitnessTable> witnessTable);

        enum class Flavor
        {
            none,
            declRef,
            val,
            witnessTable,
        };

        Flavor getFlavor()
        {
            return m_flavor;
        }

        DeclRef<Decl> getDeclRef()
        {
            SLANG_ASSERT(getFlavor() == Flavor::declRef);
            return m_declRef;
        }

        RefPtr<Val> getVal()
        {
            SLANG_ASSERT(getFlavor() == Flavor::val);
            return m_obj.as<Val>();
        }

        RefPtr<WitnessTable> getWitnessTable();

        RequirementWitness specialize(SubstitutionSet const& subst);

        Flavor              m_flavor;
        DeclRef<Decl>       m_declRef;
        RefPtr<RefObject>   m_obj;

    };

    typedef Dictionary<Decl*, RequirementWitness> RequirementDictionary;

    struct WitnessTable : RefObject
    {
        RequirementDictionary requirementDictionary;
    };

    typedef Dictionary<unsigned int, RefPtr<RefObject>> AttributeArgumentValueDict;

    struct SpecializationParam
    {
        enum class Flavor
        {
            GenericType,
            GenericValue,
            ExistentialType,
            ExistentialValue,
        };
        Flavor              flavor;
        SourceLoc           loc;
        RefPtr<NodeBase>    object;
    };
    typedef List<SpecializationParam> SpecializationParams;

    struct SpecializationArg
    {
        RefPtr<Val> val;
    };
    typedef List<SpecializationArg> SpecializationArgs;

    struct ExpandedSpecializationArg : SpecializationArg
    {
        RefPtr<Val> witness;
    };
    typedef List<ExpandedSpecializationArg> ExpandedSpecializationArgs;

    // Generate class definition for all syntax classes
#define SYNTAX_FIELD(TYPE, NAME) TYPE NAME;
#define FIELD(TYPE, NAME) TYPE NAME;
#define FIELD_INIT(TYPE, NAME, INIT) TYPE NAME = INIT;
#define RAW(...) __VA_ARGS__
#define END_SYNTAX_CLASS() };
#define SYNTAX_CLASS(NAME, BASE, ...) class NAME : public BASE {public:
#include "slang-object-meta-begin.h"

#include "slang-syntax-base-defs.h"
#undef SYNTAX_CLASS

#undef ABSTRACT_SYNTAX_CLASS
#define ABSTRACT_SYNTAX_CLASS(NAME, BASE, ...)                          \
    class NAME : public BASE {                                          \
    public: /* ... */
#define SYNTAX_CLASS(NAME, BASE, ...)                                   \
    class NAME : public BASE {                                          \
    virtual void accept(NAME::Visitor* visitor, void* extra) override;  \
    public: virtual const SyntaxClassBase::ClassInfo& getClassInfo() const override;          \
    public: /* ... */
#include "slang-expr-defs.h"
#include "slang-decl-defs.h"
#include "slang-modifier-defs.h"
#include "slang-stmt-defs.h"
#include "slang-type-defs.h"
#include "slang-val-defs.h"

#include "slang-object-meta-end.h"

    inline RefPtr<Type> GetSub(DeclRef<GenericTypeConstraintDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->sub.Ptr());
    }

    inline RefPtr<Type> GetSup(DeclRef<TypeConstraintDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->getSup().type);
    }

    // Note(tfoley): These logically belong to `Type`,
    // but order-of-declaration stuff makes that tricky
    //
    // TODO(tfoley): These should really belong to the compilation context!
    //
    void registerBuiltinDecl(
        Session*                    session,
        RefPtr<Decl>                decl,
        RefPtr<BuiltinTypeModifier> modifier);
    void registerMagicDecl(
        Session*                    session,
        RefPtr<Decl>                decl,
        RefPtr<MagicTypeModifier>   modifier);

    // Look up a magic declaration by its name
    RefPtr<Decl> findMagicDecl(
        Session*        session,
        String const&   name);

    // Create an instance of a syntax class by name
    SyntaxNodeBase* createInstanceOfSyntaxClassByName(
        String const&   name);

    // `Val`

    inline bool areValsEqual(Val* left, Val* right)
    {
        if(!left || !right) return left == right;
        return left->EqualsVal(right);
    }

    //

    inline BaseType GetVectorBaseType(VectorExpressionType* vecType)
    {
        auto basicExprType = as<BasicExpressionType>(vecType->elementType);
        return basicExprType->baseType;
    }

    inline int GetVectorSize(VectorExpressionType* vecType)
    {
        auto constantVal = as<ConstantIntVal>(vecType->elementCount);
        if (constantVal)
            return (int) constantVal->value;
        // TODO: what to do in this case?
        return 0;
    }

    //
    // Declarations
    //

    inline ExtensionDecl* GetCandidateExtensions(DeclRef<AggTypeDecl> const& declRef)
    {
        return declRef.getDecl()->candidateExtensions;
    }

    inline FilteredMemberRefList<Decl> getMembers(DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle)
    {
        return FilteredMemberRefList<Decl>(declRef.getDecl()->Members, declRef.substitutions, filterStyle);
    }

    template<typename T>
    inline FilteredMemberRefList<T> getMembersOfType( DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle)
    {
        return FilteredMemberRefList<T>(declRef.getDecl()->Members, declRef.substitutions, filterStyle);
    }

    template<typename T>
    inline List<DeclRef<T>> getMembersOfTypeWithExt(DeclRef<ContainerDecl> const& declRef, MemberFilterStyle filterStyle)
    {
        List<DeclRef<T>> rs;
        for (auto d : getMembersOfType<T>(declRef, filterStyle))
            rs.add(d);
        if (auto aggDeclRef = declRef.as<AggTypeDecl>())
        {
            for (auto ext = GetCandidateExtensions(aggDeclRef); ext; ext = ext->nextCandidateExtension)
            {
                auto extMembers = getMembersOfType<T>(DeclRef<ContainerDecl>(ext, declRef.substitutions), filterStyle);
                for (auto mbr : extMembers)
                    rs.add(mbr);
            }
        }
        return rs;
    }

        /// The the user-level name for a variable that might be a shader parameter.
        ///
        /// In most cases this is just the name of the variable declaration itself,
        /// but in the specific case of a `cbuffer`, the name that the user thinks
        /// of is really metadata. For example:
        ///
        ///     cbuffer C { int x; }
        ///
        /// In this example, error messages relating to the constant buffer should
        /// really use the name `C`, but that isn't the name of the declaration
        /// (it is in practice anonymous, and `C` can be used for a different
        /// declaration in the same file).
        ///
    Name* getReflectionName(VarDeclBase* varDecl);

    inline RefPtr<Type> GetType(DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Expr> getInitExpr(DeclRef<VarDeclBase> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->initExpr);
    }

    inline RefPtr<Type> getType(DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Expr> getTagExpr(DeclRef<EnumCaseDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->tagExpr);
    }

    inline RefPtr<Type> GetTargetType(DeclRef<ExtensionDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->targetType.Ptr());
    }
    
    inline FilteredMemberRefList<VarDecl> GetFields(DeclRef<StructDecl> const& declRef, MemberFilterStyle filterStyle)
    {
        return getMembersOfType<VarDecl>(declRef, filterStyle);
    }

    

    inline RefPtr<Type> getBaseType(DeclRef<InheritanceDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->base.type);
    }
    
    inline RefPtr<Type> GetType(DeclRef<TypeDefDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->type.Ptr());
    }

    inline RefPtr<Type> GetResultType(DeclRef<CallableDecl> const& declRef)
    {
        return declRef.Substitute(declRef.getDecl()->ReturnType.type.Ptr());
    }

    inline FilteredMemberRefList<ParamDecl> GetParameters(DeclRef<CallableDecl> const& declRef)
    {
        return getMembersOfType<ParamDecl>(declRef, MemberFilterStyle::All);
    }

    inline Decl* GetInner(DeclRef<GenericDecl> const& declRef)
    {
        // TODO: Should really return a `DeclRef<Decl>` for the inner
        // declaration, and not just a raw pointer
        return declRef.getDecl()->inner.Ptr();
    }


    //

    RefPtr<ArrayExpressionType> getArrayType(
        Type* elementType,
        IntVal*         elementCount);

    RefPtr<ArrayExpressionType> getArrayType(
        Type* elementType);

    RefPtr<NamedExpressionType> getNamedType(
        Session*                    session,
        DeclRef<TypeDefDecl> const& declRef);

    RefPtr<TypeType> getTypeType(
        Type* type);

    RefPtr<FuncType> getFuncType(
        Session*                        session,
        DeclRef<CallableDecl> const&    declRef);

    RefPtr<GenericDeclRefType> getGenericDeclRefType(
        Session*                    session,
        DeclRef<GenericDecl> const& declRef);

    RefPtr<NamespaceType> getNamespaceType(
        Session*                            session,
        DeclRef<NamespaceDeclBase> const&   declRef);

    RefPtr<SamplerStateType> getSamplerStateType(
        Session*        session);


    // Definitions that can't come earlier despite
    // being in templates, because gcc/clang get angry.
    //
    template<typename T>
    void FilteredModifierList<T>::Iterator::operator++()
    {
        current = Adjust(current->next.Ptr());
    }
    //
    template<typename T>
    Modifier* FilteredModifierList<T>::Adjust(Modifier* modifier)
    {
        Modifier* m = modifier;
        for (;;)
        {
            if (!m) return m;
            if (as<T>(m))
            {
                return m;
            }
            m = m->next.Ptr();
        }        
    }

    // TODO: where should this live?
    SubstitutionSet createDefaultSubstitutions(
        Session*        session,
        Decl*           decl,
        SubstitutionSet  parentSubst);

    SubstitutionSet createDefaultSubstitutions(
        Session* session,
        Decl*   decl);

    DeclRef<Decl> createDefaultSubstitutionsIfNeeded(
        Session*        session,
        DeclRef<Decl>   declRef);

    RefPtr<GenericSubstitution> createDefaultSubsitutionsForGeneric(
        Session*                session,
        GenericDecl*            genericDecl,
        RefPtr<Substitutions>   outerSubst);

    RefPtr<GenericSubstitution> findInnerMostGenericSubstitution(Substitutions* subst);

    enum class UserDefinedAttributeTargets
    {
        None = 0,
        Struct = 1,
        Var = 2,
        Function = 4,
        All = 7
    };

        /// Get the module that a declaration is associated with, if any.
    Module* getModule(Decl* decl);

    template<typename T>
    SLANG_FORCE_INLINE T* dynamicCast(NodeBase* node)
    {
        return (node && node->getClassInfo().isSubClassOf(SyntaxClassBase::Impl<T>::kClassInfo)) ? static_cast<T*>(node) : nullptr; 
    }

    template<typename T>
    SLANG_FORCE_INLINE const T* dynamicCast(const NodeBase* node)
    {
        return (node && node->getClassInfo().isSubClassOf(SyntaxClassBase::Impl<T>::kClassInfo)) ? static_cast<const T*>(node) : nullptr;
    }

    template<typename T>
    SLANG_FORCE_INLINE T* as(NodeBase* node)
    {
        return (node && node->getClassInfo().isSubClassOf(SyntaxClassBase::Impl<T>::kClassInfo)) ? static_cast<T*>(node) : nullptr;
    }

    template<typename T>
    SLANG_FORCE_INLINE const T* as(const NodeBase* node)
    {
        return (node && node->getClassInfo().isSubClassOf(SyntaxClassBase::Impl<T>::kClassInfo)) ? static_cast<const T*>(node) : nullptr;
    }

} // namespace Slang

#endif

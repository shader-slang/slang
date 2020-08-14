#ifndef SLANG_AST_SUPPORT_TYPES_H
#define SLANG_AST_SUPPORT_TYPES_H

#include "../core/slang-basic.h"
#include "slang-lexer.h"
#include "slang-profile.h"
#include "slang-type-system-shared.h"
#include "../../slang.h"

#include "../core/slang-semantic-version.h"

#include "slang-ast-generated.h"

#include "slang-ast-reflect.h"

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

    class Decl;
    struct QualType;
    class Type;
    struct TypeExp;
    class Val;

    class NodeBase;


    template <typename T>
    T* as(NodeBase* node);

    template <typename T>
    const T* as(const NodeBase* node);

    void printDiagnosticArg(StringBuilder& sb, Decl* decl);
    void printDiagnosticArg(StringBuilder& sb, Type* type);
    void printDiagnosticArg(StringBuilder& sb, TypeExp const& type);
    void printDiagnosticArg(StringBuilder& sb, QualType const& type);
    void printDiagnosticArg(StringBuilder& sb, Val* val);

    class SyntaxNode;
    SourceLoc const& getDiagnosticPos(SyntaxNode const* syntax);
    SourceLoc const& getDiagnosticPos(TypeExp const& typeExp);

    typedef NodeBase* (*SyntaxParseCallback)(Parser* parser, void* userData);

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
            Modifier* current = nullptr;

            Modifier* operator*()
            {
                return current;
            }

            void operator++();

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

        Modifier* modifiers = nullptr;
    };

    // Helper class for iterating over heap-allocated modifiers
    // of a specific type.
    template<typename T>
    struct FilteredModifierList
    {
        struct Iterator
        {
            Modifier* current = nullptr;

            T* operator*()
            {
                return (T*)current;
            }

            void operator++();
            
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
            : modifiers(adjust(modifiers))
        {}

        Iterator begin() { return Iterator(modifiers); }
        Iterator end() { return Iterator(nullptr); }

        static Modifier* adjust(Modifier* modifier);

        Modifier* modifiers = nullptr;
    };

    // A set of modifiers attached to a syntax node
    struct Modifiers
    {
        // The first modifier in the linked list of heap-allocated modifiers
        Modifier* first = nullptr;

        template<typename T>
        FilteredModifierList<T> getModifiersOfType() { return FilteredModifierList<T>(first); }

        // Find the first modifier of a given type, or return `nullptr` if none is found.
        template<typename T>
        T* findModifier()
        {
            return *getModifiersOfType<T>().begin();
        }

        template<typename T>
        bool hasModifier() { return findModifier<T>() != nullptr; }

        FilteredModifierList<Modifier>::Iterator begin() { return FilteredModifierList<Modifier>::Iterator(first); }
        FilteredModifierList<Modifier>::Iterator end() { return FilteredModifierList<Modifier>::Iterator(nullptr); }
    };

    class NamedExpressionType;
    class GenericDecl;
    class ContainerDecl;

    // Try to extract a simple integer value from an `IntVal`.
    // This fill assert-fail if the object doesn't represent a literal value.
    IntegerLiteralValue getIntVal(IntVal* val);

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
        typedef uint8_t RawType;
        DeclCheckStateExt() {}
        DeclCheckStateExt(DeclCheckState state)
            : m_raw(uint8_t(state))
        {}

        enum : RawType
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
            m_raw = (m_raw & kBeingCheckedBit) | RawType(state);
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

        RawType getRaw() const { return m_raw; }
        void setRaw(RawType raw) { m_raw = raw; }

    private:
        RawType m_raw = 0;
    };

    void addModifier(
        ModifiableSyntaxNode*    syntax,
        Modifier*                modifier);

    struct QualType
    {
        Type*	type = nullptr;
        bool	        isLeftValue;

        QualType()
            : isLeftValue(false)
        {}

        QualType(Type* type)
            : type(type)
            , isLeftValue(false)
        {}

        Type* Ptr() { return type; }

        operator Type*() { return type; }
        Type* operator->() { return type; }
    };

    class ASTBuilder;

    struct ReflectClassInfo
    {
        typedef ReflectClassInfo ThisType;

        typedef void* (*CreateFunc)(ASTBuilder* astBuilder);
        typedef void (*DestructorFunc)(void* ptr);

        /// A constant time implementation of isSubClassOf
        SLANG_FORCE_INLINE bool isSubClassOf(const ThisType& super) const
        {
            // We include super.m_classId, because it's a subclass of itself.
            return m_classId >= super.m_classId && m_classId <= super.m_lastClassId;
        }
        // True if typeId derives from this type
        SLANG_FORCE_INLINE bool isDerivedFrom(uint32_t typeId) const
        {
            return typeId >= m_classId && typeId <= m_lastClassId;
        }
        SLANG_FORCE_INLINE static bool isSubClassOf(ASTNodeType type, const ThisType& super)
        {
            // Check the type appears valid
            SLANG_ASSERT(int(type) >= 0);
            // We include super.m_classId, because it's a subclass of itself.
            return uint32_t(type) >= super.m_classId && uint32_t(type) <= super.m_lastClassId;
        }

        /// Will produce the same result as isSubClassOf, but more slowly by traversing the m_superClass
        /// Works without initRange being called. 
        bool isSubClassOfSlow(const ThisType& super) const;

        uint32_t m_classId;
        uint32_t m_lastClassId;

        const ReflectClassInfo* m_superClass;       ///< The super class of this class, or nullptr if has no super class. 
        const char* m_name;                         ///< Textual class name, for debugging 
        CreateFunc m_createFunc;                    ///< Callback to use when creating instances (using an ASTBuilder for backing memory)
        DestructorFunc m_destructorFunc;            ///< The destructor for this type. Being just destructor, does not free backing memory for type.
        uint32_t m_sizeInBytes;                     ///< Total size of the type
        uint8_t m_alignment;                        ///< The required alignment of the type

        struct Infos
        {
            const ReflectClassInfo* infos[int(ASTNodeType::CountOf)];
        };

        SLANG_FORCE_INLINE static const ReflectClassInfo* getInfo(ASTNodeType type) { return kInfos.infos[int(type)]; }

        static const Infos kInfos;
    };

    // A reference to a class of syntax node, that can be
    // used to create instances on the fly
    struct SyntaxClassBase
    {
        SyntaxClassBase()
        {}

        SyntaxClassBase(ReflectClassInfo const* inClassInfo)
            : classInfo(inClassInfo)
        {}

        void* createInstanceImpl(ASTBuilder* astBuilder) const
        {
            auto ci = classInfo;
            if (!ci) return nullptr;

            auto cf = ci->m_createFunc;
            if (!cf) return nullptr;

            return cf(astBuilder);
        }

        SLANG_FORCE_INLINE bool isSubClassOfImpl(SyntaxClassBase const& super) const { return classInfo->isSubClassOf(*super.classInfo); }

        ReflectClassInfo const* classInfo = nullptr;
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

        T* createInstance(ASTBuilder* astBuilder) const
        {
            return (T*)createInstanceImpl(astBuilder);
        }

        SyntaxClass(const ReflectClassInfo* inClassInfo):
            SyntaxClassBase(inClassInfo) 
        {}

        static SyntaxClass<T> getClass()
        {
            return SyntaxClass<T>(&T::kReflectClassInfo);
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

        template<typename U>
        bool operator==(const SyntaxClass<U> other) const
        {
            return classInfo == other.classInfo;
        }

        template<typename U>
        bool operator!=(const SyntaxClass<U> other) const
        {
            return classInfo != other.classInfo;
        }
    };

    template<typename T>
    SyntaxClass<T> getClass()
    {
        return SyntaxClass<T>::getClass();
    }

    struct SubstitutionSet
    {
        Substitutions* substitutions = nullptr;
        operator Substitutions*() const
        {
            return substitutions;
        }

        SubstitutionSet() {}
        SubstitutionSet(Substitutions* subst)
            : substitutions(subst)
        {
        }
        bool equals(const SubstitutionSet& substSet) const;
        HashCode getHashCode() const;
    };

    class ASTBuilder;

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

        DeclRefBase(Decl* decl, Substitutions* subst)
            : decl(decl)
            , substitutions(subst)
        {}

        // Apply substitutions to a type or declaration
        Type* substitute(ASTBuilder* astBuilder, Type* type) const;

        DeclRefBase substitute(ASTBuilder* astBuilder, DeclRefBase declRef) const;

        // Apply substitutions to an expression
        Expr* substitute(ASTBuilder* astBuilder, Expr* expr) const;

        // Apply substitutions to this declaration reference
        DeclRefBase substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff);

        // Returns true if 'as' will return a valid cast
        template <typename T>
        bool is() const { return Slang::as<T>(decl) != nullptr; }

        // "dynamic cast" to a more specific declaration reference type
        template<typename T>
        DeclRef<T> as() const;

        // Check if this is an equivalent declaration reference to another
        bool equals(DeclRefBase const& declRef) const;
        bool operator == (const DeclRefBase& other) const
        {
            return equals(other);
        }

        // Convenience accessors for common properties of declarations
        Name* getName() const;
        SourceLoc getLoc() const;
        DeclRefBase getParent() const;

        HashCode getHashCode() const;

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

        DeclRef(T* decl, Substitutions* subst)
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

        Type* substitute(ASTBuilder* astBuilder, Type* type) const
        {
            return DeclRefBase::substitute(astBuilder, type);
        }
        Expr* substitute(ASTBuilder* astBuilder, Expr* expr) const
        {
            return DeclRefBase::substitute(astBuilder, expr);
        }

        // Apply substitutions to a type or declaration
        template<typename U>
        DeclRef<U> substitute(ASTBuilder* astBuilder, DeclRef<U> declRef) const
        {
            return DeclRef<U>::unsafeInit(DeclRefBase::substitute(astBuilder, declRef));
        }

        // Apply substitutions to this declaration reference
        DeclRef<T> substituteImpl(ASTBuilder* astBuilder, SubstitutionSet subst, int* ioDiff)
        {
            return DeclRef<T>::unsafeInit(DeclRefBase::substituteImpl(astBuilder, subst, ioDiff));
        }

        DeclRef<ContainerDecl> getParent() const
        {
            return DeclRef<ContainerDecl>::unsafeInit(DeclRefBase::getParent());
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

    Decl*const* adjustFilterCursorImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end);
    Decl*const* getFilterCursorByIndexImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end, Index index);
    Index getFilterCountImpl(const ReflectClassInfo& clsInfo, MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end);


    template <typename T>
    Decl*const* adjustFilterCursor(MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end)
    {
        return adjustFilterCursorImpl(T::kReflectClassInfo, filterStyle, ptr, end);
    }

        /// Finds the element at index. If there is no element at the index (for example has too few elements), returns nullptr.
    template <typename T>
    Decl*const* getFilterCursorByIndex(MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end, Index index)
    {
        return getFilterCursorByIndexImpl(T::kReflectClassInfo, filterStyle, ptr, end, index);
    }
     
    template <typename T>
    Index getFilterCount(MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end)
    {
        return getFilterCountImpl(T::kReflectClassInfo, filterStyle, ptr, end);
    }
     
    template <typename T>
    bool isFilterNonEmpty(MemberFilterStyle filterStyle, Decl*const* ptr, Decl*const* end)
    {
        return adjustFilterCursorImpl(T::kReflectClassInfo, filterStyle, ptr, end) != end;
    }

    template<typename T>
    struct FilteredMemberList
    {
        typedef Decl* Element;

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

            T* operator*() { return  static_cast<T*>(*m_cursor); }
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
        T* getFirst() { return *begin(); }
        Index getCount() { return getFilterCount<T>(m_filterStyle, m_begin, m_end); }

        T* operator[](Index index) const
        {
            Decl*const* ptr = getFilterCursorByIndex<T>(m_filterStyle, m_begin, m_end, index);
            SLANG_ASSERT(ptr);
            return  static_cast<T*>(*ptr);
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
        Decl*	decl = nullptr;
    };

    template<typename T>
    struct FilteredMemberRefList
    {
        List<Decl*> const&	m_decls;
        SubstitutionSet		m_substitutions;
        MemberFilterStyle   m_filterStyle;

        FilteredMemberRefList(
            List<Decl*> const&	decls,
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
             Decl*const* decl = getFilterCursorByIndex<T>(m_filterStyle, m_decls.begin(), m_decls.end(), index);
             SLANG_ASSERT(decl);
             return DeclRef<T>((T*) *decl, m_substitutions);
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
            Decl*const* m_ptr;
            Decl*const* m_end;
            MemberFilterStyle m_filterStyle;

            Iterator() : m_list(nullptr), m_ptr(nullptr), m_filterStyle(MemberFilterStyle::All) {}
            Iterator(
                FilteredMemberRefList const* list,
                Decl*const* ptr,
                Decl*const* end,
                MemberFilterStyle filterStyle
                )
                : m_list(list)
                , m_ptr(ptr)
                , m_end(end)
                , m_filterStyle(filterStyle)
            {}

            bool operator!=(const Iterator& other) const  { return m_ptr != other.m_ptr; }

            void operator++() { m_ptr = adjustFilterCursor<T>(m_filterStyle, m_ptr + 1, m_end); }

            DeclRef<T> operator*() { return DeclRef<T>((T*)*m_ptr, m_list->m_substitutions); }
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
        typedef TypeExp ThisType;

        TypeExp() {}
        TypeExp(TypeExp const& other)
            : exp(other.exp)
            , type(other.type)
        {}
        explicit TypeExp(Expr* exp)
            : exp(exp)
        {}
        explicit TypeExp(Type* type)
            : type(type)
        {}
        TypeExp(Expr* exp, Type* type)
            : exp(exp)
            , type(type)
        {}

        Expr* exp = nullptr;
        Type* type = nullptr;

        bool equals(Type* other);

        Type* Ptr() { return type; }
        operator Type*()
        {
            return type;
        }
        Type* operator->() { return Ptr(); }

        ThisType& operator=(const ThisType& rhs) = default;

        //TypeExp accept(SyntaxVisitor* visitor);

            /// A global immutable TypeExp, that has no type or exp set.
        static const TypeExp empty;
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
        ContainerDecl*          containerDecl = nullptr;
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

        /// Flags for options to be used when looking up declarations
    enum class LookupOptions : uint8_t
    {
        None = 0,
        IgnoreBaseInterfaces = 1 << 0,
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

            // For the `Kind::This` case, what does the implicit
            // `this` or `This` parameter refer to?
            //
            enum class ThisParameterMode : uint8_t
            {
                ImmutableValue, // An immutable `this` value
                MutableValue,   // A mutable `this` value
                Type,           // A `This` type

                Default = ImmutableValue,
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
        // The one item that was found, in the simple case
        LookupResultItem item;

        // All of the items that were found, in the complex case.
        // Note: if there was no overloading, then this list isn't
        // used at all, to avoid allocation.
        // 
        // Additionally, if `items` is used, then `item` *must* hold an item that
        // is also in the items list (typically the first entry), as an invariant.
        // Otherwise isValid/begin will not function correctly.
        List<LookupResultItem> items;

        // Was at least one result found?
        bool isValid() const { return item.declRef.getDecl() != nullptr; }

        bool isOverloaded() const { return items.getCount() > 1; }

        Name* getName() const
        {
            return items.getCount() > 1 ? items[0].declRef.getName() : item.declRef.getName();
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
        LookupOptions       options     = LookupOptions::None;
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

        RequirementWitness(Val* val);

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

        Val* getVal()
        {
            SLANG_ASSERT(getFlavor() == Flavor::val);
            return m_val;
        }

        RefPtr<WitnessTable> getWitnessTable();

        RequirementWitness specialize(ASTBuilder* astBuilder, SubstitutionSet const& subst);

        Flavor              m_flavor;
        DeclRef<Decl>       m_declRef;
        RefPtr<RefObject>   m_obj;
        Val*                m_val = nullptr;
    };

    typedef Dictionary<Decl*, RequirementWitness> RequirementDictionary;

    struct WitnessTable : RefObject
    {
        RequirementDictionary requirementDictionary;

        // The type that the witness table witnesses conformance to (e.g. an Interface)
        Type* baseType;
    };

    typedef Dictionary<unsigned int, NodeBase*> AttributeArgumentValueDict;

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
        NodeBase*    object = nullptr;
    };
    typedef List<SpecializationParam> SpecializationParams;

    struct SpecializationArg
    {
        Val* val = nullptr;
    };
    typedef List<SpecializationArg> SpecializationArgs;

    struct ExpandedSpecializationArg : SpecializationArg
    {
        Val* witness = nullptr;
    };
    typedef List<ExpandedSpecializationArg> ExpandedSpecializationArgs;

        /// A reference-counted object to hold a list of candidate extensions
        /// that might be applicable to a type based on its declaration.
        ///
    struct CandidateExtensionList : RefObject
    {
        List<ExtensionDecl*> candidateExtensions;
    };

} // namespace Slang

#endif

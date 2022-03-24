// slang-doc-markdown-writer.h
#ifndef SLANG_DOC_MARKDOWN_WRITER_H
#define SLANG_DOC_MARKDOWN_WRITER_H

#include "slang-doc-ast.h"

#include "slang-ast-print.h"
#include "slang-compiler.h"

namespace Slang {

class ASTBuilder;

struct DocMarkdownWriter
{
    typedef ASTPrinter::Part Part;
    typedef ASTPrinter::PartPair PartPair;

    struct Signature
    {
        struct GenericParam
        {
            Part name;
            Part type;
        };

        Part returnType;
        List<PartPair> params;
        List<GenericParam> genericParams;
        Part name;
    };

    struct Requirement
    {
        typedef Requirement ThisType;

        bool operator<(const ThisType& rhs) const { return Index(target) < Index(rhs.target) || (target == rhs.target && value < rhs.value); } 

        bool operator==(const ThisType& rhs) const { return target == rhs.target && value == rhs.value; }
        SLANG_FORCE_INLINE bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

            /// Using CodeGenTarget may not be most appropriate, perhaps it should use a CapabilityAtom
            /// For now use target, and since we always go through Source -> byte code it is fairly straight forward to understand the
            /// meaning.
        CodeGenTarget target;
            /// The 'value' requirement associated with a target. If it's empty it's just the target that is a requirement.
        String value;
    };

        /// Write out all documentation to the output buffer
    void writeAll();

        /// This will write information about *all* of the overridden versions of a function/method
    void writeCallableOverridable(const ASTMarkup::Entry& entry, CallableDecl* callable);

    void writeEnum(const ASTMarkup::Entry& entry, EnumDecl* enumDecl);
    void writeAggType(const ASTMarkup::Entry& entry, AggTypeDeclBase* aggTypeDecl);
    void writeDecl(const ASTMarkup::Entry& entry, Decl* decl);
    void writeVar(const ASTMarkup::Entry& entry, VarDecl* varDecl);

    void writePreamble(const ASTMarkup::Entry& entry);
    void writeDescription(const ASTMarkup::Entry& entry);

    void writeSignature(CallableDecl* callableDecl);

    bool isVisible(const ASTMarkup::Entry& entry);
    bool isVisible(Decl* decl);
    bool isVisible(const Name* name);

        /// Get the output string
    const StringBuilder& getOutput() const { return m_builder; }

        /// Ctor.
    DocMarkdownWriter(ASTMarkup* markup, ASTBuilder* astBuilder) :
        m_markup(markup),
        m_astBuilder(astBuilder)
    {
    }

    struct StringListSet;
    
        /// Given a list of ASTPrinter::Parts, works out the different parts of the sig
    static void getSignature(const List<Part>& parts, Signature& outSig);

    struct NameAndText
    {
        String name;
        String text;
    };

    List<NameAndText> _getUniqueParams(const List<Decl*>& decls);

    String _getName(Decl* decl);
    String _getName(InheritanceDecl* decl);

    NameAndText _getNameAndText(ASTMarkup::Entry* entry, Decl* decl);
    NameAndText _getNameAndText(Decl* decl);

    template <typename T>
    List<NameAndText> _getAsNameAndTextList(const FilteredMemberList<T>& in)
    {
        List<NameAndText> out;
        for (auto decl : const_cast<FilteredMemberList<T>&>(in))
        {
            out.add(_getNameAndText(decl));
        }
        return out;
    }
    template <typename T>
    List<String> _getAsStringList(const List<T*>& in)
    {
        List<String> strings;
        for (auto decl : in)
        {
            strings.add(_getName(decl));
        }
        return strings;
    }

    List<NameAndText> _getAsNameAndTextList(const List<Decl*>& in);
    List<String> _getAsStringList(const List<Decl*>& in);

    void _appendAsBullets(const List<NameAndText>& values, char wrapChar);
    void _appendAsBullets(const List<String>& values, char wrapChar);

    void _appendCommaList(const List<String>& strings, char wrapChar);

    void _appendRequirements(const List<DocMarkdownWriter::Requirement>& requirements);
    void _maybeAppendRequirements(const UnownedStringSlice& title, const List<List<DocMarkdownWriter::Requirement>>& uniqueRequirements);
    void _writeTargetRequirements(const Requirement* reqs, Index reqsCount);

        /// Appends prefix and the list of types derived from
    void _appendDerivedFrom(const UnownedStringSlice& prefix, AggTypeDeclBase* aggTypeDecl);
    void _appendEscaped(const UnownedStringSlice& text);

    void _appendAggTypeName(AggTypeDeclBase* aggTypeDecl);

    ASTMarkup* m_markup;
    ASTBuilder* m_astBuilder;
    StringBuilder m_builder;
};

} // namespace Slang

#endif

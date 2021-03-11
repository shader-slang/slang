// slang-doc-markdown.h
#ifndef SLANG_DOC_MARK_DOWN_H
#define SLANG_DOC_MARK_DOWN_H

#include "slang-doc-extractor.h"
#include "slang-ast-print.h"

namespace Slang {

class ASTBuilder;

struct DocMarkDownWriter
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

        /// Write out all documentation to the output buffer
    void writeAll();

        /// This will write information about *all* of the overridden versions of a function/method
    void writeCallableOverridable(const DocMarkup::Entry& entry, CallableDecl* callable);

    void writeEnum(const DocMarkup::Entry& entry, EnumDecl* enumDecl);
    void writeAggType(const DocMarkup::Entry& entry, AggTypeDeclBase* aggTypeDecl);
    void writeDecl(const DocMarkup::Entry& entry, Decl* decl);
    void writeVar(const DocMarkup::Entry& entry, VarDecl* varDecl);

    void writePreamble(const DocMarkup::Entry& entry);
    void writeDescription(const DocMarkup::Entry& entry);

    void writeSignature(CallableDecl* callableDecl);

    bool isVisible(const DocMarkup::Entry& entry);
    bool isVisible(Decl* decl);
    bool isVisible(const Name* name);

        /// Get the output string
    const StringBuilder& getOutput() const { return m_builder; }

        /// Ctor.
    DocMarkDownWriter(DocMarkup* markup, ASTBuilder* astBuilder) :
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

    NameAndText _getNameAndText(DocMarkup::Entry* entry, Decl* decl);
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

    void _maybeAppendSet(const UnownedStringSlice& title, const StringListSet& set);

        /// Appends prefix and the list of types derived from
    void _appendDerivedFrom(const UnownedStringSlice& prefix, AggTypeDeclBase* aggTypeDecl);
    void _appendEscaped(const UnownedStringSlice& text);

    void _appendAggTypeName(AggTypeDeclBase* aggTypeDecl);

    DocMarkup* m_markup;
    ASTBuilder* m_astBuilder;
    StringBuilder m_builder;
};

} // namespace Slang

#endif

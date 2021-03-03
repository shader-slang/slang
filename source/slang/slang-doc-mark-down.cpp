// slang-doc-mark-down.cpp
#include "slang-doc-mark-down.h"

#include "../core/slang-string-util.h"

#include "slang-ast-builder.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DocMarkDownWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

template <typename T>
static void _getDecls(ContainerDecl* containerDecl, List<T*>& out)
{
    for (Decl* decl : containerDecl->members)
    {
        if (T* declAsType = as<T>(decl))
        {
            out.add(declAsType);
        }
    }
}

template <typename T>
static void _getDeclsOfType(ContainerDecl* containerDecl, List<Decl*>& out)
{
    for (Decl* decl : containerDecl->members)
    {
        if (as<T>(decl))
        {
            out.add(decl);
        }
    }
}

template <typename T>
static void _toList(FilteredMemberList<T>& list, List<Decl*>& out)
{
    for (Decl* decl : list)
    {
        out.add(decl);
    }
}

static void _appendAsSingleLine(const UnownedStringSlice& in, StringBuilder& out)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(in, lines);

    // Ideally we'd remove any extraneous whitespace, but for now just join
    StringUtil::join(lines.getBuffer(), lines.getCount(), ' ', out);
}

void DocMarkDownWriter::_appendAsBullets(const List<Decl*>& in)
{
    auto& out = m_builder;
    for (auto decl : in)
    {
        DocMarkup::Entry* paramEntry = m_markup->getEntry(decl);

        out << "* ";

        Name* name = decl->getName();
        if (name)
        {
            out << toSlice("_") << name->text << toSlice("_ ");
        }

        if (paramEntry)
        {
            // Hmm, we'll want to make something multiline into a single line
            _appendAsSingleLine(paramEntry->m_markup.getUnownedSlice(), out);
        }

        out << "\n";
    }

    out << toSlice("\n");
}

template <typename T>
void DocMarkDownWriter::_appendAsBullets(FilteredMemberList<T>& list)
{
    List<Decl*> decls;
    _toList(list, decls);
    _appendAsBullets(decls);
}

/* static */void DocMarkDownWriter::getSignature(const List<Part>& parts, Signature& outSig)
{
    const Index count = parts.getCount();
    for (Index i = 0; i < count; ++i)
    {
        const auto& part = parts[i];
        switch (part.type)
        {
            case Part::Type::ParamType:
            {
                PartPair pair;
                pair.first = part;
                if ((i + 1) < count && parts[i + 1].type == Part::Type::ParamName)
                {
                    pair.second = parts[i + 1];
                    i++;
                }
                outSig.params.add(pair);
                break;
            }
            case Part::Type::ReturnType:
            {
                outSig.returnType = part;
                break;
            }
            case Part::Type::DeclPath:
            {
                outSig.name = part;
                break;
            }
            case Part::Type::GenericParamValue:
            case Part::Type::GenericParamType:
            {
                Signature::GenericParam genericParam;
                genericParam.name = part;
                
                if ((i + 1) < count && parts[i + 1].type == Part::Type::GenericParamValueType)
                {
                    genericParam.type = parts[i + 1];
                    i++;
                }

                outSig.genericParams.add(genericParam);
                break;
            }

            default: break;
        }
    }
}

void DocMarkDownWriter::writeVar(const DocMarkup::Entry& entry, VarDecl* varDecl)
{
    writePreamble(entry);
    auto& out = m_builder;

    out << toSlice("# ") << varDecl->getName()->text << toSlice("\n\n");

    // TODO(JS): The outputting of types this way isn't right - it doesn't handle int a[10] for example. 
    //ASTPrinter printer(m_astBuilder, ASTPrinter::OptionFlag::ParamNames);

    out << toSlice("```\n");
    out << varDecl->type << toSlice(" ") << varDecl <<  toSlice("\n");
    out << toSlice("```\n");

    writeDescription(entry);
}

void DocMarkDownWriter::writeCallable(const DocMarkup::Entry& entry, CallableDecl* callableDecl)
{
    writePreamble(entry);

    auto& out = m_builder;

    List<ASTPrinter::Part> parts;
    ASTPrinter printer(m_astBuilder, ASTPrinter::OptionFlag::ParamNames, &parts);

    GenericDecl* genericDecl = as<GenericDecl>(callableDecl->parentDecl);

    if (genericDecl)
    {
        printer.addDeclSignature(DeclRef<Decl>(genericDecl, nullptr));
    }
    else
    {
        printer.addDeclSignature(DeclRef<Decl>(callableDecl, nullptr));
    }

    Signature signature;
    getSignature(parts, signature);

    const Index paramCount = signature.params.getCount();

    // Output the signature
    {        
        // Extract the name
        out << toSlice("# ") << printer.getPartSlice(signature.name) << toSlice("\n\n");

        out << toSlice("## Signature \n");
        out << toSlice("```\n");
        out << printer.getPartSlice(signature.returnType) << toSlice(" ");

        out << printer.getPartSlice(signature.name);

        if (signature.genericParams.getCount())
        {
            out << toSlice("<");
            const Index count = signature.genericParams.getCount();
            for (Index i = 0; i < count; ++i)
            {
                const auto& genericParam = signature.genericParams[i];
                if (i > 0)
                {
                    out << toSlice(", ");
                }
                out << printer.getPartSlice(genericParam.name);

                if (genericParam.type.type != Part::Type::None)
                {
                    out << toSlice(" : ");
                    out << printer.getPartSlice(genericParam.type);
                }
            }
            out << toSlice(">");
        }

        if (paramCount > 0)
        {
            out << toSlice("(\n");

            StringBuilder line;
            for (Index i = 0; i < paramCount; ++i)
            {
                const auto& param = signature.params[i];
                line.Clear();
                // If we want to tab these over... we'll need to know how must space I have
                line << "    " << printer.getPartSlice(param.first);

                Index indent = 25;
                if (line.getLength() < indent)
                {
                    line.appendRepeatedChar(' ', indent - line.getLength());
                }
                else
                {
                    line.appendChar(' ');
                }

                line << printer.getPartSlice(param.second);
                if (i < paramCount - 1)
                {
                    line << ",\n";
                }

                out << line;
            }

            out << ");\n";
        }
        else
        {
            out << toSlice("();\n");
        }

        out << "```\n\n";
    }

    {
        // The parameters, in order
        List<Decl*> params;

        if (genericDecl)
        {
            for (Decl* decl : genericDecl->members)
            {
                if (as<GenericTypeParamDecl>(decl) ||
                    as<GenericValueParamDecl>(decl))
                {
                    params.add(decl);
                }
            }
        }

        for (ParamDecl* paramDecl : callableDecl->getParameters())
        {
            params.add(paramDecl);
        }

        if (params.getCount())
        {
            out << "## Parameters\n\n";
            // We have generic params and regular parameters, in this list
            _appendAsBullets(params);
        }
    }

    writeDescription(entry);
}

void DocMarkDownWriter::writeEnum(const DocMarkup::Entry& entry, EnumDecl* enumDecl)
{
    writePreamble(entry);

    auto& out = m_builder;

    out << toSlice("# enum ");
    Name* name = enumDecl->getName();
    if (name)
    {
        out << name->text;
    }
    out << toSlice("\n\n");

    out << toSlice("## Values \n\n");

    auto cases = enumDecl->getMembersOfType<EnumCaseDecl>();
    _appendAsBullets(cases);

    writeDescription(entry);
}

void DocMarkDownWriter::_appendDerivedFrom(const UnownedStringSlice& prefix, AggTypeDeclBase* aggTypeDecl)
{
    auto& out = m_builder;

    List<InheritanceDecl*> inheritanceDecls;
    _getDecls(aggTypeDecl, inheritanceDecls);

    const Index count = inheritanceDecls.getCount();
    if (count)
    {
        out << prefix;
        for (Index i = 0; i < count; ++i)
        {
            InheritanceDecl* inheritanceDecl = inheritanceDecls[i];
            if (i > 0)
            {
                out << toSlice(", ");
            }
            out << inheritanceDecl->base;
        }
    }
}

void DocMarkDownWriter::writeAggType(const DocMarkup::Entry& entry, AggTypeDeclBase* aggTypeDecl)
{
    writePreamble(entry);

    auto& out = m_builder;

    // This could be lots of different things - struct/class/extension/interface/..

    out << toSlice("# ");
    if (as<StructDecl>(aggTypeDecl))
    {
        out << toSlice("struct ");
    }
    else if (as<ClassDecl>(aggTypeDecl))
    {
        out << toSlice("class ");
    }
    else if (as<InterfaceDecl>(aggTypeDecl))
    {
        out << toSlice("interface ");
    }
    else if (ExtensionDecl* extensionDecl = as<ExtensionDecl>(aggTypeDecl))
    {
        out << toSlice("extension ") << extensionDecl->targetType;
        _appendDerivedFrom(toSlice(" : "), extensionDecl);   
    }
    else
    {
        out << toSlice("?");
    }

    Name* name = aggTypeDecl->getName();
    if (name)
    {
        out << name->text;
    }
    out << toSlice("\n\n");

    {
        List<InheritanceDecl*> inheritanceDecls;
        _getDecls<InheritanceDecl>(aggTypeDecl, inheritanceDecls);

        if (inheritanceDecls.getCount())
        {
            out << "*Derives from:* ";

            for (Index i = 0; i < inheritanceDecls.getCount(); ++i)
            {
                if (i > 0)
                {
                    out << toSlice(", ");
                }
                out << inheritanceDecls[i]->base;
            }
            out << toSlice("\n\n");
        }
    }

    {
        List<Decl*> fields;
        _getDeclsOfType<VarDecl>(aggTypeDecl, fields);
        if (fields.getCount())
        {
            out << "## Fields\n\n";
            _appendAsBullets(fields);
        }
    }

    {
        List<Decl*> methods;
        _getDeclsOfType<CallableDecl>(aggTypeDecl, methods);
        if (methods.getCount())
        {
            out << "## Methods\n\n";
            _appendAsBullets(methods);
        }
    }

    writeDescription(entry);
}

void DocMarkDownWriter::writePreamble(const DocMarkup::Entry& entry)
{
    SLANG_UNUSED(entry);
    auto& out = m_builder;

    out << toSlice("\n");
    out.appendRepeatedChar('-', 80);
    out << toSlice("\n");
}


void DocMarkDownWriter::writeDescription(const DocMarkup::Entry& entry)
{
    auto& out = m_builder;

    out << toSlice("\n## Description\n\n");
    out << entry.m_markup;
}

void DocMarkDownWriter::writeDecl(const DocMarkup::Entry& entry, Decl* decl)
{
    // Skip these they will be output as part of their respective 'containers'
    if (as<ParamDecl>(decl) || as<EnumCaseDecl>(decl))
    {
        return; 
    }

    if (CallableDecl* callableDecl = as<CallableDecl>(decl))
    {
        writeCallable(entry, callableDecl);
    }
    else if (EnumDecl* enumDecl = as<EnumDecl>(decl))
    {
        writeEnum(entry, enumDecl);
    }
    else if (AggTypeDeclBase* aggType = as<AggTypeDeclBase>(decl))
    {
        writeAggType(entry, aggType);
    }
    else if (VarDecl* varDecl = as<VarDecl>(decl))
    {
        writeVar(entry, varDecl);
    }
    else if (as<GenericDecl>(decl))
    {
        // We can ignore as inner decls will be picked up, and written
    }
}


void DocMarkDownWriter::writeAll()
{
    for (const auto& entry : m_markup->getEntries())
    {
        NodeBase* node = entry.m_node;
        Decl* decl = as<Decl>(node);
        if (decl)
        {
            writeDecl(entry, decl);
        }
    }
}

} // namespace Slang

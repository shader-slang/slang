// slang-doc-mark-down.cpp
#include "slang-doc-mark-down.h"

#include "../core/slang-string-util.h"

#include "slang-ast-builder.h"
#include "slang-lookup.h"

namespace Slang {


struct DocMarkDownWriter::StringListSet
{
    Index add(const HashSet<String>& set)
    {
        // -1 means empty, which we don't explicitly store
        Index index = -1;
        if (set.Count())
        {
            List<String> values;
            for (auto& value : set)
            {
                values.add(value);
            }
            // Sort so that can be compared for uniqueness
            values.sort();

            index = m_uniqueValues.indexOf(values);
            if (index < 0)
            {
                index = m_uniqueValues.getCount();
                m_uniqueValues.add(values);
            }
        }
        m_map.add(index);
        return index;
    }

    Index getValueIndex(Index index) const { return m_map[index]; }
    List<String>& getValuesAt(Index valueIndex) { return m_uniqueValues[valueIndex]; }
    const List<List<String>>& getUniqueValues() const { return m_uniqueValues; }

    StringListSet() {}

protected:
    List<Index> m_map;
    List<List<String>> m_uniqueValues;
};

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

void DocMarkDownWriter::_appendAsBullets(const List<NameAndMarkup>& values)
{
    auto& out = m_builder;
    for (const auto& value : values)
    {
        out << "* ";

        Name* name = value.name;
        if (name)
        {
            out << toSlice("_") << name->text << toSlice("_ ");
        }

        if (value.markup.getLength())
        {
            // Hmm, we'll want to make something multiline into a single line
            _appendAsSingleLine(value.markup.getUnownedSlice(), out);
        }

        out << "\n";
    }
}


void DocMarkDownWriter::_appendAsBullets(const List<Decl*>& in)
{
    auto& out = m_builder;
    for (auto decl : in)
    {
        DocMarkup::Entry* paramEntry = m_markup->getEntry(decl);

        out << toSlice("* _");
        ASTPrinter::appendDeclName(decl, out);
        out << toSlice("_ ");

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

void DocMarkDownWriter::_appendCommaList(const List<InheritanceDecl*>& inheritanceDecls)
{
    for (Index i = 0; i < inheritanceDecls.getCount(); ++i)
    {
        if (i > 0)
        {
            m_builder << toSlice(", ");
        }
        m_builder << inheritanceDecls[i]->base;
    }
}

void DocMarkDownWriter::_appendCommaList(const List<String>& strings)
{
    for (Index i = 0; i < strings.getCount(); ++i)
    {
        if (i > 0)
        {
            m_builder << toSlice(", ");
        }
        m_builder << strings[i];
    }
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
    out << toSlice("```\n\n");

    writeDescription(entry);
}

void DocMarkDownWriter::writeSignature(CallableDecl* callableDecl)
{
    auto& out = m_builder;

    List<ASTPrinter::Part> parts;
   
    ASTPrinter printer(m_astBuilder, ASTPrinter::OptionFlag::ParamNames, &parts);
    printer.addDeclSignature(DeclRef<Decl>(callableDecl, nullptr));

    Signature signature;
    getSignature(parts, signature);

    const Index paramCount = signature.params.getCount();

    {
        // Some types (like constructors say) don't have any return type, so check before outputting
        const UnownedStringSlice returnType = printer.getPartSlice(signature.returnType);
        if (returnType.getLength() > 0)
        {
            out << returnType << toSlice(" ");
        }
    }

    out << printer.getPartSlice(signature.name);

#if 0
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
#endif

    switch (paramCount)
    {
        case 0:
        {
            // Has no parameters
            out << toSlice("();\n");
            break;
        }
        case 1:
        {
            // Place all on single line
            out.appendChar('(');
            const auto& param = signature.params[0];
            out << printer.getPartSlice(param.first) << toSlice(" ") << printer.getPartSlice(param.second);
            out << ");\n";
            break;
        }
        default:
        {
            // Put each parameter on a line on it's own
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
            break;
        }
    }
}

void DocMarkDownWriter::_getUniqueParams(const List<Decl*>& decls, List<NameAndMarkup>& out)
{
    Dictionary<Name*, Index> nameDict;

    for (auto decl : decls)
    {
        Name* name = decl->getName();
        if (!name)
        {
            continue;
        }

        Index index = nameDict.GetOrAddValue(name, out.getCount());

        if (index >= out.getCount())
        {
            out.add(NameAndMarkup{ name, String() });
        }

        NameAndMarkup& nameAndMarkup = out[index];
        if (nameAndMarkup.markup.getLength() > 0)
        {
            continue;
        }

        auto entry = m_markup->getEntry(decl);
        if (entry && entry->m_markup.getLength())
        {
            nameAndMarkup.markup = entry->m_markup;
        }
    }
}

static void _addRequirements(Decl* decl, HashSet<String>& outRequirements)
{
    StringBuilder buf;

    if (auto spirvRequiredModifier = decl->findModifier<RequiredSPIRVVersionModifier>())
    {
        buf.Clear();
        buf << "SPIR-V ";
        spirvRequiredModifier->version.append(buf);
        outRequirements.Add(buf);
    }

    if (auto glslRequiredModifier = decl->findModifier<RequiredGLSLVersionModifier>())
    {
        buf.Clear();
        buf << "GLSL" << glslRequiredModifier->versionNumberToken.getContent();
        outRequirements.Add(buf);
    }

    if (auto cudaSMVersionModifier = decl->findModifier<RequiredCUDASMVersionModifier>())
    {
        buf.Clear();
        buf << "CUDA SM ";
        cudaSMVersionModifier->version.append(buf);
        outRequirements.Add(buf);
    }

    if (auto extensionModifier = decl->findModifier<RequiredGLSLExtensionModifier>())
    {
        buf.Clear();
        buf << "GLSL " << extensionModifier->extensionNameToken.getContent();
        outRequirements.Add(buf);
    }

    if (auto requiresNVAPIAttribute = decl->findModifier<RequiresNVAPIAttribute>())
    {
        outRequirements.Add("NVAPI");
    }
}

void DocMarkDownWriter::_maybeAppendSet(const UnownedStringSlice& title, const StringListSet& set)
{
     auto& out = m_builder;

    const auto& uniqueValues = set.getUniqueValues();

    const Index uniqueCount = uniqueValues.getCount();
    if (uniqueCount > 0 && uniqueValues[0].getCount() > 0)
    {
        out << title;
        if (uniqueCount > 1)
        {
            for (Index i = 0; i < uniqueCount; ++i)
            {
                out << toSlice("* _") << (i + 1) << ("_ ");
                _appendCommaList(uniqueValues[i]);
                out << toSlice("\n");
            }
        }
        else
        {
            _appendCommaList(uniqueValues[0]);
            out << toSlice("\n");
        }
        out << toSlice("\n");
    }
}

void DocMarkDownWriter::writeOverridableCallable(const DocMarkup::Entry& entry, CallableDecl* callableDecl)
{
    auto& out = m_builder;

    writePreamble(entry);

    {
        List<ASTPrinter::Part> parts;
        ASTPrinter printer(m_astBuilder, ASTPrinter::OptionFlag::ParamNames, &parts);
 
        printer.addDeclSignature(DeclRef<Decl>(callableDecl, nullptr));

        // We just want the name
        UnownedStringSlice name = printer.getPartSlice(Part::Type::DeclPath);

        // Extract the name
        out << toSlice("# ") << name << toSlice("\n\n");

    }

    writeDescription(entry);

    List<CallableDecl*> sigs;
    {
        for (Decl* curDecl = callableDecl; curDecl; curDecl = curDecl->nextInContainerWithSameName)
        {
            CallableDecl* sig = as<CallableDecl>(curDecl);
            if (!sig)
            {
                continue;
            }
            // Want to add only the primary sig
            if (sig->primaryDecl == nullptr || sig->primaryDecl == sig)
            {
                sigs.add(sig);
            }
        }

        // Lets put back into source order
        sigs.sort([](CallableDecl* a, CallableDecl* b) -> bool { return a->loc.getRaw() < b->loc.getRaw(); });
    }

    // Set of sets of requirements, holds index map from addition to the set entry 
    StringListSet requirementsSetSet;

    // Similarly for targets 
    StringListSet targetSetSet;

    // We want to determine the unique signature, and then the requirements for the signature
    {
        for (Index i = 0; i < sigs.getCount(); ++i)
        {
            CallableDecl* sig = sigs[i];
            // Add the requirements for all the different versions
            {
                HashSet<String> requirementsSet;
                HashSet<String> targetSet;
                for (CallableDecl* curSig = sig; curSig; curSig = curSig->nextDecl)
                {
                    _addRequirements(sig, requirementsSet);

                    // Handle Target info

                    for (auto targetIntrinsic : sig->getModifiersOfType<TargetIntrinsicModifier>())
                    {
                        targetSet.Add(String(targetIntrinsic->targetToken.getContent()).toUpper());
                    }
                    for (auto specializedForTarget : sig->getModifiersOfType<SpecializedForTargetModifier>())
                    {
                        targetSet.Add(String(specializedForTarget->targetToken.getContent()).toUpper());
                    }
                }
                
                requirementsSetSet.add(requirementsSet);

                // TODO(JS): This really isn't right, we ideally have markup that made hlsl availability explicit
                // We *assume* that we have 'hlsl' for now
                targetSet.Add(String("HLSL"));
                targetSetSet.add(targetSet);
            }
        }
    }
    
    // Output the signature
    {          
        out << toSlice("## Signature \n\n");
        out << toSlice("```\n");

        Index prevRequirementsIndex = -1;
        Index prevTargetIndex = -1;

        const Int sigCount = sigs.getCount();
        for (Index i = 0; i < sigCount; ++i)
        {
            auto sig = sigs[i];

            // Output if needs unique requirements
            if (requirementsSetSet.getUniqueValues().getCount() > 1)
            {
                const Index requirementsIndex = requirementsSetSet.getValueIndex(i);
                if (requirementsIndex != prevRequirementsIndex)
                {
                    if (requirementsIndex >= 0)
                    {
                        out << toSlice("/// See Requirement ") << (requirementsIndex + 1) << toSlice("\n");
                    }
                    else
                    {
                        out << toSlice("/// No requirements\n");
                    }
                    prevRequirementsIndex = requirementsIndex;
                }
            }

#if 1
            if (targetSetSet.getUniqueValues().getCount() > 1)
            {
                const Index targetIndex = targetSetSet.getValueIndex(i);
                if (targetIndex != prevTargetIndex)
                {
                    if (targetIndex >= 0)
                    {
                        out << toSlice("/// See Target Availability ") << (targetIndex + 1) << toSlice("\n");
                    }
                    else
                    {
                        out << toSlice("/// All Targets\n");
                    }
                    prevTargetIndex = targetIndex;
                }
            }
#endif

            writeSignature(sig);
        }
        out << "```\n\n";
    }

    _maybeAppendSet(toSlice("## Requirements\n\n"), requirementsSetSet);

    // Target availability

    {
        const auto& uniqueValues = targetSetSet.getUniqueValues();
        if (uniqueValues.getCount() == 1 && uniqueValues[0].getCount() == 1 && uniqueValues[0][0] == "hlsl")
        {
            // TODO(JS):
            // If something is marked up for hlsl, and nothing else, that indicates it might *only* be available on hlsl, but
            // we don't correctly handle that here.

            // If the only thing we have is 'hlsl' - we injected that so we'll *assume* it's available everywhere, so
            // don't bother outputting.
        }
        else
        {
            _maybeAppendSet(toSlice("## Target Availability\n\n"), targetSetSet);
        }
    }

    {
        // We will use the first documentation found for each parameter type
        {
            List<Decl*> paramDecls;
            List<Decl*> genericDecls;
            for (auto sig : sigs)
            {
                GenericDecl* genericDecl = as<GenericDecl>(sig->parentDecl);

                // NOTE!
                // Here we assume the names of generic parameters are such that they are 

                // We list generic parameters, as types of parameters, if they are directly associated with this
                // callable.
                if (genericDecl)
                {
                    for (Decl* decl : genericDecl->members)
                    {
                        if (as<GenericTypeParamDecl>(decl) ||
                            as<GenericValueParamDecl>(decl))
                        {
                            genericDecls.add(decl);
                        }
                    }
                }

                for (ParamDecl* paramDecl : sig->getParameters())
                {
                    paramDecls.add(paramDecl);
                }
            }

            if (paramDecls.getCount() > 0 || paramDecls.getCount() > 0)
            {
                out << "## Parameters\n\n";

                // Get the unique generics
                List<NameAndMarkup> params;
                _getUniqueParams(genericDecls, params);
                _appendAsBullets(params);

                params.clear();
                _getUniqueParams(paramDecls, params);
                _appendAsBullets(params);
            }
        }
    }
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

    // We can write out he name using the printer

    ASTPrinter printer(m_astBuilder);
    printer.addDeclPath(DeclRef<Decl>(aggTypeDecl, nullptr));

    // This could be lots of different things - struct/class/extension/interface/..

    out << toSlice("# ");
    if (as<StructDecl>(aggTypeDecl))
    {
        out << toSlice("struct ") << printer.getStringBuilder();
    }
    else if (as<ClassDecl>(aggTypeDecl))
    {
        out << toSlice("class ") << printer.getStringBuilder();
    }
    else if (as<InterfaceDecl>(aggTypeDecl))
    {
        out << toSlice("interface ") << printer.getStringBuilder();
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

    out << toSlice("\n\n");

    {
        List<InheritanceDecl*> inheritanceDecls;
        _getDecls<InheritanceDecl>(aggTypeDecl, inheritanceDecls);

        if (inheritanceDecls.getCount())
        {
            out << "*Implements:* ";
            _appendCommaList(inheritanceDecls);
            out << toSlice("\n\n");
        }
    }

    writeDescription(entry);

    {
        List<AssocTypeDecl*> assocTypeDecls;
        _getDecls<AssocTypeDecl>(aggTypeDecl, assocTypeDecls);

        if (assocTypeDecls.getCount())
        {
            out << toSlice("# Associated types\n\n");

            for (AssocTypeDecl* assocTypeDecl : assocTypeDecls)
            {    
                out << "* _" << assocTypeDecl->getName()->text << "_ ";

                // Look up markup
                DocMarkup::Entry* assocTypeDeclEntry = m_markup->getEntry(assocTypeDecl);
                if (assocTypeDeclEntry)
                {
                    _appendAsSingleLine(assocTypeDeclEntry->m_markup.getUnownedSlice(), out);
                }
                out << toSlice("\n");

                
                List<InheritanceDecl*> inheritanceDecls;
                _getDecls<InheritanceDecl>(assocTypeDecl, inheritanceDecls);

                if (inheritanceDecls.getCount())
                {
                    out << "  ";
                    _appendCommaList(inheritanceDecls);
                    out << toSlice("\n");
                }
            }

            out << toSlice("\n\n");
        }
    }

    if (GenericDecl* genericDecl = as<GenericDecl>(aggTypeDecl->parentDecl))
    {
        // The parameters, in order
        List<Decl*> params;
        for (Decl* decl : genericDecl->members)
        {
            if (as<GenericTypeParamDecl>(decl) ||
                as<GenericValueParamDecl>(decl))
            {
                params.add(decl);
            }
        }
    
        if (params.getCount())
        {
            out << "## Generic Parameters\n\n";
            // We have generic params and regular parameters, in this list
            _appendAsBullets(params);
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
        // Make sure we've got a query-able member dictionary
        buildMemberDictionary(aggTypeDecl);
        SLANG_ASSERT(aggTypeDecl->isMemberDictionaryValid());

        List<Decl*> uniqueMethods;
        for (const auto pair : aggTypeDecl->memberDictionary)
        {
            CallableDecl* callableDecl = as<CallableDecl>(pair.Value);
            if (callableDecl && isVisible(callableDecl))
            {
                uniqueMethods.add(callableDecl);
            }
        }

        if (uniqueMethods.getCount())
        {
            out << "## Methods\n\n";
            _appendAsBullets(uniqueMethods);
        }
    }
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

    if (entry.m_markup.getLength() > 0)
    {
        out << toSlice("## Description\n\n");

        out << entry.m_markup.getUnownedSlice();
#if 0
        UnownedStringSlice text(entry.m_markup.getUnownedSlice()), line;
        while (StringUtil::extractLine(text, line))
        {
            out << line << toSlice("\n");
        }
#endif
        out << toSlice("\n");
    }
}

void DocMarkDownWriter::writeDecl(const DocMarkup::Entry& entry, Decl* decl)
{
    // Skip these they will be output as part of their respective 'containers'
    if (as<ParamDecl>(decl) || as<EnumCaseDecl>(decl) || as<AssocTypeDecl>(decl) || as<InheritanceDecl>(decl))
    {
        return; 
    }

    if (CallableDecl* callableDecl = as<CallableDecl>(decl))
    {
        ContainerDecl* parentDecl = decl->parentDecl;

        // Make sure we have the member dictionary.
        buildMemberDictionary(decl->parentDecl);

        // We only write out for the primary/first
        bool isFirst = false;
        
        Name* declName = callableDecl->getName();
        if (declName)
        {
            Decl** firstDeclPtr = parentDecl->memberDictionary.TryGetValue(declName);
            isFirst = (firstDeclPtr && *firstDeclPtr == decl) || (firstDeclPtr == nullptr);
        }

        if (isFirst)
        {
            writeOverridableCallable(entry, callableDecl);
        }
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
        // If part of aggregate type will be output there.
        if (as<AggTypeDecl>(varDecl->parentDecl))
        {
            return;
        }

        writeVar(entry, varDecl);
    }
    else if (as<GenericDecl>(decl))
    {
        // We can ignore as inner decls will be picked up, and written
    }
}

bool DocMarkDownWriter::isVisible(const Name* name)
{
    return name == nullptr || !name->text.startsWith(toSlice("__"));
}

bool DocMarkDownWriter::isVisible(const DocMarkup::Entry& entry)
{
    // For now if it's not public it's not visible
    if (entry.m_visibility != MarkupVisibility::Public)
    {
        return false;
    }

    Decl* decl = as<Decl>(entry.m_node);
    return decl == nullptr || isVisible(decl->getName());
}

bool DocMarkDownWriter::isVisible(Decl* decl)
{
    if (!isVisible(decl->getName()))
    {
        return false;
    }

    auto entry = m_markup->getEntry(decl);
    return entry == nullptr || entry->m_visibility == MarkupVisibility::Public;
}

void DocMarkDownWriter::writeAll()
{
    for (const auto& entry : m_markup->getEntries())
    {
        Decl* decl = as<Decl>(entry.m_node);
    
        if (decl && isVisible(entry))
        {
            writeDecl(entry, decl);
        }
    }
}

} // namespace Slang

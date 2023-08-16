// slang-doc-markdown-writer.cpp
#include "slang-doc-markdown-writer.h"

#include "../core/slang-string-util.h"
#include "../core/slang-type-text-util.h"

#include "slang-ast-builder.h"
#include "slang-lookup.h"

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

void DocMarkdownWriter::_appendAsBullets(const List<NameAndText>& values, char wrapChar)
{
    auto& out = m_builder;
    for (const auto& value : values)
    {
        out << "* ";

        const String& name = value.name;
        if (name.getLength())
        {
            if (wrapChar)
            {
                out.appendChar(wrapChar);
                out << name;
                out.appendChar(wrapChar);   
            }
            else
            {
                out << name;
            }
        }

        if (value.text.getLength())
        {
            out.appendChar(' ');

            // Hmm, we'll want to make something multiline into a single line
            _appendAsSingleLine(value.text.getUnownedSlice(), out);
        }

        out << "\n";
    }
}

void DocMarkdownWriter::_appendAsBullets(const List<String>& values, char wrapChar)
{
    auto& out = m_builder;
    for (const auto& value : values)
    {
        out << "* ";

        if (value.getLength())
        {
            if (wrapChar)
            {
                out.appendChar(wrapChar);
                out << value;
                out.appendChar(wrapChar);

            }
            else
            {
                out << value;
            }
        }
        out << "\n";
    }
}

String DocMarkdownWriter::_getName(Decl* decl)
{
    StringBuilder buf;
    ASTPrinter::appendDeclName(decl, buf);
    return buf.produceString();
}

String DocMarkdownWriter::_getName(InheritanceDecl* decl)
{
    StringBuilder buf;
    buf.clear();
    buf << decl->base;
    return buf.produceString();
}

DocMarkdownWriter::NameAndText DocMarkdownWriter::_getNameAndText(ASTMarkup::Entry* entry, Decl* decl)
{
    NameAndText nameAndText;

    nameAndText.name = _getName(decl);
 
    // We could extract different text here, but for now just do all markup
    if (entry)
    {
        // For now we'll just use all markup, but really we need something more sophisticated here
        nameAndText.text = entry->m_markup;
    }

    return nameAndText;
}

DocMarkdownWriter::NameAndText DocMarkdownWriter::_getNameAndText(Decl* decl)
{
    ASTMarkup::Entry* entry = m_markup->getEntry(decl);
    return _getNameAndText(entry, decl);
}

List<DocMarkdownWriter::NameAndText> DocMarkdownWriter::_getAsNameAndTextList(const List<Decl*>& in)
{
    List<NameAndText> out;
    for (auto decl : in)
    {
        out.add(_getNameAndText(decl));
    }
    return out;
}

List<String> DocMarkdownWriter::_getAsStringList(const List<Decl*>& in)
{
    List<String> strings;
    for (auto decl : in)
    {
        strings.add(_getName(decl));
    }
    return strings;
}

void DocMarkdownWriter::_appendCommaList(const List<String>& strings, char wrapChar)
{
    for (Index i = 0; i < strings.getCount(); ++i)
    {
        if (i > 0)
        {
            m_builder << toSlice(", ");
        }
        if (wrapChar)
        {
            m_builder.appendChar(wrapChar);
            m_builder << strings[i];
            m_builder.appendChar(wrapChar);
        }
        else
        {
            m_builder << strings[i];
        }
    }
}

/* static */void DocMarkdownWriter::getSignature(const List<Part>& parts, Signature& outSig)
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

void DocMarkdownWriter::writeVar(const ASTMarkup::Entry& entry, VarDecl* varDecl)
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

void DocMarkdownWriter::writeSignature(CallableDecl* callableDecl)
{
    auto& out = m_builder;

    if (callableDecl->hasModifier<HLSLStaticModifier>())
    {
        out << "static ";
    }

    List<ASTPrinter::Part> parts;
   
    ASTPrinter printer(m_astBuilder, ASTPrinter::OptionFlag::ParamNames, &parts);
    printer.addDeclSignature(makeDeclRef(callableDecl));

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
                line.clear();
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

List<DocMarkdownWriter::NameAndText> DocMarkdownWriter::_getUniqueParams(const List<Decl*>& decls)
{
    List<NameAndText> out;

    Dictionary<Name*, Index> nameDict;

    for (auto decl : decls)
    {
        Name* name = decl->getName();
        if (!name)
        {
            continue;
        }

        Index index = nameDict.getOrAddValue(name, out.getCount());

        if (index >= out.getCount())
        {
            out.add(NameAndText{ getText(name), String() });
        }

        NameAndText& nameAndMarkup = out[index];
        if (nameAndMarkup.text.getLength() > 0)
        {
            continue;
        }

        auto entry = m_markup->getEntry(decl);
        if (entry && entry->m_markup.getLength())
        {
            nameAndMarkup.text = entry->m_markup;
        }
    }

    return out;
}

static void _addRequirement(const DocMarkdownWriter::Requirement& req, List<DocMarkdownWriter::Requirement>& ioReqs)
{
    if (ioReqs.indexOf(req) < 0)
    {
        ioReqs.add(req);
    }
}

static void _addRequirement(CodeGenTarget target, const String& value, List<DocMarkdownWriter::Requirement>& ioReqs)
{
    _addRequirement(DocMarkdownWriter::Requirement{ target, value }, ioReqs);
}

static DocMarkdownWriter::Requirement _getRequirementFromTargetToken(const Token& tok)
{
    typedef DocMarkdownWriter::Requirement Requirement;

    if (tok.type == TokenType::Unknown)
    {
        return Requirement{ CodeGenTarget::None, String() };
    }

    auto targetName = tok.getContent();
    if (targetName == "spirv")
    {
        return Requirement{CodeGenTarget::SPIRV, UnownedStringSlice("")};
    }

    const CapabilityAtom targetCap = findCapabilityAtom(targetName);

    if (targetCap == CapabilityAtom::Invalid)
    {
        return Requirement{ CodeGenTarget::None, String() };
    }

    static const CapabilityAtom rootAtoms[] =
    {
        CapabilityAtom::GLSL,
        CapabilityAtom::HLSL,
        CapabilityAtom::CUDA,
        CapabilityAtom::CPP,
        CapabilityAtom::C,
    };

    for (auto rootAtom : rootAtoms)
    {
        if (rootAtom == targetCap)
        {
            // If its one of the roots we don't need to store the name
            targetName = UnownedStringSlice();
            break;
        }
    }

    if (isCapabilityDerivedFrom(targetCap, CapabilityAtom::GLSL))
    {
        return Requirement{CodeGenTarget::GLSL, targetName};
    }
    else if (isCapabilityDerivedFrom(targetCap, CapabilityAtom::HLSL))
    {
        return Requirement{ CodeGenTarget::HLSL, targetName };
    }
    else if (isCapabilityDerivedFrom(targetCap, CapabilityAtom::CUDA))
    {
        return Requirement{ CodeGenTarget::CUDASource, targetName };
    }
    else if (isCapabilityDerivedFrom(targetCap, CapabilityAtom::CPP))
    {
        return Requirement{ CodeGenTarget::CPPSource, targetName };
    }
    else if (isCapabilityDerivedFrom(targetCap, CapabilityAtom::C))
    {
        return Requirement{ CodeGenTarget::CSource, targetName };
    }
    
    return Requirement{ CodeGenTarget::Unknown, String() };
}

static void _addRequirementFromTargetToken(const Token& tok, List<DocMarkdownWriter::Requirement>& ioReqs)
{
    auto req = _getRequirementFromTargetToken(tok);
    if (req.target != CodeGenTarget::None)
    {
        _addRequirement(req, ioReqs);
    }
}

static void _addRequirements(Decl* decl, List<DocMarkdownWriter::Requirement>& ioReqs)
{
    StringBuilder buf;

    if (auto spirvRequiredModifier = decl->findModifier<RequiredSPIRVVersionModifier>())
    {
        buf.clear();
        buf << "SPIR-V ";
        spirvRequiredModifier->version.append(buf);
        _addRequirement(CodeGenTarget::GLSL, buf, ioReqs);
    }

    if (auto glslRequiredModifier = decl->findModifier<RequiredGLSLVersionModifier>())
    {
        buf.clear();
        buf << "GLSL" << glslRequiredModifier->versionNumberToken.getContent();
        _addRequirement(CodeGenTarget::GLSL, buf, ioReqs);
    }

    if (auto cudaSMVersionModifier = decl->findModifier<RequiredCUDASMVersionModifier>())
    {
        buf.clear();
        buf << "SM ";
        cudaSMVersionModifier->version.append(buf);
        _addRequirement(CodeGenTarget::CUDASource, buf, ioReqs);
    }

    if (auto extensionModifier = decl->findModifier<RequiredGLSLExtensionModifier>())
    {
        buf.clear();
        buf << extensionModifier->extensionNameToken.getContent();
        _addRequirement(CodeGenTarget::GLSL, buf, ioReqs);
    }

    if (const auto requiresNVAPIAttribute = decl->findModifier<RequiresNVAPIAttribute>())
    {
        _addRequirement(CodeGenTarget::HLSL, "NVAPI", ioReqs);
    }

    for (auto targetIntrinsic : decl->getModifiersOfType<TargetIntrinsicModifier>())
    {
        _addRequirementFromTargetToken(targetIntrinsic->targetToken, ioReqs);
    }
    for (auto specializedForTarget : decl->getModifiersOfType<SpecializedForTargetModifier>())
    {
        _addRequirementFromTargetToken(specializedForTarget->targetToken, ioReqs);
    }
}

void DocMarkdownWriter::_writeTargetRequirements(const Requirement* reqs, Index reqsCount)
{
    if (reqsCount == 0)
    {
        return;
    }

     auto& out = m_builder;

    // Okay we need the name of the CodeGen target
    UnownedStringSlice name = TypeTextUtil::getCompileTargetName(SlangCompileTarget(reqs->target));

    out << toSlice("**") << String(name).toUpper() << toSlice("**");

    if (!(reqsCount == 1 && reqs[0].value.getLength() == 0))
    {
        // Put in a list so we can use convenience funcs
        List<String> values;
        for (Index i = 0; i < reqsCount; i++)
        {
            if (reqs[i].value.getLength() > 0)
            {
                values.add(reqs[i].value);
            }
        }

        out << toSlice(" ");
        _appendCommaList(values, '`');
    }

    out << toSlice(" ");
}

void DocMarkdownWriter::_appendRequirements(const List<Requirement>& requirements)
{
    Index startIndex = 0;
    CodeGenTarget curTarget = CodeGenTarget::None;

    for (Index i = 0; i < requirements.getCount(); ++i)
    {
        const auto& req = requirements[i];

        if (req.target != curTarget)
        {
            _writeTargetRequirements(requirements.getBuffer() + startIndex, i - startIndex);

            startIndex = i;
            curTarget = req.target;
        }
    }

    _writeTargetRequirements(requirements.getBuffer() + startIndex, requirements.getCount() - startIndex);
}

void DocMarkdownWriter::_maybeAppendRequirements(const UnownedStringSlice& title, const List<List<DocMarkdownWriter::Requirement>>& uniqueRequirements)
{
     auto& out = m_builder;
     const Index uniqueCount = uniqueRequirements.getCount();

     if (uniqueCount <= 0)
     {
         return;
     }

     if (uniqueCount == 1)
     {
         const auto& reqs = uniqueRequirements[0];

         // If just HLSL on own, then ignore
         if (reqs.getCount() == 0 || (reqs.getCount() == 1 && reqs[0] == Requirement{ CodeGenTarget::HLSL, String() }))
         {
             return;
         }

         out << title;

         _appendRequirements(reqs);
         out << toSlice("\n");
     }
     else
     {
         out << title;

        for (Index i = 0; i < uniqueCount; ++i)
        {
            out << (i + 1) << (". ");
            _appendRequirements(uniqueRequirements[i]);
            out << toSlice("\n");
        }
     }

     out << toSlice("\n");
}

static Decl* _getSameNameDecl(Decl* decl)
{
    auto parentDecl = decl->parentDecl;

    // Sanity check: there should always be a parent declaration.
    //
    SLANG_ASSERT(parentDecl);
    if (!parentDecl) return nullptr;

    // If the declaration is the "inner" declaration of a generic,
    // then we actually want to look one level up, because the
    // peers/siblings of the declaration will belong to the same
    // parent as the generic, not to the generic.
    //
    if (auto genericParentDecl = as<GenericDecl>(parentDecl))
    {
        // Note: we need to check here to be sure `newDecl`
        // is the "inner" declaration and not one of the
        // generic parameters, or else we will end up
        // checking them at the wrong scope.
        //
        if (decl == genericParentDecl->inner)
        {
            decl = parentDecl;
        }
    }

    return decl;
}

static bool _isFirstOverridden(Decl* decl)
{
    decl = _getSameNameDecl(decl);

    ContainerDecl* parentDecl = decl->parentDecl;

    Name* declName = decl->getName();
    if (declName)
    {
        Decl** firstDeclPtr = parentDecl->getMemberDictionary().tryGetValue(declName);
        return (firstDeclPtr && *firstDeclPtr == decl) || (firstDeclPtr == nullptr);
    }

    return false;
}

void DocMarkdownWriter::writeCallableOverridable(const ASTMarkup::Entry& entry, CallableDecl* callableDecl)
{
    auto& out = m_builder;

    writePreamble(entry);

    {
        // Output the overridable path (ie without terminal generic parameters)
        ASTPrinter printer(m_astBuilder);
        printer.addOverridableDeclPath(DeclRef<Decl>(callableDecl));
        // Extract the name
        out << toSlice("# `") << printer.getStringBuilder() << toSlice("`\n\n");
    }

    writeDescription(entry);

    List<CallableDecl*> sigs;
    {
        Decl* sameNameDecl = _getSameNameDecl(callableDecl);

        for (Decl* curDecl = sameNameDecl; curDecl; curDecl = curDecl->nextInContainerWithSameName)
        {
            CallableDecl* sig = nullptr;
            if (GenericDecl* genericDecl = as<GenericDecl>(curDecl))
            {
                sig = as<CallableDecl>(genericDecl->inner);
            }
            else
            {
                sig = as<CallableDecl>(curDecl);
            }
            
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

    // The unique requirements found 
    List<List<Requirement>> uniqueRequirements;
    // Maps a sig index to a unique requirements set
    List<Index> requirementsMap;

    for (Index i = 0; i < sigs.getCount(); ++i)
    {
        CallableDecl* sig = sigs[i];

        // Add the requirements for all the different versions
        List<Requirement> requirements;
        for (CallableDecl* curSig = sig; curSig; curSig = curSig->nextDecl)
        {
            _addRequirements(sig, requirements);
        }

        // TODO(JS): HACK - almost everything is available for HLSL, and is generally not marked up in stdlib
        // So assume here it's available
        _addRequirement(Requirement{ CodeGenTarget::HLSL, String() }, requirements);

        // We want to make the requirements set unique, so we can look it up easily in the uniqueRequirements, so we sort it
        // This also has the benefit of keeping the ordering consistent
        requirements.sort();

        // See if we already have this combination of requirements
        Index uniqueRequirementIndex = uniqueRequirements.indexOf(requirements);
        if (uniqueRequirementIndex < 0)
        {
            // If not add it
            uniqueRequirementIndex = uniqueRequirements.getCount();
            uniqueRequirements.add(requirements);
        }

        // Add the uniqueRequirement index for this sig index
        requirementsMap.add(uniqueRequirementIndex);
    }

    // Output the signature
    {          
        out << toSlice("## Signature \n\n");
        out << toSlice("```\n");

        Index prevRequirementsIndex = -1;
        
        const Int sigCount = sigs.getCount();
        for (Index i = 0; i < sigCount; ++i)
        {            
            auto sig = sigs[i];
            // Get the requirements index for this sig
            const Index requirementsIndex = requirementsMap[i];

            // Output if needs unique requirements
            if (uniqueRequirements.getCount() > 1 )
            {
                if (requirementsIndex != prevRequirementsIndex)
                {
                    out << toSlice("/// See Availability ") << (requirementsIndex + 1) << toSlice("\n");
                    prevRequirementsIndex = requirementsIndex;
                }
            }

            writeSignature(sig);
        }
        out << "```\n\n";
    }

    _maybeAppendRequirements(toSlice("## Availability\n\n"), uniqueRequirements);

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
                _appendAsBullets(_getUniqueParams(genericDecls), '`');
                // And parameters
                _appendAsBullets(_getUniqueParams(paramDecls), '`');

                out << toSlice("\n");
            }
        }
    }
}

void DocMarkdownWriter::writeEnum(const ASTMarkup::Entry& entry, EnumDecl* enumDecl)
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

    _appendAsBullets(_getAsNameAndTextList(enumDecl->getMembersOfType<EnumCaseDecl>()), '_');
    
    writeDescription(entry);
}

void DocMarkdownWriter::_appendEscaped(const UnownedStringSlice& text)
{
    auto& out = m_builder;

    const char* start = text.begin();
    const char* cur = start;
    const char*const end = text.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;

        switch (c)
        {
            case '<':
            case '>':
            case '&':
            case '"':
            case '_':
            {
                // Flush if any before
                if (cur > start)
                {
                    out.append(start, cur);
                }
                // Prefix with the 
                out.appendChar('\\');

                // Start will still include the char, for later flushing
                start = cur;
                break;
            }
            default: break;
        }
    }

    // Flush any remaining
    if (cur > start)
    {
        out.append(start, cur);
    }
}


void DocMarkdownWriter::_appendDerivedFrom(const UnownedStringSlice& prefix, AggTypeDeclBase* aggTypeDecl)
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

void DocMarkdownWriter::_appendAggTypeName(AggTypeDeclBase* aggTypeDecl)
{
    auto& out = m_builder;

    // This could be lots of different things - struct/class/extension/interface/..

    ASTPrinter printer(m_astBuilder);
    printer.addDeclPath(DeclRef<Decl>(aggTypeDecl));

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
}

void DocMarkdownWriter::writeAggType(const ASTMarkup::Entry& entry, AggTypeDeclBase* aggTypeDecl)
{
    writePreamble(entry);

    auto& out = m_builder;

    // We can write out he name using the printer

    ASTPrinter printer(m_astBuilder);
    printer.addDeclPath(DeclRef<Decl>(aggTypeDecl));

    out << toSlice("# `");
    _appendAggTypeName(aggTypeDecl);
    out << toSlice("`\n\n");

    {
        List<InheritanceDecl*> inheritanceDecls;
        _getDecls<InheritanceDecl>(aggTypeDecl, inheritanceDecls);

        if (inheritanceDecls.getCount())
        {
            out << "*Implements:* ";
            _appendCommaList(_getAsStringList(inheritanceDecls), '`');
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
                ASTMarkup::Entry* assocTypeDeclEntry = m_markup->getEntry(assocTypeDecl);
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
                    _appendCommaList(_getAsStringList(inheritanceDecls), '`');
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
            out << toSlice("## Generic Parameters\n\n");
            _appendAsBullets(_getAsNameAndTextList(params), '`');
            out << toSlice("\n");
        }
    }

    {
        List<Decl*> fields;
        _getDeclsOfType<VarDecl>(aggTypeDecl, fields);
        if (fields.getCount())
        {
            out << toSlice("## Fields\n\n");
            _appendAsBullets(_getAsNameAndTextList(fields), '`');
            out << toSlice("\n");
        }
    }

    {
        // Make sure we've got a query-able member dictionary
        auto& memberDict = aggTypeDecl->getMemberDictionary();

        List<Decl*> uniqueMethods;
        for (const auto& [_, decl] : memberDict)
        {
            CallableDecl* callableDecl = as<CallableDecl>(decl);
            if (callableDecl && isVisible(callableDecl))
            {
                uniqueMethods.add(callableDecl);
            }
        }

        if (uniqueMethods.getCount())
        {
            // Put in source definition order
            uniqueMethods.sort([](Decl* a, Decl* b) -> bool { return a->loc.getRaw() < b->loc.getRaw(); });

            out << "## Methods\n\n";
            _appendAsBullets(_getAsStringList(uniqueMethods), '`');
            out << toSlice("\n");
        }
    }
}

void DocMarkdownWriter::writePreamble(const ASTMarkup::Entry& entry)
{
    SLANG_UNUSED(entry);
    auto& out = m_builder;

    //out << toSlice("\n");

    out.appendRepeatedChar('-', 80);
    out << toSlice("\n");
}

void DocMarkdownWriter::writeDescription(const ASTMarkup::Entry& entry)
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

void DocMarkdownWriter::writeDecl(const ASTMarkup::Entry& entry, Decl* decl)
{
    // Skip these they will be output as part of their respective 'containers'
    if (as<ParamDecl>(decl) || as<EnumCaseDecl>(decl) || as<AssocTypeDecl>(decl) || as<InheritanceDecl>(decl) || as<ThisTypeDecl>(decl))
    {
        return; 
    }

    if (CallableDecl* callableDecl = as<CallableDecl>(decl))
    {
        if (_isFirstOverridden(callableDecl))
        {
             writeCallableOverridable(entry, callableDecl);
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

bool DocMarkdownWriter::isVisible(const Name* name)
{
    return name == nullptr || !name->text.startsWith(toSlice("__"));
}

bool DocMarkdownWriter::isVisible(const ASTMarkup::Entry& entry)
{
    // For now if it's not public it's not visible
    if (entry.m_visibility != MarkupVisibility::Public)
    {
        return false;
    }

    Decl* decl = as<Decl>(entry.m_node);
    return decl == nullptr || isVisible(decl->getName());
}

bool DocMarkdownWriter::isVisible(Decl* decl)
{
    if (!isVisible(decl->getName()))
    {
        return false;
    }

    auto entry = m_markup->getEntry(decl);
    return entry == nullptr || entry->m_visibility == MarkupVisibility::Public;
}

void DocMarkdownWriter::writeAll()
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

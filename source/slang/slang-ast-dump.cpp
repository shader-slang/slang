// slang-ast-dump.cpp
#include "slang-ast-dump.h"
#include <assert.h>
#include <limits>

#include "slang-compiler.h"

#include "../core/slang-string.h"

#include "slang-generated-ast-macro.h"

namespace Slang {


struct ASTDumpContext
{
    struct ObjectInfo
    {
        const ReflectClassInfo* m_typeInfo;
        NodeBase* m_object;
        bool m_isDumped;
    };

    struct ScopeWrite
    {
        ScopeWrite(ASTDumpContext* context):
            m_context(context)
        {
            if (m_context->m_scopeWriteCount == 0)
            {
                m_context->m_buf.clear();
            }
            m_context->m_scopeWriteCount++;
        }

        ~ScopeWrite()
        {
            if (--m_context->m_scopeWriteCount == 0)
            {
                m_context->m_writer->emit(m_context->m_buf);
            }
        }

        StringBuilder& getBuf() { return m_context->m_buf; }

        operator StringBuilder&() { return m_context->m_buf; }

        ASTDumpContext* m_context;
    };

    void dumpObject(const ReflectClassInfo& type, NodeBase* obj);

    void dumpObjectFull(const ReflectClassInfo& type, NodeBase* obj, Index objIndex);
    void dumpObjectReference(const ReflectClassInfo& type, NodeBase* obj, Index objIndex);

    void dump(NodeBase* node)
    {
        if (node == nullptr)
        {
            _dumpPtr(nullptr);
        }
        else
        {
            dumpObject(node->getClassInfo(), node);
        }
    }

    void dump(const Name* name)
    {
        if (name == nullptr)
        {
            _dumpPtr(nullptr);
        }
        else
        {
            dump(name->text);
        }
    }

    void _appendPtr(const void* ptr, StringBuilder& out)
    {
        if (ptr == nullptr)
        {
            out << "null";
            return;
        }

        char* start = out.prepareForAppend(sizeof(ptr) * 2 + 2);
        char* dst = start;

        *dst++ = '0';
        *dst++ = 'x';

        size_t v = size_t(ptr);

        for (Index i = (sizeof(ptr) * 2) - 1; i >= 0; --i)
        {
            dst[i] = _getHexDigit(UInt32(v) & 0xf);
            v >>= 4;
        }
        dst += sizeof(ptr) * 2;
        out.appendInPlace(start, Index(dst - start));
    }

    void _dumpPtr(const UnownedStringSlice& prefix, const void* ptr)
    {
        ScopeWrite scope(this);
        StringBuilder& buf = scope.getBuf();
        buf << prefix;
        _appendPtr(ptr, buf);
    }

    void _dumpPtr(const void* ptr)
    {
        ScopeWrite scope(this);
        StringBuilder& buf = scope.getBuf();
        if (ptr)
        {
            _appendPtr(ptr, buf);
        }
        else
        {
            buf << "null";
        }
    }

    void dump(const Scope* scope)
    {
        if (m_dumpFlags & ASTDumpUtil::Flag::HideScope)
        {
            return;
        }

        if (scope == nullptr)
        {
            _dumpPtr(nullptr);
        }
        else
        {
            ScopeWrite writeScope(this);
            StringBuilder& buf = writeScope.getBuf();

            List<const Scope*> scopes;
            const Scope* cur = scope;
            while (cur)
            {
                scopes.add(cur);
                cur = cur->parent;
            }

            for (Index i = scopes.getCount() - 1; i >= 0 ; --i)
            {
                buf << "::";
                const Scope* curScope = scopes[i];

                auto containerDecl = curScope->containerDecl;

                Name* name = containerDecl ? containerDecl->getName() : nullptr;

                if (name)
                {
                    buf << name->text;
                }
                else
                {
                    buf << "?";
                }
            }
        }
    }

    void dump(const RefObject* obj)
    {
        if (obj == nullptr)
        {
            _dumpPtr(nullptr);
        }
        else
        {
            // We don't know what this is!
            _dumpPtr(UnownedStringSlice::fromLiteral("Unknown@"), obj);
        }
    }

    template <typename T>
    void dump(const List<T>& list)
    {
        m_writer->emit(" { \n");
        m_writer->indent();
        for (Index i = 0; i < list.getCount(); ++i)
        {
            dump(list[i]);
            if (i < list.getCount() - 1)
            {
                m_writer->emit(",\n");
            }
            else
            {
                m_writer->emit("\n");
            }
        }
        m_writer->dedent();
        m_writer->emit("}");
    }

    void dump(SourceLoc sourceLoc)
    {
        if (m_dumpFlags & ASTDumpUtil::Flag::HideSourceLoc)
        {
            ScopeWrite(this).getBuf() << "SourceLoc(0)";
            return;
        }

        SourceManager* manager = m_writer->getSourceManager();

        {
            ScopeWrite(this).getBuf() << "SourceLoc(" << sourceLoc.getRaw() << ")";
        }

        if (manager && sourceLoc.isValid())
        {
            HumaneSourceLoc humaneLoc = manager->getHumaneLoc(sourceLoc);
            ScopeWrite(this).getBuf() << " " << humaneLoc.pathInfo.foundPath << ":" << humaneLoc.line;   
        }
    }

    static char _getHexDigit(UInt32 v)
    {
        return (v < 10) ? char(v + '0') : char('a' + v - 10);
    }

    static bool _charNeedsEscaping(char c)
    {
        // Escape everything except the printable characters (except DEL)
        return c < 0x20 || c >= 0x7F;
    }

    void dump(const UnownedStringSlice& slice)
    {
        
        ScopeWrite scope(this);
        auto& buf = scope.getBuf();
        buf.appendChar('\"');
        for (const char c : slice)
        {
            if (!_charNeedsEscaping(c))
            {
                buf << c;
            }
            else
            {
                buf << "\\0x" <<  _getHexDigit(UInt32(c) >> 4) << _getHexDigit(c & 0xf);
            }
        }
        buf.appendChar('\"');
    }
    
    void dump(const Token& token)
    {
        ScopeWrite(this).getBuf() << " { " << TokenTypeToString(token.type) << ", ";
        dump(token.loc);
        m_writer->emit(", ");
        dump(token.getContent());
        m_writer->emit(" }");
    }

    Index getObjectIndex(const ReflectClassInfo& typeInfo, NodeBase* obj)
    {
        Index* indexPtr = m_objectMap.tryGetValueOrAdd(obj, m_objects.getCount());
        if (indexPtr)
        {
            return *indexPtr;
        }

        ObjectInfo info;
        info.m_isDumped = false;
        info.m_object = obj;
        info.m_typeInfo = &typeInfo;

        m_objects.add(info);
        return m_objects.getCount() - 1;
    }

    void dump(uint32_t v)
    {
        m_writer->emit(UInt(v));
    }
    void dump(int32_t v)
    {
        m_writer->emit(v);
    }
    void dump(FloatingPointLiteralValue v)
    {
        m_writer->emit(v);
    }

    void dump(IntegerLiteralValue v)
    {
        m_writer->emit(v);
    }
    

    void dump(const SemanticVersion& version)
    {
        ScopeWrite(this).getBuf() << UInt(version.m_major) << "." << UInt(version.m_minor) << "." << UInt(version.m_patch);
    }
    void dump(const NameLoc& nameLoc)
    {
        m_writer->emit("NameLoc{");
        if (nameLoc.name)
        {
            dump(nameLoc.name->text.getUnownedSlice());
        }
        else
        {
            _dumpPtr(nullptr);
        }
        m_writer->emit(", ");
        dump(nameLoc.loc);
        m_writer->emit(" }");
    }
    void dump(BaseType baseType)
    {
        m_writer->emit(BaseTypeInfo::asText(baseType));
    }
    void dump(Stage stage)
    {
        m_writer->emit(getStageName(stage));
    }
    void dump(ImageFormat imageFormat)
    {
        m_writer->emit(getGLSLNameForImageFormat(imageFormat));
    }
    void dump(TryClauseType clauseType)
    {
        m_writer->emit(getTryClauseTypeName(clauseType));
    }
    void dump(BuiltinRequirementKind kind)
    {
        m_writer->emit((int)kind);
    }
    void dump(MarkupVisibility v)
    {
        m_writer->emit((int)v);
    }
    void dump(TypeTag tag)
    {
        m_writer->emit((int)tag);
    }
    void dump(const String& string)
    {
        dump(string.getUnownedSlice());
    }

    void dump(const DiagnosticInfo* info)
    {
        ScopeWrite(this).getBuf() << "DiagnosticInfo {" << info->id << "}";
    }
    void dump(const Layout* layout)
    {
        _dumpPtr(UnownedStringSlice::fromLiteral("Layout@"), layout);
    }

    void dump(const Modifiers& modifiers)
    {
        auto& nonConstModifiers = const_cast<Modifiers&>(modifiers);

        m_writer->emit(" { \n");
        m_writer->indent();

        for (const auto& mod : nonConstModifiers)
        {
            dump(mod);
            m_writer->emit("\n");
        }

        m_writer->dedent();
        m_writer->emit("}");
    }

    template <typename T>
    void dump(const SyntaxClass<T>& cls)
    {
        m_writer->emit(cls.classInfo->m_name);
    }

    template <typename KEY, typename VALUE>
    void dump(const Dictionary<KEY, VALUE>& dict)
    {
        m_writer->emit(" { \n");
        m_writer->indent();

        for (const auto& [key, value] : dict)
        {
            dump(key);
            m_writer->emit(" : ");
            dump(value);

            m_writer->emit("\n");
        }

        m_writer->dedent();
        m_writer->emit("}");
    }

    template <typename KEY, typename VALUE>
    void dump(const OrderedDictionary<KEY, VALUE>& dict)
    {
        m_writer->emit(" { \n");
        m_writer->indent();

        for (auto iter : dict)
        {
            const auto& key = iter.key;
            const auto& value = iter.value;

            dump(key);
            m_writer->emit(" : ");
            dump(value);

            m_writer->emit("\n");
        }

        m_writer->dedent();
        m_writer->emit("}");
    }

    void dump(DeclRefBase* declRef)
    {
        StringBuilder sb;
        sb << declRef;
        m_writer->emit(sb.toString());
    }

    void dump(const DeclCheckStateExt& extState)
    {
        auto state = extState.getState();
      
        ScopeWrite(this).getBuf() << "DeclCheckStateExt{" << extState.isBeingChecked() << ", " << Index(state) << "}";
    }

    void dump(FeedbackType::Kind kind)
    {
        m_buf.clear();
        const char* name = nullptr;
        switch (kind)
        {
            case FeedbackType::Kind::MinMip: name = "MinMip"; break;
            case FeedbackType::Kind::MipRegionUsed: name = "MipRegionUsed"; break;
        }

        m_buf << "FeedbackType::Kind{" << name << "}";
        m_writer->emit(m_buf);
    }


    void dump(SamplerStateFlavor flavor)
    {
        switch (flavor)
        {
            case SamplerStateFlavor::SamplerState: m_writer->emit("sampler"); break;
            case SamplerStateFlavor::SamplerComparisonState: m_writer->emit("samplerComparison"); break;
            default: m_writer->emit("unknown"); break;
        }
    }

    void dump(const QualType& qualType)
    {
        if (qualType.isLeftValue)
        {
            m_writer->emit("lvalue ");
        }
        else
        {
            m_writer->emit("rvalue ");
        }
        dump(qualType.type);
    }

    void dump(SyntaxParseCallback callback) { _dumpPtr(UnownedStringSlice::fromLiteral("SyntaxParseCallback"), (const void*)callback); }

    template <typename T, int SIZE>
    void dump(const T (&in)[SIZE])
    {
        m_writer->emit(" { \n");
        m_writer->indent();

        for (Index i = 0; i < Index(SIZE); ++i)
        {
            dump(in[i]);
            if (i < Index(SIZE) - 1)
            {
                m_writer->emit(", ");
            }
            m_writer->emit("\n");
        }

        m_writer->dedent();
        m_writer->emit("}");
    }

    void dump(const MatrixCoord& coord)
    {
        m_writer->emit("(");
        m_writer->emit(coord.row);
        m_writer->emit(", ");
        m_writer->emit(coord.col);
        m_writer->emit(")\n");
    }

    void dump(const LookupResult& result)
    {
        auto& nonConstResult = const_cast<LookupResult&>(result);

        m_writer->emit(" { \n");
        m_writer->indent();

        for (auto item : nonConstResult)
        {
            // TODO(JS):
            m_writer->emit("...\n");
        }

        m_writer->dedent();
        m_writer->emit("}");
    }
    void dump(const TypeExp& exp)
    {
        m_writer->emit(" { \n");
        m_writer->indent();

        dump(exp.exp);
        m_writer->emit(",\n");
        dump(exp.type);
        m_writer->emit("\n");

        m_writer->dedent();
        m_writer->emit("}");
    }
    void dump(const ExpandedSpecializationArg& arg)
    {
        dump(arg.witness);
    }

    void dump(const TransparentMemberInfo& memInfo)
    {
        dump(memInfo.decl);
    }

    void dumpRemaining()
    {
        // Have to keep checking count, as dumping objects can add objects
        for (Index i = 0; i < m_objects.getCount(); ++i)
        {
            ObjectInfo& info = m_objects[i];
            if (!info.m_isDumped)
            {
                dumpObjectFull(*info.m_typeInfo, info.m_object, i);
            }
        }
    }

    void dump(ContainerDecl* decl)
    {
        if (auto moduleDecl = dynamicCast<ModuleDecl>(decl))
        {
            // Lets special case handling of module decls -> we only want to output as references
            // otherwise we end up dumping everything in every module.

            const ReflectClassInfo& typeInfo = moduleDecl->getClassInfo();
            Index index = getObjectIndex(typeInfo, moduleDecl);

            // We don't want to fully dump, referenced modules as doing so dumps everything
            m_objects[index].m_isDumped = true;

            dumpObjectReference(typeInfo, moduleDecl, index);
        }
        else
        {
            dump(static_cast<NodeBase*>(decl));
        }
    }

    template <typename T>
    void dumpField(const char* name, const T& value)
    {
        m_writer->emit(name);
        m_writer->emit(" : ");
        dump(value);
        m_writer->emit("\n");
    }

    template<int N>
    void dump(const ShortList<ValNodeOperand, N>& operands)
    {
        m_writer->emit("(");
        bool isFirst = true;
        for (auto operand : operands)
        {
            if (!isFirst)
            {
                m_writer->emit(", ");
            }
            isFirst = false;
            dumpField("operand", operand);
        }

        m_writer->emit(")");
    }

    void dump(ValNodeOperand operand)
    {
        switch (operand.kind)
        {
        case ValNodeOperandKind::ConstantValue:
            dump(operand.values.intOperand);
            break;
        case ValNodeOperandKind::ValNode:
            dump(operand.values.nodeOperand);
            break;
        case ValNodeOperandKind::ASTNode:
            dump(operand.values.nodeOperand);
            break;
        }
    }

    void dump(ASTNodeType nodeType)
    {
        // Get the class
        auto info = ASTClassInfo::getInfo(nodeType);
        // Write the name
        m_writer->emit(info->m_name);
    }

    void dump(KeyValuePair<DeclRefBase*, SubtypeWitness*> pair)
    {
        m_writer->emit("(");
        dump(pair.key);
        m_writer->emit(", ");
        dump(pair.value);
        m_writer->emit(")");
    }

    void dump(const SPIRVAsmOperand& operand)
    {
        switch(operand.flavor)
        {
        case SPIRVAsmOperand::Id:
            m_writer->emit("%");
            break;
        case SPIRVAsmOperand::ResultMarker:
            m_writer->emit("result");
            break;
        case SPIRVAsmOperand::Literal:
        case SPIRVAsmOperand::NamedValue:
            break;
        case SPIRVAsmOperand::SlangValue:
            m_writer->emit("$");
            break;
        case SPIRVAsmOperand::SlangValueAddr:
            m_writer->emit("&");
            break;
        case SPIRVAsmOperand::SlangType:
            m_writer->emit("$$");
            break;
        case SPIRVAsmOperand::SlangImmediateValue:
            m_writer->emit("!");
            break;
        case SPIRVAsmOperand::BuiltinVar:
            m_writer->emit("builtin");
            break;
        default:
            SLANG_UNREACHABLE("Unhandled case in ast dump for SPIRVAsmOperand");
        }
        if(operand.expr)
            dump(operand.expr);
        else
            dump(operand.token);
    }

    void dump(const SPIRVAsmInst& inst)
    {
        dump(inst.opcode);
        for(const auto& o : inst.operands)
            dump(o);
    }

    void dump(const SPIRVAsmExpr& expr)
    {
        m_writer->emit("spirv_asm\n");
        m_writer->emit("{\n");
        m_writer->indent();
        for(const auto& i : expr.insts)
        {
            dump(i);
            m_writer->emit(";\n");
        }
        m_writer->dedent();
        m_writer->emit("}");
    }

    void dump(const CapabilitySet& capSet)
    {
        m_writer->emit("capability_set(");
        bool isFirstSet = true;
        for (auto& set : capSet.getExpandedAtoms())
        {
            if (!isFirstSet)
            {
                m_writer->emit(" | ");
            }
            bool isFirst = true;
            for (auto atom : set.getExpandedAtoms())
            {
                if (!isFirst)
                {
                    m_writer->emit("+");
                }
                dump(capabilityNameToString((CapabilityName)atom));
                isFirst = false;
            }
            isFirstSet = false;
        }
        m_writer->emit(")");
    }

    void dumpObjectFull(NodeBase* node);

    ASTDumpContext(SourceWriter* writer, ASTDumpUtil::Flags flags, ASTDumpUtil::Style dumpStyle):
        m_writer(writer),
        m_scopeWriteCount(0),
        m_dumpStyle(dumpStyle),
        m_dumpFlags(flags)
    {
    }

    ASTDumpUtil::Flags m_dumpFlags;
    ASTDumpUtil::Style m_dumpStyle;

    Index m_scopeWriteCount;

    // Using the SourceWriter, for automatic indentation.
    SourceWriter* m_writer;

    Dictionary<NodeBase*, Index> m_objectMap;  ///< Object index
    List<ObjectInfo> m_objects;

    StringBuilder m_buf;
};

// Lets generate functions one for each that attempts to write out *it's* fields.
// We can write out the Super types fields by looking that up

struct ASTDumpAccess
{

#define SLANG_AST_DUMP_FIELD(FIELD_NAME, TYPE, param) context.dumpField(#FIELD_NAME, static_cast<param*>(base)->FIELD_NAME); 

#define SLANG_AST_DUMP_FIELDS_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
case ASTNodeType::NAME: \
{ \
    SLANG_FIELDS_ASTNode_##NAME(SLANG_AST_DUMP_FIELD, NAME) \
    break; \
}

    static void dump(ASTNodeType type, NodeBase* base, ASTDumpContext& context)
    {
        switch (type)
        {
            SLANG_ALL_ASTNode_NodeBase(SLANG_AST_DUMP_FIELDS_IMPL, _)
            default: break;
        }
    }
};

void ASTDumpContext::dumpObjectReference(const ReflectClassInfo& type, NodeBase* obj, Index objIndex)
{
    SLANG_UNUSED(obj);
    ScopeWrite(this).getBuf() << type.m_name << ":" << objIndex;
}

void ASTDumpContext::dumpObjectFull(const ReflectClassInfo& type, NodeBase* obj, Index objIndex)
{
    ObjectInfo& info = m_objects[objIndex];
    SLANG_ASSERT(info.m_isDumped == false);
    info.m_isDumped = true;

    // We need to dump the fields.

    ScopeWrite(this).getBuf() << type.m_name << ":" << objIndex << " {\n";
    m_writer->indent();

    List<const ReflectClassInfo*> allTypes;
    {
        const ReflectClassInfo* curType = &type;
        do
        {
            allTypes.add(curType);
            curType = curType->m_superClass;
        } while (curType);
    }

    // Okay we go backwards so we output in the 'normal' order
    for (Index i = allTypes.getCount() - 1; i >= 0; --i)
    {
        const ReflectClassInfo* curType = allTypes[i];
        ASTDumpAccess::dump(ASTNodeType(curType->m_classId), obj, *this);
    }

    m_writer->dedent();
    m_writer->emit("}\n");
}

void ASTDumpContext::dumpObject(const ReflectClassInfo& typeInfo, NodeBase* obj)
{
    Index index = getObjectIndex(typeInfo, obj);

    ObjectInfo& info = m_objects[index];
    if (info.m_isDumped || m_dumpStyle == ASTDumpUtil::Style::Flat)
    {
        dumpObjectReference(typeInfo, obj, index);
    }
    else
    {
        dumpObjectFull(typeInfo, obj, index);
    }
}

void ASTDumpContext::dumpObjectFull(NodeBase* node)
{
    if (!node)
    {
        _dumpPtr(nullptr);
    }
    else
    {
        const ReflectClassInfo& typeInfo = node->getClassInfo();
        Index index = getObjectIndex(typeInfo, node);
        dumpObjectFull(typeInfo, node, index);
    }
}

/* static */void ASTDumpUtil::dump(NodeBase* node, Style style, Flags flags, SourceWriter* writer)
{
    ASTDumpContext context(writer, flags, style);
    context.dumpObjectFull(node);
    context.dumpRemaining();
}

} // namespace Slang

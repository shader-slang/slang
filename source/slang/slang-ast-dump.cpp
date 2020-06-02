// slang-ast-dump.cpp
#include "slang-ast-dump.h"
#include <assert.h>

#include "slang-compiler.h"

#include "../core/slang-string.h"

#include "slang-ast-generated-macro.h"

namespace Slang {

namespace { // anonymous

struct Context
{
    struct ObjectInfo
    {
        const ReflectClassInfo* m_typeInfo;
        RefObject* m_object;
        bool m_isDumped;
    };

    struct ScopeWrite
    {
        ScopeWrite(Context* context):
            m_context(context)
        {
            if (m_context->m_scopeWriteCount == 0)
            {
                m_context->m_buf.Clear();
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

        Context* m_context;
    };

    void dumpObject(const ReflectClassInfo& type, RefObject* obj);

    void dumpObjectFull(const ReflectClassInfo& type, RefObject* obj, Index objIndex);
    void dumpObjectReference(const ReflectClassInfo& type, RefObject* obj, Index objIndex);

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

    void dump(Substitutions* subs)
    {
        if (subs == nullptr)
        {
            _dumpPtr(nullptr);
        }
        else
        {
            dumpObject(subs->getClassInfo(), subs);
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

    static bool _isPrintableChar(char c)
    {
        return c >= 0x20 && c < 0x80;
    }

    void dump(const UnownedStringSlice& slice)
    {
        
        ScopeWrite scope(this);
        auto& buf = scope.getBuf();
        buf.appendChar('\"');
        for (const char c : slice)
        {
            if (_isPrintableChar(c))
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
        m_writer->emit(" ");
        dump(token.getContent());
        m_writer->emit(" }");
    }

    Index getObjectIndex(const ReflectClassInfo& typeInfo, RefObject* obj)
    {
        Index* indexPtr = m_objectMap.TryGetValueOrAdd(obj, m_objects.getCount());
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

        for (auto iter : dict)
        {
            const auto& key = iter.Key;
            const auto& value = iter.Value;

            dump(key);
            m_writer->emit(" : ");
            dump(value);

            m_writer->emit("\n");
        }

        m_writer->dedent();
        m_writer->emit("}");
    }

    void dump(const DeclCheckStateExt& extState)
    {
        auto state = extState.getState();
      
        ScopeWrite(this).getBuf() << "DeclCheckStateExt{" << extState.isBeingChecked() << ", " << Index(state) << "}";
    }

    void dump(TextureFlavor texFlavor)
    {
        m_buf.Clear();
        m_buf << "TextureFlavor{" << Index(texFlavor.flavor) << "}";
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
            m_writer->emit("left ");
        }
        else
        {
            m_writer->emit("right ");
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
    void dump(const GlobalGenericParamSubstitution::ConstraintArg& arg)
    {
        m_writer->emit(" { \n");
        m_writer->indent();

        dump(arg.decl);
        m_writer->emit(",\n");
        dump(arg.val);
        m_writer->emit("\n");

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

    void dump(ASTNodeType nodeType)
    {
        SLANG_UNUSED(nodeType)
        // Don't bother to output anything - as will already have been dumped with the object name
    }

    void dumpObjectFull(NodeBase* node);

    Context(SourceWriter* writer, ASTDumpUtil::Style dumpStyle):
        m_writer(writer),
        m_scopeWriteCount(0),
        m_dumpStyle(dumpStyle)
    {
    }

    ASTDumpUtil::Style m_dumpStyle;

    Index m_scopeWriteCount;

    // Using the SourceWriter, for automatic indentation.
    SourceWriter* m_writer;

    Dictionary<RefObject*, Index> m_objectMap;  ///< Object index
    List<ObjectInfo> m_objects;

    StringBuilder m_buf;
};

} // anonymous

// Lets generate functions one for each that attempts to write out *it's* fields.
// We can write out the Super types fields by looking that up

struct ASTDumpAccess
{
#define SLANG_AST_DUMP_FIELD(FIELD_NAME, TYPE, param) context.dumpField(#FIELD_NAME, node->FIELD_NAME); 

#define SLANG_AST_DUMP_FIELDS_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
static void dumpFields_##NAME(NAME* node, Context& context) \
{ \
    SLANG_UNUSED(node); \
    SLANG_UNUSED(context); \
    SLANG_FIELDS_ASTNode_##NAME(SLANG_AST_DUMP_FIELD, _) \
}

SLANG_ALL_ASTNode_NodeBase(SLANG_AST_DUMP_FIELDS_IMPL, _)

};

#define SLANG_AST_GET_DUMP_FUNC(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) m_funcs[Index(ASTNodeType::NAME)] = (DumpFieldsFunc)&ASTDumpAccess::dumpFields_##NAME;

typedef void (*DumpFieldsFunc)(RefObject* obj, Context& context);

struct DumpFieldFuncs
{
    DumpFieldFuncs()
    {
        memset(m_funcs, 0, sizeof(m_funcs));
        SLANG_ALL_ASTNode_NodeBase(SLANG_AST_GET_DUMP_FUNC, _)
    }

    DumpFieldsFunc m_funcs[Index(ASTNodeType::CountOf)];
};

static const DumpFieldFuncs s_funcs;

void Context::dumpObjectReference(const ReflectClassInfo& type, RefObject* obj, Index objIndex)
{
    SLANG_UNUSED(obj);
    ScopeWrite(this).getBuf() << type.m_name << ":" << objIndex;
}

void Context::dumpObjectFull(const ReflectClassInfo& type, RefObject* obj, Index objIndex)
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
        DumpFieldsFunc func = s_funcs.m_funcs[Index(curType->m_classId)];
        if (func)
        {
            func(obj, *this);
        }
    }

    m_writer->dedent();
    m_writer->emit("}\n");
}

void Context::dumpObject(const ReflectClassInfo& typeInfo, RefObject* obj)
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

void Context::dumpObjectFull(NodeBase* node)
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

/* static */void ASTDumpUtil::dump(NodeBase* node, Style style, SourceWriter* writer)
{
    Context context(writer, style);
    context.dumpObjectFull(node);
    context.dumpRemaining();
}

} // namespace Slang

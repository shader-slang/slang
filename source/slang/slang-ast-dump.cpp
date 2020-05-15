// slang-ast-dump.cpp
#include "slang-ast-dump.h"
#include <assert.h>

#include "slang-compiler.h"

#include "../core/slang-string.h"

#include "slang-ast-generated-macro.h"

// There could be an argument for using the SourceWriter, but whilst that has some useful stuff
// like the automatic indentation, it is a little clumsy in some other ways, like how to build up
// a result via << operators is not available.
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
            dumpPtr(nullptr);
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
            dumpPtr(nullptr);
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
            dumpPtr(nullptr);
        }
        else
        {
            dump(name->text);
        }
    }

    void dump(const RefObject* obj)
    {
        if (obj == nullptr)
        {
            dumpPtr(nullptr);
        }
        else
        {
            // We don't know what this is!
            ScopeWrite(this).getBuf() << "Unknown@" << size_t(obj);
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

    static const char _getHexDigit(UInt v)
    {
        return (v < 10) ? char(v + '0') : char('a' + v - 10);
    }

    void dump(const UnownedStringSlice& slice)
    {
        m_writer->emitChar('\"');

        {
            ScopeWrite scope(this);
            auto& buf = scope.getBuf();
            for (const char c : slice)
            {
                if (c < 0x20 || c >= 0x80)
                {
                    buf << "\\0x" <<  _getHexDigit(UInt32(c) >> 4) << _getHexDigit(c & 0xf);
                }
                else
                {
                    buf << c;
                }
            }
        }
        m_writer->emitChar('\"');
    }
    
    void dump(const Token& token)
    {
        ScopeWrite(this).getBuf() << " { " << TokenTypeToString(token.type) << ", ";
        dump(token.loc);
        dump(token.Content);
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

    void dump(Index v)
    {
        m_writer->emit(v);
    }

    void dump(int32_t v)
    {
        m_writer->emit(v);
    }
    void dump(FloatingPointLiteralValue v)
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
            dumpPtr(nullptr);
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
        ScopeWrite(this).getBuf() << "Layout@" << size_t(layout); 
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
        if (qualType.IsLeftValue)
        {
            m_writer->emit("left ");
        }
        else
        {
            m_writer->emit("right ");
        }
        dump(qualType.type);
    }

    void dumpPtr(const void* ptr)
    {
        if (ptr)
        {
            ScopeWrite(this).getBuf() << "Unknown@" << size_t(ptr);
        }
        else
        {
            m_writer->emit("null");
        }
    }

    void dump(SyntaxParseCallback callback) { dumpPtr((const void*)callback); }

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

    //void dump(const void* ptr) { dumpPtr(ptr); }

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

    template <typename T>
    void dumpField(const char* name, const T& value)
    {
        m_writer->emit(name);
        m_writer->emit(" : ");
        dump(value);
        m_writer->emit("\n");
    }

    void dumpObjectFull(NodeBase* node);
    void dumpObjectFull(Substitutions* subs);

    Context(SourceWriter* writer, ASTDumpUtil::Style dumpStyle):
        m_writer(writer),
        m_scopeWriteCount(0),
        m_dumpStyle(dumpStyle)
    {
    }

    ASTDumpUtil::Style m_dumpStyle;

    Index m_scopeWriteCount;

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

SLANG_ALL_ASTNode_Substitutions(SLANG_AST_DUMP_FIELDS_IMPL, _)
SLANG_ALL_ASTNode_NodeBase(SLANG_AST_DUMP_FIELDS_IMPL, _)

};

#define SLANG_AST_GET_DUMP_FUNC(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) m_funcs[Index(ASTNodeType::NAME)] = (DumpFieldsFunc)&ASTDumpAccess::dumpFields_##NAME;

typedef void (*DumpFieldsFunc)(RefObject* obj, Context& context);

struct DumpFieldFuncs
{
    DumpFieldFuncs()
    {
        memset(m_funcs, 0, sizeof(m_funcs));
        SLANG_ALL_ASTNode_Substitutions(SLANG_AST_GET_DUMP_FUNC, _)
        SLANG_ALL_ASTNode_NodeBase(SLANG_AST_GET_DUMP_FUNC, _)
    }

    DumpFieldsFunc m_funcs[Index(ASTNodeType::CountOf)];
};

static const DumpFieldFuncs s_funcs;

void Context::dumpObjectReference(const ReflectClassInfo& type, RefObject* obj, Index objIndex)
{
    SLANG_UNUSED(obj);
    ScopeWrite(this).getBuf() << type.m_name << "@" << objIndex;
}

void Context::dumpObjectFull(const ReflectClassInfo& type, RefObject* obj, Index objIndex)
{
    ObjectInfo& info = m_objects[objIndex];
    SLANG_ASSERT(info.m_isDumped == false);
    info.m_isDumped = true;

    // We need to dump the fields.

    ScopeWrite(this).getBuf() << type.m_name << "(" << objIndex << ") {\n";
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
        dumpPtr(nullptr);
    }
    else
    {
        const ReflectClassInfo& typeInfo = node->getClassInfo();
        Index index = getObjectIndex(typeInfo, node);
        dumpObjectFull(typeInfo, node, index);
    }
}

void Context::dumpObjectFull(Substitutions* subs)
{
    if (!subs)
    {
        dumpPtr(nullptr);
    }
    else
    {
        const ReflectClassInfo& typeInfo = subs->getClassInfo();
        Index index = getObjectIndex(typeInfo, subs);
        dumpObjectFull(typeInfo, subs, index);
    }
}

/* static */void ASTDumpUtil::dump(NodeBase* node, Style style, SourceWriter* writer)
{
    Context context(writer, style);
    context.dumpObjectFull(node);
    context.dumpRemaining();
}

/* static */void ASTDumpUtil::dump(Substitutions* subs, Style style, SourceWriter* writer)
{
    Context context(writer, style);
    context.dumpObjectFull(subs);
    context.dumpRemaining();

}

} // namespace Slang

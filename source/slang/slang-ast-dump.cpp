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
        RefObject* m_object;
        bool m_isDumped;
    };


    void dumpObject(const ReflectClassInfo& type, RefObject* obj);

    void dump(NodeBase* node)
    {
        if (node == nullptr)
        {
            m_builder << "null";
        }
        else
        {
            dump(node->getClassInfo(), node);
        }
    }

    void dump(Substitutions* subs)
    {
        if (subs == nullptr)
        {
            m_builder << "null";
        }
        else
        {
            dump(subs->getClassInfo(), subs);
        }
    }

    void dump(const RefObject* obj)
    {
        if (obj == nullptr)
        {
            m_builder << "null";
        }
        else
        {
            // We don't know what this is!
            m_builder << "Unknown@";
            m_builder << size_t(obj);
        }
    }

    void dump(const ReflectClassInfo& type, RefObject* obj)
    {
        const Index index = getObjectIndex(obj);
        const auto& info = m_objects[index];
        if (info.m_isDumped)
        {
            // Dump the type
            m_builder << type.m_name << "@" << index;
        }
        else
        {
            // Recurse to it's fields
            dumpObject(type, obj);
        }
    }

    template <typename T>
    void dump(const List<T>& list)
    {
        m_builder << " { \n";
        indent();
        for (Index i = 0; i < list.getCount(); ++i)
        {
            dump(list[i]);
            if (i < list.getCount() - 1)
            {
                m_builder << ",\n";
            }
            else
            {
                m_builder << "\n";
            }
        }
        dedent();
        m_builder << "}";
    }

    void dump(SourceLoc sourceLoc)
    {
        m_builder << "SourceLoc(" << sourceLoc.getRaw() << ")";
    }

    static const char _getHexDigit(UInt v)
    {
        return (v < 10) ? char(v + '0') : char('a' + v - 10);
    }

    void dump(const UnownedStringSlice& slice)
    {
        m_builder << "\"";

        for (const char c : slice)
        {
            if (c < 0x20 || c >= 0x80)
            {
                m_builder << "\\0x" <<  _getHexDigit(UInt32(c) >> 4) << _getHexDigit(c & 0xf);
            }
            else
            {
                m_builder << c;
            }
        }
        m_builder << "\"";
    }
    
    void dump(const Token& token)
    {
        m_builder << " { " << TokenTypeToString(token.type) << ", ";
        dump(token.loc);
        dump(token.Content);
        m_builder << " }";
    }

    Index getObjectIndex(RefObject* obj)
    {
        Index* indexPtr = m_objectMap.TryGetValueOrAdd(obj, m_objects.getCount());
        if (indexPtr)
        {
            return *indexPtr;
        }

        ObjectInfo info;
        info.m_isDumped = false;
        info.m_object = obj;

        m_objects.add(info);
        return m_objects.getCount() - 1;
    }

    void dump(uint32_t v)
    {
        m_builder << v;
    }

    void dump(Index v)
    {
        m_builder << v;
    }

    void dump(int32_t v)
    {
        m_builder << v;
    }
    void dump(FloatingPointLiteralValue v)
    {
        m_builder << v;
    }


    void dump(const SemanticVersion& version)
    {
        m_builder << UInt(version.m_major) << "." << UInt(version.m_minor) << "." << UInt(version.m_patch);
    }
    void dump(const NameLoc& nameLoc)
    {
        m_builder << "NameLoc{";
        if (nameLoc.name)
        {
            dump(nameLoc.name->text.getUnownedSlice());
        }
        else
        {
            m_builder << "null";
        }
        m_builder << ", ";
        dump(nameLoc.loc);
        m_builder << " }";
    }
    void dump(BaseType baseType)
    {
        m_builder << BaseTypeInfo::asText(baseType);
    }
    void dump(Stage stage)
    {
        m_builder << getStageName(stage);
    }
    void dump(ImageFormat imageFormat)
    {
        m_builder << getGLSLNameForImageFormat(imageFormat);
    }

    void dump(const String& string)
    {
        dump(string.getUnownedSlice());
    }

    void dump(const DiagnosticInfo* info)
    {
        m_builder << "DiagnosticInfo {" << info->id << "}";
    }
    void dump(const Layout* layout)
    {
        m_builder << "Layout@" << size_t(layout); 
    }

    void dump(const Modifiers& modifiers)
    {
        auto& nonConstModifiers = const_cast<Modifiers&>(modifiers);


        m_builder << " { \n";
        indent();

        for (const auto& mod : nonConstModifiers)
        {
            dump(mod);
            m_builder << " ,\n";
        }

        dedent();
        m_builder << "}";
    }

    template <typename T>
    void dump(const SyntaxClass<T>& cls)
    {
        m_builder << cls.classInfo->m_name;
    }

    template <typename KEY, typename VALUE>
    void dump(const Dictionary<KEY, VALUE>& dict)
    {
        m_builder << " { \n";
        indent();

        for (auto iter : dict)
        {
            const auto& key = iter.Key;
            const auto& value = iter.Value;

            dump(key);
            m_builder << " : ";
            dump(value);

            m_builder << "\n";
        }

        dedent();
        m_builder << "}";
    }

    void dump(const DeclCheckStateExt& extState)
    {
        auto state = extState.getState();
        m_builder << "DeclCheckStateExt{" << extState.isBeingChecked() << ", " << Index(state) << "}";
    }

    void dump(TextureFlavor texFlavor)
    {
        m_builder << "TextureFlavor{" << Index(texFlavor.flavor) << "}";
    }

    void dump(SamplerStateFlavor flavor)
    {
        switch (flavor)
        {
            case SamplerStateFlavor::SamplerState: m_builder << "sampler"; break;
            case SamplerStateFlavor::SamplerComparisonState: m_builder << "samplerComparison"; break;
            default: m_builder << "unknown"; break;
        }
    }

    void dumpPtr(const void* ptr)
    {
        if (ptr)
        {
            m_builder << size_t(ptr);
        }
        else
        {
            m_builder << "null";
        }
    }

    void dump(SyntaxParseCallback callback) { dumpPtr((const void*)callback); }

    template <typename T, int SIZE>
    void dump(const T (&in)[SIZE])
    {
        m_builder << " { \n";
        indent();

        for (Index i = 0; i < Index(SIZE); ++i)
        {
            dump(in[i]);
            if (i < Index(SIZE) - 1)
            {
                m_builder << ",";
            }
            m_builder << "\n";
        }

        dedent();
        m_builder << "}";
    }

    //void dump(const void* ptr) { dumpPtr(ptr); }

    void dump(const LookupResult& result)
    {
        auto& nonConstResult = const_cast<LookupResult&>(result);

        m_builder << " { \n";
        indent();

        for (auto item : nonConstResult)
        {
            // TODO(JS):
            m_builder << "...\n";
        }

        dedent();
        m_builder << "}";
    }
    void dump(const GlobalGenericParamSubstitution::ConstraintArg& arg)
    {
        m_builder << " { \n";
        indent();

        dump(arg.decl);
        m_builder << ",\n";
        dump(arg.val);
        m_builder << "\n";

        dedent();
        m_builder << "}";
    }
    void dump(const TypeExp& exp)
    {
        m_builder << " { \n";
        indent();

        dump(exp.exp);
        m_builder << ",\n";
        dump(exp.type);
        m_builder << "\n";

        dedent();
        m_builder << "}";
    }
    void dump(const ExpandedSpecializationArg& arg)
    {
        dump(arg.witness);
    }

    void dump(const TransparentMemberInfo& memInfo)
    {
        dump(memInfo.decl);
    }

    void dumpFieldName(const char* name)
    {
        _indent();
        m_builder << name << " : ";
    }

    void indent() { m_indent++; }
    void dedent() { m_indent--; }

    void _indent()
    {
        for (Int i = 0; i < m_indent * m_indentCount; ++i)
        {
            m_builder << " ";
        }
    }

    Dictionary<RefObject*, Index> m_objectMap;  ///< Object index
    List<ObjectInfo> m_objects;

    StringBuilder m_builder;
    Index m_indent = 0;                 ///< Current indent
    Index m_indentCount = 4;            ///< Amount of spaces in indent
};

} // anonymous

// Lets generate functions one for each that attempts to write out *it's* fields.
// We can write out the Super types fields by looking that up

struct ASTDumpAccess
{
#define SLANG_AST_DUMP_FIELD(FIELD_NAME, TYPE, param) context.dumpFieldName(#FIELD_NAME); context.dump(ASTDumpUtil::getMember(node->FIELD_NAME));

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

static DumpFieldFuncs s_funcs;


void Context::dumpObject(const ReflectClassInfo& type, RefObject* obj)
{
    Index index = getObjectIndex(obj);

    ObjectInfo& info = m_objects[index];
    if (info.m_isDumped == true)
    {
        return;
    }

    // To stop cycles
    info.m_isDumped = true;

    // We need to dump the fields.

    m_builder << type.m_name << "@" << index < " {\n";
    indent();

    List<const ReflectClassInfo*> allTypes;
    {
        const ReflectClassInfo* curType = &type;
        do
        {
            allTypes.add(curType);
            curType = curType->m_superClass;
        }while (curType);
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

    dedent();
    m_builder << "}\n";
}

/* static */void ASTDumpUtil::dump(NodeBase* node, StringBuilder& out)
{
    Context context;
    context.dump(node);
    out = context.m_builder;
}

/* static */void ASTDumpUtil::dump(Substitutions* subs, StringBuilder& out)
{
    Context context;
    context.dump(subs);
    out = context.m_builder;
}

} // namespace Slang

// slang-serialize-ast.cpp
#include "slang-serialize-ast.h"

#include "slang-ast-generated.h"
#include "slang-ast-generated-macro.h"

#include "slang-ast-dump.h"

#include "slang-ast-support-types.h"

// Needed for ModuleSerialFilter
// Needed for 'findModuleForDecl'
#include "slang-legalize-types.h"
#include "slang-mangle.h"

#include "slang-serialize-type-info.h"
#include "slang-serialize-misc-type-info.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ModuleSerialFilter  !!!!!!!!!!!!!!!!!!!!!!!!

SerialIndex ModuleSerialFilter::writePointer(SerialWriter* writer, const NodeBase* inPtr) 
{
    NodeBase* ptr = const_cast<NodeBase*>(inPtr);
    SLANG_ASSERT(ptr);
    
    if (Decl* decl = as<Decl>(ptr))
    {
        ModuleDecl* moduleDecl = findModuleForDecl(decl);
        SLANG_ASSERT(moduleDecl);
        if (moduleDecl && moduleDecl != m_moduleDecl)
        {
            ASTBuilder* astBuilder = m_moduleDecl->module->getASTBuilder();

            // It's a reference to a declaration in another module, so create an ImportExternalDecl.

            String mangledName = getMangledName(astBuilder, decl);

            ImportExternalDecl* importDecl = astBuilder->create<ImportExternalDecl>();
            importDecl->mangledName = mangledName;
            const SerialIndex index = writer->addPointer(importDecl);

            // Set as the index of this
            writer->setPointerIndex(ptr, index);

            return index;
        }
        else
        {
            // Okay... we can just write it out then
            return writer->writeObject(ptr);
        }
    }

    // TODO(JS): What we really want to do here is to ignore bodies functions.
    // It's not 100% clear if this is even right though - for example does type inference
    // imply the body is needed to say infer a return type?
    // Also not clear if statements in other scenarios (if there are others) might need to be kept.
    //
    // For now we just ignore all stmts

    if (Stmt* stmt = as<Stmt>(ptr))
    {
        //
        writer->setPointerIndex(stmt, SerialIndex(0));
        return SerialIndex(0);
    }

    // For now for everything else just write it
    return writer->writeObject(ptr);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! AST types !!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

// SyntaxClass<T>
template <typename T>
struct SerialTypeInfo<SyntaxClass<T>>
{
    typedef SyntaxClass<T> NativeType;
    typedef uint16_t SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        SLANG_UNUSED(writer);
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;
        dst = SerialType(src.classInfo->m_classId);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        SLANG_UNUSED(reader);
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;
        dst.classInfo = ASTClassInfo::getInfo(ASTNodeType(src));
    }
};

struct SerialDeclRefBaseTypeInfo
{
    typedef DeclRefBase NativeType;
    struct SerialType
    {
        SerialIndex substitutions;
        SerialIndex decl;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* inNative, void* outSerial)
    {
        SerialType& serial = *(SerialType*)outSerial;
        const NativeType& native = *(const NativeType*)inNative;

        serial.decl = writer->addPointer(native.decl);
        serial.substitutions = writer->addPointer(native.substitutions.substitutions);
    }
    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        DeclRefBase& native = *(DeclRefBase*)(outNative);
        const SerialType& serial = *(const SerialType*)inSerial;

        native.decl = reader->getPointer(serial.decl).dynamicCast<Decl>();
        native.substitutions.substitutions = reader->getPointer(serial.substitutions).dynamicCast<Substitutions>();
    }
    static const SerialFieldType* getFieldType()
    {
        static const SerialFieldType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};
// Special case DeclRef, because it always uses the same type
template <typename T>
struct SerialGetFieldType<DeclRef<T>>
{
    static const SerialFieldType* getFieldType() { return SerialDeclRefBaseTypeInfo::getFieldType(); }
};


template <typename T>
struct SerialTypeInfo<DeclRef<T>> : public SerialDeclRefBaseTypeInfo {};

// MatrixCoord can just go as is
template <>
struct SerialTypeInfo<MatrixCoord> : SerialIdentityTypeInfo<MatrixCoord> {};


// QualType

template <>
struct SerialTypeInfo<QualType>
{
    typedef QualType NativeType;
    struct SerialType
    {
        SerialIndex type;
        uint8_t isLeftValue;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto dst = (SerialType*)serial;
        auto src = (const NativeType*)native;
        dst->isLeftValue = src->isLeftValue ? 1 : 0;
        dst->type = writer->addPointer(src->type);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto src = (const SerialType*)serial;
        auto dst = (NativeType*)native;
        dst->type = reader->getPointer(src->type).dynamicCast<Type>();
        dst->isLeftValue = src->isLeftValue != 0;
    }
};


// LookupResult::Breadcrumb
template <>
struct SerialTypeInfo<LookupResultItem::Breadcrumb>
{
    typedef LookupResultItem::Breadcrumb NativeType;
    struct SerialType
    {
        NativeType::Kind kind;
        NativeType::ThisParameterMode thisParameterMode;
        SerialTypeInfo<DeclRef<Decl>>::SerialType declRef;
        SerialTypeInfo<RefPtr<NativeType>> next; 
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst.kind = src.kind;
        dst.thisParameterMode = src.thisParameterMode;
        toSerialValue(writer, src.declRef, dst.declRef);
        toSerialValue(writer, src.next, dst.next);
    }
     
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        dst.kind = src.kind;
        dst.thisParameterMode = src.thisParameterMode;
        toNativeValue(reader, src.declRef, dst.declRef);
        toNativeValue(reader, src.next, dst.next);
    }
};

// LookupResultItem
template <>
struct SerialTypeInfo<LookupResultItem>
{
    typedef LookupResultItem NativeType;
    struct SerialType
    {
        SerialTypeInfo<DeclRef<Decl>>::SerialType declRef;
        SerialTypeInfo<RefPtr<NativeType::Breadcrumb>> breadcrumbs;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        toSerialValue(writer, src.declRef, dst.declRef);
        toSerialValue(writer, src.breadcrumbs, dst.breadcrumbs);
    }

    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        toNativeValue(reader, src.declRef, dst.declRef);
        toNativeValue(reader, src.breadcrumbs, dst.breadcrumbs);
    }
};

// LookupResult
template <>
struct SerialTypeInfo<LookupResult>
{
    typedef LookupResult NativeType;
    typedef SerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        if (src.isOverloaded())
        {
            // Save off as an array
            dst = writer->addArray(src.items.getBuffer(), src.items.getCount());
        }
        else if (src.item.declRef.getDecl())
        {
            dst = writer->addArray(&src.item, 1);
        }
        else
        {
            dst = SerialIndex(0);
        }
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        // Initialize
        dst = NativeType();

        List<LookupResultItem> items;
        reader->getArray(src, items);

        if (items.getCount() == 1)
        {
            dst.item = items[0];
        }
        else
        {
            dst.items.swapWith(items);
            // We have to set item such that it is valid/member of items, if items is non empty
            dst.item = dst.items[0];
        }
    }
};

// GlobalGenericParamSubstitution::ConstraintArg
template <>
struct SerialTypeInfo<GlobalGenericParamSubstitution::ConstraintArg>
{
    typedef GlobalGenericParamSubstitution::ConstraintArg NativeType;
    struct SerialType
    {
        SerialIndex decl;
        SerialIndex val;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.decl = writer->addPointer(src.decl);
        dst.val = writer->addPointer(src.val);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.decl = reader->getPointer(src.decl).dynamicCast<Decl>();
        dst.val = reader->getPointer(src.val).dynamicCast<Val>();
    }
};

// ExpandedSpecializationArg
template <>
struct SerialTypeInfo<ExpandedSpecializationArg>
{
    typedef ExpandedSpecializationArg NativeType;
    struct SerialType
    {
        SerialIndex val;
        SerialIndex witness;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.witness = writer->addPointer(src.witness);
        dst.val = writer->addPointer(src.val);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.witness = reader->getPointer(src.witness).dynamicCast<Val>();
        dst.val = reader->getPointer(src.val).dynamicCast<Val>();
    }
};

// TypeExp
template <>
struct SerialTypeInfo<TypeExp>
{
    typedef TypeExp NativeType;
    struct SerialType
    {
        SerialIndex type;
        SerialIndex expr;
    };
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& dst = *(SerialType*)serial;
        auto& src = *(const NativeType*)native;

        dst.type = writer->addPointer(src.type);
        dst.expr = writer->addPointer(src.exp);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        dst.type = reader->getPointer(src.type).dynamicCast<Type>();
        dst.exp = reader->getPointer(src.expr).dynamicCast<Expr>();
    }
};

// DeclCheckStateExt
template <>
struct SerialTypeInfo<DeclCheckStateExt>
{
    typedef DeclCheckStateExt NativeType;
    typedef DeclCheckStateExt::RawType SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        SLANG_UNUSED(writer);
        *(SerialType*)serial = (*(const NativeType*)native).getRaw();
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        SLANG_UNUSED(reader);
        (*(NativeType*)serial).setRaw(*(const SerialType*)native); 
    }
};

// Modifiers
template <>
struct SerialTypeInfo<Modifiers>
{
    typedef Modifiers NativeType;
    typedef SerialIndex SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        // We need to make into an array
        List<SerialIndex> modifierIndices;
        for (Modifier* modifier : *(NativeType*)native)
        {
            modifierIndices.add(writer->addPointer(modifier));
        }
        *(SerialType*)serial = writer->addArray(modifierIndices.getBuffer(), modifierIndices.getCount());
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        List<Modifier*> modifiers;
        reader->getArray(*(const SerialType*)serial, modifiers);

        Modifier* prev = nullptr;
        for (Modifier* modifier : modifiers)
        {
            if (prev)
            {
                prev->next = modifier;
            }
        }

        NativeType& dst = *(NativeType*)native;
        dst.first = modifiers.getCount() > 0 ? modifiers[0] : nullptr;
    }
};

// ASTNodeType
template <>
struct SerialTypeInfo<ASTNodeType> : public SerialConvertTypeInfo<ASTNodeType, uint16_t> {};

// !!!!!!!!!!!!!!!!!!!!!! Generate fields for a type !!!!!!!!!!!!!!!!!!!!!!!!!!!

template <typename T>
SerialField _makeField(const char* name, T& in)
{
    uint8_t* ptr = &reinterpret_cast<uint8_t&>(in);

    SerialField field;
    field.name = name;
    field.type = SerialGetFieldType<T>::getFieldType();
    // This only works because we in is an offset from 1
    field.nativeOffset = uint32_t(size_t(ptr) - 1);
    field.serialOffset = 0;
    return field;
}

static const SerialClass* _addClass(SerialClasses* serialClasses, ASTNodeType type, ASTNodeType super, const List<SerialField>& fields)
{
    const SerialClass* superClass = serialClasses->getSerialClass(SerialTypeKind::NodeBase, SerialSubType(super));
    return serialClasses->add(SerialTypeKind::NodeBase, SerialSubType(type), fields.getBuffer(), fields.getCount(), superClass);
}

#define SLANG_AST_ADD_SERIAL_FIELD(FIELD_NAME, TYPE, param) fields.add(_makeField(#FIELD_NAME, obj->FIELD_NAME));

// Note that the obj point is not nullptr, because some compilers notice this is 'indexing from null'
// and warn/error. So we offset from 1.
#define SLANG_AST_ADD_SERIAL_CLASS(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
{ \
    NAME* obj = (NAME*)1; \
    SLANG_UNUSED(obj); \
    fields.clear(); \
    SLANG_FIELDS_ASTNode_##NAME(SLANG_AST_ADD_SERIAL_FIELD, param) \
    _addClass(serialClasses, ASTNodeType::NAME, ASTNodeType::SUPER, fields); \
}

struct ASTFieldAccess
{
    static void calcClasses(SerialClasses* serialClasses)
    {
        // Add NodeBase first, and specially handle so that we add a null super class
        serialClasses->add(SerialTypeKind::NodeBase, SerialSubType(ASTNodeType::NodeBase), nullptr, 0, nullptr);

        // Add the rest in order such that Super class is always added before its children
        List<SerialField> fields;
        SLANG_CHILDREN_ASTNode_NodeBase(SLANG_AST_ADD_SERIAL_CLASS, _)
    }
};

void addASTTypes(SerialClasses* serialClasses)
{
    {
        ASTFieldAccess::calcClasses(serialClasses);
    }

    {
        {
            // Let's hack Breadcrumbs...

            typedef LookupResultItem::Breadcrumb Type;
            Type* obj = (Type*)1;
            SerialField field = _makeField("_", *obj);
            serialClasses->add(SerialTypeKind::RefObject, SerialSubType(RefObjectSerialSubType::LookupResultItem_Breadcrumb), &field, 1, nullptr);
        }

        // Set these types to not serialize
        serialClasses->addUnserialized(SerialTypeKind::RefObject, SerialSubType(RefObjectSerialSubType::Module));
        serialClasses->addUnserialized(SerialTypeKind::RefObject, SerialSubType(RefObjectSerialSubType::Scope));
    }
}

// A Hack for now to turn an RefObject* into a SubType for serialization
extern RefObjectSerialSubType getRefObjectSubType(const RefObject* obj)
{
    if (as<LookupResultItem::Breadcrumb>(obj))
    {
        return RefObjectSerialSubType::LookupResultItem_Breadcrumb;
    }
    else if (as<Module>(obj))
    {
        return RefObjectSerialSubType::Module;
    }
    else if (as<Scope>(obj))
    {
        return RefObjectSerialSubType::Scope;
    }
    return RefObjectSerialSubType::Invalid;
}

/* !!!!!!!!!!!!!!!!!!!!!! DefaultSerialObjectFactory !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* DefaultSerialObjectFactory::create(SerialTypeKind typeKind, SerialSubType subType) 
{
    switch (typeKind)
    {
        case SerialTypeKind::NodeBase:
        {
            return m_astBuilder->createByNodeType(ASTNodeType(subType));
        }
        case SerialTypeKind::RefObject:
        {
            switch (RefObjectSerialSubType(subType))
            {
                case RefObjectSerialSubType::LookupResultItem_Breadcrumb:
                {
                    typedef LookupResultItem::Breadcrumb Breadcrumb;
                    return _add(new LookupResultItem::Breadcrumb(Breadcrumb::Kind::Member, DeclRef<Decl>(), nullptr, nullptr));
                }
                default: break;
            }
        }
        default: break;
    }

    return nullptr;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTSerializeUtil  !!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */SlangResult ASTSerialTestUtil::selfTest()
{
    RefPtr<SerialClasses> classes;
    SerialClasses::create(classes);

    {
        const SerialFieldType* type = SerialGetFieldType<Type*>::getFieldType();
        SLANG_UNUSED(type);
    }

    {
        const SerialFieldType* type = SerialGetFieldType<int[10]>::getFieldType();
        SLANG_UNUSED(type);
    }

    {
        const SerialFieldType* type = SerialGetFieldType<bool[3]>::getFieldType();
        SLANG_UNUSED(type);
    }

    {
        const SerialFieldType* type = SerialGetFieldType<Type*[3]>::getFieldType();
        SLANG_UNUSED(type);
    }

    return SLANG_OK;
}

/* static */SlangResult ASTSerialTestUtil::testSerialize(NodeBase* node, RootNamePool* rootNamePool, SharedASTBuilder* sharedASTBuilder, SourceManager* sourceManager)
{
    RefPtr<SerialClasses> classes;
    SerialClasses::create(classes);

    List<uint8_t> contents;

    {
        OwnedMemoryStream stream(FileAccess::ReadWrite);

        ModuleDecl* moduleDecl = as<ModuleDecl>(node);
        // Only serialize out things *in* this module
        ModuleSerialFilter filterStorage(moduleDecl);

        SerialFilter* filter = moduleDecl ? &filterStorage : nullptr;

        SerialWriter writer(classes, filter);

        // Lets serialize it all
        writer.addPointer(node);
        // Let's stick it all in a stream
        writer.write(&stream);

        stream.swapContents(contents);

        NamePool namePool;
        namePool.setRootNamePool(rootNamePool);

        SerialReader reader(classes, nullptr);

        ASTBuilder builder(sharedASTBuilder, "Serialize Check");

        DefaultSerialObjectFactory objectFactory(&builder);

        // We could now check that the loaded data matches

        {
            const List<SerialInfo::Entry*>& writtenEntries = writer.getEntries();
            List<const SerialInfo::Entry*> readEntries;

            SlangResult res = reader.loadEntries(contents.getBuffer(), contents.getCount(), readEntries);
            SLANG_UNUSED(res);

            SLANG_ASSERT(writtenEntries.getCount() == readEntries.getCount());

            // They should be identical up to the
            for (Index i = 1; i < readEntries.getCount(); ++i)
            {
                auto writtenEntry = writtenEntries[i];
                auto readEntry = readEntries[i];

                const size_t writtenSize = writtenEntry->calcSize(classes);
                const size_t readSize = readEntry->calcSize(classes);

                SLANG_ASSERT(readSize == writtenSize);
                // Check the payload is the same
                SLANG_ASSERT(memcmp(readEntry, writtenEntry, readSize) == 0);
            }

        }

        {
            SlangResult res = reader.load(contents.getBuffer(), contents.getCount(), &namePool);
            SLANG_UNUSED(res);
        }

        // Lets see what we have
        const ASTDumpUtil::Flags dumpFlags = ASTDumpUtil::Flag::HideSourceLoc | ASTDumpUtil::Flag::HideScope;

        String readDump;
        {
            SourceWriter sourceWriter(sourceManager, LineDirectiveMode::None);
            ASTDumpUtil::dump(reader.getPointer(SerialIndex(1)).dynamicCast<NodeBase>(), ASTDumpUtil::Style::Hierachical, dumpFlags, &sourceWriter);
            readDump = sourceWriter.getContentAndClear();

        }
        String origDump;
        {
            SourceWriter sourceWriter(sourceManager, LineDirectiveMode::None);
            ASTDumpUtil::dump(node, ASTDumpUtil::Style::Hierachical, dumpFlags, &sourceWriter);
            origDump = sourceWriter.getContentAndClear();
        }

        // Write out
        File::writeAllText("ast-read.ast-dump", readDump);
        File::writeAllText("ast-orig.ast-dump", origDump);

        if (readDump != origDump)
        {
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}

} // namespace Slang

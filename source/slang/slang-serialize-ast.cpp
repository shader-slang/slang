// slang-serialize-ast.cpp
#include "slang-serialize-ast.h"

#include "slang-generated-ast.h"
#include "slang-generated-ast-macro.h"

#include "slang-ast-dump.h"

#include "slang-ast-support-types.h"

#include "slang-serialize-ast-type-info.h"

#include "slang-serialize-factory.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!! Generate fields for a type !!!!!!!!!!!!!!!!!!!!!!!!!!!

static const SerialClass* _addClass(SerialClasses* serialClasses, ASTNodeType type, ASTNodeType super, const List<SerialField>& fields)
{
    const SerialClass* superClass = serialClasses->getSerialClass(SerialTypeKind::NodeBase, SerialSubType(super));
    return serialClasses->add(SerialTypeKind::NodeBase, SerialSubType(type), fields.getBuffer(), fields.getCount(), superClass);
}

#define SLANG_AST_ADD_SERIAL_FIELD(FIELD_NAME, TYPE, param) fields.add(SerialField::make(#FIELD_NAME, &obj->FIELD_NAME));

// Note that the obj point is not nullptr, because some compilers notice this is 'indexing from null'
// and warn/error. So we offset from 1.
#define SLANG_AST_ADD_SERIAL_CLASS(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
{ \
    NAME* obj = SerialField::getPtr<NAME>(); \
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ASTSerialUtil  !!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */void ASTSerialUtil::addSerialClasses(SerialClasses* serialClasses)
{
     ASTFieldAccess::calcClasses(serialClasses);
}
 
/* static */SlangResult ASTSerialUtil::testSerialize(NodeBase* node, RootNamePool* rootNamePool, SharedASTBuilder* sharedASTBuilder, SourceManager* sourceManager)
{
    RefPtr<SerialClasses> classes;

    SerialClassesUtil::create(classes);

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

        ASTBuilder builder(sharedASTBuilder, "Serialize Check");

        SetASTBuilderContextRAII astBuilderRAII(&builder);

        DefaultSerialObjectFactory objectFactory(&builder);

        // We could now check that the loaded data matches

        {
            const List<SerialInfo::Entry*>& writtenEntries = writer.getEntries();
            List<const SerialInfo::Entry*> readEntries;

            SlangResult res = SerialReader::loadEntries(contents.getBuffer(), contents.getCount(), classes, readEntries);
            SLANG_UNUSED(res);

            SLANG_ASSERT(writtenEntries.getCount() == readEntries.getCount());

            // They should be identical up to the
            for (Index i = 1; i < readEntries.getCount(); ++i)
            {
                auto writtenEntry = writtenEntries[i];
                auto readEntry = readEntries[i];

                const size_t writtenSize = writtenEntry->calcSize(classes);
                const size_t readSize = readEntry->calcSize(classes);
                SLANG_UNUSED(writtenSize);
                SLANG_UNUSED(readSize);

                SLANG_ASSERT(readSize == writtenSize);
                // Check the payload is the same
                SLANG_ASSERT(memcmp(readEntry, writtenEntry, readSize) == 0);
            }

        }

        SerialReader reader(classes, nullptr);
        {
            
            SlangResult res = reader.load(contents.getBuffer(), contents.getCount(), &namePool);
            SLANG_UNUSED(res);
        }

        // Lets see what we have
        const ASTDumpUtil::Flags dumpFlags = ASTDumpUtil::Flag::HideSourceLoc | ASTDumpUtil::Flag::HideScope;

        String readDump;
        {
            SourceWriter sourceWriter(sourceManager, LineDirectiveMode::None, nullptr);
            ASTDumpUtil::dump(reader.getPointer(SerialIndex(1)).dynamicCast<NodeBase>(), ASTDumpUtil::Style::Hierachical, dumpFlags, &sourceWriter);
            readDump = sourceWriter.getContentAndClear();

        }
        String origDump;
        {
            SourceWriter sourceWriter(sourceManager, LineDirectiveMode::None, nullptr);
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

/* static */List<uint8_t> ASTSerialUtil::serializeAST(ModuleDecl* moduleDecl)
{
    //TODO: we should store `classes` in GlobalSession to avoid recomputing them every time.
    RefPtr<SerialClasses> classes;
    SerialClassesUtil::create(classes);

    List<uint8_t> contents;
    OwnedMemoryStream stream(FileAccess::ReadWrite);

    // Only serialize out things *in* this module
    ModuleSerialFilter filterStorage(moduleDecl);

    SerialFilter* filter = moduleDecl ? &filterStorage : nullptr;

    SerialWriter writer(classes, filter);

    // Lets serialize it all
    writer.addPointer(moduleDecl);
    // Let's stick it all in a stream
    writer.write(&stream);

    stream.swapContents(contents);
    return contents;
}

} // namespace Slang

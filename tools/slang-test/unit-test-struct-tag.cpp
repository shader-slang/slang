// unit-test-struct-tag.cpp

#include "../../source/core/slang-memory-arena.h"

#include <stdio.h>
#include <stdlib.h>

#include "test-context.h"

#include "../../source/core/slang-random-generator.h"
#include "../../source/core/slang-list.h"

#include "../../source/compiler-core/slang-struct-tag-system.h"
#include "../../source/compiler-core/slang-struct-tag-converter.h"

using namespace Slang;

namespace { // anonymous

#define TAGGED_STRUCTS(x) \
    x(A) \
    x(B) \
    x(ExtensionA) \
    x(Desc)

enum class TaggedStruct
{
    TAGGED_STRUCTS(SLANG_STRUCT_TAG_ENUM)
};

#define PRIMARY_TAGGED_STRUCT(TYPE_NAME, MAJOR, MINOR) SLANG_PRIMARY_TAGGED_STRUCT_IMPL(TYPE_NAME##MAJOR##_##MINOR, slang::StructTagCategory::Core, TaggedStruct::TYPE_NAME, MAJOR, MINOR)
#define EXTENSION_TAGGED_STRUCT(TYPE_NAME, MAJOR, MINOR) SLANG_EXTENSION_TAGGED_STRUCT_IMPL(TYPE_NAME##MAJOR##_##MINOR, slang::StructTagCategory::Core, TaggedStruct::TYPE_NAME, MAJOR, MINOR)

struct A0_0
{
    PRIMARY_TAGGED_STRUCT(A, 0, 0)

    int a = 10;
};

struct A0_1
{
    PRIMARY_TAGGED_STRUCT(A, 0, 1)

    int a = 10;
    int b = 20;
};

struct B0_0
{
    PRIMARY_TAGGED_STRUCT(B, 0, 0)

    float v = -1.0f;
};

struct B0_1
{
    PRIMARY_TAGGED_STRUCT(B, 0, 1)

    float v = -1.0f;
    float u = 20.0f;
};

struct A1_0
{
    PRIMARY_TAGGED_STRUCT(A, 1, 0)
};


struct ExtensionA0_0
{
    EXTENSION_TAGGED_STRUCT(ExtensionA, 0, 0)

    int b = 20;
};

// I guess a Desc could be implemented as 'Primary' or 'Extension' with what we have, but probably makes more sense
// for 'Desc' like things to be primary

struct Desc0_0
{
    EXTENSION_TAGGED_STRUCT(Desc, 0, 0)
    int a = 1;
};

struct Desc0_1
{
    EXTENSION_TAGGED_STRUCT(Desc, 0, 1)
    int a = 1;
    int b = 2;
};


struct A0_2
{
    PRIMARY_TAGGED_STRUCT(A, 0, 2)

    int a = 10;
    int b = 20;

    Desc0_0* descs = nullptr;
    Index descsCount = 0;
};

} // anonymous

static RefPtr<StructTagSystem> _createSystem()
{
    RefPtr<StructTagSystem> system = new StructTagSystem;

    {
#define SLANG_STRUCT_TAG_ADD_CATEGORY(x) system->addCategoryInfo(slang::StructTagCategory::x, #x);
        SLANG_STRUCT_TAG_CATEGORIES(SLANG_STRUCT_TAG_ADD_CATEGORY)
    }

    return system;
}

static void structTagUnitTest()
{
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    DiagnosticSink sink;
    sink.init(&sourceManager, nullptr);

    {
        StructTagUtil::TypeInfo info = StructTagUtil::getTypeInfo(A0_1::kStructTag);

        SLANG_CHECK(info.kind == slang::StructTagKind::Primary);
        SLANG_CHECK(info.category == slang::StructTagCategory::Core);
        SLANG_CHECK(info.majorVersion == 0);
        SLANG_CHECK(info.minorVersion == 1);
    }

    {
        StructTagUtil::TypeInfo info = StructTagUtil::getTypeInfo(ExtensionA0_0::kStructTag);

        SLANG_CHECK(info.kind == slang::StructTagKind::Extension);
        SLANG_CHECK(info.category == slang::StructTagCategory::Core);
        SLANG_CHECK(info.majorVersion == 0);
        SLANG_CHECK(info.minorVersion == 0);
    }

    {
        // Set up the system with the versions
        auto system = _createSystem();

        system->addType(B0_1::kStructTag, "B", sizeof(B0_1));
        system->addType(A0_1::kStructTag, "A", sizeof(A0_1));
        system->addType(ExtensionA0_0::kStructTag, "ExtensionA", sizeof(ExtensionA0_0));

        {
            //The null operation means we are converting everything that is current (as defined by the system)

            A0_1 a;
            ExtensionA0_0 extA;
            const slang::StructTag* exts[] = { &extA.structTag };
            a.exts = exts;
            a.extsCount = SLANG_COUNT_OF(exts);

            
            LazyStructTagConverter converter(system, nullptr, nullptr);

            auto dstA = converter.convertToCurrent<A0_1>(&a);

            // We shouldn't have to convert anything, so we should be done
            SLANG_CHECK(dstA == &a);
        }

        {
            A0_0 a;
            ExtensionA0_0 extA;
            const slang::StructTag* exts[] = { &extA.structTag };
            a.exts = exts;
            a.extsCount = SLANG_COUNT_OF(exts);


            // Actually do a conversion from past
            MemoryArena arena(1024);
            LazyStructTagConverter converter(system, &arena, nullptr);

            auto dstA = converter.convertToCurrent<A0_1>(&a);

            SLANG_CHECK(dstA != nullptr);
            SLANG_CHECK(dstA->a == 10 && dstA->b == 0);

            SLANG_CHECK(dstA->extsCount == 1);
            SLANG_CHECK(((ExtensionA0_0*)dstA->exts[0])->b == 20);
        }
    }

    // Let's try going from the future backwards
    {
        // Set up the system with the versions
        auto system = _createSystem();

        system->addType(B0_1::kStructTag, "B", sizeof(B0_1));
        system->addType(A0_2::kStructTag, "A", sizeof(A0_2));
        system->addType(ExtensionA0_0::kStructTag, "ExtensionA", sizeof(ExtensionA0_0));
        system->addType(Desc0_0::kStructTag, "Desc", sizeof(Desc0_0));

        // Add the fields
        {
            auto type = system->getType(A0_2::kStructTag);
            A0_2 a;
            auto field = StructTagTypeTraits::getFieldWithCount(&a, &a.descs, &a.descsCount);
            type->m_fields.add(field);
        }

        //

        A0_2 a;
        Desc0_1 descs[2];
        descs[0].a = 27;
        descs[1].a = -1;

        a.descs = (Desc0_0*)descs;
        a.descsCount = SLANG_COUNT_OF(descs);

        // Actually do a conversion from future
        MemoryArena arena(1024);
        LazyStructTagConverter converter(system, &arena, nullptr);

        auto dstA = converter.convertToCurrent<A0_2>(&a);

        SLANG_CHECK(dstA->descsCount == a.descsCount);

        SLANG_CHECK(dstA->descs[0].a == 27 && dstA->descs[1].a == -1);
    }

    // Lets try some invalid conversions

    {
        // Set up the system with the versions
        auto system = _createSystem();

        system->addType(B0_1::kStructTag, "B", sizeof(B0_1));
        system->addType(A0_2::kStructTag, "A", sizeof(A0_2));
        system->addType(ExtensionA0_0::kStructTag, "ExtensionA", sizeof(ExtensionA0_0));
        system->addType(Desc0_0::kStructTag, "Desc", sizeof(Desc0_0));

        A1_0 a;

        MemoryArena arena(1024);
        LazyStructTagConverter converter(system, &arena, &sink);

        void* dst;
        SLANG_CHECK(SLANG_FAILED(converter.convertCurrent(&a, dst)));

    }

}

SLANG_UNIT_TEST("StructTag", structTagUnitTest);

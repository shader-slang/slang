// slang-serialize.cpp
#include "slang-serialize.h"

#include "slang-ast-base.h"
#include "slang-ast-builder.h"

namespace Slang {

const SerialClass* SerialClasses::add(const SerialClass* cls)
{
    List<const SerialClass*>& classes = m_classesByTypeKind[Index(cls->typeKind)]; 

    if (cls->subType >= classes.getCount())
    {
        classes.setCount(cls->subType + 1);
    }
    else
    {
        if (classes[cls->subType])
        {
            SLANG_ASSERT(!"Type is already set");
            return nullptr;
        }
    }

    SerialClass* copy = _createSerialClass(cls);
    classes[cls->subType] = copy;

    return copy;
}

const SerialClass* SerialClasses::add(SerialTypeKind kind, SerialSubType subType, const SerialField* fields, Index fieldsCount, const SerialClass* superCls)
{
    SerialClass cls;
    cls.typeKind = kind;
    cls.subType = subType;
        
    cls.fields = fields;
    cls.fieldsCount = fieldsCount;

    // If the superCls is set it must be owned
    SLANG_ASSERT(superCls == nullptr || isOwned(superCls));

    cls.super = superCls;

    // Set to invalid values for now
    cls.alignment = 0;
    cls.size = 0;
    cls.flags = 0;

    return add(&cls);
}

const SerialClass* SerialClasses::addUnserialized(SerialTypeKind kind, SerialSubType subType)
{
    List<const SerialClass*>& classes = m_classesByTypeKind[Index(kind)];

    if (subType >= classes.getCount())
    {
        classes.setCount(subType + 1);
    }
    else
    {
        if (classes[subType])
        {
            SLANG_ASSERT(!"Type is already set");
            return nullptr;
        }
    }

    SerialClass* dst = m_arena.allocate<SerialClass>();

    dst->typeKind = kind;
    dst->subType = subType;

    dst->size = 0;
    dst->alignment = 0;

    dst->fields = nullptr;
    dst->fieldsCount = 0;
    dst->flags = SerialClassFlag::DontSerialize;
    dst->super = nullptr;

    classes[subType] = dst;
    return dst;
}

bool SerialClasses::isOwned(const SerialClass* cls) const
{
    const List<const SerialClass*>& classes = m_classesByTypeKind[Index(cls->typeKind)];
    return cls->subType < classes.getCount() && classes[cls->subType] == cls;
}

SerialClass* SerialClasses::_createSerialClass(const SerialClass* cls)
{
    uint32_t maxAlignment = 1;
    uint32_t offset = 0;

    if (cls->super)
    {
        SLANG_ASSERT(isOwned(cls->super));

        maxAlignment = cls->super->alignment;
        offset = cls->super->size;
    }

    // Can't be 0
    SLANG_ASSERT(maxAlignment != 0);
    // Must be a power of 2
    SLANG_ASSERT((maxAlignment & (maxAlignment - 1)) == 0);

    // Check it is correctly aligned
    SLANG_ASSERT((offset & (maxAlignment - 1)) == 0);

    SerialField* dstFields = m_arena.allocateArray<SerialField>(cls->fieldsCount);

    // Okay, go through fields setting their offset
    const SerialField* srcFields = cls->fields;
    for (Index j = 0; j < cls->fieldsCount; j++)
    {
        const SerialField& srcField = srcFields[j];
        SerialField& dstField = dstFields[j];

        // Copy the field
        dstField = srcField;

        uint32_t alignment = srcField.type->serialAlignment;
        // Make sure the offset is aligned for the field requirement
        offset = (offset + alignment - 1) & ~(alignment - 1);

        // Save the field offset
        dstField.serialOffset = uint32_t(offset);

        // Move past the field
        offset += uint32_t(srcField.type->serialSizeInBytes);

        // Calc the maximum alignment
        maxAlignment = (alignment > maxAlignment) ? alignment : maxAlignment;
    }

    // Align with maximum alignment
    offset = (offset + maxAlignment - 1) & ~(maxAlignment - 1);

    SerialClass* dst = m_arena.allocate<SerialClass>();
    *dst = *cls;

    dst->alignment = uint8_t(maxAlignment);
    dst->size = uint32_t(offset);

    dst->fields = dstFields;

    return dst;
}

bool SerialClasses::isOk() const
{
    StringSlicePool pool(StringSlicePool::Style::Default);

    for (const auto& classes : m_classesByTypeKind)
    {
        for (const SerialClass* cls : classes)
        {
            // It is possible potentially to have gaps
            if (cls == nullptr)
            {
                continue;
            }

            if (cls->super && cls->super->typeKind != cls->typeKind)
            {
                // If has a super type, must be the same typeKind
                return false;
            }

            // Make sure the fields are uniquely named

            pool.clear();

            {
                const SerialClass* curCls = cls;

                do
                {
                    for (Index i = 0; i < curCls->fieldsCount; ++i)
                    {
                        const SerialField& field = curCls->fields[i];

                        StringSlicePool::Handle handle;
                        if (pool.findOrAdd(UnownedStringSlice(field.name), handle))
                        {
                            return false;
                        }
                    }

                    // Add the fields of the parent
                    curCls = curCls->super;
                }
                while (curCls);
            }
        }
    }

    return true;
}


SerialClasses::SerialClasses():
    m_arena(2097152)
{
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialWriter  !!!!!!!!!!!!!!!!!!!!!!!!!!!!

SerialWriter::SerialWriter(SerialClasses* classes, SerialFilter* filter, Flags flags)
    : m_arena(2097152)
    , m_classes(classes)
    , m_filter(filter)
    , m_flags(flags)
{
    // 0 is always the null pointer
    m_entries.add(nullptr);
    m_ptrMap.add(nullptr, 0);
}

SerialIndex SerialWriter::writeObject(const SerialClass* serialCls, const void* ptr)
{
    if (serialCls->flags & SerialClassFlag::DontSerialize)
    {
        return SerialIndex(0);
    }

    // This pointer cannot be in the map
    SLANG_ASSERT(m_ptrMap.tryGetValue(ptr) == nullptr);

    typedef SerialInfo::ObjectEntry ObjectEntry;

    ObjectEntry* nodeEntry = (ObjectEntry*)m_arena.allocateAligned(sizeof(ObjectEntry) + serialCls->size, SerialInfo::MAX_ALIGNMENT);

    nodeEntry->typeKind = serialCls->typeKind;
    nodeEntry->subType = serialCls->subType;
    nodeEntry->_pad0 = 0;

    nodeEntry->info = SerialInfo::makeEntryInfo(serialCls->alignment);

    // We add before adding fields, so if the fields point to this, the entry will be set
    auto index = _add(ptr, nodeEntry);

    // Point to start of payload
    uint8_t* serialPayload = (uint8_t*)(nodeEntry + 1);

    if (m_flags & Flag::ZeroInitialize)
    {
        ::memset(serialPayload, 0, serialCls->size);
    }

    while (serialCls)
    {
        for (Index i = 0; i < serialCls->fieldsCount; ++i)
        {
            auto field = serialCls->fields[i];

            // Work out the offsets
            auto srcField = ((const uint8_t*)ptr) + field.nativeOffset;
            auto dstField = serialPayload + field.serialOffset;

            field.type->toSerialFunc(this, srcField, dstField);
        }

        // Get the super class
        serialCls = serialCls->super;
    }

    return index;
}

SerialIndex SerialWriter::writeObject(const NodeBase* node)
{
    const SerialClass* serialClass = m_classes->getSerialClass(SerialTypeKind::NodeBase, SerialSubType(node->astNodeType));
    return writeObject(serialClass, (const void*)node);
}

SerialIndex SerialWriter::writeObject(const RefObject* obj)
{
    const SerialRefObject* serialObj = as<const SerialRefObject>(obj);
    if (!serialObj)
    {
        SLANG_ASSERT(!"Unhandled type");
        return SerialIndex(0);
    }

    const ReflectClassInfo* classInfo = serialObj->getClassInfo();
    SLANG_ASSERT(classInfo);

    const SerialClass* serialClass = m_classes->getSerialClass(SerialTypeKind::RefObject, SerialSubType(classInfo->m_classId));
    return writeObject(serialClass, (const void*)obj);
}

void SerialWriter::setPointerIndex(const NodeBase* ptr, SerialIndex index)
{
    m_ptrMap.add(ptr, Index(index));
}

void SerialWriter::setPointerIndex(const RefObject* ptr, SerialIndex index)
{
    m_ptrMap.add(ptr, Index(index));
}

SerialIndex SerialWriter::addPointer(const NodeBase* node)
{
    // Null is always 0
    if (node == nullptr)
    {
        return SerialIndex(0);
    }
    // Look up in the map
    Index* indexPtr = m_ptrMap.tryGetValue(node);
    if (indexPtr)
    {
        return SerialIndex(*indexPtr);
    }

    if (m_filter)
    {
        return m_filter->writePointer(this, node);
    }
    else
    {
        return writeObject(node);
    }
}

SerialIndex SerialWriter::addPointer(const RefObject* obj)
{
    // Null is always 0
    if (obj == nullptr)
    {
        return SerialIndex(0);
    }
    // Look up in the map
    Index* indexPtr = m_ptrMap.tryGetValue(obj);
    if (indexPtr)
    {
        return SerialIndex(*indexPtr);
    }

    // TODO(JS):
    // Arguably the lookup for these types should be done the same way as arbitrary RefObject types
    // and have a enum for them, such we can use a switch instead of all this casting

    if (auto stringRep = dynamicCast<StringRepresentation>(obj))
    {
        SerialIndex index = addString(StringRepresentation::asSlice(stringRep));
        m_ptrMap.add(obj, Index(index));
        return index;
    }
    else if (auto name = dynamicCast<const Name>(obj))
    {
        return addName(name);
    }

    if (m_filter)
    {
        return m_filter->writePointer(this, obj);
    }
    else
    {
        return writeObject(obj);
    }
}

SerialIndex SerialWriter::_addStringSlice(SerialTypeKind typeKind, SliceMap& sliceMap, const UnownedStringSlice& slice)
{
    typedef ByteEncodeUtil Util;
    typedef SerialInfo::StringEntry StringEntry;

    if (slice.getLength() == 0)
    {
        return SerialIndex(0);
    }

    Index* indexPtr = sliceMap.tryGetValue(slice);
    if (indexPtr)
    {
        return SerialIndex(*indexPtr);
    }

    // Okay we need to add the string

    uint8_t encodeBuf[Util::kMaxLiteEncodeUInt32];
    const int encodeCount = Util::encodeLiteUInt32(uint32_t(slice.getLength()), encodeBuf);

    StringEntry* entry = (StringEntry*)m_arena.allocateUnaligned(SLANG_OFFSET_OF(StringEntry, sizeAndChars) + encodeCount + slice.getLength());
    entry->info = SerialInfo::EntryInfo::Alignment1;
    entry->typeKind = typeKind;

    uint8_t* dst = (uint8_t*)(entry->sizeAndChars);
    for (int i = 0; i < encodeCount; ++i)
    {
        dst[i] = encodeBuf[i];
    }

    memcpy(dst + encodeCount, slice.begin(), slice.getLength());

    // Make a key that will stay in scope -> it's actually just stored in the arena.
    // NOTE! without terminating 0
    UnownedStringSlice keySlice(((const char*)dst) + encodeCount, slice.getLength());

    Index newIndex = m_entries.getCount();
    sliceMap.add(keySlice, newIndex);

    m_entries.add(entry);
    return SerialIndex(newIndex);
}

SerialIndex SerialWriter::addString(const String& in)
{
    return addPointer(in.getStringRepresentation());
}

SerialIndex SerialWriter::addName(const Name* name)
{
    if (name == nullptr)
    {
        return SerialIndex(0);
    }

    // Look it up
    Index* indexPtr = m_ptrMap.tryGetValue(name);
    if (indexPtr)
    {
        return SerialIndex(*indexPtr);
    }

    SerialIndex index = addString(name->text);
    m_ptrMap.add(name, Index(index));
    return index;
}

SerialIndex SerialWriter::addSerialArray(size_t elementSize, size_t alignment, const void* elements, Index elementCount)
{
    typedef SerialInfo::ArrayEntry Entry;

    if (elementCount == 0)
    {
        return SerialIndex(0);
    }

    SLANG_ASSERT(alignment >= 1 && alignment <= SerialInfo::MAX_ALIGNMENT);

    // We must at a minimum have the alignment for the array prefix info
    alignment = (alignment < SLANG_ALIGN_OF(Entry)) ? SLANG_ALIGN_OF(Entry) : alignment;

    size_t payloadSize = elementCount * elementSize;

    Entry* entry = (Entry*)m_arena.allocateAligned(sizeof(Entry) + payloadSize, alignment);

    entry->typeKind = SerialTypeKind::Array;
    entry->info = SerialInfo::makeEntryInfo(int(alignment));
    entry->elementSize = uint16_t(elementSize);
    entry->elementCount = uint32_t(elementCount);

    memcpy(entry + 1, elements, payloadSize);

    m_entries.add(entry);
    return SerialIndex(m_entries.getCount() - 1);
}

static const uint8_t s_fixBuffer[SerialInfo::MAX_ALIGNMENT]{ 0, };

SlangResult SerialWriter::write(Stream* stream)
{
    const Int entriesCount = m_entries.getCount();

    // Add a sentinal so we don't need special handling for 
    SerialInfo::Entry sentinal;
    sentinal.typeKind = SerialTypeKind::String;
    sentinal.info = SerialInfo::EntryInfo::Alignment1;

    m_entries.add(&sentinal);
    m_entries.removeLast();

    SerialInfo::Entry** entries = m_entries.getBuffer();
    // Note strictly required in our impl of List. But by writing this and
    // knowing that removeLast cannot release memory, means the sentinal must be at the last position.
    entries[entriesCount] = &sentinal;

    {
        size_t offset = 0;

        SerialInfo::Entry* entry = entries[1];
        // We start on 1, because 0 is nullptr and not used for anything
        for (Index i = 1; i < entriesCount; ++i)
        {
            SerialInfo::Entry* next = entries[i + 1];

            // Before writing we need to store the next alignment

            const size_t nextAlignment = SerialInfo::getAlignment(next->info);
            const size_t alignment = SerialInfo::getAlignment(entry->info);
            SLANG_UNUSED(alignment);

            entry->info = SerialInfo::combineWithNext(entry->info, next->info);

            // Check we are aligned correctly
            SLANG_ASSERT((offset & (alignment - 1)) == 0);

            // When we write, we need to make sure it take into account the next alignment
            const size_t entrySize = entry->calcSize(m_classes);

            // Work out the fix for next alignment
            size_t nextOffset = offset + entrySize;
            nextOffset = (nextOffset + nextAlignment - 1) & ~(nextAlignment - 1);

            size_t alignmentFixSize = nextOffset - (offset + entrySize);

            // The fix must be less than max alignment. We require it to be less because we aligned each Entry to 
            // MAX_ALIGNMENT, and so < MAX_ALIGNMENT is the most extra bytes we can write
            SLANG_ASSERT( alignmentFixSize < SerialInfo::MAX_ALIGNMENT);
            
            SLANG_RETURN_ON_FAIL(stream->write(entry, entrySize));
            // If we needed to fix so that subsequent alignment is right, write out extra bytes here
            if (alignmentFixSize)
            {
                SLANG_RETURN_ON_FAIL(stream->write(s_fixBuffer, alignmentFixSize));
            }
            
            // Onto next
            offset = nextOffset;
            entry = next;
        }
    }

    return SLANG_OK;
}

SlangResult SerialWriter::writeIntoContainer(FourCC fourCc, RiffContainer* container)
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ScopeChunk ScopeChunk;

    {
        ScopeChunk scopeData(container, Chunk::Kind::Data, fourCc);

        {
            // Sentinel so we don't need special handling for end of list
            SerialInfo::Entry sentinal;
            sentinal.typeKind = SerialTypeKind::String;
            sentinal.info = SerialInfo::EntryInfo::Alignment1;

            size_t offset = 0;
            const Int entriesCount = m_entries.getCount();

            {
                m_entries.add(&sentinal);
                m_entries.removeLast();
                // Note strictly required in our impl of List. But by writing this and
                // knowing that removeLast cannot release memory, means the sentinal must be at the last position.
                m_entries.getBuffer()[entriesCount] = &sentinal;
            }

            SerialInfo::Entry*const* entries = m_entries.getBuffer();

            SerialInfo::Entry* entry = entries[1];
            // We start on 1, because 0 is nullptr and not used for anything
            for (Index i = 1; i < entriesCount; ++i)
            {
                SerialInfo::Entry* next = entries[i + 1];

                // Before writing we need to store the next alignment

                const size_t nextAlignment = SerialInfo::getAlignment(next->info);
                const size_t alignment = SerialInfo::getAlignment(entry->info);
                SLANG_UNUSED(alignment);

                entry->info = SerialInfo::combineWithNext(entry->info, next->info);

                // Check we are aligned correctly
                SLANG_ASSERT((offset & (alignment - 1)) == 0);

                // When we write, we need to make sure it take into account the next alignment
                const size_t entrySize = entry->calcSize(m_classes);

                // Work out the fix for next alignment
                size_t nextOffset = offset + entrySize;
                nextOffset = (nextOffset + nextAlignment - 1) & ~(nextAlignment - 1);

                size_t alignmentFixSize = nextOffset - (offset + entrySize);

                // The fix must be less than max alignment. We require it to be less because we aligned each Entry to 
                // MAX_ALIGNMENT, and so < MAX_ALIGNMENT is the most extra bytes we can write
                SLANG_ASSERT(alignmentFixSize < SerialInfo::MAX_ALIGNMENT);

                container->write(entry, entrySize);
                if (alignmentFixSize)
                {
                    container->write(s_fixBuffer, alignmentFixSize);
                }

                // Onto next
                offset = nextOffset;
                entry = next;
            }
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialInfo::Entry  !!!!!!!!!!!!!!!!!!!!!!!!

size_t SerialInfo::Entry::calcSize(SerialClasses* serialClasses) const
{
    switch (typeKind)
    {
        case SerialTypeKind::ImportSymbol: 
        case SerialTypeKind::String:
        {
            auto entry = static_cast<const StringEntry*>(this);
            const uint8_t* cur = (const uint8_t*)entry->sizeAndChars;
            uint32_t charsSize;
            int sizeSize = ByteEncodeUtil::decodeLiteUInt32(cur, &charsSize);
            return SLANG_OFFSET_OF(StringEntry, sizeAndChars) + sizeSize + charsSize;
        }
        case SerialTypeKind::Array:
        {
            auto entry = static_cast<const ArrayEntry*>(this);
            return sizeof(ArrayEntry) + entry->elementSize * entry->elementCount;
        }
        case SerialTypeKind::RefObject:
        case SerialTypeKind::NodeBase:
        {
            auto entry = static_cast<const ObjectEntry*>(this);

            auto serialClass = serialClasses->getSerialClass(typeKind, entry->subType);

            // Align by the alignment of the entry 
            size_t alignment = getAlignment(entry->info);
            size_t size = sizeof(ObjectEntry) + serialClass->size;

            size = size + (alignment - 1) & ~(alignment - 1);
            return size;
        }
        
        default: break;
    }

    SLANG_ASSERT(!"Unknown type");
    return 0;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialReader  !!!!!!!!!!!!!!!!!!!!!!!!!!!!

SerialReader::~SerialReader()
{
    for (const RefObject* obj : m_scope)
    {
        const_cast<RefObject*>(obj)->releaseReference();
    }
}

const void* SerialReader::getArray(SerialIndex index, Index& outCount)
{
    if (index == SerialIndex(0))
    {
        outCount = 0;
        return nullptr;
    }

    SLANG_ASSERT(SerialIndexRaw(index) < SerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    switch (entry->typeKind)
    {
        case SerialTypeKind::Array:
        {
            auto arrayEntry = static_cast<const SerialInfo::ArrayEntry*>(entry);
            outCount = Index(arrayEntry->elementCount);
            return (arrayEntry + 1);
        }
        default: break;
    }

    SLANG_ASSERT(!"Not an array");
    outCount = 0;
    return nullptr;
}

SerialPointer SerialReader::getPointer(SerialIndex index)
{
    if (index == SerialIndex(0))
    {
        return SerialPointer();
    }

    SLANG_ASSERT(SerialIndexRaw(index) < SerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    const SerialPointer& ptr = m_objects[Index(index)];

    switch (entry->typeKind)
    {
        case SerialTypeKind::String:
        {
            // Hmm. Tricky -> we don't know if will be cast as Name or String. Lets assume string.
            String string = getString(index);
            return SerialPointer(string.getStringRepresentation());
        }
        case SerialTypeKind::ImportSymbol:
        {
            if (ptr.m_kind == SerialTypeKind::Unknown)
            {
                // TODO(JS):
                // Could have an error here, because import symbol was not set
                // For now just return nullptr
                return SerialPointer();
            }
            break;
        }
        default: break;
    }

    return ptr;
}

String SerialReader::getString(SerialIndex index)
{
    if (index == SerialIndex(0))
    {
        return String();
    }

    SLANG_ASSERT(SerialIndexRaw(index) < SerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    // It has to be a string type
    if (entry->typeKind != SerialTypeKind::String)
    {
        SLANG_ASSERT(!"Not a string");
        return String();
    }

    RefObject* obj = m_objects[Index(index)].dynamicCast<RefObject>();

    if (obj)
    {
        StringRepresentation* stringRep = dynamicCast<StringRepresentation>(obj);
        if (stringRep)
        {
            return String(stringRep);
        }
        // Must be a name then
        Name* name = dynamicCast<Name>(obj);
        SLANG_ASSERT(name);
        return name->text;
    }

    // Okay we need to construct as a string
    UnownedStringSlice slice = getStringSlice(index);

    StringRepresentation* stringRep = nullptr;

    const Index length = slice.getLength();
    if (length)
    {
        stringRep = StringRepresentation::createWithCapacityAndLength(length, length);
        memcpy(stringRep->getData(), slice.begin(), length * sizeof(char));
        addScope(stringRep);
    }

    m_objects[Index(index)] = stringRep;
    return String(stringRep);
}

Name* SerialReader::getName(SerialIndex index)
{
    if (index == SerialIndex(0))
    {
        return nullptr;
    }

    SLANG_ASSERT(SerialIndexRaw(index) < SerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    // It has to be a string type
    if (entry->typeKind != SerialTypeKind::String)
    {
        SLANG_ASSERT(!"Not a string");
        return nullptr;
    }

    RefObject* obj = m_objects[Index(index)].dynamicCast<RefObject>();

    if (obj)
    {
        Name* name = dynamicCast<Name>(obj);
        if (name)
        {
            return name;
        }
        // Can only be a string then
        StringRepresentation* stringRep = dynamicCast<StringRepresentation>(obj);
        SLANG_ASSERT(stringRep);

        // I don't need to scope, as scoped in NamePool
        name = m_namePool->getName(String(stringRep));

        // Store as name, as can always access the inner string if needed
        m_objects[Index(index)] = name;
        return name;
    }

    UnownedStringSlice slice = getStringSlice(index);
    String string(slice);
    Name* name = m_namePool->getName(string);
    // Don't need to add to scope, because scoped on the pool
    m_objects[Index(index)] = name;
    return name;
}

UnownedStringSlice SerialReader::getStringSlice(SerialIndex index)
{
    SLANG_ASSERT(SerialIndexRaw(index) < SerialIndexRaw(m_entries.getCount()));
    const Entry* entry = m_entries[Index(index)];

    // It has to be a string type
    if (entry->typeKind == SerialTypeKind::String ||
        entry->typeKind == SerialTypeKind::ImportSymbol)
    {
        auto stringEntry = static_cast<const SerialInfo::StringEntry*>(entry);

        const uint8_t* src = (const uint8_t*)stringEntry->sizeAndChars;

        // Decode the string
        uint32_t size;
        int sizeSize = ByteEncodeUtil::decodeLiteUInt32(src, &size);
        return UnownedStringSlice((const char*)src + sizeSize, size);
    }

    // Can't be accessed as a slice
    SLANG_ASSERT(!"Not accessible as a slice");
    return UnownedStringSlice();
}

/* static */SlangResult SerialReader::loadEntries(const uint8_t* data, size_t dataCount, SerialClasses* serialClasses, List<const Entry*>& outEntries)
{
    // Check the input data is at least aligned to the max alignment (otherwise everything cannot be aligned correctly)
    SLANG_ASSERT((size_t(data) & (SerialInfo::MAX_ALIGNMENT - 1)) == 0);

    outEntries.setCount(1);
    outEntries[0] = nullptr;

    const uint8_t*const end = data + dataCount;

    const uint8_t* cur = data;
    while (cur < end)
    {
        const Entry* entry = (const Entry*)cur;
        outEntries.add(entry);

        const size_t entrySize = entry->calcSize(serialClasses);
        cur += entrySize;

        // Need to get the next alignment
        const size_t nextAlignment = SerialInfo::getNextAlignment(entry->info);

        // Need to fix cur with the alignment
        cur = (const uint8_t*)((size_t(cur) + nextAlignment - 1) & ~(nextAlignment - 1));
    }

    return SLANG_OK;
}

SlangResult SerialReader::constructObjects(NamePool* namePool)
{
    m_namePool = namePool;

    m_objects.clearAndDeallocate();
    m_objects.setCount(m_entries.getCount());
    memset(m_objects.getBuffer(), 0, m_objects.getCount() * sizeof(void*));

    // Go through entries, constructing objects.
    for (Index i = 1; i < m_entries.getCount(); ++i)
    {
        const Entry* entry = m_entries[i];

        switch (entry->typeKind)
        {
            case SerialTypeKind::ImportSymbol:
            {
                // We don't construct any object for an imported symbol.
                // It will be the responsibility of external code to interpet the symbols and *set* the appopriate
                // objects prior to a call to `deserializeObjects`
                break;
            }
            case SerialTypeKind::String:
            {
                // Don't need to construct an object. This is probably a StringRepresentation, or a Name
                // Will evaluate lazily.
                break;
            }
            case SerialTypeKind::RefObject:
            case SerialTypeKind::NodeBase:
            {
                auto objectEntry = static_cast<const SerialInfo::ObjectEntry*>(entry);
                void* obj = m_objectFactory->create(objectEntry->typeKind, objectEntry->subType);
                if (!obj)
                {
                    return SLANG_FAIL;
                }
                m_objects[i].set(entry->typeKind, obj);
                break;
            }
            case SerialTypeKind::Array:
            {
                // Don't need to construct an object, as will be accessed an interpreted by the object that holds it
                break;
            }
        }
    }

    return SLANG_OK;
}

SlangResult SerialReader::deserializeObjects()
{
    // Deserialize
    for (Index i = 1; i < m_entries.getCount(); ++i)
    {
        const Entry* entry = m_entries[i];
        // First see if there is anything to construct
        SerialPointer& dstPtr = m_objects[i];
        if (!dstPtr)
        {
            continue;
        }
        switch (entry->typeKind)
        {
            case SerialTypeKind::NodeBase:
            case SerialTypeKind::RefObject:
            {
                auto objectEntry = static_cast<const SerialInfo::ObjectEntry*>(entry);
                auto serialClass = m_classes->getSerialClass(objectEntry->typeKind, objectEntry->subType);
                if (!serialClass)
                {
                    return SLANG_FAIL;
                }

                const uint8_t* src = (const uint8_t*)(objectEntry + 1);
                uint8_t* dst = (uint8_t*)dstPtr.m_ptr;

                // It must be constructed
                SLANG_ASSERT(dst);

                while (serialClass)
                {
                    for (Index j = 0; j < serialClass->fieldsCount; ++j)
                    {
                        auto field = serialClass->fields[j];
                        auto fieldType = field.type;
                        fieldType->toNativeFunc(this, src + field.serialOffset, dst + field.nativeOffset);
                    }

                    // Get the super class
                    serialClass = serialClass->super;
                }

                break;
            }
            default: break;
        }
    }

    return SLANG_OK;
}


SlangResult SerialReader::load(const uint8_t* data, size_t dataCount, NamePool* namePool)
{
    // Load and place entries into entries table
    SLANG_RETURN_ON_FAIL(loadEntries(data, dataCount));
    // Construct all of the objects
    SLANG_RETURN_ON_FAIL(constructObjects(namePool));
    SLANG_RETURN_ON_FAIL(deserializeObjects());
    return SLANG_OK;
}

} // namespace Slang

// slang-json-value.cpp
#include "slang-json-value.h"

#include "../core/slang-string-escape-util.h"
#include "../core/slang-string-util.h"

namespace Slang {

static JSONKeyValue _makeInvalidKeyValue()
{
    JSONKeyValue keyValue;
    keyValue.key = JSONKey(0);
    keyValue.value.type = JSONValue::Type::Invalid;
    return keyValue;
}

/* static */JSONKeyValue g_invalid = _makeInvalidKeyValue();

JSONContainer::JSONContainer(SourceManager* sourceManager):
    m_slicePool(StringSlicePool::Style::Default),
    m_sourceManager(sourceManager)
{
    // Index 0 is the empty array or object
    {
        Range range;

        range.type = Range::Type::None;
        range.startIndex = 0;
        range.count = 0;
        range.capacity = 0;

        m_ranges.add(range);
    }
}

/* static */bool JSONContainer::areKeysUnique(const JSONKeyValue* keyValues, Index keyValueCount)
{
    for (Index i = 1; i < keyValueCount; ++i)
    {
        const JSONKey key = keyValues[i].key;

        for (Int j = 0; j < i - 1; j++)
        {
            if (keyValues[j].key == key)
            {
                return false;
            }
        }
    }

    return true;
}

Index JSONContainer::_addRange()
{
    if (m_freeRangeIndices.getCount() > 0)
    {
        const Index rangeIndex = m_freeRangeIndices.getLast();
        m_freeRangeIndices.removeLast();
        return rangeIndex;
    }
    else
    {
        m_ranges.add(Range{});
        return m_ranges.getCount() - 1;
    }
}

JSONValue JSONContainer::makeArray(const JSONValue* values, Index valuesCount, SourceLoc loc)
{
    if (valuesCount <= 0)
    {
        return JSONValue::makeEmptyArray(loc);
    }

    const Index rangeIndex = _addRange();
    Range& range = m_ranges[rangeIndex];

    range.type = Range::Type::Array;
    range.startIndex = m_objectValues.getCount();
    range.count = valuesCount;
    range.capacity = valuesCount;

    m_arrayValues.addRange(values, valuesCount);

    JSONValue value;
    value.type = JSONValue::Type::Object;
    value.loc = loc;
    value.rangeIndex = rangeIndex;

    return value;
}

JSONValue JSONContainer::makeObject(const JSONKeyValue* keyValues, Index keyValueCount, SourceLoc loc)
{
    if (keyValueCount <= 0)
    {
        return JSONValue::makeEmptyObject(loc);
    }

    const Index rangeIndex = _addRange();
    Range& range = m_ranges[rangeIndex];

    range.type = Range::Type::Object;
    range.startIndex = m_objectValues.getCount();
    range.count = keyValueCount;
    range.capacity = keyValueCount;

    m_objectValues.addRange(keyValues, keyValueCount);

    JSONValue value;
    value.type = JSONValue::Type::Object;
    value.loc = loc;
    value.rangeIndex = rangeIndex;

    return value;
}

JSONValue JSONContainer::makeString(const UnownedStringSlice& slice, SourceLoc loc)
{
    JSONValue value;
    value.type = JSONValue::Type::StringValue;
    value.loc = loc;
    value.stringKey = getKey(slice);
    return value;
}

JSONKey JSONContainer::getKey(const UnownedStringSlice& slice)
{
    return JSONKey(m_slicePool.add(slice));
}

ConstArrayView<JSONValue> JSONContainer::getArray(const JSONValue& in) const
{
    SLANG_ASSERT(in.type == JSONValue::Type::Array);
    if (in.type != JSONValue::Type::Array || in.rangeIndex == 0)
    {
        return ConstArrayView<JSONValue>((const JSONValue*)nullptr, 0);
    }
    const Range& range = m_ranges[in.rangeIndex];
    return ConstArrayView<JSONValue>(m_arrayValues.getBuffer() + range.startIndex, range.count);
}

ConstArrayView<JSONKeyValue> JSONContainer::getObject(const JSONValue& in) const
{
    SLANG_ASSERT(in.type == JSONValue::Type::Array);
    if (in.type != JSONValue::Type::Array || in.rangeIndex == 0)
    {
        return ConstArrayView<JSONKeyValue>((const JSONKeyValue*)nullptr, 0);
    }

    const Range& range = m_ranges[in.rangeIndex];
    return ConstArrayView<JSONKeyValue>(m_objectValues.getBuffer() + range.startIndex, range.count);
}

ArrayView<JSONValue> JSONContainer::getArray(const JSONValue& in)
{
    SLANG_ASSERT(in.type == JSONValue::Type::Array);
    if (in.type != JSONValue::Type::Array || in.rangeIndex == 0)
    {
        return ArrayView<JSONValue>((JSONValue*)nullptr, 0);
    }
    const Range& range = m_ranges[in.rangeIndex];
    return ArrayView<JSONValue>(m_arrayValues.getBuffer() + range.startIndex, range.count);
}

ArrayView<JSONKeyValue> JSONContainer::getObject(const JSONValue& in)
{
    SLANG_ASSERT(in.type == JSONValue::Type::Array);
    if (in.type != JSONValue::Type::Array || in.rangeIndex == 0)
    {
        return ArrayView<JSONKeyValue>((JSONKeyValue*)nullptr, 0);
    }

    const Range& range = m_ranges[in.rangeIndex];
    return ArrayView<JSONKeyValue>(m_objectValues.getBuffer() + range.startIndex, range.count);
}

UnownedStringSlice JSONContainer::getLexeme(const JSONValue& in)
{
    SLANG_ASSERT(JSONValue::isLexeme(in.type));
    if (!JSONValue::isLexeme(in.type))
    {
        return UnownedStringSlice();
    }

    if (!(m_currentView && m_currentView->getRange().contains(in.loc)))
    {
        m_currentView = m_sourceManager->findSourceView(in.loc);
        if (!m_currentView)
        {
            return UnownedStringSlice();
        }
    }

    const auto offset = m_currentView->getRange().getOffset(in.loc);
    SourceFile* sourceFile = m_currentView->getSourceFile();

    return UnownedStringSlice(sourceFile->getContent().begin() + offset, in.length);
}

UnownedStringSlice JSONContainer::getString(const JSONValue& in)
{
    switch (in.type)
    {
        case JSONValue::Type::StringValue:  return getStringFromKey(in.stringKey);
        case JSONValue::Type::StringLexeme:
        {
            StringEscapeHandler* handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON);

            UnownedStringSlice lexeme = getLexeme(in);
            UnownedStringSlice unquoted = StringEscapeUtil::unquote(handler, lexeme);

            if (handler->isUnescapingNeeeded(unquoted))
            {
                m_buf.Clear();
                handler->appendUnescaped(unquoted, m_buf);
                return m_buf.getUnownedSlice();
            }
            else
            {
                return unquoted;
            }
        }
    }

    SLANG_ASSERT(!"Not a string type");
    return UnownedStringSlice();
}

bool JSONContainer::asBool(const JSONValue& value)
{
    switch (value.type)
    {
        case JSONValue::Type::True:             return true;
        case JSONValue::Type::False:
        case JSONValue::Type::Null:
        {
            return false;
        }
        case JSONValue::Type::IntegerValue:     return value.intValue != 0;
        case JSONValue::Type::FloatValue:       return value.floatValue != 0;
        case JSONValue::Type::IntegerLexeme:    return asInteger(value) != 0;
        case JSONValue::Type::FloatLexeme:      return asFloat(value) != 0.0;
        default: break;
    }
    SLANG_ASSERT(!"Not bool convertable");
    return false;
}

int64_t JSONContainer::asInteger(const JSONValue& value)
{
    switch (value.type)
    {
        case JSONValue::Type::True:             return 1;
        case JSONValue::Type::False:
        case JSONValue::Type::Null:
        {
            return 0;
        }
        case JSONValue::Type::IntegerValue:     return value.intValue;
        case JSONValue::Type::FloatValue:       return int64_t(value.floatValue);
        case JSONValue::Type::IntegerLexeme:
        {
            UnownedStringSlice slice = getLexeme(value);

            int64_t intValue;
            if (SLANG_SUCCEEDED(StringUtil::parseInt64(slice, intValue)) && slice.getLength() == 0)
            {
                return intValue;
            }

            SLANG_ASSERT(!"Couldn't convert int");
            return 0;
        }
        case JSONValue::Type::FloatLexeme:      return int64_t(asFloat(value));
        default: break;
    }
    SLANG_ASSERT(!"Not int convertable");
    return 0;
}

double JSONContainer::asFloat(const JSONValue& value)
{
    switch (value.type)
    {
        case JSONValue::Type::True:             return 1.0;
        case JSONValue::Type::False:
        case JSONValue::Type::Null:
        {
            return 0.0;
        }
        case JSONValue::Type::IntegerValue:     return double(value.intValue);
        case JSONValue::Type::FloatValue:       return value.floatValue;
        case JSONValue::Type::IntegerLexeme:    return double(asInteger(value));
        case JSONValue::Type::FloatLexeme:  
        {
            UnownedStringSlice slice = getLexeme(value);
            double floatValue;
            if (SLANG_SUCCEEDED(StringUtil::parseDouble(slice, floatValue)) && slice.getLength() == 0)
            {
                return floatValue;
            }
            SLANG_ASSERT(!"Couldn't convert double");
            return 0.0;
        }
        default: break;
    }
    SLANG_ASSERT(!"Not float convertable");
    return 0;
}

JSONValue& JSONContainer::getAt(const JSONValue& array, Index index)
{
    SLANG_ASSERT(array.type == JSONValue::Type::Array);
    const Range& range = m_ranges[array.rangeIndex];

    SLANG_ASSERT(index >= 0 && index < range.count);
    return m_arrayValues[range.startIndex + index];
}

void JSONContainer::addToArray(JSONValue& array, const JSONValue& value)
{
    SLANG_ASSERT(array.type == JSONValue::Type::Array);
    if (array.type != JSONValue::Type::Array)
    {
        return;
    }

    // It's empty
    if (array.rangeIndex == 0)
    {
        // We can just add to the end
        array.rangeIndex = _addRange();
        Range& range = m_ranges[array.rangeIndex];
        range.startIndex = m_arrayValues.getCount();
        range.count = 1;
        range.capacity = 1;
        m_arrayValues.add(value);
        return;
    }

    Range& range = m_ranges[array.rangeIndex];
    if (range.count < range.capacity)
    {
        m_arrayValues[range.startIndex + range.count++] = value;
        return;
    }

    // Are we at the end of the total array
    if (range.capacity + range.startIndex == m_arrayValues.getCount())
    {
        m_arrayValues.add(value);
        range.count++;
        range.capacity++;
        return;
    }

    const Index newStartIndex = m_arrayValues.getCount();

    // So there's no place to add. We want to move to the end with an extra space.
    m_arrayValues.growToCount(newStartIndex + range.count + 1);

    auto buffer = m_arrayValues.getBuffer();
    ::memmove(buffer + newStartIndex, buffer + range.startIndex, sizeof(*buffer) * range.count);

    buffer[newStartIndex + range.count] = value;

    range.startIndex = newStartIndex;
    range.count++;
    range.capacity++;
}

Index JSONContainer::findKeyGlobalIndex(const JSONValue& obj, JSONKey key)
{
    SLANG_ASSERT(obj.type == JSONValue::Type::Object);
    if (obj.type != JSONValue::Type::Object)
    {
        return -1;
    }

    auto buf = m_objectValues.getBuffer();

    const Range& range = m_ranges[obj.rangeIndex];
    for (Index i = range.startIndex; i < range.startIndex + range.count; ++i)
    {
        if (buf[i].key == key)
        {
            return i;
        }
    }

    return -1;
}

Index JSONContainer::findKeyGlobalIndex(const JSONValue& obj, const UnownedStringSlice& slice)
{
    Index keyIndex = m_slicePool.findIndex(slice);
    if (keyIndex < 0)
    {
        return -1;
    }

    return findKeyGlobalIndex(obj, JSONKey(keyIndex));
}

void JSONContainer::_removeKey(JSONValue& obj, Index globalIndex)
{
    Range& range = m_ranges[obj.rangeIndex];
    const auto localIndex = globalIndex + range.startIndex;

    if (localIndex < range.count - 1)
    {
        auto localBuf = m_objectValues.getBuffer() + range.startIndex;
        ::memmove(localBuf + localIndex, localBuf + localIndex + 1, sizeof(*localBuf) * (range.count - (localIndex + 1)));
    }

    --range.count;
}

bool JSONContainer::removeKey(JSONValue& obj, JSONKey key)
{
    const Index globalIndex = findKeyGlobalIndex(obj, key);
    if (globalIndex >= 0)
    {
        _removeKey(obj, globalIndex);
        return true;
    }
    return false;
}

bool JSONContainer::removeKey(JSONValue& obj, const UnownedStringSlice& slice)
{
    const Index globalIndex = findKeyGlobalIndex(obj, slice);
    if (globalIndex >= 0)
    {
        _removeKey(obj, globalIndex);
        return true;
    }
    return false;
}

void JSONContainer::setKeyValue(JSONValue& obj, JSONKey key, const JSONValue& value, SourceLoc loc)
{
    SLANG_ASSERT(obj.type == JSONValue::Type::Object);
    if (obj.type != JSONValue::Type::Object)
    {
        return;
    }

    if (obj.rangeIndex == 0)
    {
        // We need a new range and add to the end
        obj.rangeIndex = _addRange();
        Range& range = m_ranges[obj.rangeIndex];
        range.type = Range::Type::Object;
        range.count = 1;
        range.capacity = 1;
        range.startIndex = m_objectValues.getCount();

        m_objectValues.add(JSONKeyValue{key, loc, value});
        return;
    }

    const Index globalIndex = findKeyGlobalIndex(obj, key);
    if (globalIndex >= 0)
    {
        auto& keyValue = m_objectValues[globalIndex];
        SLANG_ASSERT(keyValue.key == key);
        keyValue.value = value;
        keyValue.keyLoc = loc;
        return;
    }

    // If we have capacity, we can add to the end
    Range& range = m_ranges[obj.rangeIndex];
    if (range.count < range.capacity)
    {
        auto& dst = m_objectValues[range.startIndex + range.count++];
        
        dst.key = key;
        dst.keyLoc = loc;
        dst.value = value;
        return;
    }

    // If we are at the end, we can just add
    if (range.startIndex + range.capacity == m_objectValues.getCount())
    {
        m_objectValues.add(JSONKeyValue{key, loc, value});
        range.capacity++;
        range.count++;
        return;
    }

    // Okay we have no choice but to make new space at the end
    // So there's no place to add. We want to move to the end with an extra space.

    const Index newStartIndex = m_objectValues.getCount();
    m_objectValues.growToCount(newStartIndex + range.count + 1);

    auto buffer = m_objectValues.getBuffer();
    ::memmove(buffer + newStartIndex, buffer + range.startIndex, sizeof(*buffer) * range.count);

    {
        auto& dst = buffer[newStartIndex + range.count];
        dst.key = key;
        dst.keyLoc = loc;
        dst.value = value;
    }

    range.startIndex = newStartIndex;
    range.count++;
    range.capacity++;
}


} // namespace Slang

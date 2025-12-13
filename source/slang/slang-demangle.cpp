#include "slang-demangle.h"

#include "../core/slang-string.h"

#include <ctype.h>

namespace Slang
{
namespace
{
struct DemangleParser
{
    UnownedStringSlice input;
    Index pos = 0;

    explicit DemangleParser(const UnownedStringSlice& in)
        : input(in)
    {
    }

    bool atEnd() const { return pos >= input.getLength(); }
    char peek() const { return atEnd() ? 0 : input[pos]; }
    bool isDigitChar(char c) const { return !!isdigit(static_cast<unsigned char>(c)); }

    bool consume(char c)
    {
        if (peek() != c)
            return false;
        ++pos;
        return true;
    }

    bool consumeStr(const char* text)
    {
        Index len = 0;
        while (text[len])
            ++len;
        if (pos + len > input.getLength())
            return false;
        for (Index i = 0; i < len; ++i)
        {
            if (input[pos + i] != text[i])
                return false;
        }
        pos += len;
        return true;
    }

    bool parseUInt(UInt& outValue)
    {
        if (atEnd() || !isDigitChar(peek()))
            return false;
        UInt value = 0;
        while (!atEnd() && isDigitChar(peek()))
        {
            value = value * 10 + UInt(peek() - '0');
            ++pos;
        }
        outValue = value;
        return true;
    }

    bool parseHexByte(UnownedStringSlice slice, unsigned& outValue)
    {
        unsigned value = 0;
        for (auto c : slice)
        {
            unsigned digit = 0;
            if ('0' <= c && c <= '9')
            {
                digit = unsigned(c - '0');
            }
            else if ('a' <= c && c <= 'f')
            {
                digit = 10u + unsigned(c - 'a');
            }
            else if ('A' <= c && c <= 'F')
            {
                digit = 10u + unsigned(c - 'A');
            }
            else
            {
                return false;
            }
            value = (value << 4) | digit;
            if (value > 0xFF)
                return false;
        }
        outValue = value;
        return true;
    }

    bool parseName(String& outName)
    {
        if (consume('R'))
        {
            UInt length = 0;
            if (!parseUInt(length))
                return false;
            Index remaining = input.getLength() - pos;
            if (length > UInt(remaining))
                return false;

            auto encoded = input.subString(pos, length);
            pos += length;

            StringBuilder decoded;
            for (Index i = 0; i < encoded.getLength(); ++i)
            {
                auto c = encoded[i];
                if (c == '_')
                {
                    if (i + 1 < encoded.getLength() && encoded[i + 1] == 'u')
                    {
                        decoded.appendChar('_');
                        ++i;
                        continue;
                    }
                    Index start = i + 1;
                    Index end = start;
                    while (end < encoded.getLength() && encoded[end] != 'x')
                        ++end;
                    if (end >= encoded.getLength() || start == end)
                        return false;

                    unsigned byteValue = 0;
                    if (!parseHexByte(encoded.subString(start, end - start), byteValue))
                        return false;
                    decoded.appendChar(char(byteValue));
                    i = end;
                }
                else
                {
                    decoded.appendChar(c);
                }
            }
            outName = decoded.produceString();
            return true;
        }

        UInt length = 0;
        if (!parseUInt(length))
            return false;
        Index remaining = input.getLength() - pos;
        if (length > UInt(remaining))
            return false;
        outName = String(input.subString(pos, length));
        pos += length;
        return true;
    }

    bool parseSimpleInt(String& outValue)
    {
        if (!atEnd() && isDigitChar(peek()))
        {
            StringBuilder builder;
            builder.appendChar(peek());
            ++pos;
            outValue = builder.produceString();
            return true;
        }
        return tryParseVal(outValue);
    }

    bool tryParseType(String& outType)
    {
        Index start = pos;
        StringBuilder builder;
        if (!parseType(builder))
        {
            pos = start;
            return false;
        }
        outType = builder.produceString();
        return true;
    }

    bool tryParseVal(String& outVal)
    {
        Index start = pos;
        StringBuilder builder;
        if (!parseVal(builder))
        {
            pos = start;
            return false;
        }
        outVal = builder.produceString();
        return true;
    }

    bool parseNamePath(List<String>& components)
    {
        bool consumed = false;
        while (!atEnd())
        {
            if (consumeStr("GP"))
            {
                UInt index = 0;
                if (!parseUInt(index))
                    return false;
                StringBuilder builder;
                builder << "GenericParam" << index;
                components.add(builder.produceString());
                consumed = true;
                continue;
            }

            if (consume('X'))
            {
                String targetType;
                if (!tryParseType(targetType))
                    return false;
                List<String> inherits;
                while (consume('I'))
                {
                    String sup;
                    if (!tryParseType(sup))
                        return false;
                    inherits.add(sup);
                }

                StringBuilder builder;
                builder << "extension(" << targetType;
                if (inherits.getCount())
                {
                    builder.append(" : ");
                    for (Index i = 0; i < inherits.getCount(); ++i)
                    {
                        if (i)
                            builder.append(", ");
                        builder.append(inherits[i]);
                    }
                }
                builder.appendChar(')');
                components.add(builder.produceString());
                consumed = true;
                continue;
            }

            if (consume('I'))
            {
                String sup;
                if (!tryParseType(sup))
                    return false;
                StringBuilder builder;
                builder << "implements(" << sup << ")";
                components.add(builder.produceString());
                consumed = true;
                continue;
            }

            if (!atEnd() && (isDigitChar(peek()) || peek() == 'R'))
            {
                String name;
                if (!parseName(name))
                    return false;
                components.add(name);
                consumed = true;
                continue;
            }
            break;
        }
        return consumed;
    }

    bool parseAccessorSuffix(StringBuilder& target)
    {
        if (consumeStr("Ag"))
        {
            target.append(".getter");
            return true;
        }
        if (consumeStr("As"))
        {
            target.append(".setter");
            return true;
        }
        if (consumeStr("Ar"))
        {
            target.append(".ref");
            return true;
        }
        return false;
    }

    bool parseGenericArgs(StringBuilder& suffixOut)
    {
        UInt argCount = 0;
        if (!parseUInt(argCount))
            return false;

        List<String> args;
        for (UInt i = 0; i < argCount; ++i)
        {
            String arg;
            if (!tryParseVal(arg))
                return false;
            args.add(arg);
        }

        suffixOut.appendChar('<');
        for (Index i = 0; i < args.getCount(); ++i)
        {
            if (i)
                suffixOut.append(", ");
            suffixOut.append(args[i]);
        }
        suffixOut.appendChar('>');
        return true;
    }

    bool parseGenericParams(StringBuilder& suffixOut, StringBuilder& whereOut)
    {
        UInt paramCount = 0;
        if (!parseUInt(paramCount))
            return false;

        List<String> params;
        UInt typeParamIndex = 0;
        UInt typePackIndex = 0;
        UInt valueParamIndex = 0;
        UInt parsedCount = 0;
        while (parsedCount < paramCount && (peek() == 'T' || peek() == 'v'))
        {
            if (consume('T'))
            {
                if (consume('P'))
                {
                    StringBuilder builder;
                    builder << "TP" << typePackIndex++;
                    params.add(builder.produceString());
                }
                else
                {
                    StringBuilder builder;
                    builder << "T" << typeParamIndex++;
                    params.add(builder.produceString());
                }
                ++parsedCount;
            }
            else if (consume('v'))
            {
                String type;
                if (!tryParseType(type))
                    return false;
                StringBuilder builder;
                builder << "V" << valueParamIndex++ << ":" << type;
                params.add(builder.produceString());
                ++parsedCount;
            }
        }

        while (consume('C'))
        {
            String keyType;
            if (!tryParseType(keyType))
                return false;

            List<String> constraints;
            String constraint;
            if (!tryParseType(constraint))
                return false;
            constraints.add(constraint);
            while (consume('_'))
            {
                if (!tryParseType(constraint))
                    return false;
                constraints.add(constraint);
            }

            StringBuilder clause;
            clause << keyType << " : ";
            for (Index i = 0; i < constraints.getCount(); ++i)
            {
                if (i)
                    clause.append(", ");
                clause.append(constraints[i]);
            }
            if (whereOut.getLength())
                whereOut.append("; ");
            whereOut.append(clause);
        }

        if (params.getCount())
        {
            suffixOut.appendChar('<');
            for (Index i = 0; i < params.getCount(); ++i)
            {
                if (i)
                    suffixOut.append(", ");
                suffixOut.append(params[i]);
            }
            suffixOut.appendChar('>');
        }

        return true;
    }

    bool parseParameterBlock(StringBuilder& outSignature)
    {
        if (!consume('p'))
            return false;

        UInt paramCount = 0;
        if (!parseUInt(paramCount))
            return false;
        if (!consume('p'))
            return false;

        List<String> params;
        for (UInt i = 0; i < paramCount; ++i)
        {
            String direction;
            if (consumeStr("r_"))
                direction = "ref";
            else if (consumeStr("c_"))
                direction = "borrow";
            else if (consumeStr("o_"))
                direction = "out";
            else if (consumeStr("io_"))
                direction = "borrow_inout";
            else if (consumeStr("i_"))
                direction = "in";
            else
                return false;

            String type;
            if (!tryParseType(type))
                return false;
            StringBuilder param;
            if (direction.getLength())
            {
                param.append(direction);
                param.appendChar(' ');
            }
            param.append(type);
            params.add(param.produceString());
        }

        String returnType;
        tryParseType(returnType);

        List<String> modifiers;
        while (!atEnd())
        {
            char flag = peek();
            if (flag == 'm')
            {
                modifiers.add("mutating");
                ++pos;
            }
            else if (flag == 'r')
            {
                modifiers.add("ref-this");
                ++pos;
            }
            else if (flag == 'f')
            {
                modifiers.add("fwd-diff");
                ++pos;
            }
            else if (flag == 'b')
            {
                modifiers.add("bwd-diff");
                ++pos;
            }
            else if (flag == 'n')
            {
                modifiers.add("nodiff-this");
                ++pos;
            }
            else
            {
                break;
            }
        }

        outSignature.appendChar('(');
        for (Index i = 0; i < params.getCount(); ++i)
        {
            if (i)
                outSignature.append(", ");
            outSignature.append(params[i]);
        }
        outSignature.appendChar(')');

        if (returnType.getLength())
        {
            outSignature.append(" -> ");
            outSignature.append(returnType);
        }

        if (modifiers.getCount())
        {
            outSignature.append(" [");
            for (Index i = 0; i < modifiers.getCount(); ++i)
            {
                if (i)
                    outSignature.append(", ");
                outSignature.append(modifiers[i]);
            }
            outSignature.appendChar(']');
        }
        return true;
    }

    bool parseType(StringBuilder& outType)
    {
        if (atEnd())
            return false;

        const char c = peek();
        switch (c)
        {
        case 'V':
            outType.append("void");
            ++pos;
            return true;
        case 'b':
            outType.append("bool");
            ++pos;
            return true;
        case 'c':
            outType.append("int8");
            ++pos;
            return true;
        case 's':
            outType.append("int16");
            ++pos;
            return true;
        case 'i':
            if (pos + 1 < input.getLength() && input[pos + 1] == 'p')
            {
                pos += 2;
                outType.append("intptr");
            }
            else
            {
                ++pos;
                outType.append("int");
            }
            return true;
        case 'I':
            outType.append("int64");
            ++pos;
            return true;
        case 'C':
            outType.append("uint8");
            ++pos;
            return true;
        case 'S':
            outType.append("uint16");
            ++pos;
            return true;
        case 'u':
            if (pos + 1 < input.getLength() && input[pos + 1] == 'p')
            {
                pos += 2;
                outType.append("uintptr");
            }
            else
            {
                ++pos;
                outType.append("uint");
            }
            return true;
        case 'U':
            outType.append("uint64");
            ++pos;
            return true;
        case 'h':
            outType.append("half");
            ++pos;
            return true;
        case 'f':
            outType.append("float");
            ++pos;
            return true;
        case 'd':
            outType.append("double");
            ++pos;
            return true;
        case 'v':
        {
            ++pos;
            String count;
            if (!parseSimpleInt(count))
                return false;
            String elementType;
            if (!tryParseType(elementType))
                return false;
            outType << "vector<" << elementType << ", " << count << ">";
            return true;
        }
        case 'm':
        {
            ++pos;
            String rows;
            if (!parseSimpleInt(rows))
                return false;
            if (!consume('x'))
                return false;
            String cols;
            if (!parseSimpleInt(cols))
                return false;
            String elementType;
            if (!tryParseType(elementType))
                return false;
            outType << "matrix<" << elementType << ", " << rows << ", " << cols << ">";
            return true;
        }
        case 'a':
        {
            ++pos;
            String count;
            if (!parseSimpleInt(count))
                return false;
            String elementType;
            if (!tryParseType(elementType))
                return false;
            outType << "array<" << elementType << ", " << count << ">";
            return true;
        }
        case 't':
        {
            ++pos;
            StringBuilder target;
            if (!parseEntityCore(target))
                return false;
            outType << "ThisType<" << target.produceString() << ">";
            return true;
        }
        case 'E':
            outType.append("error");
            ++pos;
            return true;
        case 'B':
            outType.append("bottom");
            ++pos;
            return true;
        case 'F':
        {
            ++pos;
            UInt paramCount = 0;
            if (!parseUInt(paramCount))
                return false;
            List<String> params;
            for (UInt i = 0; i < paramCount; ++i)
            {
                String paramType;
                if (!tryParseType(paramType))
                    return false;
                params.add(paramType);
            }
            String resultType;
            if (!tryParseType(resultType))
                return false;
            String errorType;
            if (!tryParseType(errorType))
                return false;

            outType.append("func(");
            for (Index i = 0; i < params.getCount(); ++i)
            {
                if (i)
                    outType.append(", ");
                outType.append(params[i]);
            }
            outType.append(") -> ");
            outType.append(resultType);
            if (errorType != "bottom")
            {
                outType.append(" throws ");
                outType.append(errorType);
            }
            return true;
        }
        case 'T':
        {
            ++pos;
            if (consume('u'))
            {
                UInt count = 0;
                if (!parseUInt(count))
                    return false;
                List<String> members;
                for (UInt i = 0; i < count; ++i)
                {
                    String member;
                    if (!tryParseType(member))
                        return false;
                    members.add(member);
                }

                outType.append("tuple<");
                for (Index i = 0; i < members.getCount(); ++i)
                {
                    if (i)
                        outType.append(", ");
                    outType.append(members[i]);
                }
                outType.appendChar('>');
                return true;
            }
            if (consume('m'))
            {
                String baseType;
                if (!tryParseType(baseType))
                    return false;

                UInt modifierCount = 0;
                if (!parseUInt(modifierCount))
                    return false;
                List<String> modifiers;
                for (UInt i = 0; i < modifierCount; ++i)
                {
                    String modifier;
                    if (!tryParseVal(modifier))
                        return false;
                    modifiers.add(modifier);
                }
                outType.append("modified<");
                outType.append(baseType);
                if (modifiers.getCount())
                {
                    outType.append(", ");
                    for (Index i = 0; i < modifiers.getCount(); ++i)
                    {
                        if (i)
                            outType.append(", ");
                        outType.append(modifiers[i]);
                    }
                }
                outType.appendChar('>');
                return true;
            }
            if (consume('a'))
            {
                String left;
                if (!tryParseType(left))
                    return false;
                String right;
                if (!tryParseType(right))
                    return false;
                outType << "(" << left << " & " << right << ")";
                return true;
            }
            if (consume('x'))
            {
                String pattern;
                if (!tryParseType(pattern))
                    return false;
                outType << "expand(" << pattern << ")";
                return true;
            }
            if (consume('e'))
            {
                String element;
                if (!tryParseType(element))
                    return false;
                outType << "each(" << element << ")";
                return true;
            }
            if (consume('p'))
            {
                UInt count = 0;
                if (!parseUInt(count))
                    return false;
                List<String> elements;
                for (UInt i = 0; i < count; ++i)
                {
                    String element;
                    if (!tryParseType(element))
                        return false;
                    elements.add(element);
                }
                outType.append("pack<");
                for (Index i = 0; i < elements.getCount(); ++i)
                {
                    if (i)
                        outType.append(", ");
                    outType.append(elements[i]);
                }
                outType.appendChar('>');
                return true;
            }
            --pos;
            break;
        }
        default:
            break;
        }

        // Treat anything else that looks like a declaration reference as a named type.
        if (!atEnd())
        {
            // Heuristic: names and extensions start with either digit/`R` or the special
            // markers handled by parseNamePath.
            if (isDigitChar(peek()) || peek() == 'R' || peek() == 'X' || peek() == 'I' ||
                peek() == 'G' || peek() == 'g')
            {
                StringBuilder name;
                if (!parseEntityCore(name))
                    return false;
                outType.append(name.produceString());
                return true;
            }
        }
        return false;
    }

    bool parseVal(StringBuilder& outVal)
    {
        String type;
        if (tryParseType(type))
        {
            outVal.append(type);
            return true;
        }

        if (consume('K'))
        {
            if (consume('C'))
            {
                if (consume('O'))
                {
                    String typeArg;
                    if (!tryParseType(typeArg))
                        return false;
                    outVal << "countof(" << typeArg << ")";
                    return true;
                }

                UInt argCount = 0;
                if (!parseUInt(argCount))
                    return false;
                String funcName;
                if (!parseName(funcName))
                    return false;
                List<String> args;
                for (UInt i = 0; i < argCount; ++i)
                {
                    String arg;
                    if (!tryParseVal(arg))
                        return false;
                    args.add(arg);
                }
                outVal << funcName << "(";
                for (Index i = 0; i < args.getCount(); ++i)
                {
                    if (i)
                        outVal.append(", ");
                    outVal.append(args[i]);
                }
                outVal.appendChar(')');
                return true;
            }
            if (consume('L'))
            {
                Index witnessStart = pos;
                String witness;
                tryParseVal(witness);
                String key;
                if (!parseName(key))
                {
                    pos = witnessStart;
                    witness = String();
                    if (!parseName(key))
                        return false;
                }
                if (witness.getLength())
                    outVal << witness << ".";
                outVal << key;
                return true;
            }
            if (consume('S'))
            {
                if (!consume('O'))
                    return false;
                String typeArg;
                if (!tryParseType(typeArg))
                    return false;
                outVal << "sizeof(" << typeArg << ")";
                return true;
            }
            if (consume('A'))
            {
                if (!consume('O'))
                    return false;
                String typeArg;
                if (!tryParseType(typeArg))
                    return false;
                outVal << "alignof(" << typeArg << ")";
                return true;
            }
            if (consume('X'))
            {
                UInt constantTerm = 0;
                if (!parseUInt(constantTerm))
                    return false;
                UInt termCount = 0;
                if (!parseUInt(termCount))
                    return false;

                List<String> terms;
                for (UInt i = 0; i < termCount; ++i)
                {
                    UInt factor = 0;
                    if (!parseUInt(factor))
                        return false;
                    UInt paramFactorCount = 0;
                    if (!parseUInt(paramFactorCount))
                        return false;
                    List<String> factors;
                    for (UInt j = 0; j < paramFactorCount; ++j)
                    {
                        String param;
                        if (!tryParseVal(param))
                            return false;
                        UInt power = 0;
                        if (!parseUInt(power))
                            return false;
                        StringBuilder factorBuilder;
                        factorBuilder << param;
                        if (power != 1)
                        {
                            factorBuilder << "^" << power;
                        }
                        factors.add(factorBuilder.produceString());
                    }
                    StringBuilder termBuilder;
                    termBuilder << factor;
                    for (auto f : factors)
                    {
                        termBuilder.append("*");
                        termBuilder.append(f);
                    }
                    terms.add(termBuilder.produceString());
                }

                outVal << constantTerm;
                for (auto const& term : terms)
                {
                    outVal << " + " << term;
                }
                return true;
            }
            if (consume('K'))
            {
                String typeArg;
                if (!tryParseType(typeArg))
                    return false;
                String base;
                if (!tryParseVal(base))
                    return false;
                outVal << "(" << typeArg << ")" << base;
                return true;
            }

            String name;
            if (!parseName(name))
                return false;
            outVal.append(name);
            return true;
        }

        if (consume('k'))
        {
            UInt value = 0;
            if (!parseUInt(value))
                return false;
            outVal.append(value);
            return true;
        }

        String name;
        if (!parseName(name))
            return false;
        outVal.append(name);
        return true;
    }

    bool parseEntityCore(StringBuilder& outEntity)
    {
        List<String> components;
        if (!parseNamePath(components))
            return false;

        StringBuilder base;
        for (Index i = 0; i < components.getCount(); ++i)
        {
            if (i)
                base.append("::");
            base.append(components[i]);
        }

        parseAccessorSuffix(base);

        if (consume('P'))
        {
            base.append(" [postfix]");
        }
        else if (peek() == 'p')
        {
            bool looksLikeParamBlock =
                (pos + 1 < input.getLength()) && isDigitChar(input[pos + 1]);
            if (!looksLikeParamBlock && consume('p'))
            {
                base.append(" [prefix]");
            }
        }

        StringBuilder genericSuffix;
        StringBuilder whereClause;
        if (consume('G'))
        {
            if (!parseGenericArgs(genericSuffix))
                return false;
        }
        else if (consume('g'))
        {
            if (!parseGenericParams(genericSuffix, whereClause))
                return false;
        }

        StringBuilder signature;
        if (peek() == 'p' && (pos + 1 < input.getLength()) && isDigitChar(input[pos + 1]))
        {
            if (!parseParameterBlock(signature))
                return false;
        }

        base.append(genericSuffix.produceString());
        outEntity.append(base.produceString());
        if (signature.getLength())
            outEntity.append(signature.produceString());
        if (whereClause.getLength())
        {
            outEntity.append(" where ");
            outEntity.append(whereClause.produceString());
        }
        return true;
    }

    bool parseWitness(StringBuilder& outStr)
    {
        String first;
        if (!tryParseVal(first))
        {
            first = String(input.subString(pos, input.getLength() - pos));
            pos = input.getLength();
            outStr << "witness(" << first << ")";
            return true;
        }
        String second;
        if (!tryParseVal(second))
        {
            second = String(input.subString(pos, input.getLength() - pos));
            pos = input.getLength();
            outStr << "witness(" << first << " -> " << second << ")";
            return true;
        }
        outStr << "witness(" << first << " : " << second << ")";
        return true;
    }

    bool parseRoot(String& outDemangled)
    {
        if (!consume('_') || !consume('S'))
            return false;

        if (consume('h'))
        {
            StringBuilder builder;
            builder << "hash[" << input.subString(pos, input.getLength() - pos) << "]";
            pos = input.getLength();
            outDemangled = builder.produceString();
            return true;
        }

        if (consume('T'))
        {
            String typeStr;
            if (!tryParseType(typeStr))
                return false;
            outDemangled = typeStr;
            return true;
        }

        if (consume('W'))
        {
            StringBuilder builder;
            if (!parseWitness(builder))
                return false;
            outDemangled = builder.produceString();
            return true;
        }

        char kind = 0;
        if (!atEnd())
        {
            char candidate = peek();
            if (candidate == 'T' || candidate == 'V' || candidate == 'G')
            {
                kind = candidate;
                ++pos;
            }
        }

        StringBuilder builder;
        if (!parseEntityCore(builder))
            return false;

        String body = builder.produceString();
        StringBuilder finalBuilder;
        if (kind == 'T')
            finalBuilder.append("type ");
        else if (kind == 'V')
            finalBuilder.append("var ");
        else if (kind == 'G')
            finalBuilder.append("generic ");
        finalBuilder.append(body);
        outDemangled = finalBuilder.produceString();
        return true;
    }
};
} // namespace

SLANG_API bool tryDemangleName(const UnownedStringSlice& mangledName, String& outDemangled)
{
    DemangleParser parser(mangledName);
    if (!parser.parseRoot(outDemangled) || !parser.atEnd())
    {
        outDemangled = String(mangledName);
        return false;
    }
    return true;
}

SLANG_API String demangleName(const UnownedStringSlice& mangledName)
{
    String result;
    if (!tryDemangleName(mangledName, result))
        return String(mangledName);
    return result;
}

} // namespace Slang

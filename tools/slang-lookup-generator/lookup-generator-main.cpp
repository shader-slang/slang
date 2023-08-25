// perfect-hash-main.cpp

#include <stdio.h>
#include "../../source/compiler-core/slang-json-parser.h"
#include "../../source/compiler-core/slang-json-value.h"
#include "../../source/compiler-core/slang-lexer.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-secure-crt.h"
#include "../../source/core/slang-string-util.h"

using namespace Slang;

static SlangResult parseJson(const char* inputPath, DiagnosticSink* sink, JSONListener& listener)
{
    auto sourceManager = sink->getSourceManager();

    String contents;
    SLANG_RETURN_ON_FAIL(File::readAllText(inputPath, contents));
    PathInfo    pathInfo = PathInfo::makeFromString(inputPath);
    SourceFile* sourceFile = sourceManager->createSourceFileWithString(pathInfo, contents);
    SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc());
    JSONLexer   lexer;
    lexer.init(sourceView, sink);
    JSONParser parser;
    SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &listener, sink));
    return SLANG_OK;
}

// Extract from a json value, the "opname" member from all the objects in the
// "instructions" array.
// Returns the empty list on failure
static List<String> extractOpNames(UnownedStringSlice& error, const JSONValue& v, JSONContainer& container)
{
    List<String> opnames;

    // Wish we could just write à la jq
    // List<String> result = match(myJSONValue, "instructions", AsArray, "opname", AsString);
    const auto instKey = container.findKey(UnownedStringSlice("instructions"));
    const auto opnameKey = container.findKey(UnownedStringSlice("opname"));
    if (!instKey)
    {
        error = UnownedStringSlice("JSON parsing failed, no \"instructions\" key\n");
        return {};
    }
    if (!opnameKey)
    {
        error = UnownedStringSlice("JSON parsing failed, no \"opname\" key\n");
        return {};
    }

    const auto instructions = container.findObjectValue(v, instKey);
    if (!instructions.isValid() || instructions.type != JSONValue::Type::Array)
    {
        error = UnownedStringSlice("JSON parsing failed, no \"instructions\" member of array type\n");
        return {};
    }
    for (const auto& inst : container.getArray(instructions))
    {
        const auto opname = container.findObjectValue(inst, opnameKey);
        if (!opname.isValid() || opname.getKind() != JSONValue::Kind::String)
        {
            error = UnownedStringSlice("JSON parsing failed, no \"opname\" member of string type for instruction\n");
            return {};
        }
        opnames.add(container.getString(opname));
    }

    return opnames;
}

struct HashParams
{
    List<UInt32> saltTable;
    List<String> destTable;
};

enum HashFindResult {
    Success,
    NonUniqueKeys,
    UnavoidableHashCollision,
};

// Implemented according to "Hash, displace, and compress"
// https://cmph.sourceforge.net/papers/esa09.pdf
static HashFindResult minimalPerfectHash(const List<String>& ss, HashParams& hashParams)
{
    // Check for uniqueness
    for (Index i = 0; i < ss.getCount(); ++i)
    {
        for (Index j = i + 1; j < ss.getCount(); ++j)
        {
            if (ss[i] == ss[j])
            {
                return NonUniqueKeys;
            }
        }
    }

    SLANG_ASSERT(UIndex(ss.getCount()) < std::numeric_limits<UInt32>::max());
    const UInt32       nBuckets = UInt32(ss.getCount());
    List<List<String>> initialBuckets;
    initialBuckets.setCount(nBuckets);

    const auto hash = [&](const String& s, const HashCode64 salt = 0) -> UInt32
    {
        //
        // The current getStableHashCode is susceptible to patterns of
        // collisions causing the search to fail for the SPIR-V opnames; it
        // performs poorly on short strings, taking over 300000 iterations to
        // diverge on "Ceil" and "FMix" (and place them in already unoccupied
        // slots)!
        //
        // Use FNV Hash here which seem perform much better on these short inputs
        // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
        //
        // If you change this, don't forget to also sync the version below in
        // the printing code.
        UInt64 h = salt;
        for (const char c : s) h = ((h * 0x00000100000001B3) ^ c);
        return h % nBuckets;
    };

    // Assign the inputs into their buckets according to the hash without salt.
    // Sort the buckets according to size, so that later we can make these have
    // unique destinations starting with the largest ones first as they are at
    // most risk of collision.
    for (const auto& s : ss)
    {
        initialBuckets[hash(s)].add(s);
    }
    initialBuckets.stableSort([](const List<String>& a, const List<String>& b) { return a.getCount() > b.getCount(); });

    // These are our outputs, the salts are calculated such that for all input
    // word, x, hash(x, salt[hash(x, 0)]) is unique
    //
    // We keep the final table as we need to detect when we've been given a
    // word not in our language.
    hashParams.saltTable.setCount(nBuckets);
    for (auto& s : hashParams.saltTable)
    {
        s = 0;
    }
    hashParams.destTable.setCount(nBuckets);
    for (auto& s : hashParams.destTable)
    {
        s.reduceLength(0);
    }

    // This mask will, in each salt tryout, be used to prevent collisions
    // within a single bucket.
    List<bool> bucketDestinations = List<bool>::makeRepeated(false, nBuckets);

    for (const auto& b : initialBuckets)
    {
        // Break if we've reached the empty buckets
        if (!b.getCount())
        {
            break;
        }

        // Try out all the salts until we get one which has no internal
        // collisions for this bucket and also no collisions with the buckets
        // we've processed so far.
        UInt32 salt = 1;
        while (true)
        {
            bool collision = false;
            for (auto& d : bucketDestinations)
            {
                d = false;
            }

            for (const auto& s : b)
            {
                const auto i = hash(s, salt);
                if (hashParams.destTable[i].getLength() || bucketDestinations[i])
                {
                    collision = true;
                    break;
                }
                bucketDestinations[i] = true;
            }
            if (!collision)
            {
                break;
            }
            salt++;

            // If we fail to find a solution after some massive amount of tries
            // it's almost certainly because of some property of the hash
            // function and language causing an irresolvable collision.
            if (salt > 10000 * nBuckets)
            {
                return UnavoidableHashCollision;
            }
        }
        for (const auto& s : b)
        {
            hashParams.saltTable[hash(s)] = salt;
            hashParams.destTable[hash(s, salt)] = s;
        }
    }
    return Success;
}

void writeHashFile(
    const char* const  outCppPath,
    const char*        valueType,
    const char*        valuePrefix,
    const List<String> includes,
    const HashParams&  hashParams)
{
    StringBuilder sb;
    StringWriter writer(&sb, WriterFlags(0));
    WriterHelper w(&writer);

    w.print("// Hash function for %s\n", valueType);
    w.print("//\n");
    w.print("// This file was thoughtfully generated by a machine,\n");
    w.print("// don't even think about modifying it yourself!\n");
    w.print("//\n");
    w.print("\n");
    for (const auto& i : includes)
    {
        w.print("#include \"%s\"\n", i.getBuffer());
    }
    w.print("\n");
    w.print("\n");
    w.print("namespace Slang\n");
    w.print("{\n");
    w.print("\n");

    w.print("static const unsigned tableSalt[%ld] =", hashParams.saltTable.getCount());
    w.print("{\n   ");
    for (Index i = 0; i < hashParams.saltTable.getCount(); ++i)
    {
        const auto salt = hashParams.saltTable[i];
        if (i != hashParams.saltTable.getCount() - 1)
        {
            w.print(" %d,", salt);
            if (i % 16 == 15)
            {
                w.print("\n   ");
            }
        }
        else
        {
            w.print(" %d", salt);
        }
    }
    w.print("\n};\n");
    w.print("\n");

    w.print("struct KV\n");
    w.print("{\n");
    w.print("    const char* name;\n");
    w.print("    %s value;\n", valueType);
    w.print("};\n");
    w.print("\n");

    w.print("static const KV words[%ld] =\n", hashParams.destTable.getCount());
    w.print("{\n");
    for (const auto& s : hashParams.destTable)
    {
        w.print("    {\"%s\", %s%s},\n", s.getBuffer(), valuePrefix, s.getBuffer());
    }
    w.print("};\n");
    w.print("\n");

    // Make sure to update the hash function in the search function above if
    // you change this.
    w.print("static UInt32 hash(const UnownedStringSlice& str, UInt32 salt)\n");
    w.print("{\n");
    w.print("    UInt64 h = salt;\n");
    w.print("    for(const char c : str)\n");
    w.print("        h = ((h * 0x00000100000001B3) ^ c);\n");
    w.print("    return h %% (sizeof(tableSalt)/sizeof(tableSalt[0]));\n");
    w.print("}\n");
    w.print("\n");

    w.print("bool lookup%s(const UnownedStringSlice& str, %s& value)\n", valueType, valueType);
    w.print("{\n");
    w.print("    const auto i = hash(str, tableSalt[hash(str, 0)]);\n");
    w.print("    if(str == words[i].name)\n");
    w.print("    {\n");
    w.print("        value = words[i].value;\n");
    w.print("        return true;\n");
    w.print("    }\n");
    w.print("    else\n");
    w.print("    {\n");
    w.print("        return false;\n");
    w.print("    }\n");
    w.print("}\n");
    w.print("\n");

    w.print("}\n");

    File::writeAllTextIfChanged(outCppPath, sb.getUnownedSlice());
}

int main(int argc, const char* const* argv)
{
    using namespace Slang;

    if (argc != 6)
    {
        fprintf(
            stderr,
            "Usage: %s input.grammar.json output.cpp enum-name enumerant-prefix enum-header-file\n",
            argc >= 1 ? argv[0] : "slang-lookup-generator");
        return 1;
    }

    const char* const inPath = argv[1];
    const char* const outCppPath = argv[2];
    const char* const enumName = argv[3];
    const char* const enumerantPrefix = argv[4];
    const char* const enumHeader = argv[5];

    RefPtr<FileWriter> writer(new FileWriter(stderr, WriterFlag::AutoFlush));
    SourceManager      sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    DiagnosticSink sink(&sourceManager, Lexer::sourceLocationLexer);
    sink.writer = writer;

    List<String> opnames;

    if (String(inPath).endsWith("json"))
    {
        // If source is a json file parse it.
        JSONContainer container(sink.getSourceManager());
        JSONBuilder   builder(&container);
        if (SLANG_FAILED(parseJson(inPath, &sink, builder)))
        {
            sink.diagnoseRaw(Severity::Error, "Json parsing failed\n");
            return 1;
        }

        UnownedStringSlice error;
        opnames = extractOpNames(error, builder.getRootValue(), container);
        if (error.getLength())
        {
            sink.diagnoseRaw(Severity::Error, error);
            return 1;
        }
    }
    else
    {
        // Otherwise, we assume the input is a text file with one name per line.
        String content;
        File::readAllText(inPath, content);
        List<UnownedStringSlice> words;
        StringUtil::split(content.getUnownedSlice(), '\n', words);
        for (auto w : words)
            opnames.add(w);
    }

    HashParams hashParams;
    auto       r = minimalPerfectHash(opnames, hashParams);
    switch (r)
    {
    case UnavoidableHashCollision:
        {
            sink.diagnoseRaw(
                Severity::Error,
                "Unable to find a non-overlapping hash function.\n"
                "The hash function probably has a unavoidable "
                "collision for some input words\n");
            return 1;
        }
    case NonUniqueKeys:
        {
            sink.diagnoseRaw(Severity::Error, "Input word list has duplicates\n");
            return 1;
        }
    case Success:;
    }

    writeHashFile(
        outCppPath,
        enumName,
        enumerantPrefix,
        { "../core/slang-common.h", "../core/slang-string.h", enumHeader },
        hashParams);

    return 0;
}

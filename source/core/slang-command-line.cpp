// slang-command-line.cpp
#include "slang-command-line.h"

#include "slang-process.h"

#include "slang-string.h"
#include "slang-string-escape-util.h"
#include "slang-string-util.h"

#include "../../slang-com-helper.h"

namespace Slang {

void CommandLine::addPrefixPathArg(const char* prefix, const String& path, const char* pathPostfix)
{
    StringBuilder builder;
    builder << prefix << path;
    if (pathPostfix)
    {
        // Work out the path with the postfix
        builder << pathPostfix;
    }
    addArg(builder.ProduceString());
}

void CommandLine::setExecutable(const String& dir, const String& name)
{
    StringBuilder builder;
    Path::combineIntoBuilder(dir.getUnownedSlice(), name.getUnownedSlice(), builder);
    builder << Process::getExecutableSuffix();
    setExecutablePath(builder.ProduceString());
}

void CommandLine::append(StringBuilder& out) const
{
    auto escapeHandler = Process::getEscapeHandler();

    StringEscapeUtil::appendMaybeQuoted(escapeHandler, m_executable.getUnownedSlice(), out);

    for (const auto& arg : m_args)
    {
        out << " ";
        StringEscapeUtil::appendMaybeQuoted(escapeHandler, arg.getUnownedSlice(), out);
    }
}

String CommandLine::toString() const
{
    StringBuilder buf;
    append(buf);
    return buf.ProduceString();
}

} // namespace Slang

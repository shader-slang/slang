#include "../core/slang-io.h"
#include "../core/slang-string.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

using namespace Slang;

struct DispatchContext
{
    String binDir;
    bool verbose;
    bool dryRun;
};

typedef int (*SubcommandFunc)(
    DispatchContext* ctx,
    const char* name,
    int argc,
    const char* const* argv,
    void* extra);

struct SubcommandEntry
{
    const char* name;
    const char* description;
    SubcommandFunc callback;
    void* extra;
};

static int delegateToExecutable(
    DispatchContext* ctx,
    const char* name,
    int argc,
    const char* const* argv,
    void* extra);

static int handleHelp(
    DispatchContext* ctx,
    const char* name,
    int argc,
    const char* const* argv,
    void* extra);

static int handleVersion(
    DispatchContext* ctx,
    const char* name,
    int argc,
    const char* const* argv,
    void* extra);

static const SubcommandEntry kBuiltinSubcommands[] = {
    {"compile",
     "Compile Slang source (delegates to slangc)",
     delegateToExecutable,
     (void*)"slangc"},
    {"interpret",
     "Interpret Slang source (delegates to slangi)",
     delegateToExecutable,
     (void*)"slangi"},
    {"help", "Show this help message", handleHelp, nullptr},
    {"version", "Show version information", handleVersion, nullptr},
};

static const SubcommandEntry* findSubcommand(const char* name)
{
    for (const auto& entry : kBuiltinSubcommands)
    {
        if (strcmp(entry.name, name) == 0)
            return &entry;
    }
    return nullptr;
}

static bool isFlag(const char* arg)
{
    return arg[0] == '-';
}

static bool looksLikeFilePath(const char* arg)
{
    return strchr(arg, '.') != nullptr;
}

static bool matchFlag(const char* arg, const char* longName)
{
    if (strcmp(arg, longName) == 0)
        return true;
    // Accept single-dash form for compat: -verbose matches --verbose
    if (arg[0] == '-' && arg[1] != '-' && strcmp(arg + 1, longName + 2) == 0)
        return true;
    return false;
}

static String findExecutableInDir(const String& dir, const String& name)
{
#ifdef _WIN32
    String path = Path::combine(dir, name + ".exe");
#else
    String path = Path::combine(dir, name);
#endif
    if (File::exists(path))
        return path;
    return String();
}

// Spawn a child process that inherits our stdin/stdout/stderr,
// wait for it to exit, and return its exit code via outExitCode.
// Returns true if the process was launched successfully (even if it
// exits with a nonzero code), false if it could not be started.
// `execPath` must be a full path to the executable.
static bool spawnInherited(const String& execPath, const List<String>& args, int& outExitCode)
{
#ifdef _WIN32
    // lpApplicationName specifies the executable to run.
    // lpCommandLine provides the argument string; its first token becomes argv[0].
    OSString exeWide = execPath.toWString();

    StringBuilder cmdBuilder;
    for (Index i = 0; i < args.getCount(); ++i)
    {
        if (i > 0)
            cmdBuilder << " ";
        cmdBuilder << "\"" << args[i] << "\"";
    }
    OSString cmdWide = cmdBuilder.toString().toWString();

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(
        exeWide.begin(),
        (LPWSTR)cmdWide.begin(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi);

    if (!ok)
        return false;

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    outExitCode = (int)exitCode;
    return true;
#else
    List<const char*> cargs;
    for (const auto& arg : args)
        cargs.add(arg.getBuffer());
    cargs.add(nullptr);

    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);

    sigset_t sigdefault;
    sigemptyset(&sigdefault);
    sigaddset(&sigdefault, SIGPIPE);
    posix_spawnattr_setsigdefault(&attr, &sigdefault);
    short flags = 0;
    posix_spawnattr_getflags(&attr, &flags);
    flags |= POSIX_SPAWN_SETSIGDEF;
    posix_spawnattr_setflags(&attr, flags);

    pid_t pid;
    int result = posix_spawn(
        &pid,
        execPath.getBuffer(),
        nullptr,
        &attr,
        (char* const*)cargs.getBuffer(),
        environ);
    posix_spawnattr_destroy(&attr);

    if (result != 0)
        return false;

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        outExitCode = WEXITSTATUS(status);
    else
        outExitCode = 1;
    return true;
#endif
}

static void printCommand(const String& executable, const List<String>& args)
{
    fprintf(stderr, "slang: %s", executable.getBuffer());
    for (const auto& arg : args)
        fprintf(stderr, " %s", arg.getBuffer());
    fprintf(stderr, "\n");
}

static int launchTool(
    DispatchContext* ctx,
    const String& toolName,
    const char* argv0,
    int argc,
    const char* const* argv,
    const char* errorContext)
{
    List<String> args;
    args.add(argv0);
    for (int i = 0; i < argc; ++i)
        args.add(argv[i]);

    int exitCode = 1;

    // Search the sibling directory (where the dispatcher itself lives).
    String path = findExecutableInDir(ctx->binDir, toolName);
    if (path.getLength() > 0)
    {
        if (ctx->verbose || ctx->dryRun)
        {
            printCommand(path, args);
            if (ctx->dryRun)
                return 0;
        }
        if (spawnInherited(path, args, exitCode))
            return exitCode;
    }

    fprintf(stderr, "slang: %s\n", errorContext);
    return 1;
}

static int delegateToExecutable(
    DispatchContext* ctx,
    const char* name,
    int argc,
    const char* const* argv,
    void* extra)
{
    const char* executableName = extra ? (const char*)extra : name;
    String toolName(executableName);
    String argv0 = String("slang-") + name;

    StringBuilder errMsg;
    errMsg << "unknown command '" << name << "'. See 'slang help'.";
    String errString = errMsg.toString();

    return launchTool(ctx, toolName, argv0.getBuffer(), argc, argv, errString.getBuffer());
}

static int delegateToInterpreter(DispatchContext* ctx, int argc, const char* const* argv)
{
    return launchTool(ctx, String("slangi"), "slangi", argc, argv, "failed to launch 'slangi'");
}

static int handleHelp(
    DispatchContext* ctx,
    const char* name,
    int argc,
    const char* const* argv,
    void* extra)
{
    (void)ctx;
    (void)name;
    (void)argc;
    (void)argv;
    (void)extra;

    printf("Usage: slang [flags] <command> [args...]\n"
           "       slang [args...] <file.slang> [args...]\n"
           "\n"
           "Commands:\n");
    for (const auto& entry : kBuiltinSubcommands)
        printf("  %-12s%s\n", entry.name, entry.description);
    printf("\n"
           "Flags (before <command> only):\n"
           "  --verbose   Print the command before executing\n"
           "  --dry-run   Print the command without executing\n"
           "\n"
           "Run 'slang <command> --help' for more information on a command.\n"
           "\n"
           "When invoked without a command, or with a file path as the first\n"
           "argument, slang delegates to slangi (the Slang interpreter).\n");
    return 0;
}

static int handleVersion(
    DispatchContext* ctx,
    const char* name,
    int argc,
    const char* const* argv,
    void* extra)
{
    (void)ctx;
    (void)name;
    (void)argc;
    (void)argv;
    (void)extra;

#ifdef SLANG_VERSION_STRING
    printf("slang version %s\n", SLANG_VERSION_STRING);
#else
    printf("slang version unknown\n");
#endif
    return 0;
}

#ifdef _WIN32
#define MAIN slang_dispatcher_main
#else
#define MAIN main
#endif

int MAIN(int argc, const char* const* argv)
{
    DispatchContext ctx;
    ctx.binDir = Path::getParentDirectory(Path::getExecutablePath());
    ctx.verbose = false;
    ctx.dryRun = false;

    int argStart = 1;

    // Find the first positional (non-flag) argument to determine the mode.
    int firstPositional = -1;
    for (int i = argStart; i < argc; ++i)
    {
        if (!isFlag(argv[i]))
        {
            firstPositional = i;
            break;
        }
    }

    bool isSubcommandMode = false;
    if (firstPositional >= 0 && !looksLikeFilePath(argv[firstPositional]))
        isSubcommandMode = true;

    if (!isSubcommandMode)
    {
        // Interpreter mode: forward ALL original arguments to slangi verbatim.
        return delegateToInterpreter(&ctx, argc - argStart, argv + argStart);
    }

    // Subcommand mode: parse dispatcher flags before the subcommand name.
    for (int i = argStart; i < firstPositional; ++i)
    {
        if (matchFlag(argv[i], "--verbose"))
            ctx.verbose = true;
        else if (matchFlag(argv[i], "--dry-run"))
            ctx.dryRun = true;
        else
        {
            fprintf(stderr, "slang: unknown flag '%s'. See 'slang help'.\n", argv[i]);
            return 1;
        }
    }

    const char* subcommand = argv[firstPositional];
    int subArgc = argc - (firstPositional + 1);
    const char* const* subArgv = argv + (firstPositional + 1);

    const SubcommandEntry* entry = findSubcommand(subcommand);
    if (entry)
        return entry->callback(&ctx, subcommand, subArgc, subArgv, entry->extra);

    return delegateToExecutable(&ctx, subcommand, subArgc, subArgv, nullptr);
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    List<String> args;
    for (int i = 0; i < argc; ++i)
        args.add(String::fromWString(argv[i]));

    List<const char*> argBuffers;
    for (const auto& arg : args)
        argBuffers.add(arg.getBuffer());

    return MAIN((int)argBuffers.getCount(), argBuffers.getBuffer());
}
#endif

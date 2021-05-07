// slang-unix-process-util.cpp
#include "../slang-process-util.h"

#include "../slang-common.h"
#include "../slang-string-util.h"
#include "../slang-string-escape-util.h"
#include "../slang-memory-arena.h"

#include <stdio.h>
#include <stdlib.h>

//#include <dirent.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <time.h>

namespace Slang {


/* static */UnownedStringSlice ProcessUtil::getExecutableSuffix()
{
#if __CYGWIN__
    return UnownedStringSlice::fromLiteral(".exe");
#else
    return UnownedStringSlice::fromLiteral("");
#endif
}

/* static */StringEscapeHandler* ProcessUtil::getEscapeHandler()
{
    return StringEscapeUtil::getHandler(StringEscapeUtil::Style::Space);
}

/* static */String ProcessUtil::getCommandLineString(const CommandLine& commandLine)
{
    auto escapeHandler = getEscapeHandler();

    // When outputting the command line we potentially need to escape the path to the
    // command and args - that aren't already explicitly marked as escaped.

    StringBuilder cmd;
    StringEscapeUtil::appendMaybeQuoted(escapeHandler, commandLine.m_executable.getUnownedSlice(), cmd);
    for (const auto& arg : commandLine.m_args)
    {
        cmd << " ";

        StringEscapeUtil::appendMaybeQuoted(escapeHandler, arg.getUnownedSlice(), cmd);
    }
    return cmd.ToString();
}

/* static */SlangResult ProcessUtil::execute(const CommandLine& commandLine, ExecuteResult& outExecuteResult)
{
    outExecuteResult.init();
    
    List<char const*> argPtrs;

    // Add the command
    argPtrs.add(commandLine.m_executable.getBuffer());

    // Add all the args - they don't need any explicit escaping 
    for (auto arg : commandLine.m_args)
    {
        // All args for this target must be unescaped (as they are in CommandLine)
        argPtrs.add(arg.getBuffer());
    }

    // Terminate with a null
    argPtrs.add(nullptr);

    int stdoutPipe[2];
    int stderrPipe[2];

    if (pipe(stdoutPipe) == -1)
    {
        fprintf(stderr, "error: `pipe` failed\n");
        return SLANG_FAIL;
    }

    if (pipe(stderrPipe) == -1)
    {
        fprintf(stderr, "error: `pipe` failed\n");
        return SLANG_FAIL;
    }

    pid_t childProcessID = fork();
    if (childProcessID == -1)
    {
        fprintf(stderr, "error: `fork` failed\n");
        return SLANG_FAIL;
    }

    if (childProcessID == 0)
    {
        // We are the child process.

        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        close(stdoutPipe[0]);
        close(stdoutPipe[1]);

        close(stderrPipe[0]);
        close(stderrPipe[1]);

        execvp(argPtrs[0], (char* const*)&argPtrs[0]);

        // If we get here, then `exec` failed
        fprintf(stderr, "error: `exec` failed\n");
        return SLANG_FAIL;
    }
    else
    {
        // We are the parent process

        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        int stdoutFD = stdoutPipe[0];
        int stderrFD = stderrPipe[0];

        pollfd pollInfos[2];
        nfds_t pollInfoCount = 2;

        pollInfos[0].fd = stdoutFD;
        pollInfos[0].events = POLLIN;
        pollInfos[0].revents = 0;
        pollInfos[1].fd = stderrFD;
        pollInfos[1].events = POLLIN;
        pollInfos[1].revents = 0;

        int remainingCount = 2;
        int iterations = 0;
        while (remainingCount)
        {
            // Safeguard against infinite loop:
            iterations++;
            if (iterations > 10000)
            {
                fprintf(stderr, "poll(): %d iterations\n", iterations);
                return SLANG_FAIL;
            }

            // Set a timeout of twenty seconds;
            // we really shouldn't wait too long...
            int pollTimeout = 20000;
            int pollResult = poll(pollInfos, pollInfoCount, pollTimeout);
            if (pollResult <= 0)
            {
                // If there was a signal that got in
                // the way, then retry...
                if (pollResult == -1 && errno == EINTR)
                    continue;

                // timeout or error...
                fprintf(stderr, "error: `poll` failed or timed out\n");
                return SLANG_FAIL;
            }

            enum { kBufferSize = 1024 };
            char buffer[kBufferSize];

            if (pollInfos[0].revents)
            {
                auto count = read(stdoutFD, buffer, kBufferSize);
                if (count <= 0)
                {
                    // end-of-file
                    close(stdoutFD);
                    pollInfos[0].fd = -1;
                    remainingCount--;
                }

                outExecuteResult.standardOutput.append(buffer, buffer + count);
            }

            if (pollInfos[1].revents)
            {
                auto count = read(stderrFD, buffer, kBufferSize);
                if (count <= 0)
                {
                    // end-of-file
                    close(stderrFD);
                    pollInfos[1].fd = -1;
                    remainingCount--;
                }

                outExecuteResult.standardError.append(buffer, buffer + count);
            }
        }

        int childStatus = 0;
        iterations = 0;
        for (;;)
        {
            // Safeguard against infinite loop:
            iterations++;
            if (iterations > 10000)
            {
                fprintf(stderr, "waitpid(): %d iterations\n", iterations);
                return SLANG_FAIL;
            }

            pid_t terminatedProcessID = waitpid(childProcessID, &childStatus, 0);
            if (terminatedProcessID == -1)
            {
                fprintf(stderr, "error: `waitpid` failed\n");
                return SLANG_FAIL;
            }

            if (terminatedProcessID == childProcessID)
            {
                if (WIFEXITED(childStatus))
                {
                    outExecuteResult.resultCode = (int)(int8_t)WEXITSTATUS(childStatus);
                }
                else
                {
                    outExecuteResult.resultCode = 1;
                }
                return SLANG_OK;
            }
        }
    }

    return SLANG_FAIL;
}

/* static */uint64_t ProcessUtil::getClockFrequency()
{
    return 1000000000;
}

/* static */uint64_t ProcessUtil::getClockTick()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return uint64_t(now.tv_sec) * 1000000000 + now.tv_nsec;
}

} // namespace Slang

// unit-test-replay-misuse.cpp
//
// Defensive tests for replay-context state-machine misuse (this branch
// carries the loadLatestReplay() failure-path cases that map to the
// #10479 "error path handling" bullet).

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

#include <cstring>

// loadLatestReplay() against an empty replay directory must fail cleanly.
SLANG_UNIT_TEST(replayContextLoadLatestReplayNoFolders)
{
    // Issue #10479 error-path bullet: loadReplay / loadLatestReplay against
    // degenerate inputs. The directory exists but contains no recording
    // subfolders, so the function must return a SlangResult failure code
    // rather than throwing or asserting.
    REPLAY_TEST;

    // Create an empty directory and point the replay context at it.
    const char* dir = ".slang-replays-misuse-empty-latest";
    Slang::Path::createDirectoryRecursive(dir);
    ctx().reset();
    ctx().setReplayDirectory(dir);

    // loadLatestReplay() iterates the directory looking for recording
    // subfolders. With none present, it must report failure cleanly.
    SlangResult lr = ctx().loadLatestReplay();
    SLANG_CHECK(SLANG_FAILED(lr));

    // Tear down the test directory and restore the default so subsequent
    // tests in the same process aren't affected.
    Slang::Path::removeNonEmpty(dir);
    ctx().setReplayDirectory(".slang-replays");
}

// loadLatestReplay() against a non-existent directory must fail cleanly.
SLANG_UNIT_TEST(replayContextLoadLatestReplayMissingDirectory)
{
    // Companion to LoadLatestReplayNoFolders: the configured directory does
    // not exist at all. Same SlangResult-failure contract, but guards the
    // path where stat() of the parent directory itself fails.
    REPLAY_TEST;

    // Point at a path we know doesn't exist (no creation step).
    ctx().reset();
    ctx().setReplayDirectory(".slang-replays-misuse-does-not-exist-xyz");

    // loadLatestReplay() must surface that as a clean SlangResult failure
    // rather than throwing on the missing-directory stat error.
    SlangResult lr = ctx().loadLatestReplay();
    SLANG_CHECK(SLANG_FAILED(lr));

    // Restore the default directory.
    ctx().setReplayDirectory(".slang-replays");
}

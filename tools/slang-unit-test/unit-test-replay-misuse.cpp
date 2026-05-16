// unit-test-replay-misuse.cpp
//
// Defensive tests for replay-context state-machine misuse: loadLatestReplay()
// against an empty or missing replay directory must fail cleanly with a
// SlangResult code rather than throwing or asserting.

#include "unit-test-replay-common.h"

// loadLatestReplay() against an empty replay directory must fail cleanly.
SLANG_UNIT_TEST(replayContextLoadLatestReplayNoFolders)
{
    // The directory exists but contains no recording subfolders, so the
    // function must return a SlangResult failure code rather than throwing
    // or asserting.
    REPLAY_TEST;

    // RAII: the scratch directory and the replay-directory setting are
    // both restored on scope exit regardless of how the test ends, so a
    // later throw from inside loadLatestReplay() can't leak the directory
    // or the non-default setting into subsequent tests.
    ScopedReplayDir scratch(".slang-replays-misuse-empty-latest");
    ScopedReplayDirectorySetting restoreReplayDir;

    ctx().reset();
    ctx().setReplayDirectory(scratch.path());

    // loadLatestReplay() iterates the directory looking for recording
    // subfolders. With none present, it must report failure cleanly.
    SlangResult lr = ctx().loadLatestReplay();
    SLANG_CHECK(SLANG_FAILED(lr));
}

// loadLatestReplay() against a non-existent directory must fail cleanly.
SLANG_UNIT_TEST(replayContextLoadLatestReplayMissingDirectory)
{
    // Companion to LoadLatestReplayNoFolders: the configured directory does
    // not exist at all. Same SlangResult-failure contract, but guards the
    // path where stat() of the parent directory itself fails.
    REPLAY_TEST;

    // No directory is created on disk here; the guard only restores the
    // replay-directory setting on scope exit.
    ScopedReplayDirectorySetting restoreReplayDir;

    // Point at a path we know doesn't exist (no creation step).
    ctx().reset();
    ctx().setReplayDirectory(".slang-replays-misuse-does-not-exist-xyz");

    // loadLatestReplay() must surface that as a clean SlangResult failure
    // rather than throwing on the missing-directory stat error.
    SlangResult lr = ctx().loadLatestReplay();
    SLANG_CHECK(SLANG_FAILED(lr));
}

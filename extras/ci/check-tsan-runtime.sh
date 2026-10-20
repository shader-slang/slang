#!/usr/bin/env bash

# Run before the Slang build: its instrumented generators need a working runtime too.
set -euo pipefail

compiler=${CC:-gcc}
canary_dir=$(mktemp -d)
trap 'rm -rf "$canary_dir"' EXIT

cat >"$canary_dir/canary.c" <<'CANARY'
#include <pthread.h>
#include <stdatomic.h>

static volatile int value;
// The handshake precedes both writes; it does not order them with each other.
static atomic_int ready;
#ifdef CLEAN
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static void *write_value(void *unused)
{
    (void)unused;
    atomic_store(&ready, 1);
    while (atomic_load(&ready) != 2) {}
#ifdef CLEAN
    pthread_mutex_lock(&mutex);
#endif
    value = 1;
#ifdef CLEAN
    pthread_mutex_unlock(&mutex);
#endif
    return 0;
}

int main(void)
{
    pthread_t thread;
    if (pthread_create(&thread, 0, write_value, 0))
        return 1;
    while (atomic_load(&ready) != 1) {}
    atomic_store(&ready, 2);
#ifdef CLEAN
    pthread_mutex_lock(&mutex);
#endif
    value = 2;
#ifdef CLEAN
    pthread_mutex_unlock(&mutex);
#endif
    return pthread_join(thread, 0);
}
CANARY

"$compiler" -fsanitize=thread -g -O1 -pthread \
  -DCLEAN "$canary_dir/canary.c" -o "$canary_dir/clean"
"$compiler" -fsanitize=thread -g -O1 -pthread \
  "$canary_dir/canary.c" -o "$canary_dir/racy"

# A clean program must start and exit normally. A startup abort is not evidence
# that race detection works, even if it happens to use the expected exit code.
TSAN_OPTIONS="halt_on_error=1:abort_on_error=0:exitcode=66" "$canary_dir/clean"
race_exit=0
TSAN_OPTIONS="halt_on_error=1:abort_on_error=0:exitcode=66" "$canary_dir/racy" \
  >"$canary_dir/report.txt" 2>&1 || race_exit=$?
cat "$canary_dir/report.txt"
echo "Racy canary exit status: $race_exit"
if [ "$race_exit" -ne 66 ] ||
  ! grep -q 'WARNING: ThreadSanitizer: data race' "$canary_dir/report.txt" ||
  ! grep -q 'canary.c:' "$canary_dir/report.txt"; then
  "$compiler" --version
  ldd "$canary_dir/racy"
  echo "::error::TSan canary did not produce a symbolized race report with exit code 66"
  exit 1
fi
echo "TSan startup, symbolization, and race detection verified"

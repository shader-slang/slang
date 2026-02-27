package gcp

import (
	"context"
	"testing"
	"time"
)

func TestCleanupFilter(t *testing.T) {
	got := cleanupFilter("win-runner")
	want := "name=win-runner-* AND status=TERMINATED"
	if got != want {
		t.Fatalf("cleanupFilter() = %q, want %q", got, want)
	}
}

func TestRemoveTrackedVMByVMName(t *testing.T) {
	m := &Manager{
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "win-runner-1234"},
			"runner-b": {vmName: "win-runner-5678"},
		},
	}

	m.removeTrackedVMByVMName("win-runner-1234")

	if _, ok := m.vms["runner-a"]; ok {
		t.Fatalf("runner-a should be removed when vmName matches")
	}
	if _, ok := m.vms["runner-b"]; !ok {
		t.Fatalf("runner-b should remain")
	}
}

func TestRunCleanupLoopRunsImmediatePass(t *testing.T) {
	m := &Manager{}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	calls := 0
	m.cleanupPass = func(_ context.Context) {
		calls++
		cancel()
	}

	done := make(chan struct{})
	go func() {
		m.runCleanupLoop(ctx, make(chan time.Time))
		close(done)
	}()

	select {
	case <-done:
	case <-time.After(2 * time.Second):
		t.Fatal("cleanup loop did not exit")
	}

	if calls != 1 {
		t.Fatalf("cleanup pass calls = %d, want 1", calls)
	}
}

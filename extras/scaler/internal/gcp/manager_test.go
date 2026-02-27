package gcp

import (
	"context"
	"errors"
	"slices"
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

func TestRunCleanupLoopRunsOnTick(t *testing.T) {
	m := &Manager{}
	ticks := make(chan time.Time, 1)

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	calls := 0
	m.cleanupPass = func(_ context.Context) {
		calls++
		if calls == 2 {
			cancel()
		}
	}

	done := make(chan struct{})
	go func() {
		m.runCleanupLoop(ctx, ticks)
		close(done)
	}()

	ticks <- time.Now()

	select {
	case <-done:
	case <-time.After(2 * time.Second):
		t.Fatal("cleanup loop did not exit")
	}

	if calls != 2 {
		t.Fatalf("cleanup pass calls = %d, want 2", calls)
	}
}

func TestDoCleanupTerminatedVMsContinuesAfterListError(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Zones: "us-east1-c,us-west1-a",
		},
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "win-runner-a"},
			"runner-b": {vmName: "win-runner-b"},
		},
	}

	listCalls := 0
	m.listTerminated = func(_ context.Context, zone string) ([]string, error) {
		listCalls++
		if zone == "us-east1-c" {
			return nil, errors.New("list failed")
		}
		return []string{"win-runner-b"}, nil
	}

	deleted := make([]string, 0, 1)
	m.deleteVMFunc = func(_ context.Context, vmName, _ string) error {
		deleted = append(deleted, vmName)
		return nil
	}

	m.doCleanupTerminatedVMs(context.Background())

	if listCalls != 2 {
		t.Fatalf("list calls = %d, want 2", listCalls)
	}
	if !slices.Equal(deleted, []string{"win-runner-b"}) {
		t.Fatalf("deleted VMs = %v, want [win-runner-b]", deleted)
	}
	if _, ok := m.vms["runner-b"]; ok {
		t.Fatalf("runner-b should be removed after successful cleanup delete")
	}
	if _, ok := m.vms["runner-a"]; !ok {
		t.Fatalf("runner-a should remain when its zone list failed")
	}
}

func TestDoCleanupTerminatedVMsDeleteErrorStillRemovesTrackedEntry(t *testing.T) {
	m := &Manager{
		config: ManagerConfig{
			Zones: "us-east1-c",
		},
		vms: map[string]*vmInfo{
			"runner-a": {vmName: "win-runner-a"},
		},
	}

	m.listTerminated = func(_ context.Context, _ string) ([]string, error) {
		return []string{"win-runner-a"}, nil
	}
	m.deleteVMFunc = func(_ context.Context, _, _ string) error {
		return errors.New("delete failed")
	}

	m.doCleanupTerminatedVMs(context.Background())

	if _, ok := m.vms["runner-a"]; ok {
		t.Fatalf("runner-a should be removed from tracked map even when delete fails")
	}
}

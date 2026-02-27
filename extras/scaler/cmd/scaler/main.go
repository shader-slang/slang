// Windows GPU Runner Scaler
//
// A standalone service that auto-scales Windows GPU VMs on GCP based on
// GitHub Actions job queue depth. Uses the GitHub Actions Scale Set Client
// (github.com/actions/scaleset) to poll for queued jobs and GCP Compute API
// to create/delete VMs.
//
// Based on the dockerscaleset example from github.com/actions/scaleset,
// replacing Docker containers with GCP Compute Engine VMs.
//
// Usage:
//
//	windows-scaler \
//	  --url=https://github.com/shader-slang/slang \
//	  --name=windows-gpu-runners \
//	  --token=ghp_... \
//	  --labels=Windows,self-hosted,GCP-T4 \
//	  --gcp-project=slang-runners \
//	  --gcp-zone=us-west1-a \
//	  --gcp-instance-template=windows-gpu-runner
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"log/slog"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/actions/scaleset"
	"github.com/actions/scaleset/listener"
	"github.com/google/uuid"

	gcpvm "extras/scaler/internal/gcp"
)

// errDrainComplete is returned when drain mode finishes and all VMs have
// completed their jobs. It causes a clean exit without error.
var errDrainComplete = errors.New("drain complete")

type config struct {
	// GitHub configuration
	registrationURL string // e.g. https://github.com/shader-slang/slang
	scaleSetName    string
	labels          string
	runnerGroup     string
	maxRunners      int
	minRunners      int

	// Authentication (GitHub App or PAT)
	appClientID       string
	appInstallationID int64
	appPrivateKey     string
	token             string

	// GCP configuration
	gcpProject          string
	gcpZones            string
	gcpInstanceTemplate string
	gcpGPUType          string
	gcpPlatform         string
	gcpCleanupInterval  time.Duration
}

func (c *config) buildLabels() []scaleset.Label {
	parts := strings.Split(c.labels, ",")
	labels := make([]scaleset.Label, 0, len(parts))
	for _, l := range parts {
		l = strings.TrimSpace(l)
		if l != "" {
			labels = append(labels, scaleset.Label{Name: l, Type: "System"})
		}
	}
	return labels
}

func (c *config) scalesetClient() (*scaleset.Client, error) {
	if c.appClientID != "" {
		return scaleset.NewClientWithGitHubApp(scaleset.ClientWithGitHubAppConfig{
			GitHubConfigURL: c.registrationURL,
			GitHubAppAuth: scaleset.GitHubAppAuth{
				ClientID:       c.appClientID,
				InstallationID: c.appInstallationID,
				PrivateKey:     c.appPrivateKey,
			},
			SystemInfo: scaleset.SystemInfo{
				System:    "windows-gpu-scaler",
				Subsystem: "scaler",
			},
		})
	}
	if c.token != "" {
		return scaleset.NewClientWithPersonalAccessToken(scaleset.NewClientWithPersonalAccessTokenConfig{
			GitHubConfigURL:     c.registrationURL,
			PersonalAccessToken: c.token,
			SystemInfo: scaleset.SystemInfo{
				System:    "windows-gpu-scaler",
				Subsystem: "scaler",
			},
		})
	}
	return nil, fmt.Errorf("either --app-client-id or --token is required")
}

func main() {
	cfg := parseFlags()

	logger := slog.New(slog.NewTextHandler(os.Stdout, &slog.HandlerOptions{
		Level: slog.LevelInfo,
	}))
	slog.SetDefault(logger)

	ctx, cancel := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer cancel()

	if err := run(ctx, cfg, logger); err != nil && !errors.Is(err, context.Canceled) && !errors.Is(err, errDrainComplete) {
		slog.Error("scaler exited with error", "error", err)
		os.Exit(1)
	}
}

func parseFlags() config {
	var cfg config

	flag.StringVar(&cfg.registrationURL, "url", "", "REQUIRED: GitHub URL (e.g. https://github.com/shader-slang/slang)")
	flag.StringVar(&cfg.scaleSetName, "name", "windows-gpu-runners", "Scale set name (must be unique)")
	flag.StringVar(&cfg.labels, "labels", "Windows,self-hosted,GCP-T4", "Comma-separated runner labels")
	flag.StringVar(&cfg.runnerGroup, "runner-group", scaleset.DefaultRunnerGroup, "Runner group name")
	flag.IntVar(&cfg.maxRunners, "max-runners", 5, "Maximum concurrent runners")
	flag.IntVar(&cfg.minRunners, "min-runners", 0, "Minimum runners to keep warm")

	flag.StringVar(&cfg.appClientID, "app-client-id", "", "GitHub App client ID")
	flag.Int64Var(&cfg.appInstallationID, "app-installation-id", 0, "GitHub App installation ID")
	flag.StringVar(&cfg.appPrivateKey, "app-private-key", "", "GitHub App private key (PEM contents)")
	flag.StringVar(&cfg.token, "token", "", "GitHub PAT (alternative to App auth)")

	flag.StringVar(&cfg.gcpProject, "gcp-project", "slang-runners", "GCP project ID")
	flag.StringVar(&cfg.gcpZones, "gcp-zones", "us-east1-c,us-east1-d,us-central1-a,us-west1-a", "Comma-separated zones in preference order (selects by GPU quota availability)")
	flag.StringVar(&cfg.gcpInstanceTemplate, "gcp-instance-template", "windows-gpu-runner", "GCP instance template name")
	flag.StringVar(&cfg.gcpGPUType, "gcp-gpu-type", "nvidia-tesla-t4", "GPU accelerator type")
	flag.StringVar(&cfg.gcpPlatform, "platform", "windows", "Runner platform: windows or linux")
	flag.DurationVar(&cfg.gcpCleanupInterval, "gcp-cleanup-interval", 2*time.Minute, "Interval for scanning and deleting terminated VMs")

	flag.Parse()

	if cfg.registrationURL == "" {
		fmt.Fprintln(os.Stderr, "error: --url is required")
		flag.Usage()
		os.Exit(1)
	}

	if cfg.gcpPlatform != "windows" && cfg.gcpPlatform != "linux" {
		fmt.Fprintf(os.Stderr, "error: --platform must be 'windows' or 'linux', got %q\n", cfg.gcpPlatform)
		flag.Usage()
		os.Exit(1)
	}

	// Allow environment variables to override auth flags.
	// This lets systemd's EnvironmentFile provide credentials.
	if v := os.Getenv("SCALER_TOKEN"); v != "" && cfg.token == "" {
		cfg.token = v
	}
	if v := os.Getenv("SCALER_APP_CLIENT_ID"); v != "" && cfg.appClientID == "" {
		cfg.appClientID = v
	}
	if v := os.Getenv("SCALER_APP_INSTALLATION_ID"); v != "" && cfg.appInstallationID == 0 {
		id, err := strconv.ParseInt(v, 10, 64)
		if err == nil {
			cfg.appInstallationID = id
		}
	}
	if v := os.Getenv("SCALER_APP_PRIVATE_KEY"); v != "" && cfg.appPrivateKey == "" {
		cfg.appPrivateKey = v
	}
	if v := os.Getenv("SCALER_GCP_CLEANUP_INTERVAL"); v != "" {
		d, err := parseCleanupInterval(v)
		if err != nil {
			fmt.Fprintf(os.Stderr, "error: invalid SCALER_GCP_CLEANUP_INTERVAL: %v\n", err)
			os.Exit(1)
		}
		cfg.gcpCleanupInterval = d
	}

	return cfg
}

func parseCleanupInterval(v string) (time.Duration, error) {
	d, err := time.ParseDuration(v)
	if err != nil {
		return 0, fmt.Errorf("%q: %w", v, err)
	}
	return d, nil
}

func run(ctx context.Context, cfg config, logger *slog.Logger) error {
	// Create scaleset client
	ssClient, err := cfg.scalesetClient()
	if err != nil {
		return fmt.Errorf("creating scaleset client: %w", err)
	}

	// Resolve runner group ID
	var runnerGroupID int
	switch cfg.runnerGroup {
	case scaleset.DefaultRunnerGroup:
		runnerGroupID = 1
	default:
		rg, err := ssClient.GetRunnerGroupByName(ctx, cfg.runnerGroup)
		if err != nil {
			return fmt.Errorf("getting runner group: %w", err)
		}
		runnerGroupID = rg.ID
	}

	// Create or reuse runner scale set.
	// If the scale set already exists (e.g. from a previous crash), reuse it
	// instead of failing.
	ss, err := ssClient.GetRunnerScaleSet(ctx, runnerGroupID, cfg.scaleSetName)
	if err != nil {
		return fmt.Errorf("checking for existing scale set: %w", err)
	}
	if ss != nil {
		logger.Info("reusing existing scale set", "name", ss.Name, "id", ss.ID)
		// Update labels in case they changed
		ss, err = ssClient.UpdateRunnerScaleSet(ctx, ss.ID, &scaleset.RunnerScaleSet{
			Name:          cfg.scaleSetName,
			RunnerGroupID: runnerGroupID,
			Labels:        cfg.buildLabels(),
			RunnerSetting: scaleset.RunnerSetting{
				DisableUpdate: true,
			},
		})
		if err != nil {
			return fmt.Errorf("updating existing scale set: %w", err)
		}
	} else {
		ss, err = ssClient.CreateRunnerScaleSet(ctx, &scaleset.RunnerScaleSet{
			Name:          cfg.scaleSetName,
			RunnerGroupID: runnerGroupID,
			Labels:        cfg.buildLabels(),
			RunnerSetting: scaleset.RunnerSetting{
				DisableUpdate: true,
			},
		})
		if err != nil {
			return fmt.Errorf("creating runner scale set: %w", err)
		}
	}

	logger.Info("scale set created",
		"name", ss.Name,
		"id", ss.ID,
		"labels", cfg.labels,
	)

	ssClient.SetSystemInfo(scaleset.SystemInfo{
		System:     "windows-gpu-scaler",
		Subsystem:  "scaler",
		ScaleSetID: ss.ID,
	})

	// Clean up scale set on exit
	defer func() {
		logger.Info("deleting scale set", "id", ss.ID)
		if err := ssClient.DeleteRunnerScaleSet(context.WithoutCancel(ctx), ss.ID); err != nil {
			logger.Error("failed to delete scale set", "error", err)
		}
	}()

	// Runner name prefix based on platform
	vmPrefix := "win-runner"
	if cfg.gcpPlatform == "linux" {
		vmPrefix = "linux-runner"
	}

	// Initialize GCP VM manager
	vmManager, err := gcpvm.NewManager(ctx, gcpvm.ManagerConfig{
		Project:          cfg.gcpProject,
		Zones:            cfg.gcpZones,
		InstanceTemplate: cfg.gcpInstanceTemplate,
		GPUType:          cfg.gcpGPUType,
		Platform:         cfg.gcpPlatform,
		VMPrefix:         vmPrefix,
		CleanupInterval:  cfg.gcpCleanupInterval,
	})
	if err != nil {
		return fmt.Errorf("creating GCP VM manager: %w", err)
	}
	defer vmManager.Close()

	// Create message session
	hostname, err := os.Hostname()
	if err != nil {
		hostname = uuid.NewString()
	}

	sessionClient, err := ssClient.MessageSessionClient(ctx, ss.ID, hostname)
	if err != nil {
		return fmt.Errorf("creating message session: %w", err)
	}
	defer sessionClient.Close(context.Background())

	// Create listener
	lst, err := listener.New(sessionClient, listener.Config{
		ScaleSetID: ss.ID,
		MaxRunners: cfg.maxRunners,
		Logger:     logger.WithGroup("listener"),
	})
	if err != nil {
		return fmt.Errorf("creating listener: %w", err)
	}

	// Create the scaler (implements listener.Scaler interface)
	gcpScaler := &gcpRunnerScaler{
		logger:         logger.WithGroup("scaler"),
		vmManager:      vmManager,
		scalesetClient: ssClient,
		scaleSetID:     ss.ID,
		maxRunners:     cfg.maxRunners,
		minRunners:     cfg.minRunners,
		vmPrefix:       vmPrefix,
	}

	// SIGUSR1 enters drain mode: stop accepting new jobs, wait for running
	// jobs to finish. This enables seamless binary updates:
	//   1. Send SIGUSR1 (or: systemctl reload scaler-windows)
	//   2. Wait for "all VMs finished, exiting drain mode" in logs
	//   3. Send SIGTERM (or: systemctl stop scaler-windows)
	//   4. Replace binary, restart service
	drainCh := make(chan os.Signal, 1)
	signal.Notify(drainCh, syscall.SIGUSR1)
	go func() {
		<-drainCh
		logger.Info("entering drain mode: no new jobs will be accepted, waiting for running VMs to finish")
		lst.SetMaxRunners(0)
		gcpScaler.setDraining(true)
	}()

	defer gcpScaler.shutdown(context.WithoutCancel(ctx))

	logger.Info("starting listener", "max_runners", cfg.maxRunners)
	return lst.Run(ctx, gcpScaler)
}

// gcpRunnerScaler implements the listener.Scaler interface, creating and
// deleting GCP VMs instead of Docker containers.
type gcpRunnerScaler struct {
	logger         *slog.Logger
	vmManager      *gcpvm.Manager
	scalesetClient *scaleset.Client
	scaleSetID     int
	maxRunners     int
	minRunners     int
	vmPrefix       string

	mu       sync.Mutex
	draining bool
}

func (s *gcpRunnerScaler) setDraining(v bool) {
	s.mu.Lock()
	s.draining = v
	s.mu.Unlock()
}

func (s *gcpRunnerScaler) isDraining() bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.draining
}

// HandleDesiredRunnerCount is called when the listener receives a new
// desired runner count from the scale set API.
func (s *gcpRunnerScaler) HandleDesiredRunnerCount(ctx context.Context, count int) (int, error) {
	currentCount := s.vmManager.ActiveCount()

	if s.isDraining() {
		if currentCount == 0 {
			s.logger.Info("all VMs finished, exiting drain mode")
			return 0, errDrainComplete
		}
		s.logger.Info("draining", "active_vms", currentCount, "pending_jobs", count)
		return currentCount, nil
	}

	targetCount := min(s.maxRunners, s.minRunners+count)

	switch {
	case targetCount > currentCount:
		scaleUp := targetCount - currentCount
		s.logger.Info("scaling up", "current", currentCount, "target", targetCount, "creating", scaleUp)

		for range scaleUp {
			name := fmt.Sprintf("%s-%s", s.vmPrefix, uuid.NewString()[:8])

			jit, err := s.scalesetClient.GenerateJitRunnerConfig(
				ctx,
				&scaleset.RunnerScaleSetJitRunnerSetting{Name: name},
				s.scaleSetID,
			)
			if err != nil {
				s.logger.Error("failed to generate JIT config", "error", err)
				continue
			}

			vmName, err := s.vmManager.CreateVM(ctx, name, jit.EncodedJITConfig)
			if err != nil {
				s.logger.Error("failed to create VM", "error", err)
				// JIT config was generated (runner registered) but VM
				// creation failed. Clean up the stale runner entry.
				s.removeRunnerFromGitHub(ctx, name)
				continue
			}

			s.logger.Info("created runner VM", "vm", vmName, "runner", name)
		}
	case targetCount == currentCount:
		// No scaling needed
	default:
		// Scale-down is handled by HandleJobCompleted
	}

	return s.vmManager.ActiveCount(), nil
}

// HandleJobStarted is called when a job starts on one of our runners.
func (s *gcpRunnerScaler) HandleJobStarted(_ context.Context, jobInfo *scaleset.JobStarted) error {
	s.logger.Info("job started",
		"runner", jobInfo.RunnerName,
		"job", jobInfo.JobDisplayName,
		"workflow_run", jobInfo.WorkflowRunID,
	)
	s.vmManager.MarkBusy(jobInfo.RunnerName)
	return nil
}

// HandleJobCompleted is called when a job finishes. We delete the VM and
// remove the runner from GitHub to prevent stale "offline" entries.
func (s *gcpRunnerScaler) HandleJobCompleted(ctx context.Context, jobInfo *scaleset.JobCompleted) error {
	s.logger.Info("job completed",
		"runner", jobInfo.RunnerName,
		"result", jobInfo.Result,
		"job", jobInfo.JobDisplayName,
	)

	if err := s.vmManager.DeleteByRunnerName(ctx, jobInfo.RunnerName); err != nil {
		s.logger.Error("failed to delete VM after job completed", "runner", jobInfo.RunnerName, "error", err)
	}

	// Remove the runner from GitHub to prevent stale "offline" entries.
	// The runner may already be gone if it deregistered cleanly, so
	// errors here are expected and non-fatal.
	if jobInfo.RunnerName != "" {
		s.removeRunnerFromGitHub(ctx, jobInfo.RunnerName)
	}

	return nil
}

// removeRunnerFromGitHub looks up a runner by name and removes it from
// the GitHub Actions runner list.
func (s *gcpRunnerScaler) removeRunnerFromGitHub(ctx context.Context, runnerName string) {
	runner, err := s.scalesetClient.GetRunnerByName(ctx, runnerName)
	if err != nil {
		s.logger.Warn("failed to look up runner for cleanup", "runner", runnerName, "error", err)
		return
	}
	if runner == nil {
		s.logger.Info("runner already removed from GitHub", "runner", runnerName)
		return
	}

	if err := s.scalesetClient.RemoveRunner(ctx, int64(runner.ID)); err != nil {
		s.logger.Warn("failed to remove runner from GitHub", "runner", runnerName, "id", runner.ID, "error", err)
		return
	}

	s.logger.Info("removed runner from GitHub", "runner", runnerName, "id", runner.ID)
}

func (s *gcpRunnerScaler) shutdown(ctx context.Context) {
	if s.isDraining() {
		remaining := s.vmManager.ActiveCount()
		if remaining > 0 {
			s.logger.Info("shutdown while draining: leaving running VMs to finish", "remaining", remaining)
		}
		return
	}
	s.logger.Info("shutting down, deleting all VMs and cleaning up runners")

	// Get all tracked runner names before deleting VMs
	runnerNames := s.vmManager.ActiveRunnerNames()

	s.vmManager.DeleteAll(ctx)

	// Clean up any runner registrations from GitHub
	for _, name := range runnerNames {
		s.removeRunnerFromGitHub(ctx, name)
	}
}

// Compile-time check that gcpRunnerScaler implements listener.Scaler.
var _ listener.Scaler = (*gcpRunnerScaler)(nil)

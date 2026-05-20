-- Latest-attempt views. Snapshot queries read from these; ad-hoc per-attempt
-- analysis reads the base tables with explicit (run_id, run_attempt) params.

CREATE OR REPLACE VIEW `slang-runners.slang_ci.workflow_runs_latest` AS
SELECT * FROM `slang-runners.slang_ci.workflow_runs`
QUALIFY ROW_NUMBER() OVER (PARTITION BY run_id ORDER BY run_attempt DESC) = 1;

CREATE OR REPLACE VIEW `slang-runners.slang_ci.jobs_latest` AS
SELECT j.* FROM `slang-runners.slang_ci.jobs` j
JOIN (
  SELECT run_id, MAX(run_attempt) AS run_attempt
  FROM `slang-runners.slang_ci.workflow_runs`
  GROUP BY run_id
) latest
  ON j.run_id = latest.run_id AND j.run_attempt = latest.run_attempt;

CREATE OR REPLACE VIEW `slang-runners.slang_ci.job_steps_latest` AS
SELECT s.* FROM `slang-runners.slang_ci.job_steps` s
JOIN (
  SELECT run_id, MAX(run_attempt) AS run_attempt
  FROM `slang-runners.slang_ci.workflow_runs`
  GROUP BY run_id
) latest
  ON s.run_id = latest.run_id AND s.run_attempt = latest.run_attempt;

CREATE OR REPLACE VIEW `slang-runners.slang_ci.build_stats_latest` AS
SELECT b.* FROM `slang-runners.slang_ci.build_stats` b
JOIN (
  SELECT run_id, MAX(run_attempt) AS run_attempt
  FROM `slang-runners.slang_ci.workflow_runs`
  GROUP BY run_id
) latest
  ON b.run_id = latest.run_id AND b.run_attempt = latest.run_attempt;

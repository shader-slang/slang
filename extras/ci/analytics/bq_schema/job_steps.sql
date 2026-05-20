CREATE TABLE IF NOT EXISTS `slang-runners.slang_ci.job_steps`
(
  row_key STRING NOT NULL,
  job_id INT64 NOT NULL,
  run_id INT64 NOT NULL,
  run_attempt INT64 NOT NULL,
  run_created_at TIMESTAMP NOT NULL,
  step_number INT64 NOT NULL,
  step_name STRING,
  conclusion STRING,
  started_at TIMESTAMP,
  completed_at TIMESTAMP,
  duration_seconds FLOAT64
)
PARTITION BY DATE(run_created_at)
CLUSTER BY run_id, run_attempt, job_id;

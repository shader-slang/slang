CREATE TABLE IF NOT EXISTS `slang-runners.slang_ci.jobs`
(
  row_key STRING NOT NULL,
  job_id INT64,
  run_id INT64 NOT NULL,
  run_attempt INT64 NOT NULL,
  name STRING,
  caller_workflow_file STRING,
  caller_job_key STRING,
  called_workflow_file STRING,
  called_job_key STRING,
  node_kind STRING NOT NULL,
  graph_instance_key STRING,
  needs ARRAY<STRING>,
  conclusion STRING,
  started_at TIMESTAMP,
  completed_at TIMESTAMP,
  duration_seconds FLOAT64,
  queue_wait_seconds FLOAT64,
  runner_name STRING,
  runner_labels ARRAY<STRING>,
  job_type STRING,
  matrix_os STRING,
  matrix_arch STRING,
  matrix_compiler STRING,
  matrix_config STRING,
  matrix_gpu_tier STRING,
  run_created_at TIMESTAMP NOT NULL
)
PARTITION BY DATE(run_created_at)
CLUSTER BY run_id, run_attempt, node_kind;

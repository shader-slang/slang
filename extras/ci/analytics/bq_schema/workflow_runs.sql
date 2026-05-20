CREATE TABLE IF NOT EXISTS `slang-runners.slang_ci.workflow_runs`
(
  row_key STRING NOT NULL,
  run_id INT64 NOT NULL,
  run_attempt INT64 NOT NULL,
  workflow_file STRING,
  workflow_name STRING,
  event STRING,
  head_sha STRING,
  head_branch STRING,
  triggering_actor STRING,
  parent_run_id INT64,
  created_at TIMESTAMP NOT NULL,
  started_at TIMESTAMP,
  completed_at TIMESTAMP,
  conclusion STRING
)
PARTITION BY DATE(created_at)
CLUSTER BY run_id, run_attempt;

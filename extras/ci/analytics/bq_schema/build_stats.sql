CREATE TABLE IF NOT EXISTS `slang-runners.slang_ci.build_stats`
(
  row_key STRING NOT NULL,
  run_id INT64 NOT NULL,
  run_attempt INT64 NOT NULL,
  job_id INT64 NOT NULL,
  matrix_os STRING,
  matrix_arch STRING,
  matrix_compiler STRING,
  matrix_config STRING,
  cache_hits INT64,
  cache_misses INT64,
  cache_hit_rate FLOAT64,
  compile_requests INT64,
  run_created_at TIMESTAMP NOT NULL
)
PARTITION BY DATE(run_created_at)
CLUSTER BY run_id, run_attempt;

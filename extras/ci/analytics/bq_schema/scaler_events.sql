CREATE TABLE IF NOT EXISTS `slang-runners.slang_ci.scaler_events`
(
  row_key STRING NOT NULL,
  event_id STRING NOT NULL,
  event_time TIMESTAMP NOT NULL,
  event_type STRING,
  vm_name STRING,
  runner_name STRING,
  runner_labels ARRAY<STRING>,
  gpu_type STRING,
  zone STRING,
  reason STRING
)
PARTITION BY DATE(event_time)
CLUSTER BY event_type, vm_name;

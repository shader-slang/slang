#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <PR id>"
  exit 1
fi

export PATH=$PATH:$PWD

PR=$1

# # Trigger the pipeline
trigger_response=$(curl -s --request POST \
  --form token=${PIPELINE_TRIGGER_TOKEN} \
  --form ref=nv-master \
  --form "variables[PR]=$PR" \
  "$INTERNAL_PIPELINE_API_ENDPOINT/trigger/pipeline")

pipeline_id=$(echo "$trigger_response" | jq -r '.id')

if [[ -z "$pipeline_id" || "$pipeline_id" == "null" ]]; then
  echo "Failed to trigger the pipeline. Response: $trigger_response"
  exit 1
fi

echo "Pipeline triggered successfully. Pipeline ID: $pipeline_id"

start_time=$(date +%s)
running_timeout=$((10 * 60))    # 10 minutes in seconds
completion_timeout=$((30 * 60)) # 30 minutes in seconds

# Check pipeline status
pipeline_status=""
while true; do

  current_time=$(date +%s)
  elapsed_time=$((current_time - start_time))

  # Check if the pipeline hasn't started running after 10 minutes
  if [[ $elapsed_time -ge $running_timeout && "$pipeline_status" != "running" ]]; then
    echo "Error: Pipeline did not start running within 10 minutes."
    exit 1
  fi

  # Check if the pipeline hasn't finished after 30 minutes
  if [[ $elapsed_time -ge $completion_timeout ]]; then
    echo "Error: Pipeline did not finish within 30 minutes."
    exit 1
  fi

  status_response=$(curl -s --header "PRIVATE-TOKEN: $ACCESS_TOKEN" "$INTERNAL_PIPELINE_API_ENDPOINT/pipelines/$pipeline_id")
  pipeline_status=$(echo "$status_response" | jq -r '.status')

  if [[ -z "$pipeline_status" || "$pipeline_status" == "null" ]]; then
    echo "Failed to fetch pipeline status. Response: $status_response"
    exit 1
  fi

  if [[ "$pipeline_status" == "success" ]]; then
    echo "Pipeline finished successfully."
    break
  elif [[ "$pipeline_status" == "failed" || "$pipeline_status" == "canceled" ]]; then
    echo "Pipeline finished with status: $pipeline_status"
    break
  fi

  echo "Pipeline status: $pipeline_status. Checking again in 2 minutes..."
  sleep 120
done

echo "Fetching test report for Pipeline ID: $pipeline_id..."
test_report_response=$(curl -s --header "PRIVATE-TOKEN: $ACCESS_TOKEN" "$INTERNAL_PIPELINE_API_ENDPOINT/pipelines/$pipeline_id/test_report_summary")
curl_exit_code=$?

if [[ $curl_exit_code -ne 0 ]]; then
  echo "Error: Failed to execute curl command while fetching test report. Exit code: $curl_exit_code"
else
  echo "Test Report:"
  echo "$test_report_response" | jq --color-output
fi

# Exit with appropriate status code
if [[ "$pipeline_status" == "success" ]]; then
  exit 0
else
  exit 1
fi

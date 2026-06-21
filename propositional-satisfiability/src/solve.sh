#!/bin/bash

set -u

TIMEOUT_SECONDS=60

show_help() {
  cat <<EOF
    Usage:
    $(basename "$0") <PROBLEM_SOLVER> <INPUT_DIR> <OUTPUT_DIR>

    Runs PROBLEM_SOLVER for each one of the files in INPUT_DIR and writes the results in OUTPUT_DIR.
    Each execution has a timeout of $TIMEOUT_SECONDS. The script reports the time used by each exection in ms.
EOF
}

if [[ $# -eq 1 && ("$1" == "-h" || "$1" == "--help") ]]; then
  show_help
  exit 0
fi

if [[ $# -ne 3 ]]; then
  echo "Error: Invalid number of arguments" >&2
  exit 1
fi

PROBLEM_SOLVER=$(which "$1")
INPUT_DIR="$2"
OUTPUT_DIR="$3"

if [[ ! -e "$PROBLEM_SOLVER" || ! -x "$PROBLEM_SOLVER" ]]; then
  echo "Error: $PROBLEM_SOLVER can't be executed" >&2
  exit 1
fi

if [[ ! -d "$INPUT_DIR" ]]; then
  echo "Error: $INPUT_DIR does not exists" >&2
  exit 1
fi

mkdir -p "$OUTPUT_DIR"
input_files=$(ls "$INPUT_DIR" | grep "\.inp"$)
for input_file in $input_files; do
    base_name="$(basename "$input_file" .inp)"
    output_file="$OUTPUT_DIR/$base_name.out"
    echo "$base_name execution:"
    time timeout "$TIMEOUT_SECONDS" "$PROBLEM_SOLVER" < "$INPUT_DIR/$input_file" > "$output_file"
done



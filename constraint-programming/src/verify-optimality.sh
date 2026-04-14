#!/usr/bin/env bash

set -u

show_help() {
  cat <<EOF
Usage:
  $(basename "$0") <EXPECTED_RESULTS_FILE> <OUTPUT_DIR>

    For each entry, the script checks <OUTPUT_DIR>/<name>.out
    and compares the last line of that file with the expected value.
    The format of the result file is expected to be the same as in the materials
    provided.
EOF
}

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
  show_help
  exit 0
fi

if [[ $# -ne 2 ]]; then
  echo "Error: invalid number of arguments" >&2
  exit 1
fi

RESULTS_FILE="$1"
OUTPUT_DIR="$2"

if [[ ! -f "$RESULTS_FILE" ]]; then
  echo "Error: $RESULTS_FILE does not exists" >&2
  exit 1
fi

if [[ ! -d "$OUTPUT_DIR" ]]; then
  echo "Error: $OUTPUT_DIR does not exists" >&2
  exit 1
fi

while read -r line; do
    line=$(printf '%s' "$line" | tr -d '\r')

    name=$(echo "$line" | awk '{print $1}')
    expected=$(echo "$line" | awk '{print $2}')
    output_file="$OUTPUT_DIR/$name.out"

    if [[ ! -f "$output_file" ]]; then
        echo "$name -> NOT FOUND" >&2
        exit 1
    fi
    actual=$(tail -n 1 "$output_file" | tr -d '\r')
    
    if [[ "$expected" == "$actual" ]]; then
        echo "$name -> expected: $expected, got: $actual - OK"
    else
        echo "$name -> expected: $expected, got: $actual - ERROR"
    fi
done < "$RESULTS_FILE"

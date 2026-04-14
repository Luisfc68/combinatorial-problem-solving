#!/bin/bash

set -u

show_help() {
  cat <<EOF
    Usage:
    $(basename "$0") <CHECKER> <DIR>

    Runs CHECKER for each one of the files in DIR that end in .out
EOF
}

if [[ $# -eq 1 && ("$1" == "-h" || "$1" == "--help") ]]; then
  show_help
  exit 0
fi

if [[ $# -ne 2 ]]; then
  echo "Error: Invalid number of arguments" >&2
  exit 1
fi

CHECKER=$(which "$1")
DIR="$2"

if [[ ! -e "$CHECKER" || ! -x "$CHECKER" ]]; then
  echo "Error: $CHECKER can't be executed" >&2
  exit 1
fi

if [[ ! -d "$DIR" ]]; then
  echo "Error: $DIR does not exists" >&2
  exit 1
fi

files=$(ls "$DIR" | grep "\.out"$)
checked=0
for f in $files; do
    echo "$f -> $("$CHECKER" < "$DIR/$f")"
    let checked++
done
echo "Checked a total of $checked files"
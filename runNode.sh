#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: $0 <filename> <max_duplicates>"
  exit 1
fi

FILENAME=$1
MAX_DUPES=$2

for ((n=1; n<=MAX_DUPES; n++))
do
  FILES_ARG=$(printf "%s," $(yes "$FILENAME" | head -n "$n"))
  FILES_ARG=${FILES_ARG%,} # remove trailing comma

  TOTAL=0

  for i in {1..10}
  do
    TIME=$(node ./js/node/reader.mjs --files "$FILES_ARG")

    # Make sure TIME is just the number (strip ms if present)
    TIME=$(echo "$TIME" | grep -oE '[0-9]+')
    
    TOTAL=$((TOTAL + TIME))
  done

  AVG=$((TOTAL / 10))
  echo "${n},${AVG}"
done
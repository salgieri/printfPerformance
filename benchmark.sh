#!/usr/bin/env bash
set -euo pipefail

RUNS=${1:-100}
RESULTS_DIR="results"
BINARY="./stringperformance"

# Methods we extract (order matches the program output) — skip Generation Time (index 0)
METHODS=(
  "Slow Runtime"
  "Fast Runtime"
  "Hex2 Runtime"
  "Hex16 Runtime"
  "Hex16 Prefill Runtime"
  "Hex16 Aligned Runtime"
  "Hex16 Aligned Stored Runtime"
  "Hex32 Prefill Runtime"
  "Hex16 MemCpy Runtime"
  "Arith Runtime"
)

# --- Build ---
echo "Building..."
make

# --- Prepare results directory ---
rm -rf "$RESULTS_DIR"
mkdir -p "$RESULTS_DIR"

# --- Run benchmark ---
echo "Running benchmark $RUNS times..."
for i in $(seq 1 $RUNS); do
  padded=$(printf "%03d" "$i")
  echo "  Run $padded / $RUNS"
  $BINARY > "$RESULTS_DIR/run_${padded}.txt" 2>&1
done

echo ""
echo "Parsing results..."

# --- Parse all runs into a single CSV-like temp file ---
TMPFILE=$(mktemp)

for i in $(seq 1 $RUNS); do
  padded=$(printf "%03d" "$i")
  file="$RESULTS_DIR/run_${padded}.txt"
  for m in "${!METHODS[@]}"; do
    pattern="${METHODS[$m]}:"
    value=$(grep "$pattern" "$file" | head -1 | sed 's/.*: *\([0-9.]*\).*/\1/')
    if [[ -n "$value" ]]; then
      echo "$m,$value" >> "$TMPFILE"
    fi
  done
done

# --- Compute mean for each method ---
means=()
for m in "${!METHODS[@]}"; do
  mean=$(awk -F, -v idx="$m" '$1 == idx {s+=$2; c++} END {printf "%.3f", s/c}' "$TMPFILE")
  means+=("$mean")
done

# Baseline is index 0 (Slow Runtime)
baseline="${means[0]}"

# Find fastest (smallest time) — skip index 0 which is the baseline
fastest_idx=1
fastest_val="${means[1]}"
for i in $(seq 1 $((${#means[@]} - 1))); do
  if awk -v a="${means[$i]}" -v b="$fastest_val" 'BEGIN {exit !(a < b)}'; then
    fastest_val="${means[$i]}"
    fastest_idx=$i
  fi
done

# --- Output a single Markdown row ---
echo ""
echo "# Copy this row into README.md Performance Results table:"
printf "| "

for m in "${!METHODS[@]}"; do
  speedup=$(awk -v b="$baseline" -v v="${means[$m]}" 'BEGIN {printf "%.1f", b/v}')
  if [[ "$m" -eq "$fastest_idx" ]]; then
    printf "**%s s (%sx)** ⚡ | " "${means[$m]}" "$speedup"
  else
    printf "%s s (%sx) | " "${means[$m]}" "$speedup"
  fi
done
echo ""

# Cleanup
rm -f "$TMPFILE"

echo ""
echo "Done. Individual run files are in $RESULTS_DIR/"
echo "Runs completed: $(ls "$RESULTS_DIR"/run_*.txt 2>/dev/null | wc -l | tr -d ' ')"
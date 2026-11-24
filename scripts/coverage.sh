#!/usr/bin/env bash
# 覆盖率报告（gcovr）。只统计 src/ 与 include/mimage，third_party 与测试不计入。
set -uo pipefail

# shellcheck source=_build.sh
. "$(dirname "$0")/_build.sh"

lib="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="$(mimage_build_dir coverage)"
out_dir="${MIMAGE_COVERAGE_DIR:-$(mimage_build_root)/coverage-report}"

if ! command -v gcovr > /dev/null 2>&1; then
    echo "SKIP: 缺少 gcovr（python3 -m pip install gcovr）"
    exit 0
fi

mimage_ensure_build_dir "$lib" "$build_dir"

cmake -S "$lib" -B "$build_dir" -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="--coverage -O0 -g" \
      -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
      -Dmimage_build_tests=ON > /dev/null || exit 1
cmake --build "$build_dir" -j"$(nproc)" > /dev/null || exit 1
ctest --test-dir "$build_dir" > /dev/null || {
    echo "测试失败，覆盖率数据不完整" >&2
    exit 1
}

mkdir -p "$out_dir"
gcovr --root "$lib" \
      --filter "$lib/src/" --filter "$lib/include/" \
      --exclude ".*third_party.*" --exclude "$lib/tests/" \
      --gcov-ignore-parse-errors=negative_hits.warn_once_per_file \
      --exclude-throw-branches \
      --print-summary --txt "$out_dir/summary.txt" \
      --json-summary "$out_dir/summary.json" "$build_dir"

MIN_LINES="${MIMAGE_MIN_LINES:-70}"
MIN_FUNCTIONS="${MIMAGE_MIN_FUNCTIONS:-80}"
MIN_BRANCHES="${MIMAGE_MIN_BRANCHES:-50}"

python3 - "$out_dir/summary.json" "$MIN_LINES" "$MIN_FUNCTIONS" "$MIN_BRANCHES" <<'PYEOF'
import json
import sys

path, min_lines, min_functions, min_branches = sys.argv[1], float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4])
data = json.load(open(path, encoding="utf-8"))
failed = False
for name, actual, minimum in (
    ("行覆盖", data["line_percent"], min_lines),
    ("函数覆盖", data["function_percent"], min_functions),
    ("分支覆盖", data["branch_percent"], min_branches),
):
    ok = actual >= minimum
    print(f"{name}: {actual:5.1f}%  阈值 {minimum:5.1f}%  {'OK' if ok else 'FAIL'}")
    failed = failed or not ok
sys.exit(1 if failed else 0)
PYEOF

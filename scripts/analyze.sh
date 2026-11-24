#!/usr/bin/env bash
# 静态分析与内存检查。
#
# 用法:
#   scripts/analyze.sh              # clang-tidy + valgrind memcheck
#   scripts/analyze.sh tidy         # 只跑 clang-tidy
#   scripts/analyze.sh memcheck     # 只跑 valgrind memcheck
set -uo pipefail

# shellcheck source=_build.sh
. "$(dirname "$0")/_build.sh"

lib="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="${MIMAGE_BUILD_DIR:-$(mimage_build_dir test)}"
work_dir="$(mimage_build_root)/logs"
mkdir -p "$work_dir"
what="${1:-all}"
fail=0

run_tidy() {
    echo "== clang-tidy =="
    if ! command -v clang-tidy > /dev/null 2>&1; then
        echo "缺少 clang-tidy" >&2
        return 1
    fi

    local files rc=0
    # clang-tidy 只分析新增代码与公共头；移植的 codec/resize/drawing 由
    # 编译器告警、ASan/UBSan、valgrind 覆盖（见 QUALITY-GATES.md）。
    files=$(cd "$lib" && { find src -maxdepth 1 -name 'log.cpp'; find include -name '*.hpp'; } | sort)
    (cd "$lib" && for f in $files; do
        clang-tidy "$f" --quiet -- \
            -std=c++20 -I"$lib/include" -I"$build_dir/include" -I"$lib/src" -isystem "$lib/third_party" \
            -I"$lib/../mmemory/include" -I"$build_dir/mmemory/include" \
            -I"$lib/../mplatform/include" -I"$build_dir/mmemory/mplatform/include" 2>&1 | \
            grep -vE "^[0-9]+ warnings? generated|^Suppressed" || true
    done) | tee "$work_dir/tidy.log"
    if grep -qE "warning:|error:" "$work_dir/tidy.log"; then
        echo "!! clang-tidy 有告警"
        rc=1
    else
        echo "CHECK OK: clang-tidy clean"
    fi
    return "$rc"
}

run_valgrind() {
    echo "== valgrind memcheck =="
    if ! command -v valgrind > /dev/null 2>&1; then
        echo "缺少 valgrind" >&2
        return 1
    fi
    if [ ! -d "$build_dir/tests" ]; then
        echo "找不到 $build_dir/tests；先运行 scripts/build.sh test" >&2
        return 1
    fi

    local rc=0
    for binary in "$build_dir"/tests/mimage_*_tests; do
        [ -x "$binary" ] || continue
        if ! valgrind --quiet --error-exitcode=99 --leak-check=full \
                --errors-for-leak-kinds=definite "$binary" > /dev/null 2>"$work_dir/valgrind.log"; then
            echo "!! valgrind 失败: $(basename "$binary")" >&2
            cat "$work_dir/valgrind.log" >&2
            rc=1
        fi
    done
    [ "$rc" -eq 0 ] && echo "CHECK OK: valgrind clean"
    return "$rc"
}

case "$what" in
    tidy)     run_tidy || fail=1 ;;
    memcheck) run_valgrind ;;
    all)      run_tidy || fail=1; run_valgrind || fail=1 ;;
    *) echo "未知模式 $what" >&2; exit 2 ;;
esac
exit "$fail"

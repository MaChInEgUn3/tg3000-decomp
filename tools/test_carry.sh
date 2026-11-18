#!/bin/sh
# The rule that lets verified rows outlive later commits (tools/progress.py): in a scratch
# clone, rows stay accepted after additions to src/ and tools/bench/, and are refused
# after a changed line, after a harness change, and more than MAX_BEHIND commits on.
# Needs no ROM (CI runs it).
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
git clone -q "$root" "$tmp/r"
cd "$tmp/r"
git config user.email test@example.invalid
git config user.name test
# the rule as it is in the working tree (it may not be committed yet)
cp "$root/tools/progress.py" tools/progress.py
python3 -I tools/progress.py >/dev/null
git commit -qam "test setup" --allow-empty
check() {  # expect: 0 (accepted) or 1 (refused)
  s=0
  python3 -I tools/progress.py --check >/dev/null 2>&1 || s=$?
  if [ "$s" -ne "$1" ]; then echo "test_carry: $2: progress.py --check gave $s, wanted $1" >&2; exit 1; fi
  echo "ok   $2"
}
check 0 "the clone as it is"
base=$(git rev-parse HEAD)

# an addition to the registry and to a source: carried
printf '\n// an added line\n' >> tools/bench/routines.c
printf '\n// an added line\n' >> src/misc.c
check 1 "additions, the progress block not yet regenerated"
python3 -I tools/progress.py >/dev/null
check 0 "additions (rows carried)"
grep -q "verified at an earlier commit, the code only added to since | [1-9]" README.md \
  || { echo "test_carry: the README does not count the carried rows" >&2; exit 1; }
echo "ok   the README counts the carried rows"
git commit -qam "additions"
check 0 "additions, committed"

# a changed line in the registry: refused
git reset -q --hard "$base"
sed -i '0,/static/s//static /' tools/bench/routines.c
python3 -I tools/progress.py >/dev/null || true
check 1 "a changed line in the registry"

# a harness change, even an addition: refused
git reset -q --hard "$base"
printf '\n// an added line\n' >> tools/bench/difftest.c
python3 -I tools/progress.py >/dev/null || true
check 1 "an addition to the harness"

# too many commits since the run: refused
git reset -q --hard "$base"
for i in 1 2 3 4 5; do
  printf '\n// added %s\n' "$i" >> src/misc.c
  python3 -I tools/progress.py >/dev/null
  git commit -qam "addition $i"
done
check 0 "five commits of additions"
printf '\n// added 6\n' >> src/misc.c
python3 -I tools/progress.py >/dev/null || true
check 1 "a sixth commit of additions"
echo "carry rule: all cases as expected"

#!/usr/bin/env bash
# macos-merge.sh X86_64_DIR ARM64_DIR OUT_DIR
# joins two single-architecture builds into one universal build. a file that differs between them has to be a
# Mach-O file of each architecture and gets joined with lipo, everything else has to be byte-identical
set -euo pipefail

x86=$1
arm=$2
out=$3

fail() {
  echo "::error::$*"
  exit 1
}

listing() {
  (cd "$1" && find . -print | LC_ALL=C sort)
}

if ! diff <(listing "$x86") <(listing "$arm"); then
  fail "the x86_64 and arm64 builds hold different files"
fi

mkdir -p "$out"
ditto "$x86" "$out"

joined=0
while IFS= read -r -d '' path; do
  rel=${path#"$x86"/}
  other="$arm/$rel"
  if [ -L "$path" ]; then
    [ -L "$other" ] && [ "$(readlink "$path")" = "$(readlink "$other")" ] || fail "$rel: the symlinks differ"
    continue
  fi
  [ -f "$other" ] && [ ! -L "$other" ] || fail "$rel: a file in one build, something else in the other"
  [ "$(stat -f %p "$path")" = "$(stat -f %p "$other")" ] || fail "$rel: the file modes differ"
  if cmp -s "$path" "$other"; then
    continue
  fi
  archs_x86=$(lipo -archs "$path" 2> /dev/null) || fail "$rel differs between the builds and isn't a Mach-O file"
  archs_arm=$(lipo -archs "$other" 2> /dev/null) || fail "$rel differs between the builds and isn't a Mach-O file"
  if [ "$archs_x86" != x86_64 ] || [ "$archs_arm" != arm64 ]; then
    fail "$rel: expected x86_64 and arm64, got '$archs_x86' and '$archs_arm'"
  fi
  rm -f "$out/$rel"
  lipo -create "$path" "$other" -output "$out/$rel"
  chmod "$(stat -f %Lp "$path")" "$out/$rel"
  joined=$((joined + 1))
  echo "joined $rel"
done < <(find "$x86" \( -type f -o -type l \) -print0)

[ "$joined" -gt 0 ] || fail "the two builds are identical, nothing was joined"

while IFS= read -r -d '' path; do
  archs=$(lipo -archs "$path" 2> /dev/null) || continue
  for arch in x86_64 arm64; do
    case " $archs " in
      *" $arch "*) ;;
      *) fail "${path#"$out"/} has no $arch slice, only: $archs" ;;
    esac
  done
done < <(find "$out" -type f -print0)
echo "joined $joined files, every Mach-O file has x86_64 and arm64"

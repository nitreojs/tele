#!/usr/bin/env bash
# macos-merge.sh X86_64_DIR ARM64_DIR OUT_DIR
# joins two single-architecture builds into one universal build, the way xcode's universal build comes out:
# - a file in both builds that is byte-identical is kept as is (fat helpers, swift libraries both builds embed)
# - a file in both builds that differs has to be a Mach-O file of each architecture and gets joined with lipo
# - qt resource files and the asset catalog differ between any two builds (timestamps, equal images): the x86_64
#   build's copy is kept when the sizes match
# - the swift runtime libraries only the x86_64 build embeds (back-deployment below macos 10.14.4) are kept as is,
#   thin x86_64: arm64 never needs them, and xcode's universal build ships them the same way
# anything else that is in one build only is an error
set -euo pipefail

x86=$1
arm=$2
out=$3

fail() {
  echo "::error::$*"
  exit 1
}

listing() {
  (cd "$1" && find . \( -type f -o -type l \) -print | sed 's|^\./||' | LC_ALL=C sort)
}

archs() {
  lipo -archs "$1" 2> /dev/null || true
}

x86_files=$(listing "$x86")
arm_files=$(listing "$arm")
only_arm=$(LC_ALL=C comm -13 <(echo "$x86_files") <(echo "$arm_files"))
only_x86=$(LC_ALL=C comm -23 <(echo "$x86_files") <(echo "$arm_files"))

[ -z "$only_arm" ] || fail "only the arm64 build has: $(echo "$only_arm" | tr '\n' ' ')"
while IFS= read -r rel; do
  [ -n "$rel" ] || continue
  case "$rel" in
    */Contents/Frameworks/libswift*.dylib) ;;
    *) fail "only the x86_64 build has $rel" ;;
  esac
  [ ! -L "$x86/$rel" ] || fail "$rel: a symlink only the x86_64 build has"
  [ "$(archs "$x86/$rel")" = x86_64 ] || fail "$rel: expected a thin x86_64 swift library, got '$(archs "$x86/$rel")'"
  echo "kept the x86_64 swift library $rel"
done <<< "$only_x86"

mkdir -p "$out"
ditto "$x86" "$out"

joined=0
while IFS= read -r rel; do
  [ -n "$rel" ] || continue
  path="$x86/$rel"
  other="$arm/$rel"
  if [ -L "$path" ] || [ -L "$other" ]; then
    [ -L "$path" ] && [ -L "$other" ] && [ "$(readlink "$path")" = "$(readlink "$other")" ] \
      || fail "$rel: the symlinks differ"
    continue
  fi
  [ "$(stat -f %p "$path")" = "$(stat -f %p "$other")" ] || fail "$rel: the file modes differ"
  if cmp -s "$path" "$other"; then
    continue
  fi
  archs_x86=$(archs "$path")
  archs_arm=$(archs "$other")
  if [ -z "$archs_x86" ] && [ -z "$archs_arm" ]; then
    case "$rel" in
      *.rcc|*/Assets.car)
        # qt's rcc stores each file's modification time and actool picks one of several equal images by name, so
        # these differ between any two builds of the same sources: keep the x86_64 build's copy
        [ "$(stat -f %z "$path")" = "$(stat -f %z "$other")" ] || fail "$rel: the sizes differ between the builds"
        echo "kept the x86_64 build's $rel"
        continue
        ;;
    esac
  fi
  if [ "$archs_x86" != x86_64 ] || [ "$archs_arm" != arm64 ]; then
    fail "$rel differs between the builds: expected thin x86_64 and arm64 Mach-O files, got '$archs_x86' and '$archs_arm'"
  fi
  rm -f "$out/$rel"
  lipo -create "$path" "$other" -output "$out/$rel"
  chmod "$(stat -f %Lp "$path")" "$out/$rel"
  [ "$(archs "$out/$rel")" = "x86_64 arm64" ] || fail "$rel: lipo gave '$(archs "$out/$rel")'"
  joined=$((joined + 1))
  echo "joined $rel"
done < <(LC_ALL=C comm -12 <(echo "$x86_files") <(echo "$arm_files"))

[ "$joined" -gt 0 ] || fail "the two builds are identical, nothing was joined"
for main in "$out"/*.app/Contents/MacOS/*; do
  [ "$(archs "$main")" = "x86_64 arm64" ] || fail "${main#"$out"/} has '$(archs "$main")', not x86_64 and arm64"
done
[ "$(listing "$out")" = "$x86_files" ] || fail "the joined build doesn't hold exactly the x86_64 build's files"
echo "joined $joined files, the executables have x86_64 and arm64"

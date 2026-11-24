#!/usr/bin/env bash
# macos libraries live in ghcr instead of the actions cache: they're too big for it and ghcr isn't scoped to a branch
set -euo pipefail

action=$1
shift

owner=$(printf '%s' "$GITHUB_REPOSITORY_OWNER" | tr '[:upper:]' '[:lower:]')
repo="ghcr.io/$owner/tele-macos-libs"
work="$RUNNER_TEMP/macos-libs"
printf '%s' "$GH_TOKEN" | oras login ghcr.io -u "$GITHUB_ACTOR" --password-stdin > /dev/null
rm -rf "$work"
mkdir -p "$work"

case "$action" in
  pull)
    for tag in "$@"; do
      # a tag pushed moments ago can fail to pull for a while: retry before trying the next tag
      for attempt in 1 2 3 4; do
        if oras pull --output "$work" "$repo:$tag" > "$RUNNER_TEMP/oras-pull.log" 2>&1; then
          cat "$work"/libs.tar.zst.part-* | zstd -d -q | tar -xf -
          rm -rf "$work"
          echo "restored $repo:$tag"
          exit 0
        fi
        echo "pull $repo:$tag attempt $attempt failed: $(tail -n 3 "$RUNNER_TEMP/oras-pull.log" | tr '\n' ' ')"
        rm -rf "$work"
        mkdir -p "$work"
        # with fallbacks a missing tag is expected; a single tag was just pushed and must show up
        if [ $# -gt 1 ] && grep -qi "not found" "$RUNNER_TEMP/oras-pull.log"; then
          break
        fi
        sleep $((attempt * 30))
      done
    done
    echo "no libraries in $repo for: $*"
    exit 1
    ;;
  push)
    key=$1
    tar -cf - Libraries ThirdParty | zstd -q -T0 -3 | (cd "$work" && split -b 1900m - libs.tar.zst.part-)
    (cd "$work" && oras push "$repo:$key" --artifact-type application/vnd.tele.macos-libs libs.tar.zst.part-*)
    oras tag "$repo:$key" latest
    rm -rf "$work"
    ;;
  *)
    echo "usage: $0 pull TAG... | push TAG" >&2
    exit 2
    ;;
esac

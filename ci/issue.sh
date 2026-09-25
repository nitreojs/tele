#!/usr/bin/env bash
# ci/issue.sh open LABEL TITLE BODY_FILE  open an issue, or comment on the open one with the same title
# ci/issue.sh close LABEL COMMENT         close every open issue that has LABEL
# Needs GH_TOKEN and GH_REPO.
set -euo pipefail

action=$1
label=$2

case "$action" in
  open)
    export TITLE=$3
    body=$4
    gh label create "$label" --color d73a4a --force > /dev/null
    number=$(gh issue list --state open --label "$label" --json number,title \
      --jq '.[] | select(.title == env.TITLE) | .number' | head -n1)
    if [ -n "$number" ]; then
      gh issue comment "$number" --body-file "$body"
    else
      gh issue create --title "$TITLE" --label "$label" --body-file "$body"
    fi
    ;;
  close)
    comment=$3
    for number in $(gh issue list --state open --label "$label" --json number --jq '.[].number'); do
      gh issue close "$number" --comment "$comment"
    done
    ;;
  *)
    echo "usage: $0 open LABEL TITLE BODY_FILE | close LABEL COMMENT" >&2
    exit 2
    ;;
esac

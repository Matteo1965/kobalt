#!/usr/bin/env bash
# Verify the exact, pinned CRengine patch sequence before starting a Qt5 build.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$root"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

url="$(jq -r '.crengine.url' dependencies/manifest.json)"
sha="$(jq -r '.crengine.sha256' dependencies/manifest.json)"
directory="$(jq -r '.crengine.directory' dependencies/manifest.json)"
case "$url" in https://codeload.github.com/*) ;; *) echo "Unexpected CRengine URL: $url" >&2; exit 1;; esac
curl --fail --location --retry 3 --silent --show-error "$url" -o "$tmp/crengine.tar.gz"
printf '%s  %s\n' "$sha" "$tmp/crengine.tar.gz" | sha256sum --check --status
tar -xzf "$tmp/crengine.tar.gz" -C "$tmp"
source_dir="$tmp/$directory"
test -d "$source_dir/crengine/src" || { echo "Missing CRengine source tree" >&2; exit 1; }

while IFS= read -r name || [[ -n "$name" ]]; do
    [[ -n "$name" ]] || continue
    file="$root/dependencies/patches/$name"
    test -f "$file" || { echo "Missing patch: $file" >&2; exit 1; }
    echo "Applying CRengine patch: $name"
    (cd "$source_dir" && patch --batch --forward -p1 -i "$file") || {
        echo "FAIL: CRengine patch does not apply: $name" >&2
        exit 1
    }
done < dependencies/crengine-patches.txt
echo "PASS: all pinned CRengine patches apply in production order"

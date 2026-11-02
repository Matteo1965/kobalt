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
    if [[ "$name" == "crengine-hungarian-extended.patch" ]]; then
        echo "Pre-Hungarian active hyphenation call sites:"
        grep -n -F -C 12 'getHyphMethod()->hyphenate' "$source_dir/crengine/src/lvtextfm.cpp" || true
        grep -n -F -C 12 'hyphenate(fmt->m_text' "$source_dir/crengine/src/lvtextfm.cpp" || true
    fi
    if [[ "$name" == "crengine-hungarian-width.patch" ]]; then
        echo "Pre-width CRengine candidate sites:"
        grep -n -C 5 -E "hyphen found, but max width reached|widths\\[i\\].*_hyphen_width|LCHAR_ALLOW_HYPH_WRAP_AFTER" "$source_dir/crengine/src/lvtextfm.cpp" | tail -100 || true
    fi
    echo "Applying CRengine patch: $name"
    (cd "$source_dir" && patch --batch --forward -p1 -i "$file") || {
        echo "FAIL: CRengine patch does not apply: $name" >&2
        exit 1
    }
done < dependencies/crengine-patches.txt
# Validate the patched engine source, not only the text of separate patches.
# The primary line breaker must call the HU helper once; the formatted-text
# width gate and the renderer must consume its replacement marker.
engine="$source_dir/crengine/src/lvtextfm.cpp"
require_one() {
    local pattern="$1" description="$2" count
    count="$(grep -F -c "$pattern" "$engine" || true)"
    if [[ "$count" -ne 1 ]]; then
        echo "FAIL: $description: expected 1 occurrence, got $count" >&2
        exit 1
    fi
    echo "PASS: $description (line $(grep -F -n "$pattern" "$engine" | cut -d: -f1))"
}
echo "CRengine hyphenation call-site inventory:"
grep -n -C 7 -E 'getHyphMethod\\(\\)|hyphen_found|huAddExtendedHyphenation|max_extent reached' "$engine" | tail -170 || true
require_one 'huAddExtendedHyphenation(fmt->m_text+wstart' 'HU helper integrated in line breaking'
hu_line="$(grep -F -n 'huAddExtendedHyphenation(fmt->m_text+wstart' "$engine" | cut -d: -f1)"
if [[ "$hu_line" -lt 5000 ]]; then
    echo "FAIL: HU integration landed outside active CRengine line breaker (line $hu_line)" >&2
    exit 1
fi
if grep -Fq 'if ( m_srcs[wordpos]->lang_cfg->getHyphMethod()->hyphenate(' "$engine"; then
    echo "FAIL: original direct hyphenation call remains, HU override is inactive" >&2
    exit 1
fi
require_one 'huCandidateWidth > max_extent' 'formatted-text width gate includes HU replacement'
require_one 'if ( flgHyphen && (word->hungarian_replace) )' 'renderer consumes HU replacement flag'
echo "PASS: all pinned CRengine patches apply in production order"

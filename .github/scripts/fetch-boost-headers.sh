#!/bin/sh
# Unpack the Boost headers into Dependencies/boost_<version>/, the include
# directory the Visual Studio and Xcode projects expect.
# usage: fetch-boost-headers.sh <version> <sha256 of the .tar.gz>
set -eu

version=$1
sha256=$2
name="boost_$(printf '%s' "$version" | tr . _)"
repo=$(cd -- "$(dirname -- "$0")/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

curl -fsSL --retry 3 -o "$work/$name.tar.gz" \
	"https://archives.boost.io/release/$version/source/$name.tar.gz"
if command -v sha256sum >/dev/null 2>&1; then
	checksum='sha256sum'
else
	checksum='shasum -a 256'
fi
printf '%s  %s\n' "$sha256" "$work/$name.tar.gz" | $checksum -c -
tar -xzf "$work/$name.tar.gz" -C "$repo/Dependencies" "$name/boost"

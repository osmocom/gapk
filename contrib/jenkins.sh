#!/bin/sh
# jenkins build helper script for jenkins.osmocom.org

if ! [ -x "$(command -v osmo-build-dep.sh)" ]; then
	echo "Error: We need to have scripts/osmo-deps.sh from http://git.osmocom.org/osmo-ci/ in PATH !"
	exit 2
fi

set -ex

base="$PWD"
deps="$base/deps"
inst="$deps/install"
export deps inst

osmo-clean-workspace.sh

mkdir "$deps" || true

osmo-build-dep.sh libosmocore "" ac_cv_path_DOXYGEN=false

verify_value_string_arrays_are_terminated.py $(find . -name "*.[hc]")

export PKG_CONFIG_PATH="$inst/lib/pkgconfig:$PKG_CONFIG_PATH"
export LD_LIBRARY_PATH="$inst/lib"

set +x
echo
echo
echo
echo " =============================== gapk ==============================="
echo
set -x

cd "$base"
autoreconf --install --force
./configure --enable-sanitize
$MAKE $PARALLEL_MAKE
LD_LIBRARY_PATH="$inst/lib" $MAKE check || cat-testlogs.sh
LD_LIBRARY_PATH="$inst/lib" DISTCHECK_CONFIGURE_FLAGS="" $MAKE distcheck || cat-testlogs.sh
$MAKE maintainer-clean

osmo-clean-workspace.sh

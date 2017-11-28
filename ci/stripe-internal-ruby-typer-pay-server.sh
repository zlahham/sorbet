#!/bin/bash
set -eux

DIR=./pay-server

if [ ! -d $DIR ]; then
    echo "$DIR doesn't exist"
    exit 1
fi

cd $DIR

if [ ! -f "../ci/stripe-internal-ruby-typer-pay-server-sha" ]; then
    echo "ci/stripe-internal-ruby-typer-pay-server-sha doesn't exist"
    exit 1
fi
git checkout "$(cat ../ci/stripe-internal-ruby-typer-pay-server-sha)"

cd -

bazel build main:ruby-typer -c opt

TMP="$(mktemp)"

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
find $DIR -name *.rb | sort > $TMP

ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=verbosity=1:log_threads=1 ./bazel-bin/main/ruby-typer --quiet --error-stats --trace @$TMP

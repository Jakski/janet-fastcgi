#!/usr/bin/env bash
################################################################################
# Very crude FastCGI responsiveness test with Valgrind report.
# Janet installation managed by asdf is expected.
################################################################################

set -euo pipefail

declare asdf_dir
asdf_dir=$(asdf where janet)
jpm install
LD_LIBRARY_PATH="${asdf_dir}/lib" valgrind \
	--trace-children=yes \
	--leak-check=full \
	spawn-fcgi -n -p 9090 -- \
	"${asdf_dir}/bin/janet" "test.janet" &
while ! nc -zw 1 127.0.0.1 9090; do
	printf "%s\n" "Waiting for FastCGI on port 9090"
	sleep 1
done
printf "%s" "test-content" \
	| CONTENT_LENGTH=9 TEST_HEADER="test-header" cgi-fcgi -bind -connect 127.0.0.1:9090 2>&1 \
	| grep -cE "test-(content|header|error)" \
	| grep -qE '^3$'
cgi-fcgi -bind -connect 127.0.0.1:9090 &>/dev/null || true
wait

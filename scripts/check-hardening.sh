#!/usr/bin/env bash
# Verifies that ELF binaries were built with the expected exploit mitigations.
#
#   scripts/check-hardening.sh [--require-cet] FILE...
#
# Fails if a file lacks: PIE (executables), full RELRO (GNU_RELRO + BIND_NOW)
# or a non-executable stack. --require-cet also requires Intel CET (IBT and
# SHSTK), which only our C++ code enables (Rust does not support it yet).
set -euo pipefail

require_cet=0
if [[ "${1:-}" == "--require-cet" ]]; then
  require_cet=1
  shift
fi
if [[ $# -eq 0 ]]; then
  echo "usage: $0 [--require-cet] FILE..." >&2
  exit 2
fi

status=0
fail() {
  echo "  FAIL: $1"
  status=1
}

for file in "$@"; do
  echo "== ${file}"
  header=$(readelf --file-header --wide "${file}")
  segments=$(readelf --program-headers --wide "${file}")
  dynamic=$(readelf --dynamic --wide "${file}")
  notes=$(readelf --notes --wide "${file}" 2>/dev/null || true)
  symbols=$(readelf --dyn-syms --wide "${file}")

  if grep -q 'Type:[[:space:]]*EXEC' <<<"${header}"; then
    fail "not position independent (ET_EXEC): no ASLR"
  elif grep -q 'Type:[[:space:]]*DYN' <<<"${header}"; then
    echo "  ok:   position independent (ET_DYN)"
  fi

  if grep -q 'GNU_RELRO' <<<"${segments}"; then
    echo "  ok:   GNU_RELRO segment"
  else
    fail "no GNU_RELRO segment"
  fi

  if grep -Eq 'BIND_NOW|FLAGS.*NOW' <<<"${dynamic}"; then
    echo "  ok:   BIND_NOW (full RELRO)"
  else
    fail "no BIND_NOW: GOT stays writable (partial RELRO)"
  fi

  stack=$(grep 'GNU_STACK' <<<"${segments}" || true)
  if [[ -z "${stack}" ]]; then
    fail "no GNU_STACK segment: stack may be executable"
  elif grep -q 'RWE' <<<"${stack}"; then
    fail "executable stack (GNU_STACK RWE)"
  else
    echo "  ok:   non-executable stack"
  fi

  if [[ ${require_cet} -eq 1 ]]; then
    if grep -q 'IBT' <<<"${notes}" && grep -q 'SHSTK' <<<"${notes}"; then
      echo "  ok:   Intel CET (IBT + SHSTK)"
    else
      fail "Intel CET (IBT + SHSTK) not enabled"
    fi
  fi

  # Informational: these depend on what the code does, so they are not required.
  if grep -q '__stack_chk_fail' <<<"${symbols}"; then
    echo "  info: stack protector in use"
  fi
  if grep -Eq '__[a-z_]+_chk(@|[[:space:]]|$)' <<<"${symbols}"; then
    echo "  info: _FORTIFY_SOURCE checks in use"
  fi
done

exit "${status}"

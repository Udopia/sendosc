#!/usr/bin/env bash
set -euo pipefail

for command in sclang scsynth; do
    if ! command -v "$command" >/dev/null 2>&1; then
        echo "Missing $command. Install SuperCollider (Debian/Ubuntu: sudo apt install supercollider)." >&2
        exit 1
    fi
done

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
sender="$script_dir/../build/sendosc"
if [[ ! -x "$sender" ]]; then
    echo "Build sendosc first; executable not found: $sender" >&2
    exit 1
fi

log=$(mktemp)
sc_pid=
ready=false
cleanup() {
    if [[ -n "$sc_pid" ]]; then
        if kill -0 "$sc_pid" 2>/dev/null; then
            if grep -q '^SENDOSC_TEST_STARTED$' "$log"; then
                if ! "$sender" 127.0.0.1 57120 /sendosc/test/quit i 1; then
                    echo "Could not send the SuperCollider shutdown request." >&2
                fi
                for ((attempt=0; attempt<30; attempt++)); do
                    kill -0 "$sc_pid" 2>/dev/null || break
                    sleep .1
                done
            fi
            if kill -0 "$sc_pid" 2>/dev/null; then
                kill "$sc_pid"
            fi
        fi
        wait "$sc_pid" || true
    fi
    rm -f -- "$log"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

QT_QPA_PLATFORM=offscreen sclang -D -u 57120 "$script_dir/sctest.scd" >"$log" 2>&1 &
sc_pid=$!
for ((attempt=0; attempt<300; attempt++)); do
    if ! kill -0 "$sc_pid" 2>/dev/null; then
        echo "SuperCollider exited during startup:" >&2
        cat "$log" >&2
        exit 1
    fi
    if grep -q '^SENDOSC_TEST_READY$' "$log"; then
        ready=true
        break
    fi
    sleep .1
done
if [[ "$ready" != true ]]; then
    echo "SuperCollider was not ready within 30 seconds. Check the audio device and JACK configuration:" >&2
    cat "$log" >&2
    exit 1
fi

i=1
prev=0
while ((i <= 100000000)); do
    if ! kill -0 "$sc_pid" 2>/dev/null; then
        echo "SuperCollider exited during the sound test:" >&2
        cat "$log" >&2
        exit 1
    fi
    play=$((i % 3 - 1))
    echo "$play"
    "$sender" 127.0.0.1 57120 /program/state i "$play"
    ((i += prev, prev = i - prev))
    sleep .1
done
cat "$log"
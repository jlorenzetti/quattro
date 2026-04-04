#!/usr/bin/env bash
#
# Build a pixel-perfect GIF from a VICE ZMBV (or similar) capture using ffmpeg.
#
# Usage:
#   ./tools/make_demo_gif.sh INPUT_AVI [OUTPUT_GIF]
#   make demo_gif INPUT=path/to/capture.avi [OUTPUT=docs/quattro-demo.gif]
#
# Optional environment variables (defaults in parentheses):
#   DURATION — seconds to encode (30)
#   FPS      — output frame rate (15)
#   WIDTH    — output width in pixels; height scales (-1) (640)
#
set -euo pipefail

if [ $# -lt 1 ]; then
	echo "Usage: $0 INPUT_AVI [OUTPUT_GIF]" >&2
	exit 1
fi

INPUT="$1"
OUTPUT="${2:-docs/quattro-demo.gif}"

DURATION="${DURATION:-30}"
FPS="${FPS:-15}"
WIDTH="${WIDTH:-640}"

PALETTE="$(mktemp /tmp/quattro-demo-palette.XXXXXX.png)"
trap 'rm -f "$PALETTE"' EXIT

ffmpeg -t "$DURATION" -i "$INPUT" \
	-an \
	-vf "fps=${FPS},scale=${WIDTH}:-1:flags=neighbor,palettegen" \
	-frames:v 1 \
	-y "$PALETTE"

ffmpeg -t "$DURATION" -i "$INPUT" \
	-i "$PALETTE" \
	-an \
	-lavfi "fps=${FPS},scale=${WIDTH}:-1:flags=neighbor[x];[x][1:v]paletteuse" \
	-y "$OUTPUT"

echo "Wrote $OUTPUT"

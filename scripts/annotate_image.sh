#!/bin/bash

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

FONT="$DIR/RobotoMono-Bold.ttf"

# Make sure font exists
ls "$FONT"
echo "$DIR"

set -v

LABEL="`date --rfc-3339=seconds`\n`git rev-parse --short HEAD`"
convert -background black -fill white -pointsize 12 -font "$FONT" -size 300x36 label:"$LABEL" -bordercolor black -border 3 $1 +swap -append $1


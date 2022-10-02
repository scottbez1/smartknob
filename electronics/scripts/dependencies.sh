#!/bin/bash
set -e

if [[ -z "${GITHUB_WORKFLOW}" ]]; then
    >&2 echo "Aborting! This script is meant to be run in CI (Github Actions) only. It may modify/damage your system configuration if run outside of CI."
    exit 1
fi

set -v

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

sudo add-apt-repository --yes ppa:kicad/kicad-6.0-releases
sudo apt-get update -qq
sudo DEBIAN_FRONTEND=noninteractive apt install -y kicad kicad-packages3d inkscape poppler-utils xdotool recordmydesktop python3-dev python3-pip xvfb

sudo python3 -m pip install psutil kikit==1.1.1 xvfbwrapper

mkdir -p ~/.config/kicad
cp /usr/share/kicad/template/fp-lib-table ~/.config/kicad/
cp /usr/share/kicad/template/sym-lib-table ~/.config/kicad/

cp "$DIR/config/eeschema" ~/.config/kicad/
cp "$DIR/config/pcbnew" ~/.config/kicad/

# Install ImageMagick policy that allows PDF conversion (safe in CI because we control all inputs/outputs)
sudo cp "$DIR/config/policy.xml" /etc/ImageMagick-6/policy.xml

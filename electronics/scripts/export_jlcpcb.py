#!/usr/bin/env python3

#   Copyright 2021 Scott Bezek and the splitflap contributors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import argparse
import logging
import os
import subprocess
import sys

electronics_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
repo_root = os.path.dirname(electronics_root)
sys.path.append(repo_root)

from util import file_util
from export_util import (
    versioned_file,
)

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def export_jlcpcb(pcb, schematic, alt_fields, release_prefix):
    pcb_file = os.path.abspath(pcb)

    output_dir = os.path.join(electronics_root, 'build', os.path.splitext(os.path.basename(pcb_file))[0] + '-jlc')
    file_util.mkdir_p(output_dir)

    with versioned_file(pcb_file, release_prefix):
        command = [
            'kikit',
            'fab',
            'jlcpcb',
        ]
        if schematic is not None:
            schematic_file = os.path.abspath(schematic)
            command += [
                '--assembly',
                '--schematic',
                schematic_file,
                '--field',
            ]
            command.append(','.join(alt_fields + ['LCSC']))
        command += [
            pcb_file,
            output_dir,
        ]
        subprocess.check_call(command)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('pcb')
    parser.add_argument('--assembly-schematic')
    parser.add_argument('--alt-fields', nargs='+')
    parser.add_argument('--release-prefix', type=str, required=True, help='Tag prefix to check if this is a tagged/versioned release. E.g. "releases/" for tags like "releases/v1.0"')
    args = parser.parse_args()
    export_jlcpcb(args.pcb, args.assembly_schematic, args.alt_fields, args.release_prefix)

#!/usr/bin/env python3
#   Copyright 2015-2021 Scott Bezek and the splitflap contributors
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
import pcbnew
import shutil
import subprocess

from collections import namedtuple

import pcb_util

electronics_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)


def run(pcb_file, release_prefix):
    output_directory = os.path.join(electronics_root, 'build')
    temp_dir = os.path.join(output_directory, 'temp_pdfs')
    shutil.rmtree(temp_dir, ignore_errors=True)
    try:
        os.makedirs(temp_dir)
        plot_to_directory(pcb_file, output_directory, temp_dir, release_prefix)
    finally:
        shutil.rmtree(temp_dir, ignore_errors=True)


def plot_to_directory(pcb_file, output_directory, temp_dir, release_prefix):
    board_name = os.path.splitext(os.path.basename(pcb_file))[0]

    with pcb_util.get_plotter(pcb_file, temp_dir, release_prefix) as plotter:
        plotter.plot_options.SetDrillMarksType(pcbnew.PCB_PLOT_PARAMS.NO_DRILL_SHAPE)
        plotter.plot_options.SetExcludeEdgeLayer(False)

        LayerDef = namedtuple('LayerDef', ['layer', 'mirror'])
        layers = [
            LayerDef(pcbnew.F_Cu, False),
            LayerDef(pcbnew.B_Cu, True),
            LayerDef(pcbnew.F_SilkS, False),
            LayerDef(pcbnew.B_SilkS, True),
            LayerDef(pcbnew.F_Mask, False),
            LayerDef(pcbnew.B_Mask, True),
            LayerDef(pcbnew.F_Paste, False),
        ]

        pdfs = []
        for layer in layers:
            plotter.plot_options.SetMirror(layer.mirror)
            output_filename = plotter.plot(layer.layer, pcbnew.PLOT_FORMAT_PDF)
            pdfs.append(output_filename)

        _, map_file = plotter.plot_drill()
        pdfs.append(map_file)

        output_pdf_filename = os.path.join(output_directory, '%s-pcb-packet.pdf' % (board_name,))

        command = ['pdfunite'] + pdfs + [output_pdf_filename]
        subprocess.check_call(command)

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Generate a pdf of the PCB')
    parser.add_argument('--release-prefix', type=str, required=True, help='Tag prefix to check if this is a tagged/versioned release. E.g. "releases/" for tags like "releases/v1.0"')
    parser.add_argument('pcb_file')
    args = parser.parse_args()
    run(args.pcb_file, args.release_prefix)


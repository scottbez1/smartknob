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
import tempfile

from contextlib import contextmanager

from export_util import (
    get_versioned_contents
)

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

@contextmanager
def versioned_board(filename, release_search_prefix):
    _, versioned_contents = get_versioned_contents(filename, release_search_prefix)
    with tempfile.NamedTemporaryFile(suffix='.kicad_pcb', mode='w') as temp_pcb:
        logger.debug('Writing to %s', temp_pcb.name)
        temp_pcb.write(versioned_contents)
        temp_pcb.flush()

        logger.debug('Load board')
        board = pcbnew.LoadBoard(temp_pcb.name)
        yield board

@contextmanager
def get_plotter(pcb_filename, build_directory, release_prefix):
    with versioned_board(pcb_filename, release_prefix) as board:
        yield Plotter(board, build_directory)


class Plotter(object):
    def __init__(self, board, build_directory):
        self.board = board
        self.build_directory = build_directory
        self.plot_controller = pcbnew.PLOT_CONTROLLER(board)
        self.plot_options = self.plot_controller.GetPlotOptions()
        self.plot_options.SetOutputDirectory(build_directory)

        self.plot_options.SetPlotFrameRef(False)
        # self.plot_options.SetLineWidth(pcbnew.FromMM(0.35))
        self.plot_options.SetScale(1)
        self.plot_options.SetUseAuxOrigin(True)
        self.plot_options.SetMirror(False)
        self.plot_options.SetExcludeEdgeLayer(True)

    def plot(self, layer, plot_format):
        layer_name = self.board.GetLayerName(layer)
        logger.info('Plotting layer %s (kicad layer=%r)', layer_name, layer)
        self.plot_controller.SetLayer(layer)
        self.plot_controller.OpenPlotfile(layer_name, plot_format , 'Plot')
        output_filename = self.plot_controller.GetPlotFileName()
        self.plot_controller.PlotLayer()
        self.plot_controller.ClosePlot()
        return output_filename

    def plot_drill(self):
        board_name = os.path.splitext(os.path.basename(self.board.GetFileName()))[0]
        logger.info('Plotting drill file')
        drill_writer = pcbnew.EXCELLON_WRITER(self.board)
        drill_writer.SetMapFileFormat(pcbnew.PLOT_FORMAT_PDF)

        mirror = False
        minimalHeader = False
        offset = pcbnew.wxPoint(0, 0)
        merge_npth = True
        drill_writer.SetOptions(mirror, minimalHeader, offset, merge_npth)

        metric_format = True
        drill_writer.SetFormat(metric_format)

        generate_drill = True
        generate_map = True
        drill_writer.CreateDrillandMapFilesSet(self.build_directory, generate_drill, generate_map)

        drill_file_name = os.path.join(
            self.build_directory,
            '%s.drl' % (board_name,)
        )

        map_file_name = os.path.join(
            self.build_directory,
            '%s-drl_map.pdf' % (board_name,)
        )
        return drill_file_name, map_file_name


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Test pcb util')
    parser.add_argument('--release-prefix', type=str, required=True, help='Tag prefix to check if this is a tagged/versioned release. E.g. "releases/" for tags like "releases/v1.0"')
    parser.add_argument('input_file', help='Input .kicad_pcb file')
    args = parser.parse_args()
    with versioned_board(args.input_file, args.release_prefix) as board:
        logger.info('Loaded %s', board.GetFileName())
        for module in board.GetModules():
            logger.info('Module %s: %s', module.GetReference(), module.GetValue())

# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0

'''Board priority ordering script
'''

import argparse
from os.path import exists
from pathlib import Path
import sys
import yaml

def parse_args():
    parser = argparse.ArgumentParser(description='processing',
                                     allow_abbrev=False)
    parser.add_argument('--boards', metavar='BOARDS',
                        help='Comma separated list of boards', required=True,
                        type=str, nargs='+')
    parser.add_argument('--board-dirs', metavar='BOARD_DIRS',
                        help='Comma separated list of board directories',
                        required=True, type=Path, nargs='+')

    return parser.parse_args()

def do_run_common(boards, board_dirs):
    # Holds a dictionary of board flashing priorities which allows for
    # flashing specific boards prior to others.
    board_priority = []

    for board_dir in board_dirs:
        board_runner_file = board_dir / 'flash_runner.yml'
        flash_order_present = False

        if exists(board_runner_file):
            # Load flash order from file, if the entries exist
            with open(board_runner_file, 'r') as f:
                board_runner_yaml = yaml.safe_load(f.read())

                try:
                    for c in board_runner_yaml['board_flash_priority']:
                        if c in boards:
                            board_priority.append(len(board_priority))
                            board_priority[len(board_priority)-1] = c

                except KeyError:
                    pass

            flash_order_present = True

    for board in boards:
        if board not in board_priority:
            board_priority.append(len(board_priority))
            board_priority[len(board_priority)-1] = board

    board_priority = " ".join(board_priority)

    print(board_priority)

def main():
    args = parse_args()
    print(args.boards)
    print(args.board_dirs)
    do_run_common(args.boards, args.board_dirs)

if __name__ == "__main__":
    main()


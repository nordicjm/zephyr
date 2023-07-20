# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
from __future__ import annotations

import logging
import time
import re
import shlex

from subprocess import check_output
from pathlib import Path
from twister_harness import Device


logger = logging.getLogger(__name__)


def wait_for_message(dut: Device, message, timeout=60):
    time_started = time.time()
    for line in dut.iter_stdout:
        if line:
            logger.debug("#: " + line)
        if message in line:
            return True
        if time.time() > time_started + timeout:
            return False


def west_sign_with_imgtool(
        build_dir,
        output_bin=None,
        key_file=None,
        version=None,
        timeout=10
):
    """Wrapper method for `west sign -t imgtool` comamnd"""
    command = [
        'west', 'sign',
        '-t', 'imgtool',
        '--no-hex',
        '--build-dir', str(build_dir)
    ]
    if output_bin:
        command.extend(['--sbin', str(output_bin)])

    command_extra_args = []
    if key_file:
        command_extra_args.extend(['--key', str(key_file)])
    if version:
        command_extra_args.extend(['--version', version])

    if command_extra_args:
        command.append('--')
        command.extend(command_extra_args)

    logger.info(f"CMD: {shlex.join(command)}")
    output = check_output(command, text=True, timeout=timeout)
    logger.debug('OUT: %s' % output)


def find_in_config(config_file: Path | str, config_key: str) -> str:
    re_key = re.compile(rf'{config_key}=(.+)')
    with open(config_file) as f:
        lines = f.readlines()
    for line in lines:
        if m := re_key.match(line):
            logger.debug('Found matching key: %s' % line)
            return m.group(1)
    return ''

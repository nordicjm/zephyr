# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
from __future__ import annotations

import logging
import re

from subprocess import check_output
from pathlib import Path


logger = logging.getLogger(__name__)


class McuMgrException(Exception):
    """General mcumgr exception."""


class McuMgr:
    """Sample wrapper for mcumgr command-line tool"""
    mcumgr_exec = 'mcumgr'

    def __init__(self, connection_options: str):
        self.conn_opts = connection_options

    @classmethod
    def create_for_serial(cls, serial_port: str) -> McuMgr:
        return cls(connection_options=f'--conntype serial --connstring={serial_port}')

    def run_command(self, cmd: str, log_output=False) -> str:
        command = f'{self.mcumgr_exec} {self.conn_opts} {cmd}'
        logger.info(f"CMD: {command}")
        return check_output(command, shell=True, text=True)

    def reset_device(self):
        self.run_command('reset')

    def image_upload(self, image: Path | str):
        self.run_command(f'-t 60 image upload {image}')
        logger.info('Image successfully uploaded')

    def get_image_list(self):
        return self.run_command('image list')

    def get_hash_to_test(self) -> str:
        image_list = self.get_image_list()
        hashes = re.findall(r"hash:\s(.*)\n", image_list)
        if len(hashes) != 2:
            logger.info(image_list)
            raise McuMgrException('Please check image list returned by mcumgr')
        return hashes[1]

    def image_test(self, hash: str | None = None):
        if not hash:
            hash = self.get_hash_to_test()
        self.run_command(f'image test {hash}')

    def image_confirm(self, hash: str):
        self.run_command(f'image confirm {hash}')


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG, format='%(levelname)-8s:  %(message)s')
    mcumgr = McuMgr.create_for_serial('/dev/ttyACM0')
    print(mcumgr.get_image_list())

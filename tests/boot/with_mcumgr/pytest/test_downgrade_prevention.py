# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
from __future__ import annotations

import time
import logging
import pytest  # noqa # pylint: disable=unused-import

from pathlib import Path
from twister_harness.device.device_abstract import DeviceAbstract
from mcumgr import McuMgr
from test_upgrade import wait_for_message


logger = logging.getLogger(__name__)


def test_downgrade_prevention(dut: DeviceAbstract, mcumgr: McuMgr, builddir):
    """
    Verify that the application is not downgraded
    1) Device flashed with MCUboot and an application that contains SMP server.
       Version of MCUboot is 1.1.1
    2) Prepare an update of an application containing the SMP server, where
       MCUboot version is 0.0.0 (lower than version of the original app)
    3) Upload the application update to slot 1 using mcumgr
    4) Flag the application update in slot 1 as 'pending' by using mcumgr 'test'
    5) Restart the device, verify that downgrade prevention mechanism
       blocked the image swap
    6) Verify that the original application is booted (version 1.1.1)
    """
    assert wait_for_message(dut, "Launching primary slot application")

    dut.write(b'mcuboot\n')
    assert wait_for_message(dut, "version: 1.1.1")

    logger.info('Find second image to upload with mcumgr')
    second_image = (Path(builddir) / 'smp_svr' / 'zephyr' / 'zephyr.signed.bin').resolve()
    assert second_image.is_file()

    logger.info('Upload second image with mcumgr')
    mcumgr.image_upload(second_image)

    logger.info('Test uploaded APP image')
    second_hash = mcumgr.get_hash_to_test()
    mcumgr.image_test(second_hash)
    mcumgr.reset_device()
    assert wait_for_message(dut, "Image in slot 1 erased due to downgrade prevention")
    assert wait_for_message(dut, "Jumping to the first image slot")
    time.sleep(1)

    dut.write(b'mcuboot\n')
    assert wait_for_message(dut, "version: 1.1.1")

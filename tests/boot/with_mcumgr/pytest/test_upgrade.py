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

logger = logging.getLogger(__name__)


def wait_for_message(dut: DeviceAbstract, message, timeout=60):
    time_started = time.time()
    for line in dut.iter_stdout:
        if line:
            logger.debug("#: " + line)
        if message in line:
            return True
        if time.time() > time_started + timeout:
            return False


def test_upgrade_valid(dut: DeviceAbstract, mcumgr: McuMgr, builddir):
    """
    Verify that the application can be updated
    1) Device flashed with MCUboot and an application that contains SMP server
    2) Prepare an update of an application containing the SMP server
    3) Upload the application update to slot 1 using mcumgr
    4) Flag the application update in slot 1 as 'pending' by using mcumgr 'test'
    5) Restart the device, verify that swapping process is initiated
    6) Verify that the updated application is booted
    7) Confirm the image using mcumgr
    8) Restart the device, and verify that the new application is still booted
    """
    assert wait_for_message(dut, "Launching primary slot application")

    logger.info('Find second image to upload with mcumgr')
    second_image = (Path(builddir) / 'smp_svr' / 'zephyr' / 'zephyr.signed.bin').resolve()
    assert second_image.is_file()

    logger.info('Upload second image with mcumgr')
    mcumgr.image_upload(second_image)

    logger.info('Test uploaded APP image')
    second_hash = mcumgr.get_hash_to_test()
    mcumgr.image_test(second_hash)
    mcumgr.reset_device()
    assert wait_for_message(dut, "Swap type: test")
    assert wait_for_message(dut, "Jumping to the first image slot")
    time.sleep(1)

    logger.info('Verify the SMP Server app is booted')
    dut.write(b'stats list\n')
    assert wait_for_message(dut, "smp_svr_stats")

    logger.info('Confirm the image')
    mcumgr.image_confirm(second_hash)
    mcumgr.reset_device()
    assert wait_for_message(dut, "Swap type: none")
    assert wait_for_message(dut, "Jumping to the first image slot")
    time.sleep(1)

    logger.info('Verify that the SMP Server app  is still booted')
    dut.write(b'stats list\n')
    assert wait_for_message(dut, "smp_svr_stats")


def test_upgrade_revert_and_confirm(dut: DeviceAbstract, mcumgr: McuMgr, builddir):
    """
    Verify that MCUboot will roll back an image that is not confirmed
    1) Device flashed with MCUboot and an application that contains SMP server
    2) Prepare an update of an application containing the SMP server
    3) Upload the application update to slot 1 using mcumgr
    4) Flag the application update in slot 1 as 'pending' by using mcumgr 'test'
    5) Restart the device, verify that swapping process is initiated
    6) Verify that the updated application is booted
    7) Reset the device without confirming the image
    8) Verify that MCUboot reverts update

    Verify that application can be confirmed after rollback
    9) Repeat steps 4-6
    10) Confirm the image using mcumgr
    11) Restart the device, and verify that the new application is still booted
    """
    assert wait_for_message(dut, "Launching primary slot application")

    logger.info('Find second image to upload with mcumgr')
    second_image = (Path(builddir) / 'smp_svr' / 'zephyr' / 'zephyr.signed.bin').resolve()
    assert second_image.is_file()

    logger.info('Upload second image with mcumgr')
    mcumgr.image_upload(second_image)

    logger.info('Test uploaded APP image')
    second_hash = mcumgr.get_hash_to_test()
    mcumgr.image_test(second_hash)
    mcumgr.reset_device()
    assert wait_for_message(dut, "Swap type: test")
    assert wait_for_message(dut, "Jumping to the first image slot")
    time.sleep(1)

    logger.info('Verify the SMP Server app is booted')
    dut.write(b'stats list\n')
    assert wait_for_message(dut, "smp_svr_stats")

    logger.info('Revert images')
    mcumgr.reset_device()
    assert wait_for_message(dut, "Swap type: revert")
    assert wait_for_message(dut, "Launching primary slot application")
    time.sleep(1)

    logger.info('Test and confirm APP image')
    mcumgr.image_test(second_hash)
    mcumgr.reset_device()
    assert wait_for_message(dut, "Swap type: test")
    assert wait_for_message(dut, "Jumping to the first image slot")
    time.sleep(1)

    mcumgr.image_confirm(second_hash)
    mcumgr.reset_device()
    assert wait_for_message(dut, "Swap type: none")
    assert wait_for_message(dut, "Jumping to the first image slot")
    time.sleep(1)

    dut.write(b'stats list\n')
    assert wait_for_message(dut, "smp_svr_stats")

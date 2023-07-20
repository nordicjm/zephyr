# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
from __future__ import annotations

import pytest
import logging

from pathlib import Path
from twister_harness import Device, MCUmgr
from conftest import (
    wait_for_message,
    west_sign_with_imgtool,
    find_in_config,
)


logger = logging.getLogger(__name__)


@pytest.mark.parametrize(
    'key_file',
    [
        '',
        'root-ec-p256.pem',
        'root-rsa-2048.pem'
    ],
    ids=[
        'no_key',
        'invalid',
        'valid'
    ])
def test_upgrade_signature(dut: Device, mcumgr: MCUmgr, key_file):
    """
    Verify that the application can be updated only when app is signed with valid key
    1) Device flashed with MCUboot and an application that contains SMP server
    2) Prepare an update of an application containing the SMP server that has
       been signed:
       a) without any key
       b) with a different key than MCUboot was compiled with
       c) with the same key as MCUboot was compiled with
    3) Upload the application update to slot 1 using mcumgr
    4) Flag the application update in slot 1 as 'pending' by using mcumgr 'test'
    5) Restart the device, verify that:
      - swap is not started with reason invalid signature and the original
      application is booted when 2a (no key) and 2b (invalid key)
      - the updated application is booted when 2c (valid key)
    """
    assert wait_for_message(dut, "Launching primary slot application")

    smp_svr_dir = Path(dut.device_config.build_dir) / 'smp_svr'
    image_to_test = Path(dut.device_config.build_dir) / 'smp_svr.tested.bin'
    new_key_file = ''
    valid_key = False
    if key_file:
        origin_key_file = find_in_config(
            Path(dut.device_config.build_dir) / 'mcuboot' / 'zephyr' / '.config',
            'CONFIG_BOOT_SIGNATURE_KEY_FILE'
        ).strip('"\'')
        new_key_file = Path(origin_key_file).parent / key_file
        assert new_key_file.is_file()
        if new_key_file.samefile(origin_key_file):
            valid_key = True

    logger.info('Sign second image')
    west_sign_with_imgtool(
        build_dir=smp_svr_dir,
        output_bin=image_to_test,
        key_file=new_key_file
    )
    assert image_to_test.is_file()

    logger.info('Upload second image with mcumgr')
    mcumgr.image_upload(image_to_test)

    logger.info('Test uploaded APP image')
    second_hash = mcumgr.get_hash_to_test()
    mcumgr.image_test(second_hash)
    mcumgr.reset_device()
    assert wait_for_message(dut, "Swap type: test")
    if valid_key:
        assert wait_for_message(dut, "Starting swap using move algorithm")
        assert wait_for_message(dut, "Jumping to the first image slot")
    else:
        assert wait_for_message(dut, "Image in the secondary slot is not valid")
        assert wait_for_message(dut, "Jumping to the first image slot")
        assert wait_for_message(dut, "Launching primary slot application")

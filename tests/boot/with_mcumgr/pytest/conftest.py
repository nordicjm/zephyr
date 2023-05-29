# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
from __future__ import annotations

import pytest

from mcumgr import McuMgr
from twister_harness.device.device_abstract import DeviceAbstract


@pytest.fixture()
def builddir(request):
    return request.config.getoption('--build-dir')


@pytest.fixture()
def mcumgr(dut: DeviceAbstract):
    yield McuMgr.create_for_serial(dut.device_config.serial)

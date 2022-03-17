/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2020 Prevas A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <stats/stats.h>
#include <usb/usb_device.h>

#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
#include <device.h>
#include <fs/fs.h>
#include "fs_mgmt/fs_mgmt.h"
#include <fs/littlefs.h>
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
#include "stat_mgmt/stat_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
#include "shell_mgmt/shell_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
#include "fs_mgmt/fs_mgmt.h"
#endif

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(smp_sample);

#include "common.h"
#include <drivers/sensor.h>

K_THREAD_STACK_DEFINE(sensor_thread_stack,
                      4096);
K_MSGQ_DEFINE(sensor_queue, 64,
              32, 4);
struct k_thread threadData;

void sensorthread(void *arg1, void *arg2, void *arg3)
{
        const struct device *dev = device_get_binding(DT_LABEL(DT_INST(0, bosch_bme680)));
        struct sensor_value temp, press, humidity, gas_res;

        LOG_WRN("Device %p name is %s\n", dev, dev->name);
                k_sleep(K_MSEC(137));

        while (1) {
                k_sleep(K_MSEC(1123));

                sensor_sample_fetch(dev);
                sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
                sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);
                sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);
                sensor_channel_get(dev, SENSOR_CHAN_GAS_RES, &gas_res);

                LOG_WRN("T: %d.%06d; P: %d.%06d; H: %d.%06d; G: %d.%06d\n",
                                temp.val1, temp.val2, press.val1, press.val2,
                                humidity.val1, humidity.val2, gas_res.val1,
                                gas_res.val2);
        }
}


/* Define an example stats group; approximates seconds since boot. */
STATS_SECT_START(smp_svr_stats)
STATS_SECT_ENTRY(ticks)
STATS_SECT_END;

/* Assign a name to the `ticks` stat. */
STATS_NAME_START(smp_svr_stats)
STATS_NAME(smp_svr_stats, ticks)
STATS_NAME_END(smp_svr_stats);

/* Define an instance of the stats group. */
STATS_SECT_DECL(smp_svr_stats) smp_svr_stats;

#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(cstorage);
static struct fs_mount_t littlefs_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &cstorage,
	.storage_dev = (void *)FLASH_AREA_ID(storage),
	.mnt_point = "/lfs1"
};
#endif

void main(void)
{
	int rc = STATS_INIT_AND_REG(smp_svr_stats, STATS_SIZE_32,
				    "smp_svr_stats");

	if (rc < 0) {
		LOG_ERR("Error initializing stats system [%d]", rc);
	}

	/* Register the built-in mcumgr command handlers. */
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
	rc = fs_mount(&littlefs_mnt);
	if (rc < 0) {
		LOG_ERR("Error mounting littlefs [%d]", rc);
	}

	fs_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
	os_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
	img_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
	stat_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
	shell_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
	fs_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_SMP_BT
	start_smp_bluetooth();
#endif
#ifdef CONFIG_MCUMGR_SMP_UDP
	start_smp_udp();
#endif

	if (IS_ENABLED(CONFIG_USB_DEVICE_STACK)) {
		rc = usb_enable(NULL);
		if (rc) {
			LOG_ERR("Failed to enable USB");
			return;
		}
	}
	/* using __TIME__ ensure that a new binary will be built on every
	 * compile which is convient when testing firmware upgrade.
	 */
	LOG_INF("build time: " __DATE__ " " __TIME__);


                k_thread_create(&threadData,
                                sensor_thread_stack,
                                K_THREAD_STACK_SIZEOF(sensor_thread_stack),
                                sensorthread, NULL, NULL, NULL,
                                2, 0, K_NO_WAIT);


	/* The system work queue handles all incoming mcumgr requests.  Let the
	 * main thread idle while the mcumgr server runs.
	 */
	while (1) {
		k_sleep(K_MSEC(630));
LOG_ERR("ya!");
LOG_ERR("here is a");
		k_sleep(K_MSEC(510));
LOG_ERR("message");
LOG_ERR("output");
		STATS_INC(smp_svr_stats, ticks);
	}
}

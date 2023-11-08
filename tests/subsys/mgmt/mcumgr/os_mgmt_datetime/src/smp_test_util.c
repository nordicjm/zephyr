/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "smp_test_util.h"
#include <zephyr/mgmt/mcumgr/grp/os_mgmt/os_mgmt.h>
#include <zephyr/net/buf.h>
#include <zephyr/sys/byteorder.h>
#include <zcbor_encode.h>

/* SMP header function for generating os_mgmt datetime command header with sequence number set
 * to 1
 */
void smp_make_hdr(struct smp_hdr *rsp_hdr, size_t len, bool version2, bool write)
{
	*rsp_hdr = (struct smp_hdr) {
		.nh_len = sys_cpu_to_be16(len),
		.nh_flags = 0,
		.nh_version = (version2 == true ? SMP_MCUMGR_VERSION_2 : SMP_MCUMGR_VERSION_1),
		.nh_op = (write == true ? MGMT_OP_WRITE : MGMT_OP_READ),
		.nh_group = sys_cpu_to_be16(MGMT_GROUP_ID_OS),
		.nh_seq = 1,
		.nh_id = OS_MGMT_ID_DATETIME_STR,
	};
}

/* Function for creating an os_mgmt datetime get command */
bool create_mcumgr_datetime_get_packet(zcbor_state_t *zse, bool version2, uint8_t *buffer,
				       uint8_t *output_buffer, uint16_t *buffer_size)
{
	bool ok;

	ok = zcbor_map_start_encode(zse, 2)	&&
	     zcbor_map_end_encode(zse, 2);

	*buffer_size = (zse->payload_mut - buffer);
	smp_make_hdr((struct smp_hdr *)output_buffer, *buffer_size, version2, false);
	memcpy(&output_buffer[sizeof(struct smp_hdr)], buffer, *buffer_size);
	*buffer_size += sizeof(struct smp_hdr);

	return ok;
}

/* Function for creating an os_mgmt datetime set command */
bool create_mcumgr_datetime_set_packet(zcbor_state_t *zse, bool version2, struct rtc_time *time,
				       uint8_t *buffer, uint8_t *output_buffer,
				       uint16_t *buffer_size)
{
	bool ok;
	char tmp_str[32];

	sprintf(tmp_str, "%4d-%02d-%02dT%02d:%02d:%02d", (uint16_t)time->tm_year,
		(uint8_t)time->tm_mon, (uint8_t)time->tm_mday, (uint8_t)time->tm_hour,
		(uint8_t)time->tm_min, (uint8_t)time->tm_sec);

	ok = zcbor_map_start_encode(zse, 2)		&&
	     zcbor_tstr_put_lit(zse, "datetime")	&&
	     zcbor_tstr_put_term(zse, tmp_str)		&&
	     zcbor_map_end_encode(zse, 2);

	*buffer_size = (zse->payload_mut - buffer);
	smp_make_hdr((struct smp_hdr *)output_buffer, *buffer_size, version2, true);
	memcpy(&output_buffer[sizeof(struct smp_hdr)], buffer, *buffer_size);
	*buffer_size += sizeof(struct smp_hdr);

	return ok;
}

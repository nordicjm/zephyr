/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/net/buf.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/mgmt/mcumgr/transport/smp_dummy.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/grp/os_mgmt/os_mgmt.h>
#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>
#include <mgmt/mcumgr/util/zcbor_bulk.h>
#include <string.h>
#include <smp_internal.h>
#include <zephyr/drivers/rtc.h>
#include "smp_test_util.h"

#define SMP_RESPONSE_WAIT_TIME 3
#define ZCBOR_BUFFER_SIZE 256
#define OUTPUT_BUFFER_SIZE 256
#define ZCBOR_HISTORY_ARRAY_SIZE 4

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_sample);

/*
x get date - expect not set error (v2)
x get date - expect ?? error (v1)
set date with invalid value - expect ?? error, check date not set (v2)
set date with invalid value - expect ?? error, check date not set (v1)
set date with invalid string data - expect ?? error, check date not set (v2)
set date with invalid string data - expect ?? error, check date not set (v1)
set date with invalid string size - expect ?? error, check date not set (v2)
set date with invalid string size - expect ?? error, check date not set (v1)
x set date with valid value - expect success (v2)
x set date with valid value - expect success (v1)
set date with hook - check hook is called and decline works (v2)
set date with hook - check hook is called and decline works (v1)
*/

/* Test sets */
enum {
	OS_MGMT_TEST_SET_MAIN = 0,
#ifdef CONFIG_MCUMGR_GRP_OS_INFO_CUSTOM_HOOKS
	OS_MGMT_TEST_SET_CUSTOM_OS,
	OS_MGMT_TEST_SET_CUSTOM_OS_DISABLED,
	OS_MGMT_TEST_SET_CUSTOM_CMD_DISABLED,
	OS_MGMT_TEST_SET_CUSTOM_CMD,
	OS_MGMT_TEST_SET_CUSTOM_CMD_DISABLED_VERIFY,
#endif

	OS_MGMT_TEST_SET_COUNT
};

static struct net_buf *nb;

struct state {
	uint8_t test_set;
};

static struct state test_state = {
	.test_set = 0,
};

static struct rtc_time invalid_time = {
	.tm_sec = 13,
	.tm_min = 40,
	.tm_hour = 25,
	.tm_mday = 4,
	.tm_mon = 8,
	.tm_year = 2023,
//	.tm_nsec;    /**< Nanoseconds [0, 999999999] (Unknown = 0) */
};

static struct rtc_time valid_time = {
	.tm_sec = 13,
	.tm_min = 40,
	.tm_hour = 4,
	.tm_mday = 4,
	.tm_mon = 8,
	.tm_year = 2023,
//	.tm_nsec;    /**< Nanoseconds [0, 999999999] (Unknown = 0) */
};

static struct rtc_time valid_time2 = {
	.tm_sec = 5,
	.tm_min = 4,
	.tm_hour = 3,
	.tm_mday = 2,
	.tm_mon = 1,
	.tm_year = 2001,
//	.tm_nsec;    /**< Nanoseconds [0, 999999999] (Unknown = 0) */
};

static const char valid_time_string[] = "2023-08-04T04:40:13";
static const char valid_time2_string[] = "2001-01-02T03:04:05";

struct group_error {
	uint16_t group;
	uint16_t rc;
	bool found;
};

static void cleanup_test(void *p);

static bool mcumgr_ret_decode(zcbor_state_t *state, struct group_error *result)
{
	bool ok;
	size_t decoded;
	uint32_t tmp_group;
	uint32_t tmp_rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("group", zcbor_uint32_decode, &tmp_group),
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_uint32_decode, &tmp_rc),
	};

	result->found = false;

	ok = zcbor_map_decode_bulk(state, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;

	if (ok &&
	    zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "group") &&
	    zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "rc")) {
		result->group = (uint16_t)tmp_group;
		result->rc = (uint16_t)tmp_rc;
		result->found = true;
	}

	return ok;
}

#ifdef CONFIG_MCUMGR_GRP_OS_INFO_CUSTOM_HOOKS
static enum mgmt_cb_return os_mgmt_info_custom_os_callback(uint32_t event,
							   enum mgmt_cb_return prev_status,
							   int32_t *rc, uint16_t *group,
							   bool *abort_more, void *data,
							   size_t data_size)
{
	if (event == MGMT_EVT_OP_OS_MGMT_INFO_CHECK) {
		struct os_mgmt_info_check *check_data = (struct os_mgmt_info_check *)data;

		*check_data->custom_os_name = true;
	} else if (event == MGMT_EVT_OP_OS_MGMT_INFO_APPEND) {
		int int_rc;
		struct os_mgmt_info_append *append_data = (struct os_mgmt_info_append *)data;

		if (*append_data->format_bitmask & OS_MGMT_INFO_FORMAT_OPERATING_SYSTEM) {
			int_rc = snprintf(&append_data->output[*append_data->output_length],
					  (append_data->buffer_size - *append_data->output_length),
					  "%s%s", (*append_data->prior_output == true ? " " : ""),
					  CONFIG_CUSTOM_OS_NAME_VALUE);

			if (int_rc < 0 ||
			    int_rc >= (append_data->buffer_size - *append_data->output_length)) {
				*abort_more = true;
				*rc = -1;
				return MGMT_CB_ERROR_RC;
			}

			*append_data->output_length += (uint16_t)int_rc;
			*append_data->prior_output = true;
			*append_data->format_bitmask &= ~OS_MGMT_INFO_FORMAT_OPERATING_SYSTEM;
		}
	}

	return MGMT_CB_OK;
}

static struct mgmt_callback custom_os_check_callback = {
	.callback = os_mgmt_info_custom_os_callback,
	.event_id = MGMT_EVT_OP_OS_MGMT_INFO_CHECK,
};

static struct mgmt_callback custom_os_append_callback = {
	.callback = os_mgmt_info_custom_os_callback,
	.event_id = MGMT_EVT_OP_OS_MGMT_INFO_APPEND,
};

static enum mgmt_cb_return os_mgmt_info_custom_cmd_callback(uint32_t event,
							    enum mgmt_cb_return prev_status,
							    int32_t *rc, uint16_t *group,
							    bool *abort_more, void *data,
							    size_t data_size)
{
	if (event == MGMT_EVT_OP_OS_MGMT_INFO_CHECK) {
		struct os_mgmt_info_check *check_data = (struct os_mgmt_info_check *)data;
		size_t i = 0;

		while (i < check_data->format->len) {
			if (check_data->format->value[i] == query_test_cmd[0]) {
				*check_data->format_bitmask |= QUERY_TEST_CMD_BITMASK;
				++(*check_data->valid_formats);
			}

			++i;
		}
	} else if (event == MGMT_EVT_OP_OS_MGMT_INFO_APPEND) {
		int int_rc;
		struct os_mgmt_info_append *append_data = (struct os_mgmt_info_append *)data;

		if (append_data->all_format_specified ||
		    (*append_data->format_bitmask & QUERY_TEST_CMD_BITMASK)) {
			int_rc = snprintf(&append_data->output[*append_data->output_length],
					  (append_data->buffer_size - *append_data->output_length),
					  "%sMagic Output for Test",
					  (*append_data->prior_output == true ? " " : ""));

			if (int_rc < 0 ||
			    int_rc >= (append_data->buffer_size - *append_data->output_length)) {
				*abort_more = true;
				*rc = -1;
				return MGMT_CB_ERROR_RC;
			}

			*append_data->output_length += (uint16_t)int_rc;
			*append_data->prior_output = true;
			*append_data->format_bitmask &= ~QUERY_TEST_CMD_BITMASK;
		}
	}

	return MGMT_ERR_EOK;
}

static struct mgmt_callback custom_cmd_check_callback = {
	.callback = os_mgmt_info_custom_cmd_callback,
	.event_id = (MGMT_EVT_OP_OS_MGMT_INFO_CHECK | MGMT_EVT_OP_OS_MGMT_INFO_APPEND),
};
#endif

ZTEST(os_mgmt_datetime, test_datetime_get_not_set_v1)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;
	struct group_error group_error;
	int rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("datetime", zcbor_tstr_decode, &output),
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_int32_decode, &rc),
		ZCBOR_MAP_DECODE_KEY_DECODER("err", mcumgr_ret_decode, &group_error),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_mcumgr_datetime_get_packet(zse, false, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "datetime"), "Did not expect to receive datetime element\n");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "rc"), "Did not expect to receive rc element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "err"), "Expected to receive err element\n");
	zassert_equal(rc, MGMT_ERR_ENOENT, "Expected 'rc' to be no entity");
}

ZTEST(os_mgmt_datetime, test_datetime_get_not_set_v2)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;
	struct group_error group_error;
	int rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("datetime", zcbor_tstr_decode, &output),
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_int32_decode, &rc),
		ZCBOR_MAP_DECODE_KEY_DECODER("err", mcumgr_ret_decode, &group_error),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_mcumgr_datetime_get_packet(zse, true, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "datetime"), "Did not expect to receive datetime element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "rc"), "Did not expect to receive rc element\n");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "err"), "Expected to receive err element\n");
	zassert_equal(group_error.group, MGMT_GROUP_ID_OS, "Expected 'err' -> 'group' to be OS");
	zassert_equal(group_error.rc, OS_MGMT_ERR_RTC_NOT_SET, "Expected 'err' -> 'rc' to be RTC not set");
}

//bob
ZTEST(os_mgmt_datetime, test_datetime_set_v1)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;
	struct group_error group_error;
	int rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("datetime", zcbor_tstr_decode, &output),
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_int32_decode, &rc),
		ZCBOR_MAP_DECODE_KEY_DECODER("err", mcumgr_ret_decode, &group_error),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 4, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_mcumgr_datetime_set_packet(zse, false, &valid_time, buffer, buffer_out,
					       &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 0, "Did not expect to receive any decoded zcbor element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "datetime"), "Did not expect to receive datetime element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "rc"), "Did not expect to receive rc element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "err"), "Did not expect to receive err element\n");

	/* Clean up test */
	cleanup_test(NULL);
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	/* Query time and ensure it is set */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_mcumgr_datetime_get_packet(zse, false, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "datetime"), "Expected to receive datetime element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "rc"), "Did not expect to receive rc element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "err"), "Did not expected to receive err element\n");

	/* Check that the date/time is as expected */
	zassert_equal(output.len, strlen(valid_time_string), "Expected received datetime length mismatch\n");
	zassert_mem_equal(output.value, valid_time_string, strlen(valid_time_string), "Expected received datetime value mismatch\n");
}

ZTEST(os_mgmt_datetime, test_datetime_set_v2)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;
	struct group_error group_error;
	int rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("datetime", zcbor_tstr_decode, &output),
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_int32_decode, &rc),
		ZCBOR_MAP_DECODE_KEY_DECODER("err", mcumgr_ret_decode, &group_error),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 4, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_mcumgr_datetime_set_packet(zse, true, &valid_time2, buffer, buffer_out,
					       &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 0, "Did not expect to receive any decoded zcbor element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "datetime"), "Did not expect to receive datetime element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "rc"), "Did not expect to receive rc element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "err"), "Did not expect to receive err element\n");

	/* Clean up test */
	cleanup_test(NULL);
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	/* Query time and ensure it is set */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_mcumgr_datetime_get_packet(zse, false, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "datetime"), "Expected to receive datetime element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "rc"), "Did not expect to receive rc element\n");
	zassert_false(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "err"), "Did not expected to receive err element\n");

	/* Check that the date/time is as expected */
	zassert_equal(output.len, strlen(valid_time2_string), "Expected received datetime length mismatch\n");
	zassert_mem_equal(output.value, valid_time2_string, strlen(valid_time2_string), "Expected received datetime value mismatch\n");
}

#if 0
#ifdef CONFIG_MCUMGR_GRP_OS_INFO_CUSTOM_HOOKS
static void *setup_custom_os(void)
{
	mgmt_callback_register(&custom_os_check_callback);
	mgmt_callback_register(&custom_os_append_callback);
	return NULL;
}

static void destroy_custom_os(void *p)
{
	mgmt_callback_unregister(&custom_os_check_callback);
	mgmt_callback_unregister(&custom_os_append_callback);
}

ZTEST(os_mgmt_info_custom_os, test_info_os_custom)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("output", zcbor_tstr_decode, &output),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);

	ok = create_mcumgr_format_packet(zse, query_os, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);

	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 3, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;

	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");

	zassert_equal((sizeof(response_os_custom) - 1), output.len,
		      "Expected to receive %d bytes but got %d\n",
		      (sizeof(response_os_custom) - 1), output.len);

	zassert_mem_equal(response_os_custom, output.value, output.len,
			  "Expected received data mismatch");
}

ZTEST(os_mgmt_info_custom_os_disabled, test_info_os_custom_disabled)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("output", zcbor_tstr_decode, &output),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);

	ok = create_mcumgr_format_packet(zse, query_os, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);

	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 3, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;

	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");

	zassert_equal((sizeof(response_os) - 1), output.len,
		      "Expected to receive %d bytes but got %d\n",
		      (sizeof(response_os) - 1), output.len);

	zassert_mem_equal(response_os, output.value, output.len,
			  "Expected received data mismatch");
}

static void *setup_custom_cmd(void)
{
	mgmt_callback_register(&custom_cmd_check_callback);

	return NULL;
}

static void destroy_custom_cmd(void *p)
{
	mgmt_callback_unregister(&custom_cmd_check_callback);
}

ZTEST(os_mgmt_info_custom_cmd, test_info_cmd_custom)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("output", zcbor_tstr_decode, &output),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);

	ok = create_mcumgr_format_packet(zse, query_test_cmd, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);

	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 3, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;

	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");

	zassert_equal((sizeof(response_custom_cmd) - 1), output.len,
		      "Expected to receive %d bytes but got %d\n",
		      (sizeof(response_custom_cmd) - 1), output.len);

	zassert_mem_equal(response_custom_cmd, output.value, output.len,
			  "Expected received data mismatch");
}

ZTEST(os_mgmt_info_custom_cmd_disabled, test_info_cmd_custom_disabled)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;
	int32_t rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("output", zcbor_tstr_decode, &output),
	};

	struct zcbor_map_decode_key_val error_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_int32_decode, &rc),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);

	ok = create_mcumgr_format_packet(zse, query_test_cmd, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);

	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 3, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;

	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor element\n");

	zcbor_new_decode_state(zsd, 3, nb->data, nb->len, 1);
	ok = zcbor_map_decode_bulk(zsd, error_decode, ARRAY_SIZE(error_decode), &decoded) == 0;

	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");

	zassert_equal(output.len, 0, "Expected to receive 0 bytes but got %d\n", output.len);

	zassert_equal(rc, MGMT_ERR_EINVAL, "Expected to receive EINVAL error but got %d\n", rc);
}

ZTEST(os_mgmt_info_custom_cmd_disabled_verify, test_info_cmd_custom_disabled)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct zcbor_string output = { 0 };
	size_t decoded = 0;
	int32_t rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("output", zcbor_tstr_decode, &output),
	};

	struct zcbor_map_decode_key_val error_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_int32_decode, &rc),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);

	ok = create_mcumgr_format_packet(zse, query_test_cmd, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful\n");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);

	zassert_true(received, "Expected to receive data but timed out\n");

	/* Retrieve response buffer and ensure validity */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Process received data by removing header */
	(void)net_buf_pull(nb, sizeof(struct smp_hdr));
	zcbor_new_decode_state(zsd, 3, nb->data, nb->len, 1);

	ok = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;

	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor element\n");

	zcbor_new_decode_state(zsd, 3, nb->data, nb->len, 1);
	ok = zcbor_map_decode_bulk(zsd, error_decode, ARRAY_SIZE(error_decode), &decoded) == 0;

	zassert_true(ok, "Expected decode to be successful\n");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element\n");

	zassert_equal(output.len, 0, "Expected to receive 0 bytes but got %d\n", output.len);

	zassert_equal(rc, MGMT_ERR_EINVAL, "Expected to receive EINVAL error but got %d\n", rc);

}
#endif
#endif

static void cleanup_test(void *p)
{
	if (nb != NULL) {
		net_buf_unref(nb);
		nb = NULL;
	}
}

#if 0
void test_main(void)
{
	while (test_state.test_set < OS_MGMT_TEST_SET_COUNT) {
		ztest_run_all(&test_state);
		++test_state.test_set;
	}

	ztest_verify_all_test_suites_ran();
}

static bool main_predicate(const void *state)
{
	return ((struct state *)state)->test_set == OS_MGMT_TEST_SET_MAIN;
}

#ifdef CONFIG_MCUMGR_GRP_OS_INFO_CUSTOM_HOOKS
static bool custom_os_predicate(const void *state)
{
	return ((struct state *)state)->test_set == OS_MGMT_TEST_SET_CUSTOM_OS;
}

static bool custom_os_disabled_predicate(const void *state)
{
	return ((struct state *)state)->test_set == OS_MGMT_TEST_SET_CUSTOM_OS_DISABLED;
}

static bool custom_cmd_disabled_predicate(const void *state)
{
	return ((struct state *)state)->test_set == OS_MGMT_TEST_SET_CUSTOM_CMD_DISABLED;
}

static bool custom_cmd_predicate(const void *state)
{
	return ((struct state *)state)->test_set == OS_MGMT_TEST_SET_CUSTOM_CMD;
}

static bool custom_cmd_disabled_verify_predicate(const void *state)
{
	return ((struct state *)state)->test_set == OS_MGMT_TEST_SET_CUSTOM_CMD_DISABLED_VERIFY;
}
#endif
#endif

/* Main test set */
//ZTEST_SUITE(os_mgmt_datetime, main_predicate, NULL, NULL, cleanup_test, NULL);
ZTEST_SUITE(os_mgmt_datetime, NULL, NULL, NULL, cleanup_test, NULL);

#ifdef CONFIG_MCUMGR_GRP_OS_INFO_CUSTOM_HOOKS
/* Custom OS test set */
ZTEST_SUITE(os_mgmt_info_custom_os, custom_os_predicate, setup_custom_os, NULL, cleanup_test,
	    destroy_custom_os);
ZTEST_SUITE(os_mgmt_info_custom_os_disabled, custom_os_disabled_predicate, NULL, NULL,
	    cleanup_test, NULL);

/* Custom cmd test set */
ZTEST_SUITE(os_mgmt_info_custom_cmd_disabled, custom_cmd_disabled_predicate, NULL, NULL,
	    cleanup_test, NULL);
ZTEST_SUITE(os_mgmt_info_custom_cmd, custom_cmd_predicate, setup_custom_cmd, NULL, cleanup_test,
	    destroy_custom_cmd);
ZTEST_SUITE(os_mgmt_info_custom_cmd_disabled_verify, custom_cmd_disabled_verify_predicate, NULL,
	    NULL, cleanup_test, NULL);
#endif

/*
 * Copyright (c) 2023-2026 Nordic Semiconductor ASA
 * Copyright (c) 2026, Jamie McCrae
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/net_buf.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/mgmt/mcumgr/transport/smp_dummy.h>
#include <zephyr/mgmt/mcumgr/transport/smp_raw_dummy.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/grp/os_mgmt/os_mgmt.h>
#include <zephyr/mgmt/mcumgr/grp/transport_mgmt/transport_mgmt.h>
#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>
#include <mgmt/mcumgr/util/zcbor_bulk.h>
#include <string.h>
#include <zephyr/sys/byteorder.h>
#include <limits.h>
#include "smp_test_util.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_sample);

#define SMP_RESPONSE_WAIT_TIME 3
#define ZCBOR_BUFFER_SIZE 128
#define OUTPUT_BUFFER_SIZE 384
#define ZCBOR_HISTORY_ARRAY_SIZE 8
#define TEST_STRING_RAW "rAW datA TEsT"
#define TEST_STRING "noRMAL d4t4 7est"

struct group_error {
	uint16_t group;
	uint16_t rc;
	bool found;
};

static struct net_buf *nb;

static void cleanup_test(void *p)
{
	if (nb != NULL) {
		net_buf_reset(nb);
		net_buf_unref(nb);
		nb = NULL;
	}
}

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

ZTEST(transport_mgmt, test_connect_invalid)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct smp_hdr *header;
	size_t decoded = 0;
	struct group_error group_error;
	int rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("err", mcumgr_ret_decode, &group_error),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	/* Test 1: Send overly high transport to bridge to (47) */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_connect_packet(zse, buffer, buffer_out, &buffer_size, 47);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_CONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
	zassert_equal(group_error.group, MGMT_GROUP_ID_TRANSPORT,
		      "Expected 'err' -> 'group' to be transport");
	zassert_equal(group_error.rc, TRANSPORT_MGMT_ERR_INVALID_TRANSPORT,
		      "Expected 'err' -> 'rc' to be invalid transport");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	cleanup_test(NULL);

	/* Test 2: Send low transport to bridge to (2) */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_connect_packet(zse, buffer, buffer_out, &buffer_size, 2);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_CONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
	zassert_equal(group_error.group, MGMT_GROUP_ID_TRANSPORT,
		      "Expected 'err' -> 'group' to be transport");
	zassert_equal(group_error.rc, TRANSPORT_MGMT_ERR_INVALID_TRANSPORT,
		      "Expected 'err' -> 'rc' to be invalid transport");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	cleanup_test(NULL);

	/* Test 3: Send same transport ID as is being used (0) */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_connect_packet(zse, buffer, buffer_out, &buffer_size, 0);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_CONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
	zassert_equal(group_error.group, MGMT_GROUP_ID_TRANSPORT,
		      "Expected 'err' -> 'group' to be transport");
	zassert_equal(group_error.rc, TRANSPORT_MGMT_ERR_SAME_BRIDGE_DEVICE_DISALLOWED,
		      "Expected 'err' -> 'rc' to be same bridge device disallowed");
}

ZTEST(transport_mgmt, test_disconnect_invalid)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct smp_hdr *header;
	size_t decoded = 0;
	struct group_error group_error;
	int rc;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("err", mcumgr_ret_decode, &group_error),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	/* Test 1: Disconnect all bridges with no bridge active */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_disconnect_packet(zse, buffer, buffer_out, &buffer_size, 0, true);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_DISCONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
	zassert_equal(group_error.group, MGMT_GROUP_ID_TRANSPORT,
		      "Expected 'err' -> 'group' to be transport");
	zassert_equal(group_error.rc, TRANSPORT_MGMT_ERR_NOT_BRIDGED,
		      "Expected 'err' -> 'rc' to be not bridged");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	cleanup_test(NULL);

	/* Test 2: Disconnect one bridge with no bridge active */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_disconnect_packet(zse, buffer, buffer_out, &buffer_size, 1, false);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_DISCONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
	zassert_equal(group_error.group, MGMT_GROUP_ID_TRANSPORT,
		      "Expected 'err' -> 'group' to be transport");
	zassert_equal(group_error.rc, TRANSPORT_MGMT_ERR_NOT_BRIDGED,
		      "Expected 'err' -> 'rc' to be not bridged");
}

/*
 * Tests commands work and get responses without bridge, then tests bridging and un-bridging works
 * along with data tranfer between the transport
 */
ZTEST(transport_mgmt, test_connection)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct smp_hdr *header;
	size_t decoded = 0;
	struct zcbor_string echo_send_data = { 0 };
	struct zcbor_string echo_receive_data = { 0 };
	struct group_error group_error;
	uint32_t rc = 0;

	struct zcbor_map_decode_key_val output_decode[] = {
                ZCBOR_MAP_DECODE_KEY_DECODER("d", zcbor_tstr_decode, &echo_send_data),
                ZCBOR_MAP_DECODE_KEY_DECODER("r", zcbor_tstr_decode, &echo_receive_data),
		ZCBOR_MAP_DECODE_KEY_DECODER("err", mcumgr_ret_decode, &group_error),
		ZCBOR_MAP_DECODE_KEY_DECODER("rc", zcbor_uint32_decode, &rc),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

        /* Test 1: Check dummy transport does echo and gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_dummy_enable();
        smp_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_dummy_tx_pkt(buffer_out, buffer_size);
        smp_dummy_add_data();

        /* For a short duration to see if response has been received */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_dummy_get_outgoing();
        smp_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_receive_data.len, strlen(TEST_STRING), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_receive_data.value, TEST_STRING, echo_receive_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 2: Check raw dummy transport does echo and gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING_RAW);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
        smp_raw_dummy_add_data();

        /* For a short duration to see if response has been received */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_raw_dummy_get_outgoing();
        smp_raw_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful %d", rc);
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_receive_data.len, strlen(TEST_STRING_RAW), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_receive_data.value, TEST_STRING_RAW, echo_receive_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

	/* Test 3: Bridge to other raw dummy transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_connect_packet(zse, buffer, buffer_out, &buffer_size, 1);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_CONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 4: Check dummy transport does echo and raw dummy gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_dummy_enable();
        smp_dummy_clear_state();
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_dummy_tx_pkt(buffer_out, buffer_size);
        smp_dummy_add_data();

        /* Ensure primary transport gets no response */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_false(received, "Expected to not receive data but received it");

        /* For a short duration to see if response has been received */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_raw_dummy_get_outgoing();
        smp_dummy_disable();
        smp_raw_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_send_data.len, strlen(TEST_STRING), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_send_data.value, TEST_STRING, echo_send_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 5: Check raw dummy transport does echo and dummy gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_response_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING_RAW);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();
        smp_dummy_enable();
        smp_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
        smp_raw_dummy_add_data();

        /* Ensure primary transport gets no response */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_false(received, "Expected to not receive data but received it");

        /* For a short duration to see if response has been received */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_dummy_get_outgoing();
        smp_raw_dummy_disable();
        smp_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_send_data.len, strlen(TEST_STRING_RAW), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_send_data.value, TEST_STRING_RAW, echo_send_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

	/* Test 6: Disconnect one bridge with bridge active */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_disconnect_packet(zse, buffer, buffer_out, &buffer_size, 1, false);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_DISCONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 7: Check dummy transport does echo and gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_dummy_enable();
        smp_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_dummy_tx_pkt(buffer_out, buffer_size);
        smp_dummy_add_data();

        /* For a short duration to see if response has been received */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_dummy_get_outgoing();
        smp_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_receive_data.len, strlen(TEST_STRING), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_receive_data.value, TEST_STRING, echo_receive_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 8: Check raw dummy transport does echo and gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING_RAW);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
        smp_raw_dummy_add_data();

        /* For a short duration to see if response has been received */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_raw_dummy_get_outgoing();
        smp_raw_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful %d", rc);
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_receive_data.len, strlen(TEST_STRING_RAW), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_receive_data.value, TEST_STRING_RAW, echo_receive_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

	/* Test 9: Bridge to other dummy transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_connect_packet(zse, buffer, buffer_out, &buffer_size, 0);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_CONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 10: Check dummy transport does echo and raw dummy gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_response_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_dummy_enable();
        smp_dummy_clear_state();
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_dummy_tx_pkt(buffer_out, buffer_size);
        smp_dummy_add_data();

        /* Ensure primary transport gets no response */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_false(received, "Expected to not receive data but received it");

        /* For a short duration to see if response has been received */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_raw_dummy_get_outgoing();
        smp_dummy_disable();
        smp_raw_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_send_data.len, strlen(TEST_STRING), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_send_data.value, TEST_STRING, echo_send_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 11: Check raw dummy transport does echo and dummy gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING_RAW);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();
        smp_dummy_enable();
        smp_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
        smp_raw_dummy_add_data();

        /* Ensure primary transport gets no response */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_false(received, "Expected to not receive data but received it");

        /* For a short duration to see if response has been received */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_dummy_get_outgoing();
        smp_raw_dummy_disable();
        smp_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_send_data.len, strlen(TEST_STRING_RAW), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_send_data.value, TEST_STRING_RAW, echo_send_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

	/* Test 12: Disconnect all bridges with one active bridge */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_disconnect_packet(zse, buffer, buffer_out, &buffer_size, 0, true);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_DISCONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 13: Check dummy transport does echo and gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_dummy_enable();
        smp_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_dummy_tx_pkt(buffer_out, buffer_size);
        smp_dummy_add_data();

        /* For a short duration to see if response has been received */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_dummy_get_outgoing();
        smp_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_receive_data.len, strlen(TEST_STRING), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_receive_data.value, TEST_STRING, echo_receive_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	echo_send_data.len = 0;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 14: Check raw dummy transport does echo and gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING_RAW);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
        smp_raw_dummy_add_data();

        /* For a short duration to see if response has been received */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_raw_dummy_get_outgoing();
        smp_raw_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful %d", rc);
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_receive_data.len, strlen(TEST_STRING_RAW), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_receive_data.value, TEST_STRING_RAW, echo_receive_data.len, "os mgmt echo response mismatch");
}

/*
 * Bridge transports, test wrong type and ensure response is received on _same_ port not on
 * bridged one
 */
ZTEST(transport_mgmt, test_connection_reverse_incoming)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct smp_hdr *header;
	size_t decoded = 0;
	struct zcbor_string echo_receive_data = { 0 };
	uint32_t rc = 0;

	struct zcbor_map_decode_key_val output_decode[] = {
                ZCBOR_MAP_DECODE_KEY_DECODER("r", zcbor_tstr_decode, &echo_receive_data),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	/* Test 1: Bridge to other raw dummy transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_connect_packet(zse, buffer, buffer_out, &buffer_size, 1);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_CONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 2: Check dummy transport gets no echo response response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_response_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_dummy_enable();
        smp_dummy_clear_state();
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_dummy_tx_pkt(buffer_out, buffer_size);
        smp_dummy_add_data();

        /* Ensure neither transport gets a response */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_false(received, "Expected to not receive data but received it");
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_false(received, "Expected to not receive data but received it");
        smp_dummy_disable();
        smp_raw_dummy_disable();

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

        /* Test 3: Check raw dummy transport does echo gets response */
        zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
        ok = create_os_mgmt_echo_packet(zse, buffer, buffer_out, &buffer_size, TEST_STRING_RAW);
        zassert_true(ok, "Expected packet creation to be successful");

        /* Enable dummy SMP backend and ready for usage */
        smp_raw_dummy_enable();
        smp_raw_dummy_clear_state();
        smp_dummy_enable();
        smp_dummy_clear_state();

        /* Send query command to dummy SMP backend */
        (void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
        smp_raw_dummy_add_data();

        /* Ensure secondary transport gets no response */
        received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_false(received, "Expected to not receive data but received it");

        /* For a short duration to see if response has been received */
        received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
        zassert_true(received, "Expected to receive data but timed out");

        /* Retrieve response buffer */
        nb = smp_raw_dummy_get_outgoing();
        smp_raw_dummy_disable();
        smp_dummy_disable();

        /* Check response is as expected */
        header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

        zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
        zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
        zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_OS),
                      "SMP header group mismatch");
        zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
        zassert_equal(header->nh_id, OS_MGMT_ID_ECHO, "SMP header command ID mismatch");
        zassert_equal(header->nh_version, 1, "SMP header version mismatch");

        /* Get the response value to compare */
        zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
        rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
        zassert_equal(rc, 1, "Expected decode to be successful");
        zassert_equal(decoded, 1, "Expected to receive 1 decoded zcbor element");
        zassert_equal(echo_receive_data.len, strlen(TEST_STRING_RAW), "os mgmt echo response length mismatch");
        zassert_mem_equal(echo_receive_data.value, TEST_STRING_RAW, echo_receive_data.len, "os mgmt echo response mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	echo_receive_data.len = 0;
	cleanup_test(NULL);

	/* Test 4: Disconnect all bridges */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_disconnect_packet(zse, buffer, buffer_out, &buffer_size, 0, true);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_DISCONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");
}

ZTEST(transport_mgmt, test_status)
{
	uint8_t buffer[ZCBOR_BUFFER_SIZE];
	uint8_t buffer_out[OUTPUT_BUFFER_SIZE];
	bool ok;
	uint16_t buffer_size;
	zcbor_state_t zse[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	zcbor_state_t zsd[ZCBOR_HISTORY_ARRAY_SIZE] = { 0 };
	bool received;
	struct smp_hdr *header;
	size_t decoded = 0;
	int rc;
	uint32_t supported = UINT32_MAX;
	uint32_t active = UINT32_MAX;
	bool bridged = true;
	uint32_t transport = UINT32_MAX;

	struct zcbor_map_decode_key_val output_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("supported", zcbor_uint32_decode, &supported),
		ZCBOR_MAP_DECODE_KEY_DECODER("active", zcbor_uint32_decode, &active),
		ZCBOR_MAP_DECODE_KEY_DECODER("bridged", zcbor_bool_decode, &bridged),
		ZCBOR_MAP_DECODE_KEY_DECODER("transport", zcbor_uint32_decode, &transport),
	};

	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));

	/* Test 1: Check status with no bridges from primary transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_status_packet(zse, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_STATUS, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 2, "Expected to receive 1 decoded zcbor element");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "supported"), "Expected 'supported' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "active"), "Expected 'active' response");
	zassert_equal(supported, CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES,
		      "Expected 'supported' value mismatch");
	zassert_equal(active, 0, "Expected 'active' value mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	supported = UINT32_MAX;
	active = UINT32_MAX;
	bridged = true;
	transport = UINT32_MAX;
	cleanup_test(NULL);

	/* Test 2: Check status with no bridges from secondary transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_status_packet(zse, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_STATUS, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 2, "Expected to receive 1 decoded zcbor element");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "supported"), "Expected 'supported' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "active"), "Expected 'active' response");
	zassert_equal(supported, CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES,
		      "Expected 'supported' value mismatch");
	zassert_equal(active, 0, "Expected 'active' value mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	supported = UINT32_MAX;
	active = UINT32_MAX;
	bridged = true;
	transport = UINT32_MAX;
	cleanup_test(NULL);

	/* Test 3: Bridge connection */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_connect_packet(zse, buffer, buffer_out, &buffer_size, SMP_SERIAL_TRANSPORT);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_CONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	supported = UINT32_MAX;
	active = UINT32_MAX;
	bridged = false;
	transport = UINT32_MAX;
	cleanup_test(NULL);

	/* Test 4: Check status with one bridge from primary transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_status_packet(zse, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_STATUS, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 2, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 4, "Expected to receive 4 decoded zcbor element");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "supported"), "Expected 'supported' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "active"), "Expected 'active' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "bridged"), "Expected 'bridged' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "transport"), "Expected 'transport' response");
	zassert_equal(supported, CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES,
		      "Expected 'supported' value mismatch");
	zassert_equal(active, 1, "Expected 'active' value mismatch");
	zassert_true(bridged, "Expected 'bridged' value mismatch");
	zassert_equal(transport, SMP_RAW_SERIAL_TRANSPORT, "Expected 'transport' value mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	supported = UINT32_MAX;
	active = UINT32_MAX;
	bridged = false;
	transport = UINT32_MAX;
	cleanup_test(NULL);

	/* Test 5: Check status with one bridge from secondary transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_status_packet(zse, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_STATUS, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 2, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 4, "Expected to receive 4 decoded zcbor element");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "supported"), "Expected 'supported' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "active"), "Expected 'active' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "bridged"), "Expected 'bridged' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "transport"), "Expected 'transport' response");
	zassert_equal(supported, CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES,
		      "Expected 'supported' value mismatch");
	zassert_equal(active, 1, "Expected 'active' value mismatch");
	zassert_true(bridged, "Expected 'bridged' value mismatch");
	zassert_equal(transport, SMP_SERIAL_TRANSPORT, "Expected 'transport' value mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	supported = UINT32_MAX;
	active = UINT32_MAX;
	bridged = false;
	transport = UINT32_MAX;
	cleanup_test(NULL);

	/* Test 6: Disconnect all bridges with one active bridge */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_disconnect_packet(zse, buffer, buffer_out, &buffer_size, 0, true);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_WRITE_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_DISCONNECT, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 0, "Expected to receive 0 decoded zcbor elements");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	supported = UINT32_MAX;
	active = UINT32_MAX;
	bridged = true;
	transport = UINT32_MAX;
	cleanup_test(NULL);

	/* Test 7: Check status with no bridges from primary transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_status_packet(zse, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_dummy_enable();
	smp_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_dummy_tx_pkt(buffer_out, buffer_size);
	smp_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_dummy_get_outgoing();
	smp_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_STATUS, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 2, "Expected to receive 1 decoded zcbor element");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "supported"), "Expected 'supported' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "active"), "Expected 'active' response");
	zassert_equal(supported, CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES,
		      "Expected 'supported' value mismatch");
	zassert_equal(active, 0, "Expected 'active' value mismatch");

	/* Clean up test */
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_out, 0, sizeof(buffer_out));
	buffer_size = 0;
	memset(zse, 0, sizeof(zse));
	memset(zsd, 0, sizeof(zsd));
	output_decode[0].found = false;
	output_decode[1].found = false;
	output_decode[2].found = false;
	output_decode[3].found = false;
	supported = UINT32_MAX;
	active = UINT32_MAX;
	bridged = true;
	transport = UINT32_MAX;
	cleanup_test(NULL);

	/* Test 8: Check status with no bridges from secondary transport */
	zcbor_new_encode_state(zse, 2, buffer, ARRAY_SIZE(buffer), 0);
	ok = create_transport_mgmt_status_packet(zse, buffer, buffer_out, &buffer_size);
	zassert_true(ok, "Expected packet creation to be successful");

	/* Enable dummy SMP backend and ready for usage */
	smp_raw_dummy_enable();
	smp_raw_dummy_clear_state();

	/* Send query command to dummy SMP backend */
	(void)smp_raw_dummy_tx_pkt(buffer_out, buffer_size);
	smp_raw_dummy_add_data();

	/* For a short duration to see if response has been received */
	received = smp_raw_dummy_wait_for_data(SMP_RESPONSE_WAIT_TIME);
	zassert_true(received, "Expected to receive data but timed out");

	/* Retrieve response buffer */
	nb = smp_raw_dummy_get_outgoing();
	smp_raw_dummy_disable();

	/* Check response is as expected */
	header = net_buf_pull_mem(nb, sizeof(struct smp_hdr));

	zassert_equal(header->nh_flags, 0, "SMP header flags mismatch");
	zassert_equal(header->nh_op, MGMT_OP_READ_RSP, "SMP header operation mismatch");
	zassert_equal(header->nh_group, sys_cpu_to_be16(MGMT_GROUP_ID_TRANSPORT),
		      "SMP header group mismatch");
	zassert_equal(header->nh_seq, 1, "SMP header sequence number mismatch");
	zassert_equal(header->nh_id, TRANSPORT_MGMT_ID_STATUS, "SMP header command ID mismatch");
	zassert_equal(header->nh_version, 1, "SMP header version mismatch");

	/* Get the response value to compare */
	zcbor_new_decode_state(zsd, 4, nb->data, nb->len, 1, NULL, 0);
	rc = zcbor_map_decode_bulk(zsd, output_decode, ARRAY_SIZE(output_decode), &decoded) == 0;
	zassert_equal(rc, 1, "Expected decode to be successful");
	zassert_equal(decoded, 2, "Expected to receive 1 decoded zcbor element");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "supported"), "Expected 'supported' response");
	zassert_true(zcbor_map_decode_bulk_key_found(output_decode, ARRAY_SIZE(output_decode), "active"), "Expected 'active' response");
	zassert_equal(supported, CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES,
		      "Expected 'supported' value mismatch");
	zassert_equal(active, 0, "Expected 'active' value mismatch");
}

//test modes
//test config details

#if 0
/* Function for creating a transport_mgmt modes of transport command */
bool create_transport_mgmt_modes_packet(zcbor_state_t *zse, uint8_t *buffer,
                                        uint8_t *output_buffer, uint16_t *buffer_size,
                                        uint8_t transport_id);

/* Function for creating a transport_mgmt config details of transport command */
bool create_transport_mgmt_config_details_packet(zcbor_state_t *zse, uint8_t *buffer,
                                                 uint8_t *output_buffer, uint16_t *buffer_size,
                                                 uint8_t transport_id, uint8_t type);
#endif

ZTEST_SUITE(transport_mgmt, NULL, NULL, NULL, cleanup_test, NULL);

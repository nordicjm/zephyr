/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/mgmt/mcumgr/smp/smp.h>
#include <zephyr/mgmt/mcumgr/mgmt/handlers.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/transport/smp.h>
#include <zephyr/mgmt/mcumgr/grp/transport_mgmt/transport_mgmt.h>
#include <mgmt/mcumgr/util/zcbor_bulk.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>

#define LOG_LEVEL CONFIG_MCUMGR_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(transport_mgmt);

#if defined(CONFIG_MCUMGR_GRP_TRANSPORT_LOCKING)
static K_SEM_DEFINE(mcumgr_transport_sem, 1, 1);

static inline void transport_mgmt_lock(void)
{
	k_sem_take(&mcumgr_transport_sem, K_FOREVER);
}

static inline void transport_mgmt_unlock(void)
{
	k_sem_give(&mcumgr_transport_sem);
}
#else
#define transport_mgmt_lock()
#define transport_mgmt_unlock()
#endif

static struct smp_transport_bridge bridges[CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES];
static bool bridge_active;

bool transport_mgmt_is_bridged(struct smp_transport *transport, bool outgoing)
{
	bool bridged = false;

	transport_mgmt_lock();

	if (bridge_active == true) {
		uint8_t i = 0;

		while (i < CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
			if (bridges[i].status == 1 && ((outgoing == false && bridges[i].incoming_transport == transport) || (outgoing == true && bridges[i].outgoing_transport == transport))) {
				bridged = true;
				break;
			}

			++i;
		}
	}

	transport_mgmt_unlock();

	return bridged;
}

struct smp_transport *transport_mgmt_get_other_transport(struct smp_transport *transport, bool outgoing)
{
	struct smp_transport *other_transport = NULL;

	transport_mgmt_lock();

	if (bridge_active == true) {
		uint8_t i = 0;

		while (i < CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
			if (bridges[i].status == 1) {
				if (outgoing == false && bridges[i].incoming_transport == transport) {
					other_transport = bridges[i].outgoing_transport;
					break;
				} else if (outgoing == true && bridges[i].outgoing_transport == transport) {
					other_transport = bridges[i].incoming_transport;
					break;
				}
			}

			++i;
		}
	}

	transport_mgmt_unlock();

	return other_transport;
}

const struct smp_transport_bridge *transport_mgmt_get_bridge(struct smp_transport *transport, bool outgoing)
{
	const struct smp_transport_bridge *bridge = NULL;

	transport_mgmt_lock();

	if (bridge_active == true) {
		uint8_t i = 0;

		while (i < CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
			if (bridges[i].status == 1 && ((outgoing == false && bridges[i].incoming_transport == transport) || (outgoing == true && bridges[i].outgoing_transport == transport))) {
				bridge = &bridges[i];
				break;
			}

			++i;
		}
	}

	transport_mgmt_unlock();

	return bridge;
}

/**
 * Command handler: transport <>
 */
#if defined(CONFIG_MCUMGR_GRP_TRANSPORT_INFO_FUNCTIONS)
static int transport_mgmt_list(struct smp_streamer *ctxt)
{
//TODO
}

static int transport_mgmt_get_details(struct smp_streamer *ctxt)
{
//TODO
}

static int transport_mgmt_get_config_details(struct smp_streamer *ctxt)
{
//TODO
}
#endif

static int transport_mgmt_connect(struct smp_streamer *ctxt)
{
	int rc;
	zcbor_state_t *zse = ctxt->writer->zs;
	zcbor_state_t *zsd = ctxt->reader->zs;
	bool ok = true;
	size_t decoded;
	uint32_t transport_id = 0;
//        struct smp_transport *smpt;
	size_t backup_element_count_reader = zsd->elem_count;

	struct zcbor_map_decode_key_val settings_save_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("transport", zcbor_uint32_decode, &transport_id),
	};

	if (!zcbor_new_backup(zsd, backup_element_count_reader)) {
		LOG_ERR("Failed to create zcbor backup");
		return MGMT_ERR_ENOMEM;
	}

	ok = zcbor_map_decode_bulk(zsd, settings_save_decode, ARRAY_SIZE(settings_save_decode),
				   &decoded) == 0;

//TODO: allow transport_id to be 0 by default?
	if (!ok || decoded == 0 || !zcbor_map_decode_bulk_key_found(settings_save_decode, ARRAY_SIZE(settings_save_decode), "transport")) {
		return MGMT_ERR_EINVAL;
	}

//zcbor_map_decode_bulk_reset(settings_save_decode, ARRAY_SIZE(settings_save_decode));
	if (!zcbor_process_backup(zsd, (ZCBOR_FLAG_RESTORE | ZCBOR_FLAG_CONSUME),
				  backup_element_count_reader)) {
		LOG_ERR("Failed to restore zcbor reader backup");
		return MGMT_ERR_ENOMEM;
	}

//TODO: check outgoing_transport is not null
	struct smp_transport *outgoing_transport = smp_client_transport_get(transport_id);

	if (outgoing_transport->functions.bridge_connect == NULL || ctxt->smpt->functions.bridge_connect == NULL) {
//TODO: error
		transport_mgmt_unlock();
return MGMT_ERR_EBADSTATE;
	}

	transport_mgmt_lock();

	uint8_t i = 0;

	while (i < CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
		if (bridges[i].status == 0) {
			break;
		}

		++i;
	}

	if (i == CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
//TODO: error
		transport_mgmt_unlock();
return MGMT_ERR_EBADSTATE;
	}

	rc = outgoing_transport->functions.bridge_connect(&bridges[i], true, zsd);

	if (rc != 0) {
//TODO: error
		transport_mgmt_unlock();
return MGMT_ERR_EACCESSDENIED;
	}

	rc = ctxt->smpt->functions.bridge_connect(&bridges[i], false, zsd);

	if (rc != 0) {
//TODO: error
		(void)outgoing_transport->functions.bridge_disconnect(&bridges[i], true);
		transport_mgmt_unlock();
return MGMT_ERR_UNSUPPORTED_TOO_OLD;
	}

	bridges[i].status = 1;
	bridges[i].incoming_transport = ctxt->smpt;
	bridges[i].outgoing_transport = outgoing_transport;
	bridge_active = true;

LOG_ERR("Bridge %p to %p with %d", ctxt->smpt, outgoing_transport, i);

	transport_mgmt_unlock();

//TODO
	return MGMT_RETURN_CHECK(ok);
}

static int transport_mgmt_disconnect(struct smp_streamer *ctxt)
{
	int rc;
	zcbor_state_t *zse = ctxt->writer->zs;
	zcbor_state_t *zsd = ctxt->reader->zs;
	bool ok = true;
	size_t decoded;
	uint32_t transport_id = 0;
//        struct smp_transport *smpt;
	bool disconnect_all;

	struct zcbor_map_decode_key_val settings_save_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("transport", zcbor_uint32_decode, &transport_id),
		ZCBOR_MAP_DECODE_KEY_DECODER("all", zcbor_bool_decode, &disconnect_all),
	};

	ok = zcbor_map_decode_bulk(zsd, settings_save_decode, ARRAY_SIZE(settings_save_decode),
				   &decoded) == 0;

//TODO: allow transport_id to be 0 by default?
	if (!ok || decoded == 0 || (!zcbor_map_decode_bulk_key_found(settings_save_decode, ARRAY_SIZE(settings_save_decode), "transport") && !zcbor_map_decode_bulk_key_found(settings_save_decode, ARRAY_SIZE(settings_save_decode), "all"))) {
		return MGMT_ERR_EINVAL;
	}

	if (zcbor_map_decode_bulk_key_found(settings_save_decode, ARRAY_SIZE(settings_save_decode), "transport") && zcbor_map_decode_bulk_key_found(settings_save_decode, ARRAY_SIZE(settings_save_decode), "all") && disconnect_all == true) {
//cannot disconnect all and just one transport at the same time
		return MGMT_ERR_EINVAL;
	}

	if (bridge_active == false) {
//TODO: error
return MGMT_ERR_EINVAL;
	}

if (disconnect_all == true) {
//TODO: disconnect all
} else {
//TODO: check outgoing_transport is not null
	struct smp_transport *outgoing_transport = smp_client_transport_get(transport_id);

	if (outgoing_transport->functions.bridge_disconnect == NULL || ctxt->smpt->functions.bridge_disconnect == NULL) {
//TODO: error
		transport_mgmt_unlock();
return MGMT_ERR_EBADSTATE;
	}

	transport_mgmt_lock();

	uint8_t i = 0;

	while (i < CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
		if (bridges[i].status == 1 && bridges[i].outgoing_transport == outgoing_transport && bridges[i].incoming_transport == ctxt->smpt) {
			break;
		}

		++i;
	}

	if (i == CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
//TODO: error
		transport_mgmt_unlock();
return MGMT_ERR_EBADSTATE;
	}

	rc = outgoing_transport->functions.bridge_disconnect(&bridges[i], true);

	if (rc != 0) {
//TODO: error
	}

	rc = ctxt->smpt->functions.bridge_disconnect(&bridges[i], false);

	if (rc != 0) {
//TODO: error
	}

	bridges[i].status = 0;
	bridges[i].incoming_transport = NULL;
	bridges[i].outgoing_transport = NULL;

	i = 0;

	while (i < CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
		if (bridges[i].status == 1) {
			break;
		}

		++i;
	}

	if (i == CONFIG_MCUMGR_GRP_TRANSPORT_MAX_BRIDGES) {
		bridge_active = false;
	}
}

	transport_mgmt_unlock();

//TODO
	return MGMT_RETURN_CHECK(ok);
}

static int transport_mgmt_status(struct smp_streamer *ctxt)
{
//TODO
/*
return:
  * number of supported bridges
  * number of active bridges
  * if current transport bridge is active
  * if so, where it is bridged to?
*/
}
//		ok = smp_add_cmd_err(zse, MGMT_GROUP_ID_SETTINGS, (uint16_t)rc);

#ifdef CONFIG_MCUMGR_SMP_SUPPORT_ORIGINAL_PROTOCOL
static int transport_mgmt_translate_error_code(uint16_t ret)
{
	int rc;

	switch (ret) {
#if 0
	case SETTINGS_MGMT_ERR_KEY_TOO_LONG:
		rc = MGMT_ERR_EINVAL;
		break;

	case SETTINGS_MGMT_ERR_KEY_NOT_FOUND:
	case SETTINGS_MGMT_ERR_READ_NOT_SUPPORTED:
		rc = MGMT_ERR_ENOENT;
		break;
#endif

	case TRANSPORT_MGMT_ERR_UNKNOWN:
	default:
		rc = MGMT_ERR_EUNKNOWN;
	}

	return rc;
}
#endif

static const struct mgmt_handler transport_mgmt_handlers[] = {
#if defined(CONFIG_MCUMGR_GRP_TRANSPORT_INFO_FUNCTIONS)
	[TRANSPORT_MGMT_ID_LIST] = {
		.mh_read = transport_mgmt_list,
		.mh_write = NULL,
	},
	[TRANSPORT_MGMT_ID_GET_DETAILS] = {
		.mh_read = transport_mgmt_get_details,
		.mh_write = NULL,
	},
	[TRANSPORT_MGMT_ID_GET_CONFIG_DETAILS] = {
		.mh_read = transport_mgmt_get_config_details,
		.mh_write = NULL,
	},
#endif
	[TRANSPORT_MGMT_ID_CONNECT] = {
		.mh_read = NULL,
		.mh_write = transport_mgmt_connect,
	},
	[TRANSPORT_MGMT_ID_DISCONNECT] = {
		.mh_read = NULL,
		.mh_write = transport_mgmt_disconnect,
	},
	[TRANSPORT_MGMT_ID_STATUS] = {
		.mh_read = transport_mgmt_status,
		.mh_write = NULL,
	},
};

static struct mgmt_group transport_mgmt_group = {
	.mg_handlers = transport_mgmt_handlers,
	.mg_handlers_count = ARRAY_SIZE(transport_mgmt_handlers),
	.mg_group_id = MGMT_GROUP_ID_TRANSPORT,
#ifdef CONFIG_MCUMGR_SMP_SUPPORT_ORIGINAL_PROTOCOL
	.mg_translate_error = transport_mgmt_translate_error_code,
#endif
#ifdef CONFIG_MCUMGR_GRP_ENUM_DETAILS_NAME
	.mg_group_name = "transport mgmt",
#endif
};

static void transport_mgmt_register_group(void)
{
	mgmt_register_group(&transport_mgmt_group);
}

MCUMGR_HANDLER_DEFINE(transport_mgmt, transport_mgmt_register_group);

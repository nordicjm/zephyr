/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/settings/settings.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/mgmt/mcumgr/smp/smp.h>
#include <zephyr/mgmt/mcumgr/mgmt/handlers.h>
#include <zephyr/mgmt/mcumgr/grp/settings_mgmt/settings_mgmt.h>
#include <mgmt/mcumgr/util/zcbor_bulk.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>

#if defined(CONFIG_MCUMGR_MGMT_NOTIFICATION_HOOKS)
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#endif

#define LOG_LEVEL CONFIG_MCUMGR_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(settings_mgmt);

/**
 * Command handler: settings read
 */
static int settings_mgmt_read(struct smp_streamer *ctxt)
{
	int rc;
	zcbor_state_t *zse = ctxt->writer->zs;
	zcbor_state_t *zsd = ctxt->reader->zs;
	bool ok;
	size_t decoded;
	struct zcbor_string key = { 0 };
	uint32_t max_length = CONFIG_MCUMGR_GRP_SETTINGS_VALUE_LEN;
	bool limited_size = false;

#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	char *key_name = NULL;
	uint8_t *data = NULL;
#else
	char key_name[CONFIG_MCUMGR_GRP_SETTINGS_NAME_LEN] = { 0 };
	uint8_t data[CONFIG_MCUMGR_GRP_SETTINGS_VALUE_LEN] = { 0 };
#endif

	struct zcbor_map_decode_key_val settings_read_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("key", zcbor_tstr_decode, &key),
		ZCBOR_MAP_DECODE_KEY_DECODER("max", zcbor_uint32_decode, &max_length),
	};

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	struct settings_mgmt_access settings_access_data = {
		.access = SETTINGS_ACCESS_READ,
		.name = key_name,
	};

	enum mgmt_cb_return status;
	int32_t ret_rc;
	uint16_t ret_group;
#endif

	if (ctxt->source_header_version < SMP_MCUMGR_VERSION_2) {
		/* Settings management group needs version 2 of the protocol or higher */
		return MGMT_ERR_UNSUPPORTED_TOO_OLD;
	}

	ok = zcbor_map_decode_bulk(zsd, settings_read_decode, ARRAY_SIZE(settings_read_decode),
				   &decoded) == 0;

	if (!ok || key.len == 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Check if the length of the user supplied key is too large for the configuration
	 * to allow and return an error if so
	 */
	if (key.len >= CONFIG_MCUMGR_GRP_SETTINGS_NAME_LEN) {
		ok = smp_add_cmd_ret(zse, MGMT_GROUP_ID_SETTINGS,
				     SETTINGS_MGMT_RET_RC_KEY_TOO_LONG);
		goto end;
	}

	if (max_length > CONFIG_MCUMGR_GRP_SETTINGS_VALUE_LEN) {
		max_length = CONFIG_MCUMGR_GRP_SETTINGS_VALUE_LEN;
		limited_size = true;
	}

#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	key_name = (char *)malloc(key.len + 1);
	data = (uint8_t *)malloc(max_length);

	if (data == NULL || key_name == NULL) {
		if (key_name != NULL) {
			free(key_name);
		}

		return MGMT_ERR_ENOMEM;
	}
#endif

	memcpy(key_name, key.value, key.len);
	key_name[key.len] = 0;

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	/* Send request to application to check if access should be allowed or not */
	status = mgmt_callback_notify(MGMT_EVT_OP_SETTINGS_MGMT_ACCESS, &settings_access_data,
				      sizeof(settings_access_data), &ret_rc, &ret_group);

	if (status != MGMT_CB_OK) {
		bool ok;

		if (status == MGMT_CB_ERROR_RC) {
			return ret_rc;
		}

		ok = smp_add_cmd_ret(zse, ret_group, (uint16_t)ret_rc);
		goto end;
	}
#endif

	/* Read the settings key */
	rc = settings_runtime_get(key_name, data, max_length);

	if (rc < 0) {
		if (rc == -EINVAL) {
			rc = SETTINGS_MGMT_RET_RC_KEY_NOT_FOUND;
		} else if (rc == -ENOTSUP) {
			rc = SETTINGS_MGMT_RET_RC_READ_NOT_SUPPORTED;
		} else {
			rc = SETTINGS_MGMT_RET_RC_UNKNOWN;
		}

		ok = smp_add_cmd_ret(zse, MGMT_GROUP_ID_SETTINGS, (uint16_t)rc);
		goto end;
	}

	ok = zcbor_tstr_put_lit(zse, "data")		&&
	     zcbor_tstr_encode_ptr(zse, data, rc);

	if (ok && limited_size) {
		ok = zcbor_tstr_put_lit(zse, "limited")	&&
		     zcbor_bool_put(zse, true);
	}

end:
#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	free(key_name);
	free(data);
#endif

	return ok ? MGMT_ERR_EOK : MGMT_ERR_EMSGSIZE;
}

/**
 * Command handler: settings write
 */
static int settings_mgmt_write(struct smp_streamer *ctxt)
{
	int rc;
	zcbor_state_t *zse = ctxt->writer->zs;
	zcbor_state_t *zsd = ctxt->reader->zs;
	bool ok;
	struct zcbor_string data = { 0 };
	size_t decoded;
	struct zcbor_string key = { 0 };

#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	char *key_name = NULL;
#else
	char key_name[CONFIG_MCUMGR_GRP_SETTINGS_NAME_LEN];
#endif

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	struct settings_mgmt_access settings_access_data = {
		.access = SETTINGS_ACCESS_WRITE,
		.name = key_name,
		.data = data.value,
		.data_length = &data.len,
	};

	enum mgmt_cb_return status;
	int32_t ret_rc;
	uint16_t ret_group;
#endif

	struct zcbor_map_decode_key_val settings_write_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("key", zcbor_tstr_decode, &key),
		ZCBOR_MAP_DECODE_KEY_DECODER("data", zcbor_bstr_decode, &data),
	};

	if (ctxt->source_header_version < SMP_MCUMGR_VERSION_2) {
		/* Settings management group needs version 2 of the protocol or higher */
		return MGMT_ERR_UNSUPPORTED_TOO_OLD;
	}

	ok = zcbor_map_decode_bulk(zsd, settings_write_decode, ARRAY_SIZE(settings_write_decode),
				   &decoded) == 0;

	if (!ok || key.len == 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Check if the length of the user supplied key is too large for the configuration
	 * to allow and return an error if so
	 */
	if (key.len >= CONFIG_MCUMGR_GRP_SETTINGS_NAME_LEN) {
		ok = smp_add_cmd_ret(zse, MGMT_GROUP_ID_SETTINGS,
				     SETTINGS_MGMT_RET_RC_KEY_TOO_LONG);
		goto end;
	}

#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	key_name = (char *)malloc(key.len + 1);

	if (key_name == NULL) {
		return MGMT_ERR_ENOMEM;
	}
#endif

	memcpy(key_name, key.value, key.len);
	key_name[key.len] = 0;

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	/* Send request to application to check if access should be allowed or not */
	status = mgmt_callback_notify(MGMT_EVT_OP_SETTINGS_MGMT_ACCESS, &settings_access_data,
				      sizeof(settings_access_data), &ret_rc, &ret_group);

	if (status != MGMT_CB_OK) {
		bool ok;

		if (status == MGMT_CB_ERROR_RC) {
			return ret_rc;
		}

		ok = smp_add_cmd_ret(zse, ret_group, (uint16_t)ret_rc);
		goto end;
	}
#endif

	/* Update the settings data */
	rc = settings_runtime_set(key_name, data.value, data.len);

	if (rc < 0) {
		if (rc == -EINVAL) {
			rc = SETTINGS_MGMT_RET_RC_KEY_NOT_FOUND;
		} else {
			rc = SETTINGS_MGMT_RET_RC_UNKNOWN;
		}

		ok = smp_add_cmd_ret(zse, MGMT_GROUP_ID_SETTINGS, (uint16_t)rc);
	}

end:
#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	free(key_name);
#endif

	return ok ? MGMT_ERR_EOK : MGMT_ERR_EMSGSIZE;
}

/**
 * Command handler: settings delete
 */
static int settings_mgmt_delete(struct smp_streamer *ctxt)
{
	zcbor_state_t *zse = ctxt->writer->zs;
	zcbor_state_t *zsd = ctxt->reader->zs;
	int rc;
	bool ok;
	size_t decoded;
	struct zcbor_string key = { 0 };

#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	char *key_name = NULL;
#else
	char key_name[CONFIG_MCUMGR_GRP_SETTINGS_NAME_LEN];
#endif

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	struct settings_mgmt_access settings_access_data = {
		.access = SETTINGS_ACCESS_DELETE,
		.name = key_name,
	};

	enum mgmt_cb_return status;
	int32_t ret_rc;
	uint16_t ret_group;
#endif

	struct zcbor_map_decode_key_val settings_delete_decode[] = {
		ZCBOR_MAP_DECODE_KEY_DECODER("key", zcbor_tstr_decode, &key),
	};

	if (ctxt->source_header_version < SMP_MCUMGR_VERSION_2) {
		/* Settings management group needs version 2 of the protocol or higher */
		return MGMT_ERR_UNSUPPORTED_TOO_OLD;
	}

	ok = zcbor_map_decode_bulk(zsd, settings_delete_decode, ARRAY_SIZE(settings_delete_decode),
				   &decoded) == 0;

	if (!ok || key.len == 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Check if the length of the user supplied key is too large for the configuration
	 * to allow and return an error if so
	 */
	if (key.len >= CONFIG_MCUMGR_GRP_SETTINGS_NAME_LEN) {
		ok = smp_add_cmd_ret(zse, MGMT_GROUP_ID_SETTINGS,
				     SETTINGS_MGMT_RET_RC_KEY_TOO_LONG);
		goto end;
	}

#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	key_name = (char *)malloc(key.len + 1);

	if (key_name == NULL) {
		return MGMT_ERR_ENOMEM;
	}
#endif

	memcpy(key_name, key.value, key.len);
	key_name[key.len] = 0;

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	/* Send request to application to check if access should be allowed or not */
	status = mgmt_callback_notify(MGMT_EVT_OP_SETTINGS_MGMT_ACCESS, &settings_access_data,
				      sizeof(settings_access_data), &ret_rc, &ret_group);

	if (status != MGMT_CB_OK) {
		bool ok;

		if (status == MGMT_CB_ERROR_RC) {
			return ret_rc;
		}

		ok = smp_add_cmd_ret(zse, ret_group, (uint16_t)ret_rc);
		goto end;
	}
#endif

	/* Delete requested key from settings */
	rc = settings_delete(key_name);

#ifdef CONFIG_MCUMGR_GRP_SETTINGS_BUFFER_TYPE_HEAP
	free(key_name);
#endif

	if (rc < 0) {
		if (rc == -ENOENT) {
			rc = SETTINGS_MGMT_RET_RC_KEY_NOT_FOUND;
		} else {
			rc = SETTINGS_MGMT_RET_RC_UNKNOWN;
		}

		ok = smp_add_cmd_ret(zse, MGMT_GROUP_ID_SETTINGS, (uint16_t)rc);
	}

end:
	return ok ? MGMT_ERR_EOK : MGMT_ERR_EMSGSIZE;
}

/**
 * Command handler: settings commit
 */
static int settings_mgmt_commit(struct smp_streamer *ctxt)
{
	bool ok = true;

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	struct settings_mgmt_access settings_access_data = {
		.access = SETTINGS_ACCESS_COMMIT,
	};

	zcbor_state_t *zse = ctxt->writer->zs;
	enum mgmt_cb_return status;
	int32_t ret_rc;
	uint16_t ret_group;
#endif

	if (ctxt->source_header_version < SMP_MCUMGR_VERSION_2) {
		/* Settings management group needs version 2 of the protocol or higher */
		return MGMT_ERR_UNSUPPORTED_TOO_OLD;
	}

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	/* Send request to application to check if access should be allowed or not */
	status = mgmt_callback_notify(MGMT_EVT_OP_SETTINGS_MGMT_ACCESS, &settings_access_data,
				      sizeof(settings_access_data), &ret_rc, &ret_group);

	if (status != MGMT_CB_OK) {
		bool ok;

		if (status == MGMT_CB_ERROR_RC) {
			return ret_rc;
		}

		ok = smp_add_cmd_ret(zse, ret_group, (uint16_t)ret_rc);
		goto end;
	}
#endif

	settings_commit();

end:
	return ok ? MGMT_ERR_EOK : MGMT_ERR_EMSGSIZE;
}

/**
 * Command handler: settings save
 */
static int settings_mgmt_save(struct smp_streamer *ctxt)
{
	bool ok = true;

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	struct settings_mgmt_access settings_access_data = {
		.access = SETTINGS_ACCESS_SAVE,
	};

	zcbor_state_t *zse = ctxt->writer->zs;
	enum mgmt_cb_return status;
	int32_t ret_rc;
	uint16_t ret_group;
#endif

	if (ctxt->source_header_version < SMP_MCUMGR_VERSION_2) {
		/* Settings management group needs version 2 of the protocol or higher */
		return MGMT_ERR_UNSUPPORTED_TOO_OLD;
	}

#if defined(CONFIG_MCUMGR_GRP_SETTINGS_ACCESS_HOOK)
	/* Send request to application to check if access should be allowed or not */
	status = mgmt_callback_notify(MGMT_EVT_OP_SETTINGS_MGMT_ACCESS, &settings_access_data,
				      sizeof(settings_access_data), &ret_rc, &ret_group);

	if (status != MGMT_CB_OK) {
		bool ok;

		if (status == MGMT_CB_ERROR_RC) {
			return ret_rc;
		}

		ok = smp_add_cmd_ret(zse, ret_group, (uint16_t)ret_rc);
		goto end;
	}
#endif

	settings_save();

end:
	return ok ? MGMT_ERR_EOK : MGMT_ERR_EMSGSIZE;
}

static const struct mgmt_handler settings_mgmt_handlers[] = {
	[SETTINGS_MGMT_ID_READ_WRITE] = {
		.mh_read = settings_mgmt_read,
		.mh_write = settings_mgmt_write,
	},
	[SETTINGS_MGMT_ID_DELETE] = {
		.mh_read = NULL,
		.mh_write = settings_mgmt_delete,
	},
	[SETTINGS_MGMT_ID_COMMIT] = {
		.mh_read = NULL,
		.mh_write = settings_mgmt_commit,
	},
	[SETTINGS_MGMT_ID_SAVE] = {
		.mh_read = NULL,
		.mh_write = settings_mgmt_save,
	},
};

#define SETTINGS_MGMT_HANDLER_CNT ARRAY_SIZE(settings_mgmt_handlers)

static struct mgmt_group settings_mgmt_group = {
	.mg_handlers = settings_mgmt_handlers,
	.mg_handlers_count = SETTINGS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_SETTINGS,
};

static void settings_mgmt_register_group(void)
{
	mgmt_register_group(&settings_mgmt_group);
}

MCUMGR_HANDLER_DEFINE(settings_mgmt, settings_mgmt_register_group);

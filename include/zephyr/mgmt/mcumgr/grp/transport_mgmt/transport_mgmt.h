/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_TRANSPORT_MGMT_
#define H_TRANSPORT_MGMT_

/**
 * @brief MCUmgr Transport management API
 * @defgroup mcumgr_transport_mgmt Transport management
 * @ingroup mcumgr_mgmt_api
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Command IDs for transport management group.
 * @{
 */
#define TRANSPORT_MGMT_ID_CONNECT 0 /**<  */
#define TRANSPORT_MGMT_ID_DISCONNECT 1 /**<  */
#define TRANSPORT_MGMT_ID_STATUS 2 /**<  */
/* Following are purposely at the end to be able to see if they have been excluded when using enum mgmt due to being optional when using a custom derivative group ID */
#define TRANSPORT_MGMT_ID_LIST 3 /**<  */
#define TRANSPORT_MGMT_ID_GET_DETAILS 4 /**<  */
#define TRANSPORT_MGMT_ID_GET_CONFIG_DETAILS 5 /**<  */
/** @} */

bool transport_mgmt_is_bridged(struct smp_transport *transport, bool outgoing);
struct smp_transport *transport_mgmt_get_other_transport(struct smp_transport *transport, bool outgoing);
const struct smp_transport_bridge *transport_mgmt_get_bridge(struct smp_transport *transport, bool outgoing);

/**
 * Command result codes for transport management group.
 */
enum transport_mgmt_ret_code_t {
	/** No error, this is implied if there is no ret value in the response. */
	TRANSPORT_MGMT_ERR_OK = 0,

	/** Unknown error occurred. */
	TRANSPORT_MGMT_ERR_UNKNOWN,
};

#if defined(CONFIG_MCUMGR_GRP_TRANSPORT_GROUP_ID_CUSTOM_FUNCTION)
/** Gets the group ID for transport management group, users must implement this in their own code, the value should be within the range 64 - 65535 */
const uint16_t transport_mgmt_group_id(void);
#elif defined(CONFIG_MCUMGR_GRP_TRANSPORT_GROUP_ID_CUSTOM_VALUE)
#define transport_mgmt_group_id() CONFIG_MCUMGR_GRP_TRANSPORT_GROUP_ID_CUSTOM_VALUE_GROUP_ID
#else
#define transport_mgmt_group_id() MGMT_GROUP_ID_TRANSPORT
#endif

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif

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
#define TRANSPORT_MGMT_ID_LIST 0 /**<  */
#define TRANSPORT_MGMT_ID_GET_DETAILS 1 /**<  */
#define TRANSPORT_MGMT_ID_GET_CONFIG_DETAILS 2 /**<  */
#define TRANSPORT_MGMT_ID_CONNECT 3 /**<  */
#define TRANSPORT_MGMT_ID_DISCONNECT 4 /**<  */
#define TRANSPORT_MGMT_ID_STATUS 5 /**<  */
/** @} */

bool transport_mgmt_is_bridged(struct smp_transport *transport, bool outgoing);
struct smp_transport *transport_mgmt_get_other_transport(struct smp_transport *transport, bool outgoing);

/**
 * Command result codes for transport management group.
 */
enum transport_mgmt_ret_code_t {
	/** No error, this is implied if there is no ret value in the response. */
	TRANSPORT_MGMT_ERR_OK = 0,

	/** Unknown error occurred. */
	TRANSPORT_MGMT_ERR_UNKNOWN,
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif

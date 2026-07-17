/*
 * Copyright (c) 2025-2026 Nordic Semiconductor ASA
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
enum transport_mgmt_ids {
	/** Used to connect (bridge) to another transport */
	TRANSPORT_MGMT_ID_CONNECT,

	/** Used to disconnect current bridge or all bridges */
	TRANSPORT_MGMT_ID_DISCONNECT,

	/** Returns current bridge status */
	TRANSPORT_MGMT_ID_STATUS,

	/** Reserved for future use */
	TRANSPORT_MGMT_ID_RESERVED_1,

	/** Reserved for future use */
	TRANSPORT_MGMT_ID_RESERVED_2,

	/** Reserved for future use */
	TRANSPORT_MGMT_ID_RESERVED_3,

	/*
	 * The following are purposely at the end to be able to see if they have been excluded
	 * when using enum mgmt due to being optional when using a custom derivative group ID
	 */

	/** List supported transports */
	TRANSPORT_MGMT_ID_LIST,

	/** Get the supported modes of a transport  */
	TRANSPORT_MGMT_ID_GET_MODES,

	/** Get the configuration details of a transport */
	TRANSPORT_MGMT_ID_GET_CONFIG_DETAILS,
};
/** @} */

/**
 * @brief Checks if a given transport is bridged or not.
 *
 * @param transport	transport to check status of
 * @param outgoing	if true, will check the outgoing state, othersie checks incoming state
 *
 * @return true if given transport in given mode is bridged, false otherwise.
 */
bool transport_mgmt_is_bridged(struct smp_transport *transport, bool outgoing);

/**
 * @brief Gets the other transport which is part of a bridge.
 *
 * @param transport	transport which is bridged
//TODO
 * @param outgoing	if true, will check the outgoing state, othersie checks incoming state
 *
 * @return the other transport context if a valid bridge was provided, otherwise NULL.
 */
struct smp_transport *transport_mgmt_get_other_transport(struct smp_transport *transport, bool outgoing);

/**
 * @brief Gets a transport bridge context.
 *
 * @param transport	transport which is bridged
//TODO
 * @param outgoing	if true, will check the outgoing state, othersie checks incoming state
 *
 * @return the transport bridge context if a valid bridged transport was provided, otherwise NULL.
 */
const struct smp_transport_bridge *transport_mgmt_get_bridge(struct smp_transport *transport, bool outgoing);

/**
 * Command result codes for transport management group.
 */
enum transport_mgmt_ret_code_t {
	/** No error, this is implied if there is no ret value in the response. */
	TRANSPORT_MGMT_ERR_OK = 0,

	/** Unknown error occurred. */
	TRANSPORT_MGMT_ERR_UNKNOWN,

	/** The transport is missing the required mandatory bridging functions. */
	TRANSPORT_MGMT_ERR_TRANSPORT_MISSING_REQUIRED_FUNCTIONS,

	/** The transport is missing the information bridging functions. */
	TRANSPORT_MGMT_ERR_TRANSPORT_MISSING_INFO_FUNCTIONS,

	/** Invalid, unsupported or no transport ID provided. */
	TRANSPORT_MGMT_ERR_INVALID_TRANSPORT,

	/** Invalid, unsupported or no mode provided. */
	TRANSPORT_MGMT_ERR_INVALID_MODE,

	/** All transport bridging context are in use. */
	TRANSPORT_MGMT_ERR_ALL_CONTEXTS_USED,

	/** The transport or all parameters were not provided and one is required. */
	TRANSPORT_MGMT_ERR_INVALID_TRANSPORT_OR_ALL_PARAMETERS,

	/** The transport or all parameters were both provided and only one should be supplied. */
	TRANSPORT_MGMT_ERR_BOTH_TRANSPORT_AND_ALL_PARAMETERS,

	/** The transport is not bridged. */
	TRANSPORT_MGMT_ERR_NOT_BRIDGED,

	/** The transport does not support being used as both the input and output bridge device. */
	TRANSPORT_MGMT_ERR_SAME_BRIDGE_DEVICE_DISALLOWED,

	/** . */
//	TRANSPORT_MGMT_ERR_,
	/** . */
//	TRANSPORT_MGMT_ERR_,
};

#if defined(CONFIG_MCUMGR_GRP_TRANSPORT_GROUP_ID_CUSTOM_FUNCTION) || defined(__DOXYGEN__)
/**
 * @brief Gets the group ID for transport management group.
 *
 * Users must implement this function in their own code if they wish to use a dynamic group ID
 * set at run-time when ``CONFIG_MCUMGR_GRP_TRANSPORT_GROUP_ID_CUSTOM_FUNCTION`` is enabled.
 *
 * @return The transport mgmt group ID which must be within the range 64 - 65535.
 */
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

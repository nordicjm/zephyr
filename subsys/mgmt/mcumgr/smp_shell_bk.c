/*
 * Copyright Runtime.io 2018. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 * @brief Shell transport for the mcumgr SMP protocol.
 */

#include <string.h>
#include <zephyr.h>
#include <init.h>
#include "net/buf.h"
#include "mgmt/mgmt.h"
#include "mgmt/mcumgr/serial.h"
#include "mgmt/mcumgr/buf.h"
#include "mgmt/mcumgr/smp.h"
#include "mgmt/mcumgr/smp_shell.h"
#include "drivers/uart.h"
#include "syscalls/uart.h"
#include "shell/shell.h"
#include "shell/shell_uart.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(smp_shell);

static struct zephyr_smp_transport smp_shell_transport;

static struct mcumgr_serial_rx_ctxt smp_shell_rx_ctxt;

extern void lockuart();
extern void unlockuart();

/** SMP mcumgr frame fragments. */
enum smp_shell_esc_mcumgr {
	ESC_MCUMGR_PKT_1,
	ESC_MCUMGR_PKT_2,
	ESC_MCUMGR_FRAG_1,
	ESC_MCUMGR_FRAG_2,
};

/** These states indicate whether an mcumgr frame is being received. */
enum smp_shell_mcumgr_state {
	SMP_SHELL_MCUMGR_STATE_NONE,
	SMP_SHELL_MCUMGR_STATE_HEADER,
	SMP_SHELL_MCUMGR_STATE_PAYLOAD
};

static int read_mcumgr_byte(struct smp_shell_data *data, uint8_t byte)
{
	bool frag_1;
	bool frag_2;
	bool pkt_1;
	bool pkt_2;

	pkt_1 = atomic_test_bit(&data->esc_state, ESC_MCUMGR_PKT_1);
	pkt_2 = atomic_test_bit(&data->esc_state, ESC_MCUMGR_PKT_2);
	frag_1 = atomic_test_bit(&data->esc_state, ESC_MCUMGR_FRAG_1);
	frag_2 = atomic_test_bit(&data->esc_state, ESC_MCUMGR_FRAG_2);

	if (pkt_2 || frag_2) {
		/* Already fully framed. */
		return SMP_SHELL_MCUMGR_STATE_PAYLOAD;
	}

	if (pkt_1) {
		if (byte == MCUMGR_SERIAL_HDR_PKT_2) {
			/* Final framing byte received. */
			atomic_set_bit(&data->esc_state, ESC_MCUMGR_PKT_2);
			return SMP_SHELL_MCUMGR_STATE_PAYLOAD;
		}
	} else if (frag_1) {
		if (byte == MCUMGR_SERIAL_HDR_FRAG_2) {
			/* Final framing byte received. */
			atomic_set_bit(&data->esc_state, ESC_MCUMGR_FRAG_2);
			return SMP_SHELL_MCUMGR_STATE_PAYLOAD;
		}
	} else {
		if (byte == MCUMGR_SERIAL_HDR_PKT_1) {
			/* First framing byte received. */
			atomic_set_bit(&data->esc_state, ESC_MCUMGR_PKT_1);
			return SMP_SHELL_MCUMGR_STATE_HEADER;
		} else if (byte == MCUMGR_SERIAL_HDR_FRAG_1) {
			/* First framing byte received. */
			atomic_set_bit(&data->esc_state, ESC_MCUMGR_FRAG_1);
			return SMP_SHELL_MCUMGR_STATE_HEADER;
		}
	}

	/* Non-mcumgr byte received. */
	return SMP_SHELL_MCUMGR_STATE_NONE;
}

const struct shell_uart *tmpshuart;

size_t smp_shell_rx_bytes(struct smp_shell_data *data, const uint8_t *bytes,
			  size_t size, const struct shell_uart *sh_uart)
{
	size_t consumed = 0;		/* Number of bytes consumed by SMP */
tmpshuart = sh_uart;

//shell_print(sh_uart, "lol");

	/* Process all bytes that are accepted as SMP commands. */
	while (size != consumed) {
		uint8_t byte = bytes[consumed];
		int mcumgr_state = read_mcumgr_byte(data, byte);

		if (mcumgr_state == SMP_SHELL_MCUMGR_STATE_NONE) {
			break;
		} else if (mcumgr_state == SMP_SHELL_MCUMGR_STATE_HEADER &&
			   !data->buf) {
			data->buf = net_buf_alloc(data->buf_pool, K_NO_WAIT);
			if (!data->buf) {
				LOG_WRN("Failed to alloc SMP buf");
			}
		}

		if (data->buf && net_buf_tailroom(data->buf) > 0) {
			net_buf_add_u8(data->buf, byte);
		}

		/* Newline in payload means complete frame */
		if (mcumgr_state == SMP_SHELL_MCUMGR_STATE_PAYLOAD &&
		    byte == '\n') {
			if (data->buf) {
				net_buf_put(&data->buf_ready, data->buf);
				data->buf = NULL;
			}
			atomic_clear_bit(&data->esc_state, ESC_MCUMGR_PKT_1);
			atomic_clear_bit(&data->esc_state, ESC_MCUMGR_PKT_2);
			atomic_clear_bit(&data->esc_state, ESC_MCUMGR_FRAG_1);
			atomic_clear_bit(&data->esc_state, ESC_MCUMGR_FRAG_2);
		}

		++consumed;
	}

	return consumed;
}

void smp_shell_process(struct smp_shell_data *data)
{
	struct net_buf *buf;
	struct net_buf *nb;

	while (true) {
		buf = net_buf_get(&data->buf_ready, K_NO_WAIT);
		if (!buf) {
			break;
		}

		nb = mcumgr_serial_process_frag(&smp_shell_rx_ctxt,
						buf->data,
						buf->len);
		if (nb != NULL) {
			zephyr_smp_rx_req(&smp_shell_transport, nb);
		}

		net_buf_unref(buf);
	}
}

static uint16_t smp_shell_get_mtu(const struct net_buf *nb)
{
	return CONFIG_MCUMGR_SMP_SHELL_MTU;
}

uint8_t buf[1024];
uint16_t bufpos = 0;

static int smp_shell_tx_raw(const void *data, int len, void *arg)
{
	const struct shell *const sh = shell_backend_uart_get_ptr();
	const struct shell_uart *const su = sh->iface->ctx;
	const struct shell_uart_ctrl_blk *const scb = su->ctrl_blk;
	const uint8_t *out = data;
//shell_hexdump(sh, data, len);
//shell_print(sh, "OK");

memcpy(&buf[bufpos], data, len);
bufpos += len;

/*
extern const struct shell_transport_api shell_uart_transport_api;
extern const struct shell_transport shell_transport_uart;
uint16_t wtf = 0;
while (wtf < len) {
size_t whocares = 0;
shell_uart_transport_api.write(&shell_transport_uart, &out[wtf], (len - wtf), &whocares);
wtf += whocares;
}
*/
//		uart_poll_out2(scb->dev, out, len);
//	while ((out != NULL) && (len != 0)) {
//		uart_poll_out(scb->dev, *out);
//		++out;
//		--len;
//	}

	return 0;
}

static int smp_shell_tx_pkt(struct zephyr_smp_transport *zst,
			    struct net_buf *nb)
{
	int rc;

lockuart();
bufpos = 0;
	rc = mcumgr_serial_tx_pkt(nb->data, nb->len, smp_shell_tx_raw, NULL);
	mcumgr_buf_free(nb);

/*
	const struct shell *const sh = shell_backend_uart_get_ptr();
	const struct shell_uart *const su = sh->iface->ctx;
	const struct shell_uart_ctrl_blk *const scb = su->ctrl_blk;
uint8_t crap[16];
shell_hexdump(sh, crap, 8);
//shell_hexdump(sh, buf, bufpos);
//shell_print(sh, "wtf");
*/

	const struct shell *const sh = shell_backend_uart_get_ptr();
extern const struct shell_transport_api shell_uart_transport_api;
extern const struct shell_transport shell_transport_uart;
uint16_t wtf = 0;
        k_mutex_lock(&sh->ctx->wr_mtx, K_FOREVER);

while (wtf < bufpos) {
size_t whocares = 0;
shell_uart_transport_api.write(&shell_transport_uart, &buf[wtf], (bufpos - wtf), &whocares);
wtf += whocares;
}
        k_mutex_unlock(&sh->ctx->wr_mtx);


/*
extern const struct shell_transport_api shell_uart_transport_api;
extern const struct shell_transport shell_transport_uart;
size_t whocares;
shell_uart_transport_api.write(&shell_transport_uart, buf, bufpos, &whocares);
*/
unlockuart();

	return rc;
}

int smp_shell_init(void)
{
	zephyr_smp_transport_init(&smp_shell_transport, smp_shell_tx_pkt,
				  smp_shell_get_mtu, NULL, NULL);

	return 0;
}

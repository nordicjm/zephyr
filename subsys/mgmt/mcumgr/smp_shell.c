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
#include "drivers/gpio.h"
#include "syscalls/uart.h"
#include "shell/shell.h"
#include "shell/shell_uart.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(smp_shell);

static struct zephyr_smp_transport smp_shell_transport;

static struct mcumgr_serial_rx_ctxt smp_shell_rx_ctxt;

extern struct shell_transport shell_transport_uart;

const struct device *dev;

extern void lockuart();
//extern void unlockuart();
extern void unlockuart(const struct device *dev);
extern void lockuart2();
extern void unlockuart2();


#if 0
K_MUTEX_DEFINE(my_mutex3);

void lockuart3()
{
k_mutex_lock(&my_mutex3, K_FOREVER);
//while (k_mutex_lock(&my_mutex3, K_NO_WAIT) != 0)
//{
//k_yield();
//}
}

void unlockuart3()
{
k_mutex_unlock(&my_mutex3);
}
#endif


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
//gpio_pin_set(dev, 5, 1);
//lockuart();
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
//gpio_pin_set(dev, 4, 1);
			return SMP_SHELL_MCUMGR_STATE_HEADER;
		} else if (byte == MCUMGR_SERIAL_HDR_FRAG_1) {
			/* First framing byte received. */
			atomic_set_bit(&data->esc_state, ESC_MCUMGR_FRAG_1);
			return SMP_SHELL_MCUMGR_STATE_HEADER;
		}
	}

	/* Non-mcumgr byte received. */
//gpio_pin_set(dev, 4, 0);
//gpio_pin_set(dev, 5, 0);
//gpio_pin_set(dev, 6, 0);
//gpio_pin_set(dev, 7, 0);
	return SMP_SHELL_MCUMGR_STATE_NONE;
}

size_t smp_shell_rx_bytes(struct smp_shell_data *data, const uint8_t *bytes,
			  size_t size)
{
	size_t consumed = 0;		/* Number of bytes consumed by SMP */

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
//gpio_pin_set(dev, 6, 1);
			net_buf_add_u8(data->buf, byte);
		}

		/* Newline in payload means complete frame */
		if (mcumgr_state == SMP_SHELL_MCUMGR_STATE_PAYLOAD &&
		    byte == '\n') {
			if (data->buf) {
				net_buf_put(&data->buf_ready, data->buf);
				data->buf = NULL;
			}
//gpio_pin_set(dev, 7, 1);

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

static void irq_write2(const struct shell_uart *sh_uart, const void *data,
                     size_t length, size_t *cnt)
{
//lockuart();

        *cnt = ring_buf_put(sh_uart->tx_ringbuf, data, length);

        if (atomic_set(&sh_uart->ctrl_blk->tx_busy, 1) == 0) {
#ifdef CONFIG_SHELL_BACKEND_SERIAL_INTERRUPT_DRIVEN
                uart_irq_tx_enable(sh_uart->ctrl_blk->dev);
#endif
        }
//unlockuart();
}


static int smp_shell_tx_raw(const void *data, int len, void *arg)
{
	const struct shell *const sh = shell_backend_uart_get_ptr();
	const struct shell_uart *const su = sh->iface->ctx;
	const struct shell_uart_ctrl_blk *const scb = su->ctrl_blk;
	const uint8_t *out = data;

	if (out != NULL && len > 0) {
		size_t cnt;

		while (len > 0) {
			shell_uart_transport_api.write(&shell_transport_uart, data, len, &cnt);
			data += cnt;
			len -= cnt;
		}
	}

	return 0;
}

static int smp_shell_tx_pkt(struct zephyr_smp_transport *zst,
			    struct net_buf *nb)
{
	int rc;

	const struct shell *const sh = shell_backend_uart_get_ptr();
	const struct shell_uart *const su = sh->iface->ctx;
	const struct shell_uart_ctrl_blk *const scb = su->ctrl_blk;
//skipfunc();
//lockuart();
//lockuart2();
//        k_mutex_lock(&sh->ctx->wr_mtx, K_FOREVER);
                uart_lock(scb->dev);
//lockuart3();
	rc = mcumgr_serial_tx_pkt(nb->data, nb->len, smp_shell_tx_raw, NULL);
	mcumgr_buf_free(nb);
                uart_unlock(scb->dev);
//unlockuart3();

//while (uart_irq_tx_complete(scb->dev))
//{
//k_sleep(K_SECONDS(1));
//}
//        k_mutex_unlock(&sh->ctx->wr_mtx);
//unlockuart(scb->dev);
//unlockuart();
//unlockuart2();


	return rc;
}


int smp_shell_init(void)
{
	zephyr_smp_transport_init(&smp_shell_transport, smp_shell_tx_pkt,
				  smp_shell_get_mtu, NULL, NULL);


int ret;

//dev = device_get_binding("//gpio_1");
//ret = gpio_pin_configure(dev, 4, //gpio_OUTPUT | //gpio_ACTIVE_HIGH);
//ret = gpio_pin_configure(dev, 5, //gpio_OUTPUT | //gpio_ACTIVE_HIGH);
//ret = gpio_pin_configure(dev, 6, //gpio_OUTPUT | //gpio_ACTIVE_HIGH);
//ret = gpio_pin_configure(dev, 7, //gpio_OUTPUT | //gpio_ACTIVE_HIGH);
//gpio_pin_set(dev, 4, 0);
//gpio_pin_set(dev, 5, 0);
//gpio_pin_set(dev, 6, 0);
//gpio_pin_set(dev, 7, 0);

	return 0;
}

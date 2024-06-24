/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void trigger_signal()
{
//uint32_t loops = 17615;
uint32_t loops = 15;
int ret;

while (loops > 0) {
		ret = gpio_pin_toggle_dt(&led);
//tmp = sys_clock_cycle_get_32() + 1;
//while (tmp > sys_clock_cycle_get_32());

__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");

//77 = 7.3us
//1 = 0.0948051948052
//8.564

#if 0
tmp = 12;
while (tmp > 0)
{
--tmp;
}
#endif

		ret = gpio_pin_toggle_dt(&led);

__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");

--loops;
}
}

int main(void)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

//AC mode, high fan, 18c, move up/down
uint8_t data[] = {
0b01001010,
0b01110101,
0b11000011,
0b01100100,
0b10011011,
0b11111111,
0b00000000,
0b11110110,
0b00001001,
0b01100111,
0b10011000
};

#if 0
//AC mode, medium fan, 18c, move up/down
0b01001010
0b01110101
0b11000011
0b01100100
0b10011011
0b11111111
0b00000000
0b11110001
0b00001110
0b01100111
0b10011000

//AC mode 2h cooldown
0b01001010
0b01110101
0b11000011
0b01100100
0b10011011
0b11111011
0b00000100
0b11110001
0b00001110
0b01100111
0b10011000

//off
0b01001010
0b01110101
0b11000011
0b01100100
0b10011011
0b11111111
0b00000000
0b11110001
0b00001110
0b01110111
0b10001000
#endif

trigger_signal();
trigger_signal();
trigger_signal();
trigger_signal();
trigger_signal();
trigger_signal();
trigger_signal();
trigger_signal();

//k_busy_wait(1606);
k_busy_wait(1608);
__asm("NOP");

//k_sleep(K_MSEC(40));

uint8_t q = 0;
while (q < ARRAY_SIZE(data))
{
uint8_t check = data[q];
uint8_t t = 0;
while (t < 8)
{

trigger_signal();

if ((check & 0x80) == 0)
{
//420us
//k_busy_wait(420);
k_busy_wait(404);
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");
__asm("NOP");

}
else
{
//1.214ms
k_busy_wait(1205);
__asm("NOP");
}

check = check << 1;
++t;
}
++q;
}

trigger_signal();


//		ret = gpio_pin_toggle_dt(&led);
	return 0;
}

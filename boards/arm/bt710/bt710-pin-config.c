/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2023 Jamie M.
 *
 * This file is not apache licensed
*/
#define HIGH_DRIVE (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) | \
                   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                   (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) | \
                   (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos) | \
                   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <nrf.h>
#include <nrf_peripherals.h>

static int pin_setup(void)
{
	NRF_P0->PIN_CNF[4] = HIGH_DRIVE;
	NRF_P0->PIN_CNF[11] = HIGH_DRIVE;
	NRF_P1->PIN_CNF[8] = HIGH_DRIVE;
	NRF_P1->PIN_CNF[9] = HIGH_DRIVE;

	return 0;
}

SYS_INIT(pin_setup, POST_KERNEL, 69);

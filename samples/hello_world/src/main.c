/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#ifdef CONFIG_RETENTION
#include <zephyr/retention/retention.h>
#include <zephyr/retention/blinfo.h>
#endif

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

#ifdef CONFIG_RETENTION
	uint8_t retval;
	int rc;

	rc = blinfo_lookup(BLINFO_MODE, &retval, sizeof(retval));
	printk("Mode (%d): %d\n", rc, retval);

	rc = blinfo_lookup(BLINFO_SIGNATURE_TYPE, &retval, sizeof(retval));
	printk("Signature type (%d): %d\n", rc, retval);

	rc = blinfo_lookup(BLINFO_RUNNING_SLOT, &retval, sizeof(retval));
	printk("Slot (%d): %d\n", rc, retval);
#endif

	return 0;
}

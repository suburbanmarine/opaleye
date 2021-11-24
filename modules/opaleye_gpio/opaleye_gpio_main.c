// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
// Copyright (C) 2021 Suburban Marine, Inc.

#include "opaleye_gpio_main.h"

#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/interrupt.h>

MODULE_AUTHOR("Suburban Marine, Inc.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("opaleye_gpio");
MODULE_SUPPORTED_DEVICE("opaleye_gpio");

int __init opaleye_gpio_init(void)
{
	struct task_struct* task_ptr = kthread_run(&opaleye_gpio_main, NULL, "opaleye_gpio_main");
	return 0;
}
void __exit opaleye_gpio_exit(void)
{

}

int opaleye_gpio_main(void* data)
{
	for(;;)
	{

	}
	return 0;
}
module_init(opaleye_gpio_init);
module_exit(opaleye_gpio_exit);

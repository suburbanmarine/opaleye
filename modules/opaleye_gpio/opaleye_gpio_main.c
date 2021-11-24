// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
// Copyright (C) 2021 Suburban Marine, Inc.

#include "opaleye_gpio_main.h"

#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>

MODULE_AUTHOR("Suburban Marine, Inc.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("opaleye_gpio");
MODULE_SUPPORTED_DEVICE("opaleye_gpio");

typedef struct opaleye_gpio_state_t
{
	struct task_struct* main_task_ptr;
	struct hrtimer gpio_timer;
} opaleye_gpio_state_t;

static struct opaleye_gpio_state_t* g_gpio_state;

int __init opaleye_gpio_init(void)
{
	printk(KERN_INFO "opaleye_gpio starting");

	opaleye_gpio_state_t* state = kzalloc(sizeof(opaleye_gpio_state_t), GFP_KERNEL);
	if( ! state )
	{
		printk(KERN_ERR "opaleye_gpio failed to alloc block");
		return -1;
	}

	state->main_task_ptr = kthread_run(&opaleye_gpio_main, NULL, "opaleye_gpio_main");
	if( ! state->main_task_ptr )
	{
		printk(KERN_ERR "opaleye_gpio failed to start thread");

		kfree(g_gpio_state);
		g_gpio_state = NULL;
		return -1;
	}

	g_gpio_state = state;

	return 0;
}
void __exit opaleye_gpio_exit(void)
{
	if(g_gpio_state)
	{
		//stop thread
		int task_ret = kthread_stop(g_gpio_state->main_task_ptr);

		//release mem
		kfree(g_gpio_state);
		g_gpio_state = NULL;
	}

	printk(KERN_INFO "opaleye_gpio done");
}

enum hrtimer_restart opaleye_gpio_timer_cb(struct hrtimer* t)
{
	opaleye_gpio_state_t* state = container_of(t, opaleye_gpio_state_t, gpio_timer);
	hrtimer_forward_now(&state->gpio_timer, ktime_set(1, 0));

	//do something, toggle a pin

	// return HRTIMER_NORESTART;
	return HRTIMER_RESTART;
}

int opaleye_gpio_main(void* data)
{
	printk(KERN_INFO "opaleye_gpio_main starting");

	//startup
	hrtimer_init(&g_gpio_state->gpio_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD
	g_gpio_state->gpio_timer.function = opaleye_gpio_timer_cb;

	// ktime_t kt_now = ktime_get_real();
	struct timespec64 t_now;
	ktime_get_real_ts64(&t_now);

	ktime_t kt_0 = ktime_set(t_now.tv_sec + 1, 0);

	hrtimer_start(&g_gpio_state->gpio_timer, kt_0, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD

	while( ! kthread_should_stop() )
	{
		msleep_interruptible(1000);
	}

	//tear down
	{
		//stop event cb
		hrtimer_cancel(&g_gpio_state->gpio_timer);
	}

	printk(KERN_INFO "opaleye_gpio_main done");

	return 0;
}
module_init(opaleye_gpio_init);
module_exit(opaleye_gpio_exit);

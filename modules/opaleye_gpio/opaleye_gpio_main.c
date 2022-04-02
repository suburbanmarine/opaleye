// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
// Copyright (C) 2021 Suburban Marine, Inc.

#include "opaleye_gpio_main.h"

#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/gpio.h>

//xavier
#include <dt-bindings/gpio/tegra194-gpio.h>
#define TEGRA194_BASE 288

//tx2
// #include <linux/dt-bindings/gpio/tegra186-gpio.h>

//nano
// #include <linux/dt-bindings/gpio/tegra-gpio.h>

// for sched_setscheduler
// #include <linux/sched.h>
// #include <linux/sched/types.h>
// #include <uapi/linux/sched/types.h>

MODULE_AUTHOR("Suburban Marine, Inc.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("opaleye_gpio");
MODULE_SUPPORTED_DEVICE("opaleye_gpio");




// CAM0_PWDN	114	SOC_GPIO04 / GPIO3_PP.04
// CAM1_PWDN	120	SOC_GPIO05 / GPIO3_PP.05
// CAM0_MCLK	116	EXTPERIPH1_CLK / GPIO3_PP.00
// CAM1_MCLK	122	EXTPERIPH2_CLK / GPIO3_PP.01

typedef struct opaleye_gpio_state_t
{
	struct task_struct* main_task_ptr;
	struct hrtimer gpio_on_timer;
	struct hrtimer gpio_off_timer;

	//CAM0_PWDN
	unsigned GPIO03_P04;
	int GPIO03_P04_val;

	//CAM1_PWDN
	unsigned GPIO03_P05;
	int GPIO03_P05_val;

	//CAM0_MCLK
	unsigned GPIO03_P00;
	int GPIO03_P00_val;

	//CAM1_MCLK
	unsigned GPIO03_P01;
	int GPIO03_P01_val;

	// unsigned GPIO13_N01;
	// int GPIO13_N01_val;
	// unsigned GPIO01_Q05;
	// int GPIO01_Q05_val;
	// unsigned GPIO11_Q06;
	// int GPIO11_Q06_val;
	// unsigned GPIO09_R00;
	// int GPIO09_R00_val;
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

	//get CSI gpios
	// CAM0_PWDN cam0 GPIO0 - xavier GPIO03_P04
	state->GPIO03_P04 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 4);
	int ret = gpio_request_one(state->GPIO03_P04, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_GPIO03_P04");
	if(ret != 0)
	{
		printk(KERN_ERR "gpio_request_one failed to start GPIO03_P04");
		kfree(state);
		state = NULL;
		return -1;
	}

	// CAM1_PWDN cam1 GPIO0 - xavier GPIO03_P05
	state->GPIO03_P05 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 5);
	ret = gpio_request_one(state->GPIO03_P05, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_GPIO03_P05");
	if(ret != 0)
	{
		printk(KERN_ERR "gpio_request_one failed to start GPIO03_P05");
		kfree(state);
		state = NULL;
		return -1;
	}

	// CAM0_MCLK cam0 GPIO1 - xavier GPIO03_P00
	state->GPIO03_P00 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 0);
	ret = gpio_request_one(state->GPIO03_P00, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_GPIO03_P00");
	if(ret != 0)
	{
		printk(KERN_ERR "gpio_request_one failed to start GPIO03_P00");
		kfree(state);
		state = NULL;
		return -1;
	}

	// CAM1_MCLK cam1 GPIO1 - xavier GPIO03_P01
	state->GPIO03_P01 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 1);
	ret = gpio_request_one(state->GPIO03_P01, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_GPIO03_P01");
	if(ret != 0)
	{
		printk(KERN_ERR "gpio_request_one failed to start GPIO03_P01");
		kfree(state);
		state = NULL;
		return -1;
	}


	// // get Ext40 gpios
	// state->GPIO13_N01 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_N, 1);
	// ret = gpio_request_one(state->GPIO13_N01, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_gpio_pin13");
	// if(ret != 0)
	// {
	// 	printk(KERN_ERR "gpio_request_one failed to start GPIO13_N01");
	// 	kfree(state);
	// 	state = NULL;
	// 	return -1;
	// }

	// state->GPIO01_Q05 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_Q, 5);
	// ret = gpio_request_one(state->GPIO01_Q05, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_gpio_pin1");
	// if(ret != 0)
	// {
	// 	printk(KERN_ERR "gpio_request_one failed to start GPIO01_Q05");
	// 	kfree(state);
	// 	state = NULL;
	// 	return -1;
	// }

	// state->GPIO11_Q06 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_Q, 6);
	// ret = gpio_request_one(state->GPIO11_Q06, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_gpio_pin11");
	// if(ret != 0)
	// {
	// 	printk(KERN_ERR "gpio_request_one failed to start GPIO11_Q06");
	// 	kfree(state);
	// 	state = NULL;
	// 	return -1;
	// }

	// state->GPIO09_R00 = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_R, 0);
	// ret = gpio_request_one(state->GPIO09_R00, GPIOF_DIR_OUT | GPIOF_INIT_LOW, "opaleye_gpio_pin9");
	// if(ret != 0)
	// {
	// 	printk(KERN_ERR "gpio_request_one failed to start GPIO09_R00");
	// 	kfree(state);
	// 	state = NULL;
	// 	return -1;
	// }

	// or to pin to a cpu
	// kthread_create
	// kthread_bind
	// wake_up_process
	state->main_task_ptr = kthread_run(&opaleye_gpio_main, NULL, "opaleye_gpio_main");
	if( ! state->main_task_ptr )
	{
		printk(KERN_ERR "opaleye_gpio failed to start thread");

		kfree(state);
		state = NULL;
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

enum hrtimer_restart opaleye_gpio_on_timer_cb(struct hrtimer* t)
{
	if( ! t )
	{
		printk(KERN_ERR "opaleye_gpio_on_timer_cb t is null");
	}

	opaleye_gpio_state_t* state = container_of(t, opaleye_gpio_state_t, gpio_on_timer);
	if( ! state )
	{
		printk(KERN_ERR "opaleye_gpio_on_timer_cb state is null");
	}

	hrtimer_forward_now(&state->gpio_on_timer, ktime_set(1, 0)); // 1s
	// hrtimer_forward_now(&state->gpio_on_timer, ktime_set(0, 500 * 1000 * 1000)); // 500ms
	//or hrtimer_set_expires

	//do something, toggle a pin
	gpio_set_value(state->GPIO03_P04, 1);
	state->GPIO03_P04_val = 1;

	gpio_set_value(state->GPIO03_P05, 1);
	state->GPIO03_P05_val = 1;

	gpio_set_value(state->GPIO03_P00, 1);
	state->GPIO03_P00_val = 1;

	gpio_set_value(state->GPIO03_P01, 1);
	state->GPIO03_P01_val = 1;

	// return HRTIMER_NORESTART;
	return HRTIMER_RESTART;
}

enum hrtimer_restart opaleye_gpio_off_timer_cb(struct hrtimer* t)
{
	if( ! t )
	{
		printk(KERN_ERR "opaleye_gpio_off_timer_cb t is null");
	}

	opaleye_gpio_state_t* state = container_of(t, opaleye_gpio_state_t, gpio_off_timer);
	if( ! state )
	{
		printk(KERN_ERR "opaleye_gpio_off_timer_cb state is null");
	}

	hrtimer_forward_now(&state->gpio_off_timer, ktime_set(1, 0)); // 1s
	// hrtimer_forward_now(&state->gpio_off_timer, ktime_set(0, 500 * 1000 * 1000)); // 500ms
	//or hrtimer_set_expires

	//do something, toggle a pin
	gpio_set_value(state->GPIO03_P04, 0);
	state->GPIO03_P04_val = 0;

	gpio_set_value(state->GPIO03_P05, 0);
	state->GPIO03_P05_val = 0;

	gpio_set_value(state->GPIO03_P00, 0);
	state->GPIO03_P00_val = 0;

	gpio_set_value(state->GPIO03_P01, 0);
	state->GPIO03_P01_val = 0;

	// return HRTIMER_NORESTART;
	return HRTIMER_RESTART;
}

// https://github.com/torvalds/linux/blob/master/include/dt-bindings/gpio/tegra194-gpio.h
unsigned get_tegra194_gpio(int base, int port, int offset)
{
	return base + port*8 + offset;
}

int opaleye_gpio_main(void* data)
{
	printk(KERN_INFO "opaleye_gpio_main starting");

	// set prio
	// 	struct sched_param param = {
	// 		.sched_priority = 50
	// 	};
	// sched_setscheduler(g_gpio_state->main_task_ptr, SCHED_RR, &param); // or SCHED_FIFO

	//startup
	hrtimer_init(&g_gpio_state->gpio_on_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD
	g_gpio_state->gpio_on_timer.function = opaleye_gpio_on_timer_cb;

	hrtimer_init(&g_gpio_state->gpio_off_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD
	g_gpio_state->gpio_off_timer.function = opaleye_gpio_off_timer_cb;

	// ktime_t kt_now = ktime_get_real();
	struct timespec64 t_now;
	ktime_get_real_ts64(&t_now);

	ktime_t kt_0_on = ktime_set(t_now.tv_sec + 1, 0);
	hrtimer_start(&g_gpio_state->gpio_on_timer, kt_0_on, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD

	ktime_t kt_0_off = ktime_set(t_now.tv_sec + 1, 1 * 1000 * 1000);
	hrtimer_start(&g_gpio_state->gpio_off_timer, kt_0_off, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD

	while( ! kthread_should_stop() )
	{
		msleep_interruptible(1000);
	}

	//tear down
	{
		//stop event cb
		hrtimer_cancel(&g_gpio_state->gpio_on_timer);
		hrtimer_cancel(&g_gpio_state->gpio_off_timer);
	}

	{
		gpio_free(g_gpio_state->GPIO03_P04);
		gpio_free(g_gpio_state->GPIO03_P05);

		gpio_free(g_gpio_state->GPIO03_P00);
		gpio_free(g_gpio_state->GPIO03_P01);
	}

	printk(KERN_INFO "opaleye_gpio_main done");

	return 0;
}
module_init(opaleye_gpio_init);
module_exit(opaleye_gpio_exit);

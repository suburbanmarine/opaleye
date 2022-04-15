// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
// Copyright (C) 2021 Suburban Marine, Inc.

#include "opaleye_gpio_main.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>

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

	struct csi_gpio[4];

	struct hdr_gpio[4];

	class* cls;
	dev_t  dev;
} opaleye_gpio_state_t;

struct file_operations ioctl_d_interface_fops = {
	.owner = THIS_MODULE,
	.read = NULL,
	.write = NULL,
	.open = ioctl_d_interface_open,
	.unlocked_ioctl = ioctl_d_interface_ioctl,
	.release = ioctl_d_interface_release
};

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
	state->csi_gpio[0].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 4);
	state->csi_gpio[0].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->csi_gpio[0].label = "opaleye_GPIO03_P04";
	
	// CAM1_PWDN cam1 GPIO0 - xavier GPIO03_P05
	state->csi_gpio[1].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 5);
	state->csi_gpio[1].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->csi_gpio[1].label = "opaleye_GPIO03_P05";
	
	// CAM0_MCLK cam0 GPIO1 - xavier GPIO03_P00
	state->csi_gpio[2].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 0);
	state->csi_gpio[2].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->csi_gpio[2].label = "opaleye_GPIO03_P00";
	
	// CAM1_MCLK cam1 GPIO1 - xavier GPIO03_P01
	state->csi_gpio[3].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_P, 1);
	state->csi_gpio[3].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->csi_gpio[3].label = "opaleye_GPIO03_P01";

	int ret = gpio_request_array(state->csi_gpio, sizeof(state->csi_gpio) / sizeof(state->csi_gpio[0]));
	if(ret != 0)
	{
		printk(KERN_ERR "gpio_request_one failed to start csi_gpio");
		kfree(state);
		state = NULL;
		return -1;
	}

	// get Ext40 gpios
	// GPIO 13 - xavier GPIO03_N01
	state->hdr_gpio[0].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_N, 1);
	state->hdr_gpio[0].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->hdr_gpio[0].label = "opaleye_GPIO03_N01";
	
	// GPIO 1 - xavier GPIO03_Q05
	state->hdr_gpio[1].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_Q, 5);
	state->hdr_gpio[1].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->hdr_gpio[1].label = "opaleye_GPIO03_Q05";
	
	// GPIO 11 - xavier GPIO03_Q06
	state->hdr_gpio[2].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_Q, 6);
	state->hdr_gpio[2].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->hdr_gpio[2].label = "opaleye_GPIO03_Q06";
	
	// GPIO 9 - xavier GPIO03_R00
	state->hdr_gpio[3].gpio  = get_tegra194_gpio(TEGRA194_BASE, TEGRA194_MAIN_GPIO_PORT_R, 0);
	state->hdr_gpio[3].flags = GPIOF_DIR_OUT | GPIOF_INIT_LOW;
	state->hdr_gpio[3].label = "opaleye_GPIO03_R00";

	ret = gpio_request_array(state->hdr_gpio, sizeof(state->hdr_gpio) / sizeof(state->hdr_gpio[0]));
	if(ret != 0)
	{
		printk(KERN_ERR "gpio_request_one failed to start hdr_gpio");
		kfree(state);
		state = NULL;
		return -1;
	}

	state->cls         = class_create(THIS_MODULE, "opaleye");
	struct device* dvs = device_create(state->cls, NULL, state->dev, NULL, "gpio");

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

		//stop gpio
		if(g_gpio_state)
		{
			gpio_free_array(state->csi_gpio, sizeof(state->csi_gpio) / sizeof(state->csi_gpio[0]));
			gpio_free_array(state->hdr_gpio, sizeof(state->hdr_gpio) / sizeof(state->hdr_gpio[0]));

			device_destroy(state->cls, state->dev);
			class_destroy(state->cls);

			//release mem
			kfree(g_gpio_state);
			g_gpio_state = NULL;
		}

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
	gpio_set_value(state->csi_gpio[0], 1);

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
	gpio_set_value(state->csi_gpio[0], 0);

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

	}

	printk(KERN_INFO "opaleye_gpio_main done");

	return 0;
}
module_init(opaleye_gpio_init);
module_exit(opaleye_gpio_exit);

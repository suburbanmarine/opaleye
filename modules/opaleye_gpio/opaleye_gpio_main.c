// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
// Copyright (C) 2021 Suburban Marine, Inc.

#include "opaleye_gpio_main.h"

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/printk.h>
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

	struct gpio csi_gpio[4];
	struct gpio hdr_gpio[4];

	bool enabled;
	ktime_t t0;
	ktime_t period;
	ktime_t width;

	struct kobject* opaleye_dir_kobj;
} opaleye_gpio_state_t;

static struct opaleye_gpio_state_t* g_gpio_state;
DEFINE_MUTEX(g_gpio_state_mutex);

static enum hrtimer_restart opaleye_gpio_on_timer_cb(struct hrtimer* t)
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

	hrtimer_forward_now(&state->gpio_on_timer, state->period);
	//or hrtimer_set_expires

	//do something, toggle a pin
	gpio_set_value(state->csi_gpio[0].gpio, 1);
	gpio_set_value(state->csi_gpio[1].gpio, 1);

	// return HRTIMER_NORESTART;
	return HRTIMER_RESTART;
}

static enum hrtimer_restart opaleye_gpio_off_timer_cb(struct hrtimer* t)
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

	hrtimer_forward_now(&state->gpio_off_timer, state->period); // 1s
	//or hrtimer_set_expires

	//do something, toggle a pin
	gpio_set_value(state->csi_gpio[0].gpio, 0);
	gpio_set_value(state->csi_gpio[1].gpio, 0);

	// return HRTIMER_NORESTART;
	return HRTIMER_RESTART;
}

// https://github.com/torvalds/linux/blob/master/include/dt-bindings/gpio/tegra194-gpio.h
static unsigned get_tegra194_gpio(int base, int port, int offset)
{
	return base + port*8 + offset;
}

static ssize_t opaleye_enable_attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "enabled: %d\n", (int)g_gpio_state->enabled);
}
static ssize_t opaleye_timer_settings_attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	mutex_lock(&g_gpio_state_mutex);
	struct timespec ts_t0     = ktime_to_timespec(g_gpio_state->t0);
	struct timespec ts_period = ktime_to_timespec(g_gpio_state->period);
	struct timespec ts_width  = ktime_to_timespec(g_gpio_state->width);
	mutex_unlock(&g_gpio_state_mutex);

	return sprintf(buf, "t0: %ld.%09ld\nperiod: %ld.%09ld\nwidth: %ld.%09ld\n",
		ts_t0.tv_sec,     ts_t0.tv_nsec,
		ts_period.tv_sec, ts_period.tv_nsec,
		ts_width.tv_sec,  ts_width.tv_nsec
		);
}

static ssize_t opaleye_enable_attr_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int val;
	int ret = sscanf(buf, "%d", &val);
	if(ret != 1)
	{
		printk(KERN_ERR "opaleye_enable_attr_store failed to parse");
	}
	else
	{
		unsigned int edge = 0;
		edge |= (g_gpio_state->enabled) ? (1U << 0) : (0U);
		edge |= (val)                   ? (1U << 1) : (0U);

		mutex_lock(&g_gpio_state_mutex);

		switch(edge)
		{
			case 1: // on-> off
			{
				hrtimer_cancel(&g_gpio_state->gpio_on_timer);
				hrtimer_cancel(&g_gpio_state->gpio_off_timer);
				break;
			}
			case 2: // off -> on
			{
				ktime_t kt_0_on;
				if(ktime_equal(g_gpio_state->t0, ktime_set(0, 0)))
				{
					//if t0 == 0, start in 1s at top of second, if less than 0.25 wait till next second
					struct timespec64 t_now;
					ktime_get_real_ts64(&t_now);

					kt_0_on = ktime_set(t_now.tv_sec + 1, 0);
					
					ktime_t diff = ktime_sub(kt_0_on, ktime_set(t_now.tv_sec, t_now.tv_nsec));
					if(ktime_to_ns(diff) < 250*1000*1000ULL)
					{
						kt_0_on = ktime_add(kt_0_on, ktime_set(1, 0));	
					}
				}
				else
				{
					kt_0_on = g_gpio_state->t0;
				}
				hrtimer_start(&g_gpio_state->gpio_on_timer, kt_0_on, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD

				ktime_t kt_0_off = ktime_add(kt_0_on, g_gpio_state->width);
				hrtimer_start(&g_gpio_state->gpio_off_timer, kt_0_off, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD
				break;
			}
			case 0:
			case 3:
			default:
			{
				break;
			}
		}

		g_gpio_state->enabled = val;
		mutex_unlock(&g_gpio_state_mutex);
	}

	return count;
}

static long int char_to_num(const char c)
{
	return c - '0';
}

static long int nsstr_to_ns(const char* buf)
{
	long int accum = 0;
	long int mul   = 100000000;

	char const * ptr = buf;
	while(*ptr)
	{
		accum += char_to_num(*ptr) * mul;
		mul = mul / 10;
		ptr++;
	}

	return accum;
}

static ssize_t opaleye_timer_settings_attr_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	// long int t0_sec;
	// long int t0_nsec;
	// long int period_sec;
	// long int period_nsec;
	// long int width_sec;
	// long int width_nsec;

	// int ret = sscanf(buf, "%ld.%ld,%ld.%ld,%ld.%ld",
	// 			&t0_sec,     &t0_nsec,
	// 			&period_sec, &period_nsec,
	// 			&width_sec,  &width_nsec
	// 	);

	long int t0_sec;
	char  t0_nsec_str[10];
	long int period_sec;
	char  period_nsec_str[10];
	long int width_sec;
	char width_nsec_str[10];

	int ret = sscanf(buf, "%ld.%9[0123456789],%ld.%9[0123456789],%ld.%9[0123456789]",
				&t0_sec,     t0_nsec_str,
				&period_sec, period_nsec_str,
				&width_sec,  width_nsec_str
		);


	if(ret != 6)
	{
		printk(KERN_ERR "opaleye_timer_settings_attr_store failed to parse");
	}
	else
	{
		const long int t0_nsec     = nsstr_to_ns(t0_nsec_str);
		const long int period_nsec = nsstr_to_ns(period_nsec_str);
		const long int width_nsec  = nsstr_to_ns(width_nsec_str);
		
		mutex_lock(&g_gpio_state_mutex);
		if(g_gpio_state->enabled)
		{
			printk(KERN_ERR "opaleye_timer_settings_attr_store failed - timer is enabled");
		}
		else
		{
			g_gpio_state->t0     = ktime_set(t0_sec,     t0_nsec);
			g_gpio_state->period = ktime_set(period_sec, period_nsec);
			g_gpio_state->width  = ktime_set(width_sec,  width_nsec);
		}
		mutex_unlock(&g_gpio_state_mutex);
	}

	return count;
}

static struct kobj_attribute opaleye_enable_attr         =__ATTR(enable, 0660, opaleye_enable_attr_show, opaleye_enable_attr_store);
static struct kobj_attribute opaleye_timer_settings_attr =__ATTR(timer_settings, 0660, opaleye_timer_settings_attr_show, opaleye_timer_settings_attr_store);

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

	state->opaleye_dir_kobj = kobject_create_and_add("opaleye", kernel_kobj);
	sysfs_create_file(state->opaleye_dir_kobj, &opaleye_enable_attr.attr);
	sysfs_create_file(state->opaleye_dir_kobj, &opaleye_timer_settings_attr.attr);

	state->t0     = ktime_set(0, 0);
	state->width  = ktime_set(0, 100*1000*1000);
	state->period = ktime_set(1, 0);

	//startup
	hrtimer_init(&state->gpio_on_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD
	state->gpio_on_timer.function = opaleye_gpio_on_timer_cb;

	hrtimer_init(&state->gpio_off_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS); // HRTIMER_MODE_ABS_HARD
	state->gpio_off_timer.function = opaleye_gpio_off_timer_cb;

	// or to pin to a cpu
	// kthread_create
	// kthread_bind
	// wake_up_process
	state->main_task_ptr = kthread_run(&opaleye_gpio_main, state, "opaleye_gpio_main");
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
			//release mem
			kfree(g_gpio_state);
			g_gpio_state = NULL;
		}

	}

	printk(KERN_INFO "opaleye_gpio done");
}

int opaleye_gpio_main(void* data)
{
	printk(KERN_INFO "opaleye_gpio_main starting");

	opaleye_gpio_state_t* state = (opaleye_gpio_state_t*) data;

	// set prio
	// struct sched_param param = {
	// 	.sched_priority = 50
	// };
	// sched_setscheduler(state->main_task_ptr, SCHED_RR, &param); // or SCHED_FIFO

	while( ! kthread_should_stop() )
	{
		msleep_interruptible(1000);
	}

	//tear down
	{
		//stop event cb
		hrtimer_cancel(&state->gpio_on_timer);
		hrtimer_cancel(&state->gpio_off_timer);
	}

	{
		gpio_free_array(state->csi_gpio, sizeof(state->csi_gpio) / sizeof(state->csi_gpio[0]));
		gpio_free_array(state->hdr_gpio, sizeof(state->hdr_gpio) / sizeof(state->hdr_gpio[0]));
	}

	{
		kobject_put(state->opaleye_dir_kobj);
	}

	printk(KERN_INFO "opaleye_gpio_main done");

	return 0;
}
module_init(opaleye_gpio_init);
module_exit(opaleye_gpio_exit);

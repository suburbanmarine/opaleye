// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/// Copyright (C) 2021 Suburban Marine, Inc.

#pragma once

int opaleye_gpio_init(void);
void opaleye_gpio_exit(void);

int opaleye_gpio_main(void* data);

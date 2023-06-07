/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
/* Copyright (c) 2023 Red Hat */

#ifndef __NOTIFIER_H
#define __NOTIFIER_H

struct notifier_event {
	unsigned long val;
	char mac[ETH_ALEN];
	__u8 name[IFNAMSIZ];
};

#endif /* __NOTIFIER_H */

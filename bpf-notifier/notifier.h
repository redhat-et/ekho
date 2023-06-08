/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
/* Copyright (c) 2023 Red Hat */

#ifndef __NOTIFIER_H
#define __NOTIFIER_H

enum notifier_event_type {
	N_SWITCHDEV,
	N_FIB
};

struct notifier_event {
	enum notifier_event_type type;
	int family;
	unsigned long val;
	char mac[ETH_ALEN];
	__u8 name[IFNAMSIZ];
	__u32 ipv4_dest;
	struct in6_addr ipv6_dest;
	int dest_len;
};

#endif /* __NOTIFIER_H */

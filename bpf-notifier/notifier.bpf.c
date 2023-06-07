// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2023 Red Hat */

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "notifier.h"

struct net_device {
	char name[IFNAMSIZ];
} __attribute__((preserve_access_index));

struct switchdev_notifier_info {
	struct net_device *dev;
	struct netlink_ext_ack *extack;
	const void *ctx;
} __attribute__((preserve_access_index));

struct switchdev_notifier_fdb_info {
	struct switchdev_notifier_info info;
	const unsigned char *addr;
	__u16 vid;
	__u8 added_by_user:1,
	     is_local:1,
	     locked:1,
	     offloaded:1;
} __attribute__((preserve_access_index));;

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 4096);
} notifier_events SEC(".maps");

SEC("fentry/call_switchdev_notifiers")
int BPF_PROG(switchdev_notifier,
	     unsigned long val,
	     struct net_device *dev,
	     struct switchdev_notifier_info *info)
{
	struct notifier_event *event =
		bpf_ringbuf_reserve(&notifier_events,
				    sizeof(struct notifier_event), 0);
	if (!event)
		return 0;

	event->val = val;

	struct switchdev_notifier_fdb_info* fdb =
		(struct switchdev_notifier_fdb_info*) info;
	const unsigned char *addr =
		BPF_CORE_READ(fdb, addr);
	bpf_probe_read_kernel(event->mac, ETH_ALEN, addr);

	bpf_probe_read_kernel(event->name, IFNAMSIZ, dev->name);

        bpf_ringbuf_submit(event, 0);
	return 0;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";

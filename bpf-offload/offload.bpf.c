// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2023 Red Hat */

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <linux/netdevice.h>

#include "offload.h"

char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct net_device {
	char name[IFNAMSIZ];
} __attribute__((preserve_access_index));

struct flow_stats {
	__u64	pkts;
	__u64	bytes;
	__u64	drops;
	__u64	lastused;
} __attribute__((preserve_access_index));

struct flow_cls_offload {
	struct flow_stats stats;
} __attribute__((preserve_access_index));

struct net_device_hw_ops {
	void (*offload)(struct net_device *dev,
			struct flow_cls_offload *off);
	char name[16];
};

SEC("struct_ops/offload")
void BPF_PROG(offload,
	      struct net_device *dev,
	      struct flow_cls_offload *off)
{
	bpf_printk("struct_ops/offload %s\n", dev->name);
	off->stats.pkts = 1;
	off->stats.bytes = 1;
	off->stats.lastused = bpf_ktime_get_ns();
}

SEC(".struct_ops.link")
struct net_device_hw_ops hwops = {
	.offload = (void *)offload,
	.name = "netdev_hwops",
};

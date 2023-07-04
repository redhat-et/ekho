// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2023 Red Hat */

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <linux/netdevice.h>

#include "netdev_hw.h"
#include "offload.h"

char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct net_device {
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


SEC("struct_ops/offload")
void BPF_PROG(offload,
	      struct net_device *dev,
	      struct flow_cls_offload *off)
{
	// bpf_printk("struct_ops/offload called\n");
}

SEC(".struct_ops")
struct net_device_hw_ops hwops = {
	.offload = (void *)offload,
	.name = "netdev_hwops",
};

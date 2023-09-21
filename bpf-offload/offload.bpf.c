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

enum flow_cls_command {
	FLOW_CLS_REPLACE,
	FLOW_CLS_DESTROY,
	FLOW_CLS_STATS,
	FLOW_CLS_TMPLT_CREATE,
	FLOW_CLS_TMPLT_DESTROY,
};

struct flow_cls_offload {
	enum flow_cls_command command;
	unsigned long cookie;
	struct flow_stats stats;
} __attribute__((preserve_access_index));

enum tc_setup_type {
	TC_QUERY_CAPS,
	TC_SETUP_QDISC_MQPRIO,
	TC_SETUP_CLSU32,
	TC_SETUP_CLSFLOWER,
	TC_SETUP_CLSMATCHALL,
	TC_SETUP_CLSBPF,
	TC_SETUP_BLOCK,
	TC_SETUP_QDISC_CBS,
	TC_SETUP_QDISC_RED,
	TC_SETUP_QDISC_PRIO,
	TC_SETUP_QDISC_MQ,
	TC_SETUP_QDISC_ETF,
	TC_SETUP_ROOT_QDISC,
	TC_SETUP_QDISC_GRED,
	TC_SETUP_QDISC_TAPRIO,
	TC_SETUP_FT,
	TC_SETUP_QDISC_ETS,
	TC_SETUP_QDISC_TBF,
	TC_SETUP_QDISC_FIFO,
	TC_SETUP_QDISC_HTB,
	TC_SETUP_ACT,
};

struct net_device_hw_ops {
	void (*offload)(struct net_device *dev,
			struct flow_cls_offload *off);
	int (*setup_tc)(struct net_device *dev,
			enum tc_setup_type type,
			void *type_data);
	int (*setup_ft)(enum tc_setup_type type,
			void *type_data,
			void *cb_priv);
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

SEC("struct_ops/setup_tc")
int BPF_PROG(setup_tc,
	     struct net_device *dev,
	     enum tc_setup_type type,
	     void *type_data)
{
	bpf_printk("struct_ops/setup_tc dev=%s, type=%d\n", dev->name, type);

	struct flow_block_offload *f = type_data;
	switch (type) {
	case TC_SETUP_FT:
		return 0;
	default:
		return -95; /* -EOPNOTSUPP */
	}
}

SEC("struct_ops/setup_ft")
int BPF_PROG(setup_ft,
	     enum tc_setup_type type,
	     struct flow_cls_offload *type_data,
	     void *cb_priv)
{
	struct flow_cls_offload *f = type_data;
	int command = BPF_CORE_READ(f, command);
	bpf_printk("struct_ops/setup_ft type=%d command=%d\n", type, command);

	switch (type) {
	case TC_SETUP_FT:
		return 0;
	default:
		return -95; /* -EOPNOTSUPP */
	}
}

SEC(".struct_ops.link")
struct net_device_hw_ops hwops = {
	.offload = (void *)offload,
	.setup_tc = (void *)setup_tc,
	.setup_ft = (void *)setup_ft,
	.name = "netdev_hwops",
};

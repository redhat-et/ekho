// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2023 Red Hat */

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/in6.h>

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


struct net {
	int ifindex;
} __attribute__((preserve_access_index));

enum fib_event_type {
	FIB_EVENT_ENTRY_REPLACE,
	FIB_EVENT_ENTRY_APPEND,
	FIB_EVENT_ENTRY_ADD,
	FIB_EVENT_ENTRY_DEL,
	FIB_EVENT_RULE_ADD,
	FIB_EVENT_RULE_DEL,
	FIB_EVENT_NH_ADD,
	FIB_EVENT_NH_DEL,
	FIB_EVENT_VIF_ADD,
	FIB_EVENT_VIF_DEL,
};

struct fib_notifier_info {
	int family;
} __attribute__((preserve_access_index));

struct fib_entry_notifier_info {
	struct fib_notifier_info info; /* must be first */
	__u32 dst;
	int dst_len;
} __attribute__((preserve_access_index));

struct rt6key {
	struct in6_addr addr;
	int plen;
} __attribute__((preserve_access_index));

struct fib6_info {
	struct rt6key fib6_dst;
} __attribute__((preserve_access_index));

struct fib6_entry_notifier_info {
	struct fib_notifier_info info; /* must be first */
	struct fib6_info *rt;
} __attribute__((preserve_access_index));

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

	event->type = N_SWITCHDEV;
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

SEC("fentry/call_fib_notifiers")
int BPF_PROG(fib_notifier,
	     struct net *net,
	     enum fib_event_type event_type,
	     struct fib_notifier_info *info)
{
	struct notifier_event *event =
		bpf_ringbuf_reserve(&notifier_events,
				    sizeof(struct notifier_event), 0);
	if (!event)
		return 0;

	event->type = N_FIB;
	event->val = event_type;
	event->family = BPF_CORE_READ(info, family);

	if (event->family == AF_INET) {
		struct fib_entry_notifier_info *fib =
			(struct fib_entry_notifier_info *)info;
		event->ipv4_dest = BPF_CORE_READ(fib, dst);
		event->dest_len = BPF_CORE_READ(fib, dst_len);
	} else if (event->family == AF_INET6) {
		struct fib6_entry_notifier_info *fib =
			(struct fib6_entry_notifier_info *)info;
		event->ipv6_dest = BPF_CORE_READ(fib, rt, fib6_dst.addr);
		event->dest_len = BPF_CORE_READ(fib, rt, fib6_dst.plen);
	}

	bpf_ringbuf_submit(event, 0);
	return 0;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";

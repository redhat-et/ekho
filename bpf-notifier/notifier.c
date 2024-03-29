// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2023 Red Hat */

#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <linux/if_ether.h>
#include <bpf/bpf.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <string.h>

#include "notifier.skel.h"
#include "notifier.h"

static struct env {
	int interval;
	bool verbose;
} env = { 1, 0 };

const char *argp_program_version = "notifier 0.0";
const char *argp_program_bug_address = "<donald.hunter@redhat.com>";
const char argp_program_doc[] =
	"BPF notifier prototype.\n"
	"\n"
	"USAGE: ./notifier [-v]\n";

static const struct argp_option opts[] = {
	{ "interval", 'i', "seconds", 0, "Interval between reports" },
	{ "verbose", 'v', NULL, 0, "Verbose debug output" },
	{},
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'i':
		env.interval = atoi(arg);
		break;
	case 'v':
		env.verbose = true;
		break;
	case ARGP_KEY_ARG:
		argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static const struct argp argp = {
	.options = opts,
	.parser = parse_arg,
	.doc = argp_program_doc,
};

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (level == LIBBPF_DEBUG && !env.verbose)
		return 0;
	return vfprintf(stderr, format, args);
}

static volatile bool exiting = false;

static void sig_handler(int sig)
{
	exiting = true;
}

static int process_event(void *ctx, void *data, size_t len)
{
	struct notifier_event *event = data;
	char prefix[40];

	switch (event->type) {
	case N_SWITCHDEV:
		printf("switchdev event id=%lu, mac=%s, name=%s\n",
		       event->val,
		       ether_ntoa((struct ether_addr*)event->mac),
		       event->name);
		break;
	case N_FIB:
		switch (event->family) {
		case AF_INET:
			inet_ntop(AF_INET, &event->ipv4_dest, prefix, 40);
			break;
		case AF_INET6:
			inet_ntop(AF_INET6, &event->ipv6_dest, prefix, 40);
			break;
		}
		printf("fib event id=%lu, dest=%s/%d\n",
		       event->val,
		       prefix, event->dest_len);
		break;
	}
	return 0;
}

int main(int argc, char **argv)
{
	struct notifier_bpf *skel;
	int err;

	/* Parse command line arguments */
	err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (err)
		return err;

	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

	/* Cleaner handling of Ctrl-C */
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	/* Load and verify BPF application */
	skel = notifier_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}

	/* Load & verify BPF programs */
	err = notifier_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}

	/* Attach tracepoints */
	err = notifier_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		goto cleanup;
	}

	struct ring_buffer* ringbuf =
		ring_buffer__new(bpf_map__fd(skel->maps.notifier_events), process_event, NULL, NULL);
	while (!exiting) {
		ring_buffer__poll(ringbuf, 100);
	}

cleanup:
	/* Clean up */
	notifier_bpf__destroy(skel);

	return err < 0 ? -err : 0;
}

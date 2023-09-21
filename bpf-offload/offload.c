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
#include <bpf/bpf.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "offload.skel.h"
#include "offload.h"

static struct env {
	int ifindex;
	bool verbose;
} env = { 1, 0 };

const char *argp_program_version = "offload 0.0";
const char *argp_program_bug_address = "<donald.hunter@redhat.com>";
const char argp_program_doc[] =
	"BPF netdev hw offload prototype.\n"
	"\n"
	"USAGE: ./offload [-v] [-i <ifindex>]\n";

static const struct argp_option opts[] = {
	{ "interface", 'i', "ifindex", 0, "Interface to attach" },
	{ "verbose", 'v', NULL, 0, "Verbose debug output" },
	{},
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'i':
		env.ifindex = atoi(arg);
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


int main(int argc, char **argv)
{
	struct offload_bpf *skel;
	struct bpf_link *link;
	char dir[100];
	char path[PATH_MAX];
	int err;

	/* Parse command line arguments */
	err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (err)
		return err;

	snprintf(dir, sizeof(dir), "/sys/fs/bpf/ekho-%d", env.ifindex);
	err = mkdir(dir, 0700);
	if (err) {
		fprintf(stderr, "Failed to create pin dir %s\n", dir);
		goto cleanup;
	}

	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

	/* Cleaner handling of Ctrl-C */
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	/* Load and verify BPF application */
	skel = offload_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}

	err = bpf_map__set_map_flags(skel->maps.hwops, BPF_F_LINK);
	if (err) {
		fprintf(stderr, "Failed to set map flags\n");
		goto cleanup;
	}

	/* Load & verify BPF programs */
	err = offload_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}
	snprintf(path, PATH_MAX, "%s/offload", dir);
	err = bpf_object__pin(skel->obj, path);
	if (err) {
		fprintf(stderr, "Failed to pin offload obj\n");
		goto cleanup;
	}

	err = bpf_map__set_ifindex(skel->maps.hwops, env.ifindex);
	if (err) {
		fprintf(stderr, "Failed to set ifindex\n");
		goto cleanup;
	}

	link = bpf_map__attach_struct_ops(skel->maps.hwops);
	if (!link) {
		perror("Failed to attach struct_ops");
		goto cleanup;
	}
	snprintf(path, sizeof(path), "%s/%s", dir, "link");
	bpf_link__pin(link, path);
	if (err) {
		fprintf(stderr, "Failed to pin link\n");
		goto cleanup;
	}

cleanup:
	/* Clean up */
	offload_bpf__destroy(skel);

	return err < 0 ? -err : 0;
}

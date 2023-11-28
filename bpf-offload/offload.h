/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
/* Copyright (c) 2023 Red Hat */

#ifndef __OFFLOAD_H
#define __OFFLOAD_H

enum flow_cls_command {
	FLOW_CLS_REPLACE,
	FLOW_CLS_DESTROY,
	FLOW_CLS_STATS,
	FLOW_CLS_TMPLT_CREATE,
	FLOW_CLS_TMPLT_DESTROY,
};

struct offload_event {
	enum flow_cls_command command;
};

#endif /* __OFFLOAD_H */

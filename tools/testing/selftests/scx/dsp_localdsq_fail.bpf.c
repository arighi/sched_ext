/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024 Meta Platforms, Inc. and affiliates.
 * Copyright (c) 2024 David Vernet <dvernet@meta.com>
 * Copyright (c) 2024 Tejun Heo <tj@kernel.org>
 */
#include <scx/common.bpf.h>

char _license[] SEC("license") = "GPL";

s32 BPF_STRUCT_OPS(dsp_localdsq_fail_select_cpu, struct task_struct *p,
		   s32 prev_cpu, u64 wake_flags)
{
	s32 cpu = scx_bpf_pick_idle_cpu(p->cpus_ptr, 0);

	if (cpu >= 0) {
		/* Shouldn't be allowed to vtime dispatch to a builtin DSQ. */
		scx_bpf_dispatch_vtime(p, SCX_DSQ_LOCAL, SCX_SLICE_DFL,
				       p->scx.dsq_vtime, 0);
		return cpu;
	}

	return prev_cpu;
}

s32 BPF_STRUCT_OPS(dsp_localdsq_fail_init)
{
	scx_bpf_switch_all();

	return 0;
}

SEC(".struct_ops.link")
struct sched_ext_ops dsp_localdsq_fail_ops = {
	.select_cpu		= dsp_localdsq_fail_select_cpu,
	.init			= dsp_localdsq_fail_init,
	.name			= "dsp_localdsq_fail",
	.timeout_ms		= 1000U,
};

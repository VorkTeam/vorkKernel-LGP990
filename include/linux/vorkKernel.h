#ifndef _LINUX_VORKKERNEL_H
#define _LINUX_VORKKERNEL_H

#ifdef __KERNEL__

#define USE_FAKE_SHMOO
//define larger_epeen
#define DISABLE_FAKE_SHMOO_UV

/* mm stuff */
#define dirty_background_ratio_default 2
#define vm_dirty_ratio_default 4
#define inactive_file_ratio_default 20

/* cfs stuff */
#define sysctl_sched_latency_default 6000000ULL
#define normalized_sysctl_sched_latency_default 6000000ULL
#define sysctl_sched_min_granularity_default 600000ULL
#define normalized_sysctl_sched_min_granularity_default 600000ULL
#define sched_nr_latency_default 10
#define sysctl_sched_wakeup_granularity_default 5000000UL
#define normalized_sysctl_sched_wakeup_granularity_default 5000000UL

/* Test some stuff */
#define CFS_BOOST
/* Possible Range: -20 19 */
#define CFS_BOOST_NICE -10

/* 
 * avp/system/emc/ddr oc 
 * We only need to set avp oc. The rest is calculable.
 * stock freqs are:
 * 240000 avp
 * 300000 ddr
 * 600000 emc
 * with a emc core voltage from 1200
 *
 * Calculation:
 * avp_freq * 2.5 = ddr_freq
 * ddr_oc * 2 = emc_freq
 */
#ifdef larger_epeen
#define avp_freq 240000
#else
#define avp_freq 264000
#endif
#define ddr_freq (avp_freq * 2.5) / 2
#define emc_freq (avp_freq * 2.5)
#define emc_voltage 1200

#define gpu_divider 6

#endif /* __KERNEL__ */

#endif




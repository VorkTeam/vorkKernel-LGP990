#ifndef _LINUX_VORKKERNEL_H
#define _LINUX_VORKKERNEL_H

#ifdef __KERNEL__

#define USE_FAKE_SHMOO
#define DISABLE_FAKE_SHMOO_UV

/* mm stuff */
#define dirty_background_ratio_default 2
#define vm_dirty_ratio_default 4
#define inactive_file_ratio_default 20

/* cfs stuff */
#define sysctl_sched_latency_default 3200000ULL
#define normalized_sysctl_sched_latency_default  3200000ULL
#define sysctl_sched_min_granularity_default 400000ULL
#define normalized_sysctl_sched_min_granularity_default 400000ULL
#define sched_nr_latency_default 8
#define sysctl_sched_wakeup_granularity_default 25000UL
#define normalized_sysctl_sched_wakeup_granularity_default 25000UL

/* Test some stuff */
#define CFS_BOOST
/* Possible Range: -20 19 */
#define CFS_BOOST_NICE -10


#endif /* __KERNEL__ */

#endif




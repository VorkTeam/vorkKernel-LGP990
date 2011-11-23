#ifndef _LINUX_VORKKERNEL_H
#define _LINUX_VORKKERNEL_H

#ifdef __KERNEL__

#define USE_FAKE_SHMOO
//define larger_epeen

#ifdef larger_epeen
#define DISABLE_FAKE_SHMOO_UV
#endif

// mm stuff
#define dirty_background_ratio_default 10
#define vm_dirty_ratio_default 10
#define inactive_file_ratio_default 20
#define vfs_cache_pressure 25

// cfs stuff
#define sysctl_sched_latency_default 6000000ULL
#define normalized_sysctl_sched_latency_default 6000000ULL
#define sysctl_sched_min_granularity_default 750000ULL
#define normalized_sysctl_sched_min_granularity_default 750000ULL
#define sched_nr_latency_default 8
#define sysctl_sched_wakeup_granularity_default 1000000UL
#define normalized_sysctl_sched_wakeup_granularity_default 1000000UL

// Test some stuff 
#define CFS_BOOST
// Possible Range: -20 19
#define CFS_BOOST_NICE -15


// AVP/SYSTEM/GPU/VDE OC related stuff

#define VORK_AVP_FREQ 280000
#define VORK_VDE_FREQ VORK_AVP_FREQ
#define VORK_SYSTEM_FREQ 280000
#define VORK_EMC_VOLT 1200
#define VORK_GPU_FREQ 366666

#define VORK_EMC2_FREQ ( 252000 * 5 / 2 )
#define VORK_DDR_FREQ ( VORK_EMC2_FREQ / 2 )

#define VORK_GPU_DIVIDER (7)
#define VORK_PLLA0 11289
#define VORK_PLLX0 1000000

#endif /* __KERNEL__ */

#endif

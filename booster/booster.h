#ifndef __BOOSTER_H__
#define __BOOSTER_H__

#define PROP_VALUE_MAX 70

typedef enum {
	POWER_MODE_BATTERYSAVE,
	POWER_MODE_BALANCED,
	POWER_MODE_PERFORMANCE,
	POWER_MODE_OMXDECODE,
	POWER_MODE_OMXENCODE,
	/* Do not use this entry */
	POWER_MODE_MAX,
} rqb_pwr_mode_t;

struct rqbalance_params {
	char min_cpus[2];
	char max_cpus[2];
	char up_thresholds[PROP_VALUE_MAX];
	char down_thresholds[PROP_VALUE_MAX];
	char balance_level[PROP_VALUE_MAX];
};

#endif

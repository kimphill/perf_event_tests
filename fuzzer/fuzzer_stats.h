#define MAX_ERRNOS 1023
#define MAX_OPEN_TYPE 16
#define MAX_IOCTL_TYPE 16

struct fuzzer_stats_t {
	long long total_iterations;
	long long overflows;
	long long sigios;
	long long current_open;
	long long open_attempts,open_successful;
	long long close_attempts,close_successful;
	long long mmap_attempts,mmap_successful;
	long long read_attempts,read_successful;
	long long write_attempts,write_successful;
	long long ioctl_attempts,ioctl_successful;
	long long prctl_attempts,prctl_successful;
	long long fork_attempts,fork_successful;
	long long poll_attempts,poll_successful;
	long long access_attempts,access_successful;
	long long trash_mmap_attempts,trash_mmap_successful;
	int errno_count[MAX_ERRNOS];
	int open_type_success[MAX_OPEN_TYPE];
	int open_type_fail[MAX_OPEN_TYPE];
	int ioctl_type_success[MAX_IOCTL_TYPE];
	int ioctl_type_fail[MAX_IOCTL_TYPE];
};

extern struct fuzzer_stats_t stats;

void dump_summary(FILE *fff, int print_values);
void stats_set_pmu_name(int which, char *name);

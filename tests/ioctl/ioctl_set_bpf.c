/* ioctl_set_bpf.c  */

/* Tests the PERF_EVENT_IOC_SET_BPF functionality */

/* by Vince Weaver   vincent.weaver@maine.edu */

#define _GNU_SOURCE 1

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include "bpf.h"

#include "libbpf.h"

#include <linux/version.h>

#include <sys/utsname.h>

#include "bpf_helpers.h"
#include "tracefs_helpers.h"

static int quiet;

#define MAX_FILTER 8192
char filter[MAX_FILTER];


int main(int argc, char** argv) {

	int fd,bpf_fd;
	struct perf_event_attr pe1;
	int errors=0;

	union bpf_attr battr;

	int result;

	char test_string[]="Testing PERF_EVENT_IOC_SET_BPF ioctl...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing PERF_EVENT_IOC_SET_BPF ioctl.\n");
	}

	/*******************************************/
	/* Creating a kprobe tracepoint event      */
	/*******************************************/

	if (!quiet) {
		printf("Creating a kprobe tracepoint event\n");
	}

	FILE *fff;
	int kprobe_id;

	char filename[BUFSIZ];
	char tracefs_location[BUFSIZ];
	char *find_result;

	find_result=find_tracefs_location(tracefs_location,quiet);

	if (find_result==NULL) {

		if (!quiet) {
			fprintf(stderr,"Error finding tracefs location!\n");
		}

		test_skip(test_string);
	}

	sprintf(filename,"%s/kprobe_events",tracefs_location);

	fff=fopen(filename, "w");
	if (fff==NULL) {
		printf("Cannot open %s!\n",filename);
		test_fail(test_string);
	}

	/*  perf probe -a VMW=handle_mm_fault */
	fprintf(fff,"p:probe/VMW _text+1664624");
	fclose(fff);

	sprintf(filename,"%s/events/probe/VMW/id",tracefs_location);

	fff=fopen(filename, "r");
	if (fff==NULL) {
		printf("Cannot open %s!\n",filename);
		test_fail(test_string);
	}

	fscanf(fff,"%d",&kprobe_id);
	fclose(fff);


	memset(&pe1,0,sizeof(struct perf_event_attr));
	pe1.type=PERF_TYPE_TRACEPOINT;
	pe1.size=sizeof(struct perf_event_attr);

	pe1.config=kprobe_id;

	pe1.disabled=1;
	pe1.exclude_kernel=0;
	pe1.exclude_hv=0;
	arch_adjust_domain(&pe1,quiet);

	/* Create group leader */
	fd=perf_event_open(&pe1,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Unexpected error %s\n",strerror(errno));
		}
		printf("Cannot open kprobe id %d\n",kprobe_id);
		test_fail(test_string);
	}

	struct bpf_insn instructions[] = {
		BPF_MOV64_IMM(BPF_REG_0, 0),		/* r0 = 0 */
		BPF_EXIT_INSN(),			/* return r0 */
	};

	unsigned char license[]="GPL";

#define LOG_BUF_SIZE 65536
	static char bpf_log_buf[LOG_BUF_SIZE];

	/* Kernel will EINVAL if unused bits aren't zero */
	memset(&battr,0,sizeof(battr));

	/* Version has to match currently running kernel */

	struct utsname version;
	int major, minor, subminor, version_code;

	uname(&version);

	printf("We are running release %s\n",version.release);


	sscanf(version.release,"%d.%d.%d",&major,&minor,&subminor);

	version_code = (major<<16) | (minor<<8) | subminor;
	printf("Using LINUX_VERSION_CODE: %d\n",
		version_code);

	battr.prog_type = BPF_PROG_TYPE_KPROBE;
//	battr.insn_cnt = sizeof(instructions);
	battr.insn_cnt= sizeof(instructions) / sizeof(struct bpf_insn);
	battr.insns = (uint64_t) (unsigned long) instructions;
	battr.license = (uint64_t) (unsigned long) license;
	battr.log_buf = (uint64_t) (unsigned long) bpf_log_buf;
	battr.log_size = LOG_BUF_SIZE;
	battr.log_level = 1;
	battr.kern_version = version_code;

	bpf_log_buf[0] = 0;

	bpf_fd = sys_bpf(BPF_PROG_LOAD, &battr, sizeof(battr));

	if (bpf_fd < 0) {
		printf("bpf: load: failed to load program, %s\n"
			"-- BEGIN DUMP LOG ---\n%s\n-- END LOG --\n",
			strerror(errno), bpf_log_buf);
			return bpf_fd;
	}


	result=ioctl(fd, PERF_EVENT_IOC_SET_BPF, bpf_fd);
	if (result<0) {
		if (!quiet) printf("Unexpected %d %s\n",result,strerror(errno));
		errors++;
	}

	/* start */
	ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	/* million */
	result=instructions_million();

	/* stop */
	ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	close(fd);

	if (errors) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}


/*
* this app is essentially the same as original nvme_chk app other than
* after running tests specified as command line parameters,
* user is prompted for new tests command or quit
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>

/* project specific includes */
#include "definitions.h"

/* timeout definition in sec */
#define TEST_TIMEOUT	300
/* pause in sec between test status query commands */
#define STEP		1

/* gets/fgets string size */
#define BUFFERSIZE		120

/* MAX TEST NUMBER PASSED TO DRIVER, DRIVER MAY/WILL/SHOULD FAIL MUCH LOWER */
#define TEST_MAX 		20

/* file handle to nvme driver */
int fd = 0;

void help(int val)
{
	LOG_NORMAL("[Nvme_App]\n");
	LOG_NORMAL("  -h print this help\n");
	LOG_NORMAL("  -g <group number>           \n");
	LOG_NORMAL("     0 - execute all tests from all groups\n");
	LOG_NORMAL("     1 - MMIO Tests ('-M')\n");
	LOG_NORMAL("     2 - Admin Commands Tests ('-A')\n");
	LOG_NORMAL("  -t <test number>                  \n");
	LOG_NORMAL("     0 - execute all tests from chosen group\n");
	LOG_NORMAL("     MMIO specific test numbers       \n");
	LOG_NORMAL("     1 - Read and validate capabilities register\n");
	LOG_NORMAL("     2 - Read and validate version register\n");
	LOG_NORMAL("     3 - Read and validate interrupt mask set register\n");
	LOG_NORMAL("     4 - Read and validate interrupt mask clear register\n");
	LOG_NORMAL("     5 - Read and validate configuration register\n");
	LOG_NORMAL("     6 - Read and validate status capabilities\n");
	LOG_NORMAL("     7 - Read and validate admin queue attributes register\n");
	LOG_NORMAL("     8 - Read and validate admin submission base register\n");
	LOG_NORMAL("     9 - Read and validate admin completion base register\n");
	LOG_NORMAL("     Admin cmd specific test numbers    \n");
	LOG_NORMAL("     1 - Check the device Identify Command\n");
	LOG_NORMAL("     2 - Create CQ with existing queue ID\n");
	LOG_NORMAL("     3 - Delete CQ with non existing queue ID\n");
	LOG_NORMAL("     4 - Request data related to non existing feature\n");
	LOG_NORMAL("     5 - Check data for Set/Get feature commands\n");
	LOG_NORMAL("     6 - Create more CQes than supported limit\n");
	LOG_NORMAL("     7 - Abort with wrong command ID and wrong SQ ID\n");
	LOG_NORMAL("  -Mi check register compliance to defined reset values\n");
	LOG_NORMAL("     [may give incorrect result unless executed immediatly following boot]\n");
	LOG_NORMAL("  -r reset device\n");
	LOG_NORMAL("  -s stop on first error\n");
	LOG_NORMAL("  -v be verbose and display detailed information\n");
	LOG_NORMAL("\n  [following commands are only valid as command line parameters]\n\n");
	LOG_NORMAL("  -l number of test loops \n");
	LOG_NORMAL("  -d device selection - format <bus>:<slot>.<function>\n");

	if (val == STATUS_OK)
		return;
	if (fd)
		close(fd);
	exit(val);
}

int run_test(struct test_sel *test_sel, int fd, int timeout)
{
	int res = STATUS_OK;
	struct test_status test_status;

	LOG_DEBUG("[Nvme_App]Running test %d.%d\n", test_sel->test_grp,
		test_sel->test_num);

	res = ioctl(fd, TEST_IOC_START, test_sel);

	LOG_NORMAL("[Nvme_App]");

	while (timeout-- > 0) {
		test_status.state = STATUS_INIT_FAILED;
		res = ioctl(fd, TEST_IOC_GET_STATUS, &test_status);
		if (res == 0) {
			if (test_status.state == STATUS_IN_PROGRESS)
				LOG_NORMAL(" .");
			else {
				LOG_DEBUG("\n[Nvme_App]Test status:%d",
					test_status.state);
				break;
			}
			sleep(STEP);
		} else {
			LOG_ERROR("\n[Nvme_App]Test status IOCTL failed\n");
			return STATUS_IOCTL_FAILED;
		}
	}
	LOG_NORMAL("\n");

	if (timeout == 0) {
		LOG_ERROR("[Nvme_App]Test timeouted\n");
		return STATUS_TIMEOUT;
	}

	switch (test_status.state) {
	case  STATUS_OK:
		LOG_NORMAL("[Nvme_App]Test PASSED\n");
		/* FIXME query results */
		break;
	case STATUS_NOK:
		LOG_ERROR("[Nvme_App]Test FAILED\n");
		res = STATUS_NOK;
		break;

	case STATUS_INIT_FAILED:
		LOG_ERROR("[Nvme_App]Test INIT FAILED\n");
		res = STATUS_INIT_FAILED;
		break;
	case STATUS_TEST_NOT_DEFINED:
		LOG_ERROR("[Nvme_App]Test UNIMPLEMENTED TEST FAILURE\n");
		res = STATUS_TEST_NOT_DEFINED;
		break;
	default:
		LOG_ERROR("[Nvme_App]Test failed with status %d\n", test_status.state);
		res = test_status.state;
		break;
	}

	return res;
}

int run_reset(int fd)
{
	int res = STATUS_OK;
	struct test_status test_status;

	res = ioctl(fd, TEST_IOC_RESET, &test_status);

	if (res != 0) {
		LOG_ERROR("\n[Nvme_App]Test status IOCTL failed\n");
		return STATUS_IOCTL_FAILED;
	}
	LOG_DEBUG("\n[Nvme_App]Reset status:%d", test_status.state);
	return test_status.state;
}

int str2int(char *str, int *value)
{
	unsigned i;

	for (i = 0; i < strlen(str); i++) {
		if (isdigit(str[i]) == 0)
			return -1;
	}
	*value = atoi(str);
	return 0;
}

int main(int argc, char *argv[])
{
	int n;
	int i;
	int j;
	int res;
	struct dev_data devices[DEVICE_LIST_SIZE];
	struct dev_data test_dev;
	struct test_sel test_sel;
	int test_grp;
	int test_grp_start;
	int test_grp_end;
	int test_num;
	int test_num_start;
	int test_num_end;
	int c;
	int timeout;
	int arg;
	int sflg = 0;
	int dflg = 0;
	int rflg = 0;
	int loop = 1;
	int qflg = 0;
	int Adminflg = 0;
	int MMIOflg = 0;
	int MMIOinitialreadflg = 0;
	/* verbose flag, currently unused - all possible prints print */	
	int vflg = 0;

	char my_str[BUFFERSIZE];
	char my_arg1[20];
	char my_arg2[20];
	char my_arg3[20];
	char my_arg4[20];
	char my_arg5[20];
	char my_arg6[20];

	int my_argc;
	char *my_argv[7] = {argv[0],my_arg1,my_arg2,my_arg3,
				my_arg4,my_arg5,my_arg6};
	/* set defaults */
	test_grp = TEST_ALL;
	test_num = TEST_ALL;

	LOG_DEBUG("[Nvme_App]Application start\n");
	/* TO DO : would be nice to put this while loop within separate 
	 * function (as it's almost identical to loop below)
         * but then would need to either pass in all these dfag, rflag,sflag, etc or 
         * make them global or get rid of some */
	/* BUG/Weirdness when specifying optional args to getopt(,,"..::..") using '::'
	 * if specifying optional arg, there MUST NOT be space between command and arg 
	 * (ie '-M4' - OK '-M 4' - NOT OK; 
	 * however with required arg ':' '-t4' - OK AND '-t 4' - ALSO OK) */
	while ((c = getopt(argc, argv, "hg:t:d:rsvlA::M::")) != -1) {

		switch (c) {
		case 'r':
			rflg = 1;
			break;
		case 'd':
			dflg = 1;
			sscanf(optarg, "%x:%x.%x", &test_dev.bus,
				&test_dev.slot, &test_dev.func);
			break;
		case 'g':
			if (str2int(optarg, &arg) == 0) {
				if ((arg  >= TEST_ALL) ||
					(arg  <= TEST_CMD_SET)) {
					test_grp = arg;
					break;
				}
			}
			LOG_ERROR("[Nvme_App]Wrong test group selection\n");
			return STATUS_WRONG_ARG;
		case 't':
			if (str2int(optarg, &arg) == 0) {
				test_num = arg;
				break;
			}
			LOG_ERROR("[Nvme_App]Wrong test number selection\n");
			return STATUS_WRONG_ARG;
		case 'l':
			if (str2int(optarg, &arg) == 0) {
				loop = arg;
				break;
			}
			LOG_ERROR("[Nvme_App]Wrong number of test iterations\n");
			return STATUS_WRONG_ARG;
		case 's':
			sflg = 1;
			break;
		case 'v':
			vflg = 1;
			break;
		case 'A':
			Adminflg = 1;
			/* -A is intended to take optional arg  see "..A::.." above */
			/*  '-A4' on command line will correctly select test #4 */
			/*  '-A 4' on command line will NOT! */
			if (optarg && ((str2int(optarg, &arg) == 0)))
				test_num = arg;
			break;
		case 'M':
			MMIOflg = 1;
			if (optarg) {
				/* check for initial read request */
				if (*optarg == 'i') {
					MMIOinitialreadflg = 1;
					test_num = 0;
				}
				else if (str2int(optarg, &arg) == 0)
					test_num = arg;
			}
			break;
		case '?':
			LOG_ERROR("[Nvme_App]Unrecognized option: -%c\n", optopt);
			help(STATUS_WRONG_ARG);
			break;
		case 'h':
			help(STATUS_NOK);
			break;
		default:
			help(STATUS_WRONG_ARG);
			break;
		}
	}

	/* remove node and recreate later */
	remove(NVME_NODE_FILE);

	/* create node for kernel-user space communication */
	mknod(NVME_NODE_FILE, S_IFCHR | S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP,
		makedev(TEST_MAJOR, 0));

	fd = open(NVME_NODE_FILE, O_RDWR);
	if (fd < 0) {
		LOG_ERROR("[Nvme_App]Can't open node file: %s, errno: %d\n",
			NVME_NODE_FILE, errno);
		return STATUS_NOK;
	}

	/* device selection procedure */
	if (dflg == 0) {
		/* Send ioctl to get NVMe type of device list */
		res = ioctl(fd, TEST_IOC_GET_DEVICE_LIST, devices);

		if (res > 1) {
			LOG_NORMAL("[Nvme_App]Select device [b:s.f]:\n");
			for (i = 0; i < res; i++) {
				LOG_NORMAL("%d. [%02x:%02x.%02x]\n", i,
					devices[i].bus, devices[i].slot,
					devices[i].func);
			}
			do {
				scanf("%d", &i);
			} while (i >= res);
		} else if (res == 1) {
			/* Only one device found, take first from the list */
			i = 0;
		} else if (res == 0) {
			LOG_ERROR("[Nvme_App]NVMe device not found\n");
			close(fd);
			return STATUS_NOK;
		} else {
			LOG_ERROR("[Nvme_App]Failed to get device list\n");
			close(fd);
			return STATUS_NOK;
		}

		test_dev = devices[i];
	} else {
		LOG_DEBUG("[Nvme_App]CLI selected device: [%02x:%02x.%02x]\n",
			test_dev.bus, test_dev.slot, test_dev.func);
	}

	/*  Send ioctl to select device */
	res = ioctl(fd, TEST_IOC_SELECT, &test_dev);
	if (res) {
		LOG_ERROR("[Nvme_App]Can't get device context\n");
		close(fd);
		return STATUS_NOK;
	}
	LOG_DEBUG("[Nvme_App]Selected device: [%02x:%02x.%02x]\n", test_dev.bus,
		test_dev.slot, test_dev.func);

	/*  loop beginning here */
	while (qflg != 1) {

		if (rflg && run_reset(fd)) {
			LOG_ERROR("[Nvme_App]Can't reset device\n");
			close(fd);
			return STATUS_NOK;
		}
		/* test group selection */
		/* override group if using one of these new flags */
		if (Adminflg) {
			test_grp = TEST_CMD_SET;
			test_grp_start = TEST_CMD_SET;
			test_grp_end = TEST_CMD_SET;
		}
		else if (MMIOflg) {
			test_grp = TEST_REG_MAP;
			test_grp_start = TEST_REG_MAP;
			test_grp_end = TEST_REG_MAP;
		}
		else if (test_grp == TEST_ALL) {
			/* individual test selection */
			if (test_num != TEST_ALL) {
				LOG_ERROR("[Nvme_App]Can't run individual test for each group\n");
				close(fd);
				return STATUS_WRONG_ARG;
			}
			test_grp_start = TEST_ALL + 1;
			test_grp_end = TEST_GRP_NUM - 1;
		} else if (test_grp >= TEST_GRP_NUM) {
			LOG_ERROR("[Nvme_App]Wrong group selection\n");
			return STATUS_WRONG_ARG;
		} else {
			test_grp_start = test_grp;
			test_grp_end = test_grp;
		}
		if (test_num == TEST_ALL) {
			test_num_start = TEST_ALL + 1;
			/*  break when we get STATUS_TEST_NOT_DEFINED */
			test_num_end = TEST_MAX;
		} else {
			test_num_start = test_num;
			test_num_end = test_num;
		}

		/* main test execution loop */
		n = 0;
		while (n++ < loop) {
			for (i = test_grp_start; i <= test_grp_end; i++) {
				test_sel.test_grp = i;
				/* TODO distingush MMIO initial value tests from RW test */
				/*test_sel.flags = TEST_MMIO_INITAL_VALUE | TEST_MMIO_RW;*/
				if (test_sel.test_grp == TEST_REG_MAP)
					test_sel.flags = MMIOinitialreadflg ? 
						TEST_MMIO_INITAL_VALUE : TEST_MMIO_RW;
				else
					test_sel.flags = 0;				
				for (j = test_num_start; j <= test_num_end; j++) {
					test_sel.test_num = j;
					timeout = TEST_TIMEOUT;
					res = run_test(&test_sel, fd, timeout);
					if (res == STATUS_TEST_NOT_DEFINED) {
						LOG_ERROR("[Nvme_App]Test %d.%d undefined\n", i, j);
						res = STATUS_OK;
						break;
					}
					LOG_ERROR("[Nvme_App]Test %d.%d %s\n", i, j,
						(res == STATUS_OK) ? "passed" :
						"failed");
					if (res && sflg) {
						LOG_ERROR("[Nvme_App]Test execution stopped on "
							"first error\n");
						close(fd);
						return res;
					}
				}
			}
		}
		/*  loop end here */
get_user_input:
		/* my_argv[0] remains prog name ie  my_argv[0] = argv[0]*/
		/*assume all strings are declared identical size as my_arg1*/
		for (i=1;i<7;i++)
			memset(my_argv[i],0,sizeof(my_arg1));
		vflg = 0;
		sflg = 0;
		rflg = 0;
		Adminflg = 0;
		MMIOflg = 0;
		MMIOinitialreadflg = 0;
		loop = 1;

		LOG_NORMAL("[Nvme_App]Enter Next Tests (or '-q'):\n >\t");

		fgets(my_str, sizeof(my_str), stdin);

		my_argc = sscanf(my_str, "%s %s %s %s %s %s", 
				my_argv[1], my_argv[2], my_argv[3], 
				my_argv[4], my_argv[5], my_argv[6]);
		if (my_argc < 1) {
			/* [enter] causes help to print */
			help(STATUS_OK);
			goto get_user_input;
		}
		/* assume user wants quit in this case too (use left out '-' from '-q') */
		if ((my_argc == 1) &&  (*my_argv[1] == 'q'))
			break;
		my_argc++;
		/* reset optind to 1 prior to getopt call*/
		optind = 1;
		while ((c = getopt(my_argc, my_argv, "g:t:qrsvhA::M::")) != -1) {

			switch (c) {
			case 'q':
				qflg = 1;	
				break;
			case 'r':
				rflg = 1;
				break;
			case 'g':
				if (str2int(optarg, &arg) == 0) {
					if ((arg  >= TEST_ALL) ||
						(arg  <= TEST_CMD_SET)) {
						test_grp = arg;
						break;
					}
				}
				LOG_ERROR("[Nvme_App]Wrong test group selection: restart app\n");
				qflg = 1;	
				break;
			case 't':
				if (str2int(optarg, &arg) == 0) {
					test_num = arg;
					break;
				}
				LOG_ERROR("[Nvme_App]Wrong test number selection: restart app\n");
				qflg = 1;	
				break;
/*			No device select in interactive mode			
			case 'd':
				. . . .
				break;
*/
/*			No looping in interactive mode
			case 'l':
				. . . .
				break;
*/
			case 's':
				sflg = 1;
				break;
			case 'v':
				vflg = 1;
				break;
			case 'h':
				qflg = 0;
				help(STATUS_OK);
				goto get_user_input;
				break;
			case 'A':
				Adminflg = 1;
				if (optarg && ((str2int(optarg, &arg) == 0)))
					test_num = arg;
				break;
			case 'M':
				MMIOflg = 1;
				if (optarg) {
					/* check for initial read request */
					if (*optarg == 'i') {
						MMIOinitialreadflg = 1;
						test_num = 0;
					}
					else if (str2int(optarg, &arg) == 0)
						test_num = arg;
				}
				break;
			case '?':
				qflg = 1;
				LOG_ERROR("[Nvme_App]Unrecognized option: -%c: restart app\n", optopt);
				break;
			default:
				qflg = 1;
				help(STATUS_NOK);
				break;
			}
		}

	}

	close(fd);
	LOG_DEBUG("[Nvme_App]Application End\n");
	return res;
}

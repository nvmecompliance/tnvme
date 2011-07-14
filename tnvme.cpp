#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include "tnvme.h"

#include "GrpCtrlRegisters/grpCtrlRegisters.h"

void Usage(void);
bool ParseTargetCmdLine(TargetType &target, char *optarg);
void DestroyTestInfrastructure(vector<Group *> &groups, int &fd);
bool BuildTestInfrastructure(vector<Group *> &groups, int &fd, string device,
    SpecRevType specRev);
bool ExecuteTests(TargetType test, size_t loop, bool ignore,
    vector<Group *> &groups);


void
Usage(void) {
    //80->  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    printf("%s\n", APPNAME);
    printf("  -h(--help)                          Display this help\n");
    printf("  -v(--rev) <spec>                    All options forced to target specified\n");
    printf("                                      NVME revision {1.0 | 1.0a}; dflt=1.0a\n");
    printf("  -s(--summary)                       Summarize all groups and tests\n");
    printf("  -e(--detail) [<grp> | <grp>:<test>] Detailed group and test description for:\n");
    printf("                                      {all | spec'd_group | test_within_group}\n");
    printf("  -t(--test) [<grp> | <grp>:<test>]   Execute tests for:\n");
    printf("                                      {all | spec'd_group | test_within_group}\n");
    printf("  -d(--device) <name>                 Device to open for testing: /dev/node\n");
    printf("                                      dflt=(1st device listed in --list)\n");
    printf("  -r(--reset) {<pci> | <ctrlr>}       Reset the device\n");
    printf("  -i(--ignore)                        Ignore detected errors\n");
    printf("  -p(--loop) <count>                  Loop test execution <count> times; dflt=1\n");
    printf("  -l(--list)                          List all devices available for test\n");
}


int
main(int argc, char *argv[])
{
    int c;
    int fd = -1;
    int idx = 0;
    int exitCode = 0;
    char *endptr;
    string work;
    vector<Group *> groups;
    vector<string> devices;
    struct CmdLineType CmdLine;
    struct dirent *dirEntry;
    bool deviceFound = false;
    const char *short_opt = "hsliv:e::r:p:t::d:";
    static struct option long_opt[] = {
        // {name,       has_arg,            flag,   val}
        {   "help",     no_argument,        NULL,   'h'},
        {   "summary",  no_argument,        NULL,   's'},
        {   "rev",      required_argument,  NULL,   'v'},
        {   "detail",   optional_argument,  NULL,   'e'},
        {   "test",     optional_argument,  NULL,   't'},
        {   "device",   required_argument,  NULL,   'd'},
        {   "reset",    required_argument,  NULL,   'r'},
        {   "ignore",   no_argument,        NULL,   'i'},
        {   "loop",     required_argument,  NULL,   'p'},
        {   "list",     no_argument,        NULL,   'l'},
        {   NULL,       no_argument,        NULL,    0}
    };

    // defaults if not spec'd on cmd line
    CmdLine.rev = SPECREV_10a;
    CmdLine.detail.req = false;
    CmdLine.test.req = false;
    CmdLine.reset = RESETTYPE_FENCE;
    CmdLine.summary = false;
    CmdLine.ignore = false;
    CmdLine.loop = 1;
    CmdLine.device = "none found";


    // Seek for all possible devices that this app may commune
    DIR *devDir = opendir("/dev");
    if (devDir == NULL) {
        printf("Unable to open system /dev directory\n");
        exit(1);
    }
    while ((dirEntry = readdir(devDir)) != NULL) {
        work = dirEntry->d_name;
        if (work.find("qnvme") != string::npos)
            devices.push_back("/dev/" + work);
    }
    if (devices.size())
        CmdLine.device = devices[0];    // Default to 1st element listed


    // Parse cmd line options
    if (argc == 1) {
        printf("%s is a compliance test suite for NVM Express hardware.\n",
            APPNAME);
        exit(0);
    }
    while ((c = getopt_long(argc, argv, short_opt, long_opt, &idx)) != -1) {
        switch (c) {

        case 'v':
            if (strcmp("1.0", optarg) == 0) {
                CmdLine.rev = SPECREV_10;
            } else if (strcmp("1.0a", optarg) == 0) {
                CmdLine.rev = SPECREV_10a;
            }
            break;

        case 'e':
            if (ParseTargetCmdLine(CmdLine.detail, optarg) == false) {
                Usage();
                exit(1);
            }
            break;

        case 't':
            if (ParseTargetCmdLine(CmdLine.test, optarg) == false) {
                Usage();
                exit(1);
            }
            break;

        case 'd':
            work = optarg;
            for (size_t i = 0; i < devices.size(); i++) {
                if (work.compare(devices[i]) == 0) {
                    CmdLine.device = work;
                    deviceFound = true;
                    break;
                }
            }
            if (deviceFound == false) {
                printf("/dev/%s is not among possible devices "
                    "which can be tested\n", work.c_str());
                exit(1);
            }
            break;

        case 'r':
            if (strcmp("pci", optarg) == 0) {
                CmdLine.reset = RESET_PCI;
            } else if (strcmp("ctrlr", optarg) == 0) {
                CmdLine.reset = RESET_CTRLR;
            } else {
                printf("Unrecognizable reset string\n");
                exit(1);
            }
            break;

        case 'p':
            CmdLine.loop = strtoul(optarg, &endptr, 10);
            if (*endptr != '\0') {
                printf("Unrecognized --loop <count>=%s\n", optarg);
                exit(1);
            } else if (CmdLine.loop <= 0) {
                printf("Negative/zero values for --loop are unproductive\n");
                exit(1);
            }
            break;

        case 'l':
            printf("Devices available for test:\n");
            if (devices.size() == 0) {
                printf("none\n");
            } else {
                for (size_t i = 0; i < devices.size(); i++) {
                    printf("%s\n", devices[i].c_str());
                }
            }
            exit(0);

        default:
        case 'h':   Usage();                    exit(0);
        case '?':   Usage();                    exit(1);
        case 's':   CmdLine.summary = true;     break;
        case 'i':   CmdLine.ignore = true;      break;
        }
    }

    if (optind < argc) {
        printf("Unable to parse all cmd line arguments: %d of %d: ",
            optind, argc);
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
        Usage();
        exit(1);
    }


    // Execute cmd line options which require the test infrastructure
    if (BuildTestInfrastructure(groups, fd, CmdLine.device,
        CmdLine.rev) == false) {
        exit(1);
    }

    if (CmdLine.summary) {
        for (size_t i = 0; i < groups.size(); i++) {
            FORMAT_GROUP_DESCRIPTION(work, groups[i])
            printf("%s\n", work.c_str());
            printf("%s", groups[i]->GetGroupSummary(false).c_str());
        }

    } else if (CmdLine.detail.req) {
        if (CmdLine.detail.group == ULONG_MAX) {
            for (size_t i = 0; i < groups.size(); i++) {
                FORMAT_GROUP_DESCRIPTION(work, groups[i])
                printf("%s\n", work.c_str());
                printf("%s", groups[i]->GetGroupSummary(true).c_str());
            }

        } else {    // user spec'd a group they are interested in
            if (CmdLine.detail.group >= groups.size()) {
                printf("Specified test group does not exist\n");

            } else {
                for (size_t i = 0; i < groups.size(); i++) {
                    if (i == CmdLine.detail.group) {
                        FORMAT_GROUP_DESCRIPTION(work, groups[i])
                        printf("%s\n", work.c_str());

                        if ((CmdLine.detail.major == ULONG_MAX) ||
                            (CmdLine.detail.minor == ULONG_MAX)) {
                            // Want info on all tests within group
                            printf("%s",
                                groups[i]->GetGroupSummary(true).c_str());
                        } else {
                            // Want info on spec'd test within group
                            printf("%s", groups[i]->GetTestDescription(true,
                                CmdLine.detail.major,
                                CmdLine.detail.minor).c_str());
                            break;
                        }
                    }
                }
            }
        }
    } else if (CmdLine.reset != RESETTYPE_FENCE) {
        ;   // todo; add some reset logic when available
    } else if (CmdLine.test.req) {
        exitCode = ExecuteTests(CmdLine.test, CmdLine.loop,
            CmdLine.ignore, groups) ? 0 : 1;
    }

    DestroyTestInfrastructure(groups, fd);
    exit(exitCode);
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "[<grp> | <grp>:<test>]" where the absent of the optional parameters means
 * a user is specifying "all" things.
 * @param target Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseTargetCmdLine(TargetType &target, char *optarg)
{
    size_t ulwork;
    char *endptr;
    string swork;


    target.req = true;
    target.group = ULONG_MAX;
    target.major = ULONG_MAX;
    target.minor = ULONG_MAX;
    if (optarg == NULL)
        return (true);

    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        // Specified format <grp>
        target.group = strtoul(swork.c_str(), &endptr, 10);
        if (*endptr != '\0') {
            printf("Unrecognized --detail <grp>:<test>=%s\n", optarg);
            return (false);
        }

    } else {
        // Specified format <grp>:<test>
        target.group = strtoul(
            swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != ':') {
            printf("Missing ':' character in format string\n");
            return (false);
        }

        // Find major piece of <test>
        swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
        if (swork.length() == 0) {
            printf("Missing <test> format string\n");
            return (false);
        }

        target.major = strtoul(
            swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '.') {
            printf("Missing '.' character in format string\n");
            return (false);
        }

        // Find minor piece of <test>
        swork = swork.substr(swork.find_first_of('.') + 1, swork.length());
        if (swork.length() == 0) {
            printf("Unrecognized --detail <grp>:<test>=%s\n", optarg);
            return (false);
        }

        target.minor = strtoul(
            swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '\0') {
            printf("Unrecognized --detail <grp>:<test>=%s\n", optarg);
            return (false);
        }
    }

    if (target.group == ULONG_MAX) {
        printf("Unrecognized --detail <grp>=%s\n", optarg);
        return (false);
    } else if (((target.major == ULONG_MAX) && (target.minor != ULONG_MAX)) ||
               ((target.major != ULONG_MAX) && (target.minor == ULONG_MAX))) {
        printf("Unrecognized --detail <grp>:<test>=%s\n", optarg);
        return (false);
    }

    return (true);
}


/**
 * A function to perform the necessary duties to create the objects, interact
 * with the OS which will allow testing to commence according to the cmd line
 * options presented to this app.
 * @param groups Pass the structure to contain the test objects
 * @param fd Pass the file descriptor to associate with the device
 * @param device Pass the device node to open for testing /dev/node
 * @param specRev Pass the appropriate NVME revision to steer compliance testing
 * @return true upon success, otherwise false
 */
bool
BuildTestInfrastructure(vector<Group *> &groups, int &fd, string device,
    SpecRevType specRev)
{
    struct flock fdlock = {F_WRLCK, SEEK_SET, 0, 0, 0};


    DestroyTestInfrastructure(groups, fd);

    // Open and lock access to the device requested for testing. The mutually
    // exclusive write lock is expected to warrant off other instances of this
    // app from choosing to test against the same device. An app which has
    // attained a lock on the target device may have multiple threads which
    // could cause testing corruption, and therefore a single threaded device
    // interaction model is needed. No more than 1 test can occur at any time
    // to any device and all tests must be single threaded.
    fd = open(device.c_str(), (O_RDWR | O_DIRECT));
    if ((fd = open(device.c_str(), O_RDWR)) == -1) {
        if ((errno == EACCES) || (errno == EAGAIN))
            printf("%s may need permission set for current user\n",
                device.c_str());
        LOG_ERR("%s", strerror(errno));
        return false;
    }
    if (fcntl(fd, F_SETLK, &fdlock) == -1) {
        if ((errno == EACCES) || (errno == EAGAIN))
            printf("%s has been locked by another process\n", device.c_str());
        LOG_ERR("%s", strerror(errno));
    }

    // All groups will be pushed here. The groups themselves dictate which
    // tests get executed based upon the constructed 'specRev' being targeted.
    groups.push_back(new GrpCtrlRegisters(groups.size(), specRev, fd));

    return true;
}


/**
 * Tear down that which has been created by BuildTestInfrastructure()
 * @param groups Pass the structure to contain the test objects, if the
 *               structure is not empty the function will abort.
 * @param fd Pass the file descriptor to free from the allocated resource pool
 */
void
DestroyTestInfrastructure(vector<Group *> &groups, int &fd)
{
    // Deallocate heap usage
    for (size_t i = 0; i < groups.size(); i++) {
        delete groups.back();
        groups.pop_back();
    }

    // If it fails what do we do? ignore it for now
    if (fd != -1) {
        if (close(fd) == -1)
            LOG_ERR("%s", strerror(errno));
    }
}


/**
 * A function to execute the desired test case(s). Param ignore
 * indicates that when an error is reported from a test case, it is ignored to
 * the point that the next test case will be allowed to run, if and only if,
 * there are more tests which could have run if the test passed. The error
 * itself won't be lost. because the end return value from this function will
 * indicate an error was detected even though it was ignored.
 * @param test Pass the structure as returned from ParseTargetCmdLine()
 * @param loop Pass the number of times to loop the test(s)
 * @param ignore Pass whether to keep running when errors are detected
 * @param groups Pass all groups being considered for execution
 * @return true upon success, false if failures/errors detected; Param
 *          ignore does not affect return value if an error is detected.
 */
bool
ExecuteTests(TargetType test, size_t loop, bool ignore, vector<Group *> &groups)
{
    bool finResult = true;    // assuming success until we find otherwise
    bool allHaveRun = false;
    TestIteratorType testIter;


    if ((test.group != ULONG_MAX) && (test.group >= groups.size())) {
        LOG_ERR("Specified test group does not exist");
        return false;
    }

    for (size_t iLoop = 0; iLoop < loop; iLoop++) {
        LOG_NORM("Start loop execution %ld", iLoop);

        for (size_t iGrp = 0; iGrp < groups.size(); iGrp++) {
            bool locResult;

            if (test.group == ULONG_MAX) {
                // Run all tests within all groups
                testIter = groups[iGrp]->GetTestIterator();
                while (allHaveRun == false) {
                    locResult = groups[iGrp]->RunTest(testIter, allHaveRun);
                    finResult = finResult ? locResult : finResult;
                    if ((ignore == false) && (finResult == false))
                        goto FAIL_OUT;
                }

            } else if ((test.major == ULONG_MAX) || (test.minor == ULONG_MAX)) {
                // Run all tests within spec'd group
                if (iGrp == test.group) {
                    testIter = groups[iGrp]->GetTestIterator();
                    while (allHaveRun == false) {
                        locResult = groups[iGrp]->RunTest(testIter, allHaveRun);
                        finResult = finResult ? locResult : finResult;
                        if ((ignore == false) && (finResult == false))
                            goto FAIL_OUT;
                    }
                    break;  // done; do we keep looping?
                }

            } else {
                // Run spec'd test within spec'd group
                if (iGrp == test.group) {
                    locResult = groups[iGrp]->RunTest(test.major, test.minor);
                    finResult = finResult ? locResult : finResult;
                    if ((ignore == false) && (finResult == false))
                        goto FAIL_OUT;
                    break;  // done; do we keep looping?
                }
            }
        }

        LOG_NORM("Stop loop execution %ld", iLoop);
    }

FAIL_OUT:
    return (finResult);
}

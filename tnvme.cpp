/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "tnvme.h"
#include "tnvmeHelpers.h"
#include "tnvmeParsers.h"
#include "version.h"
#include "globals.h"
#include "Utils/kernelAPI.h"
#include "Utils/fileSystem.h"


// ------------------------------EDIT HERE---------------------------------
#include "GrpPciRegisters/grpPciRegisters.h"
#include "GrpCtrlRegisters/grpCtrlRegisters.h"
#include "GrpBasicInit/grpBasicInit.h"
#include "GrpResets/grpResets.h"
#include "GrpQueues/grpQueues.h"
#include "GrpNVMReadCmd/grpNVMReadCmd.h"
#include "GrpNVMWriteCmd/grpNVMWriteCmd.h"
#include "GrpNVMWriteReadCombo/grpNVMWriteReadCombo.h"
#include "GrpNVMFlushCmd/grpNVMFlushCmd.h"
#include "GrpInterrupts/grpInterrupts.h"
#include "GrpGeneralCmds/grpGeneralCmds.h"
#include "GrpNVMWriteUncorrectCmd/grpNVMWriteUncorrectCmd.h"
#include "GrpNVMDatasetMgmtCmd/grpNVMDatasetMgmtCmd.h"
#include "GrpNVMCompareCmd/grpNVMCompareCmd.h"
#include "GrpAdminDeleteIOCQCmd/grpAdminDeleteIOCQCmd.h"
#include "GrpAdminDeleteIOSQCmd/grpAdminDeleteIOSQCmd.h"
#include "GrpAdminCreateIOCQCmd/grpAdminCreateIOCQCmd.h"
#include "GrpAdminCreateIOSQCmd/grpAdminCreateIOSQCmd.h"
#include "GrpAdminCreateIOQCmd/grpAdminCreateIOQCmd.h"
#include "GrpAdminGetLogPgCmd/grpAdminGetLogPgCmd.h"
#include "GrpAdminIdentifyCmd/grpAdminIdentifyCmd.h"
#include "GrpAdminSetFeatCmd/grpAdminSetFeatCmd.h"
#include "GrpAdminGetFeatCmd/grpAdminGetFeatCmd.h"
#include "GrpAdminSetGetFeatCombo/grpAdminSetGetFeatCombo.h"
#include "GrpAdminAsyncCmd/grpAdminAsyncCmd.h"


void
InstantiateGroups(vector<Group *> &groups)
{
    // Appending new groups is the most favorable action here
    groups.push_back(new GrpPciRegisters::GrpPciRegisters(groups.size()));
    groups.push_back(new GrpCtrlRegisters::GrpCtrlRegisters(groups.size()));
    groups.push_back(new GrpBasicInit::GrpBasicInit(groups.size()));
    groups.push_back(new GrpResets::GrpResets(groups.size()));
    groups.push_back(new GrpGeneralCmds::GrpGeneralCmds(groups.size()));
    // Following is assigned grp ID=5
    groups.push_back(new GrpQueues::GrpQueues(groups.size()));
    groups.push_back(new GrpNVMReadCmd::GrpNVMReadCmd(groups.size()));
    groups.push_back(new GrpNVMWriteCmd::GrpNVMWriteCmd(groups.size()));
    groups.push_back(new GrpNVMWriteReadCombo::GrpNVMWriteReadCombo(groups.size()));
    groups.push_back(new GrpNVMFlushCmd::GrpNVMFlushCmd(groups.size()));
    // Following is assigned grp ID=10
    groups.push_back(new GrpInterrupts::GrpInterrupts(groups.size()));
    groups.push_back(new GrpNVMWriteUncorrectCmd::GrpNVMWriteUncorrectCmd(groups.size()));
    groups.push_back(new GrpNVMDatasetMgmtCmd::GrpNVMDatasetMgmtCmd(groups.size()));
    groups.push_back(new GrpNVMCompareCmd::GrpNVMCompareCmd(groups.size()));
    groups.push_back(new GrpAdminDeleteIOCQCmd::GrpAdminDeleteIOCQCmd(groups.size()));
    // Following is assigned grp ID=15
    groups.push_back(new GrpAdminDeleteIOSQCmd::GrpAdminDeleteIOSQCmd(groups.size()));
    groups.push_back(new GrpAdminCreateIOCQCmd::GrpAdminCreateIOCQCmd(groups.size()));
    groups.push_back(new GrpAdminCreateIOSQCmd::GrpAdminCreateIOSQCmd(groups.size()));
    groups.push_back(new GrpAdminCreateIOQCmd::GrpAdminCreateIOQCmd(groups.size()));
    groups.push_back(new GrpAdminGetLogPgCmd::GrpAdminGetLogPgCmd(groups.size()));
    // Following is assigned grp ID=20
    groups.push_back(new GrpAdminIdentifyCmd::GrpAdminIdentifyCmd(groups.size()));
    groups.push_back(new GrpAdminSetFeatCmd::GrpAdminSetFeatCmd(groups.size()));
    groups.push_back(new GrpAdminGetFeatCmd::GrpAdminGetFeatCmd(groups.size()));
    groups.push_back(new GrpAdminSetGetFeatCombo::GrpAdminSetGetFeatCombo(groups.size()));
    groups.push_back(new GrpAdminAsyncCmd::GrpAdminAsyncCmd(groups.size()));
}
// ------------------------------EDIT HERE---------------------------------


#define BASE_DUMP_DIR           "./Logs"
#define NO_DEVICES              "no devices found"
#define INFORM_GRPNUM           0


void Usage(void);
void DestroySingletons();
bool ExecuteTests(struct CmdLine &cl, vector<Group *> &groups);
bool BuildSingletons();
void DestroyTestFoundation(vector<Group *> &groups);
bool BuildTestFoundation(vector<Group *> &groups);
void ReportTestResults(size_t numIters, int numPass, int numFail, int numSkip,
    int numGrps);
void ReportExecution(vector<TestRef> failedTests, vector<TestRef> skippedTests);


void
Usage(void) {
    //80->  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    printf("%s (revision %d.%d)\n", APPNAME, VER_MAJOR, VER_MINOR);
    printf("  -h(--help)                          Display this help\n");
    printf("  -v(--rev) <spec>                    All options are forced to target spec'd\n");
    printf("                                      NVME revision {1.0b}; dflt=1.0b\n");
    printf("  -s(--summary)                       Summarize all groups and tests\n");
    printf("  -a(--detail) [<grp> | <grp>:<test>] Detailed group and test description for:\n");
    printf("                                      {all | spec'd_group | test_within_group}\n");
    printf("  -t(--test) [<grp> | <grp>:<test>]   Execute tests for:\n");
    printf("                                      {all | spec'd_group | test_within_group}\n");
    printf("  -l(--list)                          List all devices available for test\n");
    printf("  -d(--device) <name>                 Device to open for testing: /dev/node\n");
    printf("                                      dflt=(1st device listed in --list)\n");
    printf("  -z(--reset)                         Ctrl'r level reset via CC.EN\n");
    printf("  -o(--loop) <count>                  Loop test execution <count> times; dflt=1\n");
    printf("  -k(--skiptest) <filename>           A file contains a list of tests to skip\n");
    printf("  -u(--dump) <dirname>                Pass the base dump directory path.\n");
    printf("                                      dflt=\"%s\"\n", BASE_DUMP_DIR);
    printf("  -i(--ignore)                        Ignore detected errors; An error causes\n");
    printf("                                      the next test w/o dependencies to failing\n");
    printf("                                      test to be executed next.\n");
    printf("  -p(--preserve)                      Preserve the current state of the DUT. Do\n");
    printf("                                      not write data on the media, nor modify\n");
    printf("                                      the configuration. Not all tests will run\n");
    printf("  -g(--golden) <fileIn>:<fileOut>     fileIn contains the golden identify data\n");
    printf("                                      to which the DUT's reported identify data\n");
    printf("                                      is compared; Must be only option.\n");
    printf("                                      fileOut, optional file for results output\n");
    printf("  -n(--postfail)                      Upon test failure, instruct framework to\n");
    printf("                                      take a post failure snapshot of the DUT\n");
    printf("  -b(--rsvdfields)                    Execute the optional reserved field\n");
    printf("                                      tests; verifying fields are zero value\n");
    printf("  -y(--restore)                       Upon test failure, allow an individual\n");
    printf("                                      test to restore the configuration of the\n");
    printf("                                      DUT as was detected at group start\n");
    printf("  -m(--fwimage)                       Supply a FW image to allow testing of\n");
    printf("                                      FW activate and FW image dnld cmds.\n");
    printf("                                      Recommend supply identical FW image as\n");
    printf("                                      current test image.\n");
    printf("                      --- Advanced/Debug Options Follow ---\n");
    printf("  -e(--error) <STS:PXDS:AERUCES:CSTS> Set reg bitmask for bits indicating error\n");
    printf("                                      state after each test completes.\n");
    printf("                                      Value=0 indicates ignore all errors.\n");
    printf("                                      Require base 16 values.\n");
    printf("  -q(--queues) <ncqr:nsqr>            Write <ncqr> and <nsqr> as values by the\n");
    printf("                                      Set Features, ID=7. Must be only option.\n");
    printf("                                      Option missing indicates tnvme to read\n");
    printf("                                      and learn a previous set value.\n");
    printf("                                      until next power cycle. Requires base 16\n");
    printf("  -f(--format) <filename>             A file contains admin cmd set-format NVM\n");
    printf("                                      cmd instructions to send to device\n");
    printf("                                      Must be only option. Ex: format.xml file\n");
    printf("  -r(--rmmap) <space:off:size:acc>    Read memmap'd I/O registers from\n");
    printf("                                      <space>={PCI | BAR01} at <off> bytes\n");
    printf("                                      from start of space for <size> bytes\n");
    printf("                                      access width <acc>={l | w | b} type\n");
    printf("                                      <off:size:acc> require base 16 values\n");
    printf("  -w(--wmmap) <space:off:siz:val:acc> Write <val> data to memmap'd I/O reg from\n");
    printf("                                      <space>={PCI | BAR01} at <off> bytes\n");
    printf("                                      from start of space for <siz> bytes\n");
    printf("                                      access width <acc>={l | w | b} type\n");
    printf("                                      (Require: <size> < 8)\n");
    printf("                                      <offset:size> requires base 16 values\n");
}


int
main(int argc, char *argv[])
{
    int c;
    int idx = 0;
    int exitCode = 0;   // assume success
    char *endptr;
    long tmp;
    string work;
    vector<Group *> groups;
    vector<string> devices;
    struct dirent *dirEntry;
    bool deviceFound = false;
    bool accessingHdw = true;
    uint64_t regVal = 0;
    const char *short_opt = "hsnblpyzia::t::v:o:d:k:f:r:w:q:e:m:u:g:";
    static struct option long_opt[] = {
        // {name,           has_arg,            flag,   val}
        {   "detail",       optional_argument,  NULL,   'a'},
        {   "test",         optional_argument,  NULL,   't'},

        {   "rev",          required_argument,  NULL,   'v'},
        {   "device",       required_argument,  NULL,   'd'},
        {   "rmmap" ,       required_argument,  NULL,   'r'},
        {   "wmmap" ,       required_argument,  NULL,   'w'},
        {   "loop",         required_argument,  NULL,   'o'},
        {   "skiptest",     required_argument,  NULL,   'k'},
        {   "format",       required_argument,  NULL,   'f'},
        {   "queues",       required_argument,  NULL,   'q'},
        {   "error",        required_argument,  NULL,   'e'},
        {   "dump",         required_argument,  NULL,   'u'},
        {   "golden",       required_argument,  NULL,   'g'},
        {   "fwimage",      required_argument,  NULL,   'm'},

        {   "help",         no_argument,        NULL,   'h'},
        {   "summary",      no_argument,        NULL,   's'},
        {   "preserve",     no_argument,        NULL,   'p'},
        {   "restore",      no_argument,        NULL,   'y'},
        {   "list",         no_argument,        NULL,   'l'},
        {   "reset",        no_argument,        NULL,   'z'},
        {   "ignore",       no_argument,        NULL,   'i'},
        {   "postfail",     no_argument,        NULL,   'n'},
        {   "rsvdfields",   no_argument,        NULL,   'b'},
        {   NULL,           no_argument,        NULL,    0}
    };

    // Defaults if not spec'd on cmd line
    gCmdLine.rev = SPECREV_10b;
    gCmdLine.loop = 1;
    gCmdLine.device = NO_DEVICES;
    gCmdLine.errRegs.sts = (STS_SSE | STS_STA | STS_RMA | STS_RTA);
    gCmdLine.errRegs.pxds = (PXDS_TP | PXDS_FED);
    gCmdLine.errRegs.csts = CSTS_CFS;
    gCmdLine.dump = BASE_DUMP_DIR;

    if (argc == 1) {
        printf("%s is a compliance test suite for NVM Express hardware.\n",
            APPNAME);
        exit(0);
    }

    // Disable buffering stdout, risk not seeing statements merged with dump dir
    setbuf(stdout, NULL);

    // Seek for all possible devices that this app may commune
    DIR *devDir = opendir("/dev");
    if (devDir == NULL) {
        printf("Unable to open system /dev directory\n");
        exit(1);
    }
    while ((dirEntry = readdir(devDir)) != NULL) {
        work = dirEntry->d_name;
        if (work.find("nvme") != string::npos)
            devices.push_back("/dev/" + work);
    }
    if (devices.size())
        gCmdLine.device = devices[0];    // Default to 1st element listed

    // Report what we are about to process
    work = "";
    for (int i = 0; i < argc; i++)
        work += argv[i] + string(" ");
    printf("Parsing cmd line: %s\n", work.c_str());


    // Parse cmd line options
    while ((c = getopt_long(argc, argv, short_opt, long_opt, &idx)) != -1) {
        switch (c) {

        case 'v':
            if (strcmp("1.0a", optarg) == 0) {
                gCmdLine.rev = SPECREV_10b;
            }
            break;

        case 'a':
            if (ParseTargetCmdLine(gCmdLine.detail, optarg) == false) {
                printf("Unable to parse --detail cmd line\n");
                exit(1);
            }
            accessingHdw = false;
            break;

        case 't':
            if (ParseTargetCmdLine(gCmdLine.test, optarg) == false) {
                printf("Unable to parse --test cmd line\n");
                exit(1);
            }
            break;

        case 'k':
            if (ParseSkipTestCmdLine(gCmdLine.skiptest, optarg) == false) {
                printf("Unable to parse --skiptest cmd line\n");
                exit(1);
            }
            break;

        case 'g':
            if (ParseGoldenCmdLine(gCmdLine.golden, optarg) == false) {
                printf("Unable to parse --golden cmd line\n");
                exit(1);
            }
            break;

        case 'm':
            if (ParseFWImageCmdLine(gCmdLine.fwImage, optarg) == false) {
                printf("Unable to parse --fwimage cmd line\n");
                exit(1);
            }
            break;

        case 'f':
            if (ParseFormatCmdLine(gCmdLine.format, optarg) == false) {
                printf("Unable to parse --format cmd line\n");
                exit(1);
            }
            break;

        case 'r':
            if (ParseRmmapCmdLine(gCmdLine.rmmap, optarg) == false) {
                printf("Unable to parse --rmmap cmd line\n");
                exit(1);
            }
            break;

        case 'w':
            if (ParseWmmapCmdLine(gCmdLine.wmmap, optarg) == false) {
                printf("Unable to parse --wmmap cmd line\n");
                exit(1);
            }
            break;

        case 'e':
            if (ParseErrorCmdLine(gCmdLine.errRegs, optarg) == false) {
                printf("Unable to parse --error cmd line\n");
                exit(1);
            }
            break;

        case 'd':
            work = optarg;
            for (size_t i = 0; i < devices.size(); i++) {
                if (work.compare(devices[i]) == 0) {
                    gCmdLine.device = work;
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

        case 'o':
            tmp = strtol(optarg, &endptr, 10);
            if (*endptr != '\0') {
                printf("Unrecognized --loop <count>=%s\n", optarg);
                exit(1);
            } else if (tmp <= 0) {
                printf("Negative/zero values for --loop are unproductive\n");
                exit(1);
            }
            gCmdLine.loop = tmp;
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

        case 'q':
            if (ParseQueuesCmdLine(gCmdLine.numQueues, optarg) == false) {
                printf("Unable to parse --queues cmd line\n");
                exit(1);
            }
            break;

        case 's':
            gCmdLine.summary = true;
            accessingHdw = false;
            break;

        case 'u':
            gCmdLine.dump = optarg;
            break;

        default:
        case 'h':   Usage();                            exit(0);
        case '?':   Usage();                            exit(1);
        case 'z':   gCmdLine.reset = true;              break;
        case 'i':   gCmdLine.ignore = true;             break;
        case 'p':   gCmdLine.preserve = true;           break;
        case 'n':   gCmdLine.postfail = true;           break;
        case 'b':   gCmdLine.rsvdfields = true;         break;
        case 'y':   gCmdLine.restore = true;            break;
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

    try {   // Everything below has the ability to throw exceptions

        // Instantiates and initializes all globals defined within globals.h
        if (BuildTestFoundation(groups) == false) {
            printf("Unable to build the test foundation\n");
            exit(1);
        }

        // Accessing hdw requires specific checks and inquiries before running
        if (accessingHdw) {
            if (FileSystem::SetRootDumpDir(gCmdLine.dump) == false) {
                printf("Unable to establish \"%s\" dump directory\n",
                    gCmdLine.dump.c_str());
                exit(1);
            }

            if (BuildSingletons() == false) {
                printf("Unable to instantiate mandatory framework objects\n");
                exit(1);
            }

            printf("Checking for unintended device under low powered states\n");
            if (gRegisters->Read(PCISPC_PMCS, regVal) == false) {
                printf("Mandatory PMCAP PCI capabilities is missing\n");
                exit(1);
            } else if (regVal & 0x03) {
                printf("PCI power state not fully operational\n");
            }
        }

        // Process the user's cmd line parameters
        if (gCmdLine.golden.req) {
            if ((exitCode = !CompareGolden(gCmdLine.golden))) {
                printf("FAILURE: Comparing golden data\n");
            } else {
                printf("SUCCESS: Comparing golden data\n");
            }
        } else if (gCmdLine.format.req) {
            if ((exitCode = !FormatDevice(gCmdLine.format))) {
                printf("FAILURE: Formatting device\n");
            } else {
                printf("SUCCESS: Formatting device\n");
            }
        } else if (gCmdLine.numQueues.req) {
            if ((exitCode = !SetFeaturesNumberOfQueues(gCmdLine.numQueues))) {
                printf("FAILURE: Setting number of queues\n");
            } else {
                printf("SUCCESS: Setting number of queues\n");
            }
        } else if (gCmdLine.summary) {
            for (size_t i = 0; i < groups.size(); i++) {
                FORMAT_GROUP_DESCRIPTION(work, groups[i])
                printf("%s\n", work.c_str());
                printf("%s", groups[i]->GetGroupSummary(false).c_str());
            }

        } else if (gCmdLine.detail.req) {
            if (gCmdLine.detail.t.group == UINT_MAX) {
                for (size_t i = 0; i < groups.size(); i++) {
                    FORMAT_GROUP_DESCRIPTION(work, groups[i])
                    printf("%s\n", work.c_str());
                    printf("%s", groups[i]->GetGroupSummary(true).c_str());
                }

            } else {    // user spec'd a group they are interested in
                if (gCmdLine.detail.t.group >= groups.size()) {
                    printf("Specified test group %ld does not exist\n",
                        gCmdLine.detail.t.group);
                } else {
                    for (size_t i = 0; i < groups.size(); i++) {
                        if (i == gCmdLine.detail.t.group) {
                            FORMAT_GROUP_DESCRIPTION(work, groups[i])
                            printf("%s\n", work.c_str());

                            if ((gCmdLine.detail.t.xLev == UINT_MAX) ||
                                (gCmdLine.detail.t.yLev == UINT_MAX) ||
                                (gCmdLine.detail.t.zLev == UINT_MAX)) {
                                // Want info on all tests within group
                                printf("%s",
                                    groups[i]->GetGroupSummary(true).c_str());
                            } else {
                                // Want info on spec'd test within group
                                printf("%s", groups[i]->GetTestDescription(true,
                                    gCmdLine.detail.t).c_str());
                                break;
                            }
                        }
                    }
                }
            }
        } else if (gCmdLine.rmmap.req) {
            uint8_t *value = new uint8_t[gCmdLine.rmmap.size];
            gRegisters->Read(gCmdLine.rmmap.space, gCmdLine.rmmap.size,
                gCmdLine.rmmap.offset, gCmdLine.rmmap.acc, value);
        } else if (gCmdLine.wmmap.req) {
            gRegisters->Write(gCmdLine.wmmap.space, gCmdLine.wmmap.size,
                gCmdLine.wmmap.offset, gCmdLine.wmmap.acc,
                (uint8_t *)(&gCmdLine.wmmap.value));
        } else if (gCmdLine.reset) {
            if ((exitCode = !gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY))) {
                printf("FAILURE: reset\n");
            } else {
                printf("SUCCESS: reset\n");
            }
            // At this point we cannot enable the ctrlr because that requires
            // ACQ/ASQ's to be created, ctrlr simply won't become ready w/o them
        } else if (gCmdLine.test.req) {
            if ((exitCode = !ExecuteTests(gCmdLine, groups))) {
                printf("FAILURE: testing\n");
            } else {
                printf("SUCCESS: testing\n");
            }
        }
    } catch (...) {
        LOG_ERR("An unforeseen exception has been caught");
    }

    // cleanup duties
    DestroyTestFoundation(groups);
    DestroySingletons();
    gCmdLine.skiptest.clear();
    devices.clear();
    exit(exitCode);
}


bool BuildSingletons()
{
    // Create globals/singletons here, which all tests objects will need

    // The Register singleton should be created 1st because all other Singletons
    // use it directly to become init'd or they rely on it heavily soon after.
    gRegisters = Registers::GetInstance(gDutFd, gCmdLine.rev);
    if (gRegisters == NULL) {
        LOG_ERR("Unable to create framework obj: gRegisters");
        return false;
    }

    // The CtrlrConfig singleton should be created 2nd because it's subject base
    // class is used by just about every other object in the framework to learn
    // of state changes within the ctrlr. Disabling the ctrlr is extremely
    // destructive of all resources in user space and in kernel space.
    gCtrlrConfig = CtrlrConfig::GetInstance(gDutFd, gCmdLine.rev);
    if (gCtrlrConfig == NULL) {
        LOG_ERR("Unable to create framework obj: gCtrlrConfig");
        return false;
    }

    // Create the remaining objects at random...
    gRsrcMngr = RsrcMngr::GetInstance(gDutFd, gCmdLine.rev);
    if (gRsrcMngr == NULL) {
        LOG_ERR("Unable to create framework obj: gRsrcMngr");
        return false;
    }
    gCtrlrConfig->Attach(*gRsrcMngr);

    gInformative = Informative::GetInstance(gDutFd, gCmdLine.rev);
    if (gInformative == NULL) {
        LOG_ERR("Unable to create framework obj: gInformative");
        return false;
    }

    return true;
}


void DestroySingletons()
{
    // Destroy in reverse order as was created
    RsrcMngr::KillInstance();
    Informative::KillInstance();
    CtrlrConfig::KillInstance();
    Registers::KillInstance();
}


/**
 * A function to perform the necessary duties to create the objects, interact
 * with the OS which will allow testing to commence according to the cmd line
 * options presented to this app.
 * @param groups Pass the structure to contain the test objects
 * @return true upon success, otherwise false
 */
bool
BuildTestFoundation(vector<Group *> &groups)
{
    int ret;
    struct metrics_driver driverMetrics;
    struct flock fdlock = {F_WRLCK, SEEK_SET, 0, 0, 0};


    DestroyTestFoundation(groups);

    // Open and lock access to the device requested for testing. The mutually
    // exclusive write lock is expected to warrant off other instances of this
    // app from choosing to test against the same device. An app which has
    // attained a lock on the target device may have multiple threads which
    // could cause testing corruption, and therefore a single threaded device
    // interaction model is needed. No more than 1 test can occur at any time
    // to any device and all tests must be single threaded.
    if (gCmdLine.device.compare(NO_DEVICES) == 0) {
        LOG_ERR("There are no devices present");
        return false;
    }
    if ((gDutFd = open(gCmdLine.device.c_str(), O_RDWR)) == -1) {
        if ((errno == EACCES) || (errno == EAGAIN)) {
            LOG_ERR("%s may need permission set for current user",
                gCmdLine.device.c_str());
        }
        LOG_ERR("device=%s: %s", gCmdLine.device.c_str(), strerror(errno));
        return false;
    }
    if (fcntl(gDutFd, F_SETLK, &fdlock) == -1) {
        if ((errno == EACCES) || (errno == EAGAIN)) {
            LOG_ERR("%s has been locked by another process",
            gCmdLine.device.c_str());
        }
        LOG_ERR("%s", strerror(errno));
    }

    // Validate the dnvme was compiled with the same version of API as tnvme
    ret = ioctl(gDutFd, NVME_IOCTL_GET_DRIVER_METRICS, &driverMetrics);
    if (ret < 0) {
        LOG_ERR("Unable to extract driver version information");
        return false;
    }
    LOG_NRM("tnvme binary: v/%d.%d", VER_MAJOR, VER_MINOR);
    LOG_NRM("tnvme compiled against dnvme API: v/%d.%d.%d",
        ((API_VERSION >> 16) & 0xFF),
        ((API_VERSION >> 8) & 0xFF),
        ((API_VERSION >> 0) & 0xFF));
    LOG_NRM("dnvme API residing within kernel: v/%d.%d.%d",
        ((driverMetrics.api_version >> 16) & 0xFF),
        ((driverMetrics.api_version >> 8) & 0xFF),
        ((driverMetrics.api_version >> 0) & 0xFF));
    if (driverMetrics.api_version != API_VERSION) {
        LOG_ERR("dnvme vs tnvme version mismatch, refusing to execute");
        return false;
    }

    InstantiateGroups(groups);
    return true;
}


/**
 * Tear down that which has been created by BuildTestFoundation()
 * @param groups Pass the structure to contain the test objects, if the
 *               structure is not empty the function will abort.
 */
void
DestroyTestFoundation(vector<Group *> &groups)
{
    // Deallocate heap usage
    for (size_t i = 0; i < groups.size(); i++) {
        delete groups.back();
        groups.pop_back();
    }

    // If it fails what do we do? ignore it for now
    if (gDutFd != -1) {
        if (close(gDutFd) == -1)
            LOG_ERR("%s", strerror(errno));
        gDutFd = -1;
    }
}


/**
 * A function to execute the desired test case(s). Param ignore
 * indicates that when an error is reported from a test case, it is ignored to
 * the point that the next test case will be allowed to run, if and only if,
 * there are more tests which could have run if the test passed. The error
 * itself won't be lost. because the end return value from this function will
 * indicate an error was detected even though it was ignored.
 * @param cl Pass the cmd line parameters
 * @param groups Pass all groups being considered for execution
 * @return true upon success, false if failures/errors detected;
 */
bool
ExecuteTests(struct CmdLine &cl, vector<Group *> &groups)
{
    int64_t tstIdx = 0;
    int64_t skipped = 0;
    size_t iLoop;
    int numPassed = 0;
    int numFailed = 0;
    int numSkipped = 0;
    int numGrps = 0;
    bool allTestsPass = true;    // assuming success until we find otherwise
    bool allHaveRun = false;
    bool thisTestPass;
    TestRef targetTst;
    TestSetType testsToRun;
    bool tstSetOK;
    vector<TestRef> failedTests;
    vector<TestRef> skippedTests;

    if ((cl.test.t.group != UINT_MAX) && (cl.test.t.group >= groups.size())) {
        LOG_ERR("Specified test group does not exist");
        goto ABORT_OUT;
    } else if (VerifySpecCompatibility(cl.rev) == false) {
        LOG_ERR("Targeted compliance revision incompatible with hdw");
        goto ABORT_OUT;
    }

    if ((cl.test.t.group == UINT_MAX) || (cl.test.t.xLev == UINT_MAX) ||
        (cl.test.t.yLev == UINT_MAX) || (cl.test.t.zLev == UINT_MAX)) {

        LOG_NRM("Attempting to satisfy target test: ALL:ALL.ALL.ALL");
    } else if ((cl.test.t.xLev == UINT_MAX) ||
               (cl.test.t.yLev == UINT_MAX) ||
               (cl.test.t.zLev == UINT_MAX)) {

        LOG_NRM("Attempting to satisfy target test: %ld:ALL.ALL.ALL",
            cl.test.t.group);
    } else {
        LOG_NRM("Attempting to satisfy target test: %ld:%ld.%ld.%ld",
            cl.test.t.group, cl.test.t.xLev, cl.test.t.yLev, cl.test.t.zLev);
    }

    for (iLoop = 0; iLoop < cl.loop; iLoop++) {
        LOG_NRM("Start loop execution #%ld", iLoop);

        for (size_t iGrp = 0; iGrp < groups.size(); iGrp++) {
            allHaveRun = false;

            LOG_DBG("Processing test(s) for group %ld", iGrp);
            if (cl.test.t.group == UINT_MAX) {
                targetTst.Init(iGrp, UINT_MAX, UINT_MAX, UINT_MAX);
            } else if (iGrp == cl.test.t.group) {
                targetTst.Init(iGrp, cl.test.t.xLev, cl.test.t.yLev,
                    cl.test.t.zLev);
            } else {
                continue;
            }

            LOG_NRM("Executing a new group, start from known point");
            if (FileSystem::CleanDumpDir() == false)
                LOG_WARN("Unable to cleanup dump between group runs");
            if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
                goto ABORT_OUT;

            tstSetOK = groups[iGrp]->GetTestSet(targetTst, testsToRun, tstIdx);
            if (tstSetOK == false) {
                LOG_ERR("Unable to get execution test set");
                goto ABORT_OUT;
            }

            numGrps++;
            while (allHaveRun == false) {
                thisTestPass = true;

                switch (groups[iGrp]->RunTest(testsToRun, tstIdx,
                    cl.skiptest, skipped, cl.preserve, failedTests,
                    skippedTests)) {
                case Group::TR_SUCCESS:
                    numPassed++;
                    break;
                case Group::TR_FAIL:
                    allTestsPass = false;
                    thisTestPass = false;
                    numFailed++;
                    numSkipped += skipped;
                    if (cl.ignore) {
                        LOG_WARN("Detected error, but forced to ignore");
                    } else {
                        goto EARLY_OUT;
                    }
                    break;
                case Group::TR_SKIPPING:
                    numSkipped += skipped;
                    break;
                case Group::TR_NOTFOUND:
                    allHaveRun = true;
                    break;
                }
            }
        }

        // Report each iteration results
        ReportTestResults(iLoop, numPassed, numFailed, numSkipped, numGrps);

        if (failedTests.size() || skippedTests.size())
            ReportExecution(failedTests, skippedTests);
    }
    return allTestsPass;

EARLY_OUT:
    ReportTestResults(iLoop, numPassed, numFailed, numSkipped, numGrps);
    if (failedTests.size() || skippedTests.size())
        ReportExecution(failedTests, skippedTests);
    return allTestsPass;

ABORT_OUT:
    LOG_NRM("Iteration SUMMARY  : Testing aborted");
    return false;
}


void
ReportTestResults(size_t numIters, int numPass, int numFail, int numSkip,
    int numGrps)
{
    LOG_NRM("Iteration SUMMARY");
    LOG_NRM("  passed       : %d", numPass);
    if (numFail) {
        LOG_NRM("  failed       : %d  <---", numFail);
    } else {
        LOG_NRM("  failed       : %d", numFail);
    }
    LOG_NRM("  skipped      : %d", numSkip);
    LOG_NRM("  total tests  : %d", numPass+numFail+numSkip);
    LOG_NRM("  total groups : %d", numGrps);
    LOG_NRM("Stop loop execution #%ld", numIters);
}


void
ReportExecution(vector<TestRef> failedTests, vector<TestRef> skippedTests)
{
    LOG_NRM("Detailed Iteration SUMMARY");
    LOG_NRM("   Tests Failed :");
    for(uint32_t i = 0; i < failedTests.size(); i++)
        LOG_NRM("      %d:%d.%d.%d", (int)failedTests[i].group,
            (int)failedTests[i].xLev, (int)failedTests[i].yLev,
            (int)failedTests[i].zLev);

    LOG_NRM("   Tests Skipped :");
    for(uint32_t i = 0; i < skippedTests.size(); i++)
        LOG_NRM("      %d:%d.%d.%d", (int)skippedTests[i].group,
            (int)skippedTests[i].xLev, (int)skippedTests[i].yLev,
            (int)skippedTests[i].zLev);


}

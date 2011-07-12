#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <string>
#include <vector>
#include "tnvme.h"

#include "GrpCtrlRegisters/grpCtrlRegisters.h"

using namespace std;

void Usage(void);
bool ParseTargetCmdLine(TargetType &target, char *optarg);
bool ExecuteTests(TargetType test, size_t loop, bool ignore,
    vector<Group *> &groups);


int
main(int argc, char *argv[])
{
    int c;
    int idx = 0;
    int exitCode = 0;
    char *endptr;
    string work;
    vector<Group *> groups;
    struct CmdLineType CmdLine;
    const char *short_opt = "hsiv:d::r:l:";
    static struct option long_opt[] = {
        // {name,       has_arg,            flag,   val}
        {   "help",     no_argument,        NULL,   'h'},
        {   "summary",  no_argument,        NULL,   's'},
        {   "rev",      required_argument,  NULL,   'v'},
        {   "detail",   optional_argument,  NULL,   'd'},
        {   "test",     optional_argument,  NULL,   't'},
        {   "reset",    required_argument,  NULL,   'r'},
        {   "ignore",   no_argument,        NULL,   'i'},
        {   "loop",     required_argument,  NULL,   'l'},
        {   NULL,       no_argument,        NULL,    0}
    };


    // defaults if not spec'd on cmd line
    CmdLine.rev = SPECREV_10a;
    CmdLine.detail.req = false;
    CmdLine.reset = RESETTYPE_FENCE;
    CmdLine.summary = false;
    CmdLine.ignore = false;
    CmdLine.loop = 1;

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

        case 'd':
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

        case 'r':
            if (strcmp("pci", optarg) == 0) {
                CmdLine.reset = RESET_PCI;
            } else if (strcmp("ctrlr", optarg) == 0) {
                CmdLine.reset = RESET_CTRLR;
            } else {
                LOG_ERR("Unrecognizable reset string");
                exit(1);
            }
            break;

        case 'l':
            CmdLine.loop = strtoul(optarg, &endptr, 10);
            if (*endptr != '\0') {
                LOG_ERR("Unrecognized --loop <count>=%s", optarg);
                exit(1);
            } else if (CmdLine.loop <= 0) {
                LOG_ERR("Negative/zero values for --loop are non-productive\n");
                exit(1);
            }
            break;

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


    // Create the test infrastructure per NVME specification revision
    groups.push_back(new GrpCtrlRegisters(groups.size(), CmdLine.rev));

    // Process user cmd line options
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


    // Deallocate heap usage
    for (size_t i = 0; i < groups.size(); i++) {
        delete groups.back();
        groups.pop_back();
    }

    exit(exitCode);
}


void
Usage(void) {
    //80->  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    printf("%s\n", APPNAME);
    printf("  -h(--help)                          Display this help\n");
    printf("  -v(--rev) <spec>                    All options forced to target specified\n");
    printf("                                      NVME revision {1.0 | 1.0a}; dflt=1.0a\n");
    printf("  -s(--summary)                       Summarize all groups and tests\n");
    printf("  -d(--detail) [<grp> | <grp>:<test>] Detailed group and test description for:\n");
    printf("                                      {all | spec'd_group | test_within_group}\n");
    printf("  -t(--test) [<grp> | <grp>:<test>]   Execute tests for:\n");
    printf("                                      {all | spec'd_group | test_within_group}\n");
    printf("  -r(--reset) {<pci> | <ctrlr>}       Reset the device\n");
    printf("  -i(--ignore)                        Ignore detected errors; run subsequent\n");
    printf("                                      tests even if failures detected;\n");
    printf("  -l(--loop) <count>                  Loop test execution <count> times; dflt=1\n");
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
            LOG_ERR("Unrecognized --detail <grp>:<test>=%s", optarg);
            return (false);
        }

    } else {
        // Specified format <grp>:<test>
        target.group = strtoul(
            swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != ':') {
            LOG_ERR("Missing ':' character in format string");
            return (false);
        }

        // Find major piece of <test>
        swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Missing <test> format string");
            return (false);
        }

        target.major = strtoul(
            swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '.') {
            LOG_ERR("Missing '.' character in format string");
            return (false);
        }

        // Find minor piece of <test>
        swork = swork.substr(swork.find_first_of('.') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Unrecognized --detail <grp>:<test>=%s", optarg);
            return (false);
        }

        target.minor = strtoul(
            swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '\0') {
            LOG_ERR("Unrecognized --detail <grp>:<test>=%s", optarg);
            return (false);
        }
    }

    if (target.group == ULONG_MAX) {
        LOG_ERR("Unrecognized --detail <grp>=%s", optarg);
        return (false);
    } else if (((target.major == ULONG_MAX) && (target.minor != ULONG_MAX)) ||
               ((target.major != ULONG_MAX) && (target.minor == ULONG_MAX))) {
        LOG_ERR("Unrecognized --detail <grp>:<test>=%s", optarg);
        return (false);
    }

    return (true);
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

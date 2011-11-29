#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include "tnvmeHelpers.h"
#include "globals.h"
#include "Utils/kernelAPI.h"


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
    size_t iLoop;
    int numPassed = 0;
    int numFailed = 0;
    int numSkipped = 0;
    bool allTestsPass = true;    // assuming success until we find otherwise
    bool thisTestPass;
    TestIteratorType testIter;


    if ((cl.test.t.group != UINT_MAX) && (cl.test.t.group >= groups.size())) {
        LOG_ERR("Specified test group does not exist");
        return false;
    }

    for (iLoop = 0; iLoop < cl.loop; iLoop++) {
        LOG_NRM("Start loop execution #%ld", iLoop);

        for (size_t iGrp = 0; iGrp < groups.size(); iGrp++) {
            bool allHaveRun = false;

            // Handle the --informative cmd line option
            if (cl.informative.req && (iGrp == cl.informative.grpInfoIdx)) {
                // Each test group starts from known starting point
                if (KernelAPI::SoftReset() == false)
                    return false;

                LOG_NRM("Cmd line forces executing informative group");
                testIter = groups[iGrp]->GetTestIterator();
                while (1) {
                    if (groups[iGrp]->RunTest(testIter, cl.skiptest)
                        == Group::TR_NOTFOUND) {
                        break;
                    }
                }
                // No need to do soft reset, Informative group is non-intrusive
            }


            // Now handle anything spec'd in the --test <cmd line option>
            if (cl.test.t.group == UINT_MAX) {
                // Each test group starts from known starting point
                if (KernelAPI::SoftReset() == false)
                    return false;

                // Run all tests within all groups
                testIter = groups[iGrp]->GetTestIterator();
                while (allHaveRun == false) {
                    thisTestPass = true;

                    switch (groups[iGrp]->RunTest(testIter, cl.skiptest)) {
                    case Group::TR_SUCCESS:
                        numPassed++;
                        break;
                    case Group::TR_FAIL:
                        allTestsPass = false;
                        thisTestPass = false;
                        numFailed++;
                        break;
                    case Group::TR_SKIPPING:
                        numSkipped++;
                        break;
                    case Group::TR_NOTFOUND:
                        allHaveRun = true;
                        break;
                    }
                    if ((cl.ignore == false) && (allTestsPass == false)) {
                        goto FAIL_OUT;
                    } else if (cl.ignore && (thisTestPass == false)) {
                        LOG_NRM("Detected error, but forced to ignore");
                        break;  // continue with next test in next group
                    }
                }

            } else if ((cl.test.t.major == UINT_MAX) ||
                       (cl.test.t.minor == UINT_MAX)) {

                // Run all tests within spec'd group
                if (iGrp == cl.test.t.group) {
                    // Each test group starts from known starting point
                    if (KernelAPI::SoftReset() == false)
                        return false;

                    // Run all tests within this group
                    testIter = groups[iGrp]->GetTestIterator();
                    while (allHaveRun == false) {
                        thisTestPass = true;

                        switch (groups[iGrp]->RunTest(testIter, cl.skiptest)) {
                        case Group::TR_SUCCESS:
                            numPassed++;
                            break;
                        case Group::TR_FAIL:
                            allTestsPass = false;
                            thisTestPass = false;
                            numFailed++;
                            break;
                        case Group::TR_SKIPPING:
                            numSkipped++;
                            break;
                        case Group::TR_NOTFOUND:
                            allHaveRun = true;
                            break;
                        }
                        if ((cl.ignore == false) && (allTestsPass == false)) {
                            goto FAIL_OUT;
                        } else if (cl.ignore && (thisTestPass == false)) {
                            LOG_NRM("Detected error, but forced to ignore");
                            break;  // continue with next test in next group
                        }
                    }
                    break;  // check if more loops must occur
                }

            } else {
                // Run spec'd test within spec'd group
                if (iGrp == cl.test.t.group) {
                    // Each test run starts from known starting point
                    if (KernelAPI::SoftReset() == false)
                        return false;

                    switch (groups[iGrp]->RunTest(cl.test.t, cl.skiptest)) {
                    case Group::TR_SUCCESS:
                        numPassed++;
                        break;
                    case Group::TR_FAIL:
                        allTestsPass = false;
                        numFailed++;
                        break;
                    case Group::TR_SKIPPING:
                        numSkipped++;
                        break;
                    case Group::TR_NOTFOUND:
                        LOG_DBG("Internal programming error, unknown test");
                        break;
                    }
                    if ((cl.ignore == false) && (allTestsPass == false))
                        goto FAIL_OUT;

                    break;  // check if more loops must occur
                }
            }
        }

        LOG_NRM("Iteration SUMMARY passed : %d", numPassed);
        if (numFailed) {
            LOG_NRM("                  failed : %d  <----------", numFailed);
        } else {
            LOG_NRM("                  failed : 0");
        }
        LOG_NRM("                  skipped: %d", numSkipped);
        LOG_NRM("                  total  : %d",
            numPassed+numFailed+numSkipped);
        LOG_NRM("Stop loop execution #%ld", iLoop);
    }

    return allTestsPass;

FAIL_OUT:
    LOG_NRM("Tests  passed: %d", numPassed);
    if (numFailed) {
        LOG_NRM("       failed: %d    <--------------", numFailed);
    } else {
        LOG_NRM("       failed: 0");
    }
    LOG_NRM("      skipped: %d", numSkipped);
    LOG_NRM("        total: %d", numPassed+numFailed+numSkipped);
    LOG_NRM("Stop loop execution #%ld", iLoop);
    return allTestsPass;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "--skiptest <filename>".
 * @param target Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseSkipTestCmdLine(vector<TestRef> &skipTest, const char *optarg)
{
    int fd;
    ssize_t numRead;
    char buffer[80];
    string contents;
    string line;
    TestTarget tmp;


    if ((fd = open(optarg, O_RDWR)) == -1) {
        LOG_ERR("file=%s: %s", optarg, strerror(errno));
        return false;
    }
    while ((numRead = read(fd, buffer, (sizeof(buffer)-1))) != -1) {
        if (numRead == 0)
            break;
        buffer[numRead] = '\0';
        contents += buffer;
    }
    if (numRead == -1) {
        LOG_ERR("file=%s: %s", optarg, strerror(errno));
        return false;
    }

    // Parse and make sense of the file's contents
    for (size_t i = 0; i < contents.size(); i++) {
        bool processThisLine = false;

        if (contents[i] == '#') {
            // Comment start; goes until the end of this line
            for (size_t j = i; j < contents.size(); j++) {
                if ((contents[j] == '\n') || (contents[j] == '\r')) {
                    i = j;
                    processThisLine = true;
                    break;
                }
            }
        } else if ((contents[i] == '\n') || (contents[i] == '\r')) {
            processThisLine = true;
        } else {
            if (isalnum(contents[i]) ||
                (contents[i] == '.') ||
                (contents[i] == ':')) {
                line += contents[i];
            }
        }

        if (processThisLine) {
            if (line.size()) {
                if (ParseTargetCmdLine(tmp, line.c_str()) == false) {
                    close(fd);
                    return false;
                }
                TestRef skipThis = tmp.t;
                skipTest.push_back(skipThis);
                line = "";
            }
        }
    }

    // Report what tests will be skipped
    string output = "Execution will skip test case(s): <grp>:<major>.<minor>=";
    char work[20];
    for (size_t i = 0; i < skipTest.size(); i++ ) {
        if ((skipTest[i].major == UINT_MAX) ||
            (skipTest[i].minor == UINT_MAX)) {
            snprintf(work, sizeof(work), "%ld:ALL.ALL, ", skipTest[i].group);
        } else {
            snprintf(work, sizeof(work), "%ld:%ld.%ld, ",
                skipTest[i].group, skipTest[i].major, skipTest[i].minor);
        }
        output += work;
    }
    LOG_NRM("%s", output.c_str());

    close(fd);
    return true;
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
ParseTargetCmdLine(TestTarget &target, const char *optarg)
{
    size_t ulwork;
    char *endptr;
    string swork;
    long tmp;


    target.req = true;
    target.t.group = UINT_MAX;
    target.t.major = UINT_MAX;
    target.t.minor = UINT_MAX;
    if (optarg == NULL)
        return true;

    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        // Specified format <grp>
        tmp = strtol(swork.c_str(), &endptr, 10);
        if (*endptr != '\0') {
            LOG_ERR("Unrecognized format <grp>=%s", optarg);
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<grp> values < 0 are not supported");
            return false;
        }
        target.t.group = tmp;

    } else {
        // Specified format <grp>:<test>
        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != ':') {
            LOG_ERR("Missing ':' character in format string");
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<grp> values < 0 are not supported");
            return false;
        }
        target.t.group = tmp;

        // Find major piece of <test>
        swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Missing <test> format string");
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '.') {
            LOG_ERR("Missing '.' character in format string");
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<major> values < 0 are not supported");
            return false;
        }
        target.t.major = tmp;

        // Find minor piece of <test>
        swork = swork.substr(swork.find_first_of('.') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Unrecognized format <grp>:<major>.<minor>=%s", optarg);
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '\0') {
            LOG_ERR("Unrecognized format <grp>:<major>.<minor>=%s", optarg);
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<minor> values < 0 are not supported");
            return false;
        }
        target.t.minor = tmp;
    }

    if (target.t.group == UINT_MAX) {
        LOG_ERR("Unrecognized format <grp>=%s\n", optarg);
        return false;
    } else if (((target.t.major == UINT_MAX) && (target.t.minor != UINT_MAX)) ||
               ((target.t.major != UINT_MAX) && (target.t.minor == UINT_MAX))) {
        LOG_ERR("Unrecognized format <grp>:<major>.<minor>=%s", optarg);
        return false;
    }

    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<space:offset:num:acc>".
 * @param rmmap Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseRmmapCmdLine(RmmapIo &rmmap, const char *optarg)
{
    size_t ulwork;
    char *endptr;
    string swork;
    size_t tmp;
    string sacc;


    rmmap.req = true;
    rmmap.space = NVMEIO_FENCE;
    rmmap.offset = 0;
    rmmap.size = 0;
    rmmap.acc = ACC_FENCE;

    LOG_DBG("Option selected = %s", optarg);

    // Parsing <space:offset:size>
    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        LOG_ERR("Unrecognized format <space:off:size:acc>=%s", optarg);
        return false;
    }
    if (strcmp("PCI", swork.substr(0, ulwork).c_str()) == 0) {
        rmmap.space = NVMEIO_PCI_HDR;
    } else if (strcmp("BAR01", swork.substr(0, ulwork).c_str()) == 0) {
        rmmap.space = NVMEIO_BAR01;
    } else {
        LOG_ERR("Unrecognized identifier <space>=%s", optarg);
        return false;
    }

    // Parsing <off:size:acc>
    swork = swork.substr(ulwork+1, swork.size());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <off:size:acc>=%s", optarg);
        return false;
    }
    rmmap.offset = tmp;

    // Parsing <size:acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <size> format string");
        return false;
    }
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <size:acc>=%s", optarg);
        return false;
    }
    rmmap.size = tmp;

    // Parsing <acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <acc> format string");
        return false;
    }

    sacc = swork.substr(0, swork.size()).c_str();
    tmp = strtoul(swork.substr(1, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
    	LOG_ERR("Unrecognized format <acc>=%s", optarg);
        return false;
    }
    // Detect the access width passed.
    if (sacc.compare("b") == 0) {
    	rmmap.acc = BYTE_LEN;
    } else if (sacc.compare("w") == 0) {
    	rmmap.acc = WORD_LEN;
    } else if (sacc.compare("l") == 0) {
    	rmmap.acc = DWORD_LEN;
    } else if (sacc.compare("q") == 0) {
    	rmmap.acc = QUAD_LEN;
    } else {
    	LOG_ERR("Unrecognized access width to read.");
    	return false;
    }

    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<space:offset:num:val:acc>".
 * @param wmmap Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseWmmapCmdLine(WmmapIo &wmmap, const char *optarg)
{
    size_t ulwork;
    char *endptr;
    string swork;
    size_t tmp;
    uint64_t tmpVal;
    string sacc;

    wmmap.req = true;
    wmmap.space = NVMEIO_FENCE;
    wmmap.offset = 0;
    wmmap.size = 0;
    wmmap.value = 0;
    wmmap.acc = ACC_FENCE;

    // Parsing <space:off:siz:val:acc>
    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        LOG_ERR("Unrecognized format <space:off:siz:val:acc>=%s", optarg);
        return false;
    }
    if (strcmp("PCI", swork.substr(0, ulwork).c_str()) == 0) {
        wmmap.space = NVMEIO_PCI_HDR;
    } else if (strcmp("BAR01", swork.substr(0, ulwork).c_str()) == 0) {
        wmmap.space = NVMEIO_BAR01;
    } else {
        LOG_ERR("Unrecognized identifier <space>=%s", optarg);
        return false;
    }

    // Parsing <off:siz:val:acc>
    swork = swork.substr(ulwork+1, swork.size());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <off:siz:val:acc>=%s", optarg);
        return false;
    }
    wmmap.offset = tmp;

    // Parsing <size:val:acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <siz:val:acc>=%s", optarg);
        return false;
    } else if (tmp > MAX_SUPPORTED_REG_SIZE) {
        LOG_ERR("<size> > allowed value of max of %d bytes",
            MAX_SUPPORTED_REG_SIZE);
        return false;
    }
    wmmap.size = tmp;

    // Parsing <val:acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <val> format string");
        return false;
    }
    tmpVal = strtoull(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <val:acc>=%s", optarg);
        return false;
    }
    wmmap.value = tmpVal;

    // Parsing <acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <acc> format string");
        return false;
    }

    sacc = swork.substr(0, swork.size()).c_str();
    tmp = strtoul(swork.substr(1, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
        LOG_ERR("Unrecognized format <acc>=%s", optarg);
        return false;
    }

    if (sacc.compare("b") == 0) {
    	wmmap.acc = BYTE_LEN;
    } else if (sacc.compare("w") == 0) {
    	wmmap.acc = WORD_LEN;
    } else if (sacc.compare("l") == 0) {
    	wmmap.acc = DWORD_LEN;
    } else if (sacc.compare("q") == 0) {
    	wmmap.acc = QUAD_LEN;
    } else {
    	LOG_ERR("Unrecognized access width for writing.");
    	return false;
    }

    return true;
}

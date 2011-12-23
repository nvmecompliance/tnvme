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
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include "tnvmeHelpers.h"
#include "globals.h"
#include "Utils/kernelAPI.h"
#include "Cmds/setFeatures.h"

#define INFORMATIVE_GRPNUM          0


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

            // Always run the Informative group 1st, i.e. 0 always runs 1st
            if (iGrp == INFORMATIVE_GRPNUM) {
                LOG_NRM("Executing a new group, start from known point");
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
                        break;  // don't execute any more within this group
                    }
                }
                continue;   // continue with next test in next group
            }


            // Now handle anything spec'd in the --test <cmd line option>
            if (cl.test.t.group == UINT_MAX) {
                // Do not run Informative group, that group always runs above
                if (iGrp == INFORMATIVE_GRPNUM)
                    break;  // continue with next test in next group

                LOG_NRM("Executing a new group, start from known point");
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

                // Run all tests within spec'd group, except Informative grp
                if ((iGrp == cl.test.t.group) && (iGrp != INFORMATIVE_GRPNUM)) {
                    LOG_NRM("Executing a new group, start from known point");
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
                // Run spec'd test within spec'd group, except Informative grp
                if ((iGrp == cl.test.t.group) && (iGrp != INFORMATIVE_GRPNUM)) {
                    LOG_NRM("Executing a new group, start from known point");
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


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<ncqr:nsqr>".
 * @param queues Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseQueuesCmdLine(Queues &queues, const char *optarg)
{
    char *endptr;
    string swork;
    size_t tmp;
    string sacc;

    queues.req = true;
    queues.ncqr = 0;
    queues.nsqr = 0;

    // Parsing <ncqr:nsqr>
    swork = optarg;
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <ncqr:nsqr>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<ncqr> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    queues.ncqr = (uint16_t)tmp;

    // Parsing <nsqr>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <nsqr> format string");
        return false;
    }
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
        LOG_ERR("Unrecognized format <nsqr>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<nsqr> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    queues.nsqr = (uint16_t)tmp;

    return true;
}


bool SetFeaturesNumberOfQueues(Queues &queues, int fd)
{
    uint16_t numCE;
    uint16_t ceRemain;
    uint16_t numReaped;

    try {   // The objects to perform this work throw exceptions
        LOG_NRM("Setting number of Q's; ncqr=0x%04X, nsqr=0x%04X",
            queues.ncqr, queues.nsqr);
        if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
            throw exception();

        LOG_NRM("Prepare the admin Q's to setup this request");
        SharedACQPtr acq = SharedACQPtr(new ACQ(fd));
        acq->Init(2);
        SharedASQPtr asq = SharedASQPtr(new ASQ(fd));
        asq->Init(2);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw exception();

        LOG_NRM("Create the cmd to carry this data to the DUT");
        SharedSetFeaturesPtr sfNumOfQ =
            SharedSetFeaturesPtr(new SetFeatures(fd));
        sfNumOfQ->SetFID(BaseFeatures::FID_NUM_QUEUES);
        sfNumOfQ->SetNumberOfQueues(queues.ncqr, queues.nsqr);

        LOG_NRM("Send the cmd to the ASQ, wait for it to complete");
        asq->Send(sfNumOfQ);
        asq->Ring();
        if (acq->ReapInquiryWaitSpecify(2000, 1, numCE) == false) {
            LOG_ERR("Unable to see completion of Set Features cmd");
            throw exception();
        } else if (numCE != 1) {
            LOG_ERR("The ACQ should only have 1 CE as a result of a cmd");
            throw exception();
        }

        LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        KernelAPI::LogCQMetrics(acqMetrics);

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOCQ, numCE, true)) != 1) {
            LOG_ERR("Verified there was 1 CE, but reaping produced %d",
                numReaped);
            throw exception();
        }
        LOG_NRM("The reaped get features CE is...");
        acq->LogCE(acqMetrics.head_ptr);

        union CE ce = acq->PeekCE(acqMetrics.head_ptr);
        if (ce.n.status != 0) {
            LOG_ERR("CE shows cmd failed: status = 0x%02X", ce.n.status);
            throw exception();
        }
        printf("The operation succeeded to set number of queues\n");
    } catch (...) {
        printf("Operation failed to set number of queues\n");
        return false;
    }

    return true;
}

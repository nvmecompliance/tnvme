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
#include "Queues/ce.h"
#include "Cmds/setFeatures.h"
#include "Exception/frmwkEx.h"


/**
 * Verify the spec'd spec revision is what the DUT/hdw is reporting
 * @note Requires singleton gRegisters to exist to perform job.
 * @param specRev Pass the revision to compare against what hdw is reporting
 * @return true upon validation, otherwise false.
 */
bool
VerifySpecCompatibility(SpecRev specRev)
{
    uint64_t versionReg;
    uint16_t tgtMajor, tgtMinor, hdwMajor, hdwMinor;


    if (gRegisters->Read(CTLSPC_VS, versionReg) == false)
        return false;
    hdwMajor = (uint16_t)(versionReg >> 16);
    hdwMinor = (uint16_t)(versionReg >> 0);

    switch (specRev) {
    case SPECREV_10b:   tgtMajor = 1;  tgtMinor = 0;  break;
    default:
        LOG_ERR("Requesting comparison against unknown SpecRev=%d", specRev);
        return false;
    }

    if ((tgtMajor != hdwMajor) || (tgtMinor != hdwMinor)) {
        LOG_ERR("(Targeted vs hdw) spec rev incompatibility (%d.%d != %d.%d)",
            tgtMajor, tgtMinor, hdwMajor, hdwMinor);
        return false;
    }
    return true;
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
    string output = "Execution will skip test case(s): ";
    char work[20];
    for (size_t i = 0; i < skipTest.size(); i++ ) {
        if ((skipTest[i].xLev == UINT_MAX) ||
            (skipTest[i].yLev == UINT_MAX) ||
            (skipTest[i].zLev == UINT_MAX)) {

            snprintf(work, sizeof(work), "%ld:ALL.ALL.ALL, ",
                skipTest[i].group);
        } else {
            snprintf(work, sizeof(work), "%ld:%ld.%ld.%ld, ",
                skipTest[i].group, skipTest[i].xLev, skipTest[i].yLev,
                skipTest[i].zLev);
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
    string swork;
    char *endptr;
    long tmp;

    target.req = true;
    target.t.group = UINT_MAX;
    target.t.xLev = UINT_MAX;
    target.t.yLev = UINT_MAX;
    target.t.zLev = UINT_MAX;

    if (optarg == NULL) {
        // User specified to run all test within all all groups
        return true;
    }

    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        // User specified format <grp> only
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

        // Find xLev component of <test>
        swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Missing <test> format string");
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '.') {
            LOG_ERR("Missing 1st '.' character in format string");
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<x> values < 0 are not supported");
            return false;
        }
        target.t.xLev = tmp;

        // Find yLev component of <test>
        swork = swork.substr(swork.find_first_of('.') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Unrecognized format <grp>:<x>.<y>=%s", optarg);
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '.') {
            LOG_ERR("Missing 2nd '.' character in format string");
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<y> values < 0 are not supported");
            return false;
        }
        target.t.yLev = tmp;

        // Find zLev component of <test>
        swork = swork.substr(swork.find_first_of('.') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Unrecognized format <grp>:<x>.<y>.<z>=%s", optarg);
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '\0') {
            LOG_ERR("Unrecognized format <grp>:<x>.<y>.<z>=%s", optarg);
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<z> values < 0 are not supported");
            return false;
        }
        target.t.zLev = tmp;
    }
;
    if (target.t.group == UINT_MAX) {
        LOG_ERR("Unrecognized format <grp>=%s\n", optarg);
        return false;
    } else if (!(((target.t.xLev == UINT_MAX)  &&
                  (target.t.yLev == UINT_MAX)  &&
                  (target.t.zLev == UINT_MAX)) ||
                 ((target.t.xLev != UINT_MAX)  &&
                  (target.t.yLev != UINT_MAX)  &&
                  (target.t.zLev != UINT_MAX)))) {
        LOG_ERR("Unrecognized format <grp>:<x>.<y>.<z>=%s", optarg);
        LOG_ERR("Parsed and decoded: <%ld>:<%ld>.<%ld>.<%ld>",
            target.t.group, target.t.xLev, target.t.yLev, target.t.zLev);
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

    // Parsing <space:off:size:val:acc>
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

    // Parsing <off:size:val:acc>
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
 * "<STS:PXDS:AERUCES:CSTS>".
 * @param errRegs Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseErrorCmdLine(ErrorRegs &errRegs, const char *optarg)
{
    char *endptr;
    string swork;
    size_t tmp;
    string sacc;

    errRegs.sts = 0;
    errRegs.pxds = 0;
    errRegs.aeruces = 0;
    errRegs.csts = 0;

    // Parsing <STS:PXDS:AERUCES:CSTS>
    swork = optarg;
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <STS:PXDS:AERUCES:CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<STS> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    errRegs.sts = (uint16_t)tmp;

    // Parsing <PXDS:AERUCES:CSTS>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <PXDS:AERUCES:CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<PXDS> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    errRegs.pxds = (uint16_t)tmp;

    // Parsing <AERUCES:CSTS>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <AERUCES:CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<AERUCES> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    errRegs.aeruces = (uint16_t)tmp;

    // Parsing <CSTS>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
        LOG_ERR("Unrecognized format <CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<CSTS> > allowed max value of 0x%08X", ((uint32_t)(-1)));
        return false;
    }
    errRegs.csts = (uint16_t)tmp;

    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<ncqr:nsqr>".
 * @param numQueues Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseQueuesCmdLine(NumQueues &numQueues, const char *optarg)
{
    char *endptr;
    string swork;
    size_t tmp;
    string sacc;

    numQueues.req = true;
    numQueues.ncqr = 0;
    numQueues.nsqr = 0;

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
    numQueues.ncqr = (uint16_t)tmp;

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
    numQueues.nsqr = (uint16_t)tmp;

    return true;
}


/**
 * A function to send Set Features admin cmd to a device sending the identifier
 * 0x07 to set number of queues. This value, according to the spec, shall NOT
 * change between resets, i.e. only set this value after a power up. Failure to
 * conform results in undefined behavior.
 * "<STS:PXDS:AERUCES:CSTS>".
 * @param numQueues Pass a structure to source the desired values to send to hdw
 * @param fd Pass file descriptor representing device to commune with
 * @return true upon successful parsing, otherwise false.
 */
bool
SetFeaturesNumberOfQueues(NumQueues &numQueues, int fd)
{
    uint32_t numCE;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t isrCount;


    try {   // The objects to perform this work throw exceptions
        LOG_NRM("Setting number of Q's; ncqr=0x%04X, nsqr=0x%04X",
            numQueues.ncqr, numQueues.nsqr);
        if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
            throw FrmwkEx();

        LOG_NRM("Prepare the admin Q's to setup this request");
        SharedACQPtr acq = SharedACQPtr(new ACQ(fd));
        acq->Init(2);
        SharedASQPtr asq = SharedASQPtr(new ASQ(fd));
        asq->Init(2);
        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw FrmwkEx();

        LOG_NRM("Create the cmd to carry this data to the DUT");
        SharedSetFeaturesPtr sfNumOfQ =
            SharedSetFeaturesPtr(new SetFeatures());
        sfNumOfQ->SetFID(BaseFeatures::FID_NUM_QUEUES);
        sfNumOfQ->SetNumberOfQueues(numQueues.ncqr, numQueues.nsqr);

        LOG_NRM("Send the cmd to the ASQ, wait for it to complete");
        asq->Send(sfNumOfQ);
        asq->Ring();
        if (acq->ReapInquiryWaitSpecify(SYSTEMWIDE_CMD_WAIT_ms, 1, numCE,
            isrCount) == false) {
            throw FrmwkEx("Unable to see completion of Set Features cmd");
        }
        else if (numCE != 1)
            throw FrmwkEx("The ACQ should only have 1 CE as a result of a cmd");

        LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        KernelAPI::LogCQMetrics(acqMetrics);

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemIOCQ, isrCount, numCE, true))
            != 1) {

            throw FrmwkEx("Verified there was 1 CE, but reaping produced %d",
                numReaped);
        }
        LOG_NRM("The reaped get features CE is...");
        acq->LogCE(acqMetrics.head_ptr);

        union CE ce = acq->PeekCE(acqMetrics.head_ptr);
        ProcessCE::Validate(ce);  // throws upon error
        printf("The operation succeeded to set number of queues\n");
    } catch (...) {
        printf("Operation failed to set number of queues\n");
        return false;
    }

    return true;
}

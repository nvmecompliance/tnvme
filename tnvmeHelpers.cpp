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

#include "tnvmeHelpers.h"
#include "globals.h"
#include "Utils/kernelAPI.h"
#include "Queues/ce.h"
#include "Cmds/setFeatures.h"
#include "Cmds/formatNVM.h"
#include "Utils/io.h"
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
 * A function to issue consecutive admin cmd set format NVM cmds to a DUT.
 * @param format Pass formating instructions
 * @param fd Pass file descriptor representing device to commune with
 * @return true upon successful parsing, otherwise false.
 */
bool
FormatDevice(Format &format, int fd)
{
    try {   // The objects to perform this work throw exceptions

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

        for (size_t i = 0; i < format.cmds.size(); i++) {
            LOG_NRM("Formatting namespace: %d", format.cmds[i].nsid);
            LOG_NRM("  FormatNVM:DW10.ses = 0x%02x", format.cmds[i].ses);
            LOG_NRM("  FormatNVM:DW10.pil = %c", format.cmds[i].pil ? 'T' :'F');
            LOG_NRM("  FormatNVM:DW10.pi = 0x%02x", format.cmds[i].pi);
            LOG_NRM("  FormatNVM:DW10.ms = %c", format.cmds[i].ms ? 'T' : 'F');
            LOG_NRM("  FormatNVM:DW10.lbaf = 0x%02x", format.cmds[i].lbaf);

            LOG_NRM("Create the cmd to carry this data to the DUT");
            SharedFormatNVMPtr formatNVM =
                SharedFormatNVMPtr(new FormatNVM());
            formatNVM->SetNSID(format.cmds[i].nsid);
            formatNVM->SetSES(format.cmds[i].ses);
            formatNVM->SetPIL(format.cmds[i].pil);
            formatNVM->SetPI(format.cmds[i].pi);
            formatNVM->SetMS(format.cmds[i].ms);
            formatNVM->SetLBAF(format.cmds[i].lbaf);

            IO::SendCmdToHdw("tnvme", "format", SYSTEMWIDE_CMD_WAIT_ms,
                asq, acq, formatNVM, "", true);
        }
        printf("The operation succeeded to format device\n");
    } catch (...) {
        printf("Operation failed to format device\n");
        return false;
    }

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

        IO::SendCmdToHdw("tnvme", "queues", SYSTEMWIDE_CMD_WAIT_ms,
            asq, acq, sfNumOfQ, "", true);
        printf("The operation succeeded to set number of queues\n");
    } catch (...) {
        printf("Operation failed to set number of queues\n");
        return false;
    }

    return true;
}

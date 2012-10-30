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

#include "irq.h"
#include "globals.h"


IRQ::IRQ()
{
}


IRQ::~IRQ()
{
}


bool
IRQ::VerifyAnySchemeSpecifyNum(uint16_t numIrqsDesire, enum nvme_irq_type &irq)
{
    bool capable;
    uint16_t numIrqsSupported;
    irq = INT_NONE;     // assume DUT doesn't support request


    // Can we satisfy the request using the MSI-X IRQ scheme?
    if (gCtrlrConfig->IsMSIXCapable(capable, numIrqsSupported) == false)
        throw FrmwkEx(HERE);
    else if (capable) {
        if (numIrqsDesire > numIrqsSupported) {
            LOG_NRM("Desire %d IRQs, but DUT only supports %d MSI-X IRQ's",
                numIrqsDesire, numIrqsSupported);
        } else {
            irq = INT_MSIX;
            return true;
        }
    }

    // Can we satisfy the request using the MSI IRQ scheme?
    if (gCtrlrConfig->IsMSICapable(capable, numIrqsSupported) == false)
        throw FrmwkEx(HERE);
    else if (capable) {
        if (numIrqsDesire > numIrqsSupported) {
            LOG_NRM("Desire %d IRQs, but DUT only supports %d MSI IRQ's",
                numIrqsDesire, numIrqsSupported);
        } else {
            irq = (numIrqsSupported == 1) ? INT_MSI_SINGLE : INT_MSI_MULTI;
            return true;
        }
    }
    LOG_ERR("DUT not reporting any IRQ scheme satisfying %d IRQ's",
        numIrqsDesire);
    return false;
}


void
IRQ::SetAnySchemeSpecifyNum(uint16_t numIrqsDesire)
{
    enum nvme_irq_type irqScheme;

    if (VerifyAnySchemeSpecifyNum(numIrqsDesire, irqScheme) == false)
        throw FrmwkEx(HERE);

    if (gCtrlrConfig->SetIrqScheme(irqScheme, numIrqsDesire) == false)
        throw FrmwkEx(HERE);
}


uint16_t
IRQ::GetMaxIRQsSupportedAnyScheme()
{
    uint16_t max_ivec = 0;
    enum nvme_irq_type irq;
    bool capable;

    if (gCtrlrConfig->GetIrqScheme(irq, max_ivec) == false)
        throw FrmwkEx(HERE, "Unable to retrieve current IRQ scheme");

    switch (irq) {
    case INT_MSI_MULTI:
        // Get the max ivec with MSI-Mutli scheme.
        if (gCtrlrConfig->IsMSICapable(capable, max_ivec) == false)
            throw FrmwkEx(HERE);
        LOG_NRM("MSI-Multi IRQ scheme with max ivec = %d", max_ivec);
        break;
    case INT_MSIX:
        // Get the max ivec with MSI-X scheme.
        if (gCtrlrConfig->IsMSIXCapable(capable, max_ivec) == false)
            throw FrmwkEx(HERE);
        LOG_NRM("MSI-X IRQ scheme with max ivec = %d", max_ivec);
        break;
    case INT_MSI_SINGLE:
    case INT_NONE:
        max_ivec = 0;   // Required to be zero
        LOG_NRM("NONE/MSI-S scheme with max ivec = %d", max_ivec);
        break;
    default:
        throw FrmwkEx(HERE, "Unsupported IRQ scheme, what to do?");
    }
    return max_ivec;
}

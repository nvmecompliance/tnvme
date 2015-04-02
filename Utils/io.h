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

#ifndef _IO_H_
#define _IO_H_

#include "tnvme.h"
#include "../Queues/ce.h"
#include "../Queues/sq.h"
#include "../Queues/cq.h"


/**
* This class is meant not be instantiated because it should only ever contain
* static members. These utility functions can be viewed as wrappers to
* perform common, repetitious tasks which avoids coping the same chunks of
* code throughout the framework.
*
* @note This class may throw exceptions, please see comment within specific
*       methods.
*/
class IO
{
public:
    IO();
    virtual ~IO();

    /**
     * Send and Reap an existing, user defined cmd to/from hdw using the spec'd
     * SQ/CQ pairs. This method requires 0 elements to reside in the CQ and
     * also assume no other cmd will complete into that CQ while this operation
     * is occurring. In the end the number of CE's will be verified to
     * guarantee that only 1 CE arrived as a result of sending this 1 cmd.
     * @note Throws upon errors
     * @note Method uses pre-existing values of CC.IOCQES
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param sq Pass pre-existing SQ to issue a cmd into
     * @param cq Pass pre-existing CQ to reap CE to verify successful creation
     * @param cmd Pass the cmd to issue into the supplied CQ
     * @param qualify Pass a qualifying string to append to each dump file
     * @param status Pass the expected status to verify with
     * @param verbose Pass true to dump resources to dump files, otherwise false
     * @return matching status if successful; throws exception otherwise;
     */
    static CEStat SendAndReapCmd(string grpName, string testName, uint16_t ms,
        SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd, string qualify,
        bool verbose, CEStat status = CESTAT_SUCCESS);
    static CEStat SendAndReapCmd(string grpName, string testName, uint16_t ms,
        SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd, string qualify,
        bool verbose, std::vector<CEStat> &status);

    /**
     * Send and Reap an existing, user defined cmd to/from hdw using the spec'd
     * SQ/CQ pairs. This method requires 0 elements to reside in the CQ and
     * alse assume no other cmd will complete into that CQ while this operation
     * is occurring. In the end the number of CE's will be verified to
     * guarantee that only 1 CE arived as a result of sending this 1 cmd.
     * @note Throws upon errors
     * @note Method uses pre-existing values of CC.IOCQES
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @oaran sq Pass pre-existing SQ to issue a cmd into
     * @param cq Pass pre-existing CQ to reap CE to verify successful creation
     * @param cmd Pass the cmd to issue into the supplied CQ
     * @param qualify Pass a quialifying string to append to each dump file
     * @param verbose Pass true to dump resources to dump files, otherwise false
     * @param status Pass the unexpected status to verify with
     * @return status of cmd if successful; throws exception otherwise;
     */
    static CEStat SendAndReapCmdNot(string grpName, string testName,
        uint16_t ms,SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd,
        string qualify, bool verbose, CEStat status = CESTAT_SUCCESS);
    static CEStat SendAndReapCmdNot(string grpName, string testName,
        uint16_t ms,SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd,
        string qualify, bool verbose, std::vector<CEStat> &status);

    /**
     * Send and Reap an existing, user defined cmd to/from hdw using the spec'd
     * SQ/CQ pairs. This method requires 0 elements to reside in the CQ and
     * also assume no other cmd will complete into that CQ while this operation
     * is occurring. In the end the number of CE's will be verified to
     * guarantee that only 1 CE arrived as a result of sending this 1 cmd.
     * @note Does not throw
     * @note Method uses pre-existing values of CC.IOCQES
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param sq Pass pre-existing SQ to issue a cmd into
     * @param cq Pass pre-existing CQ to reap CE to verify successful creation
     * @param cmd Pass the cmd to issue into the supplied CQ
     * @param qualify Pass a qualifying string to append to each dump file
     * @param verbose Pass true to dump resources to dump files, otherwise false
     * @return ce status
     */
    static CEStat SendAndReapCmdIgnore(string grpName, string testName,
        uint16_t ms, SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd,
        string qualify, bool verbose);

    /**
     * Send and verify that no Reap occurs for an existing, user defined cmd
     * to/from hdw using the spec'd SQ/CQ pairs. This method requires 0
     * elements to reside in the CQ and also assume no other cmd will complete
     * into that CQ while this operation is occurring. In the end the number of
     * CE's will be verified to guarantee that no CE arrived as a result of
     * sending this 1 cmd.
     * @note Does not throw
     * @note Method uses pre-existing values of CC.IOCQES
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param sq Pass pre-existing SQ to issue a cmd into
     * @param cq Pass pre-existing CQ to reap CE to verify successful creation
     * @param cmd Pass the cmd to issue into the supplied CQ
     * @param qualify Pass a qualifying string to append to each dump file
     * @param verbose Pass true to dump resources to dump files, otherwise false
     * @return ce status
     */
    static void SendAndReapCmdFail(string grpName, string testName,
        uint16_t ms, SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd,
        string qualify, bool verbose);

    /**
     * Verifies the status of the reaped CE.
     * @param ce The reaped CE
     * @param status Pass the expected status to verify with
     */
    static CEStat VerifyCE(union CE *ce, std::vector<CEStat> &status);

    /**
     * Reap a specified number of CE's from the specified CQ.
     * @note Throws upon errors
     * @param cq Pass the CQ to reap from
     * @param numCE Pass the number of CE's to reap from the CQ
     * @param isrCount Return the ISR count as a result of reaping
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param qualify Pass a qualifying string to append to each dump file
     * @param status Pass the expected status to verify with
     * @return matching status if successful; throws exception otherwise;
     */
    static CEStat ReapCE(SharedCQPtr cq, uint32_t numCE, uint32_t &isrCount,
        string grpName, string testName, string qualify,
        CEStat status = CESTAT_SUCCESS, const bool failOnIoctl = true);
    static CEStat ReapCE(SharedCQPtr cq, uint32_t numCE, uint32_t &isrCount,
        string grpName, string testName, string qualify,
        std::vector<CEStat> &status, const bool failOnIoctl = true);

    /**
     * Reap a specified number of CE's from the specified CQ.
     * @note Throws upon errors
     * @param cq Pass the CQ to reap from
     * @param numCE Pass the number of CE's to reap from the CQ
     * @param isrCount Return the ISR count as a result of reaping
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param qualify Pass a qualifying string to append to each dump file
     * @param status Pass the unexpected status to verify with
     * @return status of ce if successful; throws exception otherwise;
     */
    static CEStat ReapCENot(SharedCQPtr cq, uint32_t numCE, uint32_t &isrCount,
        string grpName, string testName, string qualify,
        CEStat status = CESTAT_SUCCESS, const bool failOnIoctl = true);
    static CEStat ReapCENot(SharedCQPtr cq, uint32_t numCE, uint32_t &isrCount,
        string grpName, string testName, string qualify,
        std::vector<CEStat> &status, const bool failOnIoctl = true);

    /**
     * Reap a specified number of CE's from the specified CQ.
     * @note Does not throw
     * @param cq Pass the CQ to reap from
     * @param numCE Pass the number of CE's to reap from the CQ
     * @param isrCount Return the ISR count as a result of reaping
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param qualify Pass a qualifying string to append to each dump file
     * @return ce status
     */
    static CEStat ReapCEIgnore(SharedCQPtr cq, uint32_t numCE,
        uint32_t &isrCount, string grpName, string testName, string qualify,
        const bool failOnIoctl = true);

    /**
     * Send and Reap an existing, user defined cmd to/from hdw using the spec'd
     * SQ/CQ pairs. This method requires 0 elements to reside in the CQ and
     * also assume no other cmd will complete into that CQ while this operation
     * is occurring. In the end the number of CE's will be verified to
     * guarantee that only 1 CE arrived as a result of sending this 1 cmd.
     * @note Throws upon errors
     * @note Method uses pre-existing values of CC.IOCQES
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param sq Pass pre-existing SQ to issue a cmd into
     * @param cq Pass pre-existing CQ to reap CE to verify successful creation
     * @param cmd Pass the cmd to issue into the supplied CQ
     * @param qualify Pass a qualifying string to append to each dump file
     * @param verbose Pass true to dump resources to dump files, otherwise false
     * @return matching status if successful; throws exception otherwise;
     */
    static union CE SendAndReapCmdWhole(string grpName, string testName,
        uint16_t ms, SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd,
        string qualify, bool verbose);

    /**
     * Reap a specified number of CE's from the specified CQ.
     * @note Throws upon errors
     * @param cq Pass the CQ to reap from
     * @param numCE Pass the number of CE's to reap from the CQ
     * @param isrCount Return the ISR count as a result of reaping
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param qualify Pass a qualifying string to append to each dump file
     * @return status of ce if successful; throws exception otherwise;
     */
    static union CE ReapCEWhole(SharedCQPtr cq, uint32_t numCE,
        uint32_t &isrCount, string grpName, string testName, string qualify,
        const bool failOnIoctl = true);

private:
    /**
     * Send and reap using the specified reap function
     */
    static CEStat SendAndReapCmd(string grpName, string testName, uint16_t ms,
        SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd, string qualify,
        bool verbose, std::vector<CEStat> &status,
        CEStat (*Reap)(SharedCQPtr cq, uint32_t numCE, uint32_t &isrCount,
            string grpName, string testName, string qualify,
            std::vector<CEStat> &status, const bool failOnIoctl));

    /**
     * Just send a command without reaping.
     * @note Throws upon errors
     * @return the unique id of the sent command
     */
    static uint16_t SendCmd(string grpName, string testName, SharedSQPtr sq,
        SharedCQPtr cq, SharedCmdPtr cmd, uint32_t &numCE, uint32_t &isrCount,
        string qualify, bool verbose);

    /**
     * Wait for a specified amount of time for a CQ entry to be ready to be
     * reaped.
     * @note Throws upon errors
     */
    static void WaitForReap(string grpName, string testName, uint16_t ms,
        SharedCQPtr cq, SharedCmdPtr cmd, uint32_t &numCE, uint32_t &isrCount,
        string qualify, bool verbose);

    /**
     * Wrapper around ReapCEIgnore to allow use with function pointer for
     * SendAndReapCmd.
     */
    static CEStat ReapCEIgnore(SharedCQPtr cq, uint32_t numCE,
        uint32_t &isrCount, string grpName, string testName, string qualify,
        std::vector<CEStat> &status, const bool failOnIoctl = true);

    /**
     * Retrieve CE from CQ.
     */
    static union CE RetrieveCE(SharedCQPtr cq, uint32_t numCE,
        uint32_t &isrCount, string grpName, string testName, string qualify,
        const bool failOnIoctl = true);

    /**
     * Attempt to retrieve CE from CQ.  Returns number retrieved.
     */
    static uint32_t AttemptRetrieveCE(SharedCQPtr cq, uint32_t numCE,
        uint32_t &isrCount, struct nvme_gen_cq *cqMetrics,
        const bool failOnIoctl = true);
};


#endif

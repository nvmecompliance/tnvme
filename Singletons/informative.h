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

#ifndef _INFORMATIVE_H_
#define _INFORMATIVE_H_

#include <vector>
#include "tnvme.h"
#include "../Cmds/identify.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"

using namespace std;


/**
* This class is created empty, basically useless, and does not contain any DUT
* data until the GrpInformative group is executed. The purpose of this
* singleton is to collect useful data extracted by GrpInformative and
* preserve it for all tests to use. This data should be data that never
* changes, volatile data should not be preserved here.
*
* This class is the information holder for all tests to access 1 place for
* common convenient DUT operational parameters. Rather than force every class
* to go through the hassle of retrieving this necessary data, do it once
* globally for all to enjoy.
*/
class Informative
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     * @return NULL upon error, otherwise a pointer to the singleton
     */
    static Informative *GetInstance(int fd, SpecRev specRev);
    static void KillInstance();
    ~Informative();

    /**
     * Issue appropriate cmds to the DUT to obtain an up-to-date snapshot of
     * the viscous data contained within. After execution the state of the DUT
     * will be known, and all members of this class will yield correct results.
     * The most common reason for calling this method is when a test or action
     * targets the DUT to cause its Identify or Get Features data to change.
     * The entire framework, and almost every test case, relies upon this data
     * to make dynamic adjustments and to understand the limits of the DUT. The
     * framework has no idea that some object caused its config to change.
     * @note The assumption here is that both the asq and acq's must be empty,
     *       and the DUT must be currently enabled.
     * @param asq Pass pre-existing ASQ in which to issue admin cmds
     * @param acq Pass pre-existing ACQ to reap any correlating CE's
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @return true upon success, otherwise false.
     */
    bool Reinit(SharedASQPtr &asq, SharedACQPtr &acq, uint16_t ms);

    /**
     * Get a previously fetched identify command's controller struct.
     * @return The requested data
     */
    ConstSharedIdentifyPtr GetIdentifyCmdCtrlr() const;

    /**
     * Get a previously fetched identify command's namspc struct.
     * @param namspcId Pass the ID of the namspc of interest
     * @return Identify::NullIdentifyPtr if the requested namspcId is larger
     *      than the total number of namspc structures available.
     */
    ConstSharedIdentifyPtr GetIdentifyCmdNamspc(uint64_t namspcId) const;

    /**
     * Get a previously fetched get features's number of queues feature ID.
     * This is DW0 of the CE which results from a Get FEatures. The spec
     * states this is a 0-based value, thus 1 IOQ will always be supported
     * @return The last known DW0 of the CE returned by the DUT.
     */
    uint32_t GetFeaturesNumOfQueues() const;
    /// @return Derives the value from DW0 of the CE & converts to a 1-base val
    uint32_t GetFeaturesNumOfIOCQs() const;
    /// @return Derives the value from DW0 of the CE & converts to a 1-base val
    uint32_t GetFeaturesNumOfIOSQs() const;

    typedef enum {
        NS_BARE,    // no meta data, nor E2E data
        NS_METAI,   // META data Interleaved (METAI) in LBA payload
        NS_METAS,   // META data Separate (METAS) buffer
        NS_E2EI,    // E2E data Interleaved (E2EI) in LBA payload
        NS_E2ES,    // E2E data Separate (E2ES) buffer
    } NamspcType;

    /**
     * Interrogate the supplied identify cmd containing a namespace struct
     * and return the type of namespace it describes.
     * @return The namespace type or throws if it couldn't be determined.
     */
    NamspcType IdentifyNamespace(ConstSharedIdentifyPtr idCmdNamspc) const;

    /**
     * Retrieve an array indicating all the namespace ID(s) for the appropriate
     * namespace type desired.
     * @note Bare: Namespaces supporting no meta data, and E2E is
     *       disabled; Implies: Identify.LBAF[Identify.FLBAS].MS=0
     * @note Meta: Namespaces supporting meta data, and E2E is disabled;
     *       Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=0
     * @note E2E: Namespaces supporting meta data, and E2E is enabled;
     *       Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=!0
     * @return vector containing all desired namespace IDs; it could be an empty
     *       vector indicating no namespaces are present in the DUT, otherwise
     *       throws if anything prevents detecting the requested data.
     */
    vector<uint32_t> GetBareNamespaces() const;
    vector<uint32_t> GetMetaINamespaces() const;    // meta data interleaved
    vector<uint32_t> GetMetaSNamespaces() const;    // meta data separate
    vector<uint32_t> GetMetaNamespaces() const;     // interleaved and separate
    vector<uint32_t> GetE2eINamespaces() const;     // E2E interleaved
    vector<uint32_t> GetE2eSNamespaces() const;     // E2E separate
    vector<uint32_t> GetE2eNamespaces() const;      // interleaved and separate

    struct Namspc {
        ConstSharedIdentifyPtr idCmdNamspc; // Namespace data struct
        uint32_t id;                        // Namespace ID (1-based)
        NamspcType type;
        Namspc(ConstSharedIdentifyPtr n, uint32_t i, NamspcType t) {
            idCmdNamspc = n; id = i; type = t; }
    };

    /**
     * Seek for the 1st bare namespace, and if not found then
     * seek for the 1st meta separate namespace, and if not found then
     * seek for the 1st meta interleaved namespace, and if not found then
     * seek for the 1st E2E separate namespace, and if not found then
     * seek for the 1st E2E interleaved namespace, and if not found then
     * throw an exception.
     * @return Namespace data if successful, otherwise throws
     */
    Namspc Get1stBareMetaE2E() const;


private:
    // Implement singleton design pattern
    Informative();
    Informative(int fd, SpecRev specRev);
    static bool mInstanceFlag;
    static Informative *mSingleton;

    /// which spec release is being targeted
    SpecRev mSpecRev;
    /// file descriptor to the device under test
    int mFd;

    /// Force this object to remove/delete all that has been previously set.
    void Clear();

    /**
     * This method should be called once during construction to init this
     * this object. Thereafter the Reinit() must be called.
     * @return true upon success, otherwise false.
     */
    bool Init();

    uint32_t mGetFeaturesNumOfQ;
    SharedIdentifyPtr mIdentifyCmdCtrlr;
    vector<SharedIdentifyPtr> mIdentifyCmdNamspc;
    void SendGetFeaturesNumOfQueues(SharedASQPtr asq, SharedACQPtr acq,
        uint16_t ms);
    void SendIdentifyCtrlrStruct(SharedASQPtr asq, SharedACQPtr acq,
        uint16_t ms);
    void SendIdentifyNamespaceStruct(SharedASQPtr asq, SharedACQPtr acq,
        uint16_t ms);
};


#endif

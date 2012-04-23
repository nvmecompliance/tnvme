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
#include "../GrpInformative/dumpIdentifyData_r10b.h"
#include "../GrpInformative/dumpGetFeatures_r10b.h"

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
*
* @note Singleton's are not allowed to throw exceptions.
*/
class Informative
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     */
    static Informative* GetInstance(int fd, SpecRev specRev);
    static void KillInstance();
    ~Informative();

    /**
     * No reason to protect the file descriptor, each instance of tnvme can
     * only ever have access to a single DUT.
     */
    int GetFD() { return mFd; }

    /**
     * Retrieve a previously fetched identify command's controller struct.
     * @return The requested data
     */
    ConstSharedIdentifyPtr GetIdentifyCmdCtrlr() const;

    /**
     * Retrieve a previously fetched identify command's namspc struct.
     * @param namspcId Pass the ID of the namspc of interest
     * @return Identify::NullIdentifyPtr if the requested namspcId is larger
     *      than the total number of namspc structures available.
     */
    ConstSharedIdentifyPtr GetIdentifyCmdNamspc(uint64_t namspcId) const;

    /**
     * Retrieve a previously fetched get features's number of queues feature ID.
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
        NS_BARE,
        NS_META,
        NS_E2E,
        NS_FENCE    // always must be the last element
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
    vector<uint32_t> GetMetaNamespaces() const;
    vector<uint32_t> GetE2ENamespaces() const;

    struct Namspc {
        ConstSharedIdentifyPtr idCmdNamspc; // namspc data struct
        uint32_t id;    // namspc identifier of the namspc data struct
        NamspcType type;
        Namspc(ConstSharedIdentifyPtr n, uint32_t i, NamspcType t) {
            idCmdNamspc = n; id = i; type = t; }
    };

    /**
     * Seek for the 1st bare namespace, and if not found, seek for the 1st
     * meta namespace, and if not found, seek for the first E2E namespace, and
     * if not found, throw.
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

    /**
     * These are the only tests within group GrpInformative which can initialize
     * this singleton; if GrpInformative does not get executed this singleton
     * won't get init'd correctly. Thus GrpInformative must run always, and
     * it should only run once at power-up.
     *
     * The "setting" of data is privatized because this data must be guaranteed
     * coherent, not modifiable for all tests for all groups to retrieve and
     * count upon. This data should be data that never changes even after
     * interaction with a DUT's various registers, Set Features attempts.
     */
    friend class GrpInformative::DumpIdentifyData_r10b;
    friend class GrpInformative::DumpGetFeatures_r10b;

    /**
     * GrpInformative must set this data.
     * @param idCmdCtrlr Pass the identify cmd after it retrieved the
     *          controller data structure .
     */
    SharedIdentifyPtr mIdentifyCmdCtrlr;
    void SetIdentifyCmdCtrlr(SharedIdentifyPtr idCmdCtrlr)
        { mIdentifyCmdCtrlr = idCmdCtrlr; }

    /**
     * GrpInformative must set this data. This method must be called in order
     * of namespace retrieval, starting from ID=1 to max possible.
     * @param idCmdNamspc Pass the identify cmd after it retrieved the
     *          namespace data structure for a given namespace ID.
     */
    vector<SharedIdentifyPtr> mIdentifyCmdNamspc;
    void SetIdentifyCmdNamespace(SharedIdentifyPtr idCmdNamspc)
        { mIdentifyCmdNamspc.push_back(idCmdNamspc); }

    /**
     * GrpInformative must set this data.
     * @param ceDword0 Pass DWORD0 of the CE resulting from sending the get
     *      features which requested the "number of queues" feature ID.
     */
    uint32_t mGetFeaturesNumOfQ;
    void SetGetFeaturesNumberOfQueues(uint32_t ceDword0)
        { mGetFeaturesNumOfQ =  ceDword0; }

    /// Force this object to remove/delete all that has been previously set.
    void Clear();
};


#endif

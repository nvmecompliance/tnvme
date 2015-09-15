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

#ifndef TEST_RESULTS_H
#define TEST_RESULTS_H

#include "globals.h"

/*     TR               desc */
#define TR_TABLE                                                               \
    ZZ(TR_SUCCESS,      "passed")                                              \
    ZZ(TR_FAIL,         "failed")                                              \
    ZZ(TR_SKIPPING,     "skipped")                                             \
    ZZ(TR_INFORMATIVE,  "informative")                                         \
    ZZ(TR_NOTFOUND,     "N/F")

#define ZZ(a, b)     a,
    typedef enum {
        TR_TABLE
        TR_FENCE            // always must be the last element
    } TestResult;
#undef ZZ

/**
 * This class represents a set of test results.  Contains a count of each
 * valid test result.
 *
 * @ note This class will not throw exceptions.
 */
class TestResults
{
public:
    TestResults();
    virtual ~TestResults();

    virtual TestResults *Clone() const
        { return new TestResults(*this); }
    TestResults &operator=(const TestResults &other);
    TestResults(const TestResults &other);

    /**
     * Add 1 to the count for the given test result.
     *
     * @param testResult the result to increment
     * @param amount the amount to increment the result by
     */
    void addResult(TestResult testResult, int amount = 1);

    /**
     * Do all currently reported results signify passing status?
     *
     * @return false if there are any failing results; true otherwise
     */
    bool allTestsPass() const;

    /**
     * Log current results.
     *
     * @param numIters number of iterations of test set which have been executed
     * @param numGrps number of groups which have been executed
     */
    void report(const size_t numIters, const int numGrps) const;

protected:
    virtual TestResults &assign(const TestResults &other);

private:
    int results[TR_FENCE];

    /**
     * Descriptions of the test results for logging purposes
     */
    static const char *resultDesc[];
};

#endif /*TEST_RESULTS_H*/

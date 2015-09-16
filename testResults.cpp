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

#include <string.h>
#include "testResults.h"

#define ZZ(a, b)          b,
const char * TestResults::resultDesc[] =
{
    TR_TABLE
};
#undef ZZ


TestResults::TestResults()
{
    std::fill(results, results + TR_FENCE, 0);
}


TestResults::~TestResults()
{

}


TestResults &
TestResults::operator=(const TestResults &other)
{
    return assign(other);
}


TestResults::TestResults(const TestResults &other)
{
    assign(other);
}


TestResults &
TestResults::assign(const TestResults &other)
{
    for (int i = 0; i < TR_FENCE; i++)
        results[i] = other.results[i];
    return *this;
}


void TestResults::addResult(TestResult testResult, int amount)
{
    results[testResult] += amount;
}


bool
TestResults::allTestsPass() const
{
    return results[TR_FAIL] == 0;
}


void
TestResults::report(const size_t numIters, const int numGrps) const
{
    int totalTests = 0;
    const int fieldLen = 13;

    LOG_NRM("Iteration SUMMARY");
    for (int i = 0; i < TR_FENCE - 1; i++) {
        if (i == TR_FAIL && results[TR_FAIL])
            LOG_NRM("  %-*s: %d  <---", fieldLen, resultDesc[i], results[i]);
        else
            LOG_NRM("  %-*s: %d", fieldLen, resultDesc[i], results[i]);
        totalTests += results[i];
    }
    LOG_NRM("  %-*s: %d", fieldLen, "total tests", totalTests);
    LOG_NRM("  %-*s: %d", fieldLen, "total groups", numGrps);
    LOG_NRM("Stop loop execution #%ld", numIters);
}

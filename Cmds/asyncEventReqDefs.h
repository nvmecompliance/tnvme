/*
* Copyright (c) 2012, Intel Corporation.
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

#ifndef _ASYNCEVENTREQDEFS_H_
#define _ASYNCEVENTREQDEFS_H_

/// Bit definitions for CE.n.async.asyncEventType
typedef enum {
    EVENT_TYPE_ERROR_STS     = 0,
    EVENT_TYPE_SMART_STS     = 1,
    EVENT_TYPE_VS_SPEC       = 7
} AsyncEventTypes;

/// Bit definitions for CE.n.async.asyncEventInfo
typedef enum {
    ERR_STS_INVALID_SQ                      = 0,
    ERR_STS_INVALID_DB_WR                   = 1,
    ERR_STS_DIAG_FAIL                       = 2,
    ERR_STS_PERSISTENT_INTERNAL_DEV_ERR     = 3,
    ERR_STS_TRANSIENT_INTERNAL_DEV_ERR      = 4,
    ERR_STS_FW_IMG_LOAD_ERR                 = 5,
    SMART_STS_RELIABILITY                   = 0,
    SMART_STS_TEMP_THRESH                   = 1,
    SMART_STS_SPARE_THRESH                  = 2
} AsyncEventInfomation;


#endif

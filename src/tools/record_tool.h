/***************************************************************************
 * A program for libmseed packing tests.
 *
 * This file is part of the miniSEED Library.
 *
 * Copyright (c) 2020 Chad Trabant, IRIS Data Management Center
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/
 
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 
#include "tools/libmseed.h"

namespace openmldb {
namespace tools {

class RecordTool {
  public:

    static MS3Record* GenRecord(const std::string& device, MS3Record* msr, int samplerate, int record_num, uint64_t start_time, int32_t* data) {
        if (!(msr = msr3_init(msr))) {
            printf ("Could not allocate MS3Record, out of memory?\n");
            return nullptr;
        }
        strcpy (msr->sid, device.c_str());
        msr->pubversion = 1;
        msr->starttime = start_time * 1000000;
        //msr->starttime = 1657639802100000000;
        msr->samprate = (double)samplerate;
        msr->encoding = DE_STEIM2;
 
        msr->numsamples  = record_num;
        msr->datasamples = data;
        msr->sampletype  = 'i';
 
        msr->samplecnt = msr->numsamples;
        return msr;
    }
};

}  // namespace tools
}  // namespace openmldb

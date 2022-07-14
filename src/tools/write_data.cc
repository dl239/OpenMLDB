/*
 * Copyright 2021 4Paradigm
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
 */
#include <thread>
#include <vector>
#include "base/random.h"
#include "common/timer.h"
#include "gflags/gflags.h"
#include "tools/parse_id.h"
#include "tools/record_tool.h"
#include "tools/libmseed.h"

DEFINE_string(id_file, "", "config the device_id file");
DEFINE_string(out_file, "", "config the data file");
//constexpr uint64_t end_time = 1657093936000;
constexpr uint64_t end_time = 1657093936000 - 3600 * 1000;
constexpr uint64_t cur_time = end_time - 3600 * 1000;
//constexpr uint64_t cur_time = 1657093936000 - 60 * 1000;

constexpr int THREAD_NUM = 10;
constexpr uint32_t MAX_VAL = 94759128;
constexpr uint32_t MAX_CNT = 1000;

std::vector<std::string> device_id;
std::vector<uint64_t> device_ts;

namespace openmldb {
namespace tools {
::openmldb::base::Random random(0xdeadbeef);

static void ms_record_handler_int (char *record, int reclen, void *ofp) {
    if (fwrite (record, reclen, 1, (FILE *)ofp) != 1) {
        ms_log (2, "Error writing to output file\n");
    }
}

void GenData(int32_t* data, int num) {
    if (data == nullptr) return;
    int sign = random.Uniform(2);
    for (int i = 0; i < num; i++) {
        int value = random.Uniform(MAX_VAL);
        if (sign == 0) {
            value *= -1;
        }
        data[i] = value;
    }
}

void WriteData(FILE *ofp) {
    int32_t* data = new int32_t[MAX_CNT];
    MS3Record* msr = nullptr;
    msr = msr3_init(msr);
    int flags = 0;
    flags |= MSF_FLUSHDATA;
    int8_t verbose  = 0;
    while(true) {
        int write_device_cnt = 0;
        for (size_t idx = 0; idx < device_id.size(); idx++) {
            uint64_t start_time = device_ts[idx];
            if (start_time >= end_time) {
                continue;
            }
            uint32_t cnt = random.Uniform(300) + 300;
            GenData(data, cnt);
            msr = RecordTool::GenRecord(device_id[idx], msr, 100, cnt, start_time, data);
            device_ts[idx] += cnt * 10;
            //msr3_print (msr, 0);
            //printf("raw %s [%d] [%d] [%c] [%d] [%d]\n", msr->sid, (int)msr->samprate, msr->samplecnt, msr->sampletype, msr->encoding, msr->numsamples);
            auto packedrecords = msr3_pack(msr, &ms_record_handler_int, ofp, NULL, flags, verbose - 1);
            write_device_cnt++;
        }
        if (write_device_cnt == 0) {
            break;
        }
    }
    delete []data;
}


}  // namespace tools
}  // namespace openmldb

int main(int argc, char** argv) {
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    if (FLAGS_id_file.empty()) {
        printf("has not set id file\n");
        return 0;
    }
    openmldb::tools::FillDeviceID(FLAGS_id_file, &device_id);
    for (size_t idx = 0; idx < device_id.size(); idx++) {
        device_ts.push_back(cur_time);
    }
    /*std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_NUM; i++) {
        threads.emplace_back(std::thread(::openmldb::tools::ImportData, i, table_info.tid(), std::ref(table_info.column_desc()), std::ref(tablets)));
    }
    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i].join();
    }*/
    FILE* ofp = fopen (FLAGS_out_file.c_str(), "wb");
    if (ofp == nullptr) {
        printf("file %s open failed\n", FLAGS_out_file.c_str());
    }
    ::openmldb::tools::WriteData(ofp);
    fclose (ofp);
    return 0;
}

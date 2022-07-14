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
#include "base/taskpool.hpp"
#include "base/random.h"
#include "boost/bind.hpp"
#include "codec/codec.h"
#include "common/timer.h"
#include "gflags/gflags.h"
#include "sdk/db_sdk.h"
#include "sdk/sql_router.h"
#include "sdk/sql_cluster_router.h"
#include "tools/parse_id.h"
#include "tools/record_tool.h"
#include "tools/libmseed.h"

DECLARE_string(zk_cluster);
DECLARE_string(zk_root_path);
DEFINE_string(id_file, "", "config the device_id file");
DEFINE_string(data_file, "", "config the data file");
DEFINE_int32(thread_num, 10, "");

std::string db = "test";
std::string table_name = "t2";
constexpr int GROUP_NUM = 100;
constexpr uint32_t MAX_VAL = 94759128;
constexpr uint32_t MAX_CNT = 1000;

namespace openmldb::tools {
class DeviceImportor;
}

std::vector<std::string> device_id;
std::map<std::string, openmldb::tools::DeviceImportor> importor_map;
std::vector<std::shared_ptr<openmldb::base::TaskPool>> task_pool_vec;

namespace openmldb {
namespace tools {
::openmldb::base::Random random(0xdeadbeef);
RecordTool tools;

struct RecordBuf {
    RecordBuf(int32_t* ptr, int num, int pos, uint64_t time) : buf(ptr), record_num(num), start_pos(pos), start_time(time) {}
    ~RecordBuf() {
        delete buf;
    }
    int32_t* buf;
    int record_num;
    int start_pos;
    uint64_t start_time;
};

class DeviceImportor {
  public:
    DeviceImportor(uint32_t id, uint32_t tid, const std::string& device_name, const Schema& schema,
            const std::vector<std::shared_ptr<::openmldb::catalog::TabletAccessor>>& tablets)
        : id_(id), tid_(tid), device_name_(device_name), total_record_num_(0), builder_(schema) {
        pid_ = (uint32_t)(::openmldb::base::hash64(device_name) % tablets.size());
        auto tablet = tablets[pid_];
        client_ = tablet->GetClient();
        int len = builder_.CalTotalLength(device_name.length());
        value_.resize(len);
    }

    inline uint32_t GetID() const { return id_; }

    void AddRecords(int32_t* buf, int record_num, uint64_t start_time) {
        record_list_.emplace_back(buf, record_num, 0, start_time);
        total_record_num_ += record_num;
        while (total_record_num_ >= GROUP_NUM) {
            PutRecords();
        }
    }
    void PutRecords() {
        if (record_list_.empty()) {
            return;
        }
        RecordBuf* record_buf = &(record_list_.front());
        uint64_t cur_time = record_buf->start_time + 10 * record_buf->start_pos;
        int8_t* buf = (int8_t*)(&value_[0]);
        int len = value_.length();
        builder_.InitBuffer(buf, len, true);
        builder_.SetString(buf, len, 0, device_name_.data(), device_name_.length());
        builder_.SetInt64(buf, 1, cur_time);
        int cnt = 0;
        while (cnt < GROUP_NUM) {
            while (cnt < GROUP_NUM && record_buf->start_pos < record_buf->record_num) {
                //printf("cnt %d cur_time %lu\n", cnt, cur_time);
                builder_.SetInt32(buf, cnt + 2, record_buf->buf[record_buf->start_pos]);
                record_buf->start_pos++;
                cnt++;
                total_record_num_--;
            }
            //printf("while cnt %d cur_time %lu start_pos %d record_num %d total_record_num_ %d\n", cnt, cur_time, record_buf->start_pos, record_buf->record_num, total_record_num_);
            if (record_buf->start_pos == record_buf->record_num && !record_list_.empty()) {
                record_list_.pop_front();
            }
            if (cnt == GROUP_NUM) {
                client_->Put(tid_, pid_, device_name_, cur_time, value_);
                //printf("put %s %lu\n", device_name_.c_str(), cur_time);
                return;
            } else if (cnt < GROUP_NUM) {
                if (record_list_.empty()) {
                    return;
                }
                record_buf = &(record_list_.front());
            }
        }
    }
  private:
    uint32_t id_;
    uint32_t tid_;
    uint32_t pid_;
    std::string device_name_;
    int total_record_num_;
    ::openmldb::codec::RowBuilder builder_;
    std::shared_ptr<::openmldb::client::TabletClient> client_;
    std::string value_;
    std::list<RecordBuf> record_list_;
};

void ReadRecord(const std::string& input_file) {
    MS3Record *msr = NULL;
    int8_t verbose = 0;
    uint32_t flags = 0;
    flags |= MSF_VALIDATECRC;
    flags |= MSF_PNAMERANGE;
    flags |= MSF_UNPACKDATA;
    int retcode = 0;
    int total = 0;
    while ((retcode = ms3_readmsr(&msr, input_file.c_str(), NULL, NULL, flags, verbose)) == MS_NOERROR) {
        // printf("%s %d %d %c %d %d\n", msr->sid, (int)msr->samprate, msr->samplecnt, msr->sampletype, msr->encoding, msr->numsamples);
        total += msr->numsamples;
        uint64_t start_time = msr->starttime / 1000000;
        int32_t* buf = (int32_t*)msr->datasamples;
        int32_t* new_buf = (int32_t*)malloc(msr->numsamples * sizeof(int32_t));
        memcpy(new_buf, buf, msr->numsamples * sizeof(int32_t));
        auto iter = importor_map.find(msr->sid);
        auto id = iter->second.GetID();
        task_pool_vec[id]->AddTask(boost::bind(&DeviceImportor::AddRecords, &(iter->second), new_buf, msr->numsamples, start_time));
        //iter->second.AddRecords(new_buf, msr->numsamples, start_time);
    }
    printf("read %d records\n", total);
}


}  // namespace tools
}  // namespace openmldb

int main(int argc, char** argv) {
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    if (FLAGS_id_file.empty()) {
        printf("has not set id file\n");
        return 0;
    }
    printf("thread_num %d\n", FLAGS_thread_num);
    openmldb::tools::FillDeviceID(FLAGS_id_file, &device_id);
    ::openmldb::sdk::ClusterOptions copt;
    copt.zk_cluster = FLAGS_zk_cluster;
    copt.zk_path = FLAGS_zk_root_path;
    auto cs = new ::openmldb::sdk::ClusterSDK(copt);
    cs->Init();
    auto sr = new ::openmldb::sdk::SQLClusterRouter(cs);
    sr->Init();
    auto table_info = sr->GetTableInfo(db, table_name);
    std::vector<std::shared_ptr<::openmldb::catalog::TabletAccessor>> tablets;
    bool ret = cs->GetTablet(db, table_name, &tablets);
    if (!ret || tablets.empty()) {
        printf("fail to get tablet!\n");
        return 0;
    }
    for (size_t idx = 0; idx < device_id.size(); idx++) {
        const auto& device_name = device_id[idx];
        uint32_t id = idx % FLAGS_thread_num;
        importor_map.emplace(device_name, ::openmldb::tools::DeviceImportor(id, table_info.tid(), device_name, table_info.column_desc(), tablets));
    }
    for (int i = 0; i < FLAGS_thread_num; i++) {
        auto task_pool = std::make_shared<openmldb::base::TaskPool>(1, 100);
        task_pool_vec.emplace_back(task_pool);
        task_pool->Start();
    }
    openmldb::tools::ReadRecord(FLAGS_data_file);
    task_pool_vec.clear();
    /*std::vector<std::thread> threads;
    for (int i = 0; i < FLAGS_thread_num; i++) {
        threads.emplace_back(std::thread(::openmldb::tools::ImportData, i, table_info.tid(), std::ref(table_info.column_desc()), std::ref(tablets)));
    }
    for (int i = 0; i < FLAGS_thread_num; i++) {
        threads[i].join();
    }*/
    // ::openmldb::tools::ImportData(table_info.tid(), table_info.column_desc(), tablets);
    return 0;
}

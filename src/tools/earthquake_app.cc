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
#include "common/timer.h"
#include "gflags/gflags.h"
#include "sdk/sql_router.h"
#include "sdk/table_reader.h"
#include "sdk/db_sdk.h"
#include "sdk/sql_router.h"
#include "sdk/sql_cluster_router.h"
#include "tools/noisepsd.h"
#include "tools/parse_id.h"

DECLARE_string(zk_cluster);
DECLARE_string(zk_root_path);
DEFINE_string(id_file, "", "config the device_id file");
DEFINE_int32(thread_num, 10, "");

std::vector<std::string> device_id;
//onstexpr int THREAD_NUM = FLAGS_thread_num;
std::string db = "test";
std::string table_name = "t2";

namespace openmldb {
namespace tools {
constexpr int GROUP_NUM = 100;
constexpr int MAX_RECORD = 200000;

void ExtractData(const std::shared_ptr<openmldb::base::ScanKvIterator>& iter, ::openmldb::codec::RowView* view, double* data) {
    int idx = 0;
    while (iter->Valid()) {
        auto value = iter->GetValue();
        view->Reset((const int8_t*)(value.data()), value.size());
        for (int i = 0; i < GROUP_NUM; i++) {
            int32_t val = 0;
            view->GetInt32(i + 2, &val);
            data[idx] = (double)val;
            idx++;
        }
        iter->Next();
    }
}

void ExtractData(const std::shared_ptr<hybridse::sdk::ResultSet>& result, double* data) {
    int idx = 0;
    while (result->Next()) {
        for (int i = 0; i < GROUP_NUM; i++) {
            int32_t val = 0;
            result->GetInt32(i + 3, &val);
            data[idx] = (double)val;
            idx++;
        }
    }
}

void CalculatePsd(int thread_id, const std::shared_ptr<openmldb::sdk::TableReader>& reader, CNoisePsd* psd) {
//void CalculatePsd(int thread_id, CNoisePsd* psd) {
    /*::openmldb::sdk::SQLRouterOptions copt;
    copt.zk_cluster = FLAGS_zk_cluster;
    copt.zk_path = FLAGS_zk_root_path;
    auto router = ::openmldb::sdk::NewClusterSQLRouter(copt);
    if (!router) {
        return;
    }
    auto reader = router->GetTableReader();*/
    // std::string device = "AH_BJN_00_BHZ";
    int num = device_id.size() / FLAGS_thread_num;
    int start = thread_id * num;
    int end = start + num;
    if (thread_id == FLAGS_thread_num - 1) {
        end = device_id.size();
    }
    uint64_t st_time = 1657093936000;
    int sampRate = 100;
    double* data = new double[MAX_RECORD];
    double* PsdFreqCenter = new double[psd->GetFreqNum()];
    while (true) {
        for (int id_pos = start; id_pos < end; id_pos++) {
            hybridse::sdk::Status status;
            openmldb::sdk::ScanOption so;
            const std::string& device = device_id[id_pos];
            //printf("device %s pos %d\n", device.c_str(), id_pos);
            auto result = reader->Scan(db, table_name, device, st_time, st_time - 600 * 1000 - 1, so, &status);
            if (status.code != 0 || result->Size() == 0) {
                printf("msg %s\n", status.msg.c_str());
            } else {
                int data_num = result->Size() * sampRate;
                // printf("device %s record num %d\n", device.c_str(), data_num);
                ExtractData(result, data);
                //printf("result size %d\n", data_num);
                //printf("sampRate %d\n", sampRate);
                psd->PsdCalculate(device, data, data_num, sampRate, 1, PsdFreqCenter);
                //for (int i = 0; i < psd->GetFreqNum(); i++) {
                //    printf("PsdFreqCenter[%d]: %lf\n", i, PsdFreqCenter[i]);
                //}
            }
        }
    }
    delete []data;
    delete []PsdFreqCenter;
}

void CalculatePsd1(int thread_id, uint32_t tid, const Schema& schema, const std::vector<std::shared_ptr<::openmldb::catalog::TabletAccessor>>& tablets, CNoisePsd* psd) {
    // std::string device = "AH_BJN_00_BHZ";
    int num = device_id.size() / FLAGS_thread_num;
    int start = thread_id * num;
    int end = start + num;
    if (thread_id == FLAGS_thread_num - 1) {
        end = device_id.size();
    }
    printf("start %d end %d num %d\n", start, end, end - start);
    uint64_t st_time = 1657093936000;
    int sampRate = 100;
    double* PsdFreqCenter = new double[psd->GetFreqNum()];
    std::string msg;
    ::openmldb::codec::RowView view(schema);
    double* data = new double[MAX_RECORD];
    /*uint64_t scan_time = 0;
    uint64_t decode_time = 0;
    uint64_t psd_time = 0;
    uint64_t count = 0;
    uint64_t total = 0;*/
    while (true) {
        for (int id_pos = start; id_pos < end; id_pos++) {
            //printf("pos %d size %d\n", id_pos, device_id.size());
            const std::string& device = device_id[id_pos];
            //printf("device %s pos %d\n", device.c_str(), id_pos);
            uint32_t pid = (uint32_t)(::openmldb::base::hash64(device) % tablets.size());
            auto tablet = tablets[pid];
            auto client = tablet->GetClient();
            //uint64_t cur_time = ::baidu::common::timer::get_micros();
            auto iter = client->Scan(tid, pid, device, "", st_time, st_time - 600 * 1000 - 1, 0, 0, msg);
            //uint64_t cur_time1 = ::baidu::common::timer::get_micros();
            //scan_time += cur_time1 - cur_time;
            int data_num = iter->Size() * sampRate;
            //printf("device %s record num %d\n", device.c_str(), data_num);
            ExtractData(iter, &view, data);
            //uint64_t cur_time2 = ::baidu::common::timer::get_micros();
            //decode_time += cur_time2 - cur_time1;
            //printf("result size %d\n", data_num);
            //printf("sampRate %d\n", sampRate);
            psd->PsdCalculate(device, data, data_num, sampRate, 1, PsdFreqCenter);
            //psd_time += ::baidu::common::timer::get_micros() - cur_time2;
            //total++;
            //for (int i = 0; i < psd->GetFreqNum(); i++) {
            //    printf("PsdFreqCenter[%d]: %lf\n", i, PsdFreqCenter[i]);
            //}
            /*if (total > 1000) {
                break;
            }*/
        }
    }
    //printf("thread_id %d scan_time %lu decode_time %lu psd_time %lu total %lu\n", thread_id, scan_time / total, decode_time / total, psd_time / total, total);
    delete []data;
    delete []PsdFreqCenter;
}

}  // namespace tools
}  // namespace openmldb

void RequestByReader(::openmldb::tools::CNoisePsd *psd) {
    ::openmldb::sdk::SQLRouterOptions copt;
    copt.zk_cluster = FLAGS_zk_cluster;
    copt.zk_path = FLAGS_zk_root_path;
    auto router = ::openmldb::sdk::NewClusterSQLRouter(copt);
    if (router) {
        auto reader = router->GetTableReader();
        std::vector<std::thread> threads;
        for (int i = 0; i < FLAGS_thread_num; i++) {
            threads.emplace_back(std::thread(openmldb::tools::CalculatePsd, i, std::ref(reader), psd));
        }
        for (int i = 0; i < FLAGS_thread_num; i++) {
            threads[i].join();
        }
        //openmldb::tools::CalculatePsd(0, reader, &psd);
    } else {
        printf("failed to init sql router\n");
    }
}

void RequestByIterator(::openmldb::tools::CNoisePsd *psd) {
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
        return;
    }
    std::vector<std::thread> threads;
    for (int i = 0; i < FLAGS_thread_num; i++) {
        threads.emplace_back(std::thread(::openmldb::tools::CalculatePsd1, i, table_info.tid(), std::ref(table_info.column_desc()), std::ref(tablets), psd));
    }
    for (int i = 0; i < FLAGS_thread_num; i++) {
        threads[i].join();
    }
    //::openmldb::tools::CalculatePsd1(0, table_info.tid(), table_info.column_desc(), tablets, psd);
}

int main(int argc, char** argv) {
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    openmldb::tools::FillDeviceID(FLAGS_id_file, &device_id);
    ::openmldb::tools::CNoisePsd psd;
    double min_frequence = 1.0 / 120;
    double max_frequence = 50;
    psd.GetFreqCenter(min_frequence, max_frequence);
    printf("m_FreqNum %d\n", psd.GetFreqNum());
    printf("thread num %d\n", FLAGS_thread_num);
    /*pid_t pid = fork();
    if (pid == 0) {
        printf("sub\n");
        RequestByReader(&psd);
    } else if (pid > 0) {
        printf("main\n");
        RequestByReader(&psd);
    } else {
        printf("fork failed\n");
    }*/

    RequestByIterator(&psd);

    return 0;
}

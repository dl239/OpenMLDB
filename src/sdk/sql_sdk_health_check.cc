/*
 * Copyright 2023 4paradigm authors
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

// Run the given YAML cases under SQLClusterRouter connection
//
// The cases by default run once, with expect result assertion.
// Or run repeated times as a tiny benchmark.

#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "sdk/sql_router.h"
#include "sdk/sql_sdk_base_test.h"

DEFINE_string(zk, "", "endpoint to zookeeper");
DEFINE_string(zk_path, "/openmldb", "zookeeper root path for openmldb cluster");
DEFINE_string(db, "tpcc", "database context");
DEFINE_string(sql, "select count(*) from customer", "query string to test health");
DEFINE_int32(wait_cnt, 60 * 10, "max retry count before exit with error");

int main(int argc, char* argv[]) {
    ::google::ParseCommandLineFlags(&argc, &argv, false);

    openmldb::sdk::SQLRouterOptions opts;
    opts.zk_cluster = FLAGS_zk;
    opts.zk_path = FLAGS_zk_path;
    opts.enable_debug = ::hybridse::sqlcase::SqlCase::IsDebug();
    // opts.zk_log_level = 0;
    // opts.glog_level = 3;

    std::shared_ptr<openmldb::sdk::SQLRouter> router;
    int32_t idx = 0;
    while (++idx < FLAGS_wait_cnt) {
        router = openmldb::sdk::NewClusterSQLRouter(opts);
        hybridse::sdk::Status s;
        if (router == nullptr) {
            LOG(WARNING) << "Fail to init OpenMLDB connection, retrying";
        } else {
            router->ExecuteSQL(FLAGS_db, FLAGS_sql, &s);
            if (s.IsOK()) {
                LOG(INFO) << "sdk ready";
                return 0;
            }

            LOG(WARNING) << s.ToString();
        }
        absl::SleepFor(absl::Seconds(1));
    }

    LOG(WARNING) << "sdk error, not able to recovery";
    return 1;
}

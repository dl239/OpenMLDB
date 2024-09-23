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

#include "absl/random/distributions.h"
#include "absl/random/random.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "sdk/sql_router.h"
#include "sdk/sql_sdk_base_test.h"

DEFINE_string(yaml_path, "", "Yaml filepath to load cases from");
DEFINE_string(case_id, "", "case id");

DEFINE_string(zk, "", "endpoint to zookeeper");
DEFINE_string(zk_path, "/openmldb", "zookeeper root path for openmldb cluster");
DEFINE_string(db, "tpcc", "database context");

namespace openmldb {
namespace sdk {

static absl::BitGen gen;

static int Run(std::shared_ptr<SQLRouter> router, absl::string_view yaml_path) {
    std::vector<::hybridse::sqlcase::SqlCase> cases;
    if (!::hybridse::sqlcase::SqlCase::CreateSqlCasesFromYaml(hybridse::sqlcase::SqlCase::SqlCaseBaseDir(),
                                                              std::string(yaml_path), cases)) {
        LOG(WARNING) << "Load cases from " << yaml_path << " failed";
        return 1;
    }

    for (auto& sql_case : cases) {
        if (!FLAGS_case_id.empty() && FLAGS_case_id != sql_case.id()) {
            continue;
        }

        hybridse::sdk::Status s;
        router->ExecuteSQL(FLAGS_db, sql_case.sql_str_, &s);
        LOG(INFO) << s.ToString();
    }
    return 0;
}

}  // namespace sdk
}  // namespace openmldb

int main(int argc, char* argv[]) {
    ::google::ParseCommandLineFlags(&argc, &argv, false);

    openmldb::sdk::SQLRouterOptions opts;
    opts.zk_cluster = FLAGS_zk;
    opts.zk_path = FLAGS_zk_path;
    opts.enable_debug = ::hybridse::sqlcase::SqlCase::IsDebug();

    auto router = openmldb::sdk::NewClusterSQLRouter(opts);
    if (router == nullptr) {
        LOG(ERROR) << "Fail to init OpenMLDB connection";
        return 1;
    }

    hybridse::sdk::Status s;
    router->ExecuteSQL("set @@execute_mode = 'online'", &s);

    return ::openmldb::sdk::Run(router, FLAGS_yaml_path);
}

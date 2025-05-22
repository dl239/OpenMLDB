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
package com._4paradigm.openmldb.benchmark;
import com._4paradigm.openmldb.proto.NS;
import com._4paradigm.openmldb.sdk.SqlExecutor;
import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;

import java.sql.*;
import java.util.*;
import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.SampleTime)
@OutputTimeUnit(TimeUnit.MICROSECONDS)
@State(Scope.Benchmark)
@Threads(10)
@Fork(value = 1, jvmArgs = {"-Xms8G", "-Xmx8G"})
@Warmup(iterations = 2)
@Measurement(iterations = 5, time = 60)


public class AIOSBenchmark {
    private SqlExecutor executor;
    private String database;
    private String deployName;
    private String mainTable = "t";
    private Map<String, TableSchema> tableSchema = new HashMap<>();
    private Random random;

    public AIOSBenchmark() {
        executor = BenchmarkConfig.GetSqlExecutor(false);
        deployName = BenchmarkConfig.DEPLOY_NAME;
        database = BenchmarkConfig.DATABASE;
        random = new Random(System.currentTimeMillis());
        addTableSchema(database, mainTable);
    }

    private void addTableSchema(String dbName, String tableName) {
        NS.TableInfo tableInfo = null;
        try {
            tableInfo = executor.getTableInfo(dbName, tableName);
            TableSchema schema = new TableSchema(tableInfo);
            tableSchema.put(tableName, schema);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Benchmark
    public void executeDeployment() {
        int numberKey = BenchmarkConfig.PK_BASE;
        numberKey += random.nextInt(BenchmarkConfig.PK_NUM);
        try {
            PreparedStatement stat = null;
            if (BenchmarkConfig.BATCH_SIZE > 0) {
                stat = Util.getBatchPreparedStatement(deployName, numberKey, BenchmarkConfig.BATCH_SIZE, tableSchema.get(mainTable), executor);
            } else {
                stat = Util.getPreparedStatement(deployName, numberKey, tableSchema.get("mt"), executor);
            }
            ResultSet resultSet = stat.executeQuery();
            long total = 0;
            while (BenchmarkConfig.PARSE_RESULT && resultSet.next()) {
                Map<String, Object> val = Util.extractResultSet(resultSet);
                total += (long) val.get("sum_w0_col_i1");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        OpenMLDBPerfBenchmark benchmark = new OpenMLDBPerfBenchmark();
        while (true) {
            benchmark.executeDeployment();
        }

        //benchmark.cleanEnv();
        /*
        try {
            Options opt = new OptionsBuilder()
                    .include(OpenMLDBPerfBenchmark.class.getSimpleName())
                    .forks(1)
                    .build();
            new Runner(opt).run();
        } catch (Exception e) {
            e.printStackTrace();
        }*/
    }
}

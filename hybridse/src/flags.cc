/*
 * Copyright 2021 4Paradigm
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gflags/gflags.h>
// batch config
DEFINE_string(default_db_name, "_hybridse",
              "config the default batch catalog db name");

// Offline Spark config
DEFINE_bool(enable_spark_unsaferow_format, false,
            "config if codec uses Spark UnsafeRow format");

// jit options
DEFINE_bool(jit_enable_mcjit, false, "Use llvm legacy mcjit engine");
DEFINE_bool(jit_enable_vtune, false, "Enable llvm jit vtune events");
DEFINE_bool(jit_enable_gdb, false, "Enable llvm jit gdb events");
DEFINE_bool(jit_enable_perf, false, "Enable llvm jit perf events");
DEFINE_int32(jit_threads, 0, "JIT thread number, default 0 as run only in current thread");

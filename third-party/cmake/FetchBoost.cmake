# Copyright 2021 4Paradigm
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(BOOST_URL https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz)

message(STATUS "build boost from ${BOOST_URL}")

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(BOOST_FLAGS compiler.blacklist clang -with-toolset=clang)
endif()

# boost require python development package, python-dev on debian or python-devel on redhat
ExternalProject_Add(
  boost
  URL ${BOOST_URL}
  URL_HASH SHA256=2575e74ffc3ef1cd0babac2c1ee8bdb5782a0ee672b1912da40e5b4b591ca01f
  PREFIX ${DEPS_BUILD_DIR}
  DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}/boost
  INSTALL_DIR ${DEPS_INSTALL_DIR}
  BUILD_IN_SOURCE True
  CONFIGURE_COMMAND ./bootstrap.sh ${BOOST_FLAGS}
  BUILD_COMMAND ./b2 link=static cxxflags=-fPIC cflags=-fPIC --without-python release install --prefix=<INSTALL_DIR>
  INSTALL_COMMAND "")

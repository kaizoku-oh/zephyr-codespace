# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

target_include_directories(app PRIVATE include)
target_include_directories(app PUBLIC ${ZEPHYR_BASE}/../deps/modules/lib/golioth-firmware-sdk/examples/zephyr/common/include)

target_sources(app PRIVATE
  src/main.c
  # src/die_temp_example.c
  src/pn532_example.c
  src/pn532.c
)

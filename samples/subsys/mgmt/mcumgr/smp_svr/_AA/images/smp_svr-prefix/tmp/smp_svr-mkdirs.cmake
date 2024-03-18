# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/smp_svr"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/smp_svr-prefix"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/smp_svr-prefix/tmp"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/smp_svr-prefix/src/smp_svr-stamp"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/smp_svr-prefix/src"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/smp_svr-prefix/src/smp_svr-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/smp_svr-prefix/src/smp_svr-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/smp_svr-prefix/src/smp_svr-stamp${cfgdir}") # cfgdir has leading slash
endif()

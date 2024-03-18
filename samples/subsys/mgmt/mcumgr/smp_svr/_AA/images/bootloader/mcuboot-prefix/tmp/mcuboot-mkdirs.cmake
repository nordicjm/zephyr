# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/tmp/aa/bootloader/mcuboot/boot/zephyr"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/mcuboot"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/bootloader/mcuboot-prefix"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/bootloader/mcuboot-prefix/tmp"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/bootloader/mcuboot-prefix/src/mcuboot-stamp"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/bootloader/mcuboot-prefix/src"
  "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/bootloader/mcuboot-prefix/src/mcuboot-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/bootloader/mcuboot-prefix/src/mcuboot-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/tmp/aa/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/_AA/images/bootloader/mcuboot-prefix/src/mcuboot-stamp${cfgdir}") # cfgdir has leading slash
endif()

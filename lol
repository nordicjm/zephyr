diff --git a/scripts/west_commands/runners/core.py b/scripts/west_commands/runners/core.py
index 7ed95d5bd1..a12e728421 100644
--- a/scripts/west_commands/runners/core.py
+++ b/scripts/west_commands/runners/core.py
@@ -228,6 +228,9 @@ class RunnerCaps:
       erased by the underlying tool before flashing; UICR on nRF SoCs
       is one example.)
 
+    - reset: whether the runner supports a --reset option, which
+      resets the device after a flash operation is complete.
+
     - tool_opt: whether the runner supports a --tool-opt (-O) option, which
       can be given multiple times and is passed on to the underlying tool
       that the runner wraps.
@@ -239,12 +242,14 @@ class RunnerCaps:
                  dev_id: bool = False,
                  flash_addr: bool = False,
                  erase: bool = False,
+                 reset: bool = False,
                  tool_opt: bool = False,
                  file: bool = False):
         self.commands = commands
         self.dev_id = dev_id
         self.flash_addr = bool(flash_addr)
         self.erase = bool(erase)
+        self.reset = bool(reset)
         self.tool_opt = bool(tool_opt)
         self.file = bool(file)
 
@@ -253,6 +258,7 @@ class RunnerCaps:
                 f'dev_id={self.dev_id}, '
                 f'flash_addr={self.flash_addr}, '
                 f'erase={self.erase}, '
+                f'reset={self.reset}, '
                 f'tool_opt={self.tool_opt}, '
                 f'file={self.file}'
                 ')')
@@ -510,6 +516,11 @@ class ZephyrBinaryRunner(abc.ABC):
                             help=("mass erase flash before loading, or don't"
                                   if caps.erase else argparse.SUPPRESS))
 
+        parser.add_argument('--reset', '--no-reset', nargs=0,
+                            action=_ToggleAction,
+                            help=("reset device after flashing, or don't"
+                                  if caps.reset else argparse.SUPPRESS))
+
         parser.add_argument('-O', '--tool-opt', dest='tool_opt',
                             default=[], action='append',
                             help=(cls.tool_opt_help() if caps.tool_opt
@@ -538,6 +549,8 @@ class ZephyrBinaryRunner(abc.ABC):
             _missing_cap(cls, '--dt-flash')
         if args.erase and not caps.erase:
             _missing_cap(cls, '--erase')
+        if args.reset and not caps.reset:
+            _missing_cap(cls, '--reset')
         if args.tool_opt and not caps.tool_opt:
             _missing_cap(cls, '--tool-opt')
         if args.file and not caps.file:
@@ -550,6 +563,8 @@ class ZephyrBinaryRunner(abc.ABC):
         ret = cls.do_create(cfg, args)
         if args.erase:
             ret.logger.info('mass erase requested')
+        if args.reset:
+            ret.logger.info('reset after flashing requested')
         return ret
 
     @classmethod
diff --git a/scripts/west_commands/runners/esp32.py b/scripts/west_commands/runners/esp32.py
index f483fbf295..62ba43b2bc 100644
--- a/scripts/west_commands/runners/esp32.py
+++ b/scripts/west_commands/runners/esp32.py
@@ -16,13 +16,17 @@ class Esp32BinaryRunner(ZephyrBinaryRunner):
     '''Runner front-end for espidf.'''
 
     def __init__(self, cfg, device, boot_address, part_table_address,
-                 app_address, erase=False, baud=921600, flash_size='detect',
+                 app_address, erase=False, reset=True, baud=921600, flash_size='detect',
                  flash_freq='40m', flash_mode='dio', espidf='espidf',
                  bootloader_bin=None, partition_table_bin=None):
         super().__init__(cfg)
         self.elf = cfg.elf_file
         self.app_bin = cfg.bin_file
         self.erase = bool(erase)
+        if reset is None:
+            self.reset = True
+        else:
+            self.reset = bool(reset)
         self.device = device
         self.boot_address = boot_address
         self.part_table_address = part_table_address
@@ -41,7 +45,7 @@ class Esp32BinaryRunner(ZephyrBinaryRunner):
 
     @classmethod
     def capabilities(cls):
-        return RunnerCaps(commands={'flash'}, erase=True)
+        return RunnerCaps(commands={'flash'}, erase=True, reset=True)
 
     @classmethod
     def do_add_parser(cls, parser):
@@ -86,9 +90,10 @@ class Esp32BinaryRunner(ZephyrBinaryRunner):
             cfg, args.esp_device, boot_address=args.esp_boot_address,
             part_table_address=args.esp_partition_table_address,
             app_address=args.esp_app_address, erase=args.erase,
-            baud=args.esp_baud_rate, flash_size=args.esp_flash_size,
-            flash_freq=args.esp_flash_freq, flash_mode=args.esp_flash_mode,
-            espidf=espidf, bootloader_bin=args.esp_flash_bootloader,
+            reset=args.reset, baud=args.esp_baud_rate,
+            flash_size=args.esp_flash_size, flash_freq=args.esp_flash_freq,
+            flash_mode=args.esp_flash_mode, espidf=espidf,
+            bootloader_bin=args.esp_flash_bootloader,
             partition_table_bin=args.esp_flash_partition_table)
 
     def do_run(self, command, **kwargs):
@@ -105,7 +110,8 @@ class Esp32BinaryRunner(ZephyrBinaryRunner):
             cmd_flash.extend(['--port', self.device])
         cmd_flash.extend(['--baud', self.baud])
         cmd_flash.extend(['--before', 'default_reset'])
-        cmd_flash.extend(['--after', 'hard_reset', 'write_flash', '-u'])
+        if self.reset is True:
+            cmd_flash.extend(['--after', 'hard_reset', 'write_flash', '-u'])
         cmd_flash.extend(['--flash_mode', self.flash_mode])
         cmd_flash.extend(['--flash_freq', self.flash_freq])
         cmd_flash.extend(['--flash_size', self.flash_size])
diff --git a/scripts/west_commands/runners/ezflashcli.py b/scripts/west_commands/runners/ezflashcli.py
index e92bcd1877..d62c1e621d 100644
--- a/scripts/west_commands/runners/ezflashcli.py
+++ b/scripts/west_commands/runners/ezflashcli.py
@@ -10,13 +10,17 @@ DEFAULT_EZFLASHCLI = "ezFlashCLI"
 class EzFlashCliBinaryRunner(ZephyrBinaryRunner):
     '''Runner front-end for ezFlashCLI'''
 
-    def __init__(self, cfg, tool, sn, erase=False):
+    def __init__(self, cfg, tool, sn, erase=False, reset=True):
         super().__init__(cfg)
         self.bin_ = cfg.bin_file
 
         self.tool = tool
         self.sn_arg = ['-j', f'{sn}'] if sn is not None else []
         self.erase = bool(erase)
+        if reset is None:
+            self.reset = True
+        else:
+            self.reset = bool(reset)
 
     @classmethod
     def name(cls):
@@ -24,7 +28,7 @@ class EzFlashCliBinaryRunner(ZephyrBinaryRunner):
 
     @classmethod
     def capabilities(cls):
-        return RunnerCaps(commands={'flash'}, erase=True)
+        return RunnerCaps(commands={'flash'}, erase=True, reset=True)
 
     @classmethod
     def do_add_parser(cls, parser):
@@ -37,7 +41,7 @@ class EzFlashCliBinaryRunner(ZephyrBinaryRunner):
     @classmethod
     def do_create(cls, cfg, args):
         return EzFlashCliBinaryRunner(cfg, tool=args.tool, sn=args.sn,
-                                      erase=args.erase)
+                                      erase=args.erase, reset=args.reset)
 
     def program_bin(self):
         if self.erase:
@@ -47,12 +51,13 @@ class EzFlashCliBinaryRunner(ZephyrBinaryRunner):
         self.logger.info(f"Flashing {self.bin_}...")
         self.check_call([self.tool] + self.sn_arg + ["image_flash", self.bin_])
 
-    def reset(self):
-        self.logger.info("Resetting...")
-        self.check_call([self.tool] + self.sn_arg + ["go"])
+    def perform_reset(self):
+        if self.reset:
+            self.logger.info("Resetting...")
+            self.check_call([self.tool] + self.sn_arg + ["go"])
 
     def do_run(self, command, **kwargs):
         self.require(self.tool)
         self.ensure_output('bin')
         self.program_bin()
-        self.reset()
+        self.perform_reset()
diff --git a/scripts/west_commands/runners/intel_cyclonev.py b/scripts/west_commands/runners/intel_cyclonev.py
index eb67c86080..92d740638f 100644
--- a/scripts/west_commands/runners/intel_cyclonev.py
+++ b/scripts/west_commands/runners/intel_cyclonev.py
@@ -100,7 +100,7 @@ class IntelCycloneVBinaryRunner(ZephyrBinaryRunner):
     @classmethod
     def capabilities(cls):
         return RunnerCaps(commands={'flash', 'debug', 'attach'},
-                          dev_id=False, flash_addr=False, erase=False)
+                          dev_id=False, flash_addr=False, erase=False, reset=False)
 
     @classmethod
     def do_add_parser(cls, parser):
diff --git a/scripts/west_commands/runners/jlink.py b/scripts/west_commands/runners/jlink.py
index 8d595f2965..61b645df17 100644
--- a/scripts/west_commands/runners/jlink.py
+++ b/scripts/west_commands/runners/jlink.py
@@ -35,7 +35,7 @@ class JLinkBinaryRunner(ZephyrBinaryRunner):
 
     def __init__(self, cfg, device, dev_id=None,
                  commander=DEFAULT_JLINK_EXE,
-                 dt_flash=True, erase=True, reset_after_load=False,
+                 dt_flash=True, erase=True, reset=False,
                  iface='swd', speed='auto',
                  loader=None,
                  gdbserver='JLinkGDBServer',
@@ -54,7 +54,7 @@ class JLinkBinaryRunner(ZephyrBinaryRunner):
         self.commander = commander
         self.dt_flash = dt_flash
         self.erase = erase
-        self.reset_after_load = reset_after_load
+        self.reset = reset
         self.gdbserver = gdbserver
         self.iface = iface
         self.speed = speed
@@ -74,7 +74,7 @@ class JLinkBinaryRunner(ZephyrBinaryRunner):
     @classmethod
     def capabilities(cls):
         return RunnerCaps(commands={'flash', 'debug', 'debugserver', 'attach'},
-                          dev_id=True, flash_addr=True, erase=True,
+                          dev_id=True, flash_addr=True, erase=True, reset=True,
                           tool_opt=True, file=True)
 
     @classmethod
@@ -111,12 +111,6 @@ class JLinkBinaryRunner(ZephyrBinaryRunner):
         parser.add_argument('--commander', default=DEFAULT_JLINK_EXE,
                             help=f'''J-Link Commander, default is
                             {DEFAULT_JLINK_EXE}''')
-        parser.add_argument('--reset-after-load', '--no-reset-after-load',
-                            dest='reset_after_load', nargs=0,
-                            action=ToggleAction,
-                            help='reset after loading? (default: no)')
-
-        parser.set_defaults(reset_after_load=False)
 
     @classmethod
     def do_create(cls, cfg, args):
@@ -125,7 +119,7 @@ class JLinkBinaryRunner(ZephyrBinaryRunner):
                                  commander=args.commander,
                                  dt_flash=args.dt_flash,
                                  erase=args.erase,
-                                 reset_after_load=args.reset_after_load,
+                                 reset=args.reset,
                                  iface=args.iface, speed=args.speed,
                                  gdbserver=args.gdbserver,
                                  loader=args.loader,
@@ -264,7 +258,7 @@ class JLinkBinaryRunner(ZephyrBinaryRunner):
                 client_cmd += ['-ex', 'monitor halt',
                                '-ex', 'monitor reset',
                                '-ex', 'load']
-                if self.reset_after_load:
+                if self.reset:
                     client_cmd += ['-ex', 'monitor reset']
             if not self.gdb_host:
                 self.require(self.gdbserver)
@@ -324,7 +318,7 @@ class JLinkBinaryRunner(ZephyrBinaryRunner):
         # Flash the selected build artifact
         lines.append(flash_cmd)
 
-        if self.reset_after_load:
+        if self.reset:
             lines.append('r') # Reset and halt the target
 
         lines.append('g') # Start the CPU
diff --git a/scripts/west_commands/runners/nrfjprog.py b/scripts/west_commands/runners/nrfjprog.py
index 314711933d..fd69901bed 100644
--- a/scripts/west_commands/runners/nrfjprog.py
+++ b/scripts/west_commands/runners/nrfjprog.py
@@ -1,5 +1,5 @@
 # Copyright (c) 2017 Linaro Limited.
-# Copyright (c) 2019 Nordic Semiconductor ASA.
+# Copyright (c) 2019-2023 Nordic Semiconductor ASA.
 #
 # SPDX-License-Identifier: Apache-2.0
 
@@ -26,7 +26,7 @@ VerifyError = 55
 class NrfJprogBinaryRunner(ZephyrBinaryRunner):
     '''Runner front-end for nrfjprog.'''
 
-    def __init__(self, cfg, family, softreset, dev_id, erase=False,
+    def __init__(self, cfg, family, softreset, dev_id, erase=False, reset=True,
                  tool_opt=[], force=False, recover=False):
         super().__init__(cfg)
         self.hex_ = cfg.hex_file
@@ -34,6 +34,10 @@ class NrfJprogBinaryRunner(ZephyrBinaryRunner):
         self.softreset = softreset
         self.dev_id = dev_id
         self.erase = bool(erase)
+        if reset is None:
+            self.reset = True
+        else:
+            self.reset = bool(reset)
         self.force = force
         self.recover = bool(recover)
 
@@ -47,7 +51,7 @@ class NrfJprogBinaryRunner(ZephyrBinaryRunner):
 
     @classmethod
     def capabilities(cls):
-        return RunnerCaps(commands={'flash'}, dev_id=True, erase=True,
+        return RunnerCaps(commands={'flash'}, dev_id=True, erase=True, reset=True,
                           tool_opt=True)
 
     @classmethod
@@ -82,8 +86,8 @@ class NrfJprogBinaryRunner(ZephyrBinaryRunner):
     def do_create(cls, cfg, args):
         return NrfJprogBinaryRunner(cfg, args.nrf_family, args.softreset,
                                     args.dev_id, erase=args.erase,
-                                    tool_opt=args.tool_opt, force=args.force,
-                                    recover=args.recover)
+                                    reset=args.reset, tool_opt=args.tool_opt,
+                                    force=args.force, recover=args.recover)
 
     def ensure_snr(self):
         if not self.dev_id or "*" in self.dev_id:
@@ -381,7 +385,8 @@ class NrfJprogBinaryRunner(ZephyrBinaryRunner):
         if self.recover:
             self.recover_target()
         self.program_hex()
-        self.reset_target()
+        if self.reset:
+            self.reset_target()
 
         self.logger.info(f'Board with serial number {self.dev_id} '
                          'flashed successfully.')
diff --git a/scripts/west_commands/runners/pyocd.py b/scripts/west_commands/runners/pyocd.py
index 0903be64ae..285151acf7 100644
--- a/scripts/west_commands/runners/pyocd.py
+++ b/scripts/west_commands/runners/pyocd.py
@@ -77,7 +77,7 @@ class PyOcdBinaryRunner(ZephyrBinaryRunner):
     @classmethod
     def capabilities(cls):
         return RunnerCaps(commands={'flash', 'debug', 'debugserver', 'attach'},
-                          dev_id=True, flash_addr=True, erase=True,
+                          dev_id=True, flash_addr=True, erase=True, reset=False,
                           tool_opt=True)
 
     @classmethod
diff --git a/scripts/west_commands/runners/spi_burn.py b/scripts/west_commands/runners/spi_burn.py
index 265d1cd29b..461f151a64 100644
--- a/scripts/west_commands/runners/spi_burn.py
+++ b/scripts/west_commands/runners/spi_burn.py
@@ -29,7 +29,7 @@ class SpiBurnBinaryRunner(ZephyrBinaryRunner):
 
     @classmethod
     def capabilities(cls):
-        return RunnerCaps(commands={'flash', 'debug'}, erase=True, flash_addr=True)
+        return RunnerCaps(commands={'flash', 'debug'}, erase=True, reset=False, flash_addr=True)
 
     @classmethod
     def do_add_parser(cls, parser):
diff --git a/scripts/west_commands/runners/stm32flash.py b/scripts/west_commands/runners/stm32flash.py
index 052c45dbb4..7f397e25db 100644
--- a/scripts/west_commands/runners/stm32flash.py
+++ b/scripts/west_commands/runners/stm32flash.py
@@ -72,9 +72,6 @@ class Stm32flashBinaryRunner(ZephyrBinaryRunner):
         parser.add_argument('--serial-mode', default='8e1', required=False,
                             help='serial port mode, default \'8e1\'')
 
-        parser.add_argument('--reset', default=False, required=False, action='store_true',
-                            help='reset device at exit, default False')
-
         parser.add_argument('--verify', default=False, required=False, action='store_true',
                             help='verify writes, default False')
 
diff --git a/scripts/west_commands/tests/test_nrf.py b/scripts/west_commands/tests/test_nrf.py
index 0e732b1d10..2f2517905e 100644
--- a/scripts/west_commands/tests/test_nrf.py
+++ b/scripts/west_commands/tests/test_nrf.py
@@ -1,5 +1,5 @@
 # Copyright (c) 2018 Foundries.io
-# Copyright (c) 2020 Nordic Semiconductor ASA
+# Copyright (c) 2020-2023 Nordic Semiconductor ASA
 #
 # SPDX-License-Identifier: Apache-2.0
 
@@ -64,111 +64,121 @@ class TC(typing.NamedTuple):    # 'TestCase'
     # --sectorerase if False (or --sectoranduicrerase on nRF52)
     erase: bool
 
+    # --reset if True, --no-reset if False
+    reset: bool
 
 EXPECTED_RESULTS = {
 
     # -------------------------------------------------------------------------
     # NRF51
     #
-    #  family   CP    recov  soft   snr    erase
-    TC('NRF51', None, False, False, False, False):
+    #  family   CP    recov  soft   snr    erase  reset
+    TC('NRF51', None, False, False, False, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF51',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF51', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF51', None, False, False, False, True):
+    TC('NRF51', None, False, False, False, True, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--chiperase', '--verify', '-f', 'NRF51',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF51', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF51', None, False, False, True, False):
+    TC('NRF51', None, False, False, True, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF51',
       '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF51', '--snr', TEST_OVR_SNR]),
 
-    TC('NRF51', None, False, True, False, False):
+    TC('NRF51', None, False, True, False, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF51',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--reset', '-f', 'NRF51', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF51', None, True, False, False, False):
+    TC('NRF51', None, True, False, False, False, True):
     (['nrfjprog', '--recover', '-f', 'NRF51', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF51',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF51', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF51', None, True, True, True, True):
+    TC('NRF51', None, True, True, True, True, True):
     (['nrfjprog', '--recover', '-f', 'NRF51', '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--program', RC_KERNEL_HEX, '--chiperase', '--verify', '-f', 'NRF51',
       '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--reset', '-f', 'NRF51', '--snr', TEST_OVR_SNR]),
 
+    TC('NRF51', None, False, False, False, False, False):
+    (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF51',
+      '--snr', TEST_DEF_SNR]),
+
     # -------------------------------------------------------------------------
     # NRF52
     #
-    #  family   CP    recov  soft   snr    erase
-    TC('NRF52', None, False, False, False, False):
+    #  family   CP    recov  soft   snr    erase  reset
+    TC('NRF52', None, False, False, False, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectoranduicrerase',
       '--verify', '-f', 'NRF52', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinresetenable', '-f', 'NRF52', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF52', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF52', None, False, False, False, True):
+    TC('NRF52', None, False, False, False, True, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--chiperase', '--verify', '-f', 'NRF52',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinresetenable', '-f', 'NRF52', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF52', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF52', None, False, False, True, False):
+    TC('NRF52', None, False, False, True, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectoranduicrerase',
       '--verify', '-f', 'NRF52', '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--pinresetenable', '-f', 'NRF52', '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF52', '--snr', TEST_OVR_SNR]),
 
-    TC('NRF52', None, False, True, False, False):
+    TC('NRF52', None, False, True, False, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectoranduicrerase',
       '--verify', '-f', 'NRF52', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--reset', '-f', 'NRF52', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF52', None, True, False, False, False):
+    TC('NRF52', None, True, False, False, False, True):
     (['nrfjprog', '--recover', '-f', 'NRF52', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--program', RC_KERNEL_HEX, '--sectoranduicrerase',
       '--verify', '-f', 'NRF52', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinresetenable', '-f', 'NRF52', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF52', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF52', None, True, True, True, True):
+    TC('NRF52', None, True, True, True, True, True):
     (['nrfjprog', '--recover', '-f', 'NRF52', '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--program', RC_KERNEL_HEX, '--chiperase', '--verify', '-f', 'NRF52',
       '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--reset', '-f', 'NRF52', '--snr', TEST_OVR_SNR]),
 
+    TC('NRF52', None, False, False, False, False, False):
+    (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectoranduicrerase',
+      '--verify', '-f', 'NRF52', '--snr', TEST_DEF_SNR]),
+
     # -------------------------------------------------------------------------
     # NRF53 APP only
     #
-    #  family   CP     recov  soft   snr    erase
+    #  family   CP     recov  soft   snr    erase  reset
 
-    TC('NRF53', 'APP', False, False, False, False):
+    TC('NRF53', 'APP', False, False, False, False, True):
     (['nrfjprog', '--program', NRF5340_APP_ONLY_HEX, '--sectorerase',
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_APPLICATION'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'APP', False, False, False, True):
+    TC('NRF53', 'APP', False, False, False, True, True):
     (['nrfjprog', '--program', NRF5340_APP_ONLY_HEX, '--chiperase',
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_APPLICATION'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'APP', False, False, True, False):
+    TC('NRF53', 'APP', False, False, True, False, True):
     (['nrfjprog', '--program', NRF5340_APP_ONLY_HEX, '--sectorerase',
       '--verify', '-f', 'NRF53', '--snr', TEST_OVR_SNR, '--coprocessor', 'CP_APPLICATION'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_OVR_SNR]),
 
-    TC('NRF53', 'APP', False, True, False, False):
+    TC('NRF53', 'APP', False, True, False, False, True):
     (['nrfjprog', '--program', NRF5340_APP_ONLY_HEX, '--sectorerase',
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_APPLICATION'],
      ['nrfjprog', '--reset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'APP', True, False, False, False):
+    TC('NRF53', 'APP', True, False, False, False, True):
     (['nrfjprog', '--recover', '-f', 'NRF53', '--coprocessor', 'CP_NETWORK',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--recover', '-f', 'NRF53', '--snr', TEST_DEF_SNR],
@@ -176,7 +186,7 @@ EXPECTED_RESULTS = {
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_APPLICATION'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'APP', True, True, True, True):
+    TC('NRF53', 'APP', True, True, True, True, True):
     (['nrfjprog', '--recover', '-f', 'NRF53', '--coprocessor', 'CP_NETWORK',
       '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--recover', '-f', 'NRF53', '--snr', TEST_OVR_SNR],
@@ -184,32 +194,36 @@ EXPECTED_RESULTS = {
       '--verify', '-f', 'NRF53', '--snr', TEST_OVR_SNR, '--coprocessor', 'CP_APPLICATION'],
      ['nrfjprog', '--reset', '-f', 'NRF53', '--snr', TEST_OVR_SNR]),
 
+    TC('NRF53', 'APP', False, False, False, False, False):
+    (['nrfjprog', '--program', NRF5340_APP_ONLY_HEX, '--sectorerase',
+      '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_APPLICATION']),
+
     # -------------------------------------------------------------------------
     # NRF53 NET only
     #
-    #  family   CP     recov  soft   snr    erase
+    #  family   CP     recov  soft   snr    erase  reset
 
-    TC('NRF53', 'NET', False, False, False, False):
+    TC('NRF53', 'NET', False, False, False, False, True):
     (['nrfjprog', '--program', NRF5340_NET_ONLY_HEX, '--sectorerase',
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_NETWORK'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'NET', False, False, False, True):
+    TC('NRF53', 'NET', False, False, False, True, True):
     (['nrfjprog', '--program', NRF5340_NET_ONLY_HEX, '--chiperase',
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_NETWORK'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'NET', False, False, True, False):
+    TC('NRF53', 'NET', False, False, True, False, True):
     (['nrfjprog', '--program', NRF5340_NET_ONLY_HEX, '--sectorerase',
       '--verify', '-f', 'NRF53', '--snr', TEST_OVR_SNR, '--coprocessor', 'CP_NETWORK'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_OVR_SNR]),
 
-    TC('NRF53', 'NET', False, True, False, False):
+    TC('NRF53', 'NET', False, True, False, False, True):
     (['nrfjprog', '--program', NRF5340_NET_ONLY_HEX, '--sectorerase',
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_NETWORK'],
      ['nrfjprog', '--reset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'NET', True, False, False, False):
+    TC('NRF53', 'NET', True, False, False, False, True):
     (['nrfjprog', '--recover', '-f', 'NRF53', '--coprocessor', 'CP_NETWORK',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--recover', '-f', 'NRF53', '--snr', TEST_DEF_SNR],
@@ -217,7 +231,7 @@ EXPECTED_RESULTS = {
       '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_NETWORK'],
      ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF53', 'NET', True, True, True, True):
+    TC('NRF53', 'NET', True, True, True, True, True):
     (['nrfjprog', '--recover', '-f', 'NRF53', '--coprocessor', 'CP_NETWORK',
       '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--recover', '-f', 'NRF53', '--snr', TEST_OVR_SNR],
@@ -225,12 +239,16 @@ EXPECTED_RESULTS = {
       '--verify', '-f', 'NRF53', '--snr', TEST_OVR_SNR, '--coprocessor', 'CP_NETWORK'],
      ['nrfjprog', '--reset', '-f', 'NRF53', '--snr', TEST_OVR_SNR]),
 
+    TC('NRF53', 'NET', False, False, False, False, False):
+    (['nrfjprog', '--program', NRF5340_NET_ONLY_HEX, '--sectorerase',
+      '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR, '--coprocessor', 'CP_NETWORK']),
+
     # -------------------------------------------------------------------------
     # NRF53 APP+NET
     #
-    #  family   CP     recov  soft   snr    erase
+    #  family    CP        recov  soft   snr    erase  reset
 
-    TC('NRF53', 'APP+NET', False, False, False, False):
+    TC('NRF53', 'APP+NET', False, False, False, False, True):
     (lambda tmpdir, infile: \
         (['nrfjprog',
           '--program',
@@ -244,7 +262,7 @@ EXPECTED_RESULTS = {
           '--coprocessor', 'CP_APPLICATION'],
          ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR])),
 
-    TC('NRF53', 'APP+NET', False, False, False, True):
+    TC('NRF53', 'APP+NET', False, False, False, True, True):
     (lambda tmpdir, infile: \
         (['nrfjprog',
           '--program',
@@ -258,7 +276,7 @@ EXPECTED_RESULTS = {
           '--coprocessor', 'CP_APPLICATION'],
          ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR])),
 
-    TC('NRF53', 'APP+NET', False, False, True, False):
+    TC('NRF53', 'APP+NET', False, False, True, False, True):
     (lambda tmpdir, infile: \
         (['nrfjprog',
           '--program',
@@ -272,7 +290,7 @@ EXPECTED_RESULTS = {
           '--coprocessor', 'CP_APPLICATION'],
          ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_OVR_SNR])),
 
-    TC('NRF53', 'APP+NET', False, True, False, False):
+    TC('NRF53', 'APP+NET', False, True, False, False, True):
     (lambda tmpdir, infile: \
         (['nrfjprog',
           '--program',
@@ -286,7 +304,7 @@ EXPECTED_RESULTS = {
           '--coprocessor', 'CP_APPLICATION'],
          ['nrfjprog', '--reset', '-f', 'NRF53', '--snr', TEST_DEF_SNR])),
 
-    TC('NRF53', 'APP+NET', True, False, False, False):
+    TC('NRF53', 'APP+NET', True, False, False, False, True):
     (lambda tmpdir, infile: \
         (['nrfjprog', '--recover', '-f', 'NRF53', '--coprocessor', 'CP_NETWORK',
           '--snr', TEST_DEF_SNR],
@@ -303,7 +321,7 @@ EXPECTED_RESULTS = {
           '--coprocessor', 'CP_APPLICATION'],
          ['nrfjprog', '--pinreset', '-f', 'NRF53', '--snr', TEST_DEF_SNR])),
 
-    TC('NRF53', 'APP+NET', True, True, True, True):
+    TC('NRF53', 'APP+NET', True, True, True, True, True):
     (lambda tmpdir, infile: \
         (['nrfjprog', '--recover', '-f', 'NRF53', '--coprocessor', 'CP_NETWORK',
           '--snr', TEST_OVR_SNR],
@@ -320,41 +338,58 @@ EXPECTED_RESULTS = {
           '--coprocessor', 'CP_APPLICATION'],
          ['nrfjprog', '--reset', '-f', 'NRF53', '--snr', TEST_OVR_SNR])),
 
+    TC('NRF53', 'APP+NET', False, False, False, False, False):
+    (lambda tmpdir, infile: \
+        (['nrfjprog',
+          '--program',
+          os.fspath(tmpdir / 'GENERATED_CP_NETWORK_' + Path(infile).name),
+          '--sectorerase', '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR,
+          '--coprocessor', 'CP_NETWORK'],
+         ['nrfjprog',
+          '--program',
+          os.fspath(tmpdir / 'GENERATED_CP_APPLICATION_' + Path(infile).name),
+          '--sectorerase', '--verify', '-f', 'NRF53', '--snr', TEST_DEF_SNR,
+          '--coprocessor', 'CP_APPLICATION'])),
+
     # -------------------------------------------------------------------------
     # NRF91
     #
-    #  family   CP    recov  soft   snr    erase
-    TC('NRF91', None, False, False, False, False):
+    #  family   CP    recov  soft   snr    erase  reset
+    TC('NRF91', None, False, False, False, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF91',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF91', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF91', None, False, False, False, True):
+    TC('NRF91', None, False, False, False, True, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--chiperase', '--verify', '-f', 'NRF91',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF91', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF91', None, False, False, True, False):
+    TC('NRF91', None, False, False, True, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF91',
       '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF91', '--snr', TEST_OVR_SNR]),
 
-    TC('NRF91', None, False, True, False, False):
+    TC('NRF91', None, False, True, False, False, True):
     (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF91',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--reset', '-f', 'NRF91', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF91', None, True, False, False, False):
+    TC('NRF91', None, True, False, False, False, True):
     (['nrfjprog', '--recover', '-f', 'NRF91', '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF91',
       '--snr', TEST_DEF_SNR],
      ['nrfjprog', '--pinreset', '-f', 'NRF91', '--snr', TEST_DEF_SNR]),
 
-    TC('NRF91', None, True, True, True, True):
+    TC('NRF91', None, True, True, True, True, True):
     (['nrfjprog', '--recover', '-f', 'NRF91', '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--program', RC_KERNEL_HEX, '--chiperase', '--verify', '-f', 'NRF91',
       '--snr', TEST_OVR_SNR],
      ['nrfjprog', '--reset', '-f', 'NRF91', '--snr', TEST_OVR_SNR]),
+
+    TC('NRF91', None, False, False, False, False, False):
+    (['nrfjprog', '--program', RC_KERNEL_HEX, '--sectorerase', '--verify', '-f', 'NRF91',
+      '--snr', TEST_DEF_SNR]),
 }
 
 #
@@ -389,8 +424,9 @@ def id_fn(test_case):
     sn = 'default snr' if test_case.snr else 'override snr'
     e = 'chip erase' if test_case.erase else 'sector[anduicr] erase'
     r = 'recover' if test_case.recover else 'no recover'
+    rst = 'reset' if test_case.reset else 'no reset'
 
-    return f'{test_case.family}{cp}, {s}, {sn}, {e}, {r}'
+    return f'{test_case.family}{cp}, {s}, {sn}, {e}, {r}, {rst}'
 
 def fix_up_runner_config(test_case, runner_config, tmpdir):
     # Helper that adjusts the common runner_config fixture for our
@@ -442,6 +478,7 @@ def test_nrfjprog_init(check_call, get_snr, require, test_case,
                                   test_case.softreset,
                                   snr,
                                   erase=test_case.erase,
+                                  reset=test_case.reset,
                                   recover=test_case.recover)
 
     with patch('os.path.isfile', side_effect=os_path_isfile_patch):
@@ -452,7 +489,10 @@ def test_nrfjprog_init(check_call, get_snr, require, test_case,
         assert (check_call.call_args_list ==
                 [call(x) for x in expected(tmpdir, runner_config.hex_file)])
     else:
-        assert check_call.call_args_list == [call(x) for x in expected]
+        if test_case.reset:
+            assert check_call.call_args_list == [call(x) for x in expected]
+        else:
+            assert check_call.call_args_list == [call(expected)]
 
     if snr is None:
         get_snr.assert_called_once_with('*')
@@ -476,6 +516,10 @@ def test_nrfjprog_create(check_call, get_snr, require, test_case,
         args.extend(['--dev-id', TEST_OVR_SNR])
     if test_case.erase:
         args.append('--erase')
+    if test_case.reset:
+        args.append('--reset')
+    else:
+        args.append('--no-reset')
     if test_case.recover:
         args.append('--recover')
 
@@ -491,7 +535,10 @@ def test_nrfjprog_create(check_call, get_snr, require, test_case,
         assert (check_call.call_args_list ==
                 [call(x) for x in expected(tmpdir, runner_config.hex_file)])
     else:
-        assert check_call.call_args_list == [call(x) for x in expected]
+        if test_case.reset:
+            assert check_call.call_args_list == [call(x) for x in expected]
+        else:
+            assert check_call.call_args_list == [call(expected)]
 
     if not test_case.snr:
         get_snr.assert_called_once_with('*')
diff --git a/scripts/west_commands/tests/test_stm32cubeprogrammer.py b/scripts/west_commands/tests/test_stm32cubeprogrammer.py
index 5f756d01b2..4e6e2c94e3 100644
--- a/scripts/west_commands/tests/test_stm32cubeprogrammer.py
+++ b/scripts/west_commands/tests/test_stm32cubeprogrammer.py
@@ -384,7 +384,7 @@ def test_stm32cubeprogrammer_create(
     if tc["frequency"]:
         args.extend(["--frequency", tc["frequency"]])
     if tc["reset_mode"]:
-        args.extend(["--reset", tc["reset_mode"]])
+        args.extend(["--reset-mode", tc["reset_mode"]])
     if tc["conn_modifiers"]:
         args.extend(["--conn-modifiers", tc["conn_modifiers"]])
     if tc["cli"]:

<?PHP
$Exclusion = array(
	'XOSC',
	'_OSC',
	'_EXT_MAINCK',
	'SAME70_PLLA_MULA',
	'SAME70_PLLA_DIVA',
	'SAMV71_PLLA_MULA',
	'SAMV71_PLLA_DIVA',
	'|| SOC_PART_NUMBER_SAMV71Q21B',
	'|| SOC_PART_NUMBER_SAME70Q21B',
	'SOC_GECKO_EMU_DCDC',
	'SOC_MEC1501_VTR3_1_8V'
);

$input = explode("\n", './zybo/zybo_defconfig
./xmc45_relax_kit/xmc45_relax_kit_defconfig
./xiao_ble/xiao_ble_defconfig
./we_proteus3ev_nrf52840/we_proteus3ev_nrf52840_defconfig
./we_proteus2ev_nrf52832/we_proteus2ev_nrf52832_defconfig
./we_ophelia1ev_nrf52805/we_ophelia1ev_nrf52805_defconfig
./waveshare_open103z/waveshare_open103z_defconfig
./warp7_m4/warp7_m4_defconfig
./v2m_musca_s1/v2m_musca_s1_ns_defconfig
./v2m_musca_s1/v2m_musca_s1_defconfig
./v2m_musca_b1/v2m_musca_b1_ns_defconfig
./v2m_musca_b1/v2m_musca_b1_defconfig
./v2m_beetle/v2m_beetle_defconfig
./usb_kw24d512/usb_kw24d512_defconfig
./udoo_neo_full_m4/udoo_neo_full_m4_defconfig
./ubx_evkninab4_nrf52833/ubx_evkninab4_nrf52833_defconfig
./ubx_evkninab3_nrf52840/ubx_evkninab3_nrf52840_defconfig
./ubx_evkninab1_nrf52832/ubx_evkninab1_nrf52832_defconfig
./ubx_evkannab1_nrf52832/ubx_evkannab1_nrf52832_defconfig
./ubx_bmd380eval_nrf52840/ubx_bmd380eval_nrf52840_defconfig
./ubx_bmd360eval_nrf52811/ubx_bmd360eval_nrf52811_defconfig
./ubx_bmd345eval_nrf52840/ubx_bmd345eval_nrf52840_defconfig
./ubx_bmd340eval_nrf52840/ubx_bmd340eval_nrf52840_defconfig
./ubx_bmd330eval_nrf52810/ubx_bmd330eval_nrf52810_defconfig
./ubx_bmd300eval_nrf52832/ubx_bmd300eval_nrf52832_defconfig
./twr_kv58f220m/twr_kv58f220m_defconfig
./twr_ke18f/twr_ke18f_defconfig
./thingy53_nrf5340/thingy53_nrf5340_cpunet_defconfig
./thingy53_nrf5340/thingy53_nrf5340_cpuapp_ns_defconfig
./thingy53_nrf5340/thingy53_nrf5340_cpuapp_defconfig
./thingy52_nrf52832/thingy52_nrf52832_defconfig
./teensy4/teensy41_defconfig
./teensy4/teensy40_defconfig
./tdk_robokit1/tdk_robokit1_defconfig
./swan_r5/swan_r5_defconfig
./stm32vl_disco/stm32vl_disco_defconfig
./stm32mp157c_dk2/stm32mp157c_dk2_defconfig
./stm32l562e_dk/stm32l562e_dk_ns_defconfig
./stm32l562e_dk/stm32l562e_dk_defconfig
./stm32l496g_disco/stm32l496g_disco_defconfig
./stm32l476g_disco/stm32l476g_disco_defconfig
./stm32l1_disco/stm32l1_disco_defconfig
./stm32h7b3i_dk/stm32h7b3i_dk_defconfig
./stm32h747i_disco/stm32h747i_disco_m7_defconfig
./stm32h747i_disco/stm32h747i_disco_m4_defconfig
./stm32h735g_disco/stm32h735g_disco_defconfig
./stm32g081b_eval/stm32g081b_eval_defconfig
./stm32g071b_disco/stm32g071b_disco_defconfig
./stm32g0316_disco/stm32g0316_disco_defconfig
./stm32f769i_disco/stm32f769i_disco_defconfig
./stm32f7508_dk/stm32f7508_dk_defconfig
./stm32f746g_disco/stm32f746g_disco_defconfig
./stm32f723e_disco/stm32f723e_disco_defconfig
./stm32f4_disco/stm32f4_disco_defconfig
./stm32f469i_disco/stm32f469i_disco_defconfig
./stm32f429i_disc1/stm32f429i_disc1_defconfig
./stm32f412g_disco/stm32f412g_disco_defconfig
./stm32f411e_disco/stm32f411e_disco_defconfig
./stm32f401_mini/stm32f401_mini_defconfig
./stm32f3_disco/stm32f3_disco_defconfig
./stm32f103_mini/stm32f103_mini_defconfig
./stm32f0_disco/stm32f0_disco_defconfig
./stm32f072b_disco/stm32f072b_disco_defconfig
./stm32f072_eval/stm32f072_eval_defconfig
./stm32f030_demo/stm32f030_demo_defconfig
./stm32_min_dev/stm32_min_dev_blue_defconfig
./stm32_min_dev/stm32_min_dev_black_defconfig
./stm32373c_eval/stm32373c_eval_defconfig
./stm3210c_eval/stm3210c_eval_defconfig
./steval_fcu001v1/steval_fcu001v1_defconfig
./sparkfun_thing_plus_nrf9160/sparkfun_thing_plus_nrf9160_ns_defconfig
./sparkfun_thing_plus_nrf9160/sparkfun_thing_plus_nrf9160_defconfig
./serpente/serpente_defconfig
./sensortile_box/sensortile_box_defconfig
./segger_trb_stm32f407/segger_trb_stm32f407_defconfig
./seeeduino_xiao/seeeduino_xiao_defconfig
./scobc_module1/scobc_module1_defconfig
./sam_v71_xult/sam_v71b_xult_defconfig
./sam_v71_xult/sam_v71_xult_defconfig
./sam_e70_xplained/sam_e70b_xplained_defconfig
./sam_e70_xplained/sam_e70_xplained_defconfig
./sam4s_xplained/sam4s_xplained_defconfig
./sam4l_ek/sam4l_ek_defconfig
./sam4e_xpro/sam4e_xpro_defconfig
./ruuvi_ruuvitag/ruuvi_ruuvitag_defconfig
./rpi_pico/rpi_pico_defconfig
./ronoth_lodev/ronoth_lodev_defconfig
./rm1xx_dvk/rm1xx_dvk_defconfig
./reel_board/reel_board_v2_defconfig
./reel_board/reel_board_defconfig
./rddrone_fmuk66/rddrone_fmuk66_defconfig
./rcar_h3ulcb/rcar_h3ulcb_cr7_defconfig
./rcar_h3_salvatorx/rcar_h3_salvatorx_cr7_defconfig
./rak5010_nrf52840/rak5010_nrf52840_defconfig
./rak4631_nrf52840/rak4631_nrf52840_defconfig
./quick_feather/quick_feather_defconfig
./qemu_cortex_r5/qemu_cortex_r5_defconfig
./qemu_cortex_m3/qemu_cortex_m3_defconfig
./qemu_cortex_m0/qemu_cortex_m0_defconfig
./qemu_cortex_a9/qemu_cortex_a9_defconfig
./pinnacle_100_dvk/pinnacle_100_dvk_defconfig
./pinetime_devkit0/pinetime_devkit0_defconfig
./pico_pi_m4/pico_pi_m4_defconfig
./particle_xenon/particle_xenon_defconfig
./particle_boron/particle_boron_defconfig
./particle_argon/particle_argon_defconfig
./pan1782_evb/pan1782_evb_defconfig
./pan1781_evb/pan1781_evb_defconfig
./pan1780_evb/pan1780_evb_defconfig
./pan1770_evb/pan1770_evb_defconfig
./olimexino_stm32/olimexino_stm32_defconfig
./olimex_stm32_p405/olimex_stm32_p405_defconfig
./olimex_stm32_h407/olimex_stm32_h407_defconfig
./olimex_stm32_h405/olimex_stm32_h405_defconfig
./olimex_stm32_h103/olimex_stm32_h103_defconfig
./olimex_stm32_e407/olimex_stm32_e407_defconfig
./olimex_lora_stm32wl_devkit/olimex_lora_stm32wl_devkit_defconfig
./nuvoton_pfm_m487/nuvoton_pfm_m487_defconfig
./nucleo_wl55jc/nucleo_wl55jc_defconfig
./nucleo_wb55rg/nucleo_wb55rg_defconfig
./nucleo_u575zi_q/nucleo_u575zi_q_defconfig
./nucleo_l552ze_q/nucleo_l552ze_q_ns_defconfig
./nucleo_l552ze_q/nucleo_l552ze_q_defconfig
./nucleo_l4r5zi/nucleo_l4r5zi_defconfig
./nucleo_l496zg/nucleo_l496zg_defconfig
./nucleo_l476rg/nucleo_l476rg_defconfig
./nucleo_l452re/nucleo_l452re_p_defconfig
./nucleo_l452re/nucleo_l452re_defconfig
./nucleo_l433rc_p/nucleo_l433rc_p_defconfig
./nucleo_l432kc/nucleo_l432kc_defconfig
./nucleo_l412rb_p/nucleo_l412rb_p_defconfig
./nucleo_l152re/nucleo_l152re_defconfig
./nucleo_l073rz/nucleo_l073rz_defconfig
./nucleo_l053r8/nucleo_l053r8_defconfig
./nucleo_l031k6/nucleo_l031k6_defconfig
./nucleo_l011k4/nucleo_l011k4_defconfig
./nucleo_h7a3zi_q/nucleo_h7a3zi_q_defconfig
./nucleo_h753zi/nucleo_h753zi_defconfig
./nucleo_h745zi_q/nucleo_h745zi_q_m7_defconfig
./nucleo_h745zi_q/nucleo_h745zi_q_m4_defconfig
./nucleo_h743zi/nucleo_h743zi_defconfig
./nucleo_h723zg/nucleo_h723zg_defconfig
./nucleo_g474re/nucleo_g474re_defconfig
./nucleo_g431rb/nucleo_g431rb_defconfig
./nucleo_g0b1re/nucleo_g0b1re_defconfig
./nucleo_g071rb/nucleo_g071rb_defconfig
./nucleo_g031k8/nucleo_g031k8_defconfig
./nucleo_f767zi/nucleo_f767zi_defconfig
./nucleo_f756zg/nucleo_f756zg_defconfig
./nucleo_f746zg/nucleo_f746zg_defconfig
./nucleo_f446ze/nucleo_f446ze_defconfig
./nucleo_f446re/nucleo_f446re_defconfig
./nucleo_f429zi/nucleo_f429zi_defconfig
./nucleo_f413zh/nucleo_f413zh_defconfig
./nucleo_f412zg/nucleo_f412zg_defconfig
./nucleo_f411re/nucleo_f411re_defconfig
./nucleo_f410rb/nucleo_f410rb_defconfig
./nucleo_f401re/nucleo_f401re_defconfig
./nucleo_f334r8/nucleo_f334r8_defconfig
./nucleo_f303re/nucleo_f303re_defconfig
./nucleo_f303k8/nucleo_f303k8_defconfig
./nucleo_f302r8/nucleo_f302r8_defconfig
./nucleo_f207zg/nucleo_f207zg_defconfig
./nucleo_f103rb/nucleo_f103rb_defconfig
./nucleo_f091rc/nucleo_f091rc_defconfig
./nucleo_f070rb/nucleo_f070rb_defconfig
./nucleo_f031k6/nucleo_f031k6_defconfig
./nucleo_f030r8/nucleo_f030r8_defconfig
./nrf9160dk_nrf52840/nrf9160dk_nrf52840_defconfig
./nrf9160_innblue22/nrf9160_innblue22_ns_defconfig
./nrf9160_innblue22/nrf9160_innblue22_defconfig
./nrf9160_innblue21/nrf9160_innblue21_ns_defconfig
./nrf9160_innblue21/nrf9160_innblue21_defconfig
./nrf5340dk_nrf5340/nrf5340dk_nrf5340_cpunet_defconfig
./nrf5340dk_nrf5340/nrf5340dk_nrf5340_cpuapp_ns_defconfig
./nrf5340dk_nrf5340/nrf5340dk_nrf5340_cpuapp_defconfig
./nrf52dk_nrf52832/nrf52dk_nrf52832_defconfig
./nrf52dk_nrf52810/nrf52dk_nrf52810_defconfig
./nrf52dk_nrf52805/nrf52dk_nrf52805_defconfig
./nrf52_vbluno52/nrf52_vbluno52_defconfig
./nrf52_sparkfun/nrf52_sparkfun_defconfig
./nrf52_blenano2/nrf52_blenano2_defconfig
./nrf52_adafruit_feather/nrf52_adafruit_feather_defconfig
./nrf52840dongle_nrf52840/nrf52840dongle_nrf52840_defconfig
./nrf52840dk_nrf52811/nrf52840dk_nrf52811_defconfig
./nrf52840_papyr/nrf52840_papyr_defconfig
./nrf52840_mdk_usb_dongle/nrf52840_mdk_usb_dongle_defconfig
./nrf52840_mdk/nrf52840_mdk_defconfig
./nrf52840_blip/nrf52840_blip_defconfig
./nrf52833dk_nrf52833/nrf52833dk_nrf52833_defconfig
./nrf52833dk_nrf52820/nrf52833dk_nrf52820_defconfig
./nrf52832_mdk/nrf52832_mdk_defconfig
./nrf51dongle_nrf51422/nrf51dongle_nrf51422_defconfig
./nrf51_vbluno51/nrf51_vbluno51_defconfig
./nrf51_blenano/nrf51_blenano_defconfig
./nrf51_ble400/nrf51_ble400_defconfig
./nrf21540dk_nrf52840/nrf21540dk_nrf52840_defconfig
./npcx9m6f_evb/npcx9m6f_evb_defconfig
./npcx7m6fb_evb/npcx7m6fb_evb_defconfig
./msp_exp432p401r_launchxl/msp_exp432p401r_launchxl_defconfig
./mps3_an547/mps3_an547_ns_defconfig
./mps3_an547/mps3_an547_defconfig
./mps2_an521/mps2_an521_remote_defconfig
./mps2_an521/mps2_an521_ns_defconfig
./mps2_an521/mps2_an521_defconfig
./mps2_an385/mps2_an385_defconfig
./mm_swiftio/mm_swiftio_defconfig
./mm_feather/mm_feather_defconfig
./mimxrt685_evk/mimxrt685_evk_cm33_defconfig
./mimxrt595_evk/mimxrt595_evk_cm33_defconfig
./mimxrt1170_evk/mimxrt1170_evk_cm7_defconfig
./mimxrt1170_evk/mimxrt1170_evk_cm4_defconfig
./mimxrt1160_evk/mimxrt1160_evk_cm7_defconfig
./mimxrt1160_evk/mimxrt1160_evk_cm4_defconfig
./mimxrt1064_evk/mimxrt1064_evk_defconfig
./mimxrt1060_evk/mimxrt1060_evkb_defconfig
./mimxrt1060_evk/mimxrt1060_evk_hyperflash_defconfig
./mimxrt1060_evk/mimxrt1060_evk_defconfig
./mimxrt1050_evk/mimxrt1050_evk_qspi_defconfig
./mimxrt1050_evk/mimxrt1050_evk_defconfig
./mimxrt1024_evk/mimxrt1024_evk_defconfig
./mimxrt1020_evk/mimxrt1020_evk_defconfig
./mimxrt1015_evk/mimxrt1015_evk_defconfig
./mimxrt1010_evk/mimxrt1010_evk_defconfig
./mimx8mq_evk/mimx8mq_evk_cm4_defconfig
./mimx8mp_evk/mimx8mp_evk_itcm_defconfig
./mimx8mp_evk/mimx8mp_evk_ddr_defconfig
./mimx8mm_evk/mimx8mm_evk_defconfig
./mikroe_mini_m4_for_stm32/mikroe_mini_m4_for_stm32_defconfig
./mikroe_clicker_2/mikroe_clicker_2_defconfig
./mercury_xu/mercury_xu_defconfig
./mec2016evb_assy6797/mec2016evb_assy6797_defconfig
./mec172xevb_assy6906/mec172xevb_assy6906_defconfig
./mec15xxevb_assy6853/mec15xxevb_assy6853_defconfig
./mec1501modular_assy6885/mec1501modular_assy6885_defconfig
./lpcxpresso55s69/lpcxpresso55s69_ns_defconfig
./lpcxpresso55s69/lpcxpresso55s69_cpu1_defconfig
./lpcxpresso55s69/lpcxpresso55s69_cpu0_defconfig
./lpcxpresso55s36/lpcxpresso55s36_defconfig
./lpcxpresso55s28/lpcxpresso55s28_defconfig
./lpcxpresso55s16/lpcxpresso55s16_defconfig
./lpcxpresso55s06/lpcxpresso55s06_defconfig
./lpcxpresso54114/lpcxpresso54114_m4_defconfig
./lpcxpresso54114/lpcxpresso54114_m0_defconfig
./lpcxpresso51u68/lpcxpresso51u68_defconfig
./lpcxpresso11u68/lpcxpresso11u68_defconfig
./lora_e5_dev_board/lora_e5_dev_board_defconfig
./legend/legend_defconfig
./ip_k66f/ip_k66f_defconfig
./holyiot_yj16019/holyiot_yj16019_defconfig
./hexiwear_kw40z/hexiwear_kw40z_defconfig
./hexiwear_k64/hexiwear_k64_defconfig
./google_kukui/google_kukui_defconfig
./gd32f470i_eval/gd32f470i_eval_defconfig
./gd32f450z_eval/gd32f450z_eval_defconfig
./gd32f450v_start/gd32f450v_start_defconfig
./gd32f450i_eval/gd32f450i_eval_defconfig
./gd32f407v_start/gd32f407v_start_defconfig
./gd32f403z_eval/gd32f403z_eval_defconfig
./gd32f350r_eval/gd32f350r_eval_defconfig
./gd32e507z_eval/gd32e507z_eval_defconfig
./gd32e507v_start/gd32e507v_start_defconfig
./gd32e103v_eval/gd32e103v_eval_defconfig
./fvp_baser_aemv8r_aarch32/fvp_baser_aemv8r_aarch32_defconfig
./frdm_kw41z/frdm_kw41z_defconfig
./frdm_kl25z/frdm_kl25z_defconfig
./frdm_k82f/frdm_k82f_defconfig
./frdm_k64f/frdm_k64f_defconfig
./frdm_k22f/frdm_k22f_defconfig
./faze/faze_defconfig
./efr32mg_sltb004a/efr32mg_sltb004a_defconfig
./efr32_radio/efr32_radio_brd4255a_defconfig
./efr32_radio/efr32_radio_brd4250b_defconfig
./efr32_radio/efr32_radio_brd4180a_defconfig
./efr32_radio/efr32_radio_brd4104a_defconfig
./efm32wg_stk3800/efm32wg_stk3800_defconfig
./efm32pg_stk3402a/efm32pg_stk3402a_jg_defconfig
./efm32pg_stk3402a/efm32pg_stk3402a_defconfig
./efm32pg_stk3401a/efm32pg_stk3401a_defconfig
./efm32hg_slstk3400a/efm32hg_slstk3400a_defconfig
./efm32gg_stk3701a/efm32gg_stk3701a_defconfig
./efm32gg_slwstk6121a/efm32gg_slwstk6121a_defconfig
./ebyte_e73_tbb_nrf52832/ebyte_e73_tbb_nrf52832_defconfig
./dragino_nbsn95/dragino_nbsn95_defconfig
./dragino_lsn50/dragino_lsn50_defconfig
./disco_l475_iot1/disco_l475_iot1_defconfig
./degu_evk/degu_evk_defconfig
./decawave_dwm1001_dev/decawave_dwm1001_dev_defconfig
./da1469x_dk_pro/da1469x_dk_pro_defconfig
./cyclonev_socdk/cyclonev_socdk_defconfig
./cy8ckit_062_wifi_bt/cy8ckit_062_wifi_bt_m4_defconfig
./cy8ckit_062_wifi_bt/cy8ckit_062_wifi_bt_m0_defconfig
./cy8ckit_062_ble/cy8ckit_062_ble_m4_defconfig
./cy8ckit_062_ble/cy8ckit_062_ble_m0_defconfig
./contextualelectronics_abc/contextualelectronics_abc_defconfig
./colibri_imx7d_m4/colibri_imx7d_m4_defconfig
./circuitdojo_feather_nrf9160/circuitdojo_feather_nrf9160_ns_defconfig
./circuitdojo_feather_nrf9160/circuitdojo_feather_nrf9160_defconfig
./cc3235sf_launchxl/cc3235sf_launchxl_defconfig
./cc3220sf_launchxl/cc3220sf_launchxl_defconfig
./cc26x2r1_launchxl/cc26x2r1_launchxl_defconfig
./cc1352r_sensortag/cc1352r_sensortag_defconfig
./cc1352r1_launchxl/cc1352r1_launchxl_defconfig
./bt610/bt610_defconfig
./bt510/bt510_defconfig
./blueclover_plt_demo_v2_nrf52832/blueclover_plt_demo_v2_nrf52832_defconfig
./blackpill_f411ce/blackpill_f411ce_defconfig
./blackpill_f401ce/blackpill_f401ce_defconfig
./blackpill_f401cc/blackpill_f401cc_defconfig
./black_f407zg_pro/black_f407zg_pro_defconfig
./black_f407ve/black_f407ve_defconfig
./bcm958402m2_m7/bcm958402m2_m7_defconfig
./bcm958401m2/bcm958401m2_defconfig
./bbc_microbit_v2/bbc_microbit_v2_defconfig
./bbc_microbit/bbc_microbit_defconfig
./b_u585i_iot02a/b_u585i_iot02a_ns_defconfig
./b_u585i_iot02a/b_u585i_iot02a_defconfig
./b_l4s5i_iot01a/b_l4s5i_iot01a_defconfig
./b_l072z_lrwan1/b_l072z_lrwan1_defconfig
./b_g474e_dpow1/b_g474e_dpow1_defconfig
./atsamr34_xpro/atsamr34_xpro_defconfig
./atsamr21_xpro/atsamr21_xpro_defconfig
./atsaml21_xpro/atsaml21_xpro_defconfig
./atsame54_xpro/atsame54_xpro_defconfig
./atsamd21_xpro/atsamd21_xpro_defconfig
./atsamd20_xpro/atsamd20_xpro_defconfig
./ast1030_evb/ast1030_evb_defconfig
./arty/arty_a7_arm_designstart_m3_defconfig
./arty/arty_a7_arm_designstart_m1_defconfig
./arduino_zero/arduino_zero_defconfig
./arduino_nicla_sense_me/arduino_nicla_sense_me_defconfig
./arduino_nano_33_iot/arduino_nano_33_iot_defconfig
./arduino_nano_33_ble/arduino_nano_33_ble_sense_defconfig
./arduino_nano_33_ble/arduino_nano_33_ble_defconfig
./arduino_mkrzero/arduino_mkrzero_defconfig
./arduino_due/arduino_due_defconfig
./adafruit_trinket_m0/adafruit_trinket_m0_defconfig
./adafruit_itsybitsy_m4_express/adafruit_itsybitsy_m4_express_defconfig
./adafruit_feather_stm32f405/adafruit_feather_stm32f405_defconfig
./adafruit_feather_nrf52840/adafruit_feather_nrf52840_defconfig
./adafruit_feather_m0_basic_proto/adafruit_feather_m0_basic_proto_defconfig
./actinius_icarus_som/actinius_icarus_som_ns_defconfig
./actinius_icarus_som/actinius_icarus_som_defconfig
./actinius_icarus_bee/actinius_icarus_bee_ns_defconfig
./actinius_icarus_bee/actinius_icarus_bee_defconfig
./actinius_icarus/actinius_icarus_ns_defconfig
./actinius_icarus/actinius_icarus_defconfig
');

foreach ($input as $ThisFile)
{
	if (strlen($ThisFile) > 5)
	{
		$Lines = explode("\n", file_get_contents($ThisFile));
		$Stolen = array();
		$BoardName = '';

		foreach ($Lines as $ThisID => $ThisLine)
		{
//echo '1';
			if (substr($ThisLine, 0, 11) == 'CONFIG_SOC_')
			{
				$NewLine = substr($ThisLine, 7);
				$Reject = false;

				foreach ($Exclusion as $ThisExclusion)
				{
					if (strpos($NewLine, $ThisExclusion) !== false)
					{
echo "REJECT: ".$NewLine."\n";
						$Reject = true;
						break;
					}
				}

				if ($Reject == true)
				{
					continue;
				}

				$NewLine = substr($NewLine, 0, strlen($NewLine)-2);
//				$Stolen[] = 'select '.$NewLine;
				$Stolen[] = $NewLine;
				unset($Lines[$ThisID]);
			}
			else if (substr($ThisLine, 0, 13) == 'CONFIG_BOARD_')
			{
//echo '??';
				if ($BoardName != '')
				{
					echo 'PROBLEM!!';
				}
				$BoardName = substr($ThisLine, 13);
				$BoardName = substr($BoardName, 0, strpos($BoardName, '='));
				unset($Lines[$ThisID]);
			}
		}

//re-write ThisFile here...
if (1)
{
$fp = fopen($ThisFile, 'w');
fwrite($fp, implode("\n", $Lines));
fclose($fp);
}

		$InputB = substr($ThisFile, 0, strrpos($ThisFile, '/') + 1).'Kconfig.board';
		if (file_exists($InputB))
		{
			$Lines = explode("\n", file_get_contents($InputB));
			$Start = false;
			$Found = false;

			foreach ($Lines as $ThisID => $ThisLine)
			{
				if (strpos($ThisLine, 'config BOARD_'.$BoardName) !== false)
				{
					$Start = true;
					$Found = true;
				}
				else if (strpos($ThisLine, 'config BOARD_') !== false)
				{
					$Start = false;
				}
				else if (stripos($ThisLine, 'select ') !== false && $Start == true)
				{
					$Check = trim(substr($ThisLine, 7));

					$Reject = false;

					foreach ($Exclusion as $ThisExclusion)
					{
						if (strpos($Check, $ThisExclusion) !== false)
						{
echo 'REJECT: '.$Check."\n";
							$Reject = true;
							break;
						}
					}

					if ($Reject == true)
					{
						continue;
					}

					if (array_search($Check, $Stolen, true) === false)
					{
						$Stolen[] = $Check;
//echo 'NOT FOUND1: '.$Check."\n";
					}

					unset($Lines[$ThisID]);
				}
				else if (stripos($ThisLine, 'depends on ') !== false && $Start == true)
				{
					$Check = trim(substr($ThisLine, 11));

					$Reject = false;

					foreach ($Exclusion as $ThisExclusion)
					{
						if (strpos($Check, $ThisExclusion) !== false)
						{
echo 'REJECT: '.$Check."\n";
							$Reject = true;
							break;
						}
					}

					if ($Reject == true)
					{
						continue;
					}


					if (array_search($Check, $Stolen, true) === false)
					{
						$Stolen[] = $Check;
//echo 'NOT FOUND2: '.$Check."\n";
					}

					unset($Lines[$ThisID]);
				}
				else if ($Start == true)
				{
//					echo 'Removing line '.$ThisLine;
					unset($Lines[$ThisID]);
				}
				else if (strlen($ThisLine) > 4 && substr($ThisLine, 0, 2) != '# ' && substr($ThisLine, 0, 2) != ' #')
				{
//					echo 'Unknown line '.$ThisLine;
				}
			}

if (1)
{
$fp = fopen($InputB, 'w');
fwrite($fp, implode("\n", $Lines));
fclose($fp);
}
//re-write InputB here...
		}


		if ($BoardName == '')
		{
			$BoardName = substr($ThisFile, strrpos($ThisFile, '/') + 1);
			$BoardName = strtoupper(substr($BoardName, 0, strrpos($BoardName, '_')));
		}
echo $BoardName."\n";
print_r($Stolen);

//create Kconfig.<board> here...
		$OutputC = substr($ThisFile, 0, strrpos($ThisFile, '/') + 1).'Kconfig.'.strtolower($BoardName);
$fp = fopen($OutputC, 'w');
fwrite($fp, 'config BOARD_'.$BoardName."\n\tbool\n\tdefault y\n");
//echo 'name: BOARD_'.$BoardName."\n";
foreach ($Stolen as $NewLine)
{
fwrite($fp, "\tselect ".$NewLine."\n");
echo "\tselect ".$NewLine."\n";
}
//echo "\n";
fclose($fp);

	}
//echo $InputB;
//file_get_contents();
//die();
}
?>

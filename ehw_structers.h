/*
 * ehw_structers.h
 *
 *  Created on: Feb 25, 2025
 *      Author: karatay
 */

typedef enum { // eSlave150Enum
	s150_none,
	s150_counter,  	// PROG_Modbus_counter 1
	s150_151_dis_isi,
	s150_151_nem_isi,
	s150_152_cikis_isi,
	s150_152_donus_isi,
	s150_152_delta_isi,
	s150_153_bina_avg,
	s150_153_bina_up,
	s150_152_gaz_status,
	s150_152_gaz_role,
	s150_152_pressure_status,
	s150_152_pressure_role,
	s150_153_gaz_status,
	s150_153_gaz_role,
	s150_153_pressure_status,
	s150_153_pressure_role,
	s150_slave_heartbeat,
	s150_s154_contact_0, // usRegHoldingBuf[8] = pre shwitch
	s150_s154_contact_1, // usRegHoldingBuf[9] = after_switch
	s150_s154_contact_2, // usRegHoldingBuf[10] = role_0; Burner status
	s150_s154_error,
	s150_s154_RE_Modbus_relay_status,
	s150_s154_phase_type,
	s150_s154_R_voltage,
	s150_s154_S_voltage,
	s150_s154_T_voltage,
	s150_s155_RE_Modbus_relay_status,
	s150_s155_phase_type,
	s150_s155_R_voltage,
	s150_s155_S_voltage,
	s150_s155_T_voltage,
	s150_master_data_income, // Modbustan bilgi gönderirse == 1 olması

} eSlave150Enum;

typedef enum { // eSlave151Enum
	s151_none,
	s151_counter,  	// PROG_Modbus_counter 1
	s151_DHT_Modbus_config_RH, // DHT_Modbus_config_RHI 2
	s151_DHT_Modbus_config_TC, // DHT_Modbus_config_TCI 4
	s151_DL_Modbus_config_0, // DL_Modbus_config_0 6
	s151_DL_Modbus_config_1, // DL_Modbus_config_1 7
	s151_DL_Modbus_config_2, // DL_Modbus_config_2 8
	s151_DL_Modbus_config, // DL_Modbus_config_3 9
	s151_DL_Modbus_analogs_adc_values, // DL_Modbus_analogs_adc_values 10
	s151_DL_Modbus_analogs_dayligh_status, // DL_Modbus_analogs_dayligh_status 11
	s151_DL_Modbus_analogs_relay_status, // DL_Modbus_analogs_relay_status 12
	s151_RA_Modbus_config_0, // RA_Modbus_config_0 13
	s151_RA_Modbus_config_1, // RA_Modbus_config_1 14
	s151_RA_Modbus_config_2, // RA_Modbus_config_2 15
	s151_RA_Modbus_config_3, // RA_Modbus_config_3 16
	s151_RA_Modbus_analogs_adc_values, // RA_Modbus_analogs_adc_values 17
	s151_RA_Modbus_analogs_rain_status, // RA_Modbus_analogs_rain_status 18
	s151_RA_Modbus_analogs_relay_status, // RA_Modbus_analogs_relay_status//  19
	s151_GA_Modbus_config_0, // RA_Modbus_config_0 13
	s151_GA_Modbus_config_1, // RA_Modbus_config_1 14
	s151_GA_Modbus_config_2, // RA_Modbus_config_2 15
	s151_GA_Modbus_config_3, // RA_Modbus_config_3 16
	s151_GA_Modbus_analogs_adc_values, // RA_Modbus_analogs_adc_values 17
	s151_GA_Modbus_analogs_gaz_status, // RA_Modbus_analogs_rain_status 18
	s151_GA_Modbus_analogs_relay_status, // RA_Modbus_analogs_relay_status//  19
	s151_master_data_income, // Modbustan bilgi gönderirse == 1 olması
} eSlave151Enum;

typedef enum {  // eSlave152Enum
	s152_none,
	s152_counter,  	// PROG_Modbus_counter 1
	s152_in_value, //usRegHoldingBuf[2] = (uint16_t) (out_heats[0] * 100);
	s152_in_max, //	usRegHoldingBuf[3] = (uint16_t) (out_heats[1] * 100);
	s152_in_min, //usRegHoldingBuf[4] = (uint16_t) (out_heats[2] * 100);
	s152_in_count, //	usRegHoldingBuf[5] = TempSensorCount_out;
	s152_out_value, //usRegHoldingBuf[6] = (uint16_t) (out_heats[0] * 100);
	s152_out_max, //	usRegHoldingBuf[7] = (uint16_t) (out_heats[1] * 100);
	s152_out_min, //usRegHoldingBuf[8] = (uint16_t) (out_heats[2] * 100);
	s152_out_count, //	usRegHoldingBuf[9] = TempSensorCount_out;
	s152_GA_Modbus_analogs_adc_values, // usRegHoldingBuf[10]
	s152_GA_Modbus_analogs_status, // usRegHoldingBuf[11]
	s152_GA_Modbus_analogs_relay_status, // usRegHoldingBuf[12}
	s152_BA_Modbus_analogs_adc_values, // usRegHoldingBuf[13]
	s152_BA_Modbus_analogs_status, // usRegHoldingBuf[14]
	s152_BA_Modbus_analogs_relay_status, // usRegHoldingBuf[15}
	s152_GA_Modbus_config_0, // usRegHoldingBuf[16] = GA_1;
	s152_GA_Modbus_config_1, // usRegHoldingBuf[17] = GA_2;
	s152_GA_Modbus_config_2, // usRegHoldingBuf[18] = GA_3;
	s152_BA_Modbus_config_0, // usRegHoldingBuf[19] = BA_1;
	s152_BA_Modbus_config_1, // usRegHoldingBuf[20] = BA_2;
	s152_BA_Modbus_config_2, // usRegHoldingBuf[21] = BA_3;
	s152_GA_Modbus_analogs_alarm_status, //usRegHoldingBuf[22] = status_alarm;
	s152_delta, //usRegHoldingBuf[23] = (uint16_t) ((out_heats[0] - in_heats[0]) * 100);
	s152_GA_Modbus_config_3, // usRegHoldingBuf[24] = GA_4;
	s152_BA_Modbus_config_3, // usRegHoldingBuf[25] = BA_4;
	s152_master_data_income, // Modbustan bilgi gönderirse == 1 olması
} eSlave152Enum;

typedef enum { // eSlave153Enum
	s153_none,
	s153_counter,  	// PROG_Modbus_counter 1
	s153_up_value, // usRegHoldingBuf[2] = (uint16_t) (up_heats[0] * 100);
	s153_up_max, // usRegHoldingBuf[3] = (uint16_t) (up_heats[1] * 100);
	s153_up_min, // usRegHoldingBuf[4] = (uint16_t) (up_heats[2] * 100);
	s153_up_avg, // usRegHoldingBuf[5] = (uint16_t) up_heats[3];
	s153_up_level, // usRegHoldingBuf[6] = (uint16_t) up_level;
	s153_flat_value, // usRegHoldingBuf[7] = (uint16_t) (flat_heats[0] * 100);
	s153_flat_max, // usRegHoldingBuf[8] = (uint16_t) (flat_heats[1] * 100);
	s153_flat_min, // usRegHoldingBuf[9] = (uint16_t) (flat_heats[2] * 100);
	s153_flat_count, // usRegHoldingBuf[10] = TempSensorCount;
	s153_pipe_0, // usRegHoldingBuf[11] = calc_pipe(0);
	s153_pipe_1, // usRegHoldingBuf[12] = calc_pipe(1);
	s153_pipe_2, // usRegHoldingBuf[13] = calc_pipe(2);
	s153_pipe_3, // usRegHoldingBuf[14] = calc_pipe(3);
	s153_pipe_4, // usRegHoldingBuf[15] = calc_pipe(4);
	s153_pipe_5, // usRegHoldingBuf[16] = calc_pipe(5);
	s153_pipe_6, // usRegHoldingBuf[17] = calc_pipe(6);
	s153_pipe_7, // usRegHoldingBuf[18] = calc_pipe(7);
	s153_GA_Modbus_analogs_adc_values, // usRegHoldingBuf[19] = 4095 - adc_values[0];
	s153_GA_Modbus_analogs_status, // usRegHoldingBuf[20]
	s153_GA_Modbus_analogs_relay_status, // usRegHoldingBuf[21}
	s153_GA_Modbus_config_0, // usRegHoldingBuf[22] = GA_0;
	s153_GA_Modbus_config_1, // usRegHoldingBuf[23] = GA_1;
	s153_GA_Modbus_config_2, // usRegHoldingBuf[24] = GA_2;
	s153_BA_Modbus_config_0, // usRegHoldingBuf[25] = GA_3;
	s153_BA_Modbus_analogs_adc_values, // usRegHoldingBuf[26] = 4095 - adc_values[1];
	s153_BA_Modbus_analogs_status, // usRegHoldingBuf[27]
	s153_BA_Modbus_analogs_relay_status, // usRegHoldingBuf[28}
	s153_BA_Modbus_config_1, // usRegHoldingBuf[29] = BA_0;
	s153_BA_Modbus_config_2, // usRegHoldingBuf[30] = BA_1;
	s153_GA_Modbus_config_3, // usRegHoldingBuf[31] = GA_2;
	s153_BA_Modbus_config_3, // usRegHoldingBuf[32] = BA_3;
	s153_master_data_income, // Modbustan bilgi gönderirse == 1 olması
} eSlave153Enum;

typedef enum { // eSlave154Enum
	s154_none,
	s154_counter,  	// PROG_Modbus_counter 1
	s154_R_voltage, // usRegHoldingBuf[2] = (uint16_t) (up_heats[0] * 100);
	s154_S_voltage, // usRegHoldingBuf[3] = (uint16_t) (up_heats[1] * 100);
	s154_T_voltage, // usRegHoldingBuf[4] = (uint16_t) (up_heats[2] * 100);
	s154_R_amper, // usRegHoldingBuf[5] = (uint16_t) up_heats[3];
	s154_S_amper, // usRegHoldingBuf[6] = (uint16_t) up_level;
	s154_T_amper, // usRegHoldingBuf[7] = (uint16_t) (flat_heats[0] * 100);
	s154_phase_type,
	s154_contact_0, // usRegHoldingBuf[8] = pre shwitch
	s154_contact_1, // usRegHoldingBuf[9] = after_switch
	s154_contact_2, // usRegHoldingBuf[10] = role_0; Burner status
	s154_contact_3, // usRegHoldingBuf[11] = role_1  Fuel status
	s154_contact_4, // usRegHoldingBuf[12] = role_2  Carbon status
	s154_contact_5, // usRegHoldingBuf[13] = role_3  gas status
	s154_contact_6, // usRegHoldingBuf[14] = role_4  oxygen status
	s154_contact_7, // usRegHoldingBuf[15] = role_5  option 0
	s154_contact_8, // usRegHoldingBuf[16] = role_6  option 1
	s154_contact_9, // usRegHoldingBuf[17] = error_0 swithc error
	s154_contact_10, // usRegHoldingBuf[18] = error_1 fuel error
	s154_contact_11, // usRegHoldingBuf[19] = error_2 oxygen error
	s154_RE_Modbus_relay_status, // usRegHoldingBuf[20}
	s154_RE_Modbus_config_0, // usRegHoldingBuf[21] = GA_0;
	s154_RE_Modbus_config_1, // usRegHoldingBuf[22] = GA_1;
	s154_RE_Modbus_config_2, // usRegHoldingBuf[23] = GA_2;
	s154_RE_Modbus_config_3, // usRegHoldingBuf[24] = GA_3;
	s154_master_data_income, // Modbustan bilgi gönderirse == 1 olması
} eSlave154Enum;

typedef enum { // eSlave155Enum
	s155_none,
	s155_counter,  	// PROG_Modbus_counter 1
	s155_R_voltage, // usRegHoldingBuf[2] = (uint16_t) (up_heats[0] * 100);
	s155_S_voltage, // usRegHoldingBuf[3] = (uint16_t) (up_heats[1] * 100);
	s155_T_voltage, // usRegHoldingBuf[4] = (uint16_t) (up_heats[2] * 100);
	s155_R_amper, // usRegHoldingBuf[5] = (uint16_t) up_heats[3];
	s155_S_amper, // usRegHoldingBuf[6] = (uint16_t) up_level;
	s155_T_amper, // usRegHoldingBuf[7] = (uint16_t) (flat_heats[0] * 100);
	s155_phase_type,
	s155_RE_Modbus_relay_status, // usRegHoldingBuf[8}
	s155_RE_Modbus_config_0, // usRegHoldingBuf[9] = GA_0;
	s155_RE_Modbus_config_1, // usRegHoldingBuf[10] = GA_1;
	s155_RE_Modbus_config_2, // usRegHoldingBuf[11] = GA_2;
	s155_RE_Modbus_config_3, // usRegHoldingBuf[12] = GA_3;
	s155_master_data_income, // Modbustan bilgi gönderirse == 1 olması
} eSlave155Enum;

typedef enum { // s100_enums
	s100_main_hardware = 0,  // Master
	s150_burner_hardware = 1, // Master-Slave
	s151_outside_hardware = 2, // Slave
	s152_pipes_hardware = 3, // Slave
	s153_building_hardware = 4, // Slave
	s154_burner_motor_hardware = 5, // Slave
	s155_circulation_motor_hardware = 6, // Slave
	s100_count,
} es100_hardware_enums;

typedef struct {
	uint8_t eslave_count;
	eSlave150Enum estruct_150;
	eSlave151Enum estruct_151;
	eSlave152Enum estruct_152;
	eSlave153Enum estruct_153;
	eSlave154Enum estruct_154;
	eSlave155Enum estruct_155;
} ehardware;

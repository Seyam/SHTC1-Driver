/*
 * Copyright (c) 2014, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * \brief Sensirion SHTC1 driver implementation
 *
 * This module provides access to the SHTC1 functionality over the ASF I2C
 * interface. It allows measurements in normal and clock stretching
 * mode as well as executing a soft reset command.
 */

 #include <asf.h>
 #include "shtc1.h"
 #include "i2c_master.h"
 
 /* all measurement commands return T (CRC) RH (CRC) */
 const uint8_t CMD_MEASURE_LPM_CS[]  = { 0x64, 0x58 };
 const uint8_t CMD_MEASURE_LPM[]     = { 0x60, 0x9c };
 const uint8_t CMD_MEASURE_HPM_CS[]  = { 0x7c, 0xa2 };
 const uint8_t CMD_MEASURE_HPM[]     = { 0x78, 0x66 };
 const uint8_t CMD_SOFT_RESET[]      = { 0x80, 0x5d };
 const uint8_t CMD_READ_ID_REG[]     = { 0xef, 0xc8 };
 const size_t COMMAND_SIZE = sizeof(CMD_MEASURE_LPM);
 const uint16_t SHTC1_ADDRESS    = 0x70;
 
 const uint8_t ID_REG_CONTENT    = 0x07;
 const uint8_t ID_REG_MASK       = 0x1f;
 const uint8_t CRC_POLYNOMIAL    = 0x31;
 const uint8_t CRC_INIT          = 0xff;
 
 static bool shtc1_check_crc(uint8_t *data, uint8_t data_length, uint8_t checksum)
 {
     uint8_t crc = CRC_INIT;
     uint8_t current_byte;
 
     /* calculates 8-Bit checksum with given polynomial */
     for (current_byte = 0; current_byte < data_length; ++current_byte)
     {
         crc ^= (data[current_byte]);
         for (uint8_t bit = 8; bit > 0; --bit)
         {
             if (crc & 0x80)
                 crc = (crc << 1) ^ CRC_POLYNOMIAL;
             else
                 crc = (crc << 1);
         }
     }
     return crc == checksum;
 }
 
 enum status_code shtc1_read_async_result(struct i2c_master_module *i2c_master_instance_ptr,
         int *temp, int *rh)
 {
     uint8_t data[6];
     struct i2c_master_packet packet = {
             .address = SHTC1_ADDRESS,
             .data_length = sizeof(data),
             .data = data,
             .ten_bit_address = false,
             .high_speed = false,
     };
     enum status_code ret = i2c_master_read_packet_wait(i2c_master_instance_ptr, &packet);
     //print_to_terminal("ret in  shtc1_read_async_result = 0x%x\n", ret);
     
     if (ret)
         return ret;
     if (!shtc1_check_crc(data, 2, data[2]) || !shtc1_check_crc(data + 3, 2, data[5]))
         return STATUS_ERR_BAD_DATA;
 
     /**
      * formulas for conversion of the sensor signals, optimized for fixed point
      * algebra:
      * T = 175 * S_T / 2^16 - 45
      * RH = 100 * S_RH / 2^16
      */
     *temp = (data[1] & 0xff) | (data[0] << 8);
     *rh = (data[4] & 0xff) | (data[3] << 8);
     *temp = ((21875 * *temp) >> 13) - 45000;
     *rh = ((12500 * *rh) >> 13);
 
     return STATUS_OK;
 }
 
 static enum status_code shtc1_read_sync(struct i2c_master_module *i2c_master_instance_ptr,
         const uint8_t command[], int *temp, int *rh)
 {
     enum status_code ret;
     struct i2c_master_packet packet = {
             .address = SHTC1_ADDRESS,
             .data_length = COMMAND_SIZE,
             .data = (uint8_t *)command,
             .ten_bit_address = false,
             .high_speed = false,
     };
     ret = i2c_master_write_packet_wait_no_stop(i2c_master_instance_ptr, &packet);
     //print_to_terminal("ret in  shtc1_read_sync = 0x%x\n", ret);
     delay_ms(50);
     
     if (ret)
         return ret;
     return shtc1_read_async_result(i2c_master_instance_ptr, temp, rh);
 }
 
 enum status_code shtc1_read_lpm_sync(struct i2c_master_module *i2c_master_instance_ptr,
         int *temp, int *rh)
 {
     return shtc1_read_sync(i2c_master_instance_ptr, CMD_MEASURE_LPM_CS, temp, rh);
 }
 
 enum status_code shtc1_read_hpm_sync(struct i2c_master_module *i2c_master_instance_ptr,
         int *temp, int *rh)
 {
     return shtc1_read_sync(i2c_master_instance_ptr, CMD_MEASURE_HPM_CS, temp, rh);
 }
 
 enum status_code shtc1_read_lpm_async(struct i2c_master_module *i2c_master_instance_ptr)
 {
     struct i2c_master_packet packet = {
             .address = SHTC1_ADDRESS,
             .data_length = COMMAND_SIZE,
             .data = (uint8_t *)CMD_MEASURE_LPM,
             .ten_bit_address = false,
             .high_speed = false,
     };
     return i2c_master_write_packet_wait_no_stop(i2c_master_instance_ptr, &packet);
 }
 
 enum status_code shtc1_read_hpm_async(struct i2c_master_module *i2c_master_instance_ptr)
 {
     struct i2c_master_packet packet = {
             .address = SHTC1_ADDRESS,
             .data_length = COMMAND_SIZE,
             .data = (uint8_t *)CMD_MEASURE_HPM,
             .ten_bit_address = false,
             .high_speed = false,
     };
     return i2c_master_write_packet_wait_no_stop(i2c_master_instance_ptr, &packet);
 }
 
 enum status_code shtc1_reset(struct i2c_master_module *i2c_master_instance_ptr)
 {
     struct i2c_master_packet packet = {
             .address = SHTC1_ADDRESS,
             .data_length = COMMAND_SIZE,
             .data = (uint8_t *)CMD_SOFT_RESET,
             .ten_bit_address = false,
             .high_speed = false,
     };
     return i2c_master_write_packet_wait(i2c_master_instance_ptr, &packet);
 }
 
 bool shtc1_probe(struct i2c_master_module *i2c_master_instance_ptr)
 {
     uint8_t data[3];
     struct i2c_master_packet packet = {
             .address = SHTC1_ADDRESS,
             .data_length = COMMAND_SIZE,
             .data = (uint8_t *)CMD_READ_ID_REG,
             .ten_bit_address = false,
             .high_speed = false,
     };
     
     i2c_master_write_packet_wait_no_stop(i2c_master_instance_ptr, &packet);
     packet.data_length = sizeof(data);
     packet.data = data;
     
     delay_ms(50);
     
     enum status_code ret = i2c_master_read_packet_wait(i2c_master_instance_ptr, &packet);
 
     if (ret)
         return false;
 
     if (!shtc1_check_crc(data, 2, data[2]))
         return false;
 
     return (data[1] & ID_REG_MASK) == ID_REG_CONTENT;
 }
/*
 *  Copyright (C) 2013 Sensirion AG, Switzerland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file
 *
 * \brief Sensirion SHTC1 driver interface
 *
 * This module provides access to the SHTC1 functionality over the ASF I2C
 * interface. It allows measurements in normal and clock stretching
 * mode as well as executing a soft reset command.
 */

 #ifndef SHTC1_H_
 #define SHTC1_H_
 
 #include "status_codes.h"
 
 /**
  * Performs a measurement in high precision mode using clock stretching. This
  * command blocks the TWI bus until the sensor returns the measured values.
  * A measurement takes about 10.8 ms to complete.
  * Temperature is returned in 1/1000 C and humidity in 1/1000 percent.
  *
  * @param i2c_master_instance the i2c master instance pointer
  * @param temp the address for the result of the temperature measurement
  * @param rh   the address for the result of the relative humidity measurement
  * @return     STATUS_OK if the command was successful, else an error code.
  */
 enum status_code shtc1_read_hpm_sync(struct i2c_master_module *i2c_master_instance_ptr,
         int *temp, int *rh);
 
 /**
  * Performs a measurement in low power mode using clock stretching. This
  * command blocks the TWI bus until the sensor returns the measured values.
  * A measurement takes about 0.7 ms to complete.
  * Temperature is returned in 1/1000 C and humidity in 1/1000 percent.
  *
  * @param i2c_master_instance the i2c master instance pointer
  * @param temp the address for the result of the temperature measurement
  * @param rh   the address for the result of the relative humidity measurement
  * @return     STATUS_OK if the command was successful, else an error code.
  */
 enum status_code shtc1_read_lpm_sync(struct i2c_master_module *i2c_master_instance_ptr,
         int *temp, int *rh);
 
 /**
  * Starts a measurement in high precision mode and returns immediately. Use
  * shtc1_read() to read out the measured value after the measurement has
  * completed.
  * A measurement takes about 10.8 ms to complete.
  *
  * @param i2c_master_instance the i2c master instance pointer
  * @return     STATUS_OK if the command was successful, else an error code.
  */
 enum status_code shtc1_read_hpm_async(struct i2c_master_module *i2c_master_instance_ptr);
 
 /**
  * Starts a measurement in low power mode and returns immediately. Use
  * shtc1_read() to read out the measured value after the measurement has
  * completed.
  * A measurement takes about 0.7 ms to complete.
  *
  * @param i2c_master_instance the i2c master instance pointer
  * @return     STATUS_OK if the command was successful, else an error code.
  */
 enum status_code shtc1_read_lpm_async(struct i2c_master_module *i2c_master_instance_ptr);
 
 /**
  * Read out the results of a measurement previously started with shtc1_start_lpm()
  " or shtc1_start_hpm().
  * Temperature is returned in 1/1000 C and humidity in 1/1000 percent.
  *
  * @param i2c_master_instance the i2c master instance pointer
  * @param temp the address for the result of the temperature measurement
  * @param rh   the address for the result of the relative humidity measurement
  * @return     STATUS_OK if the command was successful, else an error code.
  */
 enum status_code shtc1_read_async_result(struct i2c_master_module *i2c_master_instance_ptr,
         int *temp, int *rh);
 
 /**
  * @brief Sends a soft reset command to the sensor.
  * The soft reset mechanism forces the sensor into a well defined state without
  * removing the power supply. All internal state machines are reset and
  * calibration data is reloaded from memory.
  *
  * @param i2c_master_instance the i2c master instance pointer
  * @return     STATUS_OK if the command was successful, else an error code.
  */
 enum status_code shtc1_reset(struct i2c_master_module *i2c_master_instance_ptr);
 
 /**
  * @brief Detects if a sensor is connected by reading out the ID register.
  * If the sensor does not answer or if the answer is not the expected value,
  * the test fails.
  *
  * @param i2c_master_instance the i2c master instance pointer
  * @return true if a sensor was detected
  */
 bool shtc1_probe(struct i2c_master_module *i2c_master_instance_ptr);
 
 #endif /* SHTC1_H_ */
/*
 * i2c.c
 *
 *  Created on: Sep 16, 2021
 *      Author: salon
 */
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "src/i2c.h"
#include "app.h"
#include "pulse_oximeter.h"
#include "src/ble.h"

#define SI7021_DEVICE_ADDR 0x40
#define APDS9960_DEVICE_ADDR 0x39
#define PULSE_OXIMETER 0x55

uint8_t cmd_data;
uint8_t read_data[2];
I2C_TransferSeq_TypeDef transfer_seq;

/******************************************************PULSE OXIMETER VARIABLES**************************************************/

  uint8_t pulse_data[8];
  uint16_t heartRate[10]; // LSB = 0.1bpm
  uint8_t  confidence; // 0-100% LSB = 1%
  uint16_t oxygen[10]; // 0-100% LSB = 1%
  uint8_t  status=0; // 0: Success, 1: Not Ready, 2: Object Detectected, 3: Finger Detected
  uint8_t send_oximeter_data[2];
  int i=0;
  uint8_t max_heart_rate=0;
  uint8_t max_oxygen=0;


void i2c_init() {

  //struct for i2c initialization
  I2CSPM_Init_TypeDef I2C_Config = {
      .port = I2C0,
      .sclPort = gpioPortC,
      .sclPin =  10,
      .sdaPort = gpioPortC,
      .sdaPin = 11,
      .portLocationScl = 14,
      .portLocationSda = 16,
      .i2cRefFreq = 0,
      .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
      .i2cClhr = i2cClockHLRStandard
  };

  //initialize i2c peripheral
  I2CSPM_Init(&I2C_Config);

}

//function to perform write command operation on slave
void write_cmd() {

  I2C_TransferReturn_TypeDef transferStatus;

  i2c_init();

  //structure to write command from master to slave
  cmd_data = 0xF3;
  transfer_seq.addr = SI7021_DEVICE_ADDR << 1;
  transfer_seq.flags = I2C_FLAG_WRITE;
  transfer_seq.buf[0].data = &cmd_data;
  transfer_seq.buf[0].len = sizeof(cmd_data);

  //enable I2C interrupt
  NVIC_EnableIRQ(I2C0_IRQn);

  //initialize I2C transfer
  transferStatus = I2C_TransferInit(I2C0, &transfer_seq);

  //check transfer function return status
  if(transferStatus < 0) {
      LOG_ERROR("I2C_TransferInit status %d write: failed\n\r", (uint32_t)transferStatus);
  }
}

uint32_t write_read(uint8_t reg, uint8_t *data) {
  uint8_t comd_data[1];
  I2C_TransferReturn_TypeDef transferStatus;

  i2c_init();

  comd_data[0] = reg;

  //structure to write command from master to slave
    transfer_seq.addr = APDS9960_DEVICE_ADDR<<1;
    transfer_seq.flags = I2C_FLAG_WRITE_READ;
    transfer_seq.buf[0].data = comd_data;
    transfer_seq.buf[0].len = 1;
    transfer_seq.buf[1].data = data;
    transfer_seq.buf[1].len = 1;

    //enable I2C interrupt
     // NVIC_EnableIRQ(I2C0_IRQn);

      //initialize I2C transfer
      transferStatus = I2CSPM_Transfer(I2C0, &transfer_seq);

      //check transfer function return status
      if(transferStatus != i2cTransferDone) {
          LOG_ERROR("I2C_TransferInit status %d write: failed\n\r", (uint32_t)transferStatus);
          *data = 0xff;
          return (uint32_t)transferStatus;
      }

      return (uint32_t)1;
}

uint32_t write_write(uint8_t reg, uint8_t data) {
  uint8_t comd_data[2];
  uint8_t no_data[1];
  I2C_TransferReturn_TypeDef transferStatus;

  i2c_init();

  comd_data[0] = reg;
  comd_data[1] = data;

  //structure to write command from master to slave
    transfer_seq.addr = APDS9960_DEVICE_ADDR << 1;
    transfer_seq.flags = I2C_FLAG_WRITE;
    transfer_seq.buf[0].data = comd_data;
    transfer_seq.buf[0].len = 2;
    transfer_seq.buf[1].data = no_data;
    transfer_seq.buf[1].len  = 0;

    //enable I2C interrupt
     // NVIC_EnableIRQ(I2C0_IRQn);

      //initialize I2C transfer
      transferStatus = I2CSPM_Transfer(I2C0, &transfer_seq);

      //check transfer function return status
      if(transferStatus != i2cTransferDone) {
          LOG_ERROR("I2C_TransferInit status %d write: failed\n\r", (uint32_t)transferStatus);
          return (uint32_t)transferStatus;
      }

      return (uint32_t)1;
}

int ReadDataBlock(uint8_t reg, uint8_t *data, uint8_t length)
{
  I2C_TransferReturn_TypeDef transferStatus;
  uint8_t cmd_data[1];

  i2c_init();

  transfer_seq.addr = APDS9960_DEVICE_ADDR << 1;
  transfer_seq.flags = I2C_FLAG_WRITE_READ;
  /* Select register to start reading from */
  cmd_data[0] = reg;
  transfer_seq.buf[0].data = cmd_data;
  transfer_seq.buf[0].len  = 1;
  /* Select length of data to be read */
  transfer_seq.buf[1].data = data;
  transfer_seq.buf[1].len  = length;

 // NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2CSPM_Transfer(I2C0, &transfer_seq);
  if (transferStatus != i2cTransferDone)
  {
    return (int)transferStatus;
  }
  return (int)length;
}

//function to perform read operation from slave
void read_cmd() {

  I2C_TransferReturn_TypeDef transferStatus;

  i2c_init();

  //structure to read temperature measurement from slave
  transfer_seq.addr = SI7021_DEVICE_ADDR << 1;
  transfer_seq.flags = I2C_FLAG_READ;
  transfer_seq.buf[0].data = &read_data[0];
  transfer_seq.buf[0].len = sizeof(read_data);

  //enable I2C interrupt
  NVIC_EnableIRQ(I2C0_IRQn);

  //initialize I2C transfer
  transferStatus = I2C_TransferInit(I2C0, &transfer_seq);

  //check transfer function return status
  if(transferStatus < 0) {
      LOG_ERROR("I2C_TransferInit status %d read: failed\n\r", (uint32_t)transferStatus);
  }
}


void I2C_pulse_write_polled(uint8_t* command, int array_len){
  I2C_TransferReturn_TypeDef transferStatus;    // used to store the transfer status

  i2c_init();
                                   //Provide command to perform measurement
  transfer_seq.addr = PULSE_OXIMETER << 1;        //shift device address left
  transfer_seq.flags = I2C_FLAG_WRITE;                //write command
  transfer_seq.buf[0].data = command;               //pointer to data to write
  transfer_seq.buf[0].len = array_len;

  //enable I2C interrupt
  //NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2CSPM_Transfer (I2C0, &transfer_seq); //check the status of this operation

  if (transferStatus < 0) {
  LOG_ERROR("I2C_TransferInit() Write error = %d", transferStatus);
  }

}


void I2C_pulse_read_polled(){

  I2C_TransferReturn_TypeDef transferStatus;    // used to store the transfer status

  i2c_init();

  //set the transfer sequence for read
  transfer_seq.addr = PULSE_OXIMETER << 1;        //shift device address left
  transfer_seq.flags = I2C_FLAG_READ;                 //read command
  transfer_seq.buf[0].data = pulse_data;               //pointer to data to write
  transfer_seq.buf[0].len = sizeof(pulse_data);

  //enable I2C interrupt
 // NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2CSPM_Transfer (I2C0, &transfer_seq);//check the status of this operation

  if (transferStatus < 0) {
  LOG_ERROR("I2C_TransferInit() Write error = %d", transferStatus);
  }

}


//function to convert data read from sensor into temperature value in Celcius
float convertTemp() {

  float tempCelcius;
  uint16_t read_temp;
  read_temp = (read_data[0] << 8);
  read_temp += read_data[1];
  tempCelcius = (175.72*(read_temp));
  tempCelcius /= 65536;
  tempCelcius -= 46.85;
  //displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%f", tempCelcius);
  return tempCelcius;
}

//***********************************************OXIMETER_DATA_EXTRACT*************************************************//

void read_return_check(){
  if(pulse_data[0] == 0x00){
      //LOG_INFO("Return value check successfull\n\r");
  }
  else{
      LOG_ERROR("Pulse oximeter sensor initialization error!!\n\r");
  }
}

#if DEVICE_IS_BLE_SERVER

int extract_data(){

  ble_data_struct_t *bleData = getBleDataPtr();
  status = pulse_data[6];

    if(status == 3){

        displayPrintf(DISPLAY_ROW_ACTION, "Do not lift finger!");
        displayPrintf(DISPLAY_ROW_TEMPVALUE, "Loading: %d", (10 - i));

        heartRate[i] = ((uint16_t)(pulse_data[1])) << 8;
        heartRate[i] |= pulse_data[2];
        heartRate[i] = heartRate[i]/10;

        confidence = pulse_data[3];

        oxygen[i] = ((uint16_t)(pulse_data[4])) << 8;
        oxygen[i] |= pulse_data[5];
        oxygen[i] = oxygen[i]/10;

        LOG_INFO("heart_rate = %d\n\r", heartRate[i]);
        LOG_INFO("confidence = %d\n\r", confidence);
        LOG_INFO("oxygen level = %d\n\r", oxygen[i]);
        LOG_INFO("status = %d\n\n\r", status);

        i++;
    }
    else{
        LOG_INFO("Please place the finger correctly!!\n\n\r");

        displayPrintf(DISPLAY_ROW_ACTION,"Place finger!");
    }

    if(i==10){
        max_heart_rate= heartRate[0];
        max_oxygen= oxygen[0];

        for(int j=0;j<i;j++){
           if(max_heart_rate < heartRate[j]){
               max_heart_rate = (uint8_t)heartRate[j];
           }

           if(max_oxygen < oxygen[j]){
               max_oxygen = (uint8_t)oxygen[j];
           }
        }

        LOG_INFO("***************************************FINAL VALUES**********************************************\n\r");
        LOG_INFO("heart_rate = %d\n\r", max_heart_rate);
        LOG_INFO("oxygen level = %d\n\r", max_oxygen);

        displayPrintf(DISPLAY_ROW_8, "");

        if(bleData->gesture_value==0x01){
            send_oximeter_data[0] = max_oxygen;
            send_oximeter_data[1] = 0;
            displayPrintf(DISPLAY_ROW_TEMPVALUE, "Oxygen level: %d",max_oxygen);
        }
        if(bleData->gesture_value==0x02){
            send_oximeter_data[0] = max_heart_rate;
            send_oximeter_data[1] = 0;
            displayPrintf(DISPLAY_ROW_TEMPVALUE, "Heart rate: %d",max_heart_rate);
        }

        ble_SendOximeterState(send_oximeter_data);

    }

    if(i==10){
        i=0;
      return 1;
    }
    else{
      return 0;
    }

}

#endif

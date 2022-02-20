/**
 * @file    SparkFun_APDS-9960.cpp
 * @brief   Library for the SparkFun APDS-9960 breakout board
 * @author  Shawn Hymel (SparkFun Electronics)
 *
 * @copyright This code is public domain but you buy me a beer if you use
 * this and we meet someday (Beerware license).
 *
 * This library interfaces the Avago APDS-9960 to Arduino over I2C. The library
 * relies on the Arduino Wire (I2C) library. to use the library, instantiate an
 * APDS9960 object, call init(), and call the appropriate functions.
 *
 * APDS-9960 current draw tests (default parameters):
 *   Off:                   1mA
 *   Waiting for gesture:   14mA
 *   Gesture in progress:   35mA
 */

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "app.h"
#include "src/SparkFun_APDS9960.h"

int gesture_ud_delta;
int gesture_lr_delta;
int gesture_ud_count;
int gesture_lr_count;
int gesture_near_count;
int gesture_far_count;
int gesture_state;
int gesture_motion;

bool interrupts = true;

gesture_data_type gestureData;

gesture_data_type * getGestureDataPtr() {

  return (&gestureData);
}

/**
 * @brief Configures I2C communications and initializes registers to defaults
 *
 * @return True if initialized successfully. False otherwise.
 */
bool SparkFun_APDS9960_init()
{
  uint8_t id=0;

  /* Initialize I2C */
  //Wire.begin();

  /* Read ID register and check against known values for APDS-9960 */
  //if( !wireReadDataByte(APDS9960_ID, id) ) {
  if(write_read(APDS9960_ID, &id) != 1) {
      return false;
  }

  if( !(id == APDS9960_ID_1 || id == APDS9960_ID_2) ) {
      return false;
  }
  else {
      //LOG_INFO("First write_read successful\n\r");
  }

  /* Set ENABLE register to 0 (disable all features) */
  if( !setMode(ALL, OFF) ) {
      return false;
  }

  /* Set default values for gesture sense registers */
  if( !setGestureEnterThresh(DEFAULT_GPENTH) ) {
      return false;
  }
  if( !setGestureExitThresh(DEFAULT_GEXTH) ) {
      return false;
  }

  if(write_write(APDS9960_GCONF1, DEFAULT_GCONF1) != 1) {
        return false;
    }


  if( !setGestureGain(DEFAULT_GGAIN) ) {
      return false;
  }
  if( !setGestureLEDDrive(DEFAULT_GLDRIVE) ) {
      return false;
  }
  if( !setGestureWaitTime(DEFAULT_GWTIME) ) {
      return false;
  }

  if(write_write(APDS9960_GOFFSET_U, DEFAULT_GOFFSET) != 1) {
      return false;
  }

  if(write_write(APDS9960_GOFFSET_D, DEFAULT_GOFFSET) != 1) {
      return false;
  }

  if(write_write(APDS9960_GOFFSET_L, DEFAULT_GOFFSET) != 1) {
      return false;
  }

  if(write_write(APDS9960_GOFFSET_R, DEFAULT_GOFFSET) != 1) {
      return false;
  }

  //if( !wireWriteDataByte(APDS9960_GPULSE, DEFAULT_GPULSE) ) {
  if(write_write(APDS9960_GPULSE, DEFAULT_GPULSE) != 1) {
      return false;
  }

  //if( !wireWriteDataByte(APDS9960_GCONF3, DEFAULT_GCONF3) ) {
  if(write_write(APDS9960_GCONF3, DEFAULT_GCONF3) != 1) {
      return false;
  }

  if( !setGestureIntEnable(DEFAULT_GIEN) ) {
      return false;
  }

#if 0
/* Gesture config register dump */
uint8_t reg;
uint8_t val;

for(reg = 0x80; reg <= 0xAF; reg++) {
    if( (reg != 0x82) && \
        (reg != 0x8A) && \
        (reg != 0x91) && \
        (reg != 0xA8) && \
        (reg != 0xAC) && \
        (reg != 0xAD) )
      {
        wireReadDataByte(reg, val);
        Serial.print(reg, HEX);
        Serial.print(": 0x");
        Serial.println(val, HEX);
      }
}

for(reg = 0xE4; reg <= 0xE7; reg++) {
    wireReadDataByte(reg, val);
    Serial.print(reg, HEX);
    Serial.print(": 0x");
    Serial.println(val, HEX);
}
#endif

return true;
}

/**
 * @brief Reads and returns the contents of the ENABLE register
 *
 * @return Contents of the ENABLE register. 0xFF if error.
 */
uint8_t getMode()
{
  uint8_t enable_value=0;

  /* Read current ENABLE register */
  //if( !wireReadDataByte(APDS9960_ENABLE, enable_value) ) {
  if(write_read(APDS9960_ENABLE, &enable_value) != 1) {
      return 255;
  }

  return enable_value;
}

/*******************************************************************************
 * Public methods for controlling the APDS-9960
 ******************************************************************************/

/**
 * @brief Enables or disables a feature in the APDS-9960
 *
 * @param[in] mode which feature to enable
 * @param[in] enable ON (1) or OFF (0)
 * @return True if operation success. False otherwise.
 */
bool setMode(uint8_t mode, uint8_t enable)
{
  uint8_t reg_val;

  /* Read current ENABLE register */
  reg_val = getMode();
  if( reg_val == ERROR ) {
      return false;
  }

  /* Change bit(s) in ENABLE register */
  enable = enable & 0x01;
  if( (mode>=0) & (mode<=6) ) {
      if (enable) {
          reg_val |= (1 << mode);
      } else {
          reg_val &= ~(1 << mode);
      }
  } else if( mode == ALL ) {
      if (enable) {
          reg_val = 0x7F;
      } else {
          reg_val = 0x00;
      }
  }

  /* Write value back to ENABLE register */
  //if( !wireWriteDataByte(APDS9960_ENABLE, reg_val) ) {
  if(write_write(APDS9960_ENABLE, reg_val) != 1) {
      return false;
  }

  return true;
}

/**
 * @brief Starts the gesture recognition engine on the APDS-9960
 *
 * @param[in] interrupts true to enable hardware external interrupt on gesture
 * @return True if engine enabled correctly. False on error.
 */
bool enableGestureSensor(bool interrupts)
{

  /* Enable gesture mode
       Set ENABLE to 0 (power off)
       Set WTIME to 0xFF
       Set AUX to LED_BOOST_300
       Enable PON, WEN, PEN, GEN in ENABLE
   */
  resetGestureParameters();
  //if( !wireWriteDataByte(APDS9960_WTIME, 0xFF) ) {
  if(write_write(APDS9960_WTIME, 0xFF)!= 1) {
      return false;
  }

  //if( !wireWriteDataByte(APDS9960_PPULSE, DEFAULT_GESTURE_PPULSE) ) {
  if(write_write(APDS9960_PPULSE, DEFAULT_GESTURE_PPULSE) != 1) {
      return false;
  }

  if( !setLEDBoost(LED_BOOST_300) ) {
      return false;
  }
  if( interrupts ) {
      if( !setGestureIntEnable(1) ) {
          return false;
      }
  } else {
      if( !setGestureIntEnable(0) ) {
          return false;
      }
  }
  if( !setGestureMode(1) ) {
      return false;
  }
  if( !enablePower() ){
      return false;
  }
  if( !setMode(WAIT, 1) ) {
      return false;
  }
  if( !setMode(PROXIMITY, 1) ) {
      return false;
  }
  if( !setMode(GESTURE, 1) ) {
      return false;
  }

  return true;
}

/**
 * @brief Ends the gesture recognition engine on the APDS-9960
 *
 * @return True if engine disabled correctly. False on error.
 */
bool disableGestureSensor()
{
  resetGestureParameters();
  if( !setGestureIntEnable(0) ) {
      return false;
  }
  if( !setGestureMode(0) ) {
      return false;
  }
  if( !setMode(GESTURE, 0) ) {
      return false;
  }

  return true;
}

/**
 * @brief Determines if there is a gesture available for reading
 *
 * @return True if gesture available. False otherwise.
 */
bool isGestureAvailable()
{
  uint8_t val=0;

  /* Read value from GSTATUS register */
  //if( !wireReadDataByte(APDS9960_GSTATUS, val) ) {
  if(write_read(APDS9960_GSTATUS, &val) != 1) {
      return false;
  }

  /* Shift and mask out GVALID bit */
  val &= APDS9960_GVALID;

  /* Return true/false based on GVALID bit */
  if( val == 1) {
      return true;
  } else {
      return false;
  }
}

/**
 * @brief Processes a gesture event and returns best guessed gesture
 *
 * @return Number corresponding to gesture. -1 on error.
 */
int readGesture()
{
  uint8_t fifo_level = 0;
  uint8_t bytes_read = 0;
  uint8_t fifo_data[128];
  uint8_t gstatus=0;
  int motion;
  int i;
  gesture_data_type *gesture_data = getGestureDataPtr();

  /* Make sure that power and gesture is on and data is valid */
  if( !isGestureAvailable() || !(getMode() & 0b01000001) ) {
      return DIR_NONE;
  }

  /* Keep looping as long as gesture data is valid */
  while(1) {

      /* Wait some time to collect next batch of FIFO data */
      //delay(FIFO_PAUSE_TIME);
      timerWaitUs_polled(FIFO_PAUSE_TIME);

      /* Get the contents of the STATUS register. Is data still valid? */
      //if( !wireReadDataByte(APDS9960_GSTATUS, gstatus) ) {
      if(write_read(APDS9960_GSTATUS, &gstatus) != 1) {
          return false;
      }

      /* If we have valid data, read in FIFO */
      if( (gstatus & APDS9960_GVALID) == APDS9960_GVALID ) {

          /* Read the current FIFO level */
          //if( !wireReadDataByte(APDS9960_GFLVL, fifo_level) ) {
          if(write_read(APDS9960_GFLVL, &fifo_level) != 1) {
              return false;
          }

#if DEBUG
          Serial.print("FIFO Level: ");
          Serial.println(fifo_level);
#endif

          /* If there's stuff in the FIFO, read it into our data block */
          if( fifo_level > 0) {
              bytes_read = ReadDataBlock(  APDS9960_GFIFO_U,
                                               (uint8_t*)fifo_data,
                                               (fifo_level * 4) );
              if( bytes_read == -1 ) {
                  return ERROR;
              }
#if DEBUG
              Serial.print("FIFO Dump: ");
              for ( i = 0; i < bytes_read; i++ ) {
                  Serial.print(fifo_data[i]);
                  Serial.print(" ");
              }
              Serial.println();
#endif

              /* If at least 1 set of data, sort the data into U/D/L/R */
              if( bytes_read >= 4 ) {
                  for( i = 0; i < bytes_read; i += 4 ) {
                      gesture_data->u_data[gesture_data->index] = \
                          fifo_data[i + 0];
                      gesture_data->d_data[gesture_data->index] = \
                          fifo_data[i + 1];
                      gesture_data->l_data[gesture_data->index] = \
                          fifo_data[i + 2];
                      gesture_data->r_data[gesture_data->index] = \
                          fifo_data[i + 3];
                      gesture_data->index++;
                      gesture_data->total_gestures++;
                  }

#if DEBUG
Serial.print("Up Data: ");
for ( i = 0; i < gesture_data_.total_gestures; i++ ) {
    Serial.print(gesture_data_.u_data[i]);
    Serial.print(" ");
}
Serial.println();
#endif

/* Filter and process gesture data. Decode near/far state */
if( processGestureData() ) {
    if( decodeGesture() ) {
        //***TODO: U-Turn Gestures
#if DEBUG
        //Serial.println(gesture_motion_);
#endif
    }
}

/* Reset data */
gesture_data->index = 0;
gesture_data->total_gestures = 0;
              }
          }
      } else {

          /* Determine best guessed gesture and clean up */
          timerWaitUs_polled(FIFO_PAUSE_TIME);
          decodeGesture();
          motion = gesture_motion;
#if DEBUG
          Serial.print("END: ");
          Serial.println(gesture_motion_);
#endif
          resetGestureParameters();
          return motion;
      }
  }
}

/**
 * Turn the APDS-9960 on
 *
 * @return True if operation successful. False otherwise.
 */
bool enablePower()
{
  if( !setMode(POWER, 1) ) {
      return false;
  }

  return true;
}

/**
 * Turn the APDS-9960 off
 *
 * @return True if operation successful. False otherwise.
 */
bool disablePower()
{
  if( !setMode(POWER, 0) ) {
      return false;
  }

  return true;
}

/*******************************************************************************
 * High-level gesture controls
 ******************************************************************************/

/**
 * @brief Resets all the parameters in the gesture data member
 */
void resetGestureParameters()
{
  gesture_data_type *gesture_data = getGestureDataPtr();

  gesture_data->index = 0;
  gesture_data->total_gestures = 0;

  gesture_ud_delta = 0;
  gesture_lr_delta = 0;

  gesture_ud_count = 0;
  gesture_lr_count = 0;

  gesture_near_count = 0;
  gesture_far_count = 0;

  gesture_state = 0;
  gesture_motion = DIR_NONE;
}

/**
 * @brief Processes the raw gesture data to determine swipe direction
 *
 * @return True if near or far state seen. False otherwise.
 */
bool processGestureData()
{
  uint8_t u_first = 0;
  uint8_t d_first = 0;
  uint8_t l_first = 0;
  uint8_t r_first = 0;
  uint8_t u_last = 0;
  uint8_t d_last = 0;
  uint8_t l_last = 0;
  uint8_t r_last = 0;
  int ud_ratio_first;
  int lr_ratio_first;
  int ud_ratio_last;
  int lr_ratio_last;
  int ud_delta;
  int lr_delta;
  int i;
  gesture_data_type *gesture_data = getGestureDataPtr();

  /* If we have less than 4 total gestures, that's not enough */
  if( gesture_data->total_gestures <= 4 ) {
      return false;
  }

  /* Check to make sure our data isn't out of bounds */
  if( (gesture_data->total_gestures <= 32) && \
      (gesture_data->total_gestures > 0) ) {

      /* Find the first value in U/D/L/R above the threshold */
      for( i = 0; i < gesture_data->total_gestures; i++ ) {
          if( (gesture_data->u_data[i] > GESTURE_THRESHOLD_OUT) &&
              (gesture_data->d_data[i] > GESTURE_THRESHOLD_OUT) &&
              (gesture_data->l_data[i] > GESTURE_THRESHOLD_OUT) &&
              (gesture_data->r_data[i] > GESTURE_THRESHOLD_OUT) ) {

              u_first = gesture_data->u_data[i];
              d_first = gesture_data->d_data[i];
              l_first = gesture_data->l_data[i];
              r_first = gesture_data->r_data[i];
              break;
          }
      }

      /* If one of the _first values is 0, then there is no good data */
      if( (u_first == 0) || (d_first == 0) || \
          (l_first == 0) || (r_first == 0) ) {

          return false;
      }
      /* Find the last value in U/D/L/R above the threshold */
      for( i = gesture_data->total_gestures - 1; i >= 0; i-- ) {
#if DEBUG
          Serial.print(F("Finding last: "));
          Serial.print(F("U:"));
          Serial.print(gesture_data_.u_data[i]);
          Serial.print(F(" D:"));
          Serial.print(gesture_data_.d_data[i]);
          Serial.print(F(" L:"));
          Serial.print(gesture_data_.l_data[i]);
          Serial.print(F(" R:"));
          Serial.println(gesture_data_.r_data[i]);
#endif
          if( (gesture_data->u_data[i] > GESTURE_THRESHOLD_OUT) &&
              (gesture_data->d_data[i] > GESTURE_THRESHOLD_OUT) &&
              (gesture_data->l_data[i] > GESTURE_THRESHOLD_OUT) &&
              (gesture_data->r_data[i] > GESTURE_THRESHOLD_OUT) ) {

              u_last = gesture_data->u_data[i];
              d_last = gesture_data->d_data[i];
              l_last = gesture_data->l_data[i];
              r_last = gesture_data->r_data[i];
              break;
          }
      }
  }

  /* Calculate the first vs. last ratio of up/down and left/right */
  ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first);
  lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first);
  ud_ratio_last = ((u_last - d_last) * 100) / (u_last + d_last);
  lr_ratio_last = ((l_last - r_last) * 100) / (l_last + r_last);

#if DEBUG
  Serial.print(F("Last Values: "));
  Serial.print(F("U:"));
  Serial.print(u_last);
  Serial.print(F(" D:"));
  Serial.print(d_last);
  Serial.print(F(" L:"));
  Serial.print(l_last);
  Serial.print(F(" R:"));
  Serial.println(r_last);

  Serial.print(F("Ratios: "));
  Serial.print(F("UD Fi: "));
  Serial.print(ud_ratio_first);
  Serial.print(F(" UD La: "));
  Serial.print(ud_ratio_last);
  Serial.print(F(" LR Fi: "));
  Serial.print(lr_ratio_first);
  Serial.print(F(" LR La: "));
  Serial.println(lr_ratio_last);
#endif

  /* Determine the difference between the first and last ratios */
  ud_delta = ud_ratio_last - ud_ratio_first;
  lr_delta = lr_ratio_last - lr_ratio_first;

#if DEBUG
  Serial.print("Deltas: ");
  Serial.print("UD: ");
  Serial.print(ud_delta);
  Serial.print(" LR: ");
  Serial.println(lr_delta);
#endif

  /* Accumulate the UD and LR delta values */
  gesture_ud_delta += ud_delta;
  gesture_lr_delta += lr_delta;

#if DEBUG
  Serial.print("Accumulations: ");
  Serial.print("UD: ");
  Serial.print(gesture_ud_delta_);
  Serial.print(" LR: ");
  Serial.println(gesture_lr_delta_);
#endif

  /* Determine U/D gesture */
  if( gesture_ud_delta >= GESTURE_SENSITIVITY_1 ) {
      gesture_ud_count = 1;
  } else if( gesture_ud_delta <= -GESTURE_SENSITIVITY_1 ) {
      gesture_ud_count = -1;
  } else {
      gesture_ud_count = 0;
  }

  /* Determine L/R gesture */
  if( gesture_lr_delta >= GESTURE_SENSITIVITY_1 ) {
      gesture_lr_count = 1;
  } else if( gesture_lr_delta <= -GESTURE_SENSITIVITY_1 ) {
      gesture_lr_count = -1;
  } else {
      gesture_lr_count = 0;
  }

  /* Determine Near/Far gesture */
  if( (gesture_ud_count == 0) && (gesture_lr_count == 0) ) {
      if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
          (abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

          if( (ud_delta == 0) && (lr_delta == 0) ) {
              gesture_near_count++;
          } else if( (ud_delta != 0) || (lr_delta != 0) ) {
              gesture_far_count++;
          }

          if( (gesture_near_count >= 10) && (gesture_far_count >= 2) ) {
              if( (ud_delta == 0) && (lr_delta == 0) ) {
                  gesture_state = NEAR_STATE;
              } else if( (ud_delta != 0) && (lr_delta != 0) ) {
                  gesture_state = FAR_STATE;
              }
              return true;
          }
      }
  } else {
      if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
          (abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

          if( (ud_delta == 0) && (lr_delta == 0) ) {
              gesture_near_count++;
          }

          if( gesture_near_count >= 10 ) {
              gesture_ud_count = 0;
              gesture_lr_count = 0;
              gesture_ud_delta = 0;
              gesture_lr_delta = 0;
          }
      }
  }

#if DEBUG
  Serial.print("UD_CT: ");
  Serial.print(gesture_ud_count_);
  Serial.print(" LR_CT: ");
  Serial.print(gesture_lr_count_);
  Serial.print(" NEAR_CT: ");
  Serial.print(gesture_near_count_);
  Serial.print(" FAR_CT: ");
  Serial.println(gesture_far_count_);
  Serial.println("----------");
#endif

  return false;
}

/**
 * @brief Determines swipe direction or near/far state
 *
 * @return True if near/far event. False otherwise.
 */
bool decodeGesture()
{
  /* Return if near or far event is detected */
  if( gesture_state == NEAR_STATE ) {
      gesture_motion = DIR_NEAR;
      return true;
  } else if ( gesture_state == FAR_STATE ) {
      gesture_motion = DIR_FAR;
      return true;
  }

  /* Determine swipe direction */
  if( (gesture_ud_count == -1) && (gesture_lr_count == 0) ) {
      gesture_motion = DIR_UP;
  } else if( (gesture_ud_count == 1) && (gesture_lr_count == 0) ) {
      gesture_motion = DIR_DOWN;
  } else if( (gesture_ud_count == 0) && (gesture_lr_count == 1) ) {
      gesture_motion = DIR_RIGHT;
  } else if( (gesture_ud_count == 0) && (gesture_lr_count == -1) ) {
      gesture_motion = DIR_LEFT;
  } else if( (gesture_ud_count == -1) && (gesture_lr_count == 1) ) {
      if( abs(gesture_ud_delta) > abs(gesture_lr_delta) ) {
          gesture_motion = DIR_UP;
      } else {
          gesture_motion = DIR_RIGHT;
      }
  } else if( (gesture_ud_count == 1) && (gesture_lr_count == -1) ) {
      if( abs(gesture_ud_delta) > abs(gesture_lr_delta) ) {
          gesture_motion = DIR_DOWN;
      } else {
          gesture_motion = DIR_LEFT;
      }
  } else if( (gesture_ud_count == -1) && (gesture_lr_count == -1) ) {
      if( abs(gesture_ud_delta) > abs(gesture_lr_delta) ) {
          gesture_motion = DIR_UP;
      } else {
          gesture_motion = DIR_LEFT;
      }
  } else if( (gesture_ud_count == 1) && (gesture_lr_count == 1) ) {
      if( abs(gesture_ud_delta) > abs(gesture_lr_delta) ) {
          gesture_motion = DIR_DOWN;
      } else {
          gesture_motion = DIR_RIGHT;
      }
  } else {
      return false;
  }

  return true;
}

/**
 * @brief Sets the entry proximity threshold for gesture sensing
 *
 * @param[in] threshold proximity value needed to start gesture mode
 * @return True if operation successful. False otherwise.
 */
bool setGestureEnterThresh(uint8_t threshold)
{
  //if( !wireWriteDataByte(APDS9960_GPENTH, threshold) ) {
  if(write_write(APDS9960_GPENTH, threshold) != 1) {
      return false;
  }

  return true;
}

/**
 * @brief Sets the exit proximity threshold for gesture sensing
 *
 * @param[in] threshold proximity value needed to end gesture mode
 * @return True if operation successful. False otherwise.
 */
bool setGestureExitThresh(uint8_t threshold)
{
  //if( !wireWriteDataByte(APDS9960_GEXTH, threshold) ) {
  if(write_write(APDS9960_GEXTH, threshold) != 1) {
      return false;
  }

  return true;
}

/**
 * @brief Sets the gain of the photodiode during gesture mode
 *
 * Value    Gain
 *   0       1x
 *   1       2x
 *   2       4x
 *   3       8x
 *
 * @param[in] gain the value for the photodiode gain
 * @return True if operation successful. False otherwise.
 */
bool setGestureGain(uint8_t gain)
{
  uint8_t val=0;

  /* Read value from GCONF2 register */
  //if( !wireReadDataByte(APDS9960_GCONF2, val) ) {
  if(write_read(APDS9960_GCONF2, &val) != 1) {
      return false;
  }

  /* Set bits in register to given value */
  gain &= 0b00000011;
  gain = gain << 5;
  val &= 0b10011111;
  val |= gain;

  /* Write register value back into GCONF2 register */
  //if( !wireWriteDataByte(APDS9960_GCONF2, val) ) {
  if(write_write(APDS9960_GCONF2, val) != 1) {
      return false;
  }

  return true;
}

/**
 * @brief Sets the LED drive current during gesture mode
 *
 * Value    LED Current
 *   0        100 mA
 *   1         50 mA
 *   2         25 mA
 *   3         12.5 mA
 *
 * @param[in] drive the value for the LED drive current
 * @return True if operation successful. False otherwise.
 */
bool setGestureLEDDrive(uint8_t drive)
{
  uint8_t val=0;

  /* Read value from GCONF2 register */
  //if( !wireReadDataByte(APDS9960_GCONF2, val) ) {
  if(write_read(APDS9960_GCONF2, &val) != 1) {
      return false;
  }

  /* Set bits in register to given value */
  drive &= 0b00000011;
  drive = drive << 3;
  val &= 0b11100111;
  val |= drive;

  /* Write register value back into GCONF2 register */
  //if( !wireWriteDataByte(APDS9960_GCONF2, val) ) {
  if(write_write(APDS9960_GCONF2, val) != 1) {
      return false;
  }

  return true;
}

bool setLEDBoost(uint8_t boost)
{
    uint8_t val=0;

    /* Read value from CONFIG2 register */
    //if( !wireReadDataByte(APDS9960_CONFIG2, val) ) {
    if(write_read(APDS9960_CONFIG2, &val) != 1) {
        return false;
    }

    /* Set bits in register to given value */
    boost &= 0b00000011;
    boost = boost << 4;
    val &= 0b11001111;
    val |= boost;

    /* Write register value back into CONFIG2 register */
    //if( !wireWriteDataByte(APDS9960_CONFIG2, val) ) {
    if(write_write(APDS9960_CONFIG2, val) != 1) {
        return false;
    }

    return true;
}

/**
 * @brief Sets the time in low power mode between gesture detections
 *
 * Value    Wait time
 *   0          0 ms
 *   1          2.8 ms
 *   2          5.6 ms
 *   3          8.4 ms
 *   4         14.0 ms
 *   5         22.4 ms
 *   6         30.8 ms
 *   7         39.2 ms
 *
 * @param[in] the value for the wait time
 * @return True if operation successful. False otherwise.
 */
bool setGestureWaitTime(uint8_t time)
{
  uint8_t val=0;

  /* Read value from GCONF2 register */
  //if( !wireReadDataByte(APDS9960_GCONF2, val) ) {
  if(write_read(APDS9960_GCONF2, &val) != 1) {
      return false;
  }

  /* Set bits in register to given value */
  time &= 0b00000111;
  val &= 0b11111000;
  val |= time;

  /* Write register value back into GCONF2 register */
  //if( !wireWriteDataByte(APDS9960_GCONF2, val) ) {
  if(write_write(APDS9960_GCONF2, val) != 1) {
      return false;
  }

  return true;
}

/**
 * @brief Turns gesture-related interrupts on or off
 *
 * @param[in] enable 1 to enable interrupts, 0 to turn them off
 * @return True if operation successful. False otherwise.
 */
bool setGestureIntEnable(uint8_t enable)
{
  uint8_t val=0;

  /* Read value from GCONF4 register */
  //if( !wireReadDataByte(APDS9960_GCONF4, val) ) {
  if(write_read(APDS9960_GCONF4, &val) != 1) {
      return false;
  }

  /* Set bits in register to given value */
  enable &= 0b00000001;
  enable = enable << 1;
  val &= 0b11111101;
  val |= enable;

  /* Write register value back into GCONF4 register */
  //if( !wireWriteDataByte(APDS9960_GCONF4, val) ) {
  if(write_write(APDS9960_GCONF4, val) != 1) {
      return false;
  }

  return true;
}

/**
 * @brief Tells the state machine to either enter or exit gesture state machine
 *
 * @param[in] mode 1 to enter gesture state machine, 0 to exit.
 * @return True if operation successful. False otherwise.
 */
bool setGestureMode(uint8_t mode)
{
  uint8_t val=0;

  /* Read value from GCONF4 register */
  //if( !wireReadDataByte(APDS9960_GCONF4, val) ) {
  if(write_read(APDS9960_GCONF4, &val) != 1) {
      return false;
  }

  /* Set bits in register to given value */
  mode &= 0b00000001;
  val &= 0b11111110;
  val |= mode;

  /* Write register value back into GCONF4 register */
  //if( !wireWriteDataByte(APDS9960_GCONF4, val) ) {
  if(write_write(APDS9960_GCONF4, val) != 1) {
      return false;
  }

  return true;
}

/*******************************************************************************
 * Raw I2C Reads and Writes
 ******************************************************************************/

/**
 * @brief Writes a single byte to the I2C device (no register)
 *
 * @param[in] val the 1-byte value to write to the I2C device
 * @return True if successful write operation. False otherwise.
 */
/*bool wireWriteByte(uint8_t val)
{
    Wire.beginTransmission(APDS9960_I2C_ADDR);
    Wire.write(val);
    if( Wire.endTransmission() != 0 ) {
        return false;
    }

    return true;
}*/

/**
 * @brief Writes a single byte to the I2C device and specified register
 *
 * @param[in] reg the register in the I2C device to write to
 * @param[in] val the 1-byte value to write to the I2C device
 * @return True if successful write operation. False otherwise.
 */
/*bool wireWriteDataByte(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(APDS9960_I2C_ADDR);
    Wire.write(reg);
    Wire.write(val);
    if( Wire.endTransmission() != 0 ) {
        return false;
    }

    return true;
}*/

/**
 * @brief Reads a single byte from the I2C device and specified register
 *
 * @param[in] reg the register to read from
 * @param[out] the value returned from the register
 * @return True if successful read operation. False otherwise.
 */
/*bool wireReadDataByte(uint8_t reg, uint8_t &val)
{

    //Indicate which register we want to read from
    if (!wireWriteByte(reg)) {
        return false;
    }

    // Read from register
    Wire.requestFrom(APDS9960_I2C_ADDR, 1);
    while (Wire.available()) {
        val = Wire.read();
    }

    return true;
}*/

/**
 * @brief Reads a block (array) of bytes from the I2C device and register
 *
 * @param[in] reg the register to read from
 * @param[out] val pointer to the beginning of the data
 * @param[in] len number of bytes to read
 * @return Number of bytes read. -1 on read error.
 */
/*int wireReadDataBlock(   uint8_t reg,
                                        uint8_t *val,
                                        unsigned int len)
{
    unsigned char i = 0;

    // Indicate which register we want to read from
    if (!wireWriteByte(reg)) {
        return -1;
    }

    // Read block data
    Wire.requestFrom(APDS9960_I2C_ADDR, len);
    while (Wire.available()) {
        if (i >= len) {
            return -1;
        }
        val[i] = Wire.read();
        i++;
    }

    return i;
}*/

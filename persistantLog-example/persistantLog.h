/* persistantLog.h -> A library to do logging of some predefined data structure to EEPROM or Flash memory

Author: Imogen Wren
Date:  19/03/2026

For AVR (EEPROM) and SAMD21 (Flash)
 could be ported to ESP32 and STM32 in the future

 WARNING! On SAMD21 data is overwritten on program flash. B

*/

#include "Arduino.h"

#ifndef persistantLog_h 
#define persistantLog_h


// First create the structure for the data we need to save, include an ID, as this will be incremented for each data store so we know which one was written last
// Also include a signature that will help validate the saved data
// Total structure should not exceed 256 Bytes

struct logData_t {
  uint32_t recordID;
  uint32_t motorSeconds;
  uint32_t generatorRotations;
  uint16_t yawsCommanded;
  uint16_t pitchCommanded;
  uint32_t signature;
  // Overloaded operators to allow quick comparasons using boolean operators
  // This also allows us to only compare the data we actually care about
  bool operator==(const logData_t &other) const {
    return motorSeconds == other.motorSeconds && generatorRotations == other.generatorRotations && yawsCommanded == other.yawsCommanded && pitchCommanded == other.pitchCommanded;
  }

  bool operator!=(const logData_t &other) const {
    return !(*this == other);
  }
};


#ifdef __AVR__
#pragma "Compiled for AVR"
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
#pragma "Compiled for SAMD"
#include <FlashStorage.h>  // for SAMD21
//#include <FlashAsEEPROM.h>  // alternative method for SAMD21 with EEPROM like API (but erases entire block in one action, lowering lifetime?)
#endif

#define DATA_LOG_SIGNATURE 0x978


// SAMD21 Method Write -> logs[log_IDX]->write(new_data);
// SAMD21 Method Read ->  logs[log_number]->read();

// AVR Method Write ->  
//    uint32_t addr = EEPROM.length() - (((NUM_LOGS + 1) - log_IDX) * sizeof(logData_t));
//    EEPROM.put(addr, new_data);
// AVR Method Read -> 
//    uint32_t addr = EEPROM.length() - ((NUM_LOGS + 1 - _log_IDX) * sizeof(logData_t));
//    EEPROM.get(addr, recalled_log);


class persistantLog {

public:
  // constructor
  persistantLog();



logData_t g_recalled_log = {};  // global recalled log updated from memory and then remains constant to act as a comparason untill new data is written to memory
logData_t current_log = {};     // global current_log should be updated on startup from memory, then kept updated locally untill any changes are written to memory once every logging period

// Instead of the objects as defined previously, now we are going to start saving data at address, EEPROM.length()-11*sizeof(logData_t), then each subsequent data can be saved at addresses closer to the end of EEPROM
// this should keep it away from any other useage of EEPROM which is at the start.
// We will still use log index, but each index will be saved at address of EEPROM.length()-(11-log_IDX)*sizeof(logData_t)
uint8_t log_IDX = 0;  // stores the INDEX of the last log written
const uint8_t NUM_LOGS = 10;
// In code using NUM_LOGS+1 for address assigning to leave one block free at the end for errors while programming.
// May also use EEPROM.length()-((11*sizeof(logData_t))- sizeof(INDEX_STRUCTURE) as address for saving indexing data, to speed up read & writes

void init_log();
void check_eeprom_length();
void print_stats(const logData_t &new_data);
logData_t get_log(uint8_t log_number); 
logData_t get_latest_log(); 
int16_t write_log(logData_t new_data); // returns 1 on success,// returns 0 on data not changed,// returns -1 on error#]// Maybe?
int16_t update_log();    // writes log using current_log directly, does not need to be passed changed data
void struct_size_check();

// Need to add function to get and write data to the global struct to avoid users interacting with it directly
logData_t get_current();   // returns the status of the current log (does not update recalled log
void write_current(logData_t new_data);   // update current log with new data, ready to be written


private:





};


#endif
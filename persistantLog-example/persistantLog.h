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
#include <EEPROM.h>
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
#include <FlashStorage.h>  // for SAMD21
//#include <FlashAsEEPROM.h>  // alternative method for SAMD21 with EEPROM like API (but erases entire block in one action, lowering lifetime?)
// create a number of flash storage objects
// FlashStorage(log_0, secretObject::lifetimeStruct_t); // this creates a single log, we want an enumerated list of logs, each 256 Bytes big as they will each occupy one eraseable block
// then we can do load balancing across multiple areas of memory and avoid stressing one.
// SAMD21 has 1024 contiguous memory zones for flash, however, it shares this space with program code, constants etc. so useable slots is:
// useable flash - 256 kB
// Used Flash ~ U
// MaxBlocks = (256 kB - U) / 256 B

// 10 blocks is probably fine, plus at least 1 or 2 blocks for secretObject, and 1 additional block for preindexing information to speed up searches.

// Macros defined by chatGPT so use with pinch of salt
// define the list
#define FLASH_LOG_LIST \
  X(log_0) \
  X(log_1) \
  X(log_2) \
  X(log_3) \
  X(log_4) \
  X(log_5) \
  X(log_6) \
  X(log_7) \
  X(log_8) \
  X(log_9)

// Expand into objects
#define X(name) FlashStorage(name, logData_t);
FLASH_LOG_LIST
#undef X

// Build pointer array to allow enumeration through storage objects
#define X(name) &name,
FlashStorageClass<logData_t> *logs[] = {
  FLASH_LOG_LIST
};
#undef X

// Add enum for clarity, this will mean each log can be accessed using logs[log_0_IDX]->write(...); and logs[log_1_IDX]->read(...);
enum LogIndex {
#define X(name) name##_IDX,
  FLASH_LOG_LIST
#undef X
    NUM_LOGS
};

#define LOGGING_PERIOD_MIN 30
#define LOGGING_PERIOD_S LOGGING_PERIOD_MIN * 60
#define LOGGING_PERIOD_mS LOGGING_PERIOD_S * 1000

// And can be enumerated through with patterns like
/*
for (int i = 0; i < NUM_LOGS; i++){
  logs[i]->read(...);
}
*/
#endif

#define DATA_LOG_SIGNATURE 0x978


// SAMD21 Method logs[log_IDX]->write(new_data);
// SAMD21 Method logs[log_number]->read();


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

void check_eeprom_length();
void print_stats(const logData_t &new_data);
logData_t get_log(uint8_t log_number); 
logData_t get_latest_log(); 
int16_t write_log(logData_t new_data); // returns 1 on success,// returns 0 on data not changed,// returns -1 on error#]// Maybe?
void struct_size_check() ;

private:





};


#endif
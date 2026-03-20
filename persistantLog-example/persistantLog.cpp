

#include "persistantLog.h"


#ifdef __AVR__
#include <EEPROM.h>
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
// create a number of flash storage objects
// FlashStorage(log_0, secretObject::lifetimeStruct_t); // this creates a single log, we want an enumerated list of logs, each 256 Bytes big as they will each occupy one eraseable block
// then we can do load balancing across multiple areas of memory and avoid stressing one.
// SAMD21 has 1024 contiguous memory zones for flash, however, it shares this space with program code, constants etc. so useable slots is:
// useable flash - 256 kB
// Used Flash ~ U
// MaxBlocks = (256 kB - U) / 256 B
//

// 10 blocks is probably fine, plus at least 1 or 2 blocks for secretObject, and 1 additional block for preindexing information to speed up searches.
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









persistantLog::persistantLog() {
}

void persistantLog::init_log() {
  current_log = persistantLog::get_latest_log();
}


void persistantLog::check_eeprom_length() {
#ifdef __AVR__
  Serial.print("EEPROM Size: ");
  Serial.println(EEPROM.length());
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
  Serial.print("Flash Size: ");
  Serial.println("256 kB - flashUsed");
#endif
}


void persistantLog::print_stats(const logData_t &new_data) {
  Serial.print("{\"recordID\":");
  Serial.print(new_data.recordID);
  Serial.print(",\"motorHours\":");
  Serial.print(new_data.motorSeconds);
  Serial.print(",\"genRotations\":");
  Serial.print(new_data.generatorRotations);
  Serial.print(",\"yaw\":");
  Serial.print(new_data.yawsCommanded);
  Serial.print(",\"pitch\":");
  Serial.print(new_data.pitchCommanded);
  Serial.print(",\"sig\":0x");
  Serial.print(new_data.signature, HEX);
  Serial.println("}");
}



logData_t persistantLog::get_log(uint8_t _log_IDX) {
  logData_t recalled_log;
#ifdef __AVR__
  uint32_t addr = EEPROM.length() - ((NUM_LOGS + 1 - _log_IDX) * sizeof(logData_t));
  EEPROM.get(addr, recalled_log);
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
  recalled_log = logs[_log_IDX]->read();
#endif
  return recalled_log;
}



logData_t persistantLog::get_latest_log() {
  bool data_valid = false;
  uint32_t log_ID = 0;
  logData_t recalled_log;
  for (uint8_t i = 0; i < NUM_LOGS; i++) {  // enumerate through all logs looking for highest ID
    recalled_log = get_log(i);
    //  Serial.print("\nprinting log: ");
    //  Serial.println(i);
    //   print_stats(recalled_log);
    if (recalled_log.signature == DATA_LOG_SIGNATURE) {  // data has been written
                                                         //    Serial.print("Valid Data Found Log_IDX: ");
                                                         //    Serial.println(i);
      if (recalled_log.recordID >= log_ID) {             // the found record is later than the previous on
                                                         //      Serial.print("Record ID Greater than last found record, New: ");
                                                         //      Serial.print(recalled_log.recordID);
                                                         //      Serial.print(", old: ");
                                                         //      Serial.println(log_ID);
        log_ID = recalled_log.recordID;
        log_IDX = i;
        data_valid = true;
      }
    }
  }
  if (data_valid) {
    g_recalled_log = get_log(log_IDX);
    //    Serial.println();
    //    Serial.print("getting gobal recalled log, IDX: ");
    //   Serial.println(log_IDX);
    //   print_stats(g_recalled_log);
    //    Serial.println();
    return g_recalled_log;
  } else {
    return g_recalled_log;
  }
}


// returns 1 on success,
// returns 0 on data not changed,
// returns -1 on error // Maybe?
int16_t persistantLog::write_log(logData_t new_data) {
  if (new_data == g_recalled_log) {
    Serial.println(F("{\"info\":\"Data has not changed, returning\"}"));
    return 0;
  }
  new_data.recordID = g_recalled_log.recordID + 1;
  new_data.signature = DATA_LOG_SIGNATURE;
  log_IDX++;  // increment the log index
  if (log_IDX >= NUM_LOGS) {
    log_IDX = 0;
  }
  Serial.print(F("{\"info\":\"Adding New Record\",\"IDX\": "));
  Serial.print(log_IDX);
  Serial.println(F("\"}"));
  print_stats(new_data);
#ifdef __AVR__
  uint32_t addr = EEPROM.length() - (((NUM_LOGS + 1) - log_IDX) * sizeof(logData_t));
  EEPROM.put(addr, new_data);
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
  logs[log_IDX]->write(new_data);
#else
  return -1;  // if no valid method return error
#endif
  g_recalled_log = new_data;     // make sure out global variable holds the now valid data
  current_log = g_recalled_log;  // current log should now be the same but just incase
  return 1;
}

int16_t persistantLog::update_log() {  // writes log using current_log directly, does not need to be passed changed data
  return persistantLog::write_log(current_log);
}



logData_t persistantLog::get_current() {  // returns the status of the current log (does not update recalled log
  return current_log;
}


void persistantLog::write_current(logData_t new_data) {  // update current log with new data, ready to be written
  current_log = new_data;
}

void persistantLog::struct_size_check() {
  Serial.print("sizeof{struct}: ");
  Serial.print(sizeof(logData_t));
#ifdef __AVR__
  Serial.print(", size of EEPROM: ");
  Serial.print(EEPROM.length());
  Serial.print(", Max Records: ");
  Serial.println(EEPROM.length() % sizeof(logData_t));
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
  Serial.println(", Must be under 256 Bytes");
#endif
}

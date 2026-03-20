

#include "persistantLog.h"



persistantLog::persistantLog() {

}


void persistantLog::check_eeprom_length() {
  Serial.print("EEPROM Size: ");
  Serial.println(EEPROM.length());
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



logData_t persistantLog::get_log(uint8_t log_number) {
  logData_t recalled_log;
  uint32_t addr = EEPROM.length() - ((NUM_LOGS + 1 - log_number) * sizeof(logData_t));
  EEPROM.get(addr, recalled_log);
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
      if (recalled_log.recordID >= log_ID) {  // the found record is later than the previous on
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
// returns -1 on error#]// Maybe?
int16_t persistantLog::write_log(logData_t new_data) {
  if (new_data == g_recalled_log) {
    Serial.println("Data has not changed, returning (not for now)");
    return 0;
  }

  new_data.recordID = g_recalled_log.recordID + 1;
  new_data.signature = DATA_LOG_SIGNATURE;
  log_IDX++;  // increment the log index
  if (log_IDX >= NUM_LOGS) {
    log_IDX = 0;
  }
  Serial.print("Adding New Record in IDX: ");
  Serial.println(log_IDX);
  print_stats(new_data);
  uint32_t addr = EEPROM.length() - (((NUM_LOGS + 1) - log_IDX) * sizeof(logData_t));
  EEPROM.put(addr, new_data);
  return 1;
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
Serial.println("Must be under 256 Bytes");
 #endif 
}




#include "persistantLog.h"



persistantLog pLog;

void setup() {
  Serial.begin(115200);
  delay(2500);
  Serial.println("\n\nTesting Basic EEPROM Like Wear Testing Test\n");
  pLog.check_eeprom_length();
  pLog.struct_size_check();
  delay(2000);
  logData_t recalled_data = pLog.get_latest_log();
  Serial.println("Printing Recalled Stats: ");
  pLog.print_stats(recalled_data);
  delay(2000);
  recalled_data.generatorRotations++;
  delay(2000);
  Serial.println("Printing new stats (to be saved)");
  pLog.print_stats(recalled_data);
  delay(2000);
  Serial.println("writing Log");
  pLog.write_log(recalled_data);
  delay(2000);
  Serial.println("Program Finished Writing Changes");
  recalled_data = pLog.get_latest_log();
  Serial.println("Printing recalled log");
  pLog.print_stats(recalled_data);
}

void loop() {
  // put your main code here, to run repeatedly:
}

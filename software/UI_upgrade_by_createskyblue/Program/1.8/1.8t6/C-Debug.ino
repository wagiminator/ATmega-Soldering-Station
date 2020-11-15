/*
//此片段用于DEBUG
void HelpMeSerialer() {
  //串口助手
  if (Serial.available()) {
    char TmpChar = Serial.read();
    switch (TmpChar) {
      case 'x': x = Serial.parseInt();
      case 'y': y = Serial.parseInt();
      case 'a': a = Serial.parseInt();
      case 'b': b = Serial.parseInt();
    }
  }
  
    Serial.println(String("X ->") + x);
    Serial.println(String("Y ->") + y);
    Serial.println(String("A ->") + a);
    Serial.println(String("B ->") + b);
  
}
*/

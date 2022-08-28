int Brightness = 0;

void setup() {
 pinMode(D4, OUTPUT);
 pinMode(D5, OUTPUT);
}
void loop() {
  if(Brightness == 255){
    while(Brightness >0){
      Brightness--;
      analogWrite(D4, Brightness);
      analogWrite(D5, Brightness);
    }
  }
  if (Brightness == 0){
      while(Brightness <255){
        Brightness++;
        analogWrite(D4, Brightness);
        analogWrite(D5, Brightness);
      }
    }
  }

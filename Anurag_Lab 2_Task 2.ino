#include <Wire.h>
byte address = 0x48;
uint8_t MSByte = 0, LSByte = 0;
uint16_t regValue = 0;
 
void setup()
{
 Wire.begin();
 Serial.begin(9600);
}
void loop()
{
 Wire.beginTransmission(address);
 int error = Wire.endTransmission();
 if (error) {
 }else {
 Serial.println("Success");
 Wire.beginTransmission(address);
 Wire.write(0b00000001); // Config Register
 Wire.write(0b11010000); // MSB of Config Register
 Wire.write(0b10000011); // LSB of Config Register
 Wire.endTransmission();
 
 Wire.beginTransmission(address);
 Wire.write(0b00000000); // Conversion Register
 Wire.endTransmission();
 
 Wire.requestFrom(address,2);
 if(Wire.available()){
 MSByte = Wire.read();
 LSByte = Wire.read();
 }
 regValue = (MSByte<<8) + LSByte; 
 // How do we get this result below?
 float result = (regValue * 1.0 / 32768) * 6.144;
 Serial.print("Register Value: ");
 Serial.println(regValue);
 Serial.print("Volts: ");
 Serial.print(result);
 Serial.println("v");
 }
 delay(1000);
}

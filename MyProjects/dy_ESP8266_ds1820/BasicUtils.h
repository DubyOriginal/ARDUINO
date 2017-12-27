//*******************************************************************************************************
/*
static const uint8_t D0 = 16;  // D0 -> gpio 16
static const uint8_t D1 = 5;   // D1 -> gpio 5
static const uint8_t D2 = 4;   // D2 -> gpio 4
static const uint8_t D3 = 0;   // D3 -> gpio 0
static const uint8_t D4 = 2;   // D4 -> gpio 2
static const uint8_t D5 = 14;  // D5 -> gpio 14
static const uint8_t D6 = 12;  // D6 -> gpio 12
static const uint8_t D7 = 13;  // D7 -> gpio 13
static const uint8_t D8 = 15;  // D8 -> gpio 15

static const uint8_t S2 = 9;   // SD2 -> gpio 9
static const uint8_t S3 = 10;  // SD3 -> gpio 10

static const uint8_t RX = 3;  // RX -> gpio 3
static const uint8_t TX = 1;  // TX -> gpio 1
*/
//*******************************************************************************************************
/*
    0 : WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses
    1 : WL_NO_SSID_AVAILin case configured SSID cannot be reached
    3 : WL_CONNECTED after successful connection is established
    4 : WL_CONNECT_FAILED if password is incorrect
    6 : WL_DISCONNECTED if module is not configured in station mode
*/
String decodeWifiSTATUS(int wifiStatus){
  switch(wifiStatus){
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS"; 
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL"; 
    case WL_CONNECTED:
      return "WL_CONNECTED"; 
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED"; 
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED"; 
    default:
      return "UNKNOWN_STATUS"; 
  }
}

//*******************************************************************************************************
//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

//*******************************************************************************************************
// 0x05 -> getFormatedStrFromHex8(0x05) -> "05"
String getFormatedStrFromHex8(uint8_t *data, uint8_t length) {
       String result = ""; 
       for (int i=0; i<length; i++) { 
         if (data[i]<0x10) {
          result += "0" + String(data[i],HEX);
         } else{
          result += String(data[i],HEX);
         }
         if (i < (length-1)){
          result += " ";
         }
       }
       return result;
}

// prints 8-bit data in hex with leading zeroes
// uint8_t ByteData[5]={0x01, 0x0F, 0x10, 0x11, 0xFF};  -> PrintHex8(ByteData,5);
// uint8_t ByteDatum=0x01;                              -> PrintHex8(&ByteDatum,1);
void PrintHex8(uint8_t *data, uint8_t length) {
       Serial.print("0x"); 
       for (int i=0; i<length; i++) { 
         if (data[i]<0x10) {Serial.print("0");} 
         Serial.print(data[i],HEX); 
         Serial.print(" "); 
       }
}

// prints 16-bit data in hex with leading zeroes
String PrintHex16(uint16_t *data, uint8_t length) {
       Serial.print("0x"); 
       for (int i=0; i<length; i++)
       { 
         uint8_t MSB=byte(data[i]>>8);
         uint8_t LSB=byte(data[i]);
         
         if (MSB<0x10) {Serial.print("0");} Serial.print(MSB,HEX); Serial.print(" "); 
         if (LSB<0x10) {Serial.print("0");} Serial.print(LSB,HEX); Serial.print(" "); 
       }
}










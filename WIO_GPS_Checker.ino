#include <TFT_eSPI.h>

#define WIDTH 320
#define HEIGHT 240

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

const int BUFFER_SIZE = 200;
char buffer[BUFFER_SIZE];
int packetCount=0;
int mode = 0;
int bufferPos;

String UTC_Time;
String GPS_latitude;
String northSouth;
String GPS_longitude;
String eastWest;
int qualityIndicator;
int satsUsed;
float hdop;
float altAboveSeaLevel;
float geoSep;


void setup()
{
  //Serial.begin(9600);  //Uncomment if you want to use the serial terminal when connected to USB on your PC for debugging
  Serial1.begin(9600);

  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(0x1DF);
  tft.setTextColor(0xFF40);
  tft.setTextSize(1);
  
  img.createSprite(WIDTH, HEIGHT);
  img.fillSprite(TFT_BLACK);
  img.setTextSize(1);
  img.setTextColor(0xFF40);
}

void loop()
{
  if (digitalRead(WIO_KEY_B) == LOW) {
    mode = 1;
    img.fillSprite(TFT_BLACK); 
    img.setCursor(0,0);
  }
  else if (digitalRead(WIO_KEY_C) == LOW) {
    mode = 0;
  }
   
  memset (buffer, 0, 200);
    
  if (mode == 0) 
  {
    img.fillSprite(TFT_BLACK); 
    img.setCursor(0,0);
    img.print("GPS Raw NMEA");
    img.setCursor(200,0);
    img.print("Packet Count: ");
    img.print(packetCount, DEC);
    img.println();
    img.println();
    while(Serial1.available())                     
    {  
      Serial1.readBytesUntil('\n', buffer, BUFFER_SIZE);
      img.print(buffer);
      img.print('\n');


      // Slightly dubious idea about working out when a "new" packet of info comes down from the GPS by working out when it's finished sending and we
      // have nothing coming through for a while.
      int basetime = millis();
      while (!Serial1.available());
      int timecost = millis() - basetime;
      if(timecost > 100){
          packetCount++;
          img.pushSprite(0,0);
          break;
      } 
    }
  }

  if (mode == 1)
  {
    img.fillSprite(TFT_BLACK); 
    img.setCursor(0,0);
    img.print("GPS Parsed Info");
    img.setCursor(200,0);
    img.print("Packet Count: ");
    img.print(packetCount, DEC);
    img.println();
    img.println();
    while(Serial1.available())                     
    {  
      Serial1.readBytesUntil('\n', buffer, BUFFER_SIZE);
      char * pos = strstr(buffer, "$GNGGA");
      if (pos) 
      {
        DecodeGGA();
        img.print("UTC Time: " + UTC_Time + "\n");
        img.print("GPS Latitude: " + GPS_latitude + " " + northSouth + "\n");
        img.print("GPS Longitude: " + GPS_longitude + " " + eastWest + "\n");
        img.print("GPS Quality Indicator: " + String(qualityIndicator) + "\n");
        img.print("Number of Sats Used: " + String(satsUsed) + "\n");
        img.print("HDOP: " + String(hdop) + "\n");
        img.print("Altitude (above Sea level): " + String(altAboveSeaLevel) + "\n");
        img.print("Geoidal Seperation: " + String(geoSep) + "\n");
      }
        
      int basetime = millis();
      while (!Serial1.available());
      int timecost = millis() - basetime;
      if (timecost > 100) {
          packetCount++;
          img.pushSprite(0,0);
          break;
      } 
    }
  }
}

// Function to decode the GGA NMEA string (based on info from https://navspark.mybigcommerce.com/content/NMEA_Format_v0.1.pdf)
void DecodeGGA(void)
{
  bufferPos = 7; // Position ourselves the next char along from $GNGGA,
  
  UTC_Time = ReadUntilNextComma();
  bufferPos++; //Buffer position will be on the comma

  GPS_latitude = ReadUntilNextComma();
  bufferPos++;

  northSouth = ReadUntilNextComma();
  if (northSouth.length() > 1) 
  {
    northSouth = "";
  }
  bufferPos++;

  GPS_longitude = ReadUntilNextComma();
  bufferPos++;

  eastWest = ReadUntilNextComma();
  if (eastWest.length() > 1) 
  {
    eastWest = "";
  }
  bufferPos++;

  String tempBuff = ReadUntilNextComma();
  qualityIndicator = tempBuff.toInt();
  bufferPos++;

  tempBuff = ReadUntilNextComma();
  satsUsed = tempBuff.toInt();
  bufferPos++;

  tempBuff = ReadUntilNextComma();
  hdop = tempBuff.toFloat();
  bufferPos++;

  tempBuff = ReadUntilNextComma();
  altAboveSeaLevel = tempBuff.toFloat();
  bufferPos++;

  tempBuff = ReadUntilNextComma();
  geoSep = tempBuff.toFloat();
  bufferPos++;
}

// NMEA strings are comma seperated.  If we have 2 commas together it means there's no info available for that particular part of the string.
String ReadUntilNextComma()
{
  String resultString;
  if (String(buffer[bufferPos]) == ",")
  {
    resultString = "N/A";
  } else {
    while (String(buffer[bufferPos]) != ",")
    {
      resultString.concat(buffer[bufferPos]);
      bufferPos++;
    }
  }
  return resultString;
}
  

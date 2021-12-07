#define OUTpin 10
#define ReadingDelay 0

void setup()
{
/**********************************************************************************/
// Set pwm clock divider
/**********************************************************************************/
    TCCR1B &= ~(1 << CS12);
    TCCR1B  |=   (1 << CS11);
    TCCR1B &= ~(1 << CS10); 

/**********************************************************************************/
// Set pwm resolution  to mode 7 (10 bit)
/**********************************************************************************/

    TCCR1B &= ~(1 << WGM13);    // Timer B clear bit 4
    TCCR1B |=  (1 << WGM12);    // set bit 3
    TCCR1A |= (1 << WGM11);    //  Timer A set bit 1
    TCCR1A |= (1 << WGM10);    //  set bit 0
  

    pinMode(OUTpin, OUTPUT);
    
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial2.begin(115200);
    Serial3.begin(115200);
}

void loop()
{

while((Serial1.available()>=9) && (Serial2.available()>=9) && (Serial3.available()>=9))
  {
    {
      uint32_t dist[3];
      if((0x59 == Serial1.read()) && (0x59 == Serial1.read())) //Byte1 & Byte2
      {
        uint32_t b1 = Serial1.read(); //Byte3
        uint32_t b2 = Serial1.read(); //Byte4

        b2 <<= 8;
        b2 += b1;
        dist[0] = {b2};
            
        for (int i=0; i<5; i++) 
        { 
          Serial1.read(); ////Byte5,6,7,8,9
        }
      }
      if((0x59 == Serial2.read()) && (0x59 == Serial2.read())) //Byte1 & Byte2
      {
        uint32_t b1 = Serial2.read(); //Byte3
        uint32_t b2 = Serial2.read(); //Byte4

        b2 <<= 8;
        b2 += b1;
        dist[1] = {b2};
        
        for (int i=0; i<5; i++) 
        { 
          Serial2.read(); ////Byte5,6,7,8,9
        }
      }
      if((0x59 == Serial3.read()) && (0x59 == Serial3.read())) //Byte1 & Byte2
      {
        uint32_t b1 = Serial3.read(); //Byte3
        uint32_t b2 = Serial3.read(); //Byte4

        b2 <<= 8;
        b2 += b1;
        dist[2] = {b2};
            
        for (int i=0; i<5; i++) 
        { 
           Serial3.read(); ////Byte5,6,7,8,9
        }
      }
           /*   for (int i=0; i<2; i++) 
        { 
           analogWrite(OUTpin, 255);
           delay(ReadingDelay);
           analogWrite(OUTpin, 1);
           delay(ReadingDelay);
        }*/
        for (int i=0; i<2; i++) 
        { 
           uint16_t pinVal = (dist[i]);
           analogWrite(OUTpin, pinVal);
           //delay(ReadingDelay);
           //analogWrite(OUTpin, 1);
           delay(ReadingDelay);
        }
        
        Serial.print("Dist");
        Serial.print("\t");
        Serial.print(dist[0]);
        Serial.print("\t");
        Serial.print(dist[1]);
        Serial.print("\t");
        Serial.println(dist[2]);
    }
  }
}

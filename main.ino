// write yoour dump here 
/*
c56 - subaru wrx 2003-2007
char writeDump[] = ""
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 ffff ffff ffff ffff ffff ffff ffff"
"a5a5 a5a5 a5a5 a5a5 a5a5 a5a5 a5a5 5a5a"
"";
*/

// write yoour dump here 
// c46 - subaru wrx 00-03
char writeDump[] = ""
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 0000 0000 0000 0000"
"0000 0000 0000 0000 5a5a 5a5a a5a5 a5a5"
"";

int eeprom_size = 1024; // 128x16bit => (2048)bit
int eeprom_bank_size = 8; //bit -> 128x(16)bit = 2048bit = 256bytes

int eeprom_address_width_bits = 7; // see datasheet, A0-A6 - mean 7 bit of data address
                                    // As sample S220 has 7 bit address
int needDummyZero = 0; // need for c56, not need for c46
int useWriteTail = 1; // need for c46. It write 8 bit data and 8 bit null information. Don't know why it need.

/*
MicroWire protocol integration.
Use this programm to read and write dumps in to EEPROM chips like S220, 93c56 etc.

AIRBAG modules used in different cars:
Subaru	 		98221 AG180       152300-8270	93C57
 	 		98221 FE220 Denso 152300-84700	93C57
 	Forester	98221 SA011 Denso 152399-5181	93C56
 	 		98221 SC030 Denso 150300-0940	93C86
 	 		98221 SC041 Denso		93C86
 	Impreza		98221 FA101 Denso 152300-2150	93C46
 	 		98221 FA160 Denso 152300-2391	93C46
 	 		98221 FA171 Denso 152300-2402	93C46
 	 		98221 FE030 Denso 152300-3961	93C46
 	 		98221 FE100 Denso 152300-6130	93C56 <- tested
 	 		98221 FE110 Denso 152300-6140	93C56
 	 		98221 FE190 Denso 152300-8440	93C56
 	 		98221 FG040 Denso 150300-0660	93C86
 	 		98221 FG070 Denso 150300-1460	93C86
 	Justy		89170-B1200 Denso 150300-1071	93C56
 	Legacy		98221 AG260 Denso 150300-0710	93C86
 	Outback		98221 AG250       150300-0700	93C56
 	Tribeca		98221 XA00A Denso 152300-8221	93C56
 
Subaru uses: 93c46, S220, 93LC56, 93c86, 93s56, 93c56, L56R, 24c32
 
For test purposes use resistor for about 5Ω (5 ohm) - for american cars. For european not tested yet.

@see http://www.carhelp.info/forums/showthread.php?t=33475
@see http://phreaker.us/forum/showthread.php?t=4205&page=7
*/

//defining pins for eeprom   
int CHIP_SEL = 11; // CS  = chip select
int CLOCK    = 10; // CLK = clock
int DATA_IN  = 8;
int DATA_OUT = 9;
/* See datasheet how to connect your chip. Usual connecting map:

  1 pin(* marker on chip)
        ---------
   CS -| *       |- Vcc = 3-5v   - 8th pin
  Clk -|         |-
   Di -|         |-
   Do -|         |- Gnd
        ---------
     4 pin       5pin

NOTE be sure you connected Data Input of chip to Data Output of device. You need cross wires 
     for input and output pins.

@datasheet 93c56: http://pdf1.alldatasheet.com/datasheet-pdf/view/56327/ATMEL/93C56.html
@datasheet s220:  http://www.digchip.com/datasheets/parts/datasheet/000/S29XX0A-pdf.php
           s220 is same as 93c56 but has another vendor.
           
@see http://www.drive2.ru/l/1568805/
     Connecting eeprom to LPT or COM port (RS232). 
*/

// =================================

char buf[256];
#define printf(fmt, ...) sprintf(buf, fmt, __VA_ARGS__);Serial.print(buf);

/**
 * arduino setup
 */
void setup(){
  pinMode(CLOCK ,OUTPUT);
  pinMode(DATA_OUT ,OUTPUT);
  pinMode(DATA_IN ,INPUT);
  pinMode(CHIP_SEL ,OUTPUT);
  digitalWrite(CHIP_SEL ,LOW);
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

unsigned int hexToDec(String hexString) {
  
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}

String decToHex(byte decValue, byte desiredStringLength) {
  
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
  
  return hexString;
}

/**
 * send serial data to EEPROM bit by bit.
 */
void sendSer(int x, int length)
{
  int mask = 1<<(length-1);
  for(int i=0;i<length;i++)
  {
    int bitValue = x&mask;
    x<<=1;
    
    if(!bitValue)
    {
      digitalWrite(DATA_OUT, LOW);
    } 
    else
    {
      digitalWrite(DATA_OUT, HIGH);
    }
    digitalWrite(CLOCK,HIGH); 
    digitalWrite(CLOCK,LOW); 
//    delay(1);
  }
  
  //needs little delay to work
  delay(1);
}

/**
 * read EEPROM data and put formated conttents to Serial port.
 */
void readEeprom()
{
  printf("===START DUMP (%d:%d) ===\n", eeprom_size, eeprom_bank_size);
  int x=0;
  int xBit=0;
  int charsCount=0;
  
  for(int words=0;words<(int)(eeprom_size/eeprom_bank_size);words+=1)
  {
    digitalWrite(CHIP_SEL, HIGH);
    sendSer(0b110, 8); //sending READ instruction 
    sendSer(words, eeprom_address_width_bits); //sending Address 
  
    //dummy zero that is sent at start of read
    if(needDummyZero)
    {
      digitalWrite(CLOCK,HIGH);
      digitalWrite(CLOCK,LOW);
    }

    for(int i=0;i<eeprom_bank_size;i++)
    {
      char c = digitalRead(DATA_IN);
      
      x <<= 1;
      x |= (c==LOW ? 0 : 1);
      xBit++;
      if(xBit == 4)
      {      
        if(x<10)
        {
          char c='0' + x;
          Serial.print(c);
        }
        else
        {
          char c='a' + (x-10);
          Serial.print(c);
        }
        
        charsCount++;
        if(!(charsCount%4))
        {
          Serial.print(" ");
        }
  
        if(charsCount >= 32){
           Serial.print("\n");
           charsCount = 0;
        }
        
        xBit = 0;
        x = 0;
      }
      
      digitalWrite(CLOCK,HIGH); 
      digitalWrite(CLOCK,LOW); 
      
 //     delay(1);
    }
    
    digitalWrite(CHIP_SEL,LOW);
  }
  Serial.print("\n===END DUMP===\n");
}

/**
 * write EEPROM dump from writeDump to chip
 */
void writeEeprom()
{
  printf("===WRITING (%d:%d) ===\n", eeprom_size, eeprom_bank_size);
  
  // remove spare data
  String dump = String(writeDump);
  dump.replace(" ","");
  dump.replace("\n","");
  dump.replace("\r","");
  dump.toLowerCase();
  
  // check dump length
  if(dump.length() * 4 != eeprom_size)
  {
    printf("\n===WRITING ERROR: dump length is not [%d], but [%d] ===\n", eeprom_size, dump.length() * 4);
    return;
  }
  
  Serial.print("DUMP:\n");
  for(int i=0;i<dump.length();i+=1)
  {
    if(!(i%4 )) { Serial.print(' '); }
    if(!(i%32)) { Serial.print('\n'); }
    Serial.print( dump[i] );
  }
  Serial.print("\n===\n\n");
    
  // prepare to write, enable writing (EWEN instruction )
  digitalWrite(CHIP_SEL, HIGH);
  sendSer(0b100,3);
  sendSer(0b11000000,8); 
  digitalWrite(CHIP_SEL,LOW);
  delay(5);
  
  digitalWrite(CHIP_SEL, HIGH);
  sendSer(0b100,3);
  sendSer(0b10000000,8); 
  digitalWrite(CHIP_SEL,LOW);
  delay(5);
  
//  eeprom_bank_size = 16;

  int x=0;
  int xBit=0;
  int charsCount=0;
  for(int words=0;words<(int)(eeprom_size/eeprom_bank_size);words+=1)
  {
    String  wordStr = dump.substring(words*(eeprom_bank_size/4), 
                                     words*(eeprom_bank_size/4)+(eeprom_bank_size/4));
    unsigned int writeWord = hexToDec(wordStr);
    printf("%03d -> %s => ", words, wordStr.c_str());
    
    digitalWrite(CHIP_SEL, HIGH);
    sendSer(0b101, 3); //sending WRITE instruction 
    sendSer(words, eeprom_address_width_bits); //sending Address 
    sendSer(writeWord, eeprom_bank_size);
    if(useWriteTail)
    {
      sendSer(0, eeprom_bank_size);
    }
    delay(3);
    digitalWrite(CHIP_SEL, LOW);
    
    Serial.print("ok. \n");
  }
  
  // write down, disable writing (EWDS instruction )
  digitalWrite(CHIP_SEL, HIGH);
  sendSer(0b100,3);
  sendSer(0b0000000,8);
  digitalWrite(CHIP_SEL,LOW);
  delay(1);
  
  Serial.print("\n===END WRITING===\n");
}

/**
 * main program loop 
 */
void loop(){
  digitalWrite(13,1);
  delay(1000);
  digitalWrite(13,0);
//  readEeprom();
  
  if(Serial.available() > 0) 
  {
    String s = Serial.readString();
    s.toLowerCase();
    
    if(s=="write" || s=="w")
    {
      Serial.print("\nwriting...\n");
      writeEeprom();
    }
    else if(s=="read" || s=="r")
    {
      Serial.print("\nreading...\n");
      readEeprom();
    }
    else
    {
      Serial.print("\nUnknown command [");
      Serial.print(s);
      Serial.print("]\n");
      Serial.print("Help: \n"
        "write  - write dump\n"
      );
    }
  }
  
}

// === end ===

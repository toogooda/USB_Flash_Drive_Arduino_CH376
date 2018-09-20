#include "USB_CH376s.h"

USB_CH376s usb;

#include <SoftwareSerial.h>
SoftwareSerial Serial3(10,11);

char myDate[12];
char myTime[10];

void setup() {
  Serial.begin(38400);
  Serial3.begin(9600);
  usb.begin(Serial3);
}

void printResult(boolean res) {
  if (res == true)
    Serial.println("OK");
  else Serial.println("false");
}

void loop() {
  if (Serial.available()) {
    char computerByte = Serial.read();
    if (computerByte == '1') {           //1
      Serial.println();
      Serial.println("Testing the connection to the module CH376s");
      printResult(usb.checkConnection(0x01));
    }
    if (computerByte == '2') {           //2
      Serial.println();
      Serial.println("Create and write to file: LOG.TXT");
      usb.writeFile("LOG.TXT", "FoxiK");
    }
    if (computerByte == '3') {           //4
      Serial.println();
      Serial.println("Read file: LOG.TXT");

        int i = usb.getFileSize();
        usb.fileClose(0x00);
        char data[i];
        if(usb.readFile("LOG.TXT", data, 0, 0x40)){//(FileName, data returned, start at pointer, number bytes)
          Serial.print(data);
          Serial.println();
        }
        else
          Serial.println("LOG.TXT no longer exists");
    }
    if (computerByte == '4') {           //3
      Serial.println();
      Serial.println("Add to file: LOG.TXT");
      usb.appendFile("LOG.TXT", " - ARDUINO");
    }
    if (computerByte == '5') {           //5
      Serial.println();
      Serial.println("Delete file: LOG.TXT");
      usb.fileDelete("LOG.TXT");
    }
    if (computerByte == '6') {           //6
      Serial.println();
      Serial.println("Get Creation Date: LOG.TXT");
      if (usb.getFileDate("LOG.TXT", CREATE, myDate, myTime)){
        Serial.println(myDate);
        Serial.println(myTime);
      }
    }
    if (computerByte == '7') {           //7
      Serial.println();
      Serial.println("Set Creation Date: LOG.TXT");
      sprintf(myDate,"09/07/2018");
      sprintf(myTime,"01:02:03");
      if(usb.setFileDate("LOG.TXT", CREATE, myDate, myTime)){
        usb.getFileDate("LOG.TXT", CREATE, myDate, myTime);
        Serial.println(myDate);
        Serial.println(myTime);
      }
      else
        Serial.println("LOG.TXT doesn't exist...");
    }
    if (computerByte == '8') {           //8
      Serial.println();
      Serial.println("Rename LOG.TXT to LOG1.TXT");
      if(usb.renameFile("LOG.TXT","LOG1.TXT",1)){ //and trigger to delete the new file before renaming the old file.
                                                  //use a 0 to abort the renaming if the new file name exists
        Serial.println("and change the file Modified date...");
        sprintf(myDate,"09/07/2018");
        sprintf(myTime,"02:04:06");
        usb.setFileDate("LOG1.TXT", MODIFY, myDate, myTime);
        usb.getFileDate("LOG.TXT", MODIFY, myDate, myTime);
        Serial.println(myDate);
        Serial.println(myTime);
        
      }
    }
    if (computerByte == '9') {           //9
      Serial.println();
      Serial.println("Read 99 bytes from Big File skipping first 5 characters...");
      char bigFile[101];
        if(usb.readFile("TEST.TXT", bigFile, 5, 99)){//(FileName, data returned, start at pointer, number bytes)
          Serial.print(bigFile);
          Serial.println();
        }
    }    
    if (computerByte == '0') {           //0
      Serial.println();
      Serial.println("Read file: TEST.TXT");
        if(usb.fileDump("TEST.TXT", Serial)){//(FileName, data returned, start at pointer, number bytes)
          Serial.println();
        }
        else
          Serial.println("TEST.TXT no longer exists");
    }      
  
  }

  if (Serial3.available()) {
    Serial.println();
    Serial.print("CH376S code:");
    Serial.println(Serial3.read(), HEX);
  }
}

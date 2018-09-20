#ifndef usb_h
#define usb_h
#include "Wire.h"
#include "Arduino.h"
#define MODIFY  0
#define CREATE  1

class USB_CH376s
{
  public:
    
    USB_CH376s();
    void begin(Stream &serial);
    boolean checkConnection(byte);
    byte set_USB_Mode (byte);
    void resetALL();
    byte readFile(String, char*, unsigned long, byte);
    byte writeFile(String, String);
    byte appendFile(String, String);
    void setFileName(String);
    boolean diskConnectionStatus();
    byte USBdiskMount();
    byte fileOpen();
    boolean setByteRead(byte);
    unsigned long getFileSize();
    void fileRead(char*, byte);
    byte fileWrite(String);
    boolean continueRead();
    boolean fileCreate();
    byte fileDelete(String);
    void filePointer(unsigned long);
    byte fileClose(byte);
    boolean waitForResponse();
    boolean result = false;

    byte getResponseFromUSB();
    byte renameFile(String, String, byte);
	byte setFileDate(String, byte, char*, char*);
	byte getFileDate(String, byte, char*, char*);
	byte getDirInfo(String, char*);
	boolean fileExists(String);
	byte fileDump(String, Stream &serial);
  private:
	Stream* _serial;  
	Stream* _serial1;
	byte readData0(char*);
	byte readData0Mem();
	byte wrOfsData(String, byte, byte);
    byte wrOfsByte(byte*, byte, byte);	
	byte dirSave();
	byte infoRead();
};

#endif

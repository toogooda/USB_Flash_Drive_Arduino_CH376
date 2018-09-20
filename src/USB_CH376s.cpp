#include "USB_CH376s.h"


USB_CH376s::USB_CH376s() {
}

void USB_CH376s::begin(Stream &serial)
{
  _serial = &serial;
}


boolean USB_CH376s::checkConnection(byte value) {
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x06);
  _serial->write(value);
  if (waitForResponse()){
    if (getResponseFromUSB() == (255 - value))
      result = true;
    else result = false;
  }
  return result;
}

byte USB_CH376s::set_USB_Mode (byte value) {
byte USB_Byte;

  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x15);
  _serial->write(value);
  delay(20);
  if (_serial->available()) {
    USB_Byte = _serial->read();
    if (USB_Byte == 0x51) 
      USB_Byte = _serial->read();

  }  
  delay(20);
  return(USB_Byte);
}

void USB_CH376s::resetALL() {
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x05);
  delay(200);
}

byte USB_CH376s::readFile(String fileName, char *data,unsigned long point, byte numBytes) {
  //resetALL();
  //set_USB_Mode(0x06);
  diskConnectionStatus();
  USBdiskMount();
  setFileName(fileName);
  if (!fileOpen())
		return(0);
  filePointer(point);
  fileRead(data, numBytes);
  fileClose(0x00);
  return(1);
}

byte USB_CH376s::writeFile(String fileName, String data) {
  //resetALL();
  //set_USB_Mode(0x06);
  diskConnectionStatus();
  USBdiskMount();
  setFileName(fileName);
  if (fileCreate()) {
    fileWrite(data);
  } else {
    //Serial.println("File not create...");
	return(0);
  }
  return(fileClose(0x01));
  
}

byte USB_CH376s::appendFile(String fileName, String data) {
  //resetALL();
  //set_USB_Mode(0x06);
  if (diskConnectionStatus()){
	USBdiskMount();
	setFileName(fileName);
	if (!fileOpen())
		return(0xff);
	filePointer(0xffffffff);
	byte numB = fileWrite(data);
	fileClose(0x01);
	if (numB < data.length()){
		setFileName(fileName);
		fileOpen();
		filePointer(0xffffffff);
		data = data.substring(numB);
		byte numB = fileWrite(data);
		fileClose(0x01);
		return(numB);
	}
	return(numB);
  }
  return(0);
}




//fileExist=====================================================================================
boolean USB_CH376s::fileExists(String fileName){
  //resetALL();                     //Reset the module
  set_USB_Mode(0x06);             //Set to USB Mode
  if(!diskConnectionStatus())         //Check that communication with the USB device is possible
    return(false);
  USBdiskMount();                 //Prepare the USB for reading/writing - you need to mount the USB disk for proper read/write operations.
  setFileName(fileName);          //Set File name
  if (!fileOpen())
    return(false);                    //Open the file for reading
  fileClose(0x00);                //Close the file
  return(true);
}


byte USB_CH376s::getDirInfo(String fileName, char *charray){

  if(diskConnectionStatus()){         //Check that communication with the USB device is possible
	  USBdiskMount();                 //Prepare the USB for reading/writing - you need to mount the USB disk for proper read/write operations.
	  setFileName(fileName);          //Set File name
	  if (!fileOpen())
		  return(0);                     //Open the file for reading
	  infoRead();
	  readData0(charray);
	  fileClose(0);
	  return(1);
  }
  return(0);
}
byte USB_CH376s::renameFile(String oldName, String newName, byte delNewFirst){

	  boolean fE = fileExists(newName);
	  boolean ofE = fileExists(oldName);
	  if (!ofE){
		return(0x8);//return code old file name doesn't exist
	  }
	  if(fE && delNewFirst == 1){
		fileDelete(newName);//delete the new file name 
	  }

	  if(fE && delNewFirst == 0){
		return(0x10);//return code file new file exists
	  }
	  //resetALL();                     //Reset the module
	  //set_USB_Mode(0x06);             //Set to USB Mode
	  if(!diskConnectionStatus()){//Check that communication with the USB device is possible
		return(0);
	  }	  
	  USBdiskMount();                 //Prepare the USB for reading/writing - you need to mount the USB disk for proper read/write operations.
	  setFileName(oldName);          //Set File name
	  if (!fileOpen()){
		return(0x11);//return code oldName does not exist
	  }
	  infoRead();
	  readData0Mem();
	  infoRead();
	  int j = newName.indexOf(".");
	  if (j <= 0){
		return(0x9);//return code bad file name
	  }
	  String rpl = "";
	  for (int ji = j; ji <= 7; ji++)
		rpl += " ";
	  newName.replace(".",rpl);
	  //Serial.print(newName);
	  //Serial.println(":");
	  wrOfsData(newName,0,11);
	  dirSave();
	  fileClose(0);
	return(1);
}

/*	
Byte Offset:  Length: Desc
0x            8       Short File name (pad with spaces)
0x8           3       File Extension
0xB           1       File Attribute

0xE           2       Create Time 15-11(hours) 10-5(minutes) 4-0(seconds)
0x10          2       Create Date 15-9 (year 0 = 1980, 119 = 2099 up to 127 = 2107) 8-5 (month) 4-0 (day)

0x16          2       Last Modfied Time
0x18          2       Last Modfied Date	
*/


//00001 000010 00011 01:02:03  :) it worked....


byte USB_CH376s::setFileDate(String fileName, byte mdCr, char *dt, char *tm){
char buff[34];	
unsigned int ttm = 0,ddt = 0;
unsigned int tmp;

	if(!getDirInfo(fileName, buff))
		return(0);
		

	tmp = (tm[0] - 0x30) * 10 + (tm[1] - 0x30);
	if (tmp > 23) tmp = 0;
	ttm  = tmp << 11;
    tmp = (tm[3] - 0x30) * 10 + (tm[4] - 0x30);
	if (tmp > 59) tmp = 0;
	ttm |= tmp << 5;
	tmp = (tm[6] - 0x30) * 10 + (tm[7] - 0x30);
	if (tmp > 59) tmp = 0;
	ttm |= tmp;


	tmp = (dt[6] - 0x30) * 1000 + (dt[7] - 0x30) * 100 + (dt[8] - 0x30) * 10 + (dt[9] - 0x30);
	if (tmp > 1980 && tmp <= 2107) tmp -= 1980;
	else tmp = 20;//else make year 2000;
	ddt = tmp << 9;
	tmp = (dt[0] - 0x30) * 10 + (dt[1] - 0x30);
	if (tmp > 12) tmp = 1;
	ddt |= tmp << 5;
    tmp = (dt[3] - 0x30) * 10 + (dt[4] - 0x30);
	if (tmp > 31) tmp = 1;
	ddt |= tmp;

	setFileName(fileName);          //Set File name
	if (!fileOpen()){
		return(0x11);//return code oldName does not exist
	}
	infoRead();
	readData0Mem();
	infoRead();
	
	byte newTime[5];
	newTime[0] = lowByte(ttm);
	newTime[1] = highByte(ttm);
	newTime[2] = lowByte(ddt);
	newTime[3] = highByte(ddt);
	
	if (mdCr == CREATE)	
		wrOfsByte(newTime,0xE,4);
	if (mdCr == MODIFY)	
		wrOfsByte(newTime,0x16,4);
	dirSave();
	fileClose(0);
	
	return(1);
}


byte USB_CH376s::getFileDate(String fileName, byte mdCr, char *dt, char *tm){
char buff[34];
unsigned int ttm,ddt;
unsigned int hr,min,sec,month,day;
unsigned short year;

	if(!getDirInfo(fileName, buff))
		return(0);
		
	if (mdCr == MODIFY){
		ttm = (unsigned int)buff[0x17 + 1] << 8;
		ttm += buff[0x16 + 1];
		ddt = (unsigned int)buff[0x19 + 1] << 8;
		ddt += buff[0x18 + 1];  
	}
	if (mdCr == CREATE){
		ttm = (unsigned int)buff[0xF + 1] << 8;
		ttm += buff[0xE + 1];
		ddt = (unsigned int)buff[0x11 + 1] << 8;
		ddt += buff[0x10 + 1];  
	}
	
	
	hr = ttm >> 11;
	ttm &= 0x7ff;
	min = ttm >> 5;
	sec = ttm & 0x3f;

	year = ddt >> 9;
	ddt &= 0x1ff;
	month = ddt >> 5;
	day = ddt & 0x1f;

	
	if (hr > 23) hr = 0;
	if (min > 59) min = 0;
	if (sec > 59) sec = 0;
	
	if (month > 12) month = 1;
	if (day > 31) day = 1;
	if (year > 127) year = 0;

	year += 1980;
	
	sprintf(dt, "%02u/%02u/%04u",month,day,year);
	sprintf(tm, "%02u:%02u:%02u",hr,min,sec);
	
	
	return(1);
}


void USB_CH376s::setFileName(String fileName) {
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x2F);
  _serial->write(0x2F);
  _serial->print(fileName);
  _serial->write((byte)0x00);
  delay(20);
}

boolean USB_CH376s::diskConnectionStatus() {
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x30);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x14) {
		return(true);
    } else {
      //Serial.print(">Connected to USB failed....");
	  return(false);
    }
  }
  return(false);
}

byte USB_CH376s::USBdiskMount() {
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x31);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x14) {
		return(0x14);
    } else {
      //Serial.print(">USB mounting error...");
	  return(0);
    }
  }
  return(0);
}

byte USB_CH376s::fileOpen() {
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x32);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x14) {
		return(0x14);
    } else {
      //Serial.print(">Open the file failed...");
	  return(0);
    }
  }
  return(1);
}

boolean USB_CH376s::setByteRead(byte numBytes) {
  boolean bytesToRead = false;
  int timeCounter = 0;
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x3A);
  _serial->write((byte)numBytes);
  _serial->write((byte)0x00);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x1D) {
      bytesToRead = true;
    }
  }
  return (bytesToRead);
}

unsigned long USB_CH376s::getFileSize() {
  unsigned long fileSize = 0;
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x0C);
  _serial->write(0x68);
  delay(100);
  if (_serial->available()) {
    fileSize = fileSize + _serial->read();
  }
  if (_serial->available()) {
    fileSize = fileSize + (_serial->read() * 255);
  }
  if (_serial->available()) {
    fileSize = fileSize + (_serial->read() * 255 * 255);
  }
  if (_serial->available()) {
    fileSize = fileSize + (_serial->read() * 255 * 255 * 255);
  }
  delay(10);
  return (fileSize);
}



byte USB_CH376s::fileDump(String fileName, Stream &serial) {
  //Serial.print("READ: ");
  _serial1 = &serial;
  byte firstByte = 0x00;
  set_USB_Mode(0x06);             //Set to USB Mode
  
  diskConnectionStatus();
  USBdiskMount();
  setFileName(fileName);
  if (!fileOpen())
		return(0);
  filePointer(0);
  
  if(!diskConnectionStatus())         //Check that communication with the USB device is possible
    return(0);
  _serial->setTimeout(500);
  while (setByteRead(0x40)) {
    _serial->write(0x57);
    _serial->write(0xAB);
    _serial->write(0x27);
    if (waitForResponse()) {
      firstByte = _serial->read();
	  delayMicroseconds(1200);
      while (_serial->available()) {
        _serial1->write(_serial->read());
		delayMicroseconds(1200);
      }
    }
    if (!continueRead()) {
      break;
    }
  }
  fileClose(0);
  return(1);

}

void USB_CH376s::fileRead(char *data, byte numBytes) {
  //Serial.print("READ: ");
  byte firstByte = 0x00;
  //if (numBytes > 0x40) numBytes = 0x40;
  int i = 0;
  _serial->setTimeout(500);
  //int nL = numBytes / 0x40;
  
  //for (int nI = 0; nI < nL; nI++){
	  while (setByteRead(0x40)) {
		_serial->write(0x57);
		_serial->write(0xAB);
		_serial->write(0x27);
		if (waitForResponse()) {
		  firstByte = _serial->read();
		  delayMicroseconds(1200);
		  while (_serial->available()) {
			if (i < numBytes) 
				data[i++] = _serial->read();
			else 
				break;
			delayMicroseconds(1200);
		  }
		  
		}
		if (!continueRead()) {
		  break;
		}
	  }
  //}
  	  while (_serial->available()) {
 		_serial->read();
		delayMicroseconds(1200);
	  }
}

byte USB_CH376s::fileWrite(String data) {
  //Serial.print("WRITE: ");
  unsigned int numBytesWr = 0;
  byte dataLength = (byte) data.length();
  //Serial.println(data);
  delay(100);
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x3C);
  _serial->write((byte) dataLength);
  _serial->write((byte) 0x00);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x1E) {
      _serial->write(0x57);
      _serial->write(0xAB);
      _serial->write(0x2D);
      _serial->print(data);
	  
      if (waitForResponse()) {
		  numBytesWr = _serial->read();
      }
      //Serial.print("WRITE OK... (");
      //Serial.print(_serial->read(), HEX);
      //Serial.print(",");
      _serial->write(0x57);
      _serial->write(0xAB);
      _serial->write(0x3D);
      if (waitForResponse()) {
      }
	  _serial->read();
      //Serial.print(_serial->read(), HEX);
      //Serial.println(")");
	  return(numBytesWr);
    }
  }
  return(0);
}

boolean USB_CH376s::continueRead() {
  boolean readAgain = false;
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x3B);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x14) {
      readAgain = true;
    }
  }
  return (readAgain);
}

boolean USB_CH376s::fileCreate() {
  boolean createdFile = false;
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x34);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x14) {
      createdFile = true;
    }
  }
  return (createdFile);
}

byte USB_CH376s::fileDelete(String fileName) {
  setFileName(fileName);
  delay(20);
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x35);
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x14) {
      return(1);
    }
  }
  return(0);
}

void USB_CH376s::filePointer(unsigned long filePoint) {
  unsigned short a,b;
  a = filePoint>>16;
  b = filePoint & 0xffff;
  byte c,d,e,f;
  c = lowByte(a);
  d = highByte(a);
  e = lowByte(b);
  f = highByte(b);
  
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x39);
  
    _serial->write(e);
    _serial->write(f);
    _serial->write(c);
    _serial->write(d);
  
  
/*   if (fileBeginning) {
    _serial->write((byte)0x00);
    _serial->write((byte)0x00);
    _serial->write((byte)0x00);
    _serial->write((byte)0x00);
  } else {
    _serial->write((byte)0xFF);
    _serial->write((byte)0xFF);
    _serial->write((byte)0xFF);
    _serial->write((byte)0xFF);
  } */
  
  if (waitForResponse()) {
    if (getResponseFromUSB() == 0x14) {
    }
  }
}

byte USB_CH376s::fileClose(byte closeCmd) {
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x36);
  _serial->write((byte)closeCmd);
  if (waitForResponse()) {
    byte resp = getResponseFromUSB();
    if (resp == 0x14) {
		return(resp);
    } else {
      //Serial.print(">Failed to close file. Error code:");
      //Serial.println(resp, HEX);
	  return(0);
    }
  }
  return(0);
}


byte USB_CH376s::readData0Mem(){
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x27);
  if(waitForResponse()){
    while(_serial->available()){
      _serial->read();
      delayMicroseconds(1400);
    }  
  }
  return(1);
}


byte USB_CH376s::readData0(char *buff){
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x27);
  int pnt = 0;
  if(waitForResponse()){
    while(_serial->available()){
      buff[pnt] = _serial->read();
      //Serial.print(buff[pnt],HEX);
      //Serial.print(',');
      delayMicroseconds(1400);
      pnt++;
    }  
  }
 
  return(1);
}



byte USB_CH376s::wrOfsByte(byte *info, byte st, byte en){
  //Serial.print("write Ofs Data: ");
  //Serial.println(info);
  
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x2E);//write ofs Data
  _serial->write(st);
  _serial->write(en);
  for (int b = 0; b < en; b++)
	_serial->write(info[b]);

  return(1);
}

byte USB_CH376s::wrOfsData(String info, byte st, byte en){
  //Serial.print("write Ofs Data: ");
  //Serial.println(info);
  
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x2E);//write ofs Data
  _serial->write(st);
  _serial->write(en);
  _serial->print(info);

  return(1);
}

byte USB_CH376s::dirSave(){
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x38);//save dir info
  if(waitForResponse()){       //wait for a response from the CH376S. If CH376S responds, it will be true. If it times out, it will be false.
    if(getResponseFromUSB()==0x14){               //CH376S will send 0x14 if this command was successful
       //Serial.println(">Dir Save OK");
	   return(1);
    } else {
      //Serial.print(">Dir Save - FAILED.");
      return(0);
    }
  }
  return(0xff);
}

byte USB_CH376s::infoRead(){
  //Serial.print("Info Read: ");
  _serial->write(0x57);
  _serial->write(0xAB);
  _serial->write(0x37);
  _serial->write(0xFF);
  if(waitForResponse()){       //wait for a response from the CH376S. If CH376S responds, it will be true. If it times out, it will be false.
    if(getResponseFromUSB()==0x14){               //CH376S will send 0x14 if this command was successful
       //Serial.println(">Info Read OK");
	   return(0x14);
    } else {
      //Serial.print(">Info Read - FAILED.");
      return(0);
    }
  }
  return(0);
}




boolean USB_CH376s::waitForResponse() {
  boolean bytesAvailable = true;
  int counter = 0;
  while (!_serial->available()) {   //wait for CH376S to verify command
    delay(1);
    counter++;
    if (counter > 2000) {
      //Serial.print("TimeOut waiting for response: Error while: ");
      //Serial.println(errorMsg);
      bytesAvailable = false;
      break;
    }
  }
  delay(1);
  return (bytesAvailable);
}

byte USB_CH376s::getResponseFromUSB() {
  byte response = byte(0x00);
  if (_serial->available()) {
    response = _serial->read();
  }
  return (response);
}




//dumpFile=====================================================================================
//just dump contents to the serial port
/* byte dumpFile(String fileName){
  resetALL();                     //Reset the module
  set_USB_Mode(0x06);             //Set to USB Mode
  if(!diskConnectionStatus())         //Check that communication with the USB device is possible
    return(0);
  USBdiskMount();                 //Prepare the USB for reading/writing - you need to mount the USB disk for proper read/write operations.
  setFileName(fileName);          //Set File name
  if (!fileOpen())
    return(0);                    //Open the file for reading
  unsigned long fs = getFileSize();         //Get the size of the file
  fileRead();                     //***** Send the command to read the file ***
  fileClose(0x00);                //Close the file
  return(1);
}
 */




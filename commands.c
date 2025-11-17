#include "uart.h"
#include "LPC23xx.h"
#include "types.h"
#include "commands.h"
#include "string.h"
#include "main.h"
#include "timer.h"
#include "i2c_rtc.h"
#include "usbhost_lpc2468.h"
#include "disk_port.h"
#include "httpd.h"
#include "uip_1.h"
#include "gpio.h"
#include "i2s.h"
//#include "memory.h"
//#include "keyboard.h"
#include "fskDecode.h"
#include "pinConfig.h"
#include "adc.h"
#include "dtmf.h"
#include "logs.h"
#include "Program.h"



 volatile uint8_t flashReadRunning = 0;             
 const uint32_t flashReadEndOffset = 15;      
 volatile uint8_t flashResponseReady = 0;
 volatile uint32_t sector = 1;  // start sector
 volatile uint32_t offset = 0;
 volatile uint8_t FlashWaitingResponse = 0;
 volatile uint32_t eventCount = 0;
 volatile uint32_t sectorCount = 0;
 volatile uint32_t FlashWaitCount = 0;
 volatile uint8_t FlashWaitRetry = 0;



/*
#define IAP_LOCATION 0x7ffffff1

unsigned long IAP_command[5];
unsigned long IAP_result[3];

typedef void (*IAP)(unsigned int [],unsigned int[]);

IAP IAP_Entry1 = (IAP)IAP_LOCATION;
*/

const char msg0[] = "\r\nWelcome to Pulse Communication Systems ";
const char msg1[] = "\r\nBrown fox jumps over the lazy dog ";
const char msg2[] = "\r\nIndia is great contry with great ideas ";
const char msg3[] = "\r\nI have great family of four bros ";

#define TxTCR_COUNTER_ENABLE (1<<0)

char Line[80];
short Fptr;

extern volatile BYTE Test_int;
uip_ipaddr_t UDPaddr;

unsigned char SineWave[8] = {0xD5,0x83,0xB5,0x83,0x55,0x03,0x35,0x03};

volatile  unsigned char  *Buffer;                /* Buffer used by application                             */

volatile char debugSelect = 0;

//*****************************************************************************
//
//! Command line function callback type.
//
//*****************************************************************************
typedef int (*pfnCmdLine)(int argc, char *argv[]);

//*****************************************************************************
//
//! Structure for an entry in the command list table.
//
//*****************************************************************************
typedef struct
{
    //
    //! A pointer to a string containing the name of the command.
    //
    const char *pcCmd;

    //
    //! A function pointer to the implementation of the command.
    //
    pfnCmdLine pfnCmd;

    //
    //! A pointer to a string of brief help text for the command.
    //
    const char *pcHelp;
}
tCmdLineEntry;

/********************* Auto Flash code *****************************************/

void CMD_aFlash(void){
	flashReadRunning = 1;
	sectorCount = 0;
	FlashWaitingResponse =  0;
	eventCount = 0;
	sector = 1;
	offset = 0;
	eventCount = 0;
}

void CMD_autoFlashLastSect(void){
	flashReadRunning = 1;
	FlashWaitingResponse = 0;
	offset = 0;
	sector = flashSector;
	sectorCount--;
	eventCount = 0;
}


 void FlashReadServer() {
	if (!flashReadRunning) {
         return; 
     }
	 if (FlashWaitingResponse){
		if(FlashWaitCount >= 2000){
			FlashWaitCount = 0;
			if(FlashWaitRetry++ <= 3)
				ReadFlashData(sector, offset, 1);	
			else{
				puts0("\r\n Flash Read Timeout");
			 	flashReadRunning = 0;	
			}		
		}
		return;
	 }	
	 if(FlashWaitCount < 15)return; 	
	 ReadFlashData(sector, offset, 1); 	 
	 FlashWaitingResponse = 1;
	 FlashWaitRetry = 0;
	 FlashWaitCount = 0;
	 return;
	 
 }

/************************************************/



int Cmd_ReadWriteFlash(int argc, char **argv)
{
unsigned char buf[64];
int i, j, k;
int size = 32, tsize;
	 
	 if(argc >= 2)
		k = atoi(argv[1]);
	 else
		 k = 0;
	 if(argc >= 3)
		j = atoi(argv[2]);
	 else
		 j = 0;
	 if(k == 9)
		flashHeaderUpdate(1, 0);
	 else
		ReadFlashData( k, j, 1);
	 return 1;
	 
	 if(isLogBufferAvailableToWrite(size)){
		tsize = getLogBufferToWrite(buf);
		for(i=0;i<tsize;i++){
			printhex(buf[i]);
		}
	 }
	 else{
		 puts0(" -- "); 
		 return 0;	 
	 }
	 
     return 1;
}
int Cmd_Info(int argc, char **argv)
{
	SendSystemInfo();
	return 1;
}

int Cmd_BackupResponse(int argc, char **argv){
	int k = atoi(argv[1]);
	puts0("\r\n Sending Backup Response ");
	SendBackupResponse((unsigned char)(k & 0xff));
	return 1;
}	
//*****************************************************************************
//
// The buffer that holds the command line.
//
//*****************************************************************************

int Cmd_Read(int argc, char **argv)
{
int i, j, k, m, n;
signed long rc;
char buf[80];

	 k = atoi(argv[1]);
	 j = atoi(argv[2]);
	 if(j==0)j=1;
	 if(HDDState != 2){
         puts0("\r\nSorry! HDD not connected\r\n");
		 return 1;
	 }
     puts0("\r\nReading Block "); 
	 PrintDecimal(k);
	 PrintDecimal(j);
	 puts0("\r\n");
     Buffer = 0x7FD01000;
     memset(Buffer,0,512);
	 memset(buf,0,80);
	 for(i=0;i<j;i++){
         puts0("\r\n\nBLOCK "); PrintDecimal(i+k); 
         rc = MS_BulkRecv(i+k, 1, Buffer);
		 printhexL(rc);
		 puts0("\r\n");
		 if(rc < 0){
             puts0("\r\nRead Failed ");
			 return 1;
		 }
		 for(n=0;n<16;n++){
		     printhexA(n*16); putch(' ');
             for(m=0;m<16;m++)printhex(Buffer[m+(n*16)]); puts0("\r\n");
			 TimeDelay(50);
			 if(getch()==0x1b)break;
         }
	 }
     return 1;
}

int Cmd_MREAD(int argc, char **argv)
{
int i, j, k, m, n;
signed long rc;
char buf[80];

	 k = atoi(argv[1]);
	 j = atoi(argv[2]);
	 if(j==0)j=1;
     puts0("\r\nReading Block "); 
	 PrintDecimal(k);
	 PrintDecimal(j);
	 puts0("\r\n");
     Buffer = 0x0007C000;
//     Buffer = EMC;
	 memset(buf,0,80);
	 for(i=0;i<j;i++){
         puts0("\r\n\nBLOCK "); PrintDecimal(i+k); 
		 puts0("\r\n\n");
		 for(n=0;n<20;n++){
		     printhexA(n*16); putch(' ');
             for(m=0;m<16;m++)printhex(Buffer[m+(n*16)]); puts0("\r\n");
			 TimeDelay(50);
			 if(getch()==0x1b)break;
         }
	 }
     return 1;
}

int Cmd_Write(int argc, char *argv[])
{
int i, j, k;
char *ptr;
	 i = atoi(argv[1]);
	 j = atoi(argv[2]);
	 k = atoi(argv[3]);
	 if(i==0){
        puts0("\r\nCan't write at zero block");
		return 1;
	 }
	 if(HDDState != 2){
         puts0("\r\nSorry! HDD not connected\r\n");
		 return 1;
	 }
	 switch(j){
	     case 0:
		        ptr = &msg0; 
				break;
         case 1:
		        ptr = &msg1;
				break; 
		 case 2:
		        ptr = &msg2;
				break; 
		 case 3:
		        ptr = &msg3; 
		        break;
     }
	 Buffer = 0x7FD01000;
     memset(Buffer,0,512);
	 memcpy(Buffer,ptr,strlen(ptr)); 
	 memcpy(Buffer+128,ptr,strlen(ptr)); 
	 memcpy(Buffer+256,ptr,strlen(ptr)); 
	 memcpy(Buffer+384,ptr,strlen(ptr)); 

     puts0("\r\nWriting from Block "); 
	 PrintDecimal(i);
	 puts0("msg no ");
	 PrintDecimal(j);
	 puts0(" blocks ");
	 PrintDecimal(k);
	 puts0("\r\n");
	 for(j=0;j<k;j++){
         MS_BulkSend(i+j, 1, Buffer);
	 }
	 return 1;
}

int Cmd_MWRITE(int argc, char *argv[])
{
int i, j, k;
char *ptr;
/*	 i = atoi(argv[1]);
	 j = atoi(argv[2]);
	 k = atoi(argv[3]);
	 switch(j){
	     case 0:
		        ptr = &msg0; 
				break;
         case 1:
		        ptr = &msg1;
				break; 
		 case 2:
		        ptr = &msg2;
				break; 
		 case 3:
		        ptr = &msg3; 
		        break;
     }
	 Buffer = EMC;
     memset(Buffer,0,512);
	 memcpy(Buffer,ptr,strlen(ptr)); 
	 memcpy(Buffer+128,ptr,strlen(ptr)); 
	 memcpy(Buffer+256,ptr,strlen(ptr)); 
	 memcpy(Buffer+384,ptr,strlen(ptr)); 

     puts0("\r\nWriting from Block "); 
	 PrintDecimal(i);
	 puts0("msg no ");
	 PrintDecimal(j);
	 puts0(" blocks ");
	 PrintDecimal(k);
	 puts0("\r\n");*/
	 return 1;
}

int Cmd_SetTime(int argc, char **argv)
{
BYTE c;
    if(argc==1){
	    puts0("\r\nTime is : ");
		c = (CurDateTime[2] >> 4) & 0x03;
		putch(c+'0');
		c = CurDateTime[2] & 0x0F;
		putch(c+'0');
		putch(':');
		c = (CurDateTime[1] >> 4) & 0x07;
		putch(c+'0');
		c = CurDateTime[1] & 0x0F;
		putch(c+'0');
		putch(':');
		c = (CurDateTime[0] >> 4) & 0x07;
		putch(c+'0');
		c = CurDateTime[0] & 0x0F;
		putch(c+'0');
		puts0("\r\n");
		return;
	} 
    puts0("\r\nsetting Time:");
    NewSetTime(argv[1]);
    return 1;
}

int Cmd_SetDate(int argc, char **argv)
{
BYTE c;
    if(argc==1){
	    puts0("\r\nDate is : ");
		c = (CurDateTime[4] >> 4) & 0x03;
		putch(c+'0');
		c = CurDateTime[4] & 0x0F;
		putch(c+'0');
		putch('/');
		c = (CurDateTime[5] >> 4) & 0x01;
		putch(c+'0');
		c = CurDateTime[5] & 0x0F;
		putch(c+'0');
		putch('/');
		c = (CurDateTime[6] >> 4) & 0x0F;
		putch(c+'0');
		c = CurDateTime[6] & 0x0F;
		putch(c+'0');
		puts0("\r\n");
		return;
	} 
    puts0("\r\nsetting Date:");
    NewSetDate(argv[1]);
    return 1;
}

int Cmd_Records(int argc, char **argv)
{
	if(debugSelect != 5)debugSelect = 5;
	else debugSelect = 0;
	return 1;
long ul;
    puts0("\r\nTodays Records are:");
	Today.DaySerial = atoi(argv[1]);
    PrintDecimal(Today.DaySerial);
	puts0("\r\n");
    return 1;
}

int Cmd_FCBS(int argc, char **argv)
{
    puts0("\r\nStarting FCB is:");
	Today.StartRecords = atoi(argv[1]);
	PrintDecimal(Today.StartRecords);
	puts0("\r\n");
    return 1;
}

int Cmd_Update(int argc, char **argv)
{
int i;
    puts0("\r\nUpdating the Date Table:");
//    i = atoi(argv[1]); printhex(i); 
//    UpdateDateEntry(i);
    return 1;
}

int Cmd_Show(int argc, char **argv)
{
int i,j;

//    if(argc!=4){
//	    puts0("Cmd Error: Syntax: show [date/fcb/cls] from size\r\n");
//		return 1;
//	} 

if (strcmp(argv[1], "mac") == 0)
	{
		puts0("\r\nMAC - ");
		printhex(DeviceStatus.MAC_ID[4]);
		puts0(":");
		printhex(DeviceStatus.MAC_ID[5]);
		return 1;
	}
	if (strcmp(argv[1], "ip") == 0)
	{
		puts0("\r\IP - ");
		PrintDecimal(DeviceStatus.IP_Address[0]);
		PrintDecimal(DeviceStatus.IP_Address[1]);
		PrintDecimal(DeviceStatus.IP_Address[2]);
		PrintDecimal(DeviceStatus.IP_Address[3]);
		puts0(" / ");
		PrintDecimal(DeviceStatus.IP_MASK[0]);
		PrintDecimal(DeviceStatus.IP_MASK[1]);
		PrintDecimal(DeviceStatus.IP_MASK[2]);
		PrintDecimal(DeviceStatus.IP_MASK[3]);
		puts0(" / ");
		PrintDecimal(DeviceStatus.IP_Gateway[0]);
		PrintDecimal(DeviceStatus.IP_Gateway[1]);
		PrintDecimal(DeviceStatus.IP_Gateway[2]);
		PrintDecimal(DeviceStatus.IP_Gateway[3]);
		return 1;
	}
    if(strcmp(argv[1],"date")==0){
        ShowDateEntry(&argv[0]);
        return 1;
	}
    if(strcmp(argv[1],"fcb")==0){
        ShowFCBEntry(&argv[0]);
        return 1;
	}
    if(strcmp(argv[1],"cls")==0){
	   for(i=0;i<20;i++){
		  printhexA(i*16); putch(' ');
          for(j=0;j<16;j++)printhex(CurCLS[j+(i*16)]); puts0("\r\n");
		  TimeDelay(50);
       }
	}
	if(strcmp(argv[1],"rtc")==0){
        printd(CurDateTime[0]);
		printd(CurDateTime[1]);
		printd(CurDateTime[2]);
		printd(CurDateTime[3]);
		printd(CurDateTime[4]);
		printd(CurDateTime[5]);
		printd(CurDateTime[6]);
		printd(CurDateTime[7]);
		printd(CurDateTime[8]);
        return 1;
	}
    if(strcmp(argv[1],"BAT")==0){
	    puts0("\r\nDate Index "); printd(BAT.DateIndex);
		puts0("\r\nFCB "); printd(BAT.NextRecord);
		puts0("\r\nCLS "); printd(BAT.CurCLS);
		puts0("\r\nNext CLS "); printd(BAT.NextFreeCluster);
		puts0("\r\n");
		TimeDelay(50);
	}
    if(strcmp(argv[1],"lines")==0){
        puts0("\r\nchtype ");
		printhex(CH[0].chtype);
		printhex(CH[1].chtype);
		printhex(CH[2].chtype);
		printhex(CH[3].chtype);
        puts0("\r\ncstat ");
		printhex(CH[0].cstat);
		printhex(CH[1].cstat);
		printhex(CH[2].cstat);
		printhex(CH[3].cstat);
		puts0("\r\nLEDS RELAYS ");
        printhex(LEDS);
		printhex(RELAYS);
        puts0("\r\n ");
        return 1;
	}
    return 1;
}

int Cmd_Manual(int argc, char **argv)
{
int i,j;
    if(argc!=3){
	    puts0("Cmd Error: Syntax: manual channel[0/1/2/3] on/off[1/0]\r\n");
		return 1;
	}
	i = atoi(argv[1]);
	j = atoi(argv[2]);
	if(j==1)j = 11; else j=13;
	CH[i%4].cstat = j; 
    return 1;
}

int Cmd_Blank(int argc, char **argv)
{
    puts0("\r\nDate table Blanking\r\n");
    BlankDateEntry();
	BAT.DateIndex = 1;
	BAT.NextRecord = 0;
	Today.DaySerial=0;
	Today.StartRecords=0;
	UpdateDateEntry();
	WriteBatMemory();
    return 1;
}

int Cmd_Format(int argc, char **argv)
{
    puts0("\r\nFormating\r\n");
    Format_Disk();
    return 1;
}

int Cmd_GetSize(int argc, char **argv)
{
    puts0("\r\HDD Size\r\n");
	printhexL(BAT.TotalLeft);
	printhexL(BAT.TotalDiskSize);
    return 1;
}

int Cmd_Option(int argc, char **argv)
{
char c;
    puts0("\r\noption ");
	printd(BAT.DateIndex);
	printd(BAT.NextRecord);
	return 1; 
}

int Cmd_Play(int argc, char **argv)
{
int i;
struct FCB fcb;

    i = atoi(argv[1]);
	if(i==0)return 1;
	if(i>BAT.NextRecord){
        //memcpy(&LCD_Display[16],"Sorry! No Record",16);
        return 1;
	}
    // memcpy(&LCD_Display[16],"Playing ",8);
    // sPrintRecord(i, &LCD_Display[24]);
    GetFCBEntry(i-1, &fcb);
    A_PlayNextCluster = fcb.StartCluster;
	A_PlaySize = fcb.FileSize;
	PlayState = 1;
    return 1;
}

int Cmd_PlayON(int argc, char **argv)
{
int i, j;
    for(i=0;i<4096;i+=8){
	    for(j=0;j<8;j++){
            A_PlayData[i+j] = SineWave[j];
        }
	} 
    for(i=0;i<4096;i+=8){
	    for(j=0;j<8;j++){
            A_PlayData[i+j+4096] = 0x80;
        }
	} 
    puts0("\r\nPlayON is ");
	i = atoi(argv[1]);
	putch(i+'0');
    T1TCR = i;             // Timer1 Enable
    return 1; 
}

int Cmd_Quit(int argc, char **argv)
{
    puts0("\r\nQuit TCP Connection "); 
    httpd_init();
    return 1;
}

int Cmd_ADC(int argc, char **argv)
{
    puts0("\rADC ");
	printd(CH[0].Voltage);
	printd(CH[1].Voltage);
	printd(CH[2].Voltage);
	printd(CH[3].Voltage);
	if(debugSelect != 1)debugSelect = 1;
	else debugSelect = 0;
    return 1;
}

int Cmd_VOX(int argc, char **argv)
{
    puts0("\VOX ");
	printd(STP.Vox[0]);
	printd(STP.Vox[1]);
	printd(STP.Vox[2]);
	printd(STP.Vox[3]);
	if(debugSelect != 6)debugSelect = 6;
	else debugSelect = 0;
    return 1;
}


int Cmd_RING(int argc, char **argv)
{
	int i;
    puts0("\rRING "); 
	for(i=0; i<4;i++){
		printd(CH[i].RingDetect);
		printd(STP.RingDetect[i]);
		printd(CH[i].MissedCall);
		puts0(" - "); 
	}
	if(debugSelect != 2)debugSelect = 2;
	else debugSelect = 0;
    return 1;
}

int Cmd_RINGPULSE(int argc, char **argv)
{
    puts0("\rRING PULSE "); 
	printd(RingPuls[0]);
	printd(RingPuls[1]);
	printd(RingPuls[2]);
	printd(RingPuls[3]);
	if(debugSelect != 3)debugSelect = 3;
	else debugSelect = 0;
    return 1;
}

int Cmd_RELAY(int argc, char **argv)
{
	fskDecodeStart(0);
	fskDecodeStart(1);
	fskDecodeStart(2);
	puts0("\r\n Sending Network Info");
	SendNetworkInfo();
	
	return 1;
	
	if(debugSelect != 4)debugSelect = 4;
	else debugSelect = 0;
	char no, action;
    no = atoi(argv[1]);
	action = atoi(argv[2]);
    puts0("\r\nRelay ");
	printhex(no);
	puts0(" is SET\r\n"); 
	RELAYS &= ~(1 << no);
	if (action){
	    RELAYS |= (1 << no);
	}
    SetRelay();
    return 1;
}

int Cmd_STAT(int argc, char **argv)
{
char a;
    puts0("\r\nCHNL ");
	a = atoi(argv[1]);
	printhex(a);
	puts0("TYPE ");
    printhex(CH[a].chtype);	 
	puts0("VOL ");
	printd(CH[a].Voltage);
	puts0("ON-OF ");
	printd(CH[a].onhook);
	printd(CH[a].ofhook);
	puts0("VOL-THD ");
	printd(CH[a].VOX_THD);
	puts0("DLY ");
	printd(CH[a].VOX_DLY);
    return 1;
}

int Cmd_TEST(int argc, char **argv)
{
char a;
    puts0("\r\nTEST ");
	a = atoi(argv[1]);
	puts0("I2SSYNC="); printhex(I2SSYNC);
	puts0("Testbyte="); printhex(TestByte);
//	puts0("Playstate=");printhex(PlayState);
 	puts0("I2SCTR=");PrintDecimal(I2SCounterA);
 	puts0("I2SCTR=");PrintDecimal(I2SCounterC[0]);
	puts0("I2SCTR=");PrintDecimal(I2SCounterB[0]);
	I2SCounterA=0;
	I2SCounterC[0]=0;
	I2SCounterB[0]=0;
	puts0("\r\n");

    return 1;
}

int Cmd_Display(int argc, char **argv)
{
int a;
BYTE m,n,*ptr;

    /*puts0("\r\nDisplay\r\n");
	a = atoi(argv[1]);
	ptr = I2SBufPtr[a];
    for(n=0;n<20;n++){
	   printhexA(n*16); putch(' ');
       for(m=0;m<16;m++)printhex(ptr[m+(n*16)]); puts0("\r\n");
	   TimeDelay(50);
	   if(getch()==0x1b)break;
    }*/
	puts0("\r\n");
    return 1;
}

int Cmd_Init(int argc, char **argv)
{
int i;
    puts0("\r\nFCB  ");
	i = atoi(argv[1]);
	PrintDecimal(i); 
    ControlState=0;
	puts0("\r\n");
    return 1;
}

int Cmd_Speaker(int argc, char **argv)
{
int i;
    puts0("\r\nSpeaker  ");
	i = atoi(argv[1]);
	PrintDecimal(i); 
	if(i==0){
	    RELAYS &= 0xEF; 
		SetRelay();
		SpeakerOn=0;
    }
	else
	{
	    RELAYS |= 0x10; 
		SetRelay();
        SpeakerOn = 1;
	} 	 
	puts0("\r\n");
    return 1;
}

int Cmd_Fsk(int argc, char **argv)
{
int i;
    puts0("\r\n FSK  ");
	i = atoi(argv[1]);
	PrintDecimal(i); 
	
	if(i < 4){

		puts0("CN:");
		PrintDecimal(fData[i].countMax);

		puts0("ST:");
		PrintDecimal(fData[i].state);

		puts0("PC:");
		PrintDecimal(fData[i].PrintCount);
	
		puts0("TC:");
		PrintDecimal(fData[i].fTelCount);
	
		if(fData[i].fTelCount > 0){
			puts0("Tel:");
			for(int j=0;j<fData[i].fTelCount;j++)
				PrintDecimal(fData[i].fTelNo[j]);	
		}
	}
	else if(i < 8){
		i -= 4;
		fskDecodeStart(i);
	}
	 	 
	puts0("\r\n");
    return 1;
}

int Cmd_Boot(int argc, char **argv)
{
int i;
    puts0("\r\n BOOT  ");
	 Programming();	 
	puts0("\r\n");
    return 1;
}



int Cmd_IP(int argc, char **argv)
{
int i;
    puts0("\r\nUDP  ");
    uip_ipaddr_copy(&UDPaddr, uip_draddr);
	printhex(UDPaddr[0]);
	printhex(UDPaddr[1]);
	printhex(UDPaddr[2]);
	printhex(UDPaddr[3]);
	puts0("\r\n");
    return 1;
}

int Cmd_RESET(int argc, char **argv)
{
int i;
void (*user_code_entry)(void);   

    puts0("\r\nRebooting\r\n");
	TimeDelay(20);
    user_code_entry = (void (*)(void))0x0000;
    user_code_entry();   
    return 1;
}

int Cmd_WDT(int argc, char **argv)
{
int i;
    puts0("\r\nWDT  ");
	i = atoi(argv[1]);
	if(i==0){
	   WDT_Disable();
	   puts0("Disabled\r\n");
	}
	else
	{
	   WDTInit();
	   puts0("Enabled\r\n");
	}
    return 1;
}

int Cmd_RtcInit(int argc, char **argv)
{
int i;
	puts0("\r\n RTC ");
	//printhex(RTCState);
	//return 0;
	
	unsigned char status = DS3231_ReadStatus();
	printhex(status);
	return 0;
	
	if (status != 0xFF) {
		puts0("DS3231 is working "); //, status);
		if (status & 0x80)
			puts0("Oscillator has stopped (OSF = 1)\r\n");
		else
			puts0("Oscillator running (OSF = 0)\r\n");
	} else {
		puts0("DS3231 not responding!\r\n");
	}
	printhex(status);
/*
    puts0("\r\nRTC  ");
	i = atoi(argv[1]);
	if(i==0){
	   RtcOn = 0;
	   puts0("Disabled\r\n");
	}
	else
	{
	   RTCInit();
	   RtcOn = 1;
	   puts0("Enabled\r\n");
	}
	
	*/
    return 1;
}

/*
int Cmd_ID(int argc, char **argv)
{
int i;
    puts0("\r\nBoard ID  ");
	IAP_command[0] = 54;
	IAP_Entry(IAP_command, IAP_result);
	printhex(i);
	i=IAP_result[0];
	printhexL(i);
	i=IAP_result[1];
	printhexL(i);
	puts0("\r\n");
    return 1;
}
*/

int Cmd_Config(int argc, char **argv)
{
long rc;
    rc = atoi(argv[1]);
    puts0("\r\nEnumDev\r\n");
	switch(rc){
	    case 0:
//	           rc = Host_EnumDev();
			   break;
		case 1:
//		       rc = EnumTest();
			   break; 
		case 2:
//               rc = HOST_GET_DESCRIPTOR(USB_DESCRIPTOR_TYPE_INTERFACE, 0, TDBuffer, 9);
//			   puts0("\rInterce-0 "); printhex(rc); puts0(" ---- ");
//			   for(rc=0;rc<9;rc++)printhex(TDBuffer[rc]);
               break;
		case 3:
//               rc = HOST_GET_DESCRIPTOR(USB_DESCRIPTOR_TYPE_CONFIGURATION, 0, TDBuffer, 9);
//			   puts0("\rConfiguration-0 "); printhex(rc); puts0(" ---- ");
//			   for(rc=0;rc<9;rc++)printhex(TDBuffer[rc]);
               break;
        case 4: 
//               rc = HOST_GET_DESCRIPTOR(USB_DESCRIPTOR_TYPE_CONFIGURATION, 1, TDBuffer, 9);
//			   puts0("\rConfiguration-1 "); printhex(rc); puts0(" ---- ");
//			   for(rc=0;rc<9;rc++)printhex(TDBuffer[rc]);
               break;
        case 5:
//               rc = HOST_GET_DESCRIPTOR(USB_DESCRIPTOR_TYPE_ENDPOINT, 1, TDBuffer, 7);
//			   puts0("\rEndpoint-0 "); printhex(rc); puts0(" ---- ");
//			   for(rc=0;rc<7;rc++)printhex(TDBuffer[rc]);
               break;
        case 6:
//               rc = HOST_GET_DESCRIPTOR(USB_DESCRIPTOR_TYPE_ENDPOINT, 2, TDBuffer, 7);
//			   puts0("\rEndpoint-1 "); printhex(rc); puts0(" ---- ");
//			   for(rc=0;rc<7;rc++)printhex(TDBuffer[rc]);
               break;
        case 7:
//               rc = HOST_GET_DESCRIPTOR(USB_DESCRIPTOR_TYPE_ENDPOINT, 2, TDBuffer, 7);
//			   puts0("\rEndpoint-2 "); printhex(rc); puts0(" ---- ");
//			   for(rc=0;rc<7;rc++)printhex(TDBuffer[rc]);
               break;
        case 8:
//		       puts0("\rSet Interface 0");
//               rc = USBH_SET_INTERFACE(0,0);             /* Select device Interface     */
//               if (rc != OK) {
//			       puts0("\r\nInterface not Set\r\n");
//				   HDDDelay=400;
//				   EnumState=0;
//               }
			   break;
		case 9:
//		       puts0("\rSet Interface 1");
//               rc = USBH_SET_INTERFACE(1,0);             /* Select device Interface     */
//               if (rc != OK) {
//			       puts0("\r\nInterface not Set\r\n");
//				   HDDDelay=400;
//				   EnumState=0;
//               }
			   break;
        case 10:
//		       puts0("\r\nSet Interface 2");
//               rc = USBH_SET_INTERFACE(0,1);             /* Select device Interface     */
//               if (rc != OK) {
//			       puts0("\r\nInterface not Set\r\n");
//				   HDDDelay=400;
//				   EnumState=0;
//               }
			   break;
		case 11:
		       puts0("\r\nWrite Block ");
			   rc = atoi(argv[2]); PrintDecimal(rc);
			   Buffer = 0x7FD01000;
               memset(Buffer,0,512);
	           memcpy(Buffer,msg0,strlen(msg0)); 
	           memcpy(Buffer+128,msg0,strlen(msg0)); 
	           memcpy(Buffer+256,msg0,strlen(msg0)); 
	           memcpy(Buffer+384,msg0,strlen(msg0)); 
			   MS_BulkSend(rc,1,Buffer);
			   break;
        case 12:
		       puts0("\r\nRead Block ");
			   rc = atoi(argv[2]); PrintDecimal(rc);
			   Buffer = 0x7FD01000;
			   MS_BulkRecv(rc,1,Buffer);
			   puts0(&Buffer[0]);
			   puts0(&Buffer[128]);
			   break;
        case 13:
		       puts0("\r\nWrite Block ");
			   rc = atoi(argv[2]); PrintDecimal(rc);
			   Buffer = 0x7FD01000;
               memset(Buffer,0,512);
	           memcpy(Buffer,msg0,strlen(msg0)); 
	           memcpy(Buffer+128,msg0,strlen(msg0)); 
	           memcpy(Buffer+256,msg0,strlen(msg0)); 
	           memcpy(Buffer+384,msg0,strlen(msg0)); 
//			   MS_BulkSend_M(rc,1,Buffer);
			   break;
        case 14:
		       puts0("\r\nRead Block ");
			   rc = atoi(argv[2]); PrintDecimal(rc);
			   Buffer = 0x7FD01000;
//			   MS_BulkRecv_M(rc,1,Buffer);
			   puts0(&Buffer[0]);
			   puts0(&Buffer[128]);
			   break;
		 
		 
	}
    return 1;
}

int Cmd_Hang(int argc, char **argv)
{
int i;
char c;
    i=atoi(argv[1]);
	if(i==0){
       puts0("\r\nHang\r\n");
	   while(1);
    }
	if(i==1){
	   puts0("\r\nWDT Flag is "); 
	   c=WDMOD;
	   printhex(c);
	   puts0("\r\n");
	   return 1;
    }
	if(i==2){
	   puts0("\r\nFlag clearing\r\n");
	   c=0x04;
       WDMOD &= 0xFD;
	}
    return 1;
}

int Cmd_ConnShow(int argc, char **argv)
{
	int ulTemp;
    ulTemp=atoi(argv[1]);
	
	//for(ulTemp = 0; ulTemp < UIP_UDP_CONNS/2; ulTemp++){
		puts0("\r\n -- ");printhex(ulTemp); puts0(" -- ");
		printhex((uip_udp_conns[ulTemp].ripaddr[0] >> 8) & 0xff);
		printhex((uip_udp_conns[ulTemp].ripaddr[0]) & 0xff);
		printhex((uip_udp_conns[ulTemp].ripaddr[1] >> 8) & 0xff);
		printhex(uip_udp_conns[ulTemp].ripaddr[1] & 0xff);
		puts0(" ");
		printhex((uip_udp_conns[ulTemp].lport >> 8) & 0xff);
		printhex((uip_udp_conns[ulTemp].lport) & 0xff);
		puts0(" ");
		printhex((uip_udp_conns[ulTemp].rport >> 8) & 0xff);
		printhex((uip_udp_conns[ulTemp].rport) & 0xff);
	//}
    return 1;
}

int Cmd_DtmfShow(int argc, char **argv)
{
	int ulTemp;
    ulTemp=atoi(argv[1]);
	if(ulTemp >= 4)return 0;
	
	//for(ulTemp = 0; ulTemp < UIP_UDP_CONNS/2; ulTemp++){
		puts0("\r\n DTMF "); printd(ulTemp); puts0(" - ");
		for(int i=0;i<14;i++){
			printd(STP.Telno[ulTemp][i]);
		}
	//}
    return 1;
}


int Cmd_I2SByte(int argc, char **argv)
{
	int ulTemp;
    ulTemp=atoi(argv[1]);
	printhex(RxBuffer[ulTemp][100]); printhex(RxBuffer[ulTemp][101]); printhex(RxBuffer[ulTemp][102]);printhex(RxBuffer[ulTemp][103]);
	printhex(RxBuffer[ulTemp][1100]); printhex(RxBuffer[ulTemp][1101]); printhex(RxBuffer[ulTemp][1102]);printhex(RxBuffer[ulTemp][1103]);
	printhex(RxBuffer[ulTemp][2100]); printhex(RxBuffer[ulTemp][2101]); printhex(RxBuffer[ulTemp][2102]);printhex(RxBuffer[ulTemp][2103]);
	printhex(RxBuffer[ulTemp][3100]); printhex(RxBuffer[ulTemp][3101]); printhex(RxBuffer[ulTemp][3102]);printhex(RxBuffer[ulTemp][3103]);
	printhex(RxBuffer[ulTemp][4100]); printhex(RxBuffer[ulTemp][4101]); printhex(RxBuffer[ulTemp][4102]);printhex(RxBuffer[ulTemp][4103]);
    return 1;


}

int Cmd_Call(int argc, char **argv)
{
	int ulTemp;
    ulTemp=atoi(argv[1]);
	puts0("\r\n Calling...."); printd(ulTemp);
	DTMFToneCmdTemp.value[0] = ulTemp + '0';
	DTMFToneCmdTemp.value[1] = ulTemp + '0';
	DTMFToneCmdTemp.value[2] = ulTemp + '0';
	DTMFToneCmdTemp.value[3] = ulTemp + '0';
	DTMFToneCmdTemp.value[4] = ulTemp + '0';
	DTMFToneCmdTemp.value[5] = '#';
	DTMFToneCmdTemp.value[6] = 0;
	DTMFToneCmdTemp.value[7] = 0;
	DTMFToneCmdTemp.value[8] = 0;
	DTMFToneCmdTemp.state = 0;
	DTMFToneCmdTemp.isAvailable = 1;
    return 1;
}

int Cmd_Log(int argc, char **argv)
{
	int ulTemp, lSize;
    ulTemp=atoi(argv[1]);
	unsigned char buf[24];
	lSize = logPrint(buf, ulTemp);
	if(lSize > 0)puts0("\r\n LOG :: ");
	for(int i=0;i<lSize; i++)printhex(buf[i]);
    return 1;
}

int Cmd_AlarmControl(int argc, char **argv)
{
	int ulTemp;
    ulTemp=atoi(argv[1]);
	puts0("\r\n Alarm "); printd(ulTemp);
	if(ulTemp > 0)
		alarmState = 1;
	else
		alarmState = 0;
	
    return 1;
}

int Cmd_CallAccept(int argc, char **argv)
{
	puts0("\r\n Calling..!!");
	DTMFToneCmdTemp.value[0] = '2';
	DTMFToneCmdTemp.value[1] = '*';
	DTMFToneCmdTemp.value[2] = '22';
	DTMFToneCmdTemp.value[3] = '*';
	DTMFToneCmdTemp.value[4] = '5';
	DTMFToneCmdTemp.value[5] = '*';
	DTMFToneCmdTemp.value[6] = '1';
	DTMFToneCmdTemp.value[7] = '#';
	DTMFToneCmdTemp.value[8] = 0;
	DTMFToneCmdTemp.state = 0;
	DTMFToneCmdTemp.isAvailable = 1;
    return 1;
}

int Cmd_CallPicked(int argc, char **argv)
{
	puts0("\r\n Calling..!!");
	DTMFToneCmdTemp.value[0] = '3';
	DTMFToneCmdTemp.value[1] = '*';
	DTMFToneCmdTemp.value[2] = '22';
	DTMFToneCmdTemp.value[3] = '*';
	DTMFToneCmdTemp.value[4] = '5';
	DTMFToneCmdTemp.value[5] = '*';
	DTMFToneCmdTemp.value[6] = '1';
	DTMFToneCmdTemp.value[7] = '#';
	DTMFToneCmdTemp.value[8] = 0;
	DTMFToneCmdTemp.state = 0;
	DTMFToneCmdTemp.isAvailable = 1;
    return 1;
}


int Cmd_Default(int argc, char **argv)
{
	puts0("\r\n Setting Default!!");
	SetDefault();
    return 1;
}

int Cmd_GetCSQ(int argc, char **argv)
{
	puts0("\r\n CSQ : ");
	if(GET_PORT_PIN(CSQ0_PORT, CSQ0_PIN) > 0)
        puts0("\r\n HIGH");
    else
        puts0("\r\n LOW");
	
	puts0("\r\n CRRTimer : ");
	printd(CRRTimer[0]);
	puts0("\r\n PTTTimer : ");
	printd(PTTTimer[0]);
    return 1;
}

int Cmd_DialSM(int argc, char **argv){
	int id = 84, ch = 4;
	int len = 0;
	
	puts0("\r\n Dial SM ");
	if(argc >= 2)
		id = atoi(argv[1]);
	if(argc >= 3)
		ch = atoi(argv[2]);
	printd(id);
	printd(ch);
	//return 1;
	
	DTMFToneCmdTemp.value[len++] = '0';	
	DTMFToneCmdTemp.value[len++] = '1';	
	DTMFToneCmdTemp.value[len++] = '*';	
	DTMFToneCmdTemp.value[len++] = '8';
	DTMFToneCmdTemp.value[len++] = '2';
	DTMFToneCmdTemp.value[len++] = '*';
	sprintd(id % 100, &DTMFToneCmdTemp.value[len], 2); len+=2;
	DTMFToneCmdTemp.value[len++] = '*';
	DTMFToneCmdTemp.value[len++] = ((ch & 0xf) + 48);
	DTMFToneCmdTemp.value[len++] = '#';
		
	DTMFToneCmdTemp.value[len] = 0;
	DTMFToneCmdTemp.state = 0;
	DTMFToneCmdTemp.isAvailable = 1;	
	return 1;
}





/*
int Cmd_Fwrite1(int argc, char **argv)
{
int i;
unsigned long buf[256];
char *ptr;

    puts0("\r\nPreparing  ");
	IAP_command[0] = 50;
	IAP_command[1] = 26;
	IAP_command[2] = 26;
	Timer_Disable();
	IAP_Entry(IAP_command, IAP_result);
    Timer_Enable(); 
	i = IAP_result[0];
	if(i!=0){
	    puts0("FAIL "); 
		printhex(i);
		i = IAP_result[1];
		printhex(i);
		return(1);
	}
    ptr = msg0;
	for(i=0;i<64;i++){
        buf[i]  = *ptr++;
//		if(*ptr==0)ptr=argv[1];
		buf[i] |= *ptr++ << 8; 
//		if(*ptr==0)ptr=argv[1];
		buf[i] |= *ptr++ << 16; 
//		if(*ptr==0)ptr=argv[1];
		buf[i] |= *ptr++ << 24; 
//		if(*ptr==0)ptr=argv[1];
	} 
	TimeDelay(10);
	puts0("\r\nprogramming ");
	IAP_command[0] = 51;
	IAP_command[1] = 0x0007C000;
	IAP_command[2] = buf;
	IAP_command[3] = 256;
	IAP_command[4] = 12000;
	Timer_Disable();
	IAP_Entry(IAP_command, IAP_result);
	Timer_Enable();
	i = IAP_result[0];
	if(i!=0){
	    puts0("FAIL "); 
		printhex(i);
		i = IAP_result[1];
		printhex(i);
		return(1);
	}
	puts0(" OK\r\n");
    return 1;
}

int Cmd_Erase1(int argc, char **argv)
{
int i;
unsigned long buf[256];
char *ptr;

    puts0("\r\nPreparing  ");
	IAP_command[0] = 50;
	IAP_command[1] = 26;
	IAP_command[2] = 26;

	Timer_Disable();
	IAP_Entry(IAP_command, IAP_result);
	Timer_Enable();

	i = IAP_result[0];
	if(i!=0){
	   puts0("FAIL "); 
	   printhex(i);
	   i = IAP_result[1];
	   printhex(i);
	   return(1);
	}
	puts0("\r\nErasing ");
	TimeDelay(10);
	IAP_command[0] = 52;
	IAP_command[1] = 26;
	IAP_command[2] = 26;
	IAP_command[3] = 12000;

	Timer_Disable();
	IAP_Entry(IAP_command, IAP_result);
	Timer_Enable();

	i = IAP_result[0];
	if(i!=0){
	   puts0("FAIL "); 
	   printhex(i);
	   i = IAP_result[1];
	   printhex(i);
	   return(1);
	}
	puts0(" OK\r\n");
    return 1;
}
*/

int Cmd_AddEntry(int argc, char **argv)
{
int i,j,k;
int type=0;
int from=0;
int size=1;
    if(argc!=4){
	    puts0("Cmd Error: Syntax: add [date/fcb/cls] from size\r\n");
		return 1;
	} 
    from = atoi(argv[2]);
	size = atoi(argv[3]);
	if(size==0)size=1;
	if(strcmp(argv[1],"date")==0)type=1;
	if(strcmp(argv[1],"fcb")==0)type=2;
	if(strcmp(argv[1],"cls")==0)type=3;
	switch(type){
	    case 1:
		      for(i=0;i<size;i++){
			      puts0("\r\nAdding Date ");
				  printd(BAT.DateIndex);
				  Today.DaySerial = from;
                  BAT.NextRecord += from;
				  BAT.NextRecord &= MAX_FCB - 1;
				  BAT.TotalRecords += from;
				  if(BAT.TotalRecords > MAX_FCB){
				     BAT.TotalRecords = MAX_FCB;
                  }
			      k = AddDateEntry();
				  if(k==0)break;
              }
			  break; 
		case 2:
              puts0("\r\nAdding FCB ");
			  i = atoi(argv[2]);
			  j = atoi(argv[3]);
			  for(i=0;i<j;i++){
			     FillFCB(i%4);
			     k = AddFCBEntry((struct FCB *)&CH[i%4].fcb, BAT.NextRecord++); 
				 if(k==0)break;
              }
		      break;
		case 3:
		      break;
    }
	if(k==0){
        puts0(" Failed\r\n");
	}
    return 1;
}

tCmdLineEntry g_sCmdTable[] =
{
    { "help",  (void *)Cmd_help,      " : Display list of commands" },
    { "h",     (void *)Cmd_help,   "    : alias for help" },
    { "?",     (void *)Cmd_help,   "    : alias for help" },
    { "stime", (void *)Cmd_SetTime,"    : Current channel to monitor" },
    { "sdate", (void *)Cmd_SetDate,"    : Current channel to monitor" },
    { "read",  (void *)Cmd_Read,   " Read a Sector"},
    { "write", (void *)Cmd_Write,   " Write a Sector"},
    { "record", (void *)Cmd_Records,   " Set No of Records"},
    { "FCBS", (void *)Cmd_FCBS,   " Starting FCB"},
    { "update", (void *)Cmd_Update,   " Update Date Table"},
    { "show", (void *)Cmd_Show,   " Show Date Table"},
    { "blank", (void *)Cmd_Blank,   " Blank Date Table"},
    { "size", (void *)Cmd_GetSize,   " Get Size of struct"},
    { "add",  (void *)Cmd_AddEntry,   " Add Date Entry"},
    { "format",  (void *)Cmd_Format,   " Disk Format"},
    { "manual",  (void *)Cmd_Manual,   " Manual Mode Entry"},
    { "option",  (void *)Cmd_Option,   " set option"},
    { "play",  (void *)Cmd_Play,   " play a file"},
    { "playon",  (void *)Cmd_PlayON,   " play a file"},
    { "quit",  (void *)Cmd_Quit,   " Quit option"},
    { "init",  (void *)Cmd_Init,   " Uart2 Init"},
    { "adc",  (void *)Cmd_ADC,   " Show ADC"},
	{ "vox",  (void *)Cmd_VOX,   " Show VOX"},
    { "ring",  (void *)Cmd_RING,   " Show Ring Status"},
	{ "pulse",  (void *)Cmd_RINGPULSE,   " Show Ring Pulse Status"},
    { "stat",  (void *)Cmd_STAT,   " Show chnl Status"},
    { "relay",  (void *)Cmd_RELAY,   " Set Relay"},
    { "test",  (void *)Cmd_TEST,   " Testing ON"},
    { "display",  (void *)Cmd_Display,   " Display"},
    { "mread",  (void *)Cmd_MREAD,   " Read from Memory"},
    { "mwrite",  (void *)Cmd_MWRITE,   " Write To Memory"},
    { "ip",  (void *)Cmd_IP,   " Get IP"},
    //{ "speaker",  (void *)Cmd_Speaker,   "Speaker Test"},
	{ "fsk",  (void *)Cmd_Fsk,   "FSK Test"},
    { "wdt",  (void *)Cmd_WDT,   "WDT ON/OFF"},
    { "reset",  (void *)Cmd_RESET,   "I2S Reset"},
	//{ "hang",   (void *)Cmd_Hang, "To test WDT"},
	{ "boot",   (void *)Cmd_Boot, "To Enter Boot Mode"},
	{ "enum",   (void *)Cmd_Config, "Set Config"},
	{ "rtc",   (void *)Cmd_RtcInit, "Rtc Init"},
	{ "conn",   (void *)Cmd_ConnShow, "Connection List"},
	{ "dtmf",   (void *)Cmd_DtmfShow, "Dtmf Print"},
	{ "i2s",   (void *)Cmd_I2SByte, "I2S Test Byte"},
	{ "default",   (void *)Cmd_Default, "Default Settings"},
	{ "csq",   (void *)Cmd_GetCSQ, "CSQ Read"},
	{ "call",   (void *)Cmd_Call, "Call Dtmf"},
	{ "accept",   (void *)Cmd_CallAccept, "Call Accept"},
	{ "pick",   (void *)Cmd_CallPicked, "Call Picked"},
	{ "alarm",   (void *)Cmd_AlarmControl, "Alarm"},
	{ "flash",  (void *)Cmd_ReadWriteFlash,   " Read Flash"},
	{"info", (void *)Cmd_Info, " Send Info"},
	{"log", (void *)Cmd_Log, " Check Log"},
	{"dial", (void *)Cmd_DialSM, " Dial SM"},
	{"backup", (void *)Cmd_BackupResponse, " Backup Response"},
	{"autoflash", (void *)CMD_aFlash, " Auto Flash Test2"},
	{"flashls", (void *)CMD_autoFlashLastSect, " Flash Last Sector(auto)"},

    { 0, 0, 0 }
};

/*
    { "id",  (void *)Cmd_ID,   "IAP Test"},
    { "fwrite",  (void *)Cmd_Fwrite1,   "IAP Write Sector"},
    { "erase",  (void *)Cmd_Erase1,   "IAP Erase Sector"},
*/


int atoi(char *ptr)
{
int no = 0;
   while(*ptr){
      if ((*ptr < '0') || (*ptr > '9'))break;
      no *= 10;
	  no += *ptr++ - '0';
   }
   return no;
}

int atoh(char *ptr)
{
int no = 0;

   if ((*ptr >= 'a') && (*ptr <= 'f')) *ptr &= 0xdf;
   if ((*ptr >= 'A') && (*ptr <= 'F')) *ptr -= 7;
   no = *ptr++ - '0';

   if ((*ptr == 0) || (*ptr == 0x0d))return no;

   no *= 16;
   if ((*ptr >= 'a') && (*ptr <= 'f')) *ptr &= 0xdf;
   if ((*ptr >= 'A') && (*ptr <= 'F')) *ptr -= 7;
   no += *ptr - '0';

   return no;
}

int Cmd_help()
{
   puts0("\r\nYes I want to help you\r\n");
   return 0;
}

int Cmd_channel(int argc, char **argv)
{
   return 0;
}

int ProcessCommand()
{
static char *argv[10];
char *pcChar;
int argc;
int bFindArg = 1;
tCmdLineEntry *pCmdEntry;

    //
    // Initialize the argument counter, and point to the beginning of the
    // command line string.
    //
    argc = 0;
    pcChar = Line;

    //
    // Advance through the command line until a zero character is found.
    //
    while(*pcChar)
    {
        //
        // If there is a space, then replace it with a zero, and set the flag
        // to search for the next argument.
        //
        if(*pcChar == ' ')
        {
            *pcChar = 0;
            bFindArg = 1;
        }

        //
        // Otherwise it is not a space, so it must be a character that is part
        // of an argument.
        //
        else
        {
            //
            // If bFindArg is set, then that means we are looking for the start
            // of the next argument.
            //
            if(bFindArg)
            {
                //
                // As long as the maximum number of arguments has not been
                // reached, then save the pointer to the start of this new arg
                // in the argv array, and increment the count of args, argc.
                //
                if(argc < 5)
                {
                    argv[argc] = pcChar;
                    argc++;
                    bFindArg = 0;
                }

                //
                // The maximum number of arguments has been reached so return
                // the error.
                //
                else
                {
                    return 0;
                }
            }
        }
        //
        // Advance to the next character in the command line.
        //
        pcChar++;
    }

    //
    // If one or more arguments was found, then process the command.
    //
    if(argc)
    {
        //
        // Start at the beginning of the command table, to look for a matching
        // command.
        //
        pCmdEntry = &g_sCmdTable[0];
        //
        // Search through the command table until a null command string is
        // found, which marks the end of the table.
        //
        while(pCmdEntry->pcCmd)
        {
            //
            // If this command entry command string matches argv[0], then call
            // the function for this command, passing the command line
            // arguments.
            //
            if(!strcmp(argv[0], pCmdEntry->pcCmd))
            {
                return(pCmdEntry->pfnCmd(argc, argv));
            }

            //
            // Not found, so advance to the next entry.
            //
            pCmdEntry++;
        }
    }
    return(0);
}

void Commands_Init(void)
{
   memset(Line,0,80);
   Fptr = 0;
}

void CommandServer()
{
unsigned char c;
int i;
   c = getch();
   if (c == 0)return;
   if(c==0xff)return;
   putch(c);
   if(c == 0x0d){
	   puts0("\r\n");
	   if(Fptr==0)return;
       i=ProcessCommand();
	   if(i==0)puts0("\r\nwhat..\r\n");
	   Commands_Init(); 
	   return;
   }
   if(c==0x1b){
       puts0("\r\nEscaped\r\n");
       Commands_Init();
	   PlayDone |= 2;
	   return;
   }
   Line[Fptr++]=c;
}

void CommandServerTest()
{
unsigned char c;
int i;
   c = getch2();
   if (c == 0)return;
   if(c==0xff)return;
   putch2(c);
   
   switch(c){
        case 'Q':
            SET_PORT_PIN(TOE0_PORT, TOE0_PIN);
            break;
        case 'W':
            SET_PORT_PIN(TOE1_PORT, TOE1_PIN);
            break;
        case 'E':
            SET_PORT_PIN(TOE2_PORT, TOE2_PIN);
            break;
        case 'R':
            SET_PORT_PIN(TOE3_PORT, TOE3_PIN);
            break;
        case 'T':
            SET_PORT_PIN(PTT0_PORT, PTT0_PIN);
            break;
        case 'Y':
            SET_PORT_PIN(R0_CS0_PORT, R0_CS0_PIN);
            break;
        case 'U':
            SET_PORT_PIN(R0_CS1_PORT, R0_CS1_PIN);
            break;
        case 'I':
            SET_PORT_PIN(R0_CS2_PORT, R0_CS2_PIN);
            break;
        case 'O':
            SET_PORT_PIN(R0_CS3_PORT, R0_CS3_PIN);
            break;
        case 'P':
            //SET_PORT_PIN(PTT1_PORT, PTT1_PIN);
            break;
        case 'A':
            //SET_PORT_PIN(PTT2_PORT, PTT2_PIN);
            break;
        case 'S':
            SET_PORT_PIN(SPK1_PORT, SPK1_PIN);
            break;
        case 'D':
            //SET_PORT_PIN(SPK2_PORT, SPK2_PIN);
            break;
        case 'F':
            //SET_PORT_PIN(MIC1_LED_PORT, MIC1_LED_PIN);
            break;
        
        case 'q':
            CLR_PORT_PIN(TOE0_PORT, TOE0_PIN);
            break;
        case 'w':
            CLR_PORT_PIN(TOE1_PORT, TOE1_PIN);
            break;
        case 'e':
            CLR_PORT_PIN(TOE2_PORT, TOE2_PIN);
            break;
        case 'r':
            CLR_PORT_PIN(TOE3_PORT, TOE3_PIN);
            break;
        case 't':
            CLR_PORT_PIN(PTT0_PORT, PTT0_PIN);
            break;
        case 'y':
            CLR_PORT_PIN(R0_CS0_PORT, R0_CS0_PIN);
            break;
        case 'u':
            CLR_PORT_PIN(R0_CS1_PORT, R0_CS1_PIN);
            break;
        case 'i':
            CLR_PORT_PIN(R0_CS2_PORT, R0_CS2_PIN);
            break;
        case 'o':
            CLR_PORT_PIN(R0_CS3_PORT, R0_CS3_PIN);
            break;
        case 'p':
            //CLR_PORT_PIN(PTT1_PORT, PTT1_PIN);
            break;
        case 'a':
            //CLR_PORT_PIN(PTT2_PORT, PTT2_PIN);
            break;
        case 's':
            CLR_PORT_PIN(SPK1_PORT, SPK1_PIN);
            break;
        case 'd':
            //CLR_PORT_PIN(SPK2_PORT, SPK2_PIN);
            break;
        case 'f':
            //CLR_PORT_PIN(MIC1_LED_PORT, MIC1_LED_PIN);
            break;
			
        case '1':
            if(GET_PORT_PIN(DTMF0_PORT, DTMF0_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '2':
            if(GET_PORT_PIN(DTMF1_PORT, DTMF1_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '3':
            if(GET_PORT_PIN(DTMF2_PORT, DTMF2_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '4':
            if(GET_PORT_PIN(DTMF3_PORT, DTMF3_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '5':
            if(GET_PORT_PIN(STD0_PORT, STD0_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '6':
            if(GET_PORT_PIN(STD1_PORT, STD1_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '7':
            if(GET_PORT_PIN(STD2_PORT, STD2_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '8':
            if(GET_PORT_PIN(STD3_PORT, STD3_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '9':
            if(GET_PORT_PIN(I2S_0_PORT, I2S_0_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '0':
            if(GET_PORT_PIN(I2S_1_PORT, I2S_1_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '-':
            if(GET_PORT_PIN(I2S_2_PORT, I2S_2_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '=':
            if(GET_PORT_PIN(I2S_3_PORT, I2S_3_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case '[':
            if(GET_PORT_PIN(CSQ0_PORT, CSQ0_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;
        case ']':
            /*if(GET_PORT_PIN(CRADLE0_PORT, CRADLE0_PIN) > 0)
                puts0("\r\n HIGH");
            else
                puts0("\r\n LOW");*/
            break;	

        case ',':
            if(GET_PORT_PIN(RING0_PORT, RING0_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;	
        case '.':
            if(GET_PORT_PIN(RING1_PORT, RING1_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;	
        case '/':
            if(GET_PORT_PIN(RING2_PORT, RING2_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;	
        case '\\':
            if(GET_PORT_PIN(RING3_PORT, RING3_PIN) > 0)
                puts2("\r\n HIGH");
            else
                puts2("\r\n LOW");
            break;	

		case 'z':
            //printhexL(ADC0Read(0));
            break;				
		case 'x':
            //printhexL(ADC0Read(1));
            break;				
		case 'c':
            //printhexL(ADC0Read(2));
            break;				
		case 'v':
            //printhexL(ADC0Read(3));
            break;				
			
		case 'm':
			//printhexL(TestByte);
			break;
    }

	
}


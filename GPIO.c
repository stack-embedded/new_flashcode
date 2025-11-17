/******************************************************************************
 *
 * $RCSfile: $
 * $Revision: $
 *
 * This module provides interface routines to the LPC ARM UARTs.
 * Copyright 2004, R O SoftWare
 * No guarantees, warrantees, or promises, implied or otherwise.
 * May be used for hobby or commercial purposes provided copyright
 * notice remains intact.
 *
 *****************************************************************************/
#include <limits.h>
#include "type.h"
#include "LPC23xx.h"
#include "gpio.h"
#include "main.h"
#include "PortPin.h"
#include <stdint.h>
#include "uart.h"


volatile uint16_t r;





	const char* err1[32];   // Store only failed pin labels
	uint8_t count1 = 0;
	uint8_t errIndex1 = 0;
	
	const char* err0[32];   // Store only failed pin labels
	uint8_t count0 = 0;
	uint8_t errIndex0 = 0;
unsigned int x0,x1;


// ***** Global variable

void GPIOInit(void)
{
int i;
    SCS |= 0x01;

    // PINSEL some pins because they are either always high or low
     PINSEL0 |= (0U << 28); PINSEL0 |= (0U << 29);
    //PINSEL0 &=~ (3U << 28);
     PINSEL1 &=~ (3U << 20); // pins P0.14 cand P0.26
    PINSEL2 &=~ ((3U << 12) | (3U << 14) | (3U < 30)); // Pins P1.6, P1.7 and P1.15
    PINSEL4 &=~ ((3U << 16) | (3U << 26)); // Pins P2.8 and P2.13
    PINSEL7 &=~ (3U << 0); // Pin P3.16

    FIO0DIR |= (1U << 14);
	FIO0DIR |= 0xF84623F0;   // pins 4,5,6,7,8,9,14,17,22,27,28,29,30,31 AS OUTPUT
    // FIO0DIR &=~(1U << 26); // Making P0.26 as input because it is connected to Inv O/P of P1.5
	FIO1DIR |= 0x003FBFFF;   // PINS 0-13 & 15-21
    FIO1DIR |= (1U << 5);
	FIO2DIR |= 0x078C0D73;   // Pins 0,1, 4, 5, 6, 8, 10, 11, 13, 18, 19, 23, 24, 26
    FIO2DIR |= (1U << 13);
	FIO3DIR |= 0x07810000;   // Pins 16, 23, 24,25, 26 
	FIO4DIR |= (1U<<26);     // Pin 26 only 	
}



// --- Optional: Label for each pin (used in output messages)
// -- Pins  P0.26, P1.6, P1.7, P1.15, P2.8, P3.16 are removed (but they are still output {FIODIR})
/*****************************CODE START******************************** */
const char *OutPins1[] = {
    // Port 0
    "P0.4","P0.5","P0.6","P0.7","P0.8","P0.9",
    "P0.14","P0.17","P0.22","P0.27","P0.28","P0.29","P0.30","P0.31",
    // Port 1
    "P1.0","P1.1","P1.2","P1.3","P1.4","P1.5","P1.8","P1.9",
    "P1.10","P1.11","P1.12","P1.13","P1.16","P1.17","P1.18","P1.19",
    "P1.20","P1.21",
    // Port 2
    "P2.0","P2.1","P2.4","P2.5","P2.6","P2.8","P2.10","P2.11","P2.13",
    "P2.18","P2.19","P2.23","P2.24","P2.26",
    // Port 3
    "P3.23","P3.24","P3.25","P3.26",
    // Port 4
    "P4.26"
};

const uint8_t portMap[] = {
    // P0
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // P1
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    // P2
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    // P3
    3,3,3,3,
    // P4
    4
};

const uint8_t pinMap[] = {
    // P0
    4,5,6,7,8,9,14,17,22,27,28,29,30,31,
    // P1
    0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21,
    // P2
    0,1,4,5,6,8,10,11,13,18,19,23,24,26,
    // P3
    23,24,25,26,
    // P4
    26
};
// void testShortPins(void){
// outControl(2,19,0);

// }
/** --- GPIO short test function*/
void testShortPins(void)
{
    puts0("\n--- Starting GPIO Short Test ---\n\r");
	puts0(".........Connected : P0.4 --> P0.7 ..............\n\n\r");
	puts0(".........Connected : P0.5 --> P0.8 ..............\n\n\r");
    int totalPins = sizeof(OutPins1)/sizeof(OutPins1[0]);
    uint8_t errorCount = 0;

    // Set all pins LOW
    puts0("Making all pins LOW first......\n\r");
    for (int i = 0; i < totalPins; i++){
        outControl(portMap[i], pinMap[i], 0);  }

    TimeDelay(200);
    puts0("All pins set to LOW.\n\r");
    for (int i = 0; i < totalPins; i++) {
        if (FIOPIN(portMap[i], pinMap[i]) != 0) {
            puts0(OutPins1[i]); puts0(" did NOT go LOW_______\n\n\r");
            errorCount++;
        }
    }

    // Toggle each pin HIGH, check short, then back LOW
    for (int i = 0; i < totalPins; i++) {
		if((i == 0)|| (i == 1)){
		outControl(portMap[i], pinMap[i], 1);
		outControl(portMap[i+3], pinMap[i+3], 1);
        TimeDelay(90);
		}
		else if((i == 3) || (i == 4)){
		outControl(portMap[i], pinMap[i], 1);
		outControl(portMap[i-3], pinMap[i-3], 1);
        TimeDelay(90);
		}
        // else if(i==19){
        //     outControl(portMap[i], pinMap[i], 1);
        //     r = FIOPIN(0,26);
        //     puts0("P1.5 val--> 1         ");
        //     puts0("ReadPin (P0.26) Val-->");
        //     PrintDecimal(r);
        //     puts0("\n\r");
        // }
		else{
        outControl(portMap[i], pinMap[i], 1);
        TimeDelay(90);}

        if (FIOPIN(portMap[i], pinMap[i]) != 1) {
            puts0(OutPins1[i]); puts0(" did NOT go HIGH\n\r");
            errorCount++;
        }
		else{
        // Check for short: all other pins must remain LOW
        for (int j = 0; j < totalPins; j++) {
            if (j == i) continue;
			if(((i == 0)&&(j == 3))||(i==1)&&(j==4)||((i == 3)&&(j == 0))||(i==4)&&(j==1))
			continue;
            if (FIOPIN(portMap[j], pinMap[j]) != 0) {
                puts0("SHORT DETECTED: "); puts0(OutPins1[i]);
                puts0(" --> "); puts0(OutPins1[j]); puts0("\n\r");
                errorCount++;
            }
        }}

        // Set pin back LOW
		if((i==0)||(i==1)){
        outControl(portMap[i], pinMap[i], 0);
		outControl(portMap[i+3], pinMap[i+3], 0);
        TimeDelay(90);}
		else if((i==3)||(i==4)){
		outControl(portMap[i], pinMap[i], 0);
		outControl(portMap[i-3], pinMap[i-3], 0);
        TimeDelay(90);
		}
		else{
		outControl(portMap[i], pinMap[i], 0);
        TimeDelay(90);
		}
		if (FIOPIN(portMap[i], pinMap[i]) != 0) {
            puts0(OutPins1[i]); puts0(" did NOT go LOW again....\n\n\r");
            errorCount++;
        }
    }
    
    // Set all pins HIGH
    puts0("\nMaking all Pins HIGH .........\n\r");
    for (int i = 0; i < totalPins; i++){
        outControl(portMap[i], pinMap[i], 1);  }

    TimeDelay(200);
    puts0("All pins set to HIGH.\n\r");
    for (int i = 0; i < totalPins; i++) {
        if (FIOPIN(portMap[i], pinMap[i]) != 1) {
            puts0(OutPins1[i]); puts0(" did NOT go High______\n\n\r");
            errorCount++;
        }
    }
    // Toggle each pin LOW, check short, then back HIGH
    for (int i = 0; i < totalPins; i++) {
       if((i == 0)|| (i == 1)){
		outControl(portMap[i], pinMap[i], 0);
		outControl(portMap[i+3], pinMap[i+3], 0);
        TimeDelay(90);
		}
		else if((i == 3) || (i == 4)){
		outControl(portMap[i], pinMap[i], 0);
		outControl(portMap[i-3], pinMap[i-3], 0);
        TimeDelay(90);
		}
        // else if(i==19){
        //     outControl(portMap[i], pinMap[i], 0);
        //     TimeDelay(15);
        //     r = FIOPIN(0,26);
        //     puts0("P1.5 val--> 0         ");
        //     puts0("ReadPin (P0.26) Val-->");
        //     PrintDecimal(r);
        //     puts0("\n\r");
        // }
		else{
        outControl(portMap[i], pinMap[i], 0);
        TimeDelay(90);}

        if (FIOPIN(portMap[i], pinMap[i]) != 0) {
            puts0(OutPins1[i]); puts0(" did NOT go LOW\n\r");
            errorCount++;
        }
		else{
        for (int j = 0; j < totalPins; j++) {
        if (j == i) continue;
		if(((i == 0)&&(j == 3))||(i==1)&&(j==4)||((i == 3)&&(j == 0))||(i==4)&&(j==1))
		continue;
        if (FIOPIN(portMap[j], pinMap[j]) != 1) {
            puts0("SHORT DETECTED: "); puts0(OutPins1[i]);
            puts0(" --> "); puts0(OutPins1[j]); puts0("\n\r");
            errorCount++;
            }
        } }

        if((i == 0)|| (i == 1)){
		outControl(portMap[i], pinMap[i], 1);
		outControl(portMap[i+3], pinMap[i+3], 1);
        TimeDelay(90);
		}
		else if((i == 3) || (i == 4)){
		outControl(portMap[i], pinMap[i], 1);
		outControl(portMap[i-3], pinMap[i-3], 1);
        TimeDelay(90);
		}
		else{
        outControl(portMap[i], pinMap[i], 1);
        TimeDelay(90);}

		if (FIOPIN(portMap[i], pinMap[i]) != 1) {
            puts0(OutPins1[i]); puts0(" did NOT go High again....\n\n\r");
            errorCount++;
        }
    }

    // --- Final Report ---
    puts0("\n--- GPIO Test Complete ---\n\r");
    if (errorCount == 0)
        puts0("All GPIO pins tested OK.\n\r");
    else {
        puts0("Errors detected. Total Errors: ");
        PrintDecimal(errorCount);
        puts0("\n\rCheck the pins listed above.\n\r");
    }
}
/****************   CODe End*************************** */

// --- Pin Mapping Helpers
//uint8_t GetPort(uint8_t index) {
  //  if (index < 4) return 2;        // Index 0–3: Port 2
   // else if (index < 10) return 0;  // Index 4–9: Port 0
   // else if (index < 14) return 2;  // Index 10–13: Port 2
   // else if (index < 26) return 4;  // Index 14–25: Port 4
   // else return 3;                  // Index 26–31: Port 3
//}










/*

uint8_t GetPort(uint8_t index) {
    if (index < 15) return 0;        // Port 0
    else if (index < 36) return 1;  // Port 1
    else if (index < 50) return 2;  // P2
    else if (index < 55) return 3;  // P3
	else if (index < 56) return 4;  // P4
}


uint8_t GetPin(uint8_t index) {
    static const uint8_t pinMap[] = {
        4,5,6,7,8,9,14,17,22,26,27,28,29,30,31, 0,1,2,3,4,5,6,7,8,9,
		10,11,12,13,15,16,17,18,19,20,21, 0,1,4,5,6,8,10,11,13,18,19,
		23,24,26, 16,23,24,25,26, 26
    };
    return pinMap[index];
}


// --- Main GPIO Test Function with Short Detection 



void testShortPins(void) {
    puts0("\n--- Starting GPIO Full Pin Test ---\n\r");

    uint8_t errorCount = 0;
    int totalPins = sizeof(OutPins1)/sizeof(OutPins1[0]);

    // Set all pins LOW
    for (int i = 0; i < totalPins; i++) {
        outControl(GetPort(i), GetPin(i), 0);
    }
    TimeDelay(100);
    puts0("All pins set to LOW. Verifying...\n\r");

    for (int i = 0; i < totalPins; i++) {
        uint8_t read = FIOPIN(GetPort(i), GetPin(i));
        if (read != 0) {
            puts0(OutPins1[i]); puts0(" did NOT go LOW\n\r");
            errorCount++;
        }
        TimeDelay(50);
    }

    puts0("Toggling each pin HIGH then LOW with short detection:\n\r");

    for (int i = 0; i < totalPins; i++) {
        // Set current pin HIGH
        outControl(GetPort(i), GetPin(i), 1);
        TimeDelay(50);

        // Check if current pin went HIGH
        uint8_t readHigh = FIOPIN(GetPort(i), GetPin(i));
        if (readHigh != 1) {
            puts0(OutPins1[i]); puts0(" did NOT go HIGH\n\r");
            errorCount++;
        }

        // --- Short detection: Check all other pins did NOT go HIGH ---
        for (int j = 0; j < totalPins; j++) {
            if (j == i) continue;
            uint8_t valOther = FIOPIN(GetPort(j), GetPin(j));
            if (valOther != 0) {
                puts0("SHORT DETECTED: "); puts0(OutPins1[i]); puts0(" --> "); puts0(OutPins1[j]); puts0("\n\r");
                errorCount++;
            }
        }

        TimeDelay(100);

        // Set current pin back LOW
        outControl(GetPort(i), GetPin(i), 0);
        TimeDelay(50);

        // Verify it returned to LOW
        uint8_t readLow = FIOPIN(GetPort(i), GetPin(i));
        if (readLow != 0) {
            puts0(OutPins1[i]); puts0(" did NOT return to LOW\n\r");
            errorCount++;
        }

        TimeDelay(50);
    }

    // --- Phase 2: Set all pins HIGH ---
    for (int i = 0; i < totalPins; i++) {
        outControl(GetPort(i), GetPin(i), 1);
    }
    TimeDelay(100);
    puts0("All pins set to HIGH. Verifying...\n\r");

    for (int i = 0; i < totalPins; i++) {
        uint8_t read = FIOPIN(GetPort(i), GetPin(i));
        if (read != 1) {
            puts0(OutPins1[i]); puts0(" did NOT go HIGH\n\r");
            errorCount++;
        }
        TimeDelay(50);
    }

    puts0("Toggling each pin LOW then HIGH with short detection:\n\r");

    for (int i = 0; i < totalPins; i++) {
        // Set current pin LOW
        outControl(GetPort(i), GetPin(i), 0);
        TimeDelay(50);

        // Verify it went LOW
        uint8_t readLow = FIOPIN(GetPort(i), GetPin(i));
        if (readLow != 0) {
            puts0(OutPins1[i]); puts0(" did NOT go LOW\n\r");
            errorCount++;
        }

        // --- Short detection: Check all other pins did NOT go LOW ---
        for (int j = 0; j < totalPins; j++) {
            if (j == i) continue;
            uint8_t valOther = FIOPIN(GetPort(j), GetPin(j));
            if (valOther != 1) {
                puts0("SHORT DETECTED: "); puts0(OutPins1[i]); puts0(" --> "); puts0(OutPins1[j]); puts0("\n\r");
                errorCount++;
            }
        }

        TimeDelay(100);

        // Set current pin back HIGH
        outControl(GetPort(i), GetPin(i), 1);
        TimeDelay(50);

        // Verify it returned to HIGH
        uint8_t readHigh = FIOPIN(GetPort(i), GetPin(i));
        if (readHigh != 1) {
            puts0(OutPins1[i]); puts0(" did NOT return to HIGH\n\r");
            errorCount++;
        }

        TimeDelay(50);
    }

    // --- Final Report ---
    puts0("\n--- GPIO Test Complete ---\n\r");
    if (errorCount == 0) {
        puts0(" All GPIO pins tested OK.\n\r");
    } else {
        puts0(" Errors detected during GPIO testing. Total Errors: ");
        PrintDecimal(errorCount);
        puts0("\n\rPlease check the failing pins listed above.\n\r");
    }
}


*/


/*
// --- LED pin indices in OutPins1 array
#define LED1_INDEX 6   // P0.14
#define LED2_INDEX 41  // P1.18
#define LED3_INDEX 40  // P1.19

int isLED(uint8_t index) {
    return (index == LED1_INDEX || index == LED2_INDEX || index == LED3_INDEX);
}

// --- Function to safely toggle LEDs one by one at startup
void LEDStartupSequence(void) {
    int leds[] = {LED1_INDEX, LED2_INDEX, LED3_INDEX};
    int totalLEDs = sizeof(leds)/sizeof(leds[0]);

    puts0("\n--- LED Startup Test ---\n\r");

    for (int i = 0; i < totalLEDs; i++) {
	  outControl(GetPort(leds[i]), GetPin(leds[i]), 1);  
      TimeDelay(300);                                 
      outControl(GetPort(leds[i]), GetPin(leds[i]), 0);  
      TimeDelay(200);                 
    }

    puts0("--- LED Startup Test Complete ---\n\r");
}

// --- Main GPIO Test Function with LEDs excluded
void testShortPins(void) {
    LEDStartupSequence();  // Run LED startup safely

    puts0("\n--- Starting GPIO Full Pin Test ---\n\r");

    uint8_t errorCount = 0;
    int totalPins = sizeof(OutPins1)/sizeof(OutPins1[0]);

    // Set all pins LOW (skip LEDs)
    for (int i = 0; i < totalPins; i++) {
        if (isLED(i)) continue;
        outControl(GetPort(i), GetPin(i), 0);
    }
    TimeDelay(100);
    puts0("All non-LED pins set to LOW. Verifying...\n\r");

    // Verify pins LOW (skip LEDs)
    for (int i = 0; i < totalPins; i++) {
        if (isLED(i)) continue;
        uint8_t read = FIOPIN(GetPort(i), GetPin(i));
        if (read != 0) {
            puts0(OutPins1[i]); puts0(" did NOT go LOW\n\r");
            errorCount++;
        }
        TimeDelay(20);
    }

    puts0("Toggling each non-LED pin HIGH then LOW with short detection:\n\r");

    for (int i = 0; i < totalPins; i++) {
        if (isLED(i)) continue;

        // Set current pin HIGH
        outControl(GetPort(i), GetPin(i), 1);
        TimeDelay(50);

        // Verify current pin HIGH
        if (FIOPIN(GetPort(i), GetPin(i)) != 1) {
            puts0(OutPins1[i]); puts0(" did NOT go HIGH\n\r");
            errorCount++;
        }

        // Short detection on other non-LED pins
        for (int j = 0; j < totalPins; j++) {
            if (j == i || isLED(j)) continue;
            if (FIOPIN(GetPort(j), GetPin(j)) != 0) {
                puts0("SHORT DETECTED: "); puts0(OutPins1[i]); puts0(" --> "); puts0(OutPins1[j]); puts0("\n\r");
                errorCount++;
            }
        }

        // Return current pin LOW
        outControl(GetPort(i), GetPin(i), 0);
        TimeDelay(50);

        // Verify LOW
        if (FIOPIN(GetPort(i), GetPin(i)) != 0) {
            puts0(OutPins1[i]); puts0(" did NOT return to LOW\n\r");
            errorCount++;
        }
    }

    // Set all non-LED pins HIGH
    for (int i = 0; i < totalPins; i++) {
        if (isLED(i)) continue;
        outControl(GetPort(i), GetPin(i), 1);
    }
    TimeDelay(100);

    puts0("\n--- GPIO Test Complete ---\n\r");
    if (errorCount == 0) {
        puts0("All GPIO pins tested OK (LED pins skipped).\n\r");
    } else {
        puts0("Errors detected during GPIO testing. Total Errors: ");
        PrintDecimal(errorCount);
        puts0("\n\rPlease check the failing pins listed above.\n\r");
    }
}

*/

/*  UNCOMMENT  <<<<-----------------------------------
void readINPUT(void){  // different pinStates are used to store the input value of differen pins 
		
outControl(2, 0, outStates0[0]);
outControl(2, 1, outStates0[1]);
outControl(2, 2, outStates0[2]);
outControl(2, 3, outStates0[3]);
outControl(0, 6, outStates0[4]);
outControl(0, 9, outStates0[5]);
outControl(0, 4, outStates0[6]);
outControl(0, 5, outStates0[7]);
outControl(0, 7, outStates0[8]);
outControl(0, 8, outStates0[9]);
outControl(2, 4, outStates0[10]);
outControl(2, 5, outStates0[11]);
outControl(2, 6, outStates0[12]);
outControl(2, 7, outStates0[13]);
outControl(4, 0, outStates0[14]);
outControl(4, 1, outStates0[15]);
outControl(4, 2, outStates0[16]);
outControl(4, 3, outStates0[17]);
outControl(4, 4, outStates0[18]);
outControl(4, 5, outStates0[19]);
outControl(4, 6, outStates0[20]);
outControl(4, 7, outStates0[21]);
outControl(4, 8, outStates0[22]);
outControl(4, 9, outStates0[23]);
outControl(4, 10, outStates0[24]);
outControl(4, 11, outStates0[25]);
outControl(3, 0, outStates0[26]);
outControl(3, 1, outStates0[27]);
outControl(3, 2, outStates0[28]);
outControl(3, 3, outStates0[29]);
outControl(3, 4, outStates0[30]);
outControl(3, 5, outStates0[31]);

TimeDelay(300);
	//puts0("OUTPUT J8 : ");
	 // for(int i=0; i<16; i++){
	//	 putch((outStates0[i] != 0)? '1' : '0');
	 //}
	
	
outREAD[0] = FIOPIN(2, 0);
outREAD[1] = FIOPIN(2, 1);
outREAD[2] = FIOPIN(2, 2);
outREAD[3] = FIOPIN(2, 3);
outREAD[4] = FIOPIN(0, 6);
outREAD[5] = FIOPIN(0, 9);
outREAD[6] = FIOPIN(0, 4);
outREAD[7] = FIOPIN(0, 5);
outREAD[8] = FIOPIN(0, 7);
outREAD[9] = FIOPIN(0, 8);
outREAD[10] = FIOPIN(2, 4);
outREAD[11] = FIOPIN(2, 5);
outREAD[12] = FIOPIN(2, 6);
outREAD[13] = FIOPIN(2, 7);
outREAD[14] = FIOPIN(4, 0);
outREAD[15] = FIOPIN(4, 1);
outREAD[16] = FIOPIN(4, 2);
outREAD[17] = FIOPIN(4, 3);
outREAD[18] = FIOPIN(4, 4);
outREAD[19] = FIOPIN(4, 5);
outREAD[20] = FIOPIN(4, 6);
outREAD[21] = FIOPIN(4, 7);
outREAD[22] = FIOPIN(4, 8);
outREAD[23] = FIOPIN(4, 9);
outREAD[24] = FIOPIN(4, 10);
outREAD[25] = FIOPIN(4, 11);
outREAD[26] = FIOPIN(3, 0);
outREAD[27] = FIOPIN(3, 1);
outREAD[28] = FIOPIN(3, 2);
outREAD[29] = FIOPIN(3, 3);
outREAD[30] = FIOPIN(3, 4);
outREAD[31] = FIOPIN(3, 5);

TimeDelay(300);

outControl(2, 0, outStates2[0]);
outControl(2, 1, outStates2[1]);
outControl(2, 2, outStates2[2]);
outControl(2, 3, outStates2[3]);
outControl(0, 6, outStates2[4]);
outControl(0, 9, outStates2[5]);
outControl(0, 4, outStates2[6]);
outControl(0, 5, outStates2[7]);
outControl(0, 7, outStates2[8]);
outControl(0, 8, outStates2[9]);
outControl(2, 4, outStates2[10]);
outControl(2, 5, outStates2[11]);
outControl(2, 6, outStates2[12]);
outControl(2, 7, outStates2[13]);
outControl(4, 0, outStates2[14]);
outControl(4, 1, outStates2[15]);
outControl(4, 2, outStates2[16]);
outControl(4, 3, outStates2[17]);
outControl(4, 4, outStates2[18]);
outControl(4, 5, outStates2[19]);
outControl(4, 6, outStates2[20]);
outControl(4, 7, outStates2[21]);
outControl(4, 8, outStates2[22]);
outControl(4, 9, outStates2[23]);
outControl(4, 10, outStates2[24]);
outControl(4, 11, outStates2[25]);
outControl(3, 0, outStates2[26]);
outControl(3, 1, outStates2[27]);
outControl(3, 2, outStates2[28]);
outControl(3, 3, outStates2[29]);
outControl(3, 4, outStates2[30]);
outControl(3, 5, outStates2[31]);

TimeDelay(300);


outREAD1[0] = FIOPIN(2, 0);
outREAD1[1] = FIOPIN(2, 1);
outREAD1[2] = FIOPIN(2, 2);
outREAD1[3] = FIOPIN(2, 3);
outREAD1[4] = FIOPIN(0, 6);
outREAD1[5] = FIOPIN(0, 9);
outREAD1[6] = FIOPIN(0, 4);
outREAD1[7] = FIOPIN(0, 5);
outREAD1[8] = FIOPIN(0, 7);
outREAD1[9] = FIOPIN(0, 8);
outREAD1[10] = FIOPIN(2, 4);
outREAD1[11] = FIOPIN(2, 5);
outREAD1[12] = FIOPIN(2, 6);
outREAD1[13] = FIOPIN(2, 7);
outREAD1[14] = FIOPIN(4, 0);
outREAD1[15] = FIOPIN(4, 1);
outREAD1[16] = FIOPIN(4, 2);
outREAD1[17] = FIOPIN(4, 3);
outREAD1[18] = FIOPIN(4, 4);
outREAD1[19] = FIOPIN(4, 5);
outREAD1[20] = FIOPIN(4, 6);
outREAD1[21] = FIOPIN(4, 7);
outREAD1[22] = FIOPIN(4, 8);
outREAD1[23] = FIOPIN(4, 9);
outREAD1[24] = FIOPIN(4, 10);
outREAD1[25] = FIOPIN(4, 11);
outREAD1[26] = FIOPIN(3, 0);
outREAD1[27] = FIOPIN(3, 1);
outREAD1[28] = FIOPIN(3, 2);
outREAD1[29] = FIOPIN(3, 3);
outREAD1[30] = FIOPIN(3, 4);
outREAD1[31] = FIOPIN(3, 5);
	/*
	for (int i = 0; i < 16; i++) { 
    putch((pinStates[i] != 0) ? '1' : '0'); puts0(" ");   // Print all the states from i=0 to i=17 by using ternary operator
	}	
	putch('\n\n \r');
	*/
	
/*  UNCOMMET  <<<<-----------------------------------
	puts0("..............................GPIO J8 TEsting............................ \n\n\r");
	for (int j = 0; j < 32; j++) {
		if ((outStates0[j] == outREAD[j]) & (outStates2[j] == outREAD1[j])) {
			//puts0(OutPins1[j]);
			//puts0(" OK    \n\r");
		} else {
			//puts0(OutPins1[j]);
			//puts0(" NOT OK     \n\r");
			err1[errIndex1++] = OutPins1[j];  // Store only failed ones
			count1++;
		}
		//TimeDelay(42);
	}
	x1 = count1;
	errorFunc1();
}
			
	
	/*
	if(count == 16){
		puts0("COUNT: "); PrintDecimal(count);
		puts0("   GPIO J8 IS OK \n\n"); 
	}
	else{
		puts0("COUNT: "); PrintDecimal(count);
		puts0("   GPIO J8 NOT WORKING \n\n");
	}
	*/
	
	
//	TimeDelay(800);
//} 





void errorFunc0(void){
	if (count0 != 0) {
		puts0("  J7 Errors are: \r");
		for (int i = 0; i < errIndex0; i++) {
			puts0(err0[i]);
			puts0("  \n");
		}
		puts0("\n");
		TimeDelay(1000);
	} else {
		puts0("     -------->NO ERROR (J7)\r");
	}
	puts0("\n\r");
	TimeDelay(300);
}

void errorFunc1(void){
	if (count1 != 0) {
		puts0("  J8 Errors are: \r");
		for (int i = 0; i < errIndex1; i++) {
			puts0(err1[i]);
			puts0("  ");
		}
		puts0("\n");
		TimeDelay(1200);
	} else {
		puts0("     ------->NO ERROR (J8)\r");
	}
	puts0("\n\r");
	TimeDelay(300);
}

unsigned int errReturn0(void){
	return x0;
}


unsigned int errReturn1(void){
	return x1;
}

uint8_t FIOPIN(uint8_t port, uint8_t pin) {
    switch(port) {
        case 0: return (FIO0PIN >> pin) & 1U;
        case 1: return (FIO1PIN >> pin) & 1U;
        case 2: return (FIO2PIN >> pin) & 1U;
        case 3: return (FIO3PIN >> pin) & 1U;
        case 4: return (FIO4PIN >> pin) & 1U;
        default: return 0xFF; // Invalid port
    }
} 


void outControl(uint8_t port, uint8_t pin, uint8_t state){
	switch(port){
		case 0: if(state) FIO0SET |= (1U << pin); else FIO0CLR |= (1U << pin); break;
		case 1: if(state) FIO1SET |= (1U << pin); else FIO1CLR |= (1U << pin); break;
        case 2: if(state) FIO2SET |= (1U << pin); else FIO2CLR |= (1U << pin); break;
        case 3: if(state) FIO3SET |= (1U << pin); else FIO3CLR |= (1U << pin); break;
        case 4: if(state) FIO4SET |= (1U << pin); else FIO4CLR |= (1U << pin); break;
	}
}


// --- Decimal Print (Basic)
void putdec(uint16_t num) {
    char buf[6];
    int i = 0;

    if (num == 0) {
        puts0("0");
        return;
    }

    while (num > 0 && i < sizeof(buf) - 1) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    buf[i] = '\0';

    for (int j = i - 1; j >= 0; j--) {
        char ch[2] = {buf[j], '\0'};
        puts0(ch);
    }
}


	/*
FIO_DIR(uint8_t port, uint8_t pin, uint8_t state){
	switch(port){
		case 0: if(state) FIO0DIR |= (1U << pin) ; else FIO0DIR &=~ (1U << pin); break;
		case 1: if(state) FIO1DIR |= (1U << pin); else FIO1DIR &=~ (1U << pin); break;
        case 2: if(state) FIO2DIR |= (1U << pin); else FIO2DIR &=~ (1U << pin); break;
        case 3: if(state) FIO3DIR |= (1U << pin); else FIO3DIR &=~ (1U << pin); break;
        case 4: if(state) FIO4DIR |= (1U << pin); else FIO4DIR &=~ (1U << pin); break;
	}
	
}	  


note copy 

if(outStates0[j] == pinStates[j]){
			puts0(OutPins1[j]); puts0(" OK        "); TimeDelay(1300); puts0(InPins1[j]); puts0(" OK \n\r"); TimeDelay(1300);
			}
		else {
			if((outStates0[j] == outREAD[j])){
				puts0(OutPins1[j]); puts0(" OK        "); TimeDelay(1300); puts0(InPins1[j]); puts0(" NOT OK \n\r"); TimeDelay(1300);}
			else {
				puts0(OutPins1[j]); puts0(" NOT OK        "); TimeDelay(1300); puts0(InPins1[j]); puts0(" NOT OK \n\r"); TimeDelay(1300);
			}
			
		}
		
		
		inside readINPUT */
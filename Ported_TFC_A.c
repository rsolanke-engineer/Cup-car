#include "CMSIS\MKL25Z4.h"
#include "TFC\MKL25Z4.h" /* include peripheral declarations */
#include "TFC\TFC.h"
#include "core_cm0plus.h"
//#include "startup_mkl25z4.c"

int main(void)
{
	uint32_t t, Delta, i=0;
	uint16_t  LineScanTemp[128],LSarray[128], LSavg, Ledge, Redge, Center, GoodRedge, GoodLedge;
	float	TmpLSavg;

	TFC_Init();
	t = 0 ;

	for(;;)
	{
		//TFC_Task must be called in your main loop.  This keeps certain processing happy (I.E. Serial port queue check)
			TFC_Task();


			//This Demo program will look at the middle 2 switch to select one of 4 demo modes.
			//Let's look at the middle 2 switches
			switch((TFC_GetDIP_Switch()>>1)&0x03)
			{
			default:
			case 0 :
				//Demo mode 0 just tests the switches and LED's
				if(TFC_PUSH_BUTTON_0_PRESSED)
					TFC_BAT_LED0_ON;
				else
					TFC_BAT_LED0_OFF;

				if(TFC_PUSH_BUTTON_1_PRESSED)
					TFC_BAT_LED3_ON;
				else
					TFC_BAT_LED3_OFF;


				if(TFC_GetDIP_Switch()&0x01)
					TFC_BAT_LED1_ON;
				else
					TFC_BAT_LED1_OFF;

				if(TFC_GetDIP_Switch()&0x08)
					TFC_BAT_LED2_ON;
				else
					TFC_BAT_LED2_OFF;

				break;

			case 1:

				//Demo mode 1 will just move the servos with the on-board potentiometers
				if(TFC_Ticker[0]>=20)
				{
					TFC_Ticker[0] = 0; //reset the Ticker
					//Every 20 mSeconds, update the Servos
					TFC_SetServo(0,TFC_ReadPot(0));
					TFC_SetServo(1,TFC_ReadPot(1));
				}
				//Let's put a pattern on the LEDs
				if(TFC_Ticker[1] >= 125)
				{
					TFC_Ticker[1] = 0;
					t++;
					if(t>4)
					{
						t=0;
					}
					TFC_SetBatteryLED_Level(t);
				}

				TFC_SetMotorPWM(0,0); //Make sure motors are off
				TFC_HBRIDGE_DISABLE;


				break;

			case 2 :

				//Demo Mode 2 will use the Pots to make the motors move
				TFC_HBRIDGE_ENABLE;
				TFC_SetMotorPWM(TFC_ReadPot(0),TFC_ReadPot(1));

				//Let's put a pattern on the LEDs
				if(TFC_Ticker[1] >= 125)
					{
						TFC_Ticker[1] = 0;
							t++;
							if(t>4)
							{
								t=0;
							}
						TFC_SetBatteryLED_Level(t);
					}
				break;

			case 3 :

				//Demo Mode 3 will be in Freescale Garage Mode.  It will beam data from the Camera to the
				//Labview Application


				TFC_SetServo(0,0.0);						//center wheels
				TFC_HBRIDGE_ENABLE;

				TFC_SetMotorPWM(.4,.4);
				//TFC_SetMotorPWM(0,0); //Make sure motors are off
				//TFC_HBRIDGE_DISABLE;


				if(TFC_Ticker[0]>100 && LineScanImageReady==1)
					{
					 TFC_Ticker[0] = 0;
					 LineScanImageReady=0;


						//  Here is the line scan processing for black line detection Method #2
					LSavg = 0;
					TmpLSavg = 0.0;

					 for(i=0;i<128;i++)   //first save the line to the temp file
					 {
						 	 LineScanTemp[i] = LineScanImage0[i];
						 	 LSarray[i] = LineScanTemp[i];
					 }
					 for(i=0;i<128;i++)  // Now get the average
					 {
						 	 TmpLSavg += LineScanTemp[i];
					 }
					 LSavg = (uint16_t) ((TmpLSavg / 128.0)*.8) ;
					 //TERMINAL_PRINTF("Average = %X,", LSavg);
					 for(i=0;i<128;i++)
					 {
						if (LineScanTemp[i] >= LSavg)   //this turns the line scan into only 2000 or 50 values and shifts the avg down by 20 for a 1
						 LSarray[i] = 2000;
						else
						 LSarray[i] = 50;
 				     }

					 //  Determine edge detection
					 // Start from middle +/- 20-30 or so and search both ways  Only require a single point dip for line
					 Ledge = 70 ;  //dont change these with out changing if statements !!!
					 Redge = 56 ;



/*
					 int   sobel[5] = {1, 2, 0, -2, -1};
					 int   i, j;
					 int   D1[128];

					 memset(D1, 0, sizeof(D1));

					 for (i = 2; i < 125; i++)
						{
						     for (j = 0; j < 5; j++)
						     {
						          D1[i] += sobel[j] * LineScanTemp[i+j-2];

						      }
					    }


					 int max = 0;
					 int min = 11000;
					 for (i =2; i< 125; i++){
						 for (j = 0; j < 5; j++)
						 {
							 if (D1[i]> max){
								 max = D1[i];
							 }
							 if (D1[i]< min){
								 min = D1[i];
							 }
						 }
					 }
					 for (i =2; i< 125; i++){
						 for (j = 0; j < 5; j++)
						 {
							 if (D1[i]== min){
								Ledge = i;
							 }
						 }
					 }
					 for (i =2; i< 125; i++){
						 for (j = 0; j < 5; j++)
						 {
							 if (D1[i]== max){
								Redge = i;
							 }
						 }
					 }



*/
					 for (i=84; i > 4 ;i--)
					 {
						if ((LineScanTemp[i]-LineScanTemp[i-1]) > 400)
						{
							Ledge = i;        // This is the first edge to the left
							break;
						}



					 }



						GoodLedge = 0;
					 for (i=Ledge; i>=4; i--)
					 {
							if ((LSarray[i] == 2000 )&&(LSarray[i-1] == 2000))  //need two points of white
							{
								GoodLedge = 1;
								break;
							}
					 }

					 //  Now look for the Right edge

					 for (i=44;i<124;i++)
					 {
							if ((LineScanTemp[i]-LineScanTemp[i+1]) > 400)
							{
								Redge = i;
								break;
							}
					 }
					// Now check for white space after edge
						GoodRedge = 0;
					 for (i=Redge;i<124;i++)
					 {
							if ((LSarray[i] == 2000 )&&(LSarray[i+1] == 2000))  //need two points of white
							{
								GoodRedge = 1;
								break;
							}
					 }


					 // Now set the 2 edges in LSarray so we can see them
					 LSarray[Redge] = 3500;
					 LSarray[Ledge] = 2500;
					 LSarray[127-GoodRedge] = 400;
					 LSarray[GoodLedge] = 200;

					 //Calculate distance between lines
					 Delta = Redge-Ledge;
					 LSarray[Delta] += 150;
					 //calculate center
					 Center = Ledge + ((Redge - Ledge)/2);
					 if (Ledge <= 5)  Center = Redge - 40;
					 if (Redge >=123)  Center = Ledge + 40;
					 LSarray[Center] = 4000;

					 	if(t==0)
					 		t=3;
					 	else
					 		t--;

						 TFC_SetBatteryLED_Level(t);
						 TERMINAL_PRINTF("\r\n");
						 TERMINAL_PRINTF("L:");
						 for(i=0;i<128;i++)
						 {
								 TERMINAL_PRINTF("%X,",LineScanTemp[i]);
						 }

						 for(i=0;i<128;i++)
						 {
								 TERMINAL_PRINTF("%X",LSarray[i]);
								 if(i==127)
									 TERMINAL_PRINTF("\r\n",LSarray[i]);
								 else
									 TERMINAL_PRINTF(",",LSarray[i]);
						}

					}


			//now make adjustment in steering
				if((Center >= 60) && (Center <= 68)) {
					TFC_SetServo(0,0.0);						//center wheels
				}
				else if (Center <62 ){
					TFC_SetServo(0,-.5);						//turn left
				}
				else if (Center >70 ){
					TFC_SetServo(0,+.5);						//turn right
				}
				break;
			}
	}

	return 0;
}

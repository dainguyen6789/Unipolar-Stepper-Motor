#include "stc15f2k60s2.h"
#include "diaper.h"
#include <intrins.h>
#include "math.h"
#include "stdio.h"



const uint8 	CHS[] =
{
	4, 5, 6, 7
};


extern uint8	onePageChValue[512];
extern uint8	chReadCount;
extern uint32	pageWriteCount;
f2i 			prevMovAvg[4];

//uint8 NUMEVENT;											//ADDR_NUMEVENT
uint32			MOVINGAVERAGE[4] =
{
	10, 10, 10, 10
};


//ADDR_MOVINGAVERAGE
uint32			LIFESPAN_US = 0; //ADDR_LIFESPAN_US
uint32			LIFESPAN = 4; //ADDR_LIFESPAN TODO: need to change to new updata life span func
uint8			COUNT = 0; //ADDR_COUNT
uint8			ALERTTRYAGAIN = 0; //ADDR_ALERTTRYAGAIN
uint32			TRIGERCHS = 0; //ADDR_TRIGERCHS
uint32			TRIGERCHS_GLOBAL = 0; //ADDR_TRIGERCHS_GLOBAL
uint32			DIFF[4]; //ADDR_DIFF
uint32			MAXDIFF[4] =
{
	0, 0, 0, 0
};


//ADDR_MAXDIFF
uint32			MAXAVER[4] =
{
	0, 0, 0, 0
};


//ADDR_MAXAVER
uint32			BILLBOARD = 0; //ADDR_BILLBOARD

//CHSTATES CHSSTATE;						//ADDR_CHSSTATE
CHSTATES		currStates;

uint32			SumTotal[4] =
{
	0, 0, 0, 0
};


//ADDR_ALLACC
uint16			MAX[4] =
{
	0, 0, 0, 0
};


//ADDR_MAX
extern uint8	rx1Buffer[rx1BufferLength];
extern uint8	tx1Buffer[tx1BufferLength];
extern uint16	timeStamp;
extern uint16	batteryVoltage;

uint8			result[4] =
{
	0, 0, 0, 0
};


uint8 debugInfo_Ch_Value[16] = {0};


/*************************************************
Function: ReadAndProcessData
Description: process the data and make a conclusion ,return a flag if sensing the event
Input :NONE
Output:NONE
Return:
		 0		there is no event
		XXXX		the i th bit shows that in ith channel there is a event
*************************************************/

uint8 t_count = 0;

uint8 ReadAndProcessData(uint16 * chvalues)
{
	uint8			flag;
	uint16			val[4];
	uint16			val2[4];
	uint8			i	= 0, j = 0;
	f2i 			oldAvag, oldMaxAvag, oldMaxDiff, newAvag, curDiff;
	uint16			arraySubOffset;
	
	flag				= 0;
	P54 				= 1;
	
	delayMs(1);
	
	for (i = 0; i < 4; i++)
	{
		arraySubOffset		= chReadCount * 4;
		
		//P54 				= 1;
		val[i]				= ADC_GetResult(CHS[i]);
		chvalues[i] 		= val[i];
		//P54 				= 0;
		
		onePageChValue[arraySubOffset + 1] = val[i] >> 8;
		onePageChValue[arraySubOffset + 0] = val[i];
		
		val2[i] 			= ADC_GetResult(CHS[i]);

		//val3[i]						= ((val2[i] << 16) +val[1]);
		onePageChValue[arraySubOffset + 3] = val2[i] >> 8;
		onePageChValue[arraySubOffset + 2] = val2[i];


		
		// only for debug 
		// -------- begin debug info --------
		// (onePageChValue + arraySubOffset)[0] = t_count;
		// (onePageChValue + arraySubOffset)[1] = t_count;		
		// (onePageChValue + arraySubOffset)[2] = t_count;		
		// (onePageChValue + arraySubOffset)[3] = t_count;			
		
		(debugInfo_Ch_Value + i*4)[0] = (onePageChValue + arraySubOffset)[0];
		(debugInfo_Ch_Value + i*4)[1] = (onePageChValue + arraySubOffset)[1];
		(debugInfo_Ch_Value + i*4)[2] = (onePageChValue + arraySubOffset)[2];
		(debugInfo_Ch_Value + i*4)[3] = (onePageChValue + arraySubOffset)[3];	
    // -------- end debug info --------
		
		
		//debug code		
		//			printf("%8hd", val[i])
		//debug code end
		oldAvag.i			= MOVINGAVERAGE[i]; 	//ReadRTCMem(ADDR_MOVINGAVERAGE+i);
		oldMaxAvag.i		= MAXAVER[i];			//ReadRTCMem(ADDR_MAXAVER+i);
		oldMaxDiff.i		= MAXDIFF[i];			//ReadRTCMem(ADDR_MAXDIFF+i);

		prevMovAvg[i].i 	= oldAvag.i;

		// newAve[i]=((oldAve[i]<<4)-oldAve[i]+(temp[i]&0xffff))>>4;
		// newAve[i]=((oldAve[i]<<3)-oldAve[i]+(temp[i]&0xffff))>>3;
		if (val[i] > oldAvag.f)
			newAvag.f = (oldAvag.f * 15 + val[i]) / 16.0;
		else 
			newAvag.f = (oldAvag.f * 63 + val[i]) / 64.0;

		curDiff.f			= val[i] -newAvag.f;

		if (curDiff.f > oldMaxDiff.f) //WriteRTCMem((ADDR_MAXDIFF+i),diff[i].i);

		//memManager.save_MaxDifference(m_chn, curDiff.i);
			MAXDIFF[i] = curDiff.i;

		if (newAvag.f > oldMaxAvag.f) //WriteRTCMem((ADDR_MAXAVER+i),newAve[i].i);

		//memManager.save_MaxAverage(m_chn, newAvag.i);
			MAXAVER[i] = newAvag.i;


		//memManager.save_MovingAverage(m_chn, newAvag.i);
		//memManager.save_CurrentDifference(m_chn, curDiff.i);
		MOVINGAVERAGE[i]	= newAvag.i;
		DIFF[i] 			= curDiff.i;

		//SumTotal[i] += val[i] - DEFAULT_MOVING_AVERAGE;
		if (val[i] > MAX[i])
		{
			MAX[i]				= val[i];
		}


		//put val[i] into OnePageChValue buffer
		if (chReadCount < 127)
		{
			chReadCount++;
		}
		else 
		{
					chReadCount 		= 0;
			iapEraseSector((pageWriteCount * 512));
			iapWriteByte((pageWriteCount * 512), onePageChValue, 512);

			pageWriteCount++;
		}



		if (curDiff.f > DIFF_EVENTTHRESHOLD)
			flag = flag + (1 << i);


/*
#ifdef PRINTF_DEBUG

		if (curDiff.f > DIFF_EVENTTHRESHOLD)
		{
			printf("%g ", curDiff.f);
			printf("\r\n");
		}

#endif
*/


		//		#ifndef PRINTF_DEBUG
		//		
		//		if (curDiff.f > DIFF_EVENTTHRESHOLD)
		//			{
		//			//		tx1Buffer[0]		= (curDiff.i >> 24);
		//			//		tx1Buffer[1]		= (curDiff.i >> 16);
		//			//		tx1Buffer[2]		= (curDiff.i >> 8);
		//			//		tx1Buffer[3]		= (curDiff.i);
		//			//		uart1SendPackage(tx1Buffer, 4);
		//			tx1Buffer[0]		= flag;
		//			uart1SendPackage(tx1Buffer, 1);
		//		
		//		
		//			}
		//		
		//		
		//		#endif
	}

	P54 				= 0;
	
	t_count ++;
	
	return flag;

}


uint32 State3_new2(void)
{

	uint16			chvalues[4];
	uint8			newEventFlag = 0;
	uint32			currSnap = 0;
	uint32			smState = 0;
	uint8			i;

	uint32			mask = 0;
	uint32			state = 0;
	uint32			cnt = 0;

	uint8			triggered_Yellow = 0;
	uint8			triggered_Orange = 0;
	uint8			triggered_Red = 0;
	uint8			triggered_FlashRed = 0;

	uint8			triggered_Yellow_Mask = 0;
	uint8			triggered_Orange_Mask = 0;
	uint8			triggered_Red_Mask = 0;
	uint8			triggered_FlashRed_Mask = 0;


	//CHSTATES	currStates;
	//TODO: need to change to new updata life span func
	//record the timestamp in the beginning
	LIFESPAN			+= 4;

	COUNT++;

	/* check if there is a alert that failed to send	*/
	//		if (oldval>0)
	//			{
	//				//check red  high 2 bytes
	//				uint16 tmp=(oldval&0xffff0000)>>16;
	//				if (tmp>1)
	//					{
	//						tmp-=1;
	//						oldval&=0xffff;
	//						oldval|=((tmp)<<16);
	//					ALERTTRYAGAIN = oldval;
	//				}
	//				else if (tmp==1)
	//					{
	//						oldval&=0xffff;
	//						ALERTTRYAGAIN = oldval;
	//						SendAlert(REDALERT);
	//					}
	//
	//				//check yellow	 low 2 bytes
	//				tmp=(oldval&0xffff);
	//				if (tmp>1)
	//					{
	//						tmp-=1;
	//						oldval&=0xffff0000;
	//						oldval|=(tmp);
	//						ALERTTRYAGAIN = oldval;
	//					}
	//				else if (tmp==1)
	//					{
	//						oldval&=0xffff0000;
	//						ALERTTRYAGAIN = oldval;
	//						SendAlert(YELALERT);
	//
	//					}
	//			}

	/* read all channels and save all the data*/
	// set P54 to push pull mode, set to high-z mode before sleep

	newEventFlag		= ReadAndProcessData(chvalues);



	//	//test
	//	if(newEventFlag != 0)
	//		{
	//			printf("0x%bx\r\n", newEventFlag);
	//
	//		}
	for (i = 0; i < 4; i++)
	{
		result[i]			= WalkChannel_new(&currStates, i, newEventFlag, chvalues[i]);


		/*
		//debug code
#ifdef PRINTF_DEBUG

		if (result[i] > 0)
		{
			printf("ch=%bd ,result=0x%bx	", i, result[i]);
			printf("\r\n");
		}

#endif
		*/

		//#ifndef PRINTF_DEBUG
		//		if (result[i] > 0)
		//			{
		//
		//		tx1Buffer[0]		= i;
		//		tx1Buffer[1]		= result[i];
		//		uart1SendPackage(tx1Buffer, 2);
		//			}
		//#endif
		//debug code end
	}





	// Calculate bit-mask for determining alert
	for (i = 0; i < 4; i++)
	{

		// bit-mask for Red
		if ((result[i] == 4) || GetBitMask_CurrentRed(i))
		{
			mask				|= (1 << (i + 4));	// create bitset of current cycle RED flags at position 0xF0
			cnt++;
		}

		// bit-mask for Orange
		if ((result[i] == 3) || (result[i] == 4))
		{
			mask				|= (1 << (i + 8));	// create bitset of current cycle Orange flags at position 0xF00
		}

		// bit-mask for Yellow
		if (result[i] == 1)
		{
			mask				|= (1 << (i));		//create bitset of current cycle yellow flags at position 0xF
		}
	}


	// In current cycle, it is possible to have multiple triggered. We should consider this issue !!
	// Previous old version did not handle multiple triggered, only send ONE alert in current cycle.
	// Get mask for each trigger
	if ((mask & 0xF))
	{
		triggered_Yellow	= true;
		triggered_Yellow_Mask = (mask & 0xF);		// should send YELLOW alert whenever triggered in current cycle
	}

	if ((mask & 0xF00))
	{
		triggered_Orange	= true;
		triggered_Orange_Mask = (mask & 0xF00) >> 8; // should check preserved to determine if send ORANGE in current cycle

		// Check preserved ORANGE triggered channels last time
		if (GetBitMask_Preserved_Orange(triggered_Orange_Mask))
		{
			triggered_Orange	= false;			// Don't send ORANGE in this cycle
		}
	}

	if ((mask & 0xF0) && (cnt == 2))
	{
		triggered_Red		= true;
		triggered_Red_Mask	= (mask & 0xF0) >> 4;	// should check preserved to determine if send RED in current cycle

		// Check preserved RED triggered channnels last time
		if (GetBitMask_Preserved_Red(triggered_Red_Mask))
		{
			triggered_Red		= false;			// Don't send RED in this cycle
		}
	}

	if ((mask & 0xF0) && (cnt >= 3))
	{
		triggered_FlashRed	= true;
		triggered_FlashRed_Mask = (mask & 0xF0) >> 4; // should check preserved to determine if send FLASHRED in current cycle

		// Check preserved FLASHRED triggered channels last time
		if (GetBitMask_Preserved_FlashRed(triggered_FlashRed_Mask))
		{
			triggered_FlashRed	= false;			// Don't send FLASHRED in this cycle
		}
	}


	// After checking preserved, Re-build a new mask combined
	mask				= 0;

	if (triggered_Yellow)
	{
		ResetBitMask_Yellow_Triggered(triggered_Yellow_Mask); // Yellow triggered, reset bit-mask
		mask				|= triggered_Yellow_Mask;

	}

	if (triggered_Orange)
	{
		SetBitMask_Preserved_Orange(triggered_Orange_Mask); // Preserved to prevent sending ORANGE repeatly
		mask				|= (triggered_Orange_Mask << 8);

	}

	if (triggered_Red)
	{
		SetBitMask_Preserved_Red(triggered_Red_Mask); // Preserved to prevent sending RED repeatly
		mask				|= (triggered_Red_Mask << 4);

	}

	if (triggered_FlashRed)
	{
		SetBitMask_Preserved_FlashRed(triggered_FlashRed_Mask); // Preserved to prevent sending FlashRed repeatly
		mask				|= (triggered_FlashRed_Mask << 4);

	}


	// In old version, SendAlert() function which can send only ONE alert in curernt cycle.
	// The new version, we use a new parameter MULTI_ALERT to identify multiple triggered.
	if (mask)
	{
		TRIGERCHS			= mask;

		/*
#ifndef PRINTF_DEBUG
		
		uart1SendCommand(UART_COMMAND_ALERT, UART_ALERT_PACK_LENGTH);
		tx1Buffer[0]		= (mask >> 8);
		tx1Buffer[1]		= (mask);
		batteryVoltage		= ADC_GetResult(2);
		tx1Buffer[2]		= batteryVoltage >> 8;
		tx1Buffer[3]		= batteryVoltage;

		uart1SendPackage(tx1Buffer, UART_ALERT_PACK_LENGTH);
				
#endif
		*/
		
		  return mask;
	}
	
	return 0;
}


uint8 WalkChannel_new(CHSTATES * currStates, uint8 i, uint8 newEventFlag, uint16 chvalue)
{
	uint32			smState;
	uint32			diffFlag;
	uint32			averFlag;
	uint8			chsSet;
	uint32			oldval;
	uint32			currCount;
	float			Signal_Drop;
	float			Threshold_Orange;
	float			Threshold_Red;
	f2i 			maxdiff;
	f2i 			curdiff;
	f2i 			maxaver;
	f2i 			max, curr;
	f2i 			newvalue;


	smState 			= GetChannelState(currStates->STATES[i]);

	diffFlag			= GetDiffFlag(currStates->STATES[i]);

	//inc diff counting
	if (diffFlag)
	{
		currStates->STATES[i].countDiff++;
	}
	else 
	{
		currStates->STATES[i].countDiff = 0;
	}


	averFlag			= GetAverFlag(currStates->STATES[i]);

	//inc aver counting
	if (averFlag)
	{
		currStates->STATES[i].countaver++;
	}
	else 
	{
		currStates->STATES[i].countaver = 0;
	}


	chsSet				= newEventFlag & (1 << i);	// to identify yellow

	switch (smState)
	{
		case 0:
			if ((chsSet > 0) && (!diffFlag))
			{

				//currStates->STATES[i].peak=ReadSingleCh(CHS[i])&0xffff;
				currStates->STATES[i].peak = chvalue;


				//start counting
				SetDiffFlag(& (currStates->STATES[i]));
				SetAverFlag(& (currStates->STATES[i]));


				//for (int i=0; i<4; i++)
				MAXDIFF[i]			= 0;

				/*
				uint32_t result=ReadRTCMem(ADDR_COUNTAVER);
				if( (result<STANDARD2 )&&(result>(STANDARD2-4)))
					WriteRTCMem(ADDR_COUNTAVER,(result-(STANDARD1+2)));
				*/
				//next state 1
				SetChannelState(& (currStates->STATES[i]), 1);

			}
			else 
			{
				// DEBUG_LOG("state=0, but no entry to check yellow");
			}

			break;

		case 1:
		case 2:
		case 3:
			currStates->STATES[i].peak += chvalue;
			SetChannelState(& (currStates->STATES[i]), ++smState);
			break;

		// checking point@ 16 secs
		case 4:
			//max diff
			maxdiff.i = MAXDIFF[i];

			//current diff
			curdiff.i = DIFF[i];

			//max average
			maxaver.i = MAXAVER[i];

			if ((curdiff.f > (maxdiff.f / 8)) &&
				 ((currStates->STATES[i].peak) / (STANDARD1 * 1.0)) > (maxaver.f * 7 / 8.0))
			{
				// this is a real event
				if (!isInSession(currStates->STATES[i]))
				{
					SetInSessionFlag(& (currStates->STATES[i]));
					SetChannelState(& (currStates->STATES[i]), ++smState);
				}
				else 
				{
					SetChannelState(& (currStates->STATES[i]), ++smState);
					currStates->STATES[i].countaver = 5;
				}



				newvalue.f			= (currStates->STATES[i].peak) / (STANDARD1 * 1.0);
				MOVINGAVERAGE[i]	= newvalue.i;
				MAXAVER[i]			= newvalue.i;

				oldval				= BILLBOARD;

				//uint32_t currCount=ReadRTCMem(ADDR_COUNT);	 //TODO change to timestamp
				currCount			= LIFESPAN;

				//debug code
				//#ifdef PRINTF_DEBUG
				//				printf("%ld, %ld\r\n", oldval, currCount);
				//#endif
				//debug code end
				if (currCount > (oldval + 32))
				{
					BILLBOARD			= currCount;


					//reset peak value
					currStates->STATES[i].peak = 0;

					//CHSSTATE = currStates;
					return 1;
				}



			}
			else //false alarm
			{
				if (!isInSession(currStates->STATES[i])) // if this was the first event attempt
				{
					ResetAverFlag(& (currStates->STATES[i]));
					ResetDiffFlag(& (currStates->STATES[i]));
					ResetInSessionFlag(& (currStates->STATES[i]));
					currStates->STATES[i].countDiff = 0;
					currStates->STATES[i].countaver = 0;

					SetChannelState(& (currStates->STATES[i]), 0);

				}
				else //if this is a false event on the tail of a previous true event, keep previous event counters/states untouched
				{
					ResetDiffFlag(& (currStates->STATES[i])); //
					SetChannelState(& (currStates->STATES[i]), 6);
				}

			}

			//reset peak value
			currStates->STATES[i].peak = 0;
			break;

		case 5:
			if (currStates->STATES[i].countDiff == STANDARD3)
			{
				ResetDiffFlag(& (currStates->STATES[i]));
				SetChannelState(& (currStates->STATES[i]), 6);
			}
			else 
				//16-64
				SetChannelState(& (currStates->STATES[i]), 5);

			break;

		case 6: //yellow 1 min
			// After 1 minute blanking of new yellow, counts every 15 min before checking for ORANGE/RED/FLashing RED
			//check again for yellow and if potential event, start again from first state.
			if ((chsSet > 0) && (!diffFlag))
			{
				//start counting
				SetDiffFlag(& (currStates->STATES[i]));

				//for (int i=0; i<4; i++)
				MAXDIFF[i]			= 0;

				// SHAOBO Sep26, 2017
				// need to read first value of a potential event
				currStates->STATES[i].peak = chvalue;

				//next state 1
				SetChannelState(& (currStates->STATES[i]), 1);

			}

			// check point @ 15minutes (coutaver == 225)
			else if ((currStates->STATES[i].countaver) % STANDARD2 == 0) // else	if ((currStates->STATES[i].countaver) >= STANDARD2)
			{
				SetChannelState(& (currStates->STATES[i]), 7);
			}
			else 
			{
				// We should check Yellow also
				SetChannelState(& (currStates->STATES[i]), 6);
			}

			break;

		case 7: // 15min checking point
			{

				max.i				= MAXAVER[i];
				curr.i				= MOVINGAVERAGE[i];

				//adjust threshold in case signal is going up
				if ((curr.f - prevMovAvg[i].f) / max.f > PERCENT_RISE_THRESHOLD)
				{

					float			rollback_count =
						 (curr.f - prevMovAvg[i].f) / max.f * (STANDARD2 / PERCENTDROP_FLASHRED);

					if (rollback_count > 0)
					{
						int 			temp = currStates->STATES[i].countaver - rollback_count;

						if (temp > 0)
						{
							currStates->STATES[i].countaver = currStates->STATES[i].countaver - rollback_count;
						}
						else 
						{
							currStates->STATES[i].countaver = 1;
						}
					}
				}

				if (currStates->STATES[i].countaver >= STANDARD2)
				{


					//uint32_t shortlist=ReadRTCMem(ADDR_TRIGERCHS_AVER);
					// Signal_Drop
					//float leftval=max.f-curr.f;
					// Threshold
					//float rightval=max.f*PERCENTDROP*(currStates->STATES[i].countaver-1)/STANDARD2 ;
					Signal_Drop 		= max.f - curr.f;

					Threshold_Orange	=
						 ((float) (max.f * PERCENTDROP_ORANGE * (currStates->STATES[i].countaver - 1))) / STANDARD2;

					Threshold_Red		=
						 ((float) (max.f * PERCENTDROP_FLASHRED * (currStates->STATES[i].countaver - 1))) / STANDARD2;


					//if ((max.f-curr.f)<=(max.f*(currStates->STATES[i].countaver)*(DEEPSLEEPTIME/1000000)/(18000) ))
					//if Signal_Drop < Threshold_Orange then set Orange DFlag for channel
					//if Signal_Drop < Threshold_Red then set Red DFlag for channel
					if (Signal_Drop <= Threshold_Red)
					{

						ResetDiffFlag(& (currStates->STATES[i]));

						//SetChannelState(&(currStates->STATES[i]),10);
						SetChannelState(& (currStates->STATES[i]), 6);

						// mask the current channel RED state, this bit-mask will be checked later to judge sending Red/FlashRed
						SetBitMask_CurrentRed(i, true);


						//					CHSSTATE=currStates;
						return 4; // Flashing Red;
					}

					else if (Signal_Drop <= Threshold_Orange)
					{


						ResetDiffFlag(& (currStates->STATES[i]));

						//SetChannelState(&(currStates->STATES[i]), 9);
						SetChannelState(& (currStates->STATES[i]), 6);

						//					CHSSTATE=currStates;
						// return 2;	// old RED
						return 3; // new ORANGE

					}

					else 
					{

						//DEBUG_LOG("No Red alert!"," ");
						//DEBUG_LOG("Not qualified with ORANGE");
						SetChannelState(& (currStates->STATES[i]), 6);

					}

				}

			}
			break;

		default:
			ResetAverFlag(& (currStates->STATES[i]));
			ResetDiffFlag(& (currStates->STATES[i]));
			ResetInSessionFlag(& (currStates->STATES[i]));
			SetChannelState(& (currStates->STATES[i]), 0);
			currStates->STATES[i].countDiff = 0;
			currStates->STATES[i].countaver = 0;
			break;
	}

	// update data structure
	//		CHSSTATE=currStates;
	return 0;

}



///*************************************************
//Function: UpdateLifeSpan
//Description: update current time in terms of seconds
//Input:
//		newdelta: the time step that will be added in terms of rtc time ticks
//		deepsleep:
//Output:NONE
//Return: return current time in terms of seconds
//*************************************************/
//uint32 UpdateLifeSpan(uint32 newdelta, uint32 sleep_us)
//{
//	uint32			oldus;
//	uint32			newinterval, result, oldsec, temp;
//	uint32			deltaS;
//
//
//	oldus				= LIFESPAN_US;
//
//	//For 12T mode, every tick = 0.5us
//	// change round() to floor()
//	newinterval 		= oldus + floor((newdelta * 0.5) +sleep_us); //us
//
//	oldsec				= LIFESPAN;
//
//	if (newinterval > 1000000)
//		{
//		deltaS				= newinterval / 1000000;
//
//		result				= oldsec + deltaS;
//
//		LIFESPAN			= result;
//
//		temp				= newinterval % 1000000;
//
//		LIFESPAN_US 		= temp;
//		return result;
//
//		}
//	else 
//		{
//		LIFESPAN			= newinterval;
//		return oldsec;
//		}
//
//
//}
void SetBitMask_CurrentRed(uint8 i, uint8 setVal)
{
	uint32			global;
	uint32			mask;

	global				= TRIGERCHS_GLOBAL;
	mask				= 1 << (i + 0);

	if (setVal)
	{
		global				|= mask;
	}
	else 
	{
		global				&= (~mask);
	}

	TRIGERCHS_GLOBAL	= global;
}



uint8 GetBitMask_CurrentRed(uint8 i)
{
	uint32			mask;

	mask				= 1 << (i + 0);
	return ((TRIGERCHS_GLOBAL & mask) == mask) ? true: false;
}


uint8 GetBitMask_Preserved_Orange(uint8 mask)
{
	uint32			check;

	check				= (mask << 4);
	return ((TRIGERCHS_GLOBAL & check) == check);
}


uint8 GetBitMask_Preserved_Red(uint8 mask)
{
	uint32			check;

	check				= (mask << 8);
	return ((TRIGERCHS_GLOBAL & check) == check);
}


uint8 GetBitMask_Preserved_FlashRed(uint8 mask)
{
	uint32			check;

	check				= (mask << 12);
	return ((TRIGERCHS_GLOBAL & check) == check);
}


void ResetBitMask_Yellow_Triggered(uint8 triggered_Yellow_Mask)
{
	if (triggered_Yellow_Mask & 0x1)
	{
		ResetBitMask_Global(0);
	}

	if (triggered_Yellow_Mask & 0x2)
	{
		ResetBitMask_Global(1);
	}

	if (triggered_Yellow_Mask & 0x4)
	{
		ResetBitMask_Global(2);
	}

	if (triggered_Yellow_Mask & 0x8)
	{
		ResetBitMask_Global(3);
	}
}


void ResetBitMask_Global(uint8 i)
{
	uint32			mask;

	mask				= 0;

	mask				= 1 << (i + 0); 			// Reset BitMask_CurrentRed
	TRIGERCHS_GLOBAL	&= (~mask);

	mask				= 1 << (i + 4); 			// Reset BitMask_Preserved_Orange
	TRIGERCHS_GLOBAL	&= (~mask);

	mask				= 1 << (i + 8); 			// Reset BitMask_Preserved_Red
	TRIGERCHS_GLOBAL	&= (~mask);

	mask				= 1 << (i + 12);			// Reset BitMask_Preserved_FlashRed
	TRIGERCHS_GLOBAL	&= (~mask);

}


void SetBitMask_Preserved_Orange(uint8 mask)
{
	TRIGERCHS_GLOBAL	|= (mask << 4);
}


void SetBitMask_Preserved_Red(uint8 mask)
{
	TRIGERCHS_GLOBAL	|= (mask << 8);
}


void SetBitMask_Preserved_FlashRed(uint8 mask)
{
	TRIGERCHS_GLOBAL	|= (mask << 12);
}


uint8 GetChannelState(CHSTATE currStates)
{
	uint8			oldval;

	oldval				= currStates.flagstate;
	oldval				&= STATE_MASK;
	return oldval;

}


uint8 GetDiffFlag(CHSTATE currStates)
{
	uint8			tmp;

	tmp 				= currStates.flagstate;
	tmp 				&= (~DIFFFLAG_MASK);
	return (tmp > 0) ? 1: 0;

}


uint8 GetAverFlag(CHSTATE currStates)
{
	uint8			tmp;

	tmp 				= currStates.flagstate;
	tmp 				&= (~AVERFLAG_MASK);

	return tmp > 0 ? 1: 0;
}


void SetDiffFlag(CHSTATE * currStates)
{
	uint8			oldval;

	oldval				= currStates->flagstate;
	oldval				|= (~DIFFFLAG_MASK);
	currStates->flagstate = oldval;
}


void SetAverFlag(CHSTATE * currStates)
{
	uint8			tmp;

	tmp 				= currStates->flagstate;
	tmp 				|= (~AVERFLAG_MASK);
	currStates->flagstate = tmp;

}


void SetChannelState(CHSTATE * currStates, uint8 newstate)
{
	uint8			oldval;

	oldval				= currStates->flagstate;
	oldval				&= (~STATE_MASK);
	oldval				|= (newstate & (STATE_MASK));
	currStates->flagstate = oldval;
}


uint8 isInSession(CHSTATE currStates)
{
	uint8			oldval;

	oldval				= currStates.flagstate;
	oldval				&= (~SESSION_MASK);
	return (oldval > 0) ? 1: 0;

}


void SetInSessionFlag(CHSTATE * currStates)
{
	uint8			oldval;

	oldval				= currStates->flagstate;
	oldval				|= (~SESSION_MASK);
	currStates->flagstate = oldval;

}


void ResetAverFlag(CHSTATE * currStates)
{
	uint8			tmp;

	tmp 				= currStates->flagstate;
	tmp 				&= AVERFLAG_MASK;
	currStates->flagstate = tmp;

}


void ResetDiffFlag(CHSTATE * currStates)
{
	uint8			oldval;

	oldval				= currStates->flagstate;
	oldval				&= DIFFFLAG_MASK;
	currStates->flagstate = oldval;
}


void ResetInSessionFlag(CHSTATE * currStates)
{
	uint8			oldval;

	oldval				= currStates->flagstate;
	oldval				&= (SESSION_MASK);
	currStates->flagstate = oldval;

}


/*************************************************
Function: ADC_Init
Description: ADC initialize
Input: NONE
Output:NONE
Return:NONE

*************************************************/
void ADC_Init(void)
{
	//Initialize P14, P15, P16, P17 as input
	P1M1				|= ((1 << 2) | (1 << 3) |(1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));

	P1M0				&= ~((1 << 2) | (1 << 3) |(1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));


	P1ASF				|= (P1ASF_2 | P1ASF_3 |P1ASF_4 | P1ASF_5 | P1ASF_6 | P1ASF_7); //set P14, P15, P16, P17 as ADC channel

	ADC_RES 			= 0;						//clear ADC data reg

	//enable ADC power, set ADC speed
	ADC_CONTR			= ADC_POWER | ADC_SPEEDLL;
	delayMs(2); 									//delay 2ms
}


/*************************************************
Function: ADC_GetResult
Description: get ADC result by inquiry mode
Input: uint8 ch: ADC ch
Output:none
Return:uint16: the 10 bit result

*************************************************/
uint16 ADC_GetResult(uint8 ch)
{
	uint16			ADC_Value;

	//ADC_CONTR			= ADC_POWER | ADC_SPEEDL | ch | ADC_START; //Æô¶¯ADC
	ADC_CONTR			= ADC_POWER | ADC_SPEEDLL | ch | ADC_START; //Æô¶¯ADC
	_nop_();										//delay
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	while (! (ADC_CONTR & ADC_FLAG))
		; //wait for ADC finish

	ADC_CONTR			&= ~ADC_FLAG;				//clear finish flag
	ADC_Value			= ADC_RES;					//read ADC high 8bit
	ADC_Value			= (ADC_Value << 2) | ADC_RESL; //read ADC low 2bit, emerge the result
	return ADC_Value;
}




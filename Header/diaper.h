//#define PRINTF_DEBUG
//#define ESP_DOWNLOAD
typedef unsigned int uint16;
typedef unsigned long uint32;
typedef unsigned char uint8;
typedef long	int32;



// change from 90 to 150
#define DIFF_EVENTTHRESHOLD 	150 //105

#define DIFFFLAG_MASK			0x7f
#define AVERFLAG_MASK			0xBF
#define SESSION_MASK			0xDF
#define STATE_MASK				0x1F

#define DEEPSLEEPTIME			4000000 	//us
#define STANDARD1				4 //(16*1000000/DEEPSLEEPTIME)				//16 /4=4
#define STANDARD2				225 //(900*1000000/DEEPSLEEPTIME)			//900/4= 225
#define STANDARD3				16//(64*1000000/DEEPSLEEPTIME)				//64/4

#define PERCENTDROP_FLASHRED	((float)(1.0/100))
#define PERCENTDROP_ORANGE		((float)(4.0/100))
#define PERCENT_RISE_THRESHOLD	((float)(0.1/100))

#define true					0x01
#define false					0x00


#define ADC_START				0x08
#define ADC_FLAG				0x10			//ADC convert finish flag	


#define ADC_CHANNEL2			0x02			 //ADC channel P1.2
#define ADC_CHANNEL3			0x03			 //ADC channel P1.3
#define ADC_CHANNEL4			0x04			 //ADC channel P1.4
#define ADC_CHANNEL5			0x05			 //ADC channel P1.5
#define ADC_CHANNEL6			0x06			 //ADC channel P1.6
#define ADC_CHANNEL7			0x07			 //ADC channel P1.7

#define P1ASF_2 				0x04			 //set P1.2 to ADC port
#define P1ASF_3 				0x08			 //set P1.3 to ADC port
#define P1ASF_4 				0x10			 //set P1.4 to ADC port
#define P1ASF_5 				0x20			 //set P1.5 to ADC port
#define P1ASF_6 				0x40			 //set P1.6 to ADC port
#define P1ASF_7 				0x80			 //set P1.7 to ADC port

#define ESP_UART_RD_N			P26				// STC P26 is an		 input, low means ESP UART is ready, low to high means wifi transmission is over// on sch the net name of this pin is ESP_UART_RD
#define WIFI_ON_N				P27				// STC P27 is an output, low means ESP wifi on, high means ESP wifi off// on sch the net name of this pin is STC_UART_RD

#define ADC_POWER				0x80			//ADC power control bit
#define ADC_SPEEDLL 0x00					//ADC speed 540 clock
#define ADC_SPEEDL				0x20			//ADC speed 360 clock
#define ADC_SPEEDH				0x40			//ADC speed 180 clock
#define ADC_SPEEDHH 			0x60			//ADC speed 90 clock

#define DEFAULT_MOVING_AVERAGE	10


// new events, use UART_COMMAND_ALERT high 4 bits to transfer these events
#define GREEN_ON_EVENT			0x10			//sensor connect
#define GREEN_OFF_EVENT 		0x20			//sensor remove
#define CHARGER_ON_EVENT		0x30			//charger connect
#define CHARGER_OFF_EVENT		0x40			//charger remove
#define DATA_BACKUP_DONE 	  0x50

//uart1
#define rx1BufferLength 		4
#define tx1BufferLength 		512
#define UART_COMMAND_ALERT		0x01			//0x8888: head, 0x01: command, next 2 bytes: the length of next data package
#define UART_ALERT_PACK_LENGTH	0x0004
//#define UART_COMMAND_CH_VALUE_PACK 0x09			//0x8888: head, 0x09: command, next 2 bytes: the numbers of next data package
#define UART_COMMAND_CH_VALUES	0x08			//0x8888: head, 0x08: command, next 2 bytes: the length of next data package = chReadCount

#define UART_COMMAND_BATTERY_LOW 0x02			//0x8888: head, 0x02: command, next 2 bytes: the length of next data package	
#define UART_BATTERY_PACK_LENGTH 0x0004

#define UART_COMMAND_PSENSOR	0x04	//0x8888: head, 0x04: command, next 2 bytes: the length of next data package	
#define UART_PSENSOR_PACK_LENGTH 0x0004

#define ENABLE_IAP				0x83		   //if SYSCLK<12MHz

#define CMD_IDLE				0				//空闲模式
#define CMD_READ				1				//IAP字节读命令
#define CMD_PROGRAM 			2				//IAP字节编程命令
#define CMD_ERASE				3				//IAP扇区擦除命令


#define PWM_FREQUENCY			2800
#define MAIN_FREQUENCY			11059200

//#define	PAC2_VALUE					1382//(sys_clk/(4*pwm_freq))
#define ID_ADDR_RAM 			0xf8		//for 256byte ram MCU
#define POWER_DOWN_SLEEP_TIME	3999989   // us (4 seconds)
//#define POWER_DOWN_SLEEP_TIME	999989   // us (1 second)
//#define POWER_DOWN_SLEEP_TIME	199989   // us (200 milli-second)


#define ENABLE_DEBUG_OUTPUT_CH_VALUE	0

#define WAIT_UNTIL_ESP_DATA_DONE		{while(P26);}

typedef enum 
{
STANDBY = 0, 
WORKING, 
CHARGING, 
} STC_WORK_STATE;


typedef struct 
{

//diffflag:bit7 	
//averflag:bit6 ;	
//Embeddedevent:bit5 ,		 0: 初始状态，表示还没有时间发生；  1： 先前已经有一个有效事件发生了
//state:bit4-bit0 
uint8			flagstate;
uint8			countDiff;
uint16			countaver;
uint16			peak;

} CHSTATE;


typedef struct 
{
CHSTATE 		STATES[4];

} CHSTATES;


typedef union 
{
int32			i;
float			f;

} f2i;


uint8 ReadAndProcessData(uint16 * chvalues);
uint32 State3_new2(void);
uint8 WalkChannel_new(CHSTATES * currStates, uint8 i, uint8 newEventFlag, uint16 chvalue);


void SetBitMask_CurrentRed(uint8 i, uint8 setVal);
uint8 GetBitMask_CurrentRed(uint8 i);
uint8 GetBitMask_Preserved_Orange(uint8 mask);
uint8 GetBitMask_Preserved_Red(uint8 mask);
uint8 GetBitMask_Preserved_FlashRed(uint8 mask);
void ResetBitMask_Yellow_Triggered(uint8 triggered_Yellow_Mask);
void ResetBitMask_Global(uint8 i);
void SetBitMask_Preserved_Orange(uint8 mask);
void SetBitMask_Preserved_Red(uint8 mask);
void SetBitMask_Preserved_FlashRed(uint8 mask);
uint8 GetChannelState(CHSTATE currStates);
uint8 GetDiffFlag(CHSTATE currStates);
uint8 GetAverFlag(CHSTATE currStates);
void SetDiffFlag(CHSTATE * currStates);
void SetAverFlag(CHSTATE * currStates);
void SetChannelState(CHSTATE * currStates, uint8 newstate);
uint8 isInSession(CHSTATE currStates);
void SetInSessionFlag(CHSTATE * currStates);
void ResetAverFlag(CHSTATE * currStates);
void ResetDiffFlag(CHSTATE * currStates);
void ResetInSessionFlag(CHSTATE * currStates);


void ADC_Init(void);
uint16 ADC_GetResult(uint8 ch);


void IO_Init(void);
void interruptProcess(void);

void interruptInit(void);
void uart1Init(void);
void Uart1_Process(void);


void uart1_SendCommand(uint8 command, uint16 payload_length, uint8 * payload_buffer);
void uart1_SendPackage(uint8 * payload_buffer, uint16 payload_length);


void powerDownSleep(void);
void ESP_Enable(void);
void ESP_Disable(void);
void delayMs(uint16 ms);
void timer0Init(void);


void iapEraseSector(uint16 addr);
void iapIdle();
uint8 iapWriteByte(uint16 begin_addr, uint8 * array, uint16 i);
void iapReadOnePage(uint16 begin_addr, uint8 * array);
void PCA2_Init(void);
void buzzerBeeping(void);
void counter1Start(void);
void uart1SendChar(uint8 cdata);




























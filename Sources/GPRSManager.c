/*! **********************************************************************************************
* 2016  ITESM Campus Guadalajara. Laboratorio de Microcontroladores 
*  
*
* @file:      GPRSManager.c
* @author(s): Manuel Madrigal Valenzuela; Efraín Duarte López
*
* @brief (Theory of Operation)
*   
*     
**************************************************************************************************/


/*************************************************************************************************/
/*********************						Includes						**********************/
/*************************************************************************************************/
#include "GPRSManager.h"

/*************************************************************************************************/
/*********************						Defines							**********************/
/*************************************************************************************************/
#define START_TIMER_RTI SRTISC = 0b00000110 //Start counting to 512 ms
#define TIMER_RTI_NOT_FINISHED SRTISC_RTIF==0 //Timer finish indicator flag is off
#define TIMER_RTI_RESET_FLAG SRTISC_RTIACK = 1 //Turning timer finish indicator flag off again
#define STOP_TIMER_RTI SRTISC=0b00000000 //Stop timer


//Function that uses the SRTISC register to count for 512 ms and then stop 
void timer_half_a_second(void)
{
	
	
   START_TIMER_RTI; 
   do
   {
      __RESET_WATCHDOG();
		
   }while(TIMER_RTI_NOT_FINISHED);
	
   TIMER_RTI_RESET_FLAG;
   STOP_TIMER_RTI; 
}
/*************************************************************************************************/
/*********************						Typedefs						**********************/
/*************************************************************************************************/

/*************************************************************************************************/
/*********************					Function Prototypes					**********************/
/*************************************************************************************************/

/*************************************************************************************************/
/*********************                  Static Variables                    **********************/
/*************************************************************************************************/
static char debugBuf[20];
static u8 debugindex;

static char cRxBuffer[SCI_MAX_MSG_SIZE];

static bool bRxMsgRdy;
static const u8 u8HashKeyToMatch=(u8)'O'+(u8)'K'; 
static u8 HashKey;
static char pastChar;
static char	actualChar;

static u16 HashKey2;
static const u16 u16HashKeyToMatch=(u8)'C'+(u8)'M'+(u8)'T'+(u8)'I'+(u8)':'+(u8)' '+(u8)'\"'+(u8)'M'+(u8)'E'+(u8)'\"'+(u8)',';
static bool msgRcvd;

static bool readmessage;

static u8	u8RxIndex;
static bool msgready;
static char rcvMsgCmmd[]={"AT+CMGR=0000000000"};
static u8 i;

/*************************************************************************************************/
/*********************					Global Variables					**********************/
/*************************************************************************************************/

/*************************************************************************************************/
/*********************                  Static Constants                    **********************/
/*************************************************************************************************/

/*************************************************************************************************/
/*********************                  Global Constants                    **********************/
/*************************************************************************************************/

/*************************************************************************************************/
/*********************				   Exported Functions					**********************/
/*************************************************************************************************/

void GPRSManager_Init(void)
{
   u8RxIndex=0;
   i=0;
   msgRcvd=FALSE;
   msgready=FALSE;
   HashKey=0;
   HashKey2=0;
   readmessage=FALSE;
   bRxMsgRdy=FALSE;
   
   SCI_InitTx();
   SCI_InitRx();
   SCI_RxSetCallback(&GPRSManager_Rx);//Sets the function to be called when an interruption of the SCI reception is rised
   
   SCI_SendMessage("AATE0\r\n",7);
   do
   {
	   __RESET_WATCHDOG();
   }while(!bRxMsgRdy);
   bRxMsgRdy=FALSE;
   
   SCI_SendMessage("AT+CMGF=1\r\n",11);
   do
   {
    __RESET_WATCHDOG();
   }while(!bRxMsgRdy);
   bRxMsgRdy=FALSE;
   
   SCI_SendMessage("AT+CSCS=\"GSM\"\r\n",15);
   do
   {
	   __RESET_WATCHDOG();
   }while(!bRxMsgRdy);
   bRxMsgRdy=FALSE;
}

void GPRSManager_Rx(char cReceivedChar)
{
	debugBuf[debugindex++%20]=cReceivedChar;
	
	if(!bRxMsgRdy ){
		pastChar=actualChar;
		actualChar=cReceivedChar;
		HashKey=(u8)pastChar+(u8)actualChar;
		if(HashKey==u8HashKeyToMatch){
			bRxMsgRdy=TRUE;
			pastChar=0;
			actualChar=0;
			
		}
	}
	
	if(cReceivedChar=='+')
	{
		
		HashKey2=0;
		if(msgRcvd)
		{
			msgRcvd=FALSE;
			readmessage=TRUE;
		}
		else
		{
			msgRcvd=TRUE;
		}

	}
	else if(msgRcvd)
	{
		if(HashKey2==u16HashKeyToMatch)
		{
			rcvMsgCmmd[8+u8RxIndex++]=cReceivedChar;
			if(cReceivedChar=='\n')
			{
				msgready=TRUE;
				HashKey2=0;
			}
		}
		else{
			HashKey2+=(u8)cReceivedChar;
		}
	}
	else if(readmessage)
	{	
		if(!bRxMsgRdy)
		{
			cRxBuffer[u8RxIndex++]=cReceivedChar;
		}
		else
		{
			readmessage=FALSE;
			bRxMsgRdy=FALSE;
			u8RxIndex=0;
		}
	}
}



void bmessageready(void)
{
	if(msgready)
	{
		SCI_SendMessage(rcvMsgCmmd,(8+u8RxIndex));
		u8RxIndex=0;
		msgready=FALSE;
	}
}

void GPRSManager_sendMessage(char cNumber[],char cMessage[],u8 size)
{
	u8 i;
	char ctrz=26;
	char cCommandNumber[]={"AT+CMGS=\"0000000000\"\r\n"};
	for(i=0;i<10;i++)
	{
		cCommandNumber[9+i]=cNumber[i];
	}
	
	SCI_SendMessage(cCommandNumber,22);
	while(SCI_TxIsBusy()){
		__RESET_WATCHDOG();
	}
	bRxMsgRdy=FALSE;
	timer_half_a_second();
	
	
	SCI_SendMessage(cMessage,size);
	while(SCI_TxIsBusy()){
		__RESET_WATCHDOG();
	}
	bRxMsgRdy=FALSE;
	timer_half_a_second();
	
	SCI_SendMessage(&ctrz,1);
	timer_half_a_second();
	
	u8RxIndex=0;
	msgready=FALSE;
	msgRcvd=FALSE;
	readmessage=FALSE;
	bRxMsgRdy=FALSE;
	HashKey2=0;


	
}



//-------------------------------------------------------------------------------------------------


/*************************************************************************************************/
/*********************				    Private Functions					**********************/
/*************************************************************************************************/


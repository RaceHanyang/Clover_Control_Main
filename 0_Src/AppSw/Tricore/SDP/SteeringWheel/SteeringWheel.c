/*
 * SteeringWheel.h
 * Created on: 2020.08.05
 * Author: Dua
 */


/* TODO
 * SteeringWheel Can Transmition
 * -
 */

/***************************** Includes ******************************/
#include "SteeringWheel.h"
#include "SDP.h"

/**************************** Macro **********************************/
#define _MAX_BP_PER_BRAKE_LINE_ 75.0
#define _MIN_BP_PER_BRAKE_LINE_ 5.0



/************************* Data Structures ***************************/
typedef struct 
{
	CanCommunication_Message msgObj1; //StWhlMsgId1
	CanCommunication_Message msgObj2; //StWhlMsgId2
	CanCommunication_Message msgObj3; //StWhlMsgId3
	SteeringWheel_canMsg1_t canMsg1;
	SteeringWheel_canMsg2_t canMsg2;
	SteeringWheel_canMsg3_t canMsg3;

#ifdef __SDP_CLOVER__
	CanCommunication_Message msgObj4;
	SteeringWheel_RSWMsg_t canMsg4;
#endif
}SteeringWheel_t;

/*********************** Global Variables ****************************/
//TX ID~
const uint32 StWhlMsgId1 = 0x00101F00UL;
const uint32 StWhlMsgId2 = 0x00101F01UL;
const uint32 StWhlMsgId3 = 0x00101F02UL;
//~TX ID
//RX ID~
//const uint32 StWhlRSWMsgId = 0x237BB01UL;
const uint32 StWhlRSWMsgId = 0x00101F10UL;
//~RX ID

SteeringWheel_t SteeringWheel;
SteeringWheel_public_t SteeringWheel_public;
/*
amkActualValues2 INV_FL_AMK_Actual_Values2;
amkActualValues2 INV_RL_AMK_Actual_Values2;
amkActualValues2 INV_RR_AMK_Actual_Values2;
amkActualValues2 INV_FR_AMK_Actual_Values2;
*/
/******************* Private Function Prototypes *********************/


/********************* Function Implementation ***********************/

void SteeringWheel_init(void)
{
	//TX init~
	{
		CanCommunication_Message_Config config;
		config.messageId		=	StWhlMsgId1;
        config.frameType		=	IfxMultican_Frame_transmit;
        config.dataLen			=	IfxMultican_DataLengthCode_8;
        config.node				=	&CanCommunication_canNode0;
		CanCommunication_initMessage(&SteeringWheel.msgObj1, &config);
	}
	{
		CanCommunication_Message_Config config;
		config.messageId 		= 	StWhlMsgId2;
		config.frameType		=	IfxMultican_Frame_transmit;
        config.dataLen			=	IfxMultican_DataLengthCode_8;
        config.node				=	&CanCommunication_canNode0;
        CanCommunication_initMessage(&SteeringWheel.msgObj2, &config);
	}
	{
		CanCommunication_Message_Config config;
		config.messageId 		= 	StWhlMsgId3;
		config.frameType		=	IfxMultican_Frame_transmit;
        config.dataLen			=	IfxMultican_DataLengthCode_8;
        config.node				=	&CanCommunication_canNode0;
        CanCommunication_initMessage(&SteeringWheel.msgObj3, &config);
	}
	//~TX init

	//RX init~
#ifdef __SDP_CLOVER__
	{
		CanCommunication_Message_Config config;
		config.messageId = StWhlRSWMsgId;
		config.frameType = IfxMultican_Frame_receive;
		config.dataLen = IfxMultican_DataLengthCode_8;
		config.node = &CanCommunication_canNode0;
		CanCommunication_initMessage(&SteeringWheel.msgObj4, &config);
	}
#endif
	//~RX init
}

void SteeringWheel_run_1ms(void)
{
#ifdef __SDP_CLOVER__
	if(CanCommunication_receiveMessage(&SteeringWheel.msgObj4)) {
		SteeringWheel.canMsg4.U[0] = SteeringWheel.msgObj4.msg.data[0];
		SteeringWheel.canMsg4.U[1] = SteeringWheel.msgObj4.msg.data[1];
	}
#endif
}

void SteeringWheel_run_xms_c2(void) //10ms
{
	/* Shared variable update */
	while(IfxCpu_acquireMutex(&SteeringWheel_public.shared.mutex));	//Wait for mutex
	{
		SteeringWheel_public.data = SteeringWheel_public.shared.data;
		IfxCpu_releaseMutex(&SteeringWheel_public.shared.mutex);
	}

	/* Data parsing */
	SteeringWheel.canMsg1.S.vehicleSpeed = (uint8)SteeringWheel_public.data.vehicleSpeed;
	SteeringWheel.canMsg1.S.lowestVoltage = OrionBms2.msg3.lowVoltage;
	SteeringWheel.canMsg1.S.highestTemp = OrionBms2.msg3.highTemp;
	SteeringWheel.canMsg1.S.bmsTemp = OrionBms2.msg3.bmsTemp;
	SteeringWheel.canMsg1.S.soc = OrionBms2.msg1.packSoc;
	SteeringWheel.canMsg1.S.averageTemp = OrionBms2.msg3.avgTemp;
	SteeringWheel.canMsg1.S.status.S.r2d = ((SteeringWheel_public.data.isReadyToDrive & 0x1) << 3) |
	                                       ((SteeringWheel_public.data.isBppsChecked2 & 0x1) << 2) |
	                                       ((SteeringWheel_public.data.isBppsChecked1 & 0x1) << 1) |
	                                       ((SteeringWheel_public.data.isAppsChecked & 0x1) << 0);
	SteeringWheel.canMsg1.S.status.S.appsError = SteeringWheel_public.data.appsError;
	SteeringWheel.canMsg1.S.status.S.bppsError = SteeringWheel_public.data.bppsError;
	SteeringWheel.canMsg1.S.status.S.bpps0_On = SteeringWheel_public.data.bpps0_On;
	SteeringWheel.canMsg1.S.status.S.bpps1_On = SteeringWheel_public.data.bpps1_On;

	SteeringWheel.canMsg2.S.apps = (uint16)(SteeringWheel_public.data.apps*100);
	//SteeringWheel.canMsg2.S.bpps = (uint16)(SteeringWheel_public.data.bpps*100); //this is not the case, since the unit of bpps is
	double brakePercentage;
	if(SteeringWheel_public.data.bpps < _MIN_BP_PER_BRAKE_LINE_) {
		brakePercentage = 0.0;
	}
	else if(SteeringWheel_public.data.bpps > _MAX_BP_PER_BRAKE_LINE_) {
		brakePercentage = 100.0;
	}
	else {
		brakePercentage = (double)SteeringWheel_public.data.bpps / (_MAX_BP_PER_BRAKE_LINE_);
		brakePercentage *= 100;
	}
	SteeringWheel.canMsg2.S.bpps = (uint16)(brakePercentage * 100);
	SteeringWheel.canMsg2.S.lvBatteryVoltage = (uint16)(SteeringWheel_public.data.lvBatteryVoltage*100);
	SteeringWheel.canMsg2.S.accumulatorVoltage = OrionBms2.msg1.packVoltage;
/*
	SteeringWheel.canMsg3.S.inverterFLTemp = INV_FL_AMK_Actual_Values2.S.AMK_TempInverter;
	SteeringWheel.canMsg3.S.motorFLTemp = INV_FL_AMK_Actual_Values2.S.AMK_TempMotor;
	SteeringWheel.canMsg3.S.inverterRLTemp = INV_RL_AMK_Actual_Values2.S.AMK_TempInverter;
	SteeringWheel.canMsg3.S.motorRLTemp = INV_RL_AMK_Actual_Values2.S.AMK_TempMotor;
	SteeringWheel.canMsg3.S.inverterRRTemp = INV_RR_AMK_Actual_Values2.S.AMK_TempInverter;
	SteeringWheel.canMsg3.S.motorRRTemp = INV_RR_AMK_Actual_Values2.S.AMK_TempMotor;
	SteeringWheel.canMsg3.S.inverterFRTemp = INV_FR_AMK_Actual_Values2.S.AMK_TempInverter;
	SteeringWheel.canMsg3.S.motorFRTemp = INV_FR_AMK_Actual_Values2.S.AMK_TempMotor;
*/

	SteeringWheel.canMsg3.S.inverter1Temp = Inverter_L_Status.Temperature1.S.PM100_GateDriverBoardTemperature/10;
	SteeringWheel.canMsg3.S.motor1Temp = Inverter_L_Status.Temperature3.S.PM100_MotorTemperature;
	SteeringWheel.canMsg3.S.inverter2Temp = Inverter_R_Status.Temperature1.S.PM100_GateDriverBoardTemperature/10;
	SteeringWheel.canMsg3.S.motor2Temp = Inverter_R_Status.Temperature3.S.PM100_MotorTemperature;
	SteeringWheel.canMsg3.S.packPower = SteeringWheel_public.data.packPower;

	/* Set the messages */
	CanCommunication_setMessageData(SteeringWheel.canMsg1.U[0], SteeringWheel.canMsg1.U[1], &SteeringWheel.msgObj1);
	CanCommunication_setMessageData(SteeringWheel.canMsg2.U[0], SteeringWheel.canMsg2.U[1], &SteeringWheel.msgObj2);
	CanCommunication_setMessageData(SteeringWheel.canMsg3.U[0], SteeringWheel.canMsg3.U[1], &SteeringWheel.msgObj3);

	/* Transmit the messages */
	CanCommunication_transmitMessage(&SteeringWheel.msgObj1);
	CanCommunication_transmitMessage(&SteeringWheel.msgObj2);
	CanCommunication_transmitMessage(&SteeringWheel.msgObj3);
}

void SteeringWheel_readRSW1(uint8* RSW1) {
	*RSW1 = SteeringWheel.canMsg4.S.RSW1;
}

void SteeringWheel_readRSW2(uint8* RSW2) {
	*RSW2 = SteeringWheel.canMsg4.S.RSW2;
}

void SteeringWheel_readRSW3(uint8* RSW3) {
	*RSW3 = SteeringWheel.canMsg4.S.RSW3;
}

void SteeringWheel_readRSW(uint8* RSW1, uint8* RSW2, uint8* RSW3) {
	*RSW1 = SteeringWheel.canMsg4.S.RSW1;
	*RSW2 = SteeringWheel.canMsg4.S.RSW2;
	*RSW3 = SteeringWheel.canMsg4.S.RSW3;
}

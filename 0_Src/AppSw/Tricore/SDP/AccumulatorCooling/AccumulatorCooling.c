/*
* AccumulatorCooling.c
* Created: Jan 31st, 2023
* Author: Terry
* yoonsb@hanyang.ac.kr
* blog.terrysyoon.com
*/

#include "AccumulatorCooling.h"

AccumulatorCooling_t AccumulatorCooling;

void SDP_AccumulatorCooling_init(void){
	/*Tx~*/
    {
    	CanCommunication_Message_Config config;
    	config.messageId		=	ACCUMULATORCOOLING_CAN_MSGORDER_ID;
    	config.frameType		=	IfxMultican_Frame_transmit;
    	config.dataLen			=	IfxMultican_DataLengthCode_8;
    	config.node				=	&CanCommunication_canNode0;
    	CanCommunication_initMessage(&AccumulatorCooling.msgObj1, &config);
    }
    /*~Tx*/


    /*Rx~*/
    {
    	CanCommunication_Message_Config config;
    	config.messageId		=	ACCUMULATORCOOLING_CAN_MSGBATTERYDIAGNOSE_ID;
    	config.frameType		=	IfxMultican_Frame_receive;
    	config.dataLen			=	IfxMultican_DataLengthCode_8;
    	config.node				=	&CanCommunication_canNode0;
    	CanCommunication_initMessage(&AccumulatorCooling.msgObj2, &config);
    }
    {
    	CanCommunication_Message_Config config;
    	config.messageId		=	ACCUMULATORCOOLING_CAN_MSGFANSTATUSDATA_ID;
    	config.frameType		=	IfxMultican_Frame_receive;
    	config.dataLen			=	IfxMultican_DataLengthCode_8;
    	config.node				=	&CanCommunication_canNode0;
    	CanCommunication_initMessage(&AccumulatorCooling.msgObj3, &config);
    }
    {
    	CanCommunication_Message_Config config;
    	config.messageId		=	ACCUMULATORCOOLING_CAN_MSGORDERECHO_ID;
    	config.frameType		=	IfxMultican_Frame_receive;
    	config.dataLen			=	IfxMultican_DataLengthCode_8;
    	config.node				=	&CanCommunication_canNode0;
    	CanCommunication_initMessage(&AccumulatorCooling.msgObj4, &config);
    }
    {
    	CanCommunication_Message_Config config;
    	config.messageId		=	ACCUMULATORCOOLING_CAN_MSGFANTARGETDUTY_ID;
    	config.frameType		=	IfxMultican_Frame_receive;
    	config.dataLen			=	IfxMultican_DataLengthCode_8;
    	config.node				=	&CanCommunication_canNode0;
    	CanCommunication_initMessage(&AccumulatorCooling.msgObj5, &config);
    }
    /*~Rx*/
}

void SDP_AccumulatorCooling_run_10ms(void){

	/*Tx~*/
    CanCommunication_setMessageData(AccumulatorCooling.AccumulatorCooling_msgOrder.TransmitData[0],AccumulatorCooling.AccumulatorCooling_msgOrder.TransmitData[1],&AccumulatorCooling.msgObj1);
    CanCommunication_transmitMessage(&AccumulatorCooling.msgObj1);
    /*~Tx*/

    /*Rx~*/
    if(CanCommunication_receiveMessage(&AccumulatorCooling.msgObj2))
    {
    	AccumulatorCooling.AccumulatorCooling_msgBatteryDiagnose.ReceivedData[0] = AccumulatorCooling.msgObj2.msg.data[0];
    	AccumulatorCooling.AccumulatorCooling_msgBatteryDiagnose.ReceivedData[1] = AccumulatorCooling.msgObj2.msg.data[1];
    }
    if(CanCommunication_receiveMessage(&AccumulatorCooling.msgObj3))
    {
    	AccumulatorCooling.AccumulatorCooling_msgFanStatusData.ReceivedData[0] = AccumulatorCooling.msgObj3.msg.data[0];
       	AccumulatorCooling.AccumulatorCooling_msgFanStatusData.ReceivedData[1] = AccumulatorCooling.msgObj3.msg.data[1];
    }
    if(CanCommunication_receiveMessage(&AccumulatorCooling.msgObj4))
    {
       	AccumulatorCooling.AccumulatorCooling_msgOrderEcho.ReceivedData[0] = AccumulatorCooling.msgObj4.msg.data[0];
       	AccumulatorCooling.AccumulatorCooling_msgOrderEcho.ReceivedData[1] = AccumulatorCooling.msgObj4.msg.data[1];
    }
    if(CanCommunication_receiveMessage(&AccumulatorCooling.msgObj5))
    {
       	AccumulatorCooling.AccumulatorCooling_msgFanTargetDuty.ReceivedData[0] = AccumulatorCooling.msgObj5.msg.data[0];
       	AccumulatorCooling.AccumulatorCooling_msgFanTargetDuty.ReceivedData[1] = AccumulatorCooling.msgObj5.msg.data[1];
    }
    /*~Rx*/
}

/* TO-DO: Probably it might make more sense to introduce a new proprietary data structure in sake of capsulizing,
 * namely (struct commandSet), since otherwise we will be left with a function with 5 parameters dangling every single time.
*/

/*
 * Set command CAN message.
 * */
void SDP_AccumulatorCooling_setOrder(uint8 ControlMode, uint8 DutyOrder_SegmentIntake70, uint8 DutyOrder_SegmentExhaust80) {
	AccumulatorCooling.AccumulatorCooling_msgOrder.S.TCControlMode = ControlMode;
	AccumulatorCooling.AccumulatorCooling_msgOrder.S.TCFanDutyOrder_SideIntake = 100;
	AccumulatorCooling.AccumulatorCooling_msgOrder.S.TCFanDutyOrder_SegmentIntake70 = DutyOrder_SegmentIntake70;
	AccumulatorCooling.AccumulatorCooling_msgOrder.S.TCFanDutyOrder_SegmentExhaust60 = 100;
	AccumulatorCooling.AccumulatorCooling_msgOrder.S.TCFanDutyOrder_SegmentExhaust80 = DutyOrder_SegmentExhaust80;
}
//Wouldn't it be better to have lower bound value and upper bound value separated?

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
    CanCommunication_Message_Config config;
    config.messageId		=	ACCUMULATORCOOLING_CAN_MSGORDER_ID;
    config.frameType		=	IfxMultican_Frame_transmit;
    config.dataLen			=	IfxMultican_DataLengthCode_8;
    config.node				=	&CanCommunication_canNode0;
    CanCommunication_initMessage(&AccumulatorCooling.msgObj1, &config);
}

void SDP_AccumulatorCooling_run_10ms(void){
    CanCommunication_setMessageData(AccumulatorCooling.AccumulatorCooling_msgOrder.TxData[0],AccumulatorCooling.AccumulatorCooling_msgOrder.TxData[1],&AccumulatorCooling.msgObj1);
    CanCommunication_transmitMessage(&AccumulatorCooling.msgObj1);
}

void SDP_AccumulatorCooling_setOrder(uint16 ControlMode, uint16 FanDutyOrder) {
	AccumulatorCooling.AccumulatorCooling_msgOrder.S.TCControlMode = ControlMode;
	AccumulatorCooling.AccumulatorCooling_msgOrder.S.TCFanDutyOrder = FanDutyOrder;
}
//Wouldn't it be better to have lower bound value and upper bound value separated?

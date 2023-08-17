#include "Clover_CanGateway.h"

#define _NUM_GATEWAY_MSG_ (4)

CanCommunication_Message VCUtoDashboard[_NUM_GATEWAY_MSG_]; //RX
CanCommunication_Message DashboardToSteering[_NUM_GATEWAY_MSG_]; //TX

void Clover_CanGateway_init(){
	const uint32 GatewayMsg_ID[_NUM_GATEWAY_MSG_] = {0x00101F00UL, 0x00101F01UL, 0x00101F02UL, 0x00101F10UL}; //StWhlMsgId1, StWhlMsgId2, StWhlMsgId3, StWhlRSWMsgId
	for(int i = 0; i < _NUM_GATEWAY_MSG_; i++) {
		{
	        CanCommunication_Message_Config config;
	        config.messageId		=	GatewayMsg_ID[i];
	        config.frameType		=	IfxMultican_Frame_receive;
	        config.dataLen			=	IfxMultican_DataLengthCode_8;
	        config.node				=	&CanCommunication_canNode0;
	        CanCommunication_Gateway_initMessage(&VCUtoDashboard[i], &config);
		}
		{
			CanCommunication_Message_Config config;
	        config.messageId		=	GatewayMsg_ID[i];
	        config.frameType		=	IfxMultican_Frame_transmit;
	        config.dataLen			=	IfxMultican_DataLengthCode_8;
	        config.node				=	&CanCommunication_canNode1;
	        CanCommunication_initMessage(&DashboardToSteering[i], &config);
		}
	}
}

uint32 tick_1ms = 0;
uint32 tick_10ms = 0;

void Clover_CanGateway_run_1ms() { //10ms
	for(int i = 0; i < _NUM_GATEWAY_MSG_; i++) {
	    if(CanCommunication_receiveMessage(&VCUtoDashboard[i]))
	    {
	    	CanCommunication_setMessageData(VCUtoDashboard[i].msg.data[0],VCUtoDashboard[i].msg.data[1],&DashboardToSteering[i]);
	    }
	    CanCommunication_transmitMessage(&DashboardToSteering[i]);
	}
	tick_1ms++;
}

void Clover_CanGateway_run_10ms() { //10ms
	for(int i = 0; i < _NUM_GATEWAY_MSG_; i++) {
	    CanCommunication_transmitMessage(&DashboardToSteering[i]);
	}
	tick_10ms++;
}

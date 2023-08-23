#include "Clover_CanGateway.h"

#define _NUM_GATEWAY_MSG_FROM_VCU_ (3)
#define _NUM_GATEWAY_MSG_FROM_STEERING_ (1)

CanCommunication_Message VCUtoDashboard_FROM_VCU[_NUM_GATEWAY_MSG_FROM_VCU_]; //RX
CanCommunication_Message DashboardToSteering_FROM_VCU[_NUM_GATEWAY_MSG_FROM_VCU_]; //TX

CanCommunication_Message VCUtoDashboard_FROM_STEERING[_NUM_GATEWAY_MSG_FROM_STEERING_]; //RX
CanCommunication_Message DashboardToSteering_FROM_STEERING[_NUM_GATEWAY_MSG_FROM_STEERING_]; //TX
void Clover_CanGateway_init(){
	const uint32 GatewayMsg_ID_FROM_VCU[_NUM_GATEWAY_MSG_FROM_VCU_] = {0x00101F00UL, 0x00101F01UL, 0x00101F02UL}; //StWhlMsgId1, StWhlMsgId2, StWhlMsgId3, StWhlRSWMsgId
	const uint32 GatewayMsg_ID_FROM_STEERING[_NUM_GATEWAY_MSG_FROM_STEERING_] = {0x00101F10UL};
	for(int i = 0; i < _NUM_GATEWAY_MSG_FROM_VCU_; i++) {
		{
	        CanCommunication_Message_Config config;
	        config.messageId		=	GatewayMsg_ID_FROM_VCU[i];
	        config.frameType		=	IfxMultican_Frame_receive;
	        config.dataLen			=	IfxMultican_DataLengthCode_8;
	        config.node				=	&CanCommunication_canNode0;
	        CanCommunication_Gateway_initMessage(&VCUtoDashboard_FROM_VCU[i], &config);
		}
		{
			CanCommunication_Message_Config config;
	        config.messageId		=	GatewayMsg_ID_FROM_VCU[i];
	        config.frameType		=	IfxMultican_Frame_transmit;
	        config.dataLen			=	IfxMultican_DataLengthCode_8;
	        config.node				=	&CanCommunication_canNode1;
	        CanCommunication_initMessage(&DashboardToSteering_FROM_VCU[i], &config);
		}
	}
	for(int i = 0; i < _NUM_GATEWAY_MSG_FROM_STEERING_; i++) {
		{
	        CanCommunication_Message_Config config;
	        config.messageId		=	GatewayMsg_ID_FROM_STEERING[i];
	        config.frameType		=	IfxMultican_Frame_receive;
	        config.dataLen			=	IfxMultican_DataLengthCode_8;
	        config.node				=	&CanCommunication_canNode1;
	        CanCommunication_Gateway_initMessage(&VCUtoDashboard_FROM_STEERING[i], &config);
		}
		{
			CanCommunication_Message_Config config;
	        config.messageId		=	GatewayMsg_ID_FROM_STEERING[i];
	        config.frameType		=	IfxMultican_Frame_transmit;
	        config.dataLen			=	IfxMultican_DataLengthCode_8;
	        config.node				=	&CanCommunication_canNode0;
	        CanCommunication_initMessage(&DashboardToSteering_FROM_STEERING[i], &config);
		}
	}
}

uint32 tick_1ms = 0;
uint32 tick_10ms = 0;

void Clover_CanGateway_run_1ms() { //10ms
	for(int i = 0; i < _NUM_GATEWAY_MSG_FROM_VCU_; i++) {
	    if(CanCommunication_receiveMessage(&VCUtoDashboard_FROM_VCU[i]))
	    {
	    	CanCommunication_setMessageData(VCUtoDashboard_FROM_VCU[i].msg.data[0],VCUtoDashboard_FROM_VCU[i].msg.data[1],&DashboardToSteering_FROM_VCU[i]);
	    }
	    //CanCommunication_transmitMessage(&DashboardToSteering[i]);
	}
	for(int i = 0; i < _NUM_GATEWAY_MSG_FROM_STEERING_; i++) {
	    if(CanCommunication_receiveMessage(&VCUtoDashboard_FROM_STEERING[i]))
	    {
	    	CanCommunication_setMessageData(VCUtoDashboard_FROM_STEERING[i].msg.data[0],VCUtoDashboard_FROM_STEERING[i].msg.data[1],&DashboardToSteering_FROM_STEERING[i]);
	    }
	    //CanCommunication_transmitMessage(&DashboardToSteering[i]);
	}
	tick_1ms++;
}

void Clover_CanGateway_run_10ms() { //10ms
	for(int i = 0; i < _NUM_GATEWAY_MSG_FROM_VCU_; i++) {
	    CanCommunication_transmitMessage(&DashboardToSteering_FROM_VCU[i]);
	}
	for(int i = 0; i < _NUM_GATEWAY_MSG_FROM_STEERING_; i++) {
	    CanCommunication_transmitMessage(&DashboardToSteering_FROM_STEERING[i]);
	}
	tick_10ms++;
}

#include "DashBoardCan.h"
#include "RVC_Clover.h"

#define STARTBTNPushedCanMSg 0x011

#define STARTBTNMirrorCanMSg 0x010
#define StartStatusUpdateCanMSg 0x012
#define StartStatusUpdateCanMSg_echo 0x013

StartBtnPushed_t StartBtnPushed;
StartBtnPushed_t StartBtnMirror;

CanCommunication_Message StartBtnPushedMsg;
CanCommunication_Message StartBtnMirrorMsg;
/*
StartStatusUpdate_t StartStatusUpdate;
StartStatusUpdate_t StartStatusUpdate_echo;
CanCommunication_Message StartStatusUpdateMsg;
CanCommunication_Message StartStatusUpdateMsg_echo;
*/
const uint32 DshBrdMsg0 = 0x00081F00UL;
DashBoardMsg0_t DashBoard_canMsg0;
CanCommunication_Message DashBoard_msgObj0;
    
boolean RTD_flag;
boolean pastRTD_flag = 0;

boolean DashBoard_RTD_Status = 0;

volatile DashBoard_RTD_Status_UpdateRequest_t DashBoard_RTD_Status_UpdateRequest = 0;

// CanCommunication_Message ShockCanMsg1;

void SDP_DashBoardCan_init(void);
void SDP_DashBoardCan_run_1ms(void);
uint8 SDP_DashBoardCan_getDashBoard_RTD_Status();

void SDP_DashBoardCan_reset_pastRTD(void);

void SDP_DashBoardCan_init(void){
    /* CAN message init */
	{
        CanCommunication_Message_Config config;
        config.messageId		=	STARTBTNPushedCanMSg;
        config.frameType		=	IfxMultican_Frame_receive;
        config.dataLen			=	IfxMultican_DataLengthCode_8;
        config.node				=	&CanCommunication_canNode0;
        CanCommunication_initMessage(&StartBtnPushedMsg, &config);
	}
    {
        CanCommunication_Message_Config config;
        config.messageId		=	STARTBTNMirrorCanMSg;
        config.frameType		=	IfxMultican_Frame_transmit;
        config.dataLen			=	IfxMultican_DataLengthCode_8;
        config.node				=	&CanCommunication_canNode0;
        CanCommunication_initMessage(&StartBtnMirrorMsg, &config);
	}/*
	{
		CanCommunication_Message_Config config;
		config.messageId = StartStatusUpdateCanMSg;
		config.frameType = IfxMultican_Frame_transmit;
		config.dataLen = IfxMultican_DataLengthCode_8;
		config.node = &CanCommunication_canNode0;
		CanCommunication_initMessage(&StartStatusUpdateMsg, &config);
	}
	{
		CanCommunication_Message_Config config;
		config.messageId = StartStatusUpdateCanMSg_echo;
		config.frameType = IfxMultican_Frame_receive;
		config.dataLen = IfxMultican_DataLengthCode_8;
		config.node = &CanCommunication_canNode0;
		CanCommunication_initMessage(&StartStatusUpdateMsg_echo, &config);
	}*/
	{
		CanCommunication_Message_Config config;
		config.messageId = DshBrdMsg0;
		config.frameType = IfxMultican_Frame_transmit;
		config.dataLen = IfxMultican_DataLengthCode_8;
		config.node = &CanCommunication_canNode0;
		CanCommunication_initMessage(&DashBoard_msgObj0, &config);
	}
}

void SDP_DashBoardCan_reset_pastRTD() {
	pastRTD_flag = 0;
	StartBtnPushed.RxData[0] = 0;
	StartBtnPushed.RxData[1] = 0;
	DashBoard_RTD_Status = 0;
}

void SDP_DashBoardCan_run_1ms(void){
    if(CanCommunication_receiveMessage(&StartBtnPushedMsg))
    {
    	StartBtnPushed.RxData[0]      =   StartBtnPushedMsg.msg.data[0];
    	StartBtnPushed.RxData[1]      =   StartBtnPushedMsg.msg.data[1];
    }

    RTD_flag = StartBtnPushed.B.StartBtnPushed;

    if(RTD_flag == 1 && pastRTD_flag ==0){
    	DashBoard_RTD_Status = 1;
    	/* RVC_setR2d에서
        HLD_GtmTomBeeper_start(InvStartPattern);
        CascadiaInverter_initParameterWrite();
        */
    }
    else if(RTD_flag ==0 && pastRTD_flag == 1){
    	DashBoard_RTD_Status = 0;
    	/* RVC_resetR2d에서
        HLD_GtmTomBeeper_start(InvOffPattern);
        CascadiaInverter_disable();
        */
    }

    pastRTD_flag = RTD_flag;
}

void SDP_DashBoardCan_run_10ms(void) {
    if (StartBtnPushed.B.OFFvehicle || StartBtnPushed.B.StartBtnPushed ){

        CanCommunication_setMessageData(StartBtnPushed.RxData[0],StartBtnPushed.RxData[1], &StartBtnMirrorMsg);

        CanCommunication_transmitMessage(&StartBtnMirrorMsg);
    }
    SDP_DashBoardCan_updateInfo();
    SDP_DashBoardCan_updateSDCinfo();

}

boolean SDP_DashBoardCan_getDashBoard_RTD_Status(void) {
	return DashBoard_RTD_Status;
}

DashBoard_RTD_Status_UpdateRequest_t SDP_DashBoardCan_getDashBoard_RTD_UpdateRequest(void) {
	return DashBoard_RTD_Status_UpdateRequest;
}
/*
void SDP_DashBoardCan_setRTD_request(void) {
	DashBoard_RTD_Status_UpdateRequest = DashBoard_RTD_Status_UpdateRequest_set;
	StartStatusUpdate.RTD_Set = TRUE;
	StartStatusUpdate.RTD_Reset = FALSE;
	CanCommunication_setMessageData(StartStatusUpdate.TransmitData[0],StartStatusUpdate.TransmitData[1], &StartStatusUpdateMsg);
}

void SDP_DashBoardCan_resetRTD_request(void) {
	DashBoard_RTD_Status_UpdateRequest = DashBoard_RTD_Status_UpdateRequest_reset;
	StartStatusUpdate.RTD_Set = FALSE;
	StartStatusUpdate.RTD_Reset = TRUE;
	CanCommunication_setMessageData(StartStatusUpdate.TransmitData[0],StartStatusUpdate.TransmitData[1], &StartStatusUpdateMsg);
}

void SDP_DashBoardCan_setRTD(void) {
	if(DashBoard_RTD_Status_UpdateRequest == DashBoard_RTD_Status_UpdateRequest_set) {
		if(CanCommunication_receiveMessage(&StartStatusUpdateMsg_echo)) { //Echo arrived
			StartStatusUpdate_echo.TransmitData[0] = StartStatusUpdateMsg_echo.msg.data[0];
			StartStatusUpdate_echo.TransmitData[1] = StartStatusUpdateMsg_echo.msg.data[1];
		}
		if(StartStatusUpdate_echo.RTD_Set) {
			DashBoard_RTD_Status_UpdateRequest = DashBoard_RTD_Status_UpdateRequest_none;
		}
	}
}

void SDP_DashBoardCan_resetRTD(void) {
	if(DashBoard_RTD_Status_UpdateRequest == DashBoard_RTD_Status_UpdateRequest_reset) {
		if(CanCommunication_receiveMessage(&StartStatusUpdateMsg_echo)) { //Echo arrived
			StartStatusUpdate_echo.TransmitData[0] = StartStatusUpdateMsg_echo.msg.data[0];
			StartStatusUpdate_echo.TransmitData[1] = StartStatusUpdateMsg_echo.msg.data[1];
		}
		if(StartStatusUpdate_echo.RTD_Reset) {
			DashBoard_RTD_Status_UpdateRequest = DashBoard_RTD_Status_UpdateRequest_none;
		}
	}
}
*/

void SDP_DashBoardCan_updateInfo(void) {
	DashBoard_canMsg0.S.RTDOn = (RVC_public.readyToDrive.data == RVC_public_ReadyToDrive_status_run);
	//DashBoard_canMsg0.S.InverterTempWarning = 0;
	DashBoard_canMsg0.S.InverterTempWarning = (CascadiaInverter_getHotspotTemperature_RL() > 600 || CascadiaInverter_getHotspotTemperature_RR() > 600)?1:0; //Warning when 60 celcius or higher.
	//DashBoard_canMsg0.S.AccumulatorTempWarning = 0;
	DashBoard_canMsg0.S.AccumulatorTempWarning = (OrionBms2_getHighTemp() >= 52) ? 1:0; // Warning when 52 celcius or higher.
	sint8 invError = CascadiaInverter_getErrorStatus();
	DashBoard_canMsg0.S.InverterFault = invError?1:0;
}

void SDP_DashBoardCan_updateSDCinfo(void) {
	DashBoard_canMsg0.S.SdcAmsOk = RVC_public.sdc.data.SdcAmsOk;
	DashBoard_canMsg0.S.SdcImdOk = RVC_public.sdc.data.SdcImdOk;
	DashBoard_canMsg0.S.SdcBspdOk = RVC_public.sdc.data.SdcBspdOk;
	DashBoard_canMsg0.S.SdcFinalOn = RVC_public.sdc.data.SdcFinalOn;

	CanCommunication_setMessageData(DashBoard_canMsg0.TransmitData[0], DashBoard_canMsg0.TransmitData[1], &DashBoard_msgObj0);
	CanCommunication_transmitMessage(&DashBoard_msgObj0);
}

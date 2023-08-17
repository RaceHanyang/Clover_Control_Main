#include "HLD.h"
#include <Port/Io/IfxPort_Io.h>
#include "Configuration.h"
#include "CanCommunication.h"
#include "Gpio_Debounce.h"




typedef union{
	uint32 RxData[2];
	struct{
		uint8 StartBtnPushed : 1;
        uint8 OFFvehicle : 1;
		uint32 Remain1 : 30;
        uint32 Remain2;
	}B;

}StartBtnPushed_t;

typedef enum
{
	DashBoard_RTD_Status_UpdateRequest_none = 0,
	DashBoard_RTD_Status_UpdateRequest_set = 1,
	DashBoard_RTD_Status_UpdateRequest_reset = 2,
}DashBoard_RTD_Status_UpdateRequest_t;

typedef union{
	uint32 TransmitData[2];
	struct{
		uint32 RTD_Set:1;
		uint32 RTD_Reset:1;
		uint32 Reserved0:30;
		uint32 Reserved1;
	};
}StartStatusUpdate_t;

typedef union
{
	uint32 TransmitData[2];
	struct
	{
		uint8 Inverter_L_Status; // Enable lockdown + 6
		uint8 Inverter_R_Status;
		boolean SdcAmsOk : 1;
		boolean SdcImdOk : 1;
		boolean SdcBspdOk : 1;
		boolean SdcFinalOn : 1;
		boolean RTDOn : 1;
		boolean InverterTempWarning : 1;
		boolean AccumulatorTempWarning : 1;
		boolean InverterFault: 1;
		uint8 reserved1;
		uint16 reserved2;
	}S;
}DashBoardMsg0_t;

IFX_EXTERN StartBtnPushed_t StartBtnPushed;
IFX_EXTERN StartBtnPushed_t StartBtnMirror;

IFX_EXTERN void SDP_DashBoardCan_init(void);
IFX_EXTERN void SDP_DashBoardCan_run_1ms(void);
IFX_EXTERN boolean SDP_DashBoardCan_getDashBoard_RTD_Status(void);

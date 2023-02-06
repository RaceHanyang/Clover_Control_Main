/*
* AccumulatorCooling.h
* Created: Jan 31st, 2023
* Author: Terry
* yoonsb@hanyang.ac.kr
* blog.terrysyoon.com
*/

#ifndef SRC_APPSW_TRICORE_SDP_ACCUMULATORCOOLING_H
#define SRC_APPSW_TRICORE_SDP_ACCUMULATORCOOLING_H

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define ACCUMULATORCOOLING_CAN_MSGORDER_ID (0x275B01)
#define ACCUMULATORCOOLING_CAN_MSGBATTERYDIAGNOSE_ID (0x334C01)
#define ACCUMULATORCOOLING_CAN_MSGFANSTATUSDATA_ID (0x334C02)
#define ACCUMULATORCOOLING_CAN_MSGORDERECHO_ID (0x334C03)
#define ACCUMULATORCOOLING_CAN_MSGFANTARGETDUTY_ID (0x334C04)

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "SDP.h"
#include "CanCommunication.h"
#include "HLD.h"

/******************************************************************************/
/*------------------------------Data Structures-------------------------------*/
/******************************************************************************/

/* TX
 * To: Accumulator Cooling Controller
 * TX ID: 0x275B01
 * Send duty command for 4 cooling clusters, in range of 0 to 100.
 *
 *
 * RX
 * FROM: Accumulator Cooling Controller
 * RX ID: 0x334C03
 * Hear echo of the TX messgae above.
 * The echo must be identical to the sent command message described right above.
 * */
typedef union
{
	union{
		uint32 TransmitData[2];
		uint32 ReceivedData[2];
	};
	struct{
		uint8 TCControlMode	; //0: not TC control mode, 1: TC Control mode
		uint8 TCFanDutyOrder_SideIntake ;
		uint8 TCFanDutyOrder_SegmentIntake70;
		uint8 TCFanDutyOrder_SegmentExhaust60;
		uint8 TCFanDutyOrder_SegmentExhaust80;
		uint8 Remain1		;
		uint8 Remain2		;
		uint8 Remain3		;
	}S;
}AccumulatorCooling_msgOrder_t;



/*
 * RX
 * FROM: Accumulator Cooling Controller
 * RX ID: 0x334C01
 * Data: Precharge state1&2, relaycontact signal 1&2&3, TSAL signal, IMD status
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		uint8 prechargeStateSignal1 ;
		uint8 prechargeStateSignal2 ;
		uint8 RelayContactSignal1   ;
		uint8 RelayContactSignal2	  ;
		uint8 RelayContactSignal3	  ;
		uint8 TsalSignal			  ;
		uint8 IMDStatusFrequency	  ;
		uint8 Reserved			  ;
	}S;
}AccumulatorCooling_msgBatteryDiagnose_t;


/*
 * RX
 * FROM: Accumulator Cooling Controller
 * RX ID: 0x334C02
 *
 *  * Data: Fan Flag, TIM 15, 16, 17 Dutycycle & Frequency
 *
 ***Notation
 * Ex) if fanflag == 1, send frequency&duty cycle of no.7,4,1 fans.
 * Fanflag = 1: tim15: 7
 * 			 tim16: 4
 * 			 tim17: 1
 * Fanflag = 2: tim15: 8
 * 			 tim16: 5
 * 			 tim17: 2
 * Fanflag = 3: tim15: 9
 * 			 tim16: 6
 * 			 tim17: 3
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		uint8 FanFlag			;
		uint8 TIM15_Dutycycle ;
		uint8 TIM15_Frequency ;
		uint8 TIM16_Dutycycle ;
		uint8 TIM16_Frequency ;
		uint8 TIM17_Dutycycle ;
		uint8 TIM17_Frequency ;
		uint8 desiredDuty;
	}S;
}AccumulatorCooling_msgFanStatusData_t;

/* RX
 * FROM: Accumulator Cooling Controller
 * RX ID: 0x334C04
 * Receive duty command being sent to physical fans, in range of 0 to 100.
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		uint8 TargetDuty_SideIntake ;
		uint8 TargetDuty_SegmentIntake70;
		uint8 TargetDuty_SegmentExhaust60;
		uint8 TargetDuty_SegmentExhaust80;
		uint8 Remain1;
		uint8 Remain2;
		uint8 Remain3;
		uint8 Remain4;
	}S;
}AccumulatorCooling_msgFanTargetDuty_t;

typedef struct{
/*Tx~*/
	CanCommunication_Message msgObj1;
	AccumulatorCooling_msgOrder_t AccumulatorCooling_msgOrder;
/*~Tx*/

/*Rx~*/
	/*0x334C01~*/
	CanCommunication_Message msgObj2;
	AccumulatorCooling_msgBatteryDiagnose_t AccumulatorCooling_msgBatteryDiagnose;
	/*~0x334C01*/

	/*0x334C02~*/
	CanCommunication_Message msgObj3;
	AccumulatorCooling_msgFanStatusData_t AccumulatorCooling_msgFanStatusData;
	/*~0x334C02*/

	/*0x334C03~*/
	CanCommunication_Message msgObj4;
	AccumulatorCooling_msgOrder_t AccumulatorCooling_msgOrderEcho;
	/*~0x334C03*/

	/*0x334C04~*/
	CanCommunication_Message msgObj5;
	AccumulatorCooling_msgFanTargetDuty_t AccumulatorCooling_msgFanTargetDuty;
	/*~0x334C04*/
/*~Rx*/
}AccumulatorCooling_t;

IFX_EXTERN AccumulatorCooling_t AccumulatorCooling;

IFX_EXTERN void SDP_AccumulatorCooling_init(void);
IFX_EXTERN void SDP_AccumulatorCooling_run_10ms(void);
IFX_EXTERN void SDP_AccumulatorCooling_setOrder(uint8 ControlMode, uint8 DutyOrder_SegmentIntake70, uint8 DutyOrder_SegmentExhaust80);

#endif

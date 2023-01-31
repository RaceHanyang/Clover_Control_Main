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

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "SDP.h"
#include "CanCommunication.h"
#include "HLD.h"

/******************************************************************************/
/*------------------------------Data Structures-------------------------------*/
/******************************************************************************/

typedef union
{
	uint32 TxData[2];
	struct{
		uint16 TCControlMode;
		uint16 TCFanDutyOrder;
		uint32 Reserved;
	}S;
}AccumulatorCooling_msgOrder_t;

typedef struct{
	CanCommunication_Message msgObj1;
	AccumulatorCooling_msgOrder_t AccumulatorCooling_msgOrder;
}AccumulatorCooling_t;

IFX_EXTERN AccumulatorCooling_t AccumulatorCooling;

IFX_EXTERN void SDP_AccumulatorCooling_init(void);
IFX_EXTERN void SDP_AccumulatorCooling_run_10ms(void);
IFX_EXTERN void SDP_AccumulatorCooling_setOrder(uint16 ControlMode, uint16 FanDutyOrder);

#endif

/*
 * SteeringWheel.h
 * Created on: 2020.08.05
 * Author: Dua
 */
#ifndef STEERINGWHEEL_H_
#define STEERINGWHEEL_H_
/***************************** Includes ******************************/
#include "HLD.h"
#include "CanCommunication.h"
#include "RVC.h"
#ifdef __SDP_CLOVER__
#include "Cascadia_Inverter_can.h"
#endif
#ifndef __SDP_CLOVER__
#include "AmkInverter_can.h"
#endif
#include "OrionBms2.h"

#include "SteeringWheel_canMessage.h"

/**************************** Macro **********************************/


/************************* Data Structures ***************************/
typedef struct 
{
	float32 vehicleSpeed;
	float32 apps;
	float32 bpps;
	boolean isReadyToDrive;
	boolean isAppsChecked;
	boolean isBppsChecked1;
	boolean isBppsChecked2;
	boolean appsError;
	boolean bppsError;
	float32 lvBatteryVoltage;
#ifdef __SDP_CLOVER__
	float32 brakePressure1;
	float32 brakePressure2;
	float32 wssFL;
	float32 wssFR;
	float32 wssRL;
	float32 wssRR;
	float32 sta_raw;
	float32 sta;
	float32 torqueFL;
	float32 torqueFR;
	float32 torqueRL;
	float32 torqueRR;
#endif
}SteeringWheel_public_data_t;

typedef struct 
{
	SteeringWheel_public_data_t data;
	struct 
	{
		SteeringWheel_public_data_t data;
		IfxCpu_mutexLock mutex;
	}shared;
}SteeringWheel_public_t;

/************************ Global Variables ***************************/
IFX_EXTERN SteeringWheel_public_t SteeringWheel_public;

/*********************** Function Prototypes *************************/
IFX_EXTERN void SteeringWheel_init(void);
IFX_EXTERN void SteeringWheel_run_xms_c2(void);

#endif

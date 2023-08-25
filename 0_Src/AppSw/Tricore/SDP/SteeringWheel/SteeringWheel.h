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
//#include "AmkInverter_can.h"
#include "Cascadia_Inverter_can.h"
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
	float32 packPower;

	boolean bpps0_On;
	boolean bpps1_On;
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
IFX_EXTERN void SteeringWheel_run_1ms(void);
IFX_EXTERN void SteeringWheel_run_xms_c2(void);

IFX_EXTERN void SteeringWheel_readRSW1(uint8*);
IFX_EXTERN void SteeringWheel_readRSW2(uint8*);
IFX_EXTERN void SteeringWheel_readRSW3(uint8*);
IFX_EXTERN void SteeringWheel_readRSW(uint8* RSW1, uint8* RSW2, uint8* RSW3);

#endif

/*
 * WheelSpeed_Clover.c
 * Created: July 24th 2023
 * Author: Terry
 * yoonsb@hanyang.ac.kr
 * */

#ifndef SRC_APPSW_TRICORE_SDP_WHEELSPEED_WHEELSPEED_CLOVER_H_
#define SRC_APPSW_TRICORE_SDP_WHEELSPEED_WHEELSPEED_CLOVER_H_

#include "SensorHub.h"

//Source Default: WSS, reset everytime RTD is set.
typedef struct
{
	uint16 RPM;
	float32 wheelLinearVelocity;
	boolean duty_errorFlag;
//Credibility check params~
	uint16 timeout_cnt;
	boolean timeout;

	uint16 ratio_outofbound_cnt;
	boolean ratio_outofbound;

	boolean source; //set 0 for WSS, 1 for inverter
//~Credibility
}SDP_WheelSpeed_sensor_t;

typedef struct
{
	SDP_WheelSpeed_sensor_t wssFL;
	SDP_WheelSpeed_sensor_t wssFR;
	SDP_WheelSpeed_sensor_t wssRL;
	SDP_WheelSpeed_sensor_t wssRR;

	struct
	{
		float32 frontAxle;
		float32 rearAxle;
		float32 chassis;
	}velocity;
}SDP_WheelSpeed_t;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
IFX_EXTERN SDP_WheelSpeed_t SDP_WheelSpeed;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
//IFX_EXTERN void SDP_WheelSpeed_init(void);
IFX_EXTERN void SDP_WheelSpeed_run_1ms(void);
IFX_EXTERN void SDP_WheelSpeed_setSourceWSS();
IFX_EXTERN void SDP_WheelSpeed_setSourceCascadia();
/******************************************************************************/
/*---------------------Inline Function Implementations------------------------*/
/******************************************************************************/
#endif

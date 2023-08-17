/*
 * WheelSpeed_Clover.c
 * Created: July 24th 2023
 * Author: Terry
 * yoonsb@hanyang.ac.kr
 * */

#include "SDP.h"
#include "WheelSpeed_Clover.h"

#ifdef __SDP_CLOVER__
/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define AVG2(x,y)	((x + y)/2)
#define ABS(x) (x>0?x:(-1)*x)

#define TYRE_RADIUS (0.254) //The outer radius of a tyre for Clover = 10 inches = 0.254 meters.

#define WSS_TIMEOUT (100) //timeout 100ms

#define GEAR_RATIO (4.667) //Gear ratio
#define WSS_TOLERANCE (150) //Unit: RPM. Credible if WSS RPM coincides with Cascadia inverter in this tolerance.
#define RANGE_OUTOFBOUND_TIMEOUT (100)
/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/


/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/


/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
SDP_WheelSpeed_t SDP_WheelSpeed;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
IFX_STATIC void SDP_WheelSpeed_update(void);
/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/


/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void SDP_WheelSpeed_run_1ms(void)
{
	SDP_WheelSpeed_update();
}

void SDP_WheelSpeed_setSourceWSS() {
	SDP_WheelSpeed.wssFL.timeout_cnt = 0;
	SDP_WheelSpeed.wssFL.timeout = 0;
	SDP_WheelSpeed.wssFL.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssFL.ratio_outofbound = 0;
	SDP_WheelSpeed.wssFL.source = 0;

	SDP_WheelSpeed.wssFR.timeout_cnt = 0;
	SDP_WheelSpeed.wssFR.timeout = 0;
	SDP_WheelSpeed.wssFR.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssFR.ratio_outofbound = 0;
	SDP_WheelSpeed.wssFR.source = 0;

	SDP_WheelSpeed.wssRL.timeout_cnt = 0;
	SDP_WheelSpeed.wssRL.timeout = 0;
	SDP_WheelSpeed.wssRL.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssRL.ratio_outofbound = 0;
	SDP_WheelSpeed.wssRL.source = 0;

	SDP_WheelSpeed.wssRR.timeout_cnt = 0;
	SDP_WheelSpeed.wssRR.timeout = 0;
	SDP_WheelSpeed.wssRR.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssRR.ratio_outofbound = 0;
	SDP_WheelSpeed.wssRR.source = 0;
}

void SDP_WheelSpeed_setSourceCascadia() {
	SDP_WheelSpeed.wssFL.timeout_cnt = 0;
	SDP_WheelSpeed.wssFL.timeout = 0;
	SDP_WheelSpeed.wssFL.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssFL.ratio_outofbound = 0;
	SDP_WheelSpeed.wssFL.source = 1;

	SDP_WheelSpeed.wssFR.timeout_cnt = 0;
	SDP_WheelSpeed.wssFR.timeout = 0;
	SDP_WheelSpeed.wssFR.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssFR.ratio_outofbound = 0;
	SDP_WheelSpeed.wssFR.source = 1;

	SDP_WheelSpeed.wssRL.timeout_cnt = 0;
	SDP_WheelSpeed.wssRL.timeout = 0;
	SDP_WheelSpeed.wssRL.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssRL.ratio_outofbound = 0;
	SDP_WheelSpeed.wssRL.source = 1;

	SDP_WheelSpeed.wssRR.timeout_cnt = 0;
	SDP_WheelSpeed.wssRR.timeout = 0;
	SDP_WheelSpeed.wssRR.ratio_outofbound_cnt = 0;
	SDP_WheelSpeed.wssRR.ratio_outofbound = 0;
	SDP_WheelSpeed.wssRR.source = 1;
}

IFX_STATIC void SDP_WheelSpeed_update(void)
{
	SDP_WheelSpeed.wssFL.RPM = SDP_SensorHub_getRPM_FL();
	SDP_WheelSpeed.wssFR.RPM = SDP_SensorHub_getRPM_FR();

	sint16 Inv_RPM_RL, Inv_RPM_RR;
	CascadiaInverter_getRPM(&Inv_RPM_RL, &Inv_RPM_RR);
	Inv_RPM_RL = ABS(Inv_RPM_RL);
	Inv_RPM_RR = ABS(Inv_RPM_RR);
	uint16 Wss_RPM_RL, Wss_RPM_RR;
	SDP_SensorHub_getRPM_Rear(&Wss_RPM_RL, &Wss_RPM_RR);

	if(SDP_WheelSpeed.wssRL.RPM == Wss_RPM_RL && Inv_RPM_RL != 0) {
		if(SDP_WheelSpeed.wssRL.timeout_cnt > WSS_TIMEOUT) {
			SDP_WheelSpeed.wssRL.timeout = 1;
		}
		else {
			SDP_WheelSpeed.wssRL.timeout_cnt++;
		}
	}else {
		SDP_WheelSpeed.wssRL.timeout_cnt = 0;
	}
	if(SDP_WheelSpeed.wssRR.RPM == Wss_RPM_RR && Inv_RPM_RR != 0) {
		if(SDP_WheelSpeed.wssRR.timeout_cnt > WSS_TIMEOUT) {
			SDP_WheelSpeed.wssRR.timeout = 1;
		}
		else {
			SDP_WheelSpeed.wssRR.timeout_cnt++;
		}
	}else {
		SDP_WheelSpeed.wssRR.timeout_cnt = 0;
	}

	if(ABS(Wss_RPM_RL - Inv_RPM_RL/GEAR_RATIO) > WSS_TOLERANCE) {
		SDP_WheelSpeed.wssRL.ratio_outofbound_cnt++;
		if(SDP_WheelSpeed.wssRL.ratio_outofbound_cnt > RANGE_OUTOFBOUND_TIMEOUT) {
			SDP_WheelSpeed.wssRL.ratio_outofbound = 1;
		}
	} else {
		SDP_WheelSpeed.wssRL.ratio_outofbound_cnt = 0;
	}

	if(ABS(Wss_RPM_RR - Inv_RPM_RR/GEAR_RATIO) > WSS_TOLERANCE) {
		SDP_WheelSpeed.wssRR.ratio_outofbound_cnt++;
		if(SDP_WheelSpeed.wssRR.ratio_outofbound_cnt > RANGE_OUTOFBOUND_TIMEOUT) {
			SDP_WheelSpeed.wssRR.ratio_outofbound = 1;
		}
	} else {
		SDP_WheelSpeed.wssRR.ratio_outofbound_cnt = 0;
	}

	SDP_WheelSpeed.wssRL.source = SDP_WheelSpeed.wssRL.timeout | SDP_WheelSpeed.wssRL.ratio_outofbound;
	SDP_WheelSpeed.wssRR.source = SDP_WheelSpeed.wssRR.timeout | SDP_WheelSpeed.wssRR.ratio_outofbound;

	if(SDP_WheelSpeed.wssRL.source || SDP_WheelSpeed.wssRR.source) {
		SDP_WheelSpeed.wssRL.source = 1;
		SDP_WheelSpeed.wssRR.source = 1;
		//CascadiaInverter_getRPM(&SDP_WheelSpeed.wssRL.RPM, &SDP_WheelSpeed.wssRR.RPM);
		SDP_WheelSpeed.wssRL.RPM = Inv_RPM_RL;
		SDP_WheelSpeed.wssRR.RPM = Inv_RPM_RR;
	}
	else {
		//SDP_SensorHub_getRPM_Rear(&SDP_WheelSpeed.wssRL.RPM, &SDP_WheelSpeed.wssRR.RPM); //Check positivity.
		SDP_WheelSpeed.wssRL.RPM = Wss_RPM_RL;
		SDP_WheelSpeed.wssRR.RPM = Wss_RPM_RR;
	}


	SDP_WheelSpeed.wssFL.wheelLinearVelocity = (SDP_WheelSpeed.wssFL.RPM/60) * TYRE_RADIUS;
	SDP_WheelSpeed.wssFR.wheelLinearVelocity = (SDP_WheelSpeed.wssFR.RPM/60) * TYRE_RADIUS;
	SDP_WheelSpeed.wssRL.wheelLinearVelocity = (SDP_WheelSpeed.wssRL.RPM/60) * TYRE_RADIUS;
	SDP_WheelSpeed.wssRR.wheelLinearVelocity = (SDP_WheelSpeed.wssRR.RPM/60) * TYRE_RADIUS;

	SDP_WheelSpeed.wssFL.duty_errorFlag = SDP_SensorHub_getRPMStatus_FL();
	SDP_WheelSpeed.wssFR.duty_errorFlag = SDP_SensorHub_getRPMStatus_FR();
	SDP_WheelSpeed.wssRL.duty_errorFlag = SDP_SensorHub_getRPMStatus_RL();
	SDP_WheelSpeed.wssRR.duty_errorFlag = SDP_SensorHub_getRPMStatus_RR();

	SDP_WheelSpeed.velocity.frontAxle = AVG2(SDP_WheelSpeed.wssFL.wheelLinearVelocity, SDP_WheelSpeed.wssFR.wheelLinearVelocity);
	SDP_WheelSpeed.velocity.rearAxle = AVG2(SDP_WheelSpeed.wssRL.wheelLinearVelocity, SDP_WheelSpeed.wssRR.wheelLinearVelocity);
	SDP_WheelSpeed.velocity.chassis = AVG2(SDP_WheelSpeed.velocity.frontAxle, SDP_WheelSpeed.velocity.rearAxle);
}
#endif

/*
 * TorqueVectoring.c
 * Created on 2019. 12. 10.
 * Author: Dua
 */

/* Includes */
#include "TorqueVectoring_Clover.h"
#include <math.h>

/* Macros */
// #define TVOPEN_LSD_ON		TRUE
// #define TVOPEN_LSD_GAIN		1.0f

/* Global Variables */
boolean TVOPEN_LSD_ON = FALSE;  //Set TRUE to trigger TV.
IFX_EXTERN RVC_t RVC;
float alpha; //give the margin torque about half of the torque that controlled from APPS
float maxTorque; //max torque for slower torque
float totalTorque; //total torque for both motors
float maxdiff; //will saturate at 0.5
float absDiffRear;
float slowerTorque;
float fasterTorque;
float diffratio;
float diffratio_a;
/* Function Implementation */
void RVC_TorqueVectoring_run_modeOpen(void)
{
	//diff = left - right
	//TODO: difflimit, maxdiff, alpha tuning needed
	if(TVOPEN_LSD_ON == TRUE && RVC.diff.error == FALSE && SDP_WheelSpeed.velocity.chassis > RVC.lsd.speedLow )	//TODO: Diff deadzone
	{
		diffratio = 0;
		diffratio_a = 0;
		if(RVC.diff.rear > 0){
			RVC.lsd.faster = RVC_Lsd_Faster_left;
			diffratio_a = SDP_WheelSpeed.wssRR.wheelLinearVelocity/SDP_WheelSpeed.wssRL.wheelLinearVelocity;
			diffratio = 1 - diffratio_a;
			if(isnan(diffratio)){
				RVC.lsd.faster = RVC_Lsd_Faster_none;
				diffratio = 0;
			}
		}
		else if(RVC.diff.rear < 0){
			RVC.lsd.faster = RVC_Lsd_Faster_right;
			diffratio_a = SDP_WheelSpeed.wssRL.wheelLinearVelocity/SDP_WheelSpeed.wssRR.wheelLinearVelocity;
			diffratio = 1 - diffratio_a;
			if(isnan(diffratio)){
				RVC.lsd.faster = RVC_Lsd_Faster_none;
				diffratio = 0;
			}
		}
		else{
			RVC.lsd.faster = RVC_Lsd_Faster_none;
		}

		alpha = RVC.torque.controlled / 2.0; //give the margin torque about half of the torque that controlled from APPS
		maxTorque = RVC.torque.controlled + alpha; //max torque for slower torque
		totalTorque = 2.0 * RVC.torque.controlled; //total torque for both motors
		maxdiff = 0.5; //will saturate at 0.5
		absDiffRear = fabs(RVC.diff.rear);
		slowerTorque = 0;
		fasterTorque = 0;
		//K: 2.5, L:5, M:1 diffLimit: 0.2
		if(diffratio > RVC.lsd.diffLimit) //in case the diff is really strong
		{
			// slowerTorque = RVC.torque.controlled * (RVC.lsd.lGain * (absDiffRear - RVC.lsd.diffLimit) +
			//                                            (1 + RVC.lsd.diffLimit * RVC.lsd.kGain * RVC.lsd.mGain));
			// fasterTorque = RVC.torque.controlled * (RVC.lsd.lGain * (-absDiffRear + RVC.lsd.diffLimit) +
			//                                            (1 - RVC.lsd.diffLimit * RVC.lsd.kGain));
			//given equation will saturate at 0.5 diffRear
			slowerTorque = ((alpha * (absDiffRear - RVC.lsd.diffLimit)) / (maxdiff - RVC.lsd.diffLimit)) + RVC.torque.controlled;
			fasterTorque = maxTorque - slowerTorque;
		}
		// else //in case the diff relatively strong
		// {
		// 	slowerTorque = RVC.torque.controlled * (1.0 + absDiffRear * RVC.lsd.kGain * RVC.lsd.mGain);
		// 	fasterTorque = RVC.torque.controlled * (1.0 - absDiffRear * RVC.lsd.kGain);
		// }
		//satuation pt
		if(slowerTorque > maxTorque){
			slowerTorque = maxTorque;
		}
		if(fasterTorque < 0){
			fasterTorque = 0;
		}
		if(slowerTorque > 90){
			slowerTorque = 90;
		}
		// if(slowerTorque > 100)
		// 	slowerTorque = 100;
		// if(fasterTorque < 0)
		// 	fasterTorque = 0;

		if(RVC.lsd.faster == RVC_Lsd_Faster_left) //RL losts torque, RR gains torque
		{
			RVC.torque.rearLeft = fasterTorque; //decreases torque
			RVC.torque.rearRight = slowerTorque; //increases torque
		}
		else if(RVC.lsd.faster == RVC_Lsd_Faster_right)  //RR losts torque, RL gains torque
		{
			RVC.torque.rearLeft = slowerTorque; //increase torque
			RVC.torque.rearRight = fasterTorque; //decreases torque
		}
		RVC.torque.rearRight = RVC.torque.controlled;
		RVC.torque.rearLeft = RVC.torque.controlled;
	}
	else
	{
		RVC.torque.rearRight = RVC.torque.controlled;
		RVC.torque.rearLeft = RVC.torque.controlled;
	}
}

void RVC_TorqueVectoring_run_mode1(void)
{
	// TODO: TV algorithm
	/*Default*/
	RVC_TorqueVectoring_run_modeOpen();
}

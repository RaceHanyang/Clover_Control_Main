/*
 * RVC_privateDataStructure.h
 * Created on 2019. 12. 10.
 * Author: Dua
 */

#ifndef RVC_PRIVATEDATASTRUCTURE_H_
#define RVC_PRIVATEDATASTRUCTURE_H_

/* Includes */
#include "HLD.h"
#include "UserInterface_Button.h"

/* Enumerations */
typedef enum
{
	RVC_ReadyToDrive_status_notInitialized = 0,
	RVC_ReadyToDrive_status_initialized = 1,
	RVC_ReadyToDrive_status_run = 2,
} RVC_ReadyToDrive_status;

typedef enum
{
	RVC_TorqueVectoring_modeOpen = 0,
	RVC_TorqueVectoring_mode1 = 1,
} RVC_TorqueVectoring_mode_t;

typedef struct
{
	float32 mul;
	float32 offset;
} RVC_pwmCalibration;

/* Data Structures */
typedef struct
{
	RVC_ReadyToDrive_status readyToDrive;
	RVC_TorqueVectoring_mode_t tvMode;
	HLD_button_t startButton;

	struct
	{
		float32 desired;
		float32 controlled;

		float32 rearLeft;
		float32 rearRight;
	} torque;

	struct
	{
		HLD_GtmTom_Pwm accel_rearLeft;
		HLD_GtmTom_Pwm accel_rearRight;
		HLD_GtmTom_Pwm decel_rearLeft;
		HLD_GtmTom_Pwm decel_rearRight;
	} out;

	struct
	{
		RVC_pwmCalibration left;
		RVC_pwmCalibration right;
	} calibration;

	struct
	{
		float32 rearLeft;
		float32 rearRight;
	} pwmDuty;

	struct
	{
		float32 pGain;
	} tvMode1;
} RVC_t;

#endif
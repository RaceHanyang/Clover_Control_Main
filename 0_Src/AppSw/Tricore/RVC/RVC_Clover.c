/*
 * RVC_Clover.c
 * Created on 2023.05.05
 * Author: Terry
 *
 * This file is a fork of RVC.c, the RVC for "Forsythia", to make it work with Clover.
 * The fork is based on commitID: c3353b8e15242fe95f5eda9b5246cccdbc50b0a7
 * */
/*
TODO:
	CAN associated functions
		- R2D entry routine display
		- Steering wheel function
		- Parameter load/save
		- Log Data Broadcasting
	Battery Management
		- Charge consumed calculation
	Analog sensor
		- Steering Wheel Analog: need calibration code
	GPIO
		- Charge Enable
 */


/***************************** Includes ******************************/
#include <math.h>

#include "Beeper_Test_Music.h"
#include "HLD.h"
#include "IfxPort.h"
#include "PedalMap.h"
#include "Gpio_Debounce.h"
#include "AdcSensor.h"
#include "AdcForceStart.h"
#include "RVC_Clover.h"
#include "RVC_privateDataStructure.h"
#include "RVC_r2dSound.h"
#include "TorqueVectoring/TorqueVectoring_Clover.h"

#include "SteeringWheel.h"

#include "DashBoardCan.h"

#include "Accumulator.h"
#include "Build_Global_Parameter.h"

/**************************** Macro **********************************/
#define PWMFREQ 5000 // PWM frequency in Hz
#define PWMVREF 5.0  // PWM reference voltage (On voltage)

/*
#define OUTCAL_LEFT_MUL 1.06
#define OUTCAL_LEFT_OFFSET 0.015
#define OUTCAL_RIGHT_MUL 1.065
#define OUTCAL_RIGHT_OFFSET 0.0125
 */

#define OUTCAL_LEFT_MUL 1.0f
#define OUTCAL_LEFT_OFFSET 0.0f
#define OUTCAL_RIGHT_MUL 1.0f
#define OUTCAL_RIGHT_OFFSET 0.0f

#define R2D_ONHOLD (3*100)			//3 seconds
#define R2D_OFFHOLD (1*100)			//1 seconds
#define R2D_REL (1*100)			//1 seconds
#define R2D_APPS_TH		95
#define R2D_APPS_LO		5
#define R2D_APPS_HOLD	(5*10)	//0.5 seconds
#define R2D_BPPS_TH		95
#define R2D_BPPS_LO		5
#define R2D_BPPS_HOLD	(5*10)	//0.5 seconds
#define R2D_TEST		//Test: start button set R2D directly

#define PEDAL_BRAKE_ON_THRESHOLD 10
#define REGEN_MUL	1	//2

#define POWER_LIM				80000	//40kW -> 80kW Clover
#define CURRENT_LIM_SET_VAL		10		//10A

#define TV1PGAIN 0.001

#define REGEN_ON_INIT	FALSE	//***** Regen is not abailable now!!! ***** //FIXME //TODO: Regen limit

#define LVBAT_LINCAL_A	1.021f
#define LVBAT_LINCAL_B	0
#define LVBAT_LINCAL_D	0

#define VAR_UPDATE_ERROR_LIM	10

#define THROTLE_5V

#define BRAKE_ON_BP
#define BRAKE_ON_TH_BP1	13.97f
#define BRAKE_ON_TH_BP2 30.0f

//#define BMS_PDL_ERROR	TRUE
#define BMS_PDL_ERROR	(FALSE)


#define __IGNORE_BRAKE_OVERRIDE__ (FALSE) //MUST BE SET FALSE ON A REAL RACE! Set this flag true and release the BSPD hardbrake threshold for testing occasions only.




#define RTDS_TIME (3000) //RTD Sound length in ms.

/*********************** Global Variables ****************************/
RVC_t RVC =
{
    .readyToDrive = RVC_ReadyToDrive_status_notInitialized,
    .torque.controlled = 0,
    .torque.rearLeft = 0,
    .torque.rearRight = 0,
	.torque.isRegenOn = REGEN_ON_INIT,

	.calibration.leftAcc.mul = 1,
	.calibration.leftAcc.offset = 0,
	.calibration.rightAcc.mul = 1,
	.calibration.rightAcc.offset = 0,
	.calibration.leftDec.mul = 1,
	.calibration.leftDec.offset = 0,
	.calibration.rightDec.mul = 1,
	.calibration.rightDec.offset = 0,

	.power.limit = POWER_LIM,
	.currentLimit.setValue = CURRENT_LIM_SET_VAL,

	// Terry:RTDSound 3second wait
	.RTDS_Tick = RTDS_TIME,
	//~Terry

	//add MS
	//difflimit --> 0.16 According to skid pad
	.lsd.speedLow = 2.5f,
	.lsd.diffLimit = 0.15f,
	.lsd.kGain = 0.5f/0.2f,
	.lsd.lGain = 5.0f,
	.lsd.mGain = 1.0f,		//faster wheel derating
	.lsd.faster = RVC_Lsd_Faster_none,
	//~MS
};

RVC_public_t RVC_public;
/******************* Private Function Prototypes *********************/
IFX_STATIC void RVC_setR2d(void);
IFX_STATIC void RVC_resetR2d(void);
IFX_STATIC void RVC_toggleR2d(void);

IFX_STATIC void RVC_initAdcSensor(void);
IFX_STATIC void RVC_initPwm(void);
IFX_STATIC void RVC_initGpio(void);
IFX_STATIC void RVC_r2d(void);
IFX_STATIC void RVC_pollGpi(RVC_Gpi_t *gpi);

IFX_INLINE void RVC_updateReadyToDriveSignal(void);
IFX_INLINE void RVC_slipComputation(void);
IFX_INLINE void RVC_getTorqueRequired(void);
IFX_INLINE void RVC_powerComputation(void);
IFX_INLINE void RVC_torqueLimit(void);
IFX_INLINE void RVC_torqueSatuation(void);
IFX_INLINE void RVC_torqueDistrobution(void);
IFX_INLINE void RVC_writeCascadiaCAN();
IFX_INLINE void RVC_updateSharedVariable(void);

/********************* Function Implementation ***********************/
void RVC_init(void)
{
	RVC_initAdcSensor();

	RVC.tvMode = RVC_TorqueVectoring_modeOpen;
	RVC.tcMode = RVC_TractionControl_modeNone;
	RVC_PedalMap_lut_setMode(0);

	RVC_initPwm();

	RVC_initGpio();

	RVC.tvMode1.pGain = TV1PGAIN;
	RVC.readyToDrive = RVC_ReadyToDrive_status_initialized;





	SDP_Cooling_setVCUmode();
	SDP_Cooling_AllOn();
	SDP_Cooling_setWaterPumpDuty(50, 50);
	SDP_Cooling_setRadiatorFanDuty(50, 50);
	SDP_Cooling_setExternalFanDuty(50);
}

void RVC_run_1ms(void)
{
	RVC_updateReadyToDriveSignal();

	RVC_slipComputation(); //to adapt

	RVC_getTorqueRequired();

#ifndef __TEST_IGNORE_BRAKE_OVERRIDE__

	if(RVC.BrakePressure1.value > BRAKE_ON_TH_BP1)
		RVC.brakeOn.bp1 = TRUE;
	else
		RVC.brakeOn.bp1 = FALSE;

	if(RVC.BrakePressure2.value > BRAKE_ON_TH_BP2)
		RVC.brakeOn.bp2 = TRUE;
	else
		RVC.brakeOn.bp2 = FALSE;

	//RVC.brakeOn.tot = RVC.brakeOn.bp1 | RVC.brakeOn.bp2 | RVC.brakePressureOn.value;
	//RVC.brakeOn.tot = RVC.brakeOn.bp1 | RVC.brakeOn.bp2;
	RVC.brakeOn.tot = RVC.brakeOn.bp2;
#else
	RVC.brakeOn.bp1 = FALSE;
	RVC.brakeOn.bp2 = FALSE;
	RVC.brakeOn.tot = FALSE;
#endif
	/* TODO: Torque limit: Traction control */

	RVC_powerComputation(); //max torque ����

	RVC_torqueLimit();

	RVC_torqueSatuation();

	RVC_torqueDistrobution(); // ���ǹ�

	/* TODO: Torque signal check*/

	RVC_writeCascadiaCAN(); //Comment to Override can!

	/* TODO: Shared variable update */
	RVC_updateSharedVariable();
}

void RVC_updateSDCinfo(void)
{
	RVC_public.sdc.data.SdcAmsOk = RVC.bmsOk.value;
	RVC_public.sdc.data.SdcImdOk = RVC.imdOk.value;
	RVC_public.sdc.data.SdcBspdOk = RVC.bspdOk.value;
	RVC_public.sdc.data.SdcFinalOn = RVC.sdcSenFinal.value;
}

void RVC_run_10ms(void)
{
	RVC_pollGpi(&RVC.airPositive);
	RVC_pollGpi(&RVC.airNegative);
	RVC_pollGpi(&RVC.brakePressureOn);
	RVC_pollGpi(&RVC.brakeSwitch);
	RVC_pollGpi(&RVC.TSALOn);
	RVC_pollGpi(&RVC.sdcSenBspd);
	RVC_pollGpi(&RVC.sdcSenImd);
	RVC_pollGpi(&RVC.sdcSenAms);
	RVC_pollGpi(&RVC.sdcSenFinal);
	RVC_pollGpi(&RVC.bmsOk);
	RVC_pollGpi(&RVC.imdOk);
	RVC_pollGpi(&RVC.bspdOk);
	RVC_pollGpi(&RVC.bmsMpo);
	RVC_pollGpi(&RVC.chargeEn);
	RVC_updateSDCinfo();
	RVC_r2d();
	AdcSensor_getData(&RVC.LvBattery_Voltage);
	AdcSensor_getData(&RVC.BrakePressure1);
	AdcSensor_getData(&RVC.BrakePressure2);
}

/****************** Private Function Implementation ******************/
IFX_STATIC void RVC_setR2d(void)
{
	if(RVC.readyToDrive == RVC_ReadyToDrive_status_initialized)
	{
		RVC.readyToDrive = RVC_ReadyToDrive_status_run;
		HLD_GtmTomBeeper_start_volume(RVC_r2dSound, 1); // R2D sound	//FIXME: R2D sound
        CascadiaInverter_initParameterWrite(); //Attempt to clear Fault

        RVC.RTDS_Tick = 0;
        SDP_WheelSpeed_setSourceWSS();

        RVC_public.readyToDrive.data = RVC_public_ReadyToDrive_status_run;
	}
}

IFX_STATIC void RVC_resetR2d(void)
{
	if(RVC.readyToDrive == RVC_ReadyToDrive_status_run)
	{
		RVC.readyToDrive = RVC_ReadyToDrive_status_initialized;
		HLD_GtmTomBeeper_start(RVC_r2dResetSound);
        CascadiaInverter_disable();

        RVC.RTDS_Tick = 0;

        RVC_public.readyToDrive.data = RVC_public_ReadyToDrive_status_initialized;
	}
}

IFX_STATIC void RVC_toggleR2d(void)
{
	if(RVC.readyToDrive == RVC_ReadyToDrive_status_initialized) {
		RVC.readyToDrive = RVC_ReadyToDrive_status_run;
        RVC_public.readyToDrive.data = RVC_public_ReadyToDrive_status_run;
	}
	else if(RVC.readyToDrive == RVC_ReadyToDrive_status_run) {
		RVC.readyToDrive = RVC_ReadyToDrive_status_initialized;
        RVC_public.readyToDrive.data = RVC_public_ReadyToDrive_status_initialized;
	}
}

IFX_STATIC void RVC_initAdcSensor(void)
{
	AdcSensor_Config adcConfig;

	/* LV battery voltage */
	adcConfig.adcConfig.channelIn = &(HLD_Vadc_Channel_In){HLD_Vadc_group5, HLD_Vadc_ChannelId_6};

	adcConfig.adcConfig.lpf.activated = TRUE;
	adcConfig.adcConfig.lpf.config.gain = 1;
	adcConfig.adcConfig.lpf.config.cutOffFrequency = 2;
	adcConfig.adcConfig.lpf.config.samplingTime = 10.0e-3;

	adcConfig.isOvervoltageProtected = FALSE;
	adcConfig.linCalConfig.isAct = TRUE;
	adcConfig.linCalConfig.a = LVBAT_LINCAL_A;
	adcConfig.linCalConfig.b = LVBAT_LINCAL_B;
	adcConfig.linCalConfig.d = LVBAT_LINCAL_D;
	adcConfig.tfConfig.a = (130.0f+20.0f)/20.0f;
	adcConfig.tfConfig.b = 0.0f;

	AdcSensor_initSensor(&RVC.LvBattery_Voltage, &adcConfig);
	HLD_AdcForceStart(RVC.LvBattery_Voltage.adcChannel.channel.group);

	/* Brake Pressure*/
	adcConfig.adcConfig.lpf.activated = TRUE;
	adcConfig.adcConfig.lpf.config.gain = 1;
	adcConfig.adcConfig.lpf.config.cutOffFrequency = 1/(2*IFX_PI*(1e-2f));
	adcConfig.adcConfig.lpf.config.samplingTime = 10.0e-3;

	adcConfig.isOvervoltageProtected = FALSE;
	adcConfig.linCalConfig.isAct = FALSE;
	//adcConfig.tfConfig.a = 400.0; // unit : psi
	adcConfig.tfConfig.a = 27.578599;// unit : bar
	adcConfig.tfConfig.b = 0.0;

	//REAR: G0CH2
	adcConfig.adcConfig.channelIn = &(HLD_Vadc_Channel_In){HLD_Vadc_group0, HLD_Vadc_ChannelId_2}; //REAR
	AdcSensor_initSensor(&RVC.BrakePressure1, &adcConfig);
	//FRONT: G4CH4
	adcConfig.adcConfig.channelIn = &(HLD_Vadc_Channel_In){HLD_Vadc_group4, HLD_Vadc_ChannelId_4}; //FRONT. Originally G0I1. Guess this is for something else?
	AdcSensor_initSensor(&RVC.BrakePressure2, &adcConfig);
	HLD_AdcForceStart(RVC.BrakePressure1.adcChannel.channel.group);

	/* Steering Angle Analog (Backup function) */
	//TODO
}

IFX_STATIC void RVC_initPwm(void)
{
	/* PWM output initialzation */
	HLD_GtmTom_Pwm_Config pwmConfig;
	pwmConfig.frequency = PWMFREQ;

	pwmConfig.tomOut = &PWMACCL;
	HLD_GtmTomPwm_initPwm(&RVC.out.accel_rearLeft, &pwmConfig);

	pwmConfig.tomOut = &PWMACCR;
	HLD_GtmTomPwm_initPwm(&RVC.out.accel_rearRight, &pwmConfig);

	pwmConfig.tomOut = &PWMDCCL;
	HLD_GtmTomPwm_initPwm(&RVC.out.decel_rearLeft, &pwmConfig);

	pwmConfig.tomOut = &PWMDCCR;
	HLD_GtmTomPwm_initPwm(&RVC.out.decel_rearRight, &pwmConfig);

	/* PWM output calibration */
	RVC.calibration.leftAcc.mul = OUTCAL_LEFT_MUL;
	RVC.calibration.leftAcc.offset = OUTCAL_LEFT_OFFSET;

	RVC.calibration.rightAcc.mul = OUTCAL_RIGHT_MUL;
	RVC.calibration.rightAcc.offset = OUTCAL_RIGHT_OFFSET;
}

IFX_STATIC void RVC_initGpio(void)
{
	/* FWD output config */
	IfxPort_setPinMode(FWD_OUT.port, FWD_OUT.pinIndex, IfxPort_OutputMode_pushPull);
	IfxPort_setPinLow(FWD_OUT.port, FWD_OUT.pinIndex);
	/* R2D signal output config */
	IfxPort_setPinMode(R2DOUT.port, R2DOUT.pinIndex, IfxPort_OutputMode_pushPull);
	IfxPort_setPinLow(R2DOUT.port, R2DOUT.pinIndex);

	/* Start button config */
	Gpio_Debounce_inputConfig gpioInputConfig;
	Gpio_Debounce_initInputConfig(&gpioInputConfig);
	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &START_BTN;
	Gpio_Debounce_initInput(&RVC.startButton, &gpioInputConfig);

	/* AIR Contact signal input config */
	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &AIR_P_IN;
	Gpio_Debounce_initInput(&RVC.airPositive.debounce, &gpioInputConfig);
	gpioInputConfig.port = &AIR_N_IN;
	Gpio_Debounce_initInput(&RVC.airNegative.debounce, &gpioInputConfig);

	/* Pedalbox signal input config */
	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &BP_IN;
	Gpio_Debounce_initInput(&RVC.brakePressureOn.debounce, &gpioInputConfig);
	gpioInputConfig.port = &BSW_IN;
	Gpio_Debounce_initInput(&RVC.brakeSwitch.debounce, &gpioInputConfig);

	/* TSAL light input config */
	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &TSAL_RED_ON_5V;
	Gpio_Debounce_initInput(&RVC.TSALOn.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &SDC_SEN_BSPD_5V;
	Gpio_Debounce_initInput(&RVC.sdcSenBspd.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &SDC_SEN_IMD_5V;
	Gpio_Debounce_initInput(&RVC.sdcSenImd.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &SDC_SEN_AMS_5V;
	Gpio_Debounce_initInput(&RVC.sdcSenAms.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &SDC_SEN_FINAL_5V;
	Gpio_Debounce_initInput(&RVC.sdcSenFinal.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &BMS_OK;
	Gpio_Debounce_initInput(&RVC.bmsOk.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &IMD_OK;
	Gpio_Debounce_initInput(&RVC.imdOk.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &BSPD_OK;
	Gpio_Debounce_initInput(&RVC.bspdOk.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &BMS_MPO_5V;
	Gpio_Debounce_initInput(&RVC.bmsMpo.debounce, &gpioInputConfig);

	gpioInputConfig.bufferLen = Gpio_Debounce_BufferLength_10;
	gpioInputConfig.inputMode = IfxPort_InputMode_noPullDevice;
	gpioInputConfig.port = &CHARGE_EN;
	Gpio_Debounce_initInput(&RVC.chargeEn.debounce, &gpioInputConfig);

}

/* TODO:
	Check APPS and BPPS
	R2D sound generation
*/
//FIXME: Using struct - Hi,Lo,None,Error
IFX_STATIC boolean CheckPpsHi(SDP_PedalBox_struct_t *pps)
{
	if(pps->isValueOk)
	{
		if(pps->pps > R2D_APPS_TH)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		return FALSE;
	}
}
IFX_STATIC boolean CheckPpsLo(SDP_PedalBox_struct_t *pps)
{
	if(pps->pps < R2D_APPS_LO)
		return TRUE;
	else
		return FALSE;
}
IFX_STATIC void PpsCheck(boolean *isChecked, uint32 *count, boolean isHi, uint32 hold)
{
	if(isHi)
	{
		(*count)++;
		if(*count > hold)
		{
			*isChecked = TRUE;
			*count = 0;
		}
	}
	else
	{
		*count = 0;
	}
}
IFX_STATIC void RVC_r2d(void) //Gpio(RH) -> CAN(Clover)
{
	//static boolean risingEdgeFlag = FALSE;	//Ready to drive contorl button hysteresis
	//static uint32 pushCount = 0;
	//static uint32 releaseCount = 0;
	//boolean buttonState = FALSE; //GPIO debounced input result

	boolean buttonOn = FALSE;	//Start button

	static uint32 appsCount = 0;
	static uint32 bppsCount = 0;
	boolean isAppsHi = FALSE;
	boolean isAppsLo = FALSE;
	boolean isBppsHi = FALSE;
	boolean isBppsLo = FALSE;

	/* Poll the button */
	//buttonState = Gpio_Debounce_pollInput(&RVC.startButton); //RH Legacy

	/* Poll PPS signals */
	isAppsHi = CheckPpsHi(&SDP_PedalBox.apps);
	isAppsLo = CheckPpsLo(&SDP_PedalBox.apps);
	isBppsHi = CheckPpsHi(&SDP_PedalBox.bpps);
	isBppsLo = CheckPpsLo(&SDP_PedalBox.bpps);

	/* Pedal check routine */
	PpsCheck(&RVC.R2d.isAppsChecked, &appsCount, isAppsHi, R2D_APPS_HOLD);
	PpsCheck(&RVC.R2d.isBppsChecked1, &bppsCount, isBppsHi, R2D_BPPS_HOLD);	//FIXME: Using Brake Pressure sensor
	if(RVC.R2d.isBppsChecked1 == TRUE)
		if(isBppsLo)
			RVC.R2d.isBppsChecked2 = TRUE;

	/* Start button routine */
	buttonOn = SDP_DashBoardCan_getDashBoard_RTD_Status();
	/* R2D routine */
#ifdef R2D_TEST
	/***** Test: start button set/reset R2D directly *****/
	if((RVC.readyToDrive == RVC_ReadyToDrive_status_initialized) && (buttonOn == TRUE)/* && (RVC.brakeOn.tot == TRUE)*/)
	{
#ifndef __IGNORE_BRAKE_TO_RTD__
		if(RVC.brakeOn.tot == TRUE) {
			RVC_setR2d();
		}
		else {
			SDP_DashBoardCan_reset_pastRTD();
		}
	}
#else
		RVC_setR2d();
#endif
	else if((RVC.readyToDrive == RVC_ReadyToDrive_status_run) && (buttonOn == FALSE))
	{
		RVC_resetR2d();
	}
#else
	/* R2D On condition */
	if((RVC.readyToDrive == RVC_ReadyToDrive_status_initialized) && (RVC.R2d.isAppsChecked == TRUE) &&
		(RVC.R2d.isBppsChecked2 == TRUE) && (isAppsLo == TRUE) && (isBppsHi == TRUE))
	{
		if((buttonOn == TRUE))
		{
			RVC_setR2d();
		}
	}
	else if(RVC.readyToDrive == RVC_ReadyToDrive_status_run)
	{
		if(buttonOn == TRUE || (RVC.airPositive.value == FALSE) || (RVC.airNegative.value == FALSE))
		{
			RVC_resetR2d();
		}
	}

#endif // R2D_TEST


//	switch(SDP_DashBoardCan_getDashBoard_RTD_UpdateRequest()){
//		case DashBoard_RTD_Status_UpdateRequest_none: { //Other than this, the RTD value from the DashBoard shall be ignored.
//
//
//		#ifndef __TEST_DISABLE_RESET_RTD_STATUS_ON_HV_OFF__
//			if((RVC.readyToDrive == RVC_ReadyToDrive_status_run) && (RVC.TSALOn.value == 0)) { //2023KSAE spec: RTD is on but HV is off -> Turn off RTD
//				SDP_DashBoardCan_resetRTD_request();
//				RVC_resetR2d();
//			}
//			break;
//		#endif
//		}
//
//		case DashBoard_RTD_Status_UpdateRequest_set: {
//			SDP_DashBoardCan_setRTD();
//			break;
//		}
//
//		case DashBoard_RTD_Status_UpdateRequest_reset: {
//			SDP_DashBoardCan_resetRTD();
//			break;
//		}
//	}

	#ifndef __TEST_DISABLE_RESET_RTD_STATUS_ON_HV_OFF__
		if((RVC.readyToDrive == RVC_ReadyToDrive_status_run) && (RVC.TSALOn.value == 0)) { //2023KSAE spec: RTD is on but HV is off -> Turn off RTD
			//SDP_DashBoardCan_resetRTD_request();
			RVC_resetR2d();
			SDP_DashBoardCan_reset_pastRTD();
		}
	#endif

}

IFX_STATIC void RVC_pollGpi(RVC_Gpi_t *gpi)
{
	gpi->value = Gpio_Debounce_pollInput(&gpi->debounce);
}

/***************** Inline Function Implementation ******************/
IFX_INLINE void RVC_updateReadyToDriveSignal(void)
{/*
	if(RVC.readyToDrive == RVC_ReadyToDrive_status_run)
	{
		IfxPort_setPinHigh(R2DOUT.port, R2DOUT.pinIndex);
		IfxPort_setPinHigh(FWD_OUT.port, FWD_OUT.pinIndex);
	}
	else
	{
		IfxPort_setPinLow(R2DOUT.port, R2DOUT.pinIndex);
		IfxPort_setPinLow(FWD_OUT.port, FWD_OUT.pinIndex);
	}
	*/
	if(RVC.RTDS_Tick < RTDS_TIME){
		RVC.RTDS_Tick++;

		if(RVC.readyToDrive == RVC_ReadyToDrive_status_run) { //Constant sound
			IfxPort_setPinLow(R2DOUT.port, R2DOUT.pinIndex); //Speaker
		}
		else if(RVC.readyToDrive == RVC_ReadyToDrive_status_initialized) { //Switching beep
			if((RVC.RTDS_Tick / 50)%2) { //Switching duty: 500ms
				IfxPort_setPinLow(R2DOUT.port, R2DOUT.pinIndex);
			}
			else {
				IfxPort_setPinHigh(R2DOUT.port, R2DOUT.pinIndex);
			}
		}

		//IfxPort_setPinLow(R2DOUT.port, R2DOUT.pinIndex);
		IfxPort_setPinLow(FWD_OUT.port, FWD_OUT.pinIndex); //Not connected to anything
	}
	else{
		IfxPort_setPinHigh(R2DOUT.port, R2DOUT.pinIndex);
		IfxPort_setPinHigh(FWD_OUT.port, FWD_OUT.pinIndex);
	}
}

IFX_INLINE void RVC_slipComputation(void)
{
//#ifndef __SDP_CLOVER__
	RVC.slip.axle = SDP_WheelSpeed.velocity.rearAxle/SDP_WheelSpeed.velocity.frontAxle;
	RVC.slip.left = SDP_WheelSpeed.wssRL.wheelLinearVelocity/SDP_WheelSpeed.wssFL.wheelLinearVelocity;
	RVC.slip.right = SDP_WheelSpeed.wssRR.wheelLinearVelocity/SDP_WheelSpeed.wssFR.wheelLinearVelocity;
	RVC.diff.rear = (SDP_WheelSpeed.wssRL.wheelLinearVelocity-SDP_WheelSpeed.wssRR.wheelLinearVelocity); //add MS Left - Right
//#endif
	// RVC.diff.rear
	if(isnan(RVC.slip.axle)||isnan(RVC.slip.left)||isnan(RVC.slip.right))
	{
		RVC.slip.error = TRUE;
	}
	else
	{
		RVC.slip.error = FALSE;
	}
	//add MS
	if(isnan(RVC.diff.rear))
	{
		RVC.diff.error = TRUE;
	}
	else
	{
		RVC.diff.error = FALSE;
	}
}

IFX_INLINE void RVC_getTorqueRequired(void)
{
	if(SDP_PedalBox.apps.isValueOk)		//APPS Plausibility check
	{
		RVC.torque.controlled = (RVC.torque.desired = RVC_PedalMap_lut_getResult(SDP_PedalBox.apps.pps));
	}
	else
	{
		RVC.torque.controlled = (RVC.torque.desired = 0);		//APPS Fail
	}
#ifdef BRAKE_ON_BP	//No Regen!	//TODO
	if(RVC.brakeOn.tot == TRUE && __IGNORE_BRAKE_OVERRIDE__ == FALSE)
	{
		RVC.torque.controlled = (RVC.torque.desired = 0);	//Zero torque signal when brake on.
	}
#else
	if(SDP_PedalBox.bpps.isValueOk)		//BPPS Plausibility check
	{
		if(SDP_PedalBox.bpps.pps > PEDAL_BRAKE_ON_THRESHOLD)
		{
			RVC.torque.desired = -(SDP_PedalBox.bpps.pps);		//BPPS overide
			if(RVC.torque.isRegenOn)							//Regen
			{
				RVC.torque.controlled = RVC.torque.desired;
			}
			else
			{
				RVC.torque.controlled = (RVC.torque.desired = 0);	//Regen off: Zero torque signal when brake on.
			}
		}
	}
	else //FIXME: BSPD control using Brake Pressure Analog signal. Failsafe for BPPS.
	{
		RVC.torque.controlled = 0;		//BPPS Fail
	}
#endif
}

IFX_INLINE void RVC_powerComputation(void)
{
	RVC.power.value = RVC_public.bms.data.current * RVC_public.bms.data.voltage;
	RVC.power.currentLimit = RVC.power.limit / RVC_public.bms.data.voltage;
}

IFX_INLINE void RVC_torqueLimit(void)
{
	uint16 dischargeLimit ;

	if(BMS_PDL_ERROR == TRUE)
	{
		dischargeLimit = 400;
		if(RVC_public.bms.data.highestTemp > 45)
		{
			dischargeLimit = 200;
		}
		else if(RVC_public.bms.data.highestTemp > 50)
		{
			dischargeLimit = 100;
		}
		else if(RVC_public.bms.data.highestTemp > 55)
		{
			dischargeLimit = 50;
		}
		if(RVC_public.bms.data.lowestVoltage < 3.0f)
		{
			dischargeLimit = 50;
		}
	}
	else
	{
		dischargeLimit = RVC_public.bms.data.dischargeLimit; //bms reported discharge limit.
	}

	float32 currentLimitByPower = RVC.power.currentLimit; //currentLimit = 80kw / live voltage

	RVC.currentLimit.value = (dischargeLimit > currentLimitByPower) ? currentLimitByPower : dischargeLimit;

	RVC.currentLimit.margin = RVC.currentLimit.value - RVC_public.bms.data.current; //note: bms.data: 0 fill

	if(RVC.currentLimit.margin < RVC.currentLimit.setValue) //setValue = 10A, Constant.
	{
		RVC.torque.controlled = RVC.torque.controlled * RVC.currentLimit.margin / RVC.currentLimit.setValue;
		RVC.currentLimit.isLimited = TRUE;
	}
	else
	{
		RVC.currentLimit.isLimited = FALSE;
	}
}

IFX_INLINE void RVC_torqueSatuation(void)
{
	if(RVC.torque.controlled > 90)
	{
		RVC.torque.controlled = 90;
	}
	else if(RVC.torque.controlled < -90)
	{
		RVC.torque.controlled = -90;
	}
	else if(RVC.torque.desired < RVC.torque.controlled)
	{
		RVC.torque.controlled = RVC.torque.desired;
	}
}

IFX_INLINE void RVC_torqueDistrobution(void)
{
	/* Traction Control Calculation */
	switch(RVC.tcMode)
	{
	case RVC_TractionControl_mode1:
		break;
	default:
		break;
	}

	/* Torque distribution */
	switch(RVC.tvMode)
	{
	case RVC_TorqueVectoring_mode1:
		RVC_TorqueVectoring_run_mode1();
		break;
	default:
		RVC_TorqueVectoring_run_modeOpen();
		break;
	}
}

IFX_INLINE void RVC_writeCascadiaCAN(void)
{
	if(RVC.readyToDrive == RVC_ReadyToDrive_status_run)
	{
		if(RVC.torque.rearLeft > 0 && RVC.torque.rearRight > 0) {
			CascadiaInverter_writeTorque(RVC.torque.rearLeft, RVC.torque.rearRight);
		} else if (RVC.torque.rearLeft > 0) {
			CascadiaInverter_writeTorque(RVC.torque.rearLeft, 0);
		} else if (RVC.torque.rearRight > 0) {
			CascadiaInverter_writeTorque(0, RVC.torque.rearRight);
		} else {
			CascadiaInverter_writeTorque(0, 0);
		}
	}
	else
	{
		//CascadiaInverter_disable();
	}
}


IFX_INLINE void VariableUpdateRoutine(void)
{

	SteeringWheel_public.shared.data.vehicleSpeed = SDP_WheelSpeed.velocity.chassis * 3.6; // m/s to kph

	SteeringWheel_public.shared.data.apps = SDP_PedalBox.apps.pps;
	SteeringWheel_public.shared.data.bpps = SDP_PedalBox.bpps.pps;
	if(RVC.readyToDrive == RVC_ReadyToDrive_status_run)
		SteeringWheel_public.shared.data.isReadyToDrive = TRUE;
	else
		SteeringWheel_public.shared.data.isReadyToDrive = FALSE;
	SteeringWheel_public.shared.data.isAppsChecked = RVC.R2d.isAppsChecked;
	SteeringWheel_public.shared.data.isBppsChecked1 = RVC.R2d.isBppsChecked1;
	SteeringWheel_public.shared.data.isBppsChecked2 = RVC.R2d.isBppsChecked2;
	if(SDP_PedalBox.apps.isValueOk == TRUE)
		SteeringWheel_public.shared.data.appsError = FALSE;
	else
		SteeringWheel_public.shared.data.appsError = TRUE;
	if(SDP_PedalBox.bpps.isValueOk == TRUE)
		SteeringWheel_public.shared.data.bppsError = FALSE;
	else
		SteeringWheel_public.shared.data.bppsError = TRUE;
	SteeringWheel_public.shared.data.lvBatteryVoltage = RVC.LvBattery_Voltage.value;
	SteeringWheel_public.shared.data.packPower = (OrionBms2.msg1.packVoltage * OrionBms2.msg1.packCurrent)/10;

	SteeringWheel_public.shared.data.bpps0_On = RVC.brakeOn.bp1;
	SteeringWheel_public.shared.data.bpps1_On = RVC.brakeOn.bp2;
}
volatile uint32 updateErrorCount = 0;
IFX_INLINE void RVC_updateSharedVariable(void)
{
	// static uint32 updateErrorCount = 0;
	if(IfxCpu_acquireMutex(&SteeringWheel_public.shared.mutex))	//Do not wait.
	{
		VariableUpdateRoutine();
		IfxCpu_releaseMutex(&SteeringWheel_public.shared.mutex);
	}
	else if(updateErrorCount < VAR_UPDATE_ERROR_LIM)
	{
		updateErrorCount++;
	}
	else
	{
		while(IfxCpu_acquireMutex(&SteeringWheel_public.shared.mutex));
		{
			VariableUpdateRoutine();
			IfxCpu_releaseMutex(&SteeringWheel_public.shared.mutex);
		}
		updateErrorCount = 0;
	}
}

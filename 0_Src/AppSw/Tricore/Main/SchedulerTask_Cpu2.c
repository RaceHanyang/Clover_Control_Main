/*
 * SchedulerTask_Cpu2.c
 *
 *  Created on: 2019. 10. 30.
 *      Author: Dua
 */


/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "SchedulerTask_Cpu2.h"
#include "AmkInverter_can.h"
#include "OrionBms2.h"
#include "SteeringWheel.h"
#include "AdcSensor.h"
#include "PedalBox.h"
/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/


/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/


/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
Task_cpu2 Task_core2 =
{
	.flag = FALSE,
};
uint64 stm_buf_c2 = 0;
uint64 stm_buf_c2_delay = 0;
uint64 ticToc_1ms_c2 = 0;
uint64 delay_1ms_c2 = 0;

extern AdcSensor APPS0;
/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/


/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/


/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void Task_core2_primaryService(void)
{

}

void Task_core2_1ms(void)
{
	stm_buf_c2_delay = IfxStm_get(&MODULE_STM0);

	delay_1ms_c2 = (IfxStm_get(&MODULE_STM0) - stm_buf_c2_delay) * 1000000 / (IfxStm_getFrequency(&MODULE_STM0));
	stm_buf_c2 = IfxStm_get(&MODULE_STM0);

	// AccumulatorManager_master_run_1ms_c2();
	// kelly8080ips_can_run_1ms_c2();
	// AmkInverter_can_Run();
	// writeMessage((int)APPS0.value,(int)APPS0.value);
	// writeMessage2((int)APPS0.value,(int)APPS0.value);
	// OrionBms2_run_1ms_c2();
	//SDP_MC_run_1ms();
	//SteeringWheel_run_xms_c2();
	SDP_Cooling_run_10ms();
	//SDP_ShockValue_run_1ms();
	ticToc_1ms_c2 = (IfxStm_get(&MODULE_STM0) - stm_buf_c2) * 1000000 / (IfxStm_getFrequency(&MODULE_STM0));
}

void Task_core2_backgroundService(void)
{

}

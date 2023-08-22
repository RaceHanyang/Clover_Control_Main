/*
 * Cascadia_Inverter_can.h
 *
 * With regard to PM100 Gen3 CAN Protocol V5.9
 *
 * Created: Jan 8th 2023
 * Author: Terry
 * yoonsb@hanayng.ac.kr
 * blog.terrysyoon.com
 *
 * */

#ifndef __INC_CASCADIA_INVERTER_H__
#define __INC_CASCADIA_INVERTER_H__

#include <Ifx_Types.h>
#include "Configuration.h"
#include "ConfigurationIsr.h"
#include <stdint.h>
#include "Multican.h"
#include "CanCommunication.h"

#include <Ifx_Assert.h>

typedef enum {
	NOWRITE = 0,
	CLEARFAULT = 20,
}PM100_ParameterAddress_t;

typedef enum {
	NOATTEMPT = 0,
	PENDING = 1,
	SUCCESS = 2,
	FAIL = 3,
}PM100_ParameterWriteStatus_t;

typedef union
{
	uint32 ReceivedData[2];
	struct{
		sint16 PM100_ModuleATemperature;
		sint16 PM100_ModuleBTemperature;
		sint16 PM100_ModuleCTemperature;
		sint16 PM100_GateDriverBoardTemperature;
	}S;
}PM100_Temperature1_Can_t;

typedef union
{
	uint32 ReceivedData[2];
	struct{
		sint16 PM100_ControlBoardTemperature;
		sint16 PM100_RTD1Temperature;
		sint16 PM100_RTD2Temperature;
		sint16 PM100_RTD3Temperature;
	}S;
}PM100_Temperature2_Can_t;

/*
 * Memory Structure for "Temperature#3" CAN
 * Address: 0x0A2
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		sint16 PM100_CoolantTemperature; //Temperature
		sint16 PM100_HotSpotTemperature; //Temperature
		sint16 PM100_MotorTemperature; //Temperature
		sint16 PM100_TorqueShudder; //Torque
	}S;
}PM100_Temperature3_Can_t;

/*
 * Memory Structure for "Motor Position Information" CAN
 * Address: 0x0A5
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		sint16 PM100_MotorAngle; //Angle
		sint16 PM100_MotorSpeed; //Angular Velocity
		sint16 PM100_ElectricalOutputFrequency;//Frequency
		sint16 PM100_DeltaResolverFiltered; //Angle
	}S;
}PM100_MotorPositionInformation_Can_t;

/*
 * Memory Structure for "Current Information" CAN
 * Address: 0x0A6
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		sint16 PM100_PhaseACurrent; //Current
		sint16 PM100_PhaseBCurrent; //Current
		sint16 PM100_PhaseCCurrent; //Current
		sint16 PM100_DCBusCurrent; //Current
	}S;
}PM100_CurrentInformation_Can_t;

/*
 * Memory Structure for "Voltage Information" CAN
 * Address: 0x0A7
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		sint16 PM100_DCBusVoltage; //High Voltage
		sint16 PM100_OutputVoltage; //High Voltage
		sint16 PM100_VAB_Vd_Voltage; //High Voltage
		sint16 PM100_VBC_Vq_Voltage; //High Voltage
	}S;
}PM100_VoltageInformation_Can_t;

/*
 * Memory Structure for "Internal States" CAN
 * Address: 0x0AA
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		uint8 VSM_State; //byte0
		uint8 Reserved; //byte1. NA for Gen3
		uint8 InverterState; //byte2
		union{ //byte3
			uint8 RelayState;
			struct{
				uint8 Relay1Status : 1;
				uint8 Relay2Status : 1;
				uint8 Relay3Status : 1;
				uint8 Relay4Status : 1;
				uint8 Relay5Status : 1;
				uint8 Relay6Status : 1;
				uint8 _Reserved_RelayStatus : 2;
			};
		};
		union{ //byte4
			uint8 byte4;
			struct{
				uint8 InverterRunMode : 1;
				uint8 _Reserved_Byte4 : 4;
				uint8 InverterActiveDischargeState : 3;
			};
		};
		union{
			uint8 byte5;
			struct{
				uint8 InverterCommandMode : 1;
				uint8 _Reserved_Byte5 : 3;
				uint8 RollingCounterValue : 4;
			};
		};
		union
		{
			uint8 byte6;
			struct{
				uint8 InverterEnableState : 1;
				uint8 _Reserved_Byte6 : 5;
				uint8 StartModeActive : 1;
				uint8 InverterEnableLockout : 1;
			};
		};
		union{
			uint8 byte7;
			struct{
				uint8 DirectionCommand : 1;
				uint8 BMSActive : 1;
				uint8 BMSLimitingTorque : 1;
				uint8 LimitMaxSpeed : 1; //For Gen3 2042+, Available for ours, 204C.
				uint8 _Reserved_Byte7 : 4;
			};
		};
	};
}PM100_InternalStates_Can_t;

/*
 * Memory Structure for "Fault Codes" CAN
 * Address: 0x0AB
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		union{
			uint16 Flag;
			struct{
				unsigned int Hardware_GATE_Desaturation_Fault : 1;
				unsigned int HW_Over_current_Fault : 1;
				unsigned int Accelerator_Shorted : 1;
				unsigned int Accelerator_Open : 1;
				unsigned int Current_Sensor_Low : 1;
				unsigned int Current_Sensor_High : 1;
				unsigned int Module_Temperature_Low : 1;
				unsigned int Module_Temperature_High : 1;
				unsigned int Control_PCB_Temperature_Low : 1;
				unsigned int Control_PCB_Temperature_High : 1;
				unsigned int Gate_Drive_PCB_Temperature_Low : 1;
				unsigned int Gate_Drive_PCB_Temperature_High : 1;
				unsigned int b5V_Sense_Voltage_Low : 1;
				unsigned int b5V_Sense_Voltage_High : 1;
				unsigned int b12V_Sense_Voltage_Low : 1;
				unsigned int b12V_Sense_Voltage_High : 1;
			}bits;
		}POST_Fault_Lo;
		union{
			uint16 Flag;
			struct{
				unsigned int b2_5V_Sense_Voltage_Low : 1;
				unsigned int b2_5V_Sense_Voltage_High : 1;
				unsigned int b1_5V_Sense_Voltage_Low : 1;
				unsigned int b1_5V_Sense_Voltage_High  : 1;
				unsigned int DC_Bus_Voltage_High : 1;
				unsigned int DC_Bus_Voltage_Low : 1;
				unsigned int Precharge_Timeout : 1;
				unsigned int Precharge_Voltage_Failure : 1;
				unsigned int EEPROM_Checksum_Invalid  : 1;
				unsigned int EEPROM_Data_Out_of_Range : 1;
				unsigned int EEPROM_Update_Required : 1;
				unsigned int Hardware_DC_Bus_OverVoltage_during_initialization  : 1;
				unsigned int Reserved1 : 1;
				unsigned int Reserved2 : 1;
				unsigned int Brake_Shorted : 1;
				unsigned int Brake_Open : 1;
			}bits;
		}POST_Fault_Hi;
		union{
			uint16 Flag;
			struct{
				unsigned int Motor_Overspeed_Fault : 1;
				unsigned int Overcurrent_Fault : 1;
				unsigned int Overvoltage_Fault : 1;
				unsigned int Inverter_Overtemperature_Fault : 1;
				unsigned int Accelerator_Input_Shorted_Fault : 1;
				unsigned int Accelerator_Input_Open_Fault  : 1;
				unsigned int Direction_Command_Fault : 1;
				unsigned int Inverter_Response_Timeout_Fault  : 1;
				unsigned int Hardware_Gate_Desaturation_Fault : 1;
				unsigned int Hardware_Overcurrent_Fault : 1;
				unsigned int Undervoltage_Fault : 1;
				unsigned int CAN_Command_Message_Lost_Fault : 1;
				unsigned int Motor_Overtemperature_Fault : 1;
				unsigned int Reserved1 : 1;
				unsigned int Reserved2 : 1;
				unsigned int Reserved3 : 1;
			}bits;
		}Run_Fault_Lo;
		union{
			uint16 Flag;
			struct{
				unsigned int Brake_Input_Shorted_Fault : 1;
				unsigned int Brake_Input_Open_Fault : 1;
				unsigned int ModuleA_OverTemperature_Fault : 1;
				unsigned int ModuleB_OverTemperature_Fault : 1;
				unsigned int ModuleC_OverTemperature_Fault : 1;
				unsigned int PCB_OverTemperature_Fault : 1;
				unsigned int GateDriveBoard1_OverTemperature_Fault : 1;
				unsigned int GateDriveBoard2_OverTemperature_Fault : 1;
				unsigned int GateDriveBoard3_OverTemperature_Fault : 1;
				unsigned int CurrentSensor_Fault : 1;
				unsigned int Reserved1 : 1;
				unsigned int Reserved2 : 1;
				unsigned int Reserved3 : 1;
				unsigned int Reserved4 : 1;
				unsigned int ResolverNotConnected : 1;
				unsigned int Reserved5 : 1;
			}bits;
		}Run_Fault_Hi;
	}S;
}PM100_FaultCodes_Can_t;


/*
 * Memory Structure for "High Speed Message" CAN
 * Address: 0x0B0
 * */
typedef union
{
	uint32 ReceivedData[2];
	struct{
		sint16 PM100_TorqueCommand;
		sint16 PM100_TorqueFeedback;
		sint16 PM100_MotorSpeed;
		sint16 PM100_DCBusVoltage;
	}S;
}PM100_HighSpeedMessage_Can_t;

/*
 * Memory Structure for "Command Message" CAN
 * Address: 0x0C0
 * */
typedef union
{
	uint32 TransmitData[2];
	struct{
		sint16 PM100_TorqueCommand;
		uint16 PM100_SpeedCommand;
		uint8 PM100_DirectionCommand;
		uint8 PM100_InverterEnable : 1;
		uint8 PM100_InverterDischarge : 1;
		uint8 PM100_SpeedModeEnable : 1;
		uint8 reservedBits : 5;
		uint16 PM100_CommandedTorqueLimit;
	}S;
}PM100_CommandMessage_Can_t;

/*
* Memory Structure for "Read / Write Parameter Command"
* Address: 0x0C1
* Tx
*/
typedef union {
	uint32 TransmitData[2];
	struct {
		uint16 ParameterAddress;
		/*
		 * July 4th 2023: Wrong reserved space?
		uint8 RW_Command : 1;
		uint8 _Reserved1 : 7;
		*/
		uint8 RW_Command;
		uint8 _Reserved2;
		uint16 Data;
		uint16 _Reserved3;
	}S;
}PM100_RWParameterCommand_Can_t;

/*
* Memory Structure for "Read / Write Parameter Response"
* Address: 0x0C2
* Rx
*/
typedef union {
	uint32 ReceivedData[2];
	struct {
		uint16 ParameterAddress;
		/* July 4th 2023: Wrong reserved space?
		uint8 WriteSuccess : 1;
		uint8 _Reserved1 : 7;
		*/
		uint8 WriteSuccess;
		uint8 _Reserved2;
		uint16 Data;
		uint16 _Reserved3;
	}S;
}PM100_RWParameterResponse_Can_t;

typedef struct {

	int node;

	/*RX~*/
	uint16 ID_PM100_Temperature1;
	uint16 ID_PM100_Temperature2;
	uint16 ID_PM100_Temperature3;
	uint16 ID_PM100_Position;
	uint16 ID_PM100_Current;
	uint16 ID_PM100_Voltage;
	uint16 ID_PM100_InternalStates; //0x0AA
	uint16 ID_PM100_FaultCodes;
	uint16 ID_PM100_HighSpeedMessage;
	/*~RX*/

	/*TX~*/
	uint16 ID_PM100_Command;
	/*~TX*/

	/*Parameter Messages~*/
	uint16 ID_PM100_RWParameterCommand;
	uint16 ID_PM100_RWParameterResponse;
	/*~Parameter Messages*/
}PM100_ID_set;

typedef struct
{
	PM100_Temperature1_Can_t Temperature1;
	PM100_Temperature2_Can_t Temperature2;
	PM100_Temperature3_Can_t Temperature3;
	PM100_MotorPositionInformation_Can_t Position;
	PM100_CurrentInformation_Can_t Current;
	PM100_VoltageInformation_Can_t Voltage;
	PM100_InternalStates_Can_t InternalStates;
	PM100_FaultCodes_Can_t FaultCodes;
	PM100_HighSpeedMessage_Can_t HighSpeedMessage;
}PM100_Status_t;

typedef struct
{
	PM100_CommandMessage_Can_t Command;
}PM100_Control_t;

typedef struct
{
	PM100_ParameterAddress_t ParameterAddress;
	PM100_ParameterWriteStatus_t WriteStatus;

	PM100_RWParameterCommand_Can_t ParameterCommand;
	PM100_RWParameterResponse_Can_t ParameterResponse;
	uint32 sentTick;
	uint32 receivedTick;
	uint32 RTT; //RTT. Equals to receivedTick - sentTick;

	uint8 failedClearCnt;
	uint8 writeReq;


	CanCommunication_Message ParameterCommand_msg;
	CanCommunication_Message ParameterResponse_msg;

}PM100_RWParameter_t;

typedef struct
{
	uint32 TransmitData[2];
	struct {
		//WS commanded torque feedback torque
		sint16 PM100_TorqueCommand;
		sint16 PM100_TorqueFeedback;
		sint16 PM100_MotorSpeed;
		uint16 reserved;
	}S;
}PM100_log_t;

IFX_EXTERN PM100_Status_t Inverter_L_Status;
IFX_EXTERN PM100_Status_t Inverter_R_Status;

IFX_EXTERN PM100_Control_t Inverter_L_Control;
IFX_EXTERN PM100_Control_t Inverter_R_Control;

IFX_EXTERN PM100_RWParameter_t Inverter_L_RWParameter;
IFX_EXTERN PM100_RWParameter_t Inverter_R_RWParameter;

IFX_EXTERN PM100_log_t Inverter_L_log;
IFX_EXTERN PM100_log_t Inverter_R_log;

IFX_EXTERN void CascadiaInverter_can_init(void);
IFX_EXTERN void CascadiaInverter_can_Run(void);
IFX_EXTERN void CascadiaInverter_enable();
IFX_EXTERN void CascadiaInverter_disable();
IFX_EXTERN void CascadiaInverter_initParameterWrite();
IFX_EXTERN void CascadiaInverter_writeTorque(uint16 torque_L, uint16 torque_R);
IFX_EXTERN void CascadiaInverter_clearFault(PM100_Status_t* Status, PM100_RWParameter_t* Parameter);

//IFX_EXTERN int16 CascadiaInverter_getRPM_RL();
//IFX_EXTERN int16 CascadiaInverter_getRPM_RR();
IFX_EXTERN void CascadiaInverter_getRPM(sint16* RPM_RL, sint16* RPM_RR);
//IFX_EXTERN void CascadiaInverter_disable();
#endif

/*
 * SteeringWheel.h
 * Created on: 2020.08.05
 * Author: Dua
 */
#ifndef STEERINGWHEEL_CANMESSAGE_H_
#define STEERINGWHEEL_CANMESSAGE_H_

/************************* Data Structures ***************************/
typedef union
{
	struct 
	{
		uint8 vehicleSpeed;		//byte0;	//SDP
		uint16 lowestVoltage;	//byte1~2;	//BMS
		uint8 highestTemp;		//byte3;	//BMS
		uint8 bmsTemp;			//byte4;	//BMS
		uint8 soc;				//byte5;	//BMS
		uint8 averageTemp;		//byte6;	//BMS
		union 					//byte7;	//RVC
		{
			uint8 U;
			struct 
			{
				uint16 r2d:4;
				uint16 appsError:1;
				uint16 bppsError:1;
				uint16 reserved:2; 
			}S;
		}status;
	}S;
	uint32 U[2];
}SteeringWheel_canMsg1_t;

typedef union 
{
	struct 
	{
		uint16 apps;				//byte0~1;		//RVC
		uint16 bpps;				//byte2~3;		//RVC
		//uint16 lvBatteryVoltage;	//byte4~5;		//RVC
		uint16 packPower;	//byte4~5;		//BMS
		uint16 accumulatorVoltage;	//byte6~7;		//BMS
	}S;
	uint32 U[2];
}SteeringWheel_canMsg2_t;

typedef union 
{
	struct 
	{
		uint8 inverter1Temp;		//byte0;		//Inverter
		uint8 motor1Temp;			//byte1;		//Inverter
		uint8 inverter2Temp;        //byte2;		//Inverter
		uint8 motor2Temp;			//byte3;		//Inverter
		uint32 Reserved;
	}S;
	uint32 U[2];
}SteeringWheel_canMsg3_t;

/*
typedef union {
	struct
	{
		uint8 ULbutton;
		uint8 URbutton;
		uint8 BLbutton;
		uint8 BRbutton;
		uint8 Rotswitch1;
		uint8 Rotswitch2;
		uint8 Rotswitch3;
	}S;
	uint32 TxData[2];
}SteeringWheel_ButtonMsg_t;
*/

/*
 * RX data
 * FROM: VCU
 * RX ID: 0x237BB01
 * Data: Three rotary switch values, each of them being an unsigned integer range of [0, 9]
 * */
typedef union{
	struct
	{
		uint8 RSW1; //The leftmost rotary switch, from the driver's perspective of view.
		uint8 RSW2; //The middle one.
		uint8 RSW3; //The rightmost one.
		uint8 _Reserved0;
		uint32 _Reserved1;
	}S;
	uint32 U[2];
}SteeringWheel_RSWMsg_t;
#endif

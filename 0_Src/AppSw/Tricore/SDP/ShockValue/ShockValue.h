/*
 * ShockValue.h
 *
 *  Created on: 2021. 5. 13.
 *      Author: Suprhimp
 */

#ifndef SRC_APPSW_TRICORE_SDP_SHOCKVALUE_SHOCKVALUE_H_
#define SRC_APPSW_TRICORE_SDP_SHOCKVALUE_SHOCKVALUE_H_

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "SDP.h"
#include "CanCommunication.h"
#include "HLD.h"

typedef union
{
	struct{
		unsigned int AngleR : 16;
		unsigned int AngleL : 16;
		unsigned int Roll : 16;
		unsigned int Heave : 16;
    }S;
    uint32 RecievedData[2];
}ShockCanMsg_data_t;

struct{
    float frontRoll;
    float frontHeave;
    float rearRoll;
    float rearHeave;
}ShockValue;

IFX_EXTERN struct ShockValue ShockValue;
IFX_EXTERN ShockCanMsg_data_t ShockCanMsgFront;
IFX_EXTERN ShockCanMsg_data_t ShockCanMsgRear;


IFX_EXTERN void SDP_ShockValue_init(void);
IFX_EXTERN void SDP_ShockValue_run_1ms(void);

#endif /* SHOCKVALUE_H */

#include <Ifx_Types.h>
#include "Configuration.h"
#include "ConfigurationIsr.h"
#include <stdint.h>
#include "Multican.h"
#include "CanCommunication.h"

#include "SteeringWheel_canMessage.h"
#include "SteeringWheel.h"

IFX_EXTERN void Clover_CanGateway_init();
IFX_EXTERN void Clover_CanGateway_run();

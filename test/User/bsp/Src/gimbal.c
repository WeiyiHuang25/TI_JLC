#include "gimbal.h"

void gimbal_set_zero(void)
{
    Emm_V5_Origin_Set_O(PITCH_MOTOR, true);
    Emm_V5_Origin_Set_O(YAW_MOTOR, true);
}

void gimbal_disable(void)
{
    Emm_V5_En_Control(PITCH_MOTOR, false, false);
    Emm_V5_En_Control(YAW_MOTOR, false, false);
}

void gimbal_enable(void)
{
    Emm_V5_En_Control(PITCH_MOTOR, true, false);
    Emm_V5_En_Control(YAW_MOTOR, true, false);   
}


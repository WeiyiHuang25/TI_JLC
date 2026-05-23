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

void gimbal_return_zero(void)
{
    Emm_V5_Origin_Trigger_Return(PITCH_MOTOR, 0, false);
    Emm_V5_Origin_Trigger_Return(YAW_MOTOR, 0, false);
}

void gimbal_update(uint8_t pitch_dir, uint16_t pitch_vel, uint32_t pitch_clk, uint8_t yaw_dir, uint16_t yaw_vel, uint32_t yaw_clk)
{
    Emm_V5_Pos_Control(PITCH_MOTOR, pitch_dir, pitch_vel, 0, pitch_clk, false, false);
    Emm_V5_Pos_Control(YAW_MOTOR, yaw_dir, yaw_vel, 0, yaw_clk, false, false);
}


#ifndef APP_RUNTIME_DEBUG_H
#define APP_RUNTIME_DEBUG_H

#include <rtthread.h>

extern volatile rt_uint32_t g_mesh_dbg_stage;
extern volatile rt_uint32_t g_mesh_dbg_arg0;
extern volatile rt_uint32_t g_mesh_dbg_arg1;
extern volatile rt_uint32_t g_mesh_dbg_cmd_count;
extern volatile rt_uint32_t g_mesh_dbg_send_ok_count;
extern volatile rt_uint32_t g_mesh_dbg_radio_send_count;
extern volatile rt_uint32_t g_mesh_dbg_radio_tx_count;
extern volatile rt_uint32_t g_mesh_dbg_tx_done_count;
extern volatile rt_uint32_t g_mesh_dbg_rx_done_count;
extern volatile rt_uint32_t g_mesh_dbg_radio_loop;
extern volatile rt_uint32_t g_mesh_dbg_main_loop;
extern volatile rt_uint32_t g_mesh_dbg_ready;
extern volatile rt_uint32_t g_mesh_dbg_fault_magic;
extern volatile rt_uint32_t g_mesh_dbg_fault_cfsr;
extern volatile rt_uint32_t g_mesh_dbg_fault_hfsr;
extern volatile rt_uint32_t g_mesh_dbg_fault_bfar;
extern volatile rt_uint32_t g_mesh_dbg_fault_mmar;
extern volatile rt_uint32_t g_mesh_dbg_fault_pc;
extern volatile rt_uint32_t g_mesh_dbg_fault_lr;
extern volatile rt_uint32_t g_mesh_dbg_fault_thread;
extern volatile rt_uint32_t g_app_assert_magic;
extern volatile rt_uint32_t g_app_assert_line;
extern volatile rt_uint32_t g_app_assert_ex;
extern volatile rt_uint32_t g_app_assert_func;

static inline void app_dbg_mark(rt_uint32_t stage)
{
    g_mesh_dbg_stage = stage;
}

static inline void app_dbg_mark2(rt_uint32_t stage, rt_uint32_t arg0, rt_uint32_t arg1)
{
    g_mesh_dbg_stage = stage;
    g_mesh_dbg_arg0 = arg0;
    g_mesh_dbg_arg1 = arg1;
}

#endif

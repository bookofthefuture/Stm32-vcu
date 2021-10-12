#ifndef PinMode_PRJ_H_INCLUDED
#define PinMode_PRJ_H_INCLUDED

#include "hwdefs.h"

// FOR STMF103/PRIUS BOARD

#define DIG_IO_LIST \
    DIG_IO_ENTRY(cruise_in,   GPIOB, GPIO5,  PinMode::INPUT_FLT)   \ // MAPPED TO ORIGINAL CRUISE INPUT
    DIG_IO_ENTRY(HV_req,       GPIOD, GPIO5,  PinMode::INPUT_FLT)   \ 
    DIG_IO_ENTRY(start_in,    GPIOB, GPIO6,  PinMode::INPUT_FLT)   \ // MAPPED TO ORIGINAL START INPUT
    DIG_IO_ENTRY(brake_in,    GPIOA, GPIO2,  PinMode::INPUT_FLT)   \ // MAPPED TO ORIGINAL BRAKE INPUT
    DIG_IO_ENTRY(fwd_in,      GPIOA, GPIO4,  PinMode::INPUT_FLT)   \ // MAPPED TO ORIGINAL FWD INPUT
    DIG_IO_ENTRY(rev_in,      GPIOC, GPIO6,  PinMode::INPUT_FLT)   \ // MAPPED TO ORIGINAL REV INPUT
    DIG_IO_ENTRY(bms_in,      GPIOC, GPIO8,  PinMode::INPUT_PD)   \ // MAPPED TO ORIGINAL BMS INPUT
    DIG_IO_ENTRY(dcsw_out,    GPIOC, GPIO13, PinMode::OUTPUT)      \ // MAPPED TO ORIGINAL DCSW OUTPUT
    DIG_IO_ENTRY(led_out,     GPIOC, GPIO2, PinMode::OUTPUT)      \ // MAPPED TO ORIGINAL LED OUTPUT
    DIG_IO_ENTRY(gp_out1,     GPIOB, GPIO13, PinMode::OUTPUT)      \ // MAPPED TO IGBT OUTPUT - PIN 34
    DIG_IO_ENTRY(gp_out2,     GPIOB, GPIO14, PinMode::OUTPUT)      \ // MAPPED TO IGBT OUTPUT - PIN 35
    DIG_IO_ENTRY(gp_out3,     GPIOB, GPIO15, PinMode::OUTPUT)      \ // MAPPED TO IGBT OUTPUT - PIN 36
 //   DIG_IO_ENTRY(sw_mode0,     GPIOD, GPIO9, PinMode::OUTPUT)      \  NO HARDWARE
 //   DIG_IO_ENTRY(sw_mode1,     GPIOD, GPIO8, PinMode::OUTPUT)      \  NO HARDWARE
//    DIG_IO_ENTRY(lin_wake,     GPIOD, GPIO10, PinMode::OUTPUT)      \ NO HARDWARE
//    DIG_IO_ENTRY(lin_nslp,     GPIOD, GPIO11, PinMode::OUTPUT)      \ NO HARDWARE
    DIG_IO_ENTRY(brk_out,     GPIOA, GPIO9,  PinMode::OUTPUT)      \ // MAPPED TO IGBT OUTPUT - PIN 42
    DIG_IO_ENTRY(prec_out,     GPIOB, GPIO1,  PinMode::OUTPUT)      \ // MAPPED TO ORIGINAL PRECHARGE RELAY OUTPUT
    DIG_IO_ENTRY(inv_out,     GPIOA, GPIO8,  PinMode::OUTPUT)      \ // MAPPED TO IGBT OUTPUT - PIN 41
 //   DIG_IO_ENTRY(SL1_out,     GPIOC, GPIO9,  PinMode::OUTPUT)      \ NO HARDWARE
 //   DIG_IO_ENTRY(SL2_out,     GPIOC, GPIO8,  PinMode::OUTPUT)      \ NO HARDWARE
 //   DIG_IO_ENTRY(SP_out,     GPIOD, GPIO12,  PinMode::OUTPUT)      \ NO HARDWARE
//    DIG_IO_ENTRY(gear1_in,   GPIOE, GPIO3,  PinMode::INPUT_FLT)   \ INSUFFICIENT INPUTS
//    DIG_IO_ENTRY(gear2_in,   GPIOE, GPIO4,  PinMode::INPUT_FLT)   \ INSUFFICIENT INPUTS
//    DIG_IO_ENTRY(gear3_in,   GPIOE, GPIO5,  PinMode::INPUT_FLT)   \ INSUFFICIENT INPUTS
    DIG_IO_ENTRY(req_out,     GPIOA, GPI10,  PinMode::OUTPUT)      \ // MAPPED TO IGBT OUTPUT - PIN 43
//    DIG_IO_ENTRY(pot1_cs,     GPIOD, GPIO3,  PinMode::OUTPUT)      \ NO HARDWARE
//    DIG_IO_ENTRY(pot2_cs,     GPIOD, GPIO2,  PinMode::OUTPUT)      \ NO HARDWARE
//    DIG_IO_ENTRY(mcp_cs,     GPIOB, GPIO12,  PinMode::OUTPUT)      \ NO HARDWARE
//    DIG_IO_ENTRY(mcp_sby,     GPIOE, GPIO14,  PinMode::OUTPUT)      \ NO HARDWARE
//    DIG_IO_ENTRY(t15_digi,     GPIOD, GPIO6,  PinMode::INPUT_FLT)      \ NO GOOD CANDIDATES
//    DIG_IO_ENTRY(gp_12Vin,     GPIOD, GPIO4,  PinMode::INPUT_FLT)      \  NO GOOD CANDIDATES


#endif // PinMode_PRJ_H_INCLUDED


/* FOR STMF107/ZOMBIE BOARD
#define DIG_IO_LIST \
    DIG_IO_ENTRY(cruise_in,   GPIOD, GPIO4,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(HV_req,       GPIOD, GPIO5,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(start_in,    GPIOD, GPIO7,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(brake_in,    GPIOA, GPIO15,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(fwd_in,      GPIOB, GPIO4,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(rev_in,      GPIOB, GPIO3,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(bms_in,      GPIOC, GPIO3,  PinMode::INPUT_PD)   \
    DIG_IO_ENTRY(dcsw_out,    GPIOC, GPIO7, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(led_out,     GPIOE, GPIO2, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(gp_out1,     GPIOD, GPIO15, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(gp_out2,     GPIOD, GPIO14, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(gp_out3,     GPIOD, GPIO13, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(sw_mode0,     GPIOD, GPIO9, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(sw_mode1,     GPIOD, GPIO8, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(lin_wake,     GPIOD, GPIO10, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(lin_nslp,     GPIOD, GPIO11, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(brk_out,     GPIOC, GPIO5,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(prec_out,     GPIOC, GPIO6,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(inv_out,     GPIOA, GPIO8,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(SL1_out,     GPIOC, GPIO9,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(SL2_out,     GPIOC, GPIO8,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(SP_out,     GPIOD, GPIO12,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(gear1_in,   GPIOE, GPIO3,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(gear2_in,   GPIOE, GPIO4,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(gear3_in,   GPIOE, GPIO5,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(req_out,     GPIOE, GPIO6,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(pot1_cs,     GPIOD, GPIO3,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(pot2_cs,     GPIOD, GPIO2,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(mcp_cs,     GPIOB, GPIO12,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(mcp_sby,     GPIOE, GPIO14,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(t15_digi,     GPIOD, GPIO6,  PinMode::INPUT_FLT)      \
    DIG_IO_ENTRY(gp_12Vin,     GPIOD, GPIO4,  PinMode::INPUT_FLT)      \


#endif // PinMode_PRJ_H_INCLUDED
/*
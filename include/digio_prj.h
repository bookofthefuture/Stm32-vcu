#ifndef PinMode_PRJ_H_INCLUDED
#define PinMode_PRJ_H_INCLUDED

#include "hwdefs.h"

// FOR STMF103/PRIUS BOARD

#define DIG_IO_LIST \
    DIG_IO_ENTRY(cruise_in,   GPIOB, GPIO5,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(HV_req,      GPIOD, GPIO5,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(start_in,    GPIOB, GPIO6,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(brake_in,    GPIOA, GPIO2,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(fwd_in,      GPIOA, GPIO4,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(rev_in,      GPIOC, GPIO6,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(bms_in,      GPIOC, GPIO8,  PinMode::INPUT_PD)    \
    DIG_IO_ENTRY(dcsw_out,    GPIOC, GPIO13, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(led_out,     GPIOC, GPIO12, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(gp_out1,     GPIOB, GPIO13, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(gp_out2,     GPIOB, GPIO14, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(gp_out3,     GPIOB, GPIO15, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(brk_out,     GPIOA, GPIO9,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(prec_out,     GPIOB, GPIO1,  PinMode::OUTPUT)     \
    DIG_IO_ENTRY(inv_out,     GPIOA, GPIO8,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(req_out,     GPIOA, GPI10,  PinMode::OUTPUT)      \

#endif // PinMode_PRJ_H_INCLUDED

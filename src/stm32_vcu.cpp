/*
 * This file is part of the tumanako_vc project.
 *
 * Copyright (C) 2010 Johannes Huebner <contact@johanneshuebner.com>
 * Copyright (C) 2010 Edward Cheeseman <cheesemanedward@gmail.com>
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2020 Damien Maguire <info@evbmw.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stm32_vcu.h"

#define RMS_SAMPLES 256
#define SQRT2OV1 0.707106781187
#define  UserCAN  2
#define  BMW_E46  0
#define  User  2
#define  None  4
//#define  BMW_E39  5
#define  VAG  6



HWREV hwRev; // Hardware variant of board we are running on
static Stm32Scheduler* scheduler;
static bool chargeMode = false;
static bool chargeModeDC = false;
static bool ChgLck = false;
static Can* can;
static _invmodes targetInverter;
static _vehmodes targetVehicle;
static _chgmodes targetCharger;
static _interface targetChgint;
static uint16_t maxRevs;
static uint32_t oldTime;
uint8_t pot_test;
uint8_t count_one=0;
uint8_t ChgSet;
bool RunChg;
bool Ampera_Not_Awake=true;
uint8_t ChgHrs_tmp;
uint8_t ChgMins_tmp;
uint16_t ChgDur_tmp;
uint8_t RTC_1Sec=0;
uint32_t ChgTicks=0,ChgTicks_1Min=0;

static volatile unsigned
	days=0,
	hours=0, minutes=0, seconds=0,
	alarm=0;			// != 0 when alarm is pending

// Instantiate Classes
BMW_E65Class E65Vehicle;
chargerClass chgtype;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Ms200Task(void)
{
    if(chargerClass::HVreq==true) Param::SetInt(Param::hvChg,1);
    if(chargerClass::HVreq==false) Param::SetInt(Param::hvChg,0);
    int opmode = Param::GetInt(Param::opmode);
    Param::SetInt(Param::Day,days);
    Param::SetInt(Param::Hour,hours);
    Param::SetInt(Param::Min,minutes);
    Param::SetInt(Param::Sec,seconds);
    Param::SetInt(Param::ChgT,ChgDur_tmp);
    if(ChgSet==2 && !ChgLck){ //if in timer mode and not locked out from a previous full charge.
        if(opmode!=MOD_CHARGE)
            {
            if((ChgHrs_tmp==hours)&&(ChgMins_tmp==minutes)&&(ChgDur_tmp!=0))RunChg=true;//if we arrive at set charge time and duration is non zero then initiate charge
            else RunChg=false;
            }

        if(opmode==MOD_CHARGE)
        {
           if(ChgTicks!=0)
           {
             ChgTicks--; //decrement charge timer ticks
             ChgTicks_1Min++;
           }

           if(ChgTicks==0)
           {
              RunChg=false; //end charge if still charging once timer expires.
              ChgTicks = (GetInt(Param::Chg_Dur)*300);//recharge the tick timer
           }

            if (ChgTicks_1Min==300)
            {
               ChgTicks_1Min=0;
                ChgDur_tmp--; //countdown minutes of charge time remaining.
            }
        }

    }
    if(ChgSet==0 && !ChgLck) RunChg=true;//enable from webui if we are not locked out from an auto termination
    if(ChgSet==1) RunChg=false;//disable from webui
    if(targetVehicle == _vehmodes::BMW_E65) BMW_E65Class::GDis();//needs to be every 200ms
    if(targetCharger == _chgmodes::Volt_Ampera)
    {
        //to be done
    }

    if(targetChgint == _interface::Unused) //No charger interface module used
    {

    }

    if(targetChgint == _interface::i3LIM) //BMW i3 LIM
    {
        i3LIMClass::Send200msMessages();

       if (opmode == MOD_OFF)
    {
        Param::SetInt(Param::chgtyp,OFF);
      auto LIMmode=i3LIMClass::Control_Charge(RunChg);
      if(LIMmode==i3LIMChargingState::DC_Chg)   //DC charge mode
      {
            chargeMode = true;
            chargeModeDC = true;   //DC charge mode
          Param::SetInt(Param::chgtyp,DCFC);
      }
      if(LIMmode==i3LIMChargingState::AC_Chg)
      {
          chargeMode = true;   //AC charge mode
          Param::SetInt(Param::chgtyp,AC);
      }

      if(LIMmode==i3LIMChargingState::No_Chg) chargeMode = false;  //no charge mode
    }

    if (opmode == MOD_CHARGE)
    {
        auto LIMmode=i3LIMClass::Control_Charge(RunChg);
        // if we are in AC charge mode,have no hv request and shutdown from the lim then end chg mode
        if((LIMmode==i3LIMChargingState::No_Chg)&&(Param::GetInt(Param::chgtyp)==AC)&&(chargerClass::HVreq==false))
        {
            chargeMode = false;  //no charge mode
            Param::SetInt(Param::chgtyp,OFF);

        }

        // if we are in DC charge mode and shutdown from the lim then end chg mode
        if((LIMmode==i3LIMChargingState::No_Chg)&&(Param::GetInt(Param::chgtyp)==DCFC))
        {
            chargeMode = false;  //no charge mode
            chargeModeDC = false;   //DC charge mode off
            Param::SetInt(Param::chgtyp,OFF);
        }


    }

}

    if(targetCharger == _chgmodes::Off)
    {
        chargeMode = false;
    }

    if(targetCharger == _chgmodes::HV_ON)
    {
      if(opmode != MOD_RUN)  chargeMode = true;

    }

    if(targetCharger == _chgmodes::EXT_CAN)
    {


    }

    ///////////////////////////////////////
    //Charge term logic
    ///////////////////////////////////////
    /*
    if we are in charge mode and battV >= setpoint and power is <= termination setpoint
        Then we end charge.
    */
    if(opmode==MOD_CHARGE)
    {
      if(Param::GetInt(Param::udc)>=Param::GetInt(Param::Voltspnt) && Param::GetInt(Param::idc)<=Param::GetInt(Param::IdcTerm))
      {
        RunChg=false;//end charge
        ChgLck=true;//set charge lockout flag
      }
    }
    if(opmode==MOD_RUN) ChgLck=false;//reset charge lockout flag when we drive off

    ///////////////////////////////////////

    count_one++;
if(count_one==1)    //just a dummy routine that sweeps the pots for testing.
{
/*    pot_test++;
    DigIo::pot1_cs.Clear();
    DigIo::pot2_cs.Clear();
    uint8_t dummy=spi_xfer(SPI3,pot_test);//test
    dummy=dummy;
    DigIo::pot1_cs.Set();
    DigIo::pot2_cs.Set();
    count_one=0;
*/
}



}






static void Ms100Task(void)
{
    DigIo::led_out.Toggle();
    iwdg_reset();
    s32fp cpuLoad = FP_FROMINT(scheduler->GetCpuLoad());
    Param::SetFlt(Param::cpuload, cpuLoad / 10);
    Param::SetInt(Param::lasterr, ErrorMessage::GetLastError());
    int opmode = Param::GetInt(Param::opmode);
    utils::SelectDirection(targetVehicle, E65Vehicle);
    utils::ProcessUdc(oldTime, GetInt(Param::speed));
    utils::CalcSOC();


        if(targetInverter == _invmodes::OpenI)
    {
      if (opmode == MOD_RUN) Can_OI::Send100msMessages();

    }

        if(targetChgint == _interface::i3LIM) //BMW i3 LIM
    {
        i3LIMClass::Send100msMessages();
    }

    if (targetInverter == _invmodes::Prius_Gen3)
    {
    
    }

        if (targetInverter == _invmodes::OpenI)
    {
        Param::SetInt(Param::tmphs,Can_OI::inv_temp);//send leaf temps to web interface
        Param::SetInt(Param::tmpm,Can_OI::motor_temp);
        Param::SetInt(Param::InvStat, Can_OI::error); //update inverter status on web interface
        Param::SetInt(Param::INVudc,Can_OI::voltage);//display inverter derived dc link voltage on web interface
    }

    if(targetVehicle == _vehmodes::BMW_E65)
    {
        if (E65Vehicle.getTerminal15())
        {
            E65Vehicle.DashOn();
            Param::SetInt(Param::T15Stat,1);
        }
        else
        {
            Param::SetInt(Param::T15Stat,0);
        }
    }
    else
    {
        E65Vehicle.DashOff();
    }

    if(targetVehicle != _vehmodes::BMW_E65) //if not E65 then T15 via digital input.
    {
      /*
      Param::SetInt(Param::T15Stat,DigIo::t15_digi.Get());
      */
    }

    if(targetVehicle==VAG) Can_VAG::SendVAG100msMessage();


    if (!chargeMode && rtc_get_counter_val() > 100)
    {
        if (Param::GetInt(Param::canperiod) == CAN_PERIOD_100MS)
            can->SendAll();
    }
    int16_t IsaTemp=ISA::Temperature;
    Param::SetInt(Param::tmpaux,IsaTemp);

    chargerClass::Send100msMessages(RunChg);

}


static void Ms10Task(void)
{
    int16_t previousSpeed=Param::GetInt(Param::speed);
    int16_t speed = 0;
    s32fp torquePercent;
    int opmode = Param::GetInt(Param::opmode);
    int newMode = MOD_OFF;
    int stt = STAT_NONE;
    ErrorMessage::SetTime(rtc_get_counter_val());


    if(targetChgint == _interface::i3LIM) //BMW i3 LIM
    {
        i3LIMClass::Send10msMessages();
    }

    if (Param::GetInt(Param::opmode) == MOD_RUN)
    {
        torquePercent = utils::ProcessThrottle(previousSpeed);
        FP_TOINT(torquePercent);
        if(ABS(previousSpeed)>=maxRevs) torquePercent=0;//Hard cut limiter:)
    }
    else
    {
        torquePercent = 0;
        utils::displayThrottle();//just displays pot and pot2 when not in run mode to allow throttle cal
    }



       if(targetInverter == _invmodes::Prius_Gen3)
    {
    }

            if(targetInverter == _invmodes::OpenI)
    {
        torquePercent = utils::change(torquePercent, 0, 3040, 0, 1000); //map throttle for OI
        Can_OI::SetThrottle(Param::Get(Param::dir),torquePercent);//send direction and torque request to inverter
        speed = ABS(Can_OI::speed);//set motor rpm on interface
    }

    Param::SetInt(Param::speed, speed);
    utils::GetDigInputs(can);

    // Send CAN 2 (Vehicle CAN) messages if necessary for vehicle integration.
    if (targetVehicle == BMW_E39)
    {
        uint16_t tempGauge = utils::change(Param::GetInt(Param::tmphs),15,80,88,254); //Map to e39 temp gauge
       //Messages required for E39
        Can_E39::Msg316(speed);//send rpm to e39 dash
        Can_E39::Msg329(tempGauge);//send heatsink temp to E39 dash temp gauge
        Can_E39::Msg545();
    }
    else if (targetVehicle == BMW_E46)
    {
        uint16_t tempGauge = utils::change(Param::GetInt(Param::tmphs),15,80,88,254); //Map to e46 temp gauge
        //Messages required for E46
        Can_E46::Msg316(speed);//send rpm to e46 dash
        Can_E46::Msg329(tempGauge);//send heatsink temp to E64 dash temp gauge
        Can_E46::Msg545();
    }
    else if (targetVehicle == _vehmodes::BMW_E65)
    {
        BMW_E65Class::absdsc(Param::Get(Param::din_brake));
        if(E65Vehicle.getTerminal15())
            BMW_E65Class::Tacho(Param::GetInt(Param::speed));//only send tach message if we are starting
    }
    else if (targetVehicle == VAG)
    {
        Can_VAG::SendVAG10msMessage(Param::GetInt(Param::speed));
    }

       else if (targetVehicle == VAG)
    {
        Can_VAG::SendVAG10msMessage(Param::GetInt(Param::speed));
    }

    //////////////////////////////////////////////////
    //            MODE CONTROL SECTION              //
    //////////////////////////////////////////////////
    s32fp udc = utils::ProcessUdc(oldTime, GetInt(Param::speed));
    stt |= Param::GetInt(Param::potnom) <= 0 ? STAT_NONE : STAT_POTPRESSED;
    stt |= udc >= Param::Get(Param::udcsw) ? STAT_NONE : STAT_UDCBELOWUDCSW;
    stt |= udc < Param::Get(Param::udclim) ? STAT_NONE : STAT_UDCLIM;


    if (opmode==MOD_OFF && (Param::GetBool(Param::din_start) || E65Vehicle.getTerminal15() || chargeMode))//on detection of ign on or charge mode enable we commence prechage and go to mode precharge
    {
      if(chargeMode==false)
      {
          //activate inv during precharge if not oi.
      if(targetInverter != _invmodes::OpenI) DigIo::inv_out.Set();//inverter power on but not if we are in charge mode!
      }
        DigIo::gp_out2.Set();//Negative contactors on
        DigIo::gp_out1.Set();//Coolant pump on
        DigIo::prec_out.Set();//commence precharge
        opmode = MOD_PRECHARGE;
        Param::SetInt(Param::opmode, opmode);
        oldTime=rtc_get_counter_val();
    }



    if(targetVehicle == _vehmodes::BMW_E65)
    {

        if(opmode==MOD_PCHFAIL && E65Vehicle.getTerminal15()==false)//use T15 status to reset
        {
            opmode = MOD_OFF;
            Param::SetInt(Param::opmode, opmode);
        }
    }
    else
    {
        if(opmode==MOD_PCHFAIL && !Param::GetBool(Param::din_start)) //use start input to reset.
        {
            opmode = MOD_OFF;
            Param::SetInt(Param::opmode, opmode);
        }
    }



        if(opmode==MOD_PCHFAIL && chargeMode)
        {
            opmode = MOD_OFF;
            Param::SetInt(Param::opmode, opmode);
        }


    /* switch on DC switch if
     * - throttle is not pressed
     * - start pin is high
     * - udc >= udcsw
     * - udc < udclim
     */
    if ((stt & (STAT_POTPRESSED | STAT_UDCBELOWUDCSW | STAT_UDCLIM)) == STAT_NONE)
    {

        if (Param::GetBool(Param::din_start) || E65Vehicle.getTerminal15())
        {
            newMode = MOD_RUN;
        }

         if (chargeMode)
        {
            newMode = MOD_CHARGE;
        }


        stt |= opmode != MOD_OFF ? STAT_NONE : STAT_WAITSTART;
    }

    Param::SetInt(Param::status, stt);

    if(opmode == MOD_RUN) //only shut off via ign command if not in charge mode
    {
    if(targetInverter == _invmodes::OpenI) DigIo::inv_out.Set();//inverter power on in run only if openi.
    if(targetVehicle == _vehmodes::BMW_E65)
    {
        if(!E65Vehicle.getTerminal15()) opmode = MOD_OFF; //switch to off mode via CAS command in an E65
    }
    else
    {
        //switch to off mode via igntition digital input.

    }
    }

  if(opmode == MOD_CHARGE && !chargeMode) opmode = MOD_OFF; //if we are in charge mode and commdn charge mode off then go to mode off.

    if (newMode != MOD_OFF)
    {
        DigIo::dcsw_out.Set();
//        DigIo::err_out.Clear();
        Param::SetInt(Param::opmode, newMode);
        ErrorMessage::UnpostAll();

    }


    if (opmode == MOD_OFF)
    {
        DigIo::inv_out.Clear();//inverter power off
        DigIo::dcsw_out.Clear();
        DigIo::gp_out2.Clear();//Negative contactors off
        DigIo::gp_out1.Clear();//Coolant pump off
//        DigIo::err_out.Clear();
        DigIo::prec_out.Clear();
        Param::SetInt(Param::opmode, newMode);
        if(targetVehicle == _vehmodes::BMW_E65) E65Vehicle.DashOff();
    }

}


static void Ms1Task(void)
{
//gpio_toggle(GPIOB,GPIO12);


        if(targetInverter == _invmodes::Prius_Gen3)
    {

    }
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern void parm_Change(Param::PARAM_NUM paramNum)
{
    // This function is called when the user changes a parameter
    if (Param::canspeed == paramNum)
        can->SetBaudrate((Can::baudrates)Param::GetInt(Param::canspeed));

    Throttle::potmin[0] = Param::GetInt(Param::potmin);
    Throttle::potmax[0] = Param::GetInt(Param::potmax);
    Throttle::potmin[1] = Param::GetInt(Param::pot2min);
    Throttle::potmax[1] = Param::GetInt(Param::pot2max);
    Throttle::throtmax = Param::Get(Param::throtmax);
    Throttle::throtmin = Param::Get(Param::throtmin);
    Throttle::idcmin = Param::Get(Param::idcmin);
    Throttle::idcmax = Param::Get(Param::idcmax);
    Throttle::udcmin = FP_MUL(Param::Get(Param::udcmin), FP_FROMFLT(0.95)); //Leave some room for the notification light
    targetInverter=static_cast<_invmodes>(Param::GetInt(Param::Inverter));//get inverter setting from menu
    Param::SetInt(Param::inv, targetInverter);//Confirm mode
    targetVehicle=static_cast<_vehmodes>(Param::GetInt(Param::Vehicle));//get vehicle setting from menu
    Param::SetInt(Param::veh, targetVehicle);//Confirm mode
    targetCharger=static_cast<_chgmodes>(Param::GetInt(Param::chargemodes));//get charger setting from menu
    targetChgint=static_cast<_interface>(Param::GetInt(Param::interface));//get interface setting from menu
    Param::SetInt(Param::Charger, targetCharger);//Confirm mode
    maxRevs=Param::GetInt(Param::revlim);//get revlimiter value
    if(ChgSet==1)
    {
    seconds=Param::GetInt(Param::Set_Sec);//only update these params if charge command is set to disable
    minutes=Param::GetInt(Param::Set_Min);
    hours=Param::GetInt(Param::Set_Hour);
    days=Param::GetInt(Param::Set_Day);
    ChgHrs_tmp=GetInt(Param::Chg_Hrs);
    ChgMins_tmp=GetInt(Param::Chg_Min);
    ChgDur_tmp=GetInt(Param::Chg_Dur);
    }
    ChgSet = Param::GetInt(Param::Chgctrl);//0=enable,1=disable,2=timer.
    ChgTicks = (GetInt(Param::Chg_Dur)*300);//number of 200ms ticks that equates to charge timer in minutes
}


static void CanCallback(uint32_t id, uint32_t data[2]) //This is where we go when a defined CAN message is received.
{
    switch (id)
    {
    case 0x521:
        ISA::handle521(data);//ISA CAN MESSAGE
        break;
    case 0x522:
        ISA::handle522(data);//ISA CAN MESSAGE
        break;
    case 0x523:
        ISA::handle523(data);//ISA CAN MESSAGE
        break;
    case 0x524:
        ISA::handle524(data);//ISA CAN MESSAGE
        break;
    case 0x525:
        ISA::handle525(data);//ISA CAN MESSAGE
        break;
    case 0x526:
        ISA::handle526(data);//ISA CAN MESSAGE
        break;
    case 0x527:
        ISA::handle527(data);//ISA CAN MESSAGE
        break;
    case 0x528:
        ISA::handle528(data);//ISA CAN MESSAGE
        break;
    case 0x108:
        chargerClass::handle108(data);// HV request from an external charger
        break;
    case 0x3b4:
        i3LIMClass::handle3B4(data);// Data msg from LIM
        break;
    case 0x272:
        i3LIMClass::handle272(data);// Data msg from LIM
        break;
    case 0x29e:
        i3LIMClass::handle29E(data);// Data msg from LIM
        break;
    case 0x2b2:
        i3LIMClass::handle2B2(data);// Data msg from LIM
        break;
    case 0x2ef:
        i3LIMClass::handle2EF(data);// Data msg from LIM
        break;
    default:
        if(targetVehicle == _vehmodes::BMW_E65)
        {
            // process BMW E65 CAS (Conditional Access System) return messages
            E65Vehicle.Cas(id, data);
            // process BMW E65 CAN Gear Stalk messages
            E65Vehicle.Gear(id, data);
        }
        if (targetInverter == _invmodes::OpenI)
        {
            // process leaf inverter return messages
            Can_OI::DecodeCAN(id, data);
        }

        if(targetVehicle == _vehmodes::BMW_E39)
        {
            Can_E39::DecodeCAN(id, data);
        }

        break;
    }
}


static void ConfigureVariantIO()
{
    hwRev = HW_REV1;
    Param::SetInt(Param::hwver, hwRev);

    ANA_IN_CONFIGURE(ANA_IN_LIST);
    DIG_IO_CONFIGURE(DIG_IO_LIST);

    AnaIn::Start();
}


extern "C" void tim3_isr(void)
{
    scheduler->Run();
}

extern "C" void rtc_isr(void)
{
	/* The interrupt flag isn't cleared by hardware, we have to do it. */
	rtc_clear_flag(RTC_SEC);    //This will fire every 10ms so we need to count to 100 to get a 1 sec tick.
    RTC_1Sec++;

        if(RTC_1Sec==100)
        {
            RTC_1Sec=0;
		if ( ++seconds >= 60 ) {
			++minutes;
			seconds -= 60;
		}
		if ( minutes >= 60 ) {
			++hours;
			minutes -= 60;
		}
		if ( hours >= 24 ) {
			++days;
			hours -= 24;
		}

        }
}



extern "C" int main(void)
{
    clock_setup();
    rtc_setup();
    ConfigureVariantIO();
    gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON,AFIO_MAPR_USART3_REMAP_PARTIAL_REMAP);//remap usart 3 to PC10 and PC11 for VCU HW
    //gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON,AFIO_MAPR_CAN2_REMAP);//32f107
    usart_setup();
    usart2_setup();//TOYOTA HYBRID INVERTER INTERFACE
    nvic_setup();
    term_Init();
    parm_load();
    spi2_setup();
    parm_Change(Param::PARAM_LAST);
    DigIo::inv_out.Clear();//inverter power off during bootup

    Can c(CAN1, (Can::baudrates)Param::GetInt(Param::canspeed));//can1 Inverter / isa shunt/LIM.
    Can c2(CAN2, (Can::baudrates)Param::GetInt(Param::canspeed));//can2 vehicle side.

    // Set up CAN 1 callback and messages to listen for
    c.SetReceiveCallback(CanCallback);
    c.RegisterUserMessage(0x1DA);//Leaf inv msg
    c.RegisterUserMessage(0x55A);//Leaf inv msg
    c.RegisterUserMessage(0x679);//Leaf obc msg
    c.RegisterUserMessage(0x390);//Leaf obc msg
    c.RegisterUserMessage(0x190);//Open Inv Msg
    c.RegisterUserMessage(0x19A);//Open Inv Msg
    c.RegisterUserMessage(0x1A4);//Open Inv Msg
    c.RegisterUserMessage(0x521);//ISA MSG
    c.RegisterUserMessage(0x522);//ISA MSG
    c.RegisterUserMessage(0x523);//ISA MSG
    c.RegisterUserMessage(0x524);//ISA MSG
    c.RegisterUserMessage(0x525);//ISA MSG
    c.RegisterUserMessage(0x526);//ISA MSG
    c.RegisterUserMessage(0x527);//ISA MSG
    c.RegisterUserMessage(0x528);//ISA MSG
    c.RegisterUserMessage(0x3b4);//LIM MSG
    c.RegisterUserMessage(0x29e);//LIM MSG
    c.RegisterUserMessage(0x2b2);//LIM MSG
    c.RegisterUserMessage(0x2ef);//LIM MSG
    c.RegisterUserMessage(0x272);//LIM MSG

    // Set up CAN 2 (Vehicle CAN) callback and messages to listen for.
    c2.SetReceiveCallback(CanCallback);
    c2.RegisterUserMessage(0x130);//E65 CAS
    c2.RegisterUserMessage(0x192);//E65 Shifter
    c2.RegisterUserMessage(0x108);//Charger HV request
    c2.RegisterUserMessage(0x153);//E39/E46 ASC1 message

    can = &c; // FIXME: What about CAN2?

    Stm32Scheduler s(TIM3); //We never exit main so it's ok to put it on stack
    scheduler = &s;

    s.AddTask(Ms1Task, 1);
    s.AddTask(Ms10Task, 10);
    s.AddTask(Ms100Task, 100);
    s.AddTask(Ms200Task, 200);


    // ISA::initialize();//only call this once if a new sensor is fitted. Might put an option on web interface to call this....
    //  DigIo::prec_out.Set();//commence precharge
    Param::SetInt(Param::version, 4); //backward compatibility

    term_Run();

    return 0;
}

/**
 ** pwm_lib library
 ** Copyright (C) 2015,2020
 **
 **   Antonio C. Domínguez Brito <antonio.dominguez@ulpgc.es>
 **     División de Robótica y Oceanografía Computacional <www.roc.siani.es>
 **     and Departamento de Informática y Sistemas <www.dis.ulpgc.es>
 **     Universidad de Las Palmas de Gran  Canaria (ULPGC) <www.ulpgc.es>
 **  
 ** This file is part of the pwm_lib library.
 ** The pwm_lib library is free software: you can redistribute it and/or modify
 ** it under  the  terms of  the GNU  General  Public  License  as  published  by
 ** the  Free Software Foundation, either  version  3  of  the  License,  or  any
 ** later version.
 ** 
 ** The  pwm_lib library is distributed in the hope that  it  will  be  useful,
 ** but   WITHOUT   ANY WARRANTY;   without   even   the  implied   warranty   of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE.  See  the  GNU  General
 ** Public License for more details.
 ** 
 ** You should have received a copy  (COPYING file) of  the  GNU  General  Public
 ** License along with the pwm_lib library.
 ** If not, see: <http://www.gnu.org/licenses/>.
 **/
/* 
 * File: pwm_defs.cpp
 * Description: This file includes definitions for using Arduino DUE's
 * ATMEL ATSAM3X8E microcontroller PWM modules. 
 * Date: December 20th, 2015
 * Author: Antonio C. Dominguez-Brito <antonio.dominguez@ulpgc.es>
 * ROC-SIANI - Universidad de Las Palmas de Gran Canaria - Spain
 */

#include "pwm_defs.h"

namespace arduino_due
{

  namespace pwm_lib
  {

    namespace pwm_core
    {
      const uint32_t two_power_values[max_clocks+1]=
      {
        0, 
        1, 
        2, 
        3,
        4, 
        5, 
        6,  
        7, 
        8,
        9, 
        10, 
        11, 
        17 
      };

      const double max_periods[max_clocks+1]=
      {
        max_period(0), 
        max_period(1),
        max_period(2), 
        max_period(3), 
        max_period(4),
        max_period(5), 
        max_period(6), 
        max_period(7), 
        max_period(8),  
        max_period(9),  
        max_period(10), 
        max_period(11), 
        max_period(12) 
      };
 
      const uint32_t clock_masks[max_clocks+1]=
      {
        PWM_CMR_CPRE_MCK,
        PWM_CMR_CPRE_MCK_DIV_2,
        PWM_CMR_CPRE_MCK_DIV_4,
        PWM_CMR_CPRE_MCK_DIV_8,
        PWM_CMR_CPRE_MCK_DIV_16,
        PWM_CMR_CPRE_MCK_DIV_32,
        PWM_CMR_CPRE_MCK_DIV_64,
        PWM_CMR_CPRE_MCK_DIV_128,
        PWM_CMR_CPRE_MCK_DIV_256,
        PWM_CMR_CPRE_MCK_DIV_512,
        PWM_CMR_CPRE_MCK_DIV_1024,
        PWM_CMR_CPRE_CLKA,
        PWM_CMR_CPRE_CLKB
      };

      const double tick_times[max_clocks+1]=
      {
        tick_time(0),
        tick_time(1),
        tick_time(2), 
        tick_time(3), 
        tick_time(4), 
        tick_time(5), 
        tick_time(6), 
        tick_time(7), 
        tick_time(8), 
        tick_time(9), 
        tick_time(10), 
        tick_time(11), 
        tick_time(12) 
      };
 
      bool find_clock(
        uint32_t period, 
        uint32_t& clock
      ) noexcept
      {
        for( 
          clock=0; 
          (clock<=max_clocks) && 
          (static_cast<double>(period)/100000000>max_periods[clock]);
          clock++
        ) { 

        if(clock>max_clocks) return false;
        return true;
      }

      void pwmc_setdutycycle(Pwm* pPwm,uint32_t ul_channel,uint16_t duty)
      {

        if ((pPwm->PWM_SR & (1 << ul_channel)) == 0) {

            pPwm->PWM_CH_NUM[ul_channel].PWM_CDTY = duty;
        }

        else {

            pPwm->PWM_CH_NUM[ul_channel].PWM_CDTYUPD = duty;
        }
      }

    }

  }

}



/*
    Copyright 2017 fishpepper <AT> gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http:// www.gnu.org/licenses/>.

   author: fishpepper <AT> gmail.com
*/

#include "adc.h"
#include "hal_adc.h"
#include "debug.h"

void adc_init(void) {
    #ifdef FEATURE_ADC
    debug("adc: init\n"); debug_flush();
    hal_adc_init();
    #else
    debug("adc: Disabled - FEATURE_ADC not defined\n"); debug_flush();
    #endif
}


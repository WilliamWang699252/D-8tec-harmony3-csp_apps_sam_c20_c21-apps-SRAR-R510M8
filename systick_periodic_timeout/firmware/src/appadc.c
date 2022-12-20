#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <stdio.h>
#include "definitions.h"                // SYS function prototypes
#include "appiot.h"
#include "appadc.h"    

#define ADC_VREF                (5.0f)

uint16_t adc_count=0;
float input_voltage=0;
void adc_convertion_fun(void){
    
    float tmp_float;
    
    ADC0_Enable();
    
    SYSTICK_DelayMs(1);
    
    /* Start ADC conversion */
    ADC0_ConversionStart();

    /* Wait till ADC conversion result is available */
    while(!ADC0_ConversionStatusGet())
    {
    };
    
    /* Read the ADC result */
    adc_count = ADC0_ConversionResultGet();
    input_voltage = (float)adc_count * ADC_VREF / 4095U;
    
    __NOP();
    __NOP();
    
    ADC0_Disable();
    
    tmp_float = input_voltage * 12;
    
    if(tmp_float <= 17.f)
        _iotdata.BKPOW_status = 0;
    else if((tmp_float > 17.0f ) && (tmp_float <= 35.32f))
        _iotdata.BKPOW_status = 1;
    else if((tmp_float > 35.32f ) && (tmp_float <= 36.16f))
        _iotdata.BKPOW_status = 2;
    else if((tmp_float > 36.16f ) && (tmp_float <= 37.49f))
        _iotdata.BKPOW_status = 3;
    else if((tmp_float > 37.49f ) && (tmp_float <= 39.43f))
        _iotdata.BKPOW_status = 4;
    else if(tmp_float > 40.53f )
        _iotdata.BKPOW_status = 5;
}

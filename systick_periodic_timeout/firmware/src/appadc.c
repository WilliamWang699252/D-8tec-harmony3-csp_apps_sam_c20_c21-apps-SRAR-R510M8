#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <stdio.h>
#include "definitions.h"                // SYS function prototypes
#include "appadc.h"    

#define ADC_VREF                (5.0f)

uint16_t adc_count=0;
float input_voltage=0;
void adc_convertion_fun(void){
    
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
}

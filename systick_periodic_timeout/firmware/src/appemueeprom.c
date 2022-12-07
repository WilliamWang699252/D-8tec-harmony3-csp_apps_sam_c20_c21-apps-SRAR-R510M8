
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <stdio.h>                     // 
#include "definitions.h"                // SYS function prototypes

#include "appemueeprom.h"
#include "library/emulated_eeprom/emulated_eeprom.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************




void emnueeprom_init(void){
    EMU_EEPROM_STATUS libStatus;
    
    libStatus = EMU_EEPROM_StatusGet();
    
    if((libStatus == EMU_EEPROM_STATUS_ERR_BAD_FORMAT) || (libStatus == EMU_EEPROM_STATUS_ERR_NOT_INITIALIZED)){
        
        /* Format EEPROM memory space */
        EMU_EEPROM_FormatMemory();
        
        libStatus = EMU_EEPROM_StatusGet();
        
        if(libStatus != EMU_EEPROM_STATUS_OK)
            NVIC_SystemReset();
    }
    
    //for test OK
    /*
    {
        static uint8_t tmp_8[4],tmp_81[4];
        
        sprintf((char *)tmp_8,"55AA");
        sprintf((char *)tmp_81,"0000");
        EMU_EEPROM_BufferWrite(0, (const uint8_t*)tmp_8, sizeof(tmp_8));
        EMU_EEPROM_BufferRead(0, (uint8_t*)tmp_81, sizeof(tmp_8));
        
        __NOP();
        __NOP();
    }
    */
    
}

void emueeprom_main(void){
    
    
    
    
    
    
    
 
    
    
    
}
/*******************************************************************************
 End of File
 */

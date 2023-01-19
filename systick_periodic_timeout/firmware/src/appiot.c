//#include <stddef.h>                     // Defines NULL
//#include <stdbool.h>                    // Defines true
//#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <stdio.h>
#include "definitions.h"                // SYS function prototypes
#include "apptimer.h"
#include "appiot.h"
#include "moden.h"
#include "canapp.h"
#include "appemueeprom.h"
#include "app_uart_debug.h"

#define CGPSINFO_MAX_LEN				(64)

#define IOT_SEND_DATA_MS    (1000*10) //10SEC

#define IOT_TIMEOUT_SEC  (5*60)  //5MIN

uint32_t iot_send_data_tick=0;

IOT_DATA _iotdata;
IOT_RAM_DATA _iotram;

MACHINE_DATA mcahineData;
IOT_TIMER _iottimer;

/****************************************************/
typedef struct iot_s
{
 //   QueueHandle_t		tx_queue;
 //	SemaphoreHandle_t	tx_sem;
 //   EventGroupHandle_t  flags;
 //   EventGroupHandle_t  ctrl_flags;
	struct tm 			timer;
    char*               uuid;
    char*               broker_url;
	char*				motor_version;
	int 				mqtt_topic_total_len;
	int 				mqtt_payload_total_len;
	uint16_t 			mqtt_topic_idx;
	uint16_t 			mqtt_payload_idx;
    uint16_t            broker_port;
    uint8_t             uuid_len;
    uint8_t             broker_url_len;
	uint8_t				motor_version_len;
	char 				cgpsinfo[CGPSINFO_MAX_LEN];
	char 				mqtt_clientid[24];
    iot_request_e       mode;
    iot_request_e       previous_mode;
} iot_t;
static iot_t iot;
/****************************************************************/
uint32_t mqtttime;
void init_iot(void){
 //   iot_init_t  init;
    memset(&iot, 0, sizeof(iot));
   
//	iot.uuid = init->uuid;
	
//	iot.uuid_len = init->uuid_len;
//	iot.mode = init->mode;
    
    _iotdata.state = IOT_NORMAL;//IOT_SLEEP;
    memset(_iotdata.UUID,0,sizeof(_iotdata.UUID));
    memset(_iotdata.PCBUUID,0,sizeof(_iotdata.PCBUUID));
    _iotdata.device_status = 1;
    _iotdata.lock_status = 0;
    _iotdata.BKPOW_status = 0;
    _iotdata.unlock_OK = 0;
    _iotdata.lock_OK = 0;
    _iotdata.dev_open_box_OK = 0;
    
    _iotdata.rtc_time.year = 20;
    _iotdata.rtc_time.month = 1;
    _iotdata.rtc_time.date = 1;
    _iotdata.rtc_time.hour = 1;
    _iotdata.rtc_time.minute = 1;
    _iotdata.rtc_time.second = 1;
    
    mqtttime=0;
    //readiotdata();
    //_iotram.odo=readOdoFromRam();
}

void setiotstate(IOT_STATES state){
    _iotdata.state = state;
    
}
void incmqtttimer(void){
    mqtttime++;
}

void updateiotodo(uint32_t odo){
    _iotram.odo = odo;
}

void iot_main(void){
    
    if((_moden.lte_4G_TX_flag == 0) && ((timer1ms - iot_send_data_tick) > IOT_SEND_DATA_MS)){
        iot_data_pack(0);
        _moden.lte_4G_TX_flag = 1;
        iot_send_data_tick = timer1ms;
    }
    
    /*
    if(_moden.lte_4G_TX_flag == 0) {
        static uint8_t state=0;
        
        switch(state){
            
            case 0:
                iot_data_pack(0);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
            case 1:
                iot_data_pack(1);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
            case 2:
                _iotdata.unlock_OK = 1;
                iot_data_pack(2);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
            case 3:
                _iotdata.unlock_OK = 2;
                iot_data_pack(2);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
            case 4:
                _iotdata.lock_OK = 1;
                iot_data_pack(3);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
            case 5:
                _iotdata.lock_OK = 2;
                iot_data_pack(3);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
            case 6:
                _iotdata.dev_open_box_OK = 1;
                iot_data_pack(4);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
            case 7:
                _iotdata.dev_open_box_OK = 2;
                iot_data_pack(4);
                _moden.lte_4G_TX_flag = 1;
                state++;
                break;
        }
    }
    */
    
    if(((_moden.lte_4G_RX_flag == 1) && (_moden.lte_4G_TX_flag == 0)) || ((_moden.lte_4G_RX_index1 != _moden.lte_4G_RX_index2) && ( _moden.lte_4G_TX_flag == 0))){
        uint8_t send_timeout_flag=0;
        
        _moden.lte_4G_RX_flag = 0;
        
        {
            char * adr;
                        
            adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"\"STIME\": \"");
            
            adr += 12;
            _iotdata.rtc_time.year = atoi(adr);
            adr += 3;
            _iotdata.rtc_time.month = atoi(adr);
            adr += 3;
            _iotdata.rtc_time.date = atoi(adr);
            adr += 3;
            _iotdata.rtc_time.hour = atoi(adr);
            adr += 3;
            _iotdata.rtc_time.minute = atoi(adr);
            adr += 3;
            _iotdata.rtc_time.second = atoi(adr);  
                        
            _iotdata.rtc_time.utc_number =  Drv_RTC_Read(&_iotdata.rtc_time);             
            
            if(_iotdata.rtc_time.utc_number >= _moden.rtc_time.utc_number){                
                if((_iotdata.rtc_time.utc_number - _moden.rtc_time.utc_number) > (IOT_TIMEOUT_SEC))
                    send_timeout_flag = 1;                
            }
            else{
                if((_moden.rtc_time.utc_number - _iotdata.rtc_time.utc_number) > (IOT_TIMEOUT_SEC))
                    send_timeout_flag = 1;            
            }
            
            #ifdef AT_IOT_DEBONG_ON
                {
                    char string[512];
                    sprintf(string,"year:%d,month:%d,date:%d,hour:%d,minute:%d,second:%d,UTC1:%d,UTC2:%d\r\n",
                            _iotdata.rtc_time.year,_iotdata.rtc_time.month,_iotdata.rtc_time.date,
                            _iotdata.rtc_time.hour,_iotdata.rtc_time.minute,_iotdata.rtc_time.second,
                            (int)_iotdata.rtc_time.utc_number,(int)_moden.rtc_time.utc_number);
                    uart_debug_megssage((uint8_t*)string, strlen(string));  
                    sprintf(string,"send_timeout_flag:%d\r\n",send_timeout_flag);
                    uart_debug_megssage((uint8_t*)string, strlen(string));  
                }
            #endif
            
            if(send_timeout_flag == 1)
                goto iot_out;
        }       
        
        //DEVCHKLIST
        if(strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"DEVCHKLIST")){
                
            char * adr;
            
            adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\": \"");
            if(adr){
                adr += 11;
            }
                
            if(adr == 0){
                adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\":\"");
                if(adr){
                    adr += 10;
                }
            }
                       
            memcpy(_iotdata.PCBUUID,adr,sizeof(_iotdata.PCBUUID)); 
                                   
            //DOWN
            if(_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)
                adr = strstr(_iotdata.PCBUUID,(char *)_moden.moden_uuid_md5);
            
            #ifdef AT_IOT_DEBONG_ON
                uart_debug_megssage((uint8_t*)_iotdata.PCBUUID, strlen(_iotdata.PCBUUID));
                uart_debug_megssage((uint8_t*)"\r\n", 2);
                uart_debug_megssage((uint8_t*)_moden.moden_uuid_md5, strlen((char *)_moden.moden_uuid_md5));
                uart_debug_megssage((uint8_t*)"\r\n", 2);
            #endif
                
            if(((adr != 0) && (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)) || (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 2)){
                
                iot_data_pack(1);
                _moden.lte_4G_TX_flag = 1;
            }                
        }    
        //DEVLOGIN
        else if(strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"DEVLOGIN")){
                
            _iotdata.device_status = 1;
            
            #ifdef AT_IOT_DEBONG_ON
                {
                    char string[512];
                    
                    sprintf(string,"_iotdata.device_status = %d\r\n", _iotdata.device_status);
                    uart_debug_megssage((uint8_t *)string, strlen(string));               
                }
            #endif
        }
        //DEVLOGOUT
        else if(strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"DEVLOGOUT")){
                
            _iotdata.device_status = 0;
            
            #ifdef AT_IOT_DEBONG_ON
                {
                    char string[512];
                    
                    sprintf(string,"_iotdata.device_status = %d\r\n", _iotdata.device_status);
                    uart_debug_megssage((uint8_t *)string, strlen(string));               
                }
            #endif
        }
        //DEVUNLOCK
        else if(strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"DEVUNLOCK")){
                
            char * adr;
            
            adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\": \"");
            if(adr){
                adr += 11;
            }
                
            if(adr == 0){
                adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\":\"");
                if(adr){
                    adr += 10;
                }
            }
                
            memcpy(_iotdata.PCBUUID,adr,sizeof(_iotdata.PCBUUID)); 
            
            #ifdef AT_IOT_DEBONG_ON
                {
                    char string[512];
                    
                    sprintf(string,"IOT DEVUNLOCK\r\n");
                    uart_debug_megssage((uint8_t *)string, strlen(string));               
                }
            #endif
            
            //DOWN
            if(_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)
                adr = strstr(_iotdata.PCBUUID,(char *)_moden.moden_uuid_md5);
                
            if(((adr != 0) && (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)) || (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 2)){
                
                iot_data_pack(2);
                _moden.lte_4G_TX_flag = 1;
            }
        }
        //DEVLOCK
        else if(strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"DEVLOCK")){
            
            char * adr;
            
            adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\": \"");
            if(adr){
                adr += 11;
            }
                
            if(adr == 0){
                adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\":\"");
                if(adr){
                    adr += 10;
                }
            }
                
            memcpy(_iotdata.PCBUUID,adr,sizeof(_iotdata.PCBUUID)); 
            
            #ifdef AT_IOT_DEBONG_ON
                {
                    char string[512];
                    
                    sprintf(string,"IOT DEVLOCK\r\n");
                    uart_debug_megssage((uint8_t *)string, strlen(string));               
                }
            #endif
            
            //DOWN
            if(_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)
                adr = strstr(_iotdata.PCBUUID,(char *)_moden.moden_uuid_md5);
                
            if(((adr != 0) && (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)) || (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 2)){
                iot_data_pack(3);
                _moden.lte_4G_TX_flag = 1;
            }
        }
        //DEVOPENBOX
        else if(strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"DEVOPENBOX")){
                
            char * adr;
            
            adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\": \"");
            if(adr){
                adr += 11;
            }
                
            if(adr == 0){
                adr = strstr((char *)_moden.lte_4G_RX_data[_moden.lte_4G_RX_index2],"PCBUUID\":\"");
                if(adr){
                    adr += 10;
                }
            }
            
            memcpy(_iotdata.PCBUUID,adr,sizeof(_iotdata.PCBUUID));
            
            #ifdef AT_IOT_DEBONG_ON
                {
                    char string[512];
                    
                    sprintf(string,"IOT DEVOPENBOX\r\n");
                    uart_debug_megssage((uint8_t *)string, strlen(string));               
                }
            #endif
            
            //DOWN
            if(_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)
                adr = strstr(_iotdata.PCBUUID,(char *)_moden.moden_uuid_md5);
                
            if(((adr != 0) && (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 1)) || (_moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] == 2)){
                iot_data_pack(4);
                _moden.lte_4G_TX_flag = 1;
            }
        }

iot_out:        
        _moden.lte_4G_RX_DOWN_PUB_flag[_moden.lte_4G_RX_index2] = 0;
        
        _moden.lte_4G_RX_index2++;
        if(_moden.lte_4G_RX_index2 >= LTE_4G_RX_DATA_COUNT)
            _moden.lte_4G_RX_index2 = 0;
    }
    
    
    switch(_iotdata.state){
        case IOT_SLEEP:
        break;
        case IOT_STANDBY:
        break;
        case IOT_NORMAL:
            
        break;
        case IOT_CLOSED:
        break;
        
//        case default:
//        break;
    }
    
}

void initmachinestates(void){
    mcahineData._moden = disconnect;
    mcahineData._network = disconnect;
    mcahineData._mqtt = disconnect;
    mcahineData._iot = working;
}

void setmcahinestates(MACHINE_STATES data){
    mcahineData._moden = data;
}
MACHINE_STATES readmcahinestates(void){
    return  mcahineData._moden ;
}
void setnetworkstates(MACHINE_STATES data){
    mcahineData._network = data;
}
void setmqttstates(MACHINE_STATES data){
    mcahineData._mqtt = data;
}

/*///////////////////////////////



*///////////////////////////////
void iot_data_pack(uint8_t pack_num){
    
    uint16_t lens;
       
    switch(pack_num){
        //DEVPING
        case 0:            
            memset(_moden.lte_4G_TX_data,0,sizeof(_moden.lte_4G_TX_data));
            sprintf((char *)_moden.lte_4G_TX_data,(const char *)"{"); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'PCBUUID':'%s',",_moden.moden_uuid_md5); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'STIME': '%d-%02d-%02d:%02d:%02d:%02d',",
                    _moden.rtc_time.year+2000,_moden.rtc_time.month,_moden.rtc_time.date,_moden.rtc_time.hour,_moden.rtc_time.minute,_moden.rtc_time.second); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD': 'DEVPING',"); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BASIC': {");
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)" 'MOTOVER': '%s',",motor_version);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'IOTVER': '%s',",iot_version);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'DEVSTU': '%d',",_iotdata.device_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LOCKSTU': '%d',",_iotdata.lock_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BKPOW': '%d',",_iotdata.BKPOW_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);  
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'GPSPOS': '%s,%s',",_moden.lte_4G_longitude,_moden.lte_4G_latitude);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)" 'MOTOECU':{");
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_batvoltage((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_motorpm((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_mototempture((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_ecuwarncode((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_ecuerrorcode((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_crankrpm((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_speed((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_crankavgpower((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            fill_motoavgpower((char *)&_moden.lte_4G_TX_data[lens]);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"}}}\"\r\n"); 
            
            break;
        //DEVCHKLIST
        case 1:
            memset(_moden.lte_4G_TX_data,0,sizeof(_moden.lte_4G_TX_data));
            sprintf((char *)_moden.lte_4G_TX_data,(const char *)"{"); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'PUBUUID':'%s',",_moden.moden_uuid_md5); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'STIME': '%d-%02d-%02d:%02d:%02d:%02d',",
                    _moden.rtc_time.year+2000,_moden.rtc_time.month,_moden.rtc_time.date,_moden.rtc_time.hour,_moden.rtc_time.minute,_moden.rtc_time.second); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD': 'DEVCHKLIST',"); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BASIC': {");
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)" 'MOTOVER': '%s',",motor_version);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'IOTVER': '%s',",iot_version);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'DEVSTU': '%d',",_iotdata.device_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LOCKSTU': '%d',",_iotdata.lock_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BKPOW': '%d',",_iotdata.BKPOW_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);   
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'GPSPOS': '%s,%s'",_moden.lte_4G_longitude,_moden.lte_4G_latitude);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"}}\"\r\n"); 
                    
            break;
        //unlock
        case 2:
            memset(_moden.lte_4G_TX_data,0,sizeof(_moden.lte_4G_TX_data));
            sprintf((char *)_moden.lte_4G_TX_data,(const char *)"{"); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
                        
            if(_iotdata.unlock_OK == 2){ //fail
                sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'PUBUUID':'%s',",_moden.moden_uuid_md5); 
                lens = strlen((char *)_moden.lte_4G_TX_data);
            }
            
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'STIME': '%d-%02d-%02d:%02d:%02d:%02d',",
                    _moden.rtc_time.year+2000,_moden.rtc_time.month,_moden.rtc_time.date,_moden.rtc_time.hour,_moden.rtc_time.minute,_moden.rtc_time.second); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            
            if(_iotdata.unlock_OK == 1) //OK
                sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD':'DEVUNLOCKOK',"); 
            //FAIL
            else
                sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD':'DEVUNLOCKFAIL',"); 
                            
            lens = strlen((char *)_moden.lte_4G_TX_data);  
            
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BASIC': {");
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'DEVSTU': '%d',",_iotdata.device_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LOCKSTU': '%d',",_iotdata.lock_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BKPOW': '%d',",_iotdata.BKPOW_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);   
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'GPSPOS': '%s,%s'",_moden.lte_4G_longitude,_moden.lte_4G_latitude);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"}}\"\r\n"); 
            
            break;
        //lock
        case 3:
            memset(_moden.lte_4G_TX_data,0,sizeof(_moden.lte_4G_TX_data));
            sprintf((char *)_moden.lte_4G_TX_data,(const char *)"{"); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'PUBUUID':'%s',",_moden.moden_uuid_md5); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'STIME': '%d-%02d-%02d:%02d:%02d:%02d',",
                    _moden.rtc_time.year+2000,_moden.rtc_time.month,_moden.rtc_time.date,_moden.rtc_time.hour,_moden.rtc_time.minute,_moden.rtc_time.second); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            
            if(_iotdata.lock_OK == 1) //OK
                sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD':'DEVLOCKOK',"); 
            //FAIL
            else
                sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD':'DEVLOCKFAIL',"); 
                            
            lens = strlen((char *)_moden.lte_4G_TX_data);  
            
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BASIC': {");
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'DEVSTU': '%d',",_iotdata.device_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LOCKSTU': '%d',",_iotdata.lock_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BKPOW': '%d',",_iotdata.BKPOW_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);   
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'GPSPOS': '%s,%s'",_moden.lte_4G_longitude,_moden.lte_4G_latitude);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"}}\"\r\n"); 
            
            break;
        //open box    
        case 4:
            memset(_moden.lte_4G_TX_data,0,sizeof(_moden.lte_4G_TX_data));
            sprintf((char *)_moden.lte_4G_TX_data,(const char *)"{"); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'PUBUUID':'%s',",_moden.moden_uuid_md5); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'STIME': '%d-%02d-%02d:%02d:%02d:%02d',",
                    _moden.rtc_time.year+2000,_moden.rtc_time.month,_moden.rtc_time.date,_moden.rtc_time.hour,_moden.rtc_time.minute,_moden.rtc_time.second); 
            lens = strlen((char *)_moden.lte_4G_TX_data);
            
            if(_iotdata.dev_open_box_OK == 1) //OK
                sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD':'DEVOPENBOXOK',"); 
            //FAIL
            else
                sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LINKCMD':'DEVOPENBOXFAIL',"); 
            
            lens = strlen((char *)_moden.lte_4G_TX_data);
            
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BASIC': {");
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'DEVSTU': '%d',",_iotdata.device_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'LOCKSTU': '%d',",_iotdata.lock_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'BKPOW': '%d',",_iotdata.BKPOW_status);
            lens = strlen((char *)_moden.lte_4G_TX_data);   
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"'GPSPOS': '%s,%s'",_moden.lte_4G_longitude,_moden.lte_4G_latitude);
            lens = strlen((char *)_moden.lte_4G_TX_data);
            sprintf((char *)&_moden.lte_4G_TX_data[lens],(const char *)"}}\"\r\n"); 
            
            break;
        case 5: 
            
            break;
        case 6:
            
            break;
        default:
            
            break;
    }   
}

/*===========================================================================+
|        ?????RTC?   ????2018?1?1?0?RTC???0??????     |
|        ?????2080?                                                    |
+===========================================================================*/
uint32_t Drv_RTC_Read(UTC_TIME *rtc_time_tmp){
										  // 1  2  3  4  5  6  7  8  9  10  11? 
	uint8_t month_day[11]={31,28,31,30,31,30,31,31,30,31,30},tmp;
	uint32_t rtc_value;
	uint16_t year;
	uint8_t month,day,hour,minute,second;
	uint32_t rtc_tmp=0;	

	year =(uint16_t) (rtc_time_tmp->year + 2000);
	month = rtc_time_tmp->month;
	day = rtc_time_tmp->date;
	hour = rtc_time_tmp->hour;
	minute = rtc_time_tmp->minute;
	second = rtc_time_tmp->second;

	rtc_value = ((year - 2018) * 31536000);  //????
	
	for(tmp=0;tmp<month-1;tmp++){		//??????
		rtc_tmp += month_day[tmp];
	}
	rtc_tmp = rtc_tmp*24*60*60;     //??????
	
	rtc_value += rtc_tmp;
	
	rtc_value += ((day-1)*24*60*60);  //??????
	
	rtc_value += ((hour*60*60) + (minute*60) + (second));

	//2?29??? //2020?2??29?
	if(year>=2020){
		rtc_tmp = 0;
		rtc_tmp = (((year-2020)/4)+1);
		
		if((month<3) &&
			 ((year==2020) || (year==2024) || (year==2028) ||//??3????
		    (year==2032) || (year==2036) || (year==2040) ||
				(year==2044) || (year==2048) || (year==2052) ||
				(year==2056) || (year==2060) || (year==2064) ||
				(year==2068) || (year==2072) || (year==2076))){               
		
			rtc_tmp--;
		}
		/*
		if(((year==2020)&&(month<3)) || ((year==2024)&&(month<3)) || ((year==2028)&&(month<3)) ||//??2????
		   ((year==2032)&&(month<3)) || ((year==2036)&&(month<3))	|| ((year==2040)&&(month<3)) ||
				(year==2044)&&(month<3)) || ((year==2048)&&(month<3))	|| ((year==2052)&&(month<3)) ||
				(year==2056)&&(month<3)) || ((year==2060)&&(month<3))	|| ((year==2064)&&(month<3)) ||
				(year==2068)&&(month<3)) || ((year==2072)&&(month<3))	|| ((year==2076)&&(month<3))){
					
			rtc_tmp--;
		}	
*/		
		
		rtc_value += (rtc_tmp*24*60*60);	
	}
	
	return rtc_value;
}

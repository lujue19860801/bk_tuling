
#ifndef __FACTORY_TOOLS_H__
#define __FACTORY_TOOLS_H__

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "MediaService.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#if 1
#define FACTORYTEST_ROUTER_SSID				"_WIFI_FACTORY_AUTO_TEST_"
#define FACTORYTEST_ROUTER_PASSWD			"wifi_auto_test_"
#else
#define FACTORYTEST_ROUTER_SSID				"apple123"
#define FACTORYTEST_ROUTER_PASSWD			"88888888"
#endif

#define	SSID_NAMESPACE 						"wifi_ssid"
#define	PSW_NAMESPACE 						"wifi_passwd"


#define ENTRY_FACTORY_TEST_MODE		 		"entry_factory"
#define EXIT_FACTORY_TEST_MODE		 		"exit_factory"

#define FACTORY_SERVICE_TASK_PRIORITY       13
#define FACTORY_SERVICE_TASK_STACK_SIZE     3*1024

#define FACTORY_REC_KEY_GPIO				GPIO_NUM_22

#define GPIO_OUTPUT_IO_1    				19
#define GPIO_OUTPUT_PIN_SEL  				(1ULL<<GPIO_OUTPUT_IO_1)

#define GPIO_HIGH_LEVEL						1
#define GPIO_LOW_LEVEL						0

#define FACTORY_MODE_PLAYSONG				"http://120.77.233.10/ESP32_FACTORY/factory_link_net_success.wav"



struct AudioTrack;

typedef struct FactoryTestService //extern from TreeUtility
{
    /*relation*/
    MediaService Based;
    /*private*/
    QueueHandle_t _FactoryTestQueue;
    struct AudioTrack *_receivedTrack;  //FIXME: is this needed??
} FactoryTestService;


int Entry_Factory_State(void);
int Get_FactoryTest_RouterInfo(void);
int Factory_Get_Flash_Router_Info(void);



FactoryTestService *FactoryTestServiceCreate();



























#endif

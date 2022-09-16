//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/**************************** 全局变量 ********************************/

uint32_t readbuff[50];

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
/* 接收任务句柄 */
static TaskHandle_t  Receive_Task_Handle = NULL;
/* 发送任务句柄 */
static TaskHandle_t  Send_Task_Handle = NULL;

//队列句柄
QueueHandle_t Test_Queue =NULL;


#define QUEUE_LEN 4 /* 队列的长度，最大可包含多少个消息 */ 
#define QUEUE_SIZE 4 /* 队列中每个消息大小（字节） */


//声明函数
static void Receive_Task(void* parameter);
static void Send_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 中断优先级分组为 4，即 4bit 都用来表示抢占优先级，范围为：0~15 
	* 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断， 
	* 都统一用这个优先级分组，千万不要再分组，切忌。 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//测试
//	led_G(on);
//	printf("串口测试");
}

int main()
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	BSP_Init();
	printf("这是全系列开发板-FreeRTOS-动态创建任务!\r\n");
	printf("按下 KEY1 或者 KEY2 发送队列消息\n");
	printf("Receive 任务接收到消息在串口回显\n\n");

	

	
	  /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
																							
	if(xReturn==pdPASS)
	{
		printf("初始任务创建成功\r\n");
		vTaskStartScheduler();
	}
	else 
	{
		return -1;
	}
	while(1)
	{
		
	}

}


//接收任务函数
static void Receive_Task(void* parameter)
{
	TickType_t timetick=xTaskGetTickCount();
	BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为 pdTRUE */
	
	while(1)
	{
		 xReturn=xQueueReceive(Test_Queue,     //接收数据的来源队列
		                       &readbuff,       //接收到数据存放地址
		                       portMAX_DELAY);  //阻塞时间（直到接收到数据位置）
		if(xReturn==pdTRUE)
		{
			printf("本次接收到的数据是%d\n",readbuff[0]); 
		}
		else
		{
			printf("数据接收出错,错误代码: 0x%lx\n",xReturn); 
		}
	}    
}

//按键任务函数
static void Send_Task(void* parameter)
{
	BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为 pdTRUE */
	uint32_t send_data1 = 1;
	uint32_t send_data2 = 2;
	while(1)
	{
		if(key_scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==1)
		{
			printf("发送消息1！\n"); 
			xReturn=xQueueSend(Test_Queue,     //发送到哪个队列
		                     &send_data1,    //发送的数据
		                     portMAX_DELAY);  //阻塞时间（直到接收到数据位置）
			
			if (pdPASS == xReturn) 
			printf("消息 send_data1 发送成功!\n\n"); 
		}
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			printf("发送消息2！\n"); 
			xReturn=xQueueSend(Test_Queue,     //发送到哪个队列
		                     &send_data2,    //发送的数据
		                     portMAX_DELAY);  //阻塞时间（直到接收到数据位置）
			
			if (pdPASS == xReturn) 
			printf("消息 send_data2 发送成功!\n\n");
			
		}
		vTaskDelay(20);/* 延时 20 个 tick */ 
		
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	taskENTER_CRITICAL();           //进入临界区
	
	//创建一个队列
	Test_Queue=xQueueCreate(QUEUE_LEN,QUEUE_SIZE);
	if (NULL != Test_Queue)
	{
		printf("创建 Test_Queue 消息队列成功!\r\n");
	}
	

	//创建接收任务函数
  xReturn=xTaskCreate((TaskFunction_t	)Receive_Task,		//任务函数
															(const char* 	)"Receive_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)2, 	//任务优先级
															(TaskHandle_t*  )&Receive_Task_Handle);/* 任务控制块指针 */ 	
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("Receive_Task任务创建成功!\n");
	else
		printf("Receive_Task任务创建失败!\n");
	
	
	 //创建 发送 任务
	 xReturn=xTaskCreate((TaskFunction_t	)Send_Task,		//任务函数
															(const char* 	)"Send_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)3, 	//任务优先级
															(TaskHandle_t*  )&Send_Task_Handle);/* 任务控制块指针 */ 
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("Send_Task任务创建成功!\n");
	else
		printf("Send_Task任务创建失败!\n");
	
	vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
	
	taskEXIT_CRITICAL();            //退出临界区
}


//静态创建任务才需要
///**
//  **********************************************************************
//  * @brief  获取空闲任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* 任务控制块内存 */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* 任务堆栈内存 */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
//}



///**
//  *********************************************************************
//  * @brief  获取定时器任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* 任务控制块内存 */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* 任务堆栈内存 */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
//}

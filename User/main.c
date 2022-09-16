//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/**************************** ȫ�ֱ��� ********************************/

uint32_t readbuff[50];

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
/* ���������� */
static TaskHandle_t  Receive_Task_Handle = NULL;
/* ���������� */
static TaskHandle_t  Send_Task_Handle = NULL;

//���о��
QueueHandle_t Test_Queue =NULL;


#define QUEUE_LEN 4 /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */ 
#define QUEUE_SIZE 4 /* ������ÿ����Ϣ��С���ֽڣ� */


//��������
static void Receive_Task(void* parameter);
static void Send_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 �ж����ȼ�����Ϊ 4���� 4bit ��������ʾ��ռ���ȼ�����ΧΪ��0~15 
	* ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ� 
	* ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ� 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//����
//	led_G(on);
//	printf("���ڲ���");
}

int main()
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	BSP_Init();
	printf("����ȫϵ�п�����-FreeRTOS-��̬��������!\r\n");
	printf("���� KEY1 ���� KEY2 ���Ͷ�����Ϣ\n");
	printf("Receive ������յ���Ϣ�ڴ��ڻ���\n\n");

	

	
	  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
																							
	if(xReturn==pdPASS)
	{
		printf("��ʼ���񴴽��ɹ�\r\n");
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


//����������
static void Receive_Task(void* parameter)
{
	TickType_t timetick=xTaskGetTickCount();
	BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��Ϊ pdTRUE */
	
	while(1)
	{
		 xReturn=xQueueReceive(Test_Queue,     //�������ݵ���Դ����
		                       &readbuff,       //���յ����ݴ�ŵ�ַ
		                       portMAX_DELAY);  //����ʱ�䣨ֱ�����յ�����λ�ã�
		if(xReturn==pdTRUE)
		{
			printf("���ν��յ���������%d\n",readbuff[0]); 
		}
		else
		{
			printf("���ݽ��ճ���,�������: 0x%lx\n",xReturn); 
		}
	}    
}

//����������
static void Send_Task(void* parameter)
{
	BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��Ϊ pdTRUE */
	uint32_t send_data1 = 1;
	uint32_t send_data2 = 2;
	while(1)
	{
		if(key_scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==1)
		{
			printf("������Ϣ1��\n"); 
			xReturn=xQueueSend(Test_Queue,     //���͵��ĸ�����
		                     &send_data1,    //���͵�����
		                     portMAX_DELAY);  //����ʱ�䣨ֱ�����յ�����λ�ã�
			
			if (pdPASS == xReturn) 
			printf("��Ϣ send_data1 ���ͳɹ�!\n\n"); 
		}
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			printf("������Ϣ2��\n"); 
			xReturn=xQueueSend(Test_Queue,     //���͵��ĸ�����
		                     &send_data2,    //���͵�����
		                     portMAX_DELAY);  //����ʱ�䣨ֱ�����յ�����λ�ã�
			
			if (pdPASS == xReturn) 
			printf("��Ϣ send_data2 ���ͳɹ�!\n\n");
			
		}
		vTaskDelay(20);/* ��ʱ 20 �� tick */ 
		
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	taskENTER_CRITICAL();           //�����ٽ���
	
	//����һ������
	Test_Queue=xQueueCreate(QUEUE_LEN,QUEUE_SIZE);
	if (NULL != Test_Queue)
	{
		printf("���� Test_Queue ��Ϣ���гɹ�!\r\n");
	}
	

	//��������������
  xReturn=xTaskCreate((TaskFunction_t	)Receive_Task,		//������
															(const char* 	)"Receive_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)2, 	//�������ȼ�
															(TaskHandle_t*  )&Receive_Task_Handle);/* ������ƿ�ָ�� */ 	
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("Receive_Task���񴴽��ɹ�!\n");
	else
		printf("Receive_Task���񴴽�ʧ��!\n");
	
	
	 //���� ���� ����
	 xReturn=xTaskCreate((TaskFunction_t	)Send_Task,		//������
															(const char* 	)"Send_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)3, 	//�������ȼ�
															(TaskHandle_t*  )&Send_Task_Handle);/* ������ƿ�ָ�� */ 
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("Send_Task���񴴽��ɹ�!\n");
	else
		printf("Send_Task���񴴽�ʧ��!\n");
	
	vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
	
	taskEXIT_CRITICAL();            //�˳��ٽ���
}


//��̬�����������Ҫ
///**
//  **********************************************************************
//  * @brief  ��ȡ��������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* �����ջ�ڴ� */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* �����ջ��С */
//}



///**
//  *********************************************************************
//  * @brief  ��ȡ��ʱ������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* �����ջ�ڴ� */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* �����ջ��С */
//}

#ifdef __cplusplus
	extern "C" {
#endif

#include <HAL.h>
#include <EERTOS.h>
#include <usart.h>
#include <nRF24.h>

#ifdef __cplusplus
	}
#endif

//Команды для управления кнопками
#define activityButton 		0b01010101 // 0x55
#define notActivityButton 	0b10101010 // 0xAA
#define changeNumberButton	0b10011001 // 0x99

#define first_byte_wait		1
#define command_wait		2
#define button_num_wait	   	3
// Глобальные переменные =====================================================

u08 parsingUART_flag = first_byte_wait;
u08 nRF_TX_data[2] = {activityButton, 0x00};
u16 delayButtonPush = 5000; //задержка после нажатия кнопки в мс

// Прототипы задач ===========================================================
void ParsingUART(void);
void Parsing_nRF24(void);
void DelayButton(void);
//============================================================================
//Верктора прерываний 
//============================================================================
#ifdef __cplusplus
	extern "C" {
#endif
//RTOS Interrupt
	ISR (RTOS_ISR)
	{
		TimerService();
	}

//Прерывание по опустошению буффера USART
	ISR (USART_UDRE_vect)
	{
		USART_UDRE_Handler();
	}
	ISR (USART_RX_vect)
	{
		USART_RXC_Handler();
		SetTask(ParsingUART);
	}
#ifdef __cplusplus
	}
#endif
//============================================================================
//Область задач
//============================================================================

void ParsingUART(void)
{
	u08 temp;
	if ((temp = USART_GetChar()))
	{
		if (parsingUART_flag == first_byte_wait)
		{
			if (temp == 0xFF)
			{
				parsingUART_flag = command_wait;
			}
			else if (temp == 0xF0)
			{
				parsingUART_flag = button_num_wait;
			}
		}
		else
		{
			u08 sendData[3];
			sendData[0] = 0x0F;
			sendData[1] = temp;
			sendData[2] = 0;
			if (parsingUART_flag == command_wait)
			{
				if (temp == 0xFF) //PING
				{
					delayButtonPush = 0;
					nRF_TX_data[0] = activityButton;
				}
				else if (temp == 0x01) //ACTIVITY
				{
					nRF_TX_data[0] = activityButton;
				}
				else if (temp == 0x02) //NOT ACTIVITY
				{
					nRF_TX_data[0] = notActivityButton;
				}
			}
			else if (parsingUART_flag == button_num_wait)
			{
				sendData[0] = 0x02;
				nRF_TX_data[0] = changeNumberButton;
				nRF_TX_data[1] = temp;
				
			}
			parsingUART_flag = first_byte_wait;
			USART_SendStr(sendData);
		}
	}
}

void DelayButton(void)
{
	nRF_TX_data[0] = activityButton;
}

void Parsing_nRF24(void)
{
	u08 temp[3];
	temp[0] = 0x01;
	temp[2] = 0;
	if (temp[1] = nRF_get_byte())
	{
		//USART_SendStr(" RECEIVING BYTE: ");
		//USART_SendNum(temp);
		nRF_write_ACK_payload(nRF_TX_data, 2);
		if (nRF_TX_data[0] == activityButton)
		{
			USART_SendStr(temp);
			if (delayButtonPush != 0)
			{
				SetTimerTask(DelayButton, delayButtonPush);
			}
		}
		else if (nRF_TX_data[0] == changeNumberButton)
		{
			temp[0] = 0x03;
			USART_SendStr(temp);
		}
		nRF_TX_data[0] = notActivityButton;	
	}			
}
//==============================================================================
int main(void)
{
	InitAll();			// Инициализируем периферию
	USART_Init();		// Инициализация USART
	nRF_init();			// Инициализация nRF24L01
	InitRTOS();			// Инициализируем ядро
	RunRTOS();			// Старт ядра. 
	// Запуск фоновых задач.


	while(1) 		// Главный цикл диспетчера
	{
		wdt_reset();	// Сброс собачьего таймера
		TaskManager();	// Вызов диспетчера
		if (checkStatus())
		{
			SetTask(Parsing_nRF24);
		}
	}

	return 0;
}


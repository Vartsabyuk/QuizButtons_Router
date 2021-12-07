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

// Глобальные переменные =====================================================

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
		//USART_SendByte(temp);
		if (temp == 's')
		{
			USART_SendStr("STATUS REG: ");
			USART_PutChar(ReadReg(STATUS));
		}
		else if (temp == 'c')
		{
			USART_SendStr("CONFIG REG: ");
			USART_PutChar(ReadReg(CONFIG)); 
		}
		/*else if (temp == 'a')
		{
			USART_SendStr("ACTIVITY");
			nRF_TX_data[0] = activityButton;		
		}
		else if (temp == 'n')
		{
			USART_SendStr("NOT_ACTIVITY");
			nRF_TX_data[0] = notActivityButton;		
		}*/
		else if (temp == 'b')
		{
			USART_SendStr("CHANGE_BUTTON_NUM");
			nRF_TX_data[0] = changeNumberButton;
			nRF_TX_data[1] = 3;
		}
	}
}

void DelayButton(void)
{
	nRF_TX_data[0] = activityButton;
}

void Parsing_nRF24(void)
{
	u08 temp;
	if (temp = nRF_get_byte())
	{
		USART_SendStr(" RECEIVING BYTE: ");
		USART_SendNum(temp);
		nRF_write_ACK_payload(nRF_TX_data, 2);
		if (nRF_TX_data[0] == activityButton)
		{
			SetTimerTask(DelayButton, delayButtonPush);
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


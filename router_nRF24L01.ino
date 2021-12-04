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

#define buttonID 0b00000010
//#define delayButtonPush 5000 //задержка после нажатия кнопки в мс

// Глобальные переменные =====================================================

u08 pushButtonStatus = 0;


// Прототипы задач ===========================================================
void parsingUART(void);
void parsing_nRF24(void);
//============================================================================
//Верктора прерываний 
//============================================================================
#ifdef __cplusplus
	extern "C" {
#endif
//Прерывание изменения уровня пинов 1-й группы (порт С) PCINT[14:8]
	/*ISR (PCINT1_vect)
	{
		nRF_IRQ_handler();
		SetTask(parsing_nRF24);
	}*/
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
		SetTask(parsingUART);
	}
#ifdef __cplusplus
	}
#endif
//============================================================================
//Область задач
//============================================================================

void parsingUART(void)
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
		else if (temp == 'b')
		{
			USART_SendStr("SENDING BYTE");
			u08 TX_data[2] = {buttonID, 0xF0};
 			nRF_send_data(TX_data, 2);
		}
	}
}

void parsing_nRF24(void)
{
	u08 temp;
	if ((temp = nRF_get_byte()))
	{
		USART_SendStr("RECEIVING BYTE: ");
		USART_SendNum(temp);
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
			SetTask(parsing_nRF24);
		}
	}

	return 0;
}


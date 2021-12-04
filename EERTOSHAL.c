#include <EERTOSHAL.h>

//RTOS ������ ���������� �������
inline void RunRTOS (void)
{
	Disable_Interrupt
	TCCR0A = 1<<WGM01; 				        // Freq = CK/64 - ���������� ����� � ������������
	TCCR0B = 3<<CS00;						// ��������� ����� ���������� �������� ���������
	TCNT0  = 0;								// ���������� ��������� �������� ���������
	OCR0A  = LO(TimerDivider); 				// ���������� �������� � ������� ���������
	TIMSK0  = 0<<TOIE0|1<<OCIE0A;				// ��������� ���������� RTOS - ������ ��
	Enable_Interrupt
}

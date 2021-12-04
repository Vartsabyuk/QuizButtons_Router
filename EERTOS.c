#include <EERTOS.h>

#define TaskQueueMask  (TaskQueueSize - 1)

// ������� �����, ��������.
// ��� ������ - ��������� �� �������
volatile static TPTR	TaskQueue[TaskQueueSize];			// ������� ����������
u08 TaskQueue_Head;
u08 TaskQueue_Tail;
volatile static struct
{									
	TPTR GoToTask; 						// ��������� ��������
	u16 Time;							// �������� � ��
} MainTimer[MainTimerQueueSize];	// ������� ��������

// RTOS ����������. ������� ��������
inline void InitRTOS(void)
{
	u08	index;

	TaskQueue_Head = 0;
	TaskQueue_Tail = 0;


	for(index=0;index!=MainTimerQueueSize;index++) // �������� ��� �������.
	{
		MainTimer[index].GoToTask = Idle;
		MainTimer[index].Time = 0;
	}
}


//������ ��������� - ������� ����. 
inline void  Idle(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
  	sleep_mode();
}


// ������� ��������� ������ � �������. ������������ �������� - ��������� �� �������
// ���������� �������� - ��� ������.
void SetTask(TPTR TS)
{
	u08	nointerrupted = 0;

	if (STATUS_REG & (1<<Interrupt_Flag))  // ���� ���������� ���������, �� ��������� ��.
	{
		Disable_Interrupt
		nointerrupted = 1;					// � ������ ����, ��� �� �� � ����������. 
	}

	if ((u08)(TaskQueue_Tail - TaskQueue_Head) & (u08)~TaskQueueMask) // ����� �����
    {
    	if (nointerrupted)	Enable_Interrupt 	// ���� �� �� � ����������, �� ��������� ����������
		return;									// ������ ������� ���������� ��� ������ - ������� �����������. ���� �����.
    }

    TaskQueue[TaskQueue_Tail & TaskQueueMask] = TS;  	//��������� � ������� ������
    ++TaskQueue_Tail;                                	//�������� �����
	
	if (nointerrupted) Enable_Interrupt					// � �������� ���������� ���� �� � ����������� ����������.
}


//������� ��������� ������ �� �������. ������������ ��������� - ��������� �� �������, 
// ����� �������� � ����� ���������� �������. ��������� ��� ������.
void SetTimerTask(TPTR TS, u16 NewTime)
{
	u08	index;
	u08 index_Empty;
	BOOL buff_Full;
	u08	nointerrupted = 0;

	if (STATUS_REG & (1<<Interrupt_Flag)) 			// �������� ������� ����������, ���������� ������� ����
	{
		Disable_Interrupt
		nointerrupted = 1;
	}

	index_Empty = 0;
	buff_Full = TRUE;
	for(index=0;index!=MainTimerQueueSize;++index)	//����������� ������� ��������
	{
		if (MainTimer[index].GoToTask == Idle)      //���� ���� ����� ��� ������ �������� ���
		{
			index_Empty = index;
			buff_Full = FALSE;
		}
		else
			if (MainTimer[index].GoToTask == TS)			// ���� ��� ���� ������ � ����� �������
			{
				MainTimer[index].Time = NewTime;			// �������������� �� ��������
				if (nointerrupted) 	Enable_Interrupt		// ��������� ���������� ���� �� ���� ���������.
				return;										// �������. ������ ��� ��� �������� ��������. ���� �����
			}
	}
	if (~buff_Full)
	{
		MainTimer[index_Empty].GoToTask = TS;			// ��������� ���� �������� ������
		MainTimer[index_Empty].Time = NewTime;		// � ���� �������� �������
	}
	if (nointerrupted) 	Enable_Interrupt	// ��������� ����������
}


/*=================================================================================
��������� ����� ��. �������� �� ������� ������ � ���������� �� ����������.
*/

inline void TaskManager(void)
{
	TPTR GoToTask;
	Disable_Interrupt				// ��������� ����������!!!
	if (TaskQueue_Head == TaskQueue_Tail)               //���� ����� ����� ����
	{
		Enable_Interrupt			// ��������� ����������
		Idle(); 					// ��������� �� ��������� ������� �����
	}
    else
    {                   
        GoToTask = TaskQueue[TaskQueue_Head & TaskQueueMask];   //��������� ����� ������  
        ++TaskQueue_Head;                                    	//�������� ������ 
        Enable_Interrupt							// ��������� ����������
		(GoToTask)();								// ��������� � ������
    }
}


/*
������ �������� ����. ������ ���������� �� ���������� ��� � 1��. ���� ����� ����� ����������� � ����������� �� ������

To DO: �������� � ����������� ��������� ������������ ������� ��������. ����� ����� ����� ��������� �� ����� ������. 
� ����� ������������ ��� ������� ������������ �������. 
� ���� ������ �� ������ �������� �������� ����������. 
*/
inline void TimerService(void)
{
	u08 index;

	for(index=0;index!=MainTimerQueueSize;index++)		// ����������� ������� ��������
	{
		if(MainTimer[index].GoToTask != Idle)
		{
			if(MainTimer[index].Time !=0)						// ���� ������ �� ��������, �� ������� ��� ���. 												// To Do: ��������� �� ������, ��� ����� !=1 ��� !=0. 
				MainTimer[index].Time --;						// ��������� ����� � ������ ���� �� �����.

			else
			{
				SetTask(MainTimer[index].GoToTask);				// ��������� �� ����? ������ � ������� ������
				MainTimer[index].GoToTask = Idle;				// � � ������ ����� �������
			}
		}
	}
}

/*
Module	:	ModBus_Command_Queue
Author	:	Peng Jin Biao
Date	:	2022-3-29

*/
#include "string.h"
#include "ModBus_Command_Queue.h"

/*���Ͷ���*/
typedef struct
{
	unsigned int front;       //����ͷ
	unsigned int rear;        //����β
	ModBus_Command_Type dat[ModBus_Command_Queue_Size];
}ModBus_Command_Queue_Type;


/******************************************************************/
/*static variable*/
/******************************************************************/
static ModBus_Command_Queue_Type ModBus_Command_Queue; /*������Ϣ����*/

/*��ʼ��ModBus_Command_Queue*/
unsigned int ModBus_Command_Queue_Init(void)
{
	ModBus_Command_Queue.front = ModBus_Command_Queue.rear = 0;//��ʼ��ʱ����ͷ����������
	return ModBus_Command_Queue_SUCCESS;
}

/*��ModBus_Command_Queue�������ݣ��ɹ��Ļ�����Queue_SUCCESS*/
unsigned int ModBus_Command_Queue_Put(const ModBus_Command_Type * cmd)
{
	unsigned int ret; /*����ֵ��ʱ����*/
	/* ���жϣ���ֹ���жϸ��� */

	/*��������*/
	memcpy(ModBus_Command_Queue.dat + ModBus_Command_Queue.rear,cmd,sizeof(ModBus_Command_Type));
	ModBus_Command_Queue.rear = (ModBus_Command_Queue.rear + 1) % ModBus_Command_Queue_Size;
	ret = ModBus_Command_Queue_SUCCESS;

	/* ���жϣ��ָ��ж���Ӧ */
	return ret;
}

/*��ModBus_Command_Queue���ȡ���ݣ��ɹ��Ļ�����Queue_SUCCESS*/
unsigned int ModBus_Command_Queue_Get(ModBus_Command_Type *cmd)
{
	unsigned int ret; /*����ֵ��ʱ����*/
	/* TODO: ���жϣ���ֹ���жϸ���*/
	if(ModBus_Command_Queue.front == ModBus_Command_Queue.rear)
	{
		/*front��rear��ͬ������Ϊ�գ���ȡʧ��*/
		ret = ModBus_Command_Queue_FAILED;
	}
	else
	{
		/*��������*/
		memcpy(cmd,ModBus_Command_Queue.dat + ModBus_Command_Queue.front,sizeof(ModBus_Command_Type));
		ModBus_Command_Queue.front = (ModBus_Command_Queue.front + 1) % ModBus_Command_Queue_Size;
		ret = ModBus_Command_Queue_SUCCESS;
	}
	/* TODO: ���жϣ��ָ��ж���Ӧ*/
	return ret;
}

unsigned int ModBus_Command_Queue_Is_Empty(void)
{
	if(ModBus_Command_Queue.front == ModBus_Command_Queue.rear)
	{
		/*front��rear��ͬ������Ϊ�գ���ȡʧ��*/
		return 1;
	}
	else
	{
		/* ���зǿ� */
		return 0;
	}
}

unsigned int ModBus_Command_Queue_Count(void)
{
	if(ModBus_Command_Queue.front == ModBus_Command_Queue.rear)
	{
		return 0;
	}
	else if(ModBus_Command_Queue.front > ModBus_Command_Queue.rear)
	{
		return (ModBus_Command_Queue_Size + ModBus_Command_Queue.rear - ModBus_Command_Queue.front);
	}
	else
	{
		return (ModBus_Command_Queue.rear - ModBus_Command_Queue.front);
	}
}


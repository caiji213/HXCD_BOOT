/*
Module	:	ModBus_Command_Queue
Author	:	Peng Jin Biao
Date	:	2022-3-29

*/
#include "string.h"
#include "ModBus_Command_Queue.h"

/*发送队列*/
typedef struct
{
	unsigned int front;       //队列头
	unsigned int rear;        //队列尾
	ModBus_Command_Type dat[ModBus_Command_Queue_Size];
}ModBus_Command_Queue_Type;


/******************************************************************/
/*static variable*/
/******************************************************************/
static ModBus_Command_Queue_Type ModBus_Command_Queue; /*发送消息队列*/

/*初始化ModBus_Command_Queue*/
unsigned int ModBus_Command_Queue_Init(void)
{
	ModBus_Command_Queue.front = ModBus_Command_Queue.rear = 0;//初始化时队列头队列首相连
	return ModBus_Command_Queue_SUCCESS;
}

/*往ModBus_Command_Queue里存放数据，成功的话返回Queue_SUCCESS*/
unsigned int ModBus_Command_Queue_Put(const ModBus_Command_Type * cmd)
{
	unsigned int ret; /*返回值临时变量*/
	/* 关中断，防止被中断更改 */

	/*拷贝数据*/
	memcpy(ModBus_Command_Queue.dat + ModBus_Command_Queue.rear,cmd,sizeof(ModBus_Command_Type));
	ModBus_Command_Queue.rear = (ModBus_Command_Queue.rear + 1) % ModBus_Command_Queue_Size;
	ret = ModBus_Command_Queue_SUCCESS;

	/* 开中断，恢复中断响应 */
	return ret;
}

/*从ModBus_Command_Queue里获取数据，成功的话返回Queue_SUCCESS*/
unsigned int ModBus_Command_Queue_Get(ModBus_Command_Type *cmd)
{
	unsigned int ret; /*返回值临时变量*/
	/* TODO: 关中断，防止被中断更改*/
	if(ModBus_Command_Queue.front == ModBus_Command_Queue.rear)
	{
		/*front和rear相同，队列为空，获取失败*/
		ret = ModBus_Command_Queue_FAILED;
	}
	else
	{
		/*拷贝数据*/
		memcpy(cmd,ModBus_Command_Queue.dat + ModBus_Command_Queue.front,sizeof(ModBus_Command_Type));
		ModBus_Command_Queue.front = (ModBus_Command_Queue.front + 1) % ModBus_Command_Queue_Size;
		ret = ModBus_Command_Queue_SUCCESS;
	}
	/* TODO: 开中断，恢复中断响应*/
	return ret;
}

unsigned int ModBus_Command_Queue_Is_Empty(void)
{
	if(ModBus_Command_Queue.front == ModBus_Command_Queue.rear)
	{
		/*front和rear相同，队列为空，获取失败*/
		return 1;
	}
	else
	{
		/* 队列非空 */
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


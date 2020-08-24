#include "link.h"

//创建一个链表
pNode Creat_link_list(void)
{
	int len;//链表长度
	int val;//节点放入的数据

	printf("请输入需要创建的节点个数len:");
	scanf_s("%d",&len);

	pNode pHead = (pNode)malloc(sizeof(NODE));//创建头节点
	pNode pTail=pHead;                        //创建指向尾节点指针
	pTail->next = NULL;                       //尾节点指针域为空
	for (int i = 0; i < len; i++)
	{
		printf("请输入需要放入节点的数据：");
		scanf_s("%d", &val);

		pNode pNew = (pNode)malloc(sizeof(NODE));//创建一个新的节点
		pNew->data = val;
		pTail->next = pNew;                      //新创建节点挂尾节点后面
		pTail = pNew;                            //新创建的节点设为尾节点
		pTail->next = NULL;                      //尾节点指针域空
	}
	return pHead;                                //返回头指针
}

//遍历链表
void Traverst(pNode pHead)
{
	pNode P=pHead->next;               
	
	while(P != NULL)                            
	{
		printf("%d\n",P->data);
		P = P->next;
	}
}

//判断链表是否为空
bool Empty(pNode pHead)
{
	if (pHead->next == NULL)               //判断头节点指针域是否为空
		return true;
	else
		return false;
}

//求链表长度
int Length(pNode pHead)
{
	int length=0;
	pNode pnode=pHead->next;
	while (pnode != NULL)                //直到指针域为空的节点
	{
		length++;
		pnode = pnode->next;
	}
	return length;
}

//排序
void  Sort(pNode pHead)
{
	int i, j, len,val;
	pNode p, q;
	len = Length(pHead);
	for (i = 0,p=pHead->next; i < len-1; i++,p=p->next)  //每大循环一次，p往后移一位
	{
		for (j = i+1,q=p->next; j < len; j++,q=q->next)  //每小循环一次，q往后移一位
		{
			if (p->data > q->data)
			{
				val = q->data;      //把后面那个小的值存起来
				q->data = p->data;  //把前面那个大值赋给后面
				p->data = val;      //再把小值给前面
			}
		}
	}
}

//插入一个节点
void Insert(pNode pHead, int site, int val)
{
	int i=0;
	pNode p = pHead;
	while (i < site-1 && p->data!=NULL)       //指针p移到第site的前一个节点
	{
		p = p->next;
		i++;
	}
	pNode pNew = (pNode)malloc(sizeof(NODE));  //创建一个新节点
	pNew->data = val;						   //
	pNode q = p->next;						   //放p后面那个节点地址
	pNew->next = q;							   //p后面节点挂在pNew后
	p->next = pNew;							   //pNew挂在p后面
}

//删除第site个节点
void Delete(pNode pHead,int site)
{
	int i=0;
	pNode p = pHead;
	while(i < site - 1 && p->next != NULL)
	{
		i++;
		p = p->next;
	}
	pNode q = p->next;
	p->next = q->next;
	free(q);

}

//链表整体删除
void Clear_list(pNode pHead)
{
	pNode q;
	pNode p = pHead->next;
	while (p != NULL)
	{
	    q = p->next;
		free(p);
		p = q;
	}
	pHead->next = NULL;
}

//数组排序
void Array_sort(int len,int* array)
{
	int val;
	for (int i = 0; i < len-1; i++)
	{
		for (int j=i+1;j<len;j++)
		{
			if (array[i] > array[j])
			{
				val = array[j];
				array[j] = array[i];
				array[i] = val;
			}
		}
	}

}
//数组遍历
void Array_traverst(int len, int* array)
{
	for (int i = 0; i < len; i++)
	{
		printf("%d\n",array[i]);
	}
}
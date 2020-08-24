#include "link.h"

//����һ������
pNode Creat_link_list(void)
{
	int len;//������
	int val;//�ڵ���������

	printf("��������Ҫ�����Ľڵ����len:");
	scanf_s("%d",&len);

	pNode pHead = (pNode)malloc(sizeof(NODE));//����ͷ�ڵ�
	pNode pTail=pHead;                        //����ָ��β�ڵ�ָ��
	pTail->next = NULL;                       //β�ڵ�ָ����Ϊ��
	for (int i = 0; i < len; i++)
	{
		printf("��������Ҫ����ڵ�����ݣ�");
		scanf_s("%d", &val);

		pNode pNew = (pNode)malloc(sizeof(NODE));//����һ���µĽڵ�
		pNew->data = val;
		pTail->next = pNew;                      //�´����ڵ��β�ڵ����
		pTail = pNew;                            //�´����Ľڵ���Ϊβ�ڵ�
		pTail->next = NULL;                      //β�ڵ�ָ�����
	}
	return pHead;                                //����ͷָ��
}

//��������
void Traverst(pNode pHead)
{
	pNode P=pHead->next;               
	
	while(P != NULL)                            
	{
		printf("%d\n",P->data);
		P = P->next;
	}
}

//�ж������Ƿ�Ϊ��
bool Empty(pNode pHead)
{
	if (pHead->next == NULL)               //�ж�ͷ�ڵ�ָ�����Ƿ�Ϊ��
		return true;
	else
		return false;
}

//��������
int Length(pNode pHead)
{
	int length=0;
	pNode pnode=pHead->next;
	while (pnode != NULL)                //ֱ��ָ����Ϊ�յĽڵ�
	{
		length++;
		pnode = pnode->next;
	}
	return length;
}

//����
void  Sort(pNode pHead)
{
	int i, j, len,val;
	pNode p, q;
	len = Length(pHead);
	for (i = 0,p=pHead->next; i < len-1; i++,p=p->next)  //ÿ��ѭ��һ�Σ�p������һλ
	{
		for (j = i+1,q=p->next; j < len; j++,q=q->next)  //ÿСѭ��һ�Σ�q������һλ
		{
			if (p->data > q->data)
			{
				val = q->data;      //�Ѻ����Ǹ�С��ֵ������
				q->data = p->data;  //��ǰ���Ǹ���ֵ��������
				p->data = val;      //�ٰ�Сֵ��ǰ��
			}
		}
	}
}

//����һ���ڵ�
void Insert(pNode pHead, int site, int val)
{
	int i=0;
	pNode p = pHead;
	while (i < site-1 && p->data!=NULL)       //ָ��p�Ƶ���site��ǰһ���ڵ�
	{
		p = p->next;
		i++;
	}
	pNode pNew = (pNode)malloc(sizeof(NODE));  //����һ���½ڵ�
	pNew->data = val;						   //
	pNode q = p->next;						   //��p�����Ǹ��ڵ��ַ
	pNew->next = q;							   //p����ڵ����pNew��
	p->next = pNew;							   //pNew����p����
}

//ɾ����site���ڵ�
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

//��������ɾ��
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

//��������
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
//�������
void Array_traverst(int len, int* array)
{
	for (int i = 0; i < len; i++)
	{
		printf("%d\n",array[i]);
	}
}
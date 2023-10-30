#ifndef CQUEUE
#define CQUEUE 
#include <windows.h>

template <typename T>
class CQueue;  

//�������� Queue�� ����Ÿ�� �� Node��� �� �� ���� 
template <typename T>
class CQueueData
{
	friend class CQueue<T>;	//CQueue Ŭ���� �ܿ��� ������ �� ���� 
	T value;				//���� �� 
	CQueueData<T>* Next;	//���� Queue
public:
	CQueueData()
	{
		Next = NULL;
		//value = NULL;
	}
	virtual ~CQueueData()
	{
		//if(value!=NULL)
		//	delete [] value;
	}
};


//QueueData�� �����ϴ� class
template <typename T>
class CQueue  
{
	friend class CQueue<T>;
	CRITICAL_SECTION cs;	//����ȭ�� ����
	#define QUEUEMAXSIZE 10000
	int nSize;		//Queue ũ�� 
	int nNum;		//���� Queue Data ��

	CQueueData<T>* Head;	//������� Queue�� �� ó���κ�
	CQueueData<T>* Last;	//������� Queue�� �� �������κ� 
	CQueueData<T>* Front;	//�ֱٿ� ���� �κ�
	CQueueData<T>* Rear;	//������ ���� �κ�  

	CQueueData<T>* Current;	//���� ����Ű�� �ִ� �κ�(��������ڿ��� ���)


	//���簡��Ű�� �ڷ���ġ�� �� ���� ����Ű���� �� 
	//������� 
	//�Ʒ� SetCurrentPosToFrontPos(), GetNext()�Լ��� 
	//����ȭ�� ���� �ʴ� ���� �� �Լ��� ��������ڿ����� ���Ǹ�(private�� �����)
	//�Ǻ��� Queue�� ���� ������ �ʴ´ٸ� �������� ����ȭ�� �������. 
	//��, �Ǻ��� Queue�� ����ȭ ó���ϸ� �ȴ�.  
	bool SetCurrentPosToFrontPos()
	{
		if(nNum == 0) 
		{
			return false;
		}
		Current->Next = Front->Next;
		return true;
	}

	//���� �ڷ������� 
	bool GetNext(T* val)
	{
		if(Current->Next == Rear->Next)
		{
			*val =0;
			return false;
		}
		*val = Current->Next->value;
		Current->Next = Current->Next->Next;
		return true;
	}


public:
	//������ 
	CQueue()
	{
		nNum = 0;						//�ʱ� �ڷ� ����
		nSize = 0;						//Queue�� ũ�� 
		InitializeCriticalSection(&cs); //CS�� �ʱ�ȭ 
	}

	//���������(���� ���縦 ���� ����)
	CQueue(CQueue<T>& queue)
	{
		EnterCriticalSection(&(queue.cs));	//�Ǻ���Ǵ� Queue�� ����Ÿ�� ������ ���ƾ� �ϹǷ�...
		nNum = 0;							//�ʱ� �ڷ� ����
		nSize = 0;							//Queue�� ũ�� 
		InitializeCriticalSection(&cs);		//CS�� �ʱ�ȭ 
		if(queue.SetCurrentPosToFrontPos() != false)
		{
			nSize = queue.GetSize();
			init(nSize);

			T data;
			while(queue.GetNext(&data))
			{
				insert(data);
			}	
		}
		LeaveCriticalSection(&(queue.cs));
	}

	//�ʱ�ȭ, Queue�� size�� ���ϰ� �޸� �Ҵ� 
	void init(int size)
	{
		EnterCriticalSection(&cs);
		Head = new CQueueData<T>;
		Front = new CQueueData<T>;
		Last = new CQueueData<T>;
		Rear = new CQueueData<T>;
		Current = new CQueueData<T>;

		nSize = size;
		nNum = 0;


		if(nSize < 1) nSize = 1;
		if( nSize > QUEUEMAXSIZE ) nSize = QUEUEMAXSIZE;

		CQueueData<T>* Now = new CQueueData<T>;
		for(int i=0; i < nSize; i++)
		{
			CQueueData<T>* New = new CQueueData<T>;

			if(i==0)
			{
				Head->Next = New;
				Front->Next = New;
				Rear->Next = New;
				Now->Next = New;
			}
			else
			{
				if(i+1 == nSize)
				{
					Now->Next->Next = New;
					Last->Next = New;
					New->Next = Head->Next; // ȯ���� ����� �ش�.
				}
				else
				{
					Now->Next->Next = New;
					Now->Next = New;
				}
			}
			

		}
		delete Now;
		LeaveCriticalSection(&cs);
	}

	//�ڷḦ �ִ´�.
	bool insert(const T& val)
	{
		EnterCriticalSection(&cs);
		//Full�ΰ��� false�� return
		if(nNum+1 > nSize)
		{
			LeaveCriticalSection(&cs);
			return false;
		}

		nNum++;
		Rear->Next->value = val;
		Rear->Next = Rear->Next->Next;
		LeaveCriticalSection(&cs);
		return true;
	}

	//�ڷḦ ������.
	bool extract(T* val)
	{
		EnterCriticalSection(&cs);
		//�ڷᰡ ���� ���� false�� return
		if(nNum==0)
		{
			*val = 0;
			LeaveCriticalSection(&cs);
			return false;
		}
		nNum--;
		*val = Front->Next->value;
		Front->Next = Front->Next->Next;
		LeaveCriticalSection(&cs);
		return true;
	}

	//��ü �ڷḦ ����.
	//�����δ� Front�� Rear�� �����͸� Head�� ���� 
	//�ڷ���� nNum=0���� �ξ� ���� ��ó�� �Ǵ��ϵ��� �� 
	bool empty()
	{
		EnterCriticalSection(&cs);
		Front->Next = Head->Next;
		Rear->Next = Head->Next;
		nNum =0;
		LeaveCriticalSection(&cs);
		return true;
	}

	//�ڷ� ���� 
	int GetNumOfData() const
	{
		return nNum;
	}

	//Queue�� ������ 
	int GetSize() const
	{
		return nSize;
	}

	//Queue�� �Ҹ��� 
	virtual ~CQueue()
	{
		EnterCriticalSection(&cs);
		CQueueData<T>* Now = new CQueueData<T>;
		Now->Next = Head->Next;

		for(;;)
		{
			Head->Next = Now->Next->Next;
			if(Head->Next!=Last->Next)
			{
				delete Now->Next;
			}
			else
			{
				break;
			}
			Now->Next = Head->Next;
		}

		delete Head;
		delete Front;
		delete Rear;
		delete Last; 
		delete Now;
		delete Current;
		LeaveCriticalSection(&cs);
		DeleteCriticalSection(&cs);
	}


};



#endif 

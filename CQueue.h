#ifndef CQUEUE
#define CQUEUE 
#include <windows.h>

template <typename T>
class CQueue;  

//실제적인 Queue의 데이타가 들어감 Node라고 할 수 있음 
template <typename T>
class CQueueData
{
	friend class CQueue<T>;	//CQueue 클래스 외에는 접근할 수 없음 
	T value;				//현재 값 
	CQueueData<T>* Next;	//다음 Queue
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


//QueueData를 관리하는 class
template <typename T>
class CQueue  
{
	friend class CQueue<T>;
	CRITICAL_SECTION cs;	//동기화를 위해
	#define QUEUEMAXSIZE 10000
	int nSize;		//Queue 크기 
	int nNum;		//현재 Queue Data 수

	CQueueData<T>* Head;	//만들어진 Queue의 맨 처음부분
	CQueueData<T>* Last;	//만들어진 Queue의 맨 마지막부분 
	CQueueData<T>* Front;	//최근에 들어온 부분
	CQueueData<T>* Rear;	//마지막 들어온 부분  

	CQueueData<T>* Current;	//현재 가리키고 있는 부분(복사생성자에서 사용)


	//현재가리키는 자료위치를 맨 앞을 가리키도록 함 
	//참고사항 
	//아래 SetCurrentPosToFrontPos(), GetNext()함수에 
	//동기화를 하지 않는 것은 이 함수는 복사생성자에서만 사용되며(private로 선언됨)
	//피복사 Queue의 값만 변하지 않는다면 직접적인 동기화는 상관없다. 
	//즉, 피복사 Queue만 동기화 처리하면 된다.  
	bool SetCurrentPosToFrontPos()
	{
		if(nNum == 0) 
		{
			return false;
		}
		Current->Next = Front->Next;
		return true;
	}

	//다음 자료참고함 
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
	//생성자 
	CQueue()
	{
		nNum = 0;						//초기 자료 갯수
		nSize = 0;						//Queue의 크기 
		InitializeCriticalSection(&cs); //CS를 초기화 
	}

	//복사생성자(깊은 복사를 위해 만듬)
	CQueue(CQueue<T>& queue)
	{
		EnterCriticalSection(&(queue.cs));	//피복사되는 Queue의 데이타가 변하지 말아야 하므로...
		nNum = 0;							//초기 자료 갯수
		nSize = 0;							//Queue의 크기 
		InitializeCriticalSection(&cs);		//CS를 초기화 
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

	//초기화, Queue의 size를 정하고 메모리 할당 
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
					New->Next = Head->Next; // 환형을 만들어 준다.
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

	//자료를 넣는다.
	bool insert(const T& val)
	{
		EnterCriticalSection(&cs);
		//Full인경우는 false를 return
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

	//자료를 빼낸다.
	bool extract(T* val)
	{
		EnterCriticalSection(&cs);
		//자료가 없는 경우는 false를 return
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

	//전체 자료를 비운다.
	//실제로는 Front와 Rear의 포인터를 Head로 놓고 
	//자료수를 nNum=0으로 두어 없는 것처럼 판단하도록 함 
	bool empty()
	{
		EnterCriticalSection(&cs);
		Front->Next = Head->Next;
		Rear->Next = Head->Next;
		nNum =0;
		LeaveCriticalSection(&cs);
		return true;
	}

	//자료 갯수 
	int GetNumOfData() const
	{
		return nNum;
	}

	//Queue의 사이즈 
	int GetSize() const
	{
		return nSize;
	}

	//Queue의 소멸자 
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

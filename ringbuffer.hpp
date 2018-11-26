#ifndef RINGBUFFER_H_INCLUDED
#define RINGBUFFER_H_INCLUDED
#include <iostream>
#include <string.h>

using namespace std;

/**
*  环形队列类模板
*
*/
template <class T>
class RingBuffer
{
public:
	RingBuffer(int capacity);
	virtual ~RingBuffer();
	//清队列
	void Clear();
	//入队列
	bool PushItem(T element);
	//出队列
	bool PopItem(T &element);   //传入一个T的引用，方便接队头，而不是返回队头，这样函数返回布尔，调用完毕后，引用拿到队头
	//弹出最新入队列的成员，并清空队列；
	bool PopLastedItem(T &element);
	//判断队列是否为空
	const bool isEmpty();
	//判断队列是否已满
	const bool isFull();
	//队列长度
	const int length();
	//列出队列成员
	void printQueue(void(*pFunc)(T));    //适配所有模板类的打印，传入一个对应类型的打印函数指针；
	void printQInfo();
private:
	//队列数组指针
	T * m_pQueue;
	//队列容量
	volatile unsigned long m_iCapacity;
	//队列头
	volatile unsigned long m_iHead;
	//队列尾部
	volatile unsigned long m_iTail;
};

template <class T>
RingBuffer<T>::RingBuffer(int capacity) {
	m_iCapacity = capacity;
	Clear();
	if ((m_pQueue = new T[m_iCapacity]) == NULL) {
		throw string("Queue Initialization Failed!");
	}
}

template <class T>
RingBuffer<T>::~RingBuffer() {
	delete[]m_pQueue;
	m_pQueue = NULL;
}

template <class T>
void RingBuffer<T>::Clear() 
{
	m_iHead = 0;
	m_iTail = 0;
}

template <class T>
bool RingBuffer<T>::PushItem(T element) 
{
	if (isFull())
	{
		return false;
	}
	m_pQueue[m_iTail % m_iCapacity] = element;
	m_iTail++;
	return true;
}

template <class T>
bool RingBuffer<T>::PopItem(T &element) 
{
	if (isEmpty())
	{
		return false;
	}
	//传入一个T的引用，方便接收队头，而不是返队头，这样函数返回布尔，调用完毕后，引用拿到队头
	element = m_pQueue[m_iHead % m_iCapacity];
	m_iHead++;
	return true;
}

template <class T>
bool RingBuffer<T>::PopLastedItem(T &element) 
{
	if (isEmpty())
	{
		return false;
	}
	m_iHead = m_iTail - 1;
	//传入一个T的引用，方便接收队头，而不是返队头，这样函数返回布尔，调用完毕后，引用拿到队头
	element = m_pQueue[m_iHead % m_iCapacity];
	m_iHead++;
	return true;
}

template <class T>
const bool RingBuffer<T>::isEmpty() 
{
	return (m_iHead == m_iTail) ? true : false;
}

template <class T>
const bool RingBuffer<T>::isFull() 
{
	return (m_iCapacity <= (m_iTail - m_iHead))? true : false;
}

template <class T>
const int RingBuffer<T>::length() 
{
	return (m_iTail - m_iHead);
}

template <class T>
void RingBuffer<T>::printQueue(void(*pFunc)(T))
{
	for (int i = m_iHead; i < m_iTail; i++) 
	{
		pFunc(m_pQueue[i % m_iCapacity]);
	}
}
template <class T>
void  RingBuffer<T>::printQInfo()
{
	printf("-- %d\t%d -- ",m_iTail,m_iHead );
}
#endif // RINGBUFFER_H_INCLUDED

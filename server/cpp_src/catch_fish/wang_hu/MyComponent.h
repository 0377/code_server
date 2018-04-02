////
#ifndef _MY_COMPONENT_H_
#define _MY_COMPONENT_H_

#include "common.h"
#include <memory>

class MyObject;
class MyComponent;
//����¼�
class CComEvent
{
public:
	CComEvent(){};
	virtual ~CComEvent(){};
	void SetID(uint32_t id){id_ = id;}
	uint32_t GetID() const{return id_;};

	int64_t	GetParam1()const{return param1_;}
	void	SetParam1(int64_t param){param1_ = param;}

	void*	GetParam2()const{return param2_;}
	void	SetParam2(void* param){param2_ = param;}

	//�¼��ķ�����
	MyComponent*	GetSender()const {return sender_;}
	void				SetSender(MyComponent* sender){sender_ = sender;}

protected:
	uint32_t				id_;            //�¼�ID 
	MyComponent*	sender_;            //���ָ��
	int64_t				param1_;        //���Ͳ���
	void*				param2_;        //�¼��Խ�
};

enum MyComponentType
{
	ECF_NONE = 0,
	ECF_MOVE,		//�ƶ����
	ECF_VISUAL,		//���ӻ����                     //ǰ��ʹ��
	ECF_EFFECTMGR,	//����Ч��������
	ECF_BUFFERMGR,	//�£գƣƣţҹ�����
};
//���
class MyComponent
{
public:
	MyComponent():owner_(0){};
	virtual ~MyComponent(){};

	const uint32_t GetID() const {return id_;};
	virtual const uint32_t GetFamilyID() const{return id_ >> 8 ;};

	//���ӵ�����󱻵���
	virtual void OnAttach(){};
	//�Ӷ����Ƴ�ǰ������
	virtual void OnDetach(){};

	//��Ӧʱ������
	virtual void OnUpdate(int ms){};

	//��Ӧ�����Ϣ
	virtual void OnCCEvent(CComEvent*){};

	void SetOwner(MyObject* owner){owner_ = owner;};
	MyObject* GetOwner() const {return owner_;};

protected:
	//�����¼�,���¼���������Ӧ
	void RaiseEvent(CComEvent*);
	void RaiseEvent(uint32_t idEvent, int64_t nParam1 = 0, void* pParam2 = 0);
	//Ͷ���¼�,���¼������Ӻ���Ӧ
	void PostEvent(std::auto_ptr<CComEvent> &evnt);
	void PostEvent(uint32_t idEvent, int64_t nParam1 = 0, void* pParam2 = 0);
private:
	void SetID(uint32_t id){id_ = id;}
	friend class MyComponentFactory;
private:
	uint32_t id_;                 //ID
	MyObject * owner_;          //�����������
};



#endif



//
#ifndef _MY_OBJECT_H_
#define _MY_OBJECT_H_

#include "common.h"
#include <set>
#include <list>
#include <map>
#include <memory>
#include <string.h>
#include "Size.h"
#include "Point.h"

class MyComponent;
class CComEvent;
class MyObjMgr;

enum ObjectType
{
	EOT_NONE = 0,				//
	EOT_PLAYER,					//���
	EOT_BULLET,					//�ӵ�
	EOT_FISH,					//��
};

enum ObjState
{
	EOS_LIVE = 0,					//���
	EOS_HIT,						//�ܻ�
	EOS_DEAD,						//����
	EOS_DESTORY,					//�ݻ�
	EOS_LIGHTING,					//������
};

enum MyEvent
{
	EME_STATE_CHANGED = 0,		//״̬�仯
	EME_QUERY_SPEED_MUL,		//��ѯ�ٶȱ���     //�ٶȱ��ʣ�ǰ�˼��٣�
	EME_QUERY_ADDMUL,			//��ѯ�������ӵı���
};

class MyObject
{
public:
	MyObject();
	virtual ~MyObject();

public:
	//���úͻ�ȡId
	uint32_t GetId()const{return id_;};
	void SetId(uint32_t newId){id_ = newId;};

	int GetObjType()const{return objType_;}
	void SetObjType(int objType){objType_ = objType;}

	//��Ӧʱ������
	virtual void OnUpdate(int msElapsed);

	void SetMgr(MyObjMgr* mgr){m_Mgr = mgr;}
	MyObjMgr* GetMgr(){return m_Mgr;}

	MyPoint GetPosition();

	float GetDirection();
	
	long long  GetScore(){return m_Score;}
	void SetScore(long long  sc){m_Score = sc;}
	void AddScore(long long  sc){m_Score += sc;}

	float	GetProbability(){return m_fProbability;}
	void SetProbability(float f){m_fProbability = f;}

	uint32_t GetCreateTick(){return m_dwCreateTick;}
	void SetCreateTick(uint32_t tk){m_dwCreateTick = tk;}

	bool InSideScreen();
	bool OutLeftSideScreen();
	bool OutUpSideScreen();
	bool OutDownSideScreen();

protected:
	MyObjMgr* m_Mgr;            //������ָ��
	uint32_t id_;
	int objType_;               //��������

	friend class ClientObjectFactory;

protected:
	typedef std::map< const uint32_t, MyComponent* >	Component_Table_t;     //���ӿؼ��б�
	typedef std::list< CComEvent* > CCEvent_Queue_t;          //�¼��б�

	Component_Table_t components_;          //���ӿؼ��б�
	CCEvent_Queue_t ccevent_queue_;         //�¼��б�
	
	long long	m_Score;       //��Ǯ

	float		m_fProbability;         //����(���ʱ��Ϊ����׽����)

	uint32_t	m_dwCreateTick;             //�������ӣ�

	int		m_nState;                       //״̬

public:
	void ProcessCCEvent(CComEvent*);//��ʱ������¼�
	void ProcessCCEvent(uint32_t idEvent, int64_t nParam1 = 0, void* pParam2 = 0);

	void PushCCEvent(std::auto_ptr<CComEvent>& evnt);//�ӳٴ�����¼�
	void PushCCEvent(uint32_t idEvent, int64_t nParam1 = 0, void* pParam2 = 0);

	MyComponent* GetComponent(const uint32_t& familyID);
	void SetComponent( MyComponent* newComponent);

	bool DelComponent(const uint32_t& familyID);//ɾ��ָ�����������ҵ����ɹ�ɾ���򷵻أ�����壬�Ҳ����򷵻أ�����
	void ClearComponent();

	void SetState(int st, MyObject* pobj = NULL);
	int GetState();
    //����
	void SetTypeID(int n){m_nTypeID = n;}
	int GetTypeID(){return m_nTypeID;}

protected:
	int			m_nTypeID;          //����

};



#endif



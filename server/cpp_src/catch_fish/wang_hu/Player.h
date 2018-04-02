//
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Point.h"
#include "MyObject.h"

extern "C" {
#include "lua.hpp"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lua_tinker_ex.h"

class CPlayer : public MyObject
{
public:
	CPlayer();
	virtual ~CPlayer();

	void ClearSet(int chairid);
	//���ô���
	void SetCannonType(int n){m_nCannonType = n;}
	int GetCannonType(){return m_nCannonType;}
	//��������
	void AddWastage(long long  s){m_Wastage += s;}
	long long  GetWastage(){return m_Wastage;}

	void SetMultiply(int n){m_nMultiply = n;}
    //��ȡ�ӵ�����
	int GetMultiply(){return m_nMultiply;}
	//��������
	void SetCannonPos(MyPoint& pt){m_CannonPos = pt;}
	const MyPoint& GetCannonPos(){return m_CannonPos;}
	//���һ�ο�������
	void SetLastFireTick(uint32_t dw){m_dwLastFireTick = dw;}
	uint32_t GetLastFireTick(){return m_dwLastFireTick;}
	//������
	void SetLockFishID(uint32_t id);
	uint32_t GetLockFishID(){return m_dwLockFishID;}

	bool HasLocked(uint32_t id);
	void ClearLockedBuffer(){LockBuffer.clear();}

	bool bLocking(){return m_bLocking;}
	void SetLocking(bool b){m_bLocking = b;}
	//�����ӵ�
	void ADDBulletCount(int n){BulletCount += n;}
	void ClearBulletCount(){BulletCount = 0;}
	int GetBulletCount(){return BulletCount;}

	void SetFired();

	int	GetCannonSetType(){return m_nCannonSetType;}
	void SetCannonSetType(int n){m_nCannonSetType = n;}
	
	void CacluteCannonPos(unsigned int wChairID);

	bool	CanFire(){return m_bCanFire;}
	void	SetCanFire(bool b = true){m_bCanFire = b;}

	void	FromLua(lua_tinker::table player);

public:
	void	SetGuidGateID(int guid, int gate_id);
	int		GetGuid() { return guid_; }
	int		GetGateID() { return gate_id_; }

	void	SetChairID(int chair_id) { chair_id_ = chair_id; }
	int		GetChairID() { return chair_id_; }

	void	SetNickname(const std::string& nickname) { nickname_ = nickname; }
	const std::string& GetNickname() { return nickname_; }

protected:
	long long 					m_Wastage;		//���
	int								m_nCannonType;  //��������
	int								m_nMultiply;    //�ӵ����ͣ�   �ӵ������Ƿ���� ����
	MyPoint						m_CannonPos;    //��������

	uint32_t					m_dwLastFireTick;   //���һ�ο�������

	uint32_t					m_dwLockFishID;     //������ID
	bool							m_bLocking;         //�Ƿ�����
	std::list<uint32_t>	LockBuffer;         //buff�б�

	int								BulletCount;         //�ӵ�����
	bool							bFired;				//�Ƿ񿪻���
	bool							m_bCanFire;			//	�Ƿ���Կ���
	int								m_nCannonSetType;    //���ڼ�����

	int								m_Level;

	// ������Ϣ���
	int								guid_;
	int								gate_id_;
	int								chair_id_;
	std::string					nickname_;
};

#endif


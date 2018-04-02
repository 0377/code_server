#pragma once
#include "common.h"
#include "Bullet.h"
#include "Fish.h"
#include "MoveCompent.h"
#include "EffectManager.h"
#include "Effect.h"
#include "MyObjectManager.h"
#include "config.pb.h"
#include "Player.h"
#include "public_fishing.pb.h"
#include "lua_tinker_ex.h"
#include <list>
#include <iostream>   
#define INVALID_CHAIR				0xFFFF								//��Ч����
#define GAME_STATUS_FREE			0									//����״̬
#define GAME_STATUS_PLAY			100									//��Ϸ״̬
#define GAME_STATUS_WAIT			200									//�ȴ�״̬

using namespace std;

class CMyEvent;

//ˢ����Ⱥ
struct RefershTroop
{
	bool	bSendDes;               //��������
	bool	bSendTroop;             //������Ⱥ
	float	fBeginTime;             //��ʼʱ��
};

struct SC_stSendFish
{
	SC_stSendFish(){
		Clear();
	}

	~SC_stSendFish(){
		Clear();
	}

	void Clear(){
		fish_id = 0;
		type_id = 0;
		path_id = 0;
		create_tick = 0;
		offest_x = 0;
		offest_y = 0;
		dir = 0;
		delay = 0;
		server_tick = 0;
		fish_speed = 0;
		fis_type = 0;
		troop = 0;
		refersh_id = 0;
	}

	SC_stSendFish& operator= (SC_stSendFish& other){
		this->fish_id = other.fish_id;
		this->type_id = other.type_id;
		this->path_id = other.path_id;
		this->create_tick = other.create_tick;
		this->offest_x = other.offest_x;
		this->offest_y = other.offest_y;
		this->dir = other.dir;
		this->delay = other.delay;
		this->server_tick = other.server_tick;
		this->fish_speed = other.fish_speed;
		this->fis_type = other.fis_type;
		this->troop = other.troop;
		this->refersh_id = other.refersh_id;
		return *this;
	}

	int	    fish_id;		 //��ID
	int	    type_id;         //���ͣ�
	int	    path_id;         //·��ID
	int	    create_tick;     //����ʱ��
	float	offest_x;        //X����
	float	offest_y;        //Y����
	float	dir;             //����
	float	delay;           //��ʱ
	int	    server_tick;     //ϵͳʱ��
	float	fish_speed;      //���ٶ�
	int	    fis_type;        //�����ͣ�
	int	    troop;           //�Ƿ���Ⱥ
	int	    refersh_id;      //��ȡˢ��ID��
};

//��Ϸ������
class catch_fish_logic
{
public:
	catch_fish_logic();
	~catch_fish_logic();

public:
	void		OnTest();
	void		ResetTable();
	bool		Initialization(lua_tinker::table luaTable);

public:
	bool		OnEventGameStart();
	bool		OnEventGameConclude(lua_tinker::table player, BYTE cbReason);
	bool		OnEventSendGameScene(lua_tinker::table player, BYTE cbGameStatus, bool bSendSecret);
	bool		OnReady(uint32_t wChairID);

public:
	bool		OnUserScroeNotify(uint32_t wChairID, int Guid, BYTE cbReason) { return true; }
	bool		OnGameMessage(uint32_t wSubCmdID, void * pData, uint32_t wDataSize, int guid){ return false; }
	void		SetGameBaseScore(int32_t lBaseScore) {}

public:
	bool		OnActionUserSitDown(uint32_t wChairID, lua_tinker::table player);
	bool		OnActionUserStandUp(lua_tinker::table player, int is_offline);
	bool		OnActionUserOnReady(uint32_t wChairID, int Guid, void* pData, uint32_t wDataSize){ return true; }
	bool		OnActionUserOffLine(lua_tinker::table player, int Guid){ return true; }

	void		OnGameUpdate();
	bool		OnTimeSync(lua_tinker::table player, int	client_tick);
	bool		OnChangeCannon(lua_tinker::table player, int add);
	bool		OnFire(lua_tinker::table player, lua_tinker::table msg);

	void		CatchFish(CBullet* pBullet, CFish* pFish, int nCatch, int* nCatched);
	void		SendCatchFish(CBullet* pBullet, CFish*pFish, long long  score);
	void		DistrubFish(float fdt);
	void		ResetSceneDistrub();

	void		SendFish(CFish* pFish, uint32_t wChairID = INVALID_CHAIR);
	void		SendFishList(uint32_t wChairID = INVALID_CHAIR);
	void		SendBullet(CBullet* pBullet, bool bNew = false);
	void		SendSceneInfo(int TargetGuid);
	void		SendPlayerInfo(int TargetGuid = 0);
	void		SendCannonSet(int wChairID);
	void		SendGameConfig(int TargetGuid);
	void		SendSystemMsg(int TargetGuid, int type, const std::string& msg);
	void		ReturnBulletScore(int guid);

	void		SendAllowFire(int GuID);

	void		OnProduceFish(CMyEvent* pEvent);
	void		OnAddBuffer(CMyEvent* pEvent);
	void		OnAdwardEvent(CMyEvent* pEvent);
	void		OnCannonSetChange(CMyEvent* pEvent);
	void		OnCatchFishBroadCast(CMyEvent* pEvent);
	void		OnFirstFire(CMyEvent* pEvent);
	void		OnMulChange(CMyEvent* pEvent);

	void		LockFish(uint32_t wChairID);
	bool		OnLockFish(lua_tinker::table player, int isLock);
	bool		OnLockSpecFish(lua_tinker::table player, int fishID);
	bool		OnNetCast(lua_tinker::table player, int bullet_id, int data, int fish_id);
	bool		OnChangeCannonSet(lua_tinker::table player, int add);
	bool		HasRealPlayer();
	void		AddBuffer(int btp, float parm, float ft);

	int				CountPlayer();

public:
	static void	LoadConfig();

	bool			OnTreasureEND(lua_tinker::table player, int64_t score);
	int				GetFirstPlayerGuID();

	void			SetGuidAndGateID(int chair_id, int guid, int gate_id);
	int				GetTableID();

	template<typename T>
	void SendTo_pb(int guid, T& pb)
	{
		if (guid <= 0){
			BroadCast_pb(pb);
			return;
		}
		std::unordered_map<int, CPlayer>::iterator iter = m_GuidPlayers.find(guid);
		if (iter == m_GuidPlayers.end())
		{
			return;
		}

		int gate_id = iter->second.GetGateID();
		if (gate_id == 0)
		{
			return;
		}
		m_lua_bridge_str = pb.SerializeAsString();
#ifdef GS_DEBUG
		lua_tinker::call<void, int, int, unsigned short, long long>(
		 catchfish_dll::sLuaState, "send2client_fish", iter->first, gate_id, T::ID, (long long)(&m_lua_bridge_str));
#else
		lua_tinker::call<void, int, int, unsigned short, std::string>(
			catchfish_dll::sLuaState, "send2client_fish", guid, gate_id, T::ID, m_lua_bridge_str);
#endif
		//lua_tinker::call<void, int, int, unsigned short, std::string>(
		//	catchfish_dll::sLuaState, "sendfishmsg", iter->first, gate_id, T::ID, m_lua_bridge_str);
	}
	template<typename T>
	void BroadCast_pb(T& pb)
	{
		m_lua_bridge_str = pb.SerializeAsString();
		std::unordered_map<int, CPlayer>::iterator iter = m_GuidPlayers.begin();
		for (; iter != m_GuidPlayers.end(); iter++)
		{
			if (iter->first != 0)
			{
				int gate_id = iter->second.GetGateID();
				if (gate_id == 0)
				{
					continue;
				}
#ifdef GS_DEBUG
				lua_tinker::call<void, int, int, unsigned short, long long>(
				 catchfish_dll::sLuaState, "send2client_fish", iter->first, gate_id, T::ID, (long long)(&m_lua_bridge_str));
#else
				lua_tinker::call<void, int, int, unsigned short, std::string>(
					catchfish_dll::sLuaState, "send2client_fish", iter->first, gate_id, T::ID, m_lua_bridge_str);
#endif
				//lua_tinker::call<void, int, int, unsigned short, std::string>(
				//	catchfish_dll::sLuaState, "sendfishmsg", iter->first, gate_id, T::ID, m_lua_bridge_str);

			}
		}
	}

protected:
	bool			IsUserInTable(int Guid);
	bool			IsUserSitDownChair(int ChairID);
	void			delete_invalid_player();

protected:
	uint32_t											m_dwLastTick;						//��һ��ɨ��ʱ��
	uint32_t											m_dwLastSave;						//��һ��ɨ�豣��ʱ��
	float													m_fSceneTime;						//����ʱ��
	int														m_nCurScene;						//��ǰ����
	MyObjMgr											m_FishManager;					//�������
	MyObjMgr											m_BulletManager;					//�ӵ�������
	bool													m_bAllowFire;						//���Կ���
	float													m_fPauseTime;						//��ͣʱ��

	int														m_nSpecialCount;       //����������
	std::list<uint32_t>							m_CanLockList;         //�������б�
	std::vector<float>							m_vDistrubFishTime;    //����ʱ��
	std::vector<RefershTroop>				m_vDistrubTroop;       //������Ⱥ
	std::unordered_map<int, CPlayer>	m_GuidPlayers;
	std::unordered_map<int, CPlayer*>	m_ChairPlayers;
	int														m_nFishCount;         //������

	std::unordered_map<int, int64_t>		m_UserWinScore;// Chair�û�������Ӯ

	int														m_TableID;
	int														m_RoomID;
	std::string												m_lua_bridge_str;
};

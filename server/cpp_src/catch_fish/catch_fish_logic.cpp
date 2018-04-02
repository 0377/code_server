//
#include "common.h"
#include "catch_fish_logic.h"
#include "GameConfig.h"
#include "MathAide.h"
#include "GameConfig.h"
#include "CommonLogic.h"
#include "PathManager.h"
#include "EventMgr.h"
#include <math.h>
#include <MMSystem.h>
#include "IDGenerator.h"
#include "BufferManager.h"
#include "MyComponentFactory.h"
#include "base_game_log.h"
#include "Earnings.h"
#include "catchfish.h"
#include <codecvt>
#include <iostream>



//���캯��
catch_fish_logic::catch_fish_logic(){
	m_nFishCount = 0;
	ResetTable();
}

//��������
catch_fish_logic::~catch_fish_logic(void){
	ResetTable();
}

void catch_fish_logic::OnTest(){
	vector<int>	counts;
	counts.resize(10, 0);
	for (int i = 1; i < 1000000; i++){
		counts[rand() % 10]++;
	}

	for (int i = 0; i < 10; i++){
		std::cout << i << " " << counts[i] << std::endl;
	}
}

//��ʼ��
bool  catch_fish_logic::Initialization(lua_tinker::table luaTable){
	m_TableID		= luaTable.get<int>("table_id_");
	m_RoomID	= luaTable.get<lua_tinker::table>("room_").get<int>("id");

	Bind_Event_Handler("ProduceFish", catch_fish_logic, OnProduceFish);
	Bind_Event_Handler("CannonSetChanaged", catch_fish_logic, OnCannonSetChange);
	Bind_Event_Handler("AddBuffer", catch_fish_logic, OnAddBuffer);
	Bind_Event_Handler("CatchFishBroadCast", catch_fish_logic, OnCatchFishBroadCast);
	Bind_Event_Handler("FirstFire", catch_fish_logic, OnFirstFire);
	Bind_Event_Handler("AdwardEvent", catch_fish_logic, OnAdwardEvent);
	Bind_Event_Handler("FishMulChange", catch_fish_logic, OnMulChange);

	m_UserWinScore.clear();

	return true;
}

void catch_fish_logic::LoadConfig(){
	int game_id = lua_tinker::get<int>(catchfish_dll::sLuaState, "def_game_id");
	std::string path = (boost::format("../data/catch_fish/%1%/") % game_id).str();
    std::cout<< "��ʼ��������..." << std::endl;
	uint32_t dwStartTick = ::GetTickCount();

	CGameConfig::GetInstance()->LoadSystemConfig(path + "System.xml");
	CGameConfig::GetInstance()->LoadBoundBox(path + "BoundingBox.xml");
	CGameConfig::GetInstance()->LoadFish(path + "Fish.xml");
	PathManager::GetInstance()->LoadNormalPath(path + "path.xml");
	PathManager::GetInstance()->LoadTroop(path + "TroopSet.xml");
	CGameConfig::GetInstance()->LoadCannonSet(path + "CannonSet.xml");
	CGameConfig::GetInstance()->LoadBulletSet(path + "BulletSet.xml");
	CGameConfig::GetInstance()->LoadScenes(path + "Scene.xml");
	CGameConfig::GetInstance()->LoadSpecialFish(path + "Special.xml");

    dwStartTick = ::GetTickCount() - dwStartTick;
	std::cout << "������� ���� �ܼƺ�ʱ" << dwStartTick / 1000.f << "��" << std::endl;
}

//��������
void catch_fish_logic::ResetTable(){
	m_FishManager.Clear();
	m_BulletManager.Clear();
	m_fPauseTime = 0.0f;
	m_nSpecialCount = 0;
	m_nFishCount = 0;
	m_ChairPlayers.clear();
	m_GuidPlayers.clear();
}

//�û�����
bool catch_fish_logic::OnActionUserSitDown(unsigned int wChairID, lua_tinker::table player)
{
	delete_invalid_player();
	std::cout << "catch_fish_logic::OnActionUserSitDown:" << wChairID << std::endl;
	int		Guid = player.get<int>("guid");
	m_GuidPlayers[Guid].ClearSet(wChairID);
	m_GuidPlayers[Guid].FromLua(player);
	m_ChairPlayers[wChairID] = &m_GuidPlayers[Guid];

	m_UserWinScore[wChairID] = 0;
        
	//��ȡBUFF������
	BufferMgr* pBMgr = (BufferMgr*)m_GuidPlayers[Guid].GetComponent(ECF_BUFFERMGR);
	if (pBMgr == NULL){
		pBMgr = (BufferMgr*)CreateComponent(EBCT_BUFFERMGR);
		if (pBMgr != NULL){
			m_GuidPlayers[Guid].SetComponent(pBMgr);
		}
	}

	if (pBMgr == NULL){
		return false;
	}

	pBMgr->Clear();
	return true;
}

//�û�����
bool  catch_fish_logic::OnActionUserStandUp(lua_tinker::table player, int is_offline){
	delete_invalid_player();
	int Guid = player.get<int>("guid");
	uint32_t wChairID = player.get<uint32_t>("chair_id");
	
	// �����û���Ϣ�����ݿ�
	ReturnBulletScore(Guid);

	lua_tinker::call<void, int>(catchfish_dll::sLuaState, "player_exit_fish", Guid);

	m_UserWinScore[wChairID] = 0;
	
	auto iterChair = m_ChairPlayers.find(wChairID);
	if (iterChair != m_ChairPlayers.end()){
		m_ChairPlayers.erase(iterChair);
	}

	auto iterGuid = m_GuidPlayers.find(Guid);
	if (iterGuid != m_GuidPlayers.end()){
		m_GuidPlayers.erase(iterGuid);
	}

	if (m_GuidPlayers.empty()){
		ResetTable();
	}

	return true;
}

//��Ϸ��ʼ
bool  catch_fish_logic::OnEventGameStart()
{
	delete_invalid_player();
	ResetTable();

	m_dwLastTick = timeGetTime();
	m_dwLastSave = timeGetTime();
	m_nCurScene = CGameConfig::GetInstance()->SceneSets.begin()->first;
	m_fSceneTime = 0.0f;
	m_fPauseTime = 0.0f;
	m_bAllowFire = false;

	ResetSceneDistrub();

	//��ʼ���������
	RandSeed(timeGetTime());
	srand(timeGetTime());

    //m_Timer.expires_from_now(boost::posix_time::millisec(1000 / 30));
    //m_Timer.async_wait(boost::bind(&catch_fish_logic::OnGameUpdate, this));
	return true;
}
//���ó���
void catch_fish_logic::ResetSceneDistrub()
{
	//���ø�����Ⱥˢ��ʱ��
	int sn = CGameConfig::GetInstance()->SceneSets[m_nCurScene].DistrubList.size();
	m_vDistrubFishTime.resize(sn);
	for (int i = 0; i < sn; ++i)
	{
		m_vDistrubFishTime[i] = 0;
	}

	//������Ⱥ
	//��ȡ����ˢ����ʱ������
	sn = CGameConfig::GetInstance()->SceneSets[m_nCurScene].TroopList.size();
	m_vDistrubTroop.resize(sn);//����ˢ������Ϣ��С
	//��ʼ��ˢ����Ϣ
	for (int i = 0; i < sn; ++i)
	{
		m_vDistrubTroop[i].bSendDes = false;
		m_vDistrubTroop[i].bSendTroop = false;
		m_vDistrubTroop[i].fBeginTime = 0.0f;
	}
}

//����ԭ��
#define GER_NORMAL					0x00								//�������
#define GER_DISMISS					0x01								//��Ϸ��ɢ
#define GER_USER_LEAVE				0x02								//�û��뿪
#define GER_NETWORK_ERROR			0x03								//�������

//��Ϸ����
bool  catch_fish_logic::OnEventGameConclude(lua_tinker::table player, BYTE cbReason)
{
	delete_invalid_player();
	int Guid = player.get<int>("guid");
	uint32_t wChairID = player.get<uint32_t>("chair_id");
	switch (cbReason)
	{
	case GER_NORMAL:
	case GER_USER_LEAVE:
	case GER_NETWORK_ERROR:
	{
		//������ң������˳�
		//ReturnBulletScore(Guid);
		m_GuidPlayers[Guid].ClearSet(wChairID);
		m_GuidPlayers[Guid].SetGuidGateID(0, 0);

		return true;
	}
	case GER_DISMISS:
	{   //��������˳� ���������Ϣ
		for (auto& iter : m_ChairPlayers){
			ReturnBulletScore(Guid);
			iter.second->ClearSet(iter.first - 1);
			iter.second->SetGuidGateID(0, 0);
		}
		return true;
	}
	}
	return false;
}

//���ͳ���
bool  catch_fish_logic::OnEventSendGameScene(lua_tinker::table player, BYTE cbGameStatus, bool bSendSecret)
{
	delete_invalid_player();
	uint32_t wChairID = player.get<uint32_t>("chair_id");
	int GuID = player.get<int>("guid");
    if (GuID == 0){
        return false;
    }

	switch (cbGameStatus)
	{
	case GAME_STATUS_FREE:
	case GAME_STATUS_PLAY:
	{
		SendGameConfig(GuID);
		SendPlayerInfo(0);
		SendAllowFire(0);
		for (auto& iter : m_ChairPlayers){
			SendCannonSet(iter.first);
		}
		SendSceneInfo(GuID);
		
		
		char szInfo[1024] = {0};
		sprintf_s(szInfo, "��ǰ�������Ϸ������ҵĶһ�����Ϊ%d��Ϸ�Ҷһ�%d���", 
			CGameConfig::GetInstance()->nChangeRatioUserScore, CGameConfig::GetInstance()->nChangeRatioFishScore);

		SendSystemMsg(GuID, SMT_CHAT, szInfo);

		return true;
	}
	}
	return false;
}

bool catch_fish_logic::OnReady(uint32_t wChairID){
	delete_invalid_player();
	SendFishList(wChairID);
	return true;
}

//������Ϸϵͳ����
void catch_fish_logic::SendGameConfig(int guid)
{
	SC_GameConfig css;
	css.set_server_id(1);
	css.set_change_ratio_fish_score(CGameConfig::GetInstance()->nChangeRatioFishScore);
	css.set_change_ratio_user_score(CGameConfig::GetInstance()->nChangeRatioUserScore);
	css.set_exchange_once(CGameConfig::GetInstance()->nExchangeOnce);
	css.set_fire_interval(CGameConfig::GetInstance()->nFireInterval);
	css.set_max_interval(CGameConfig::GetInstance()->nMaxInterval);
	css.set_min_interval(CGameConfig::GetInstance()->nMinInterval);
	css.set_show_gold_min_mul(CGameConfig::GetInstance()->nShowGoldMinMul);
	css.set_max_bullet_count(CGameConfig::GetInstance()->nMaxBullet);
	css.set_max_cannon(CGameConfig::GetInstance()->m_MaxCannon);

	SendTo_pb(guid, css);

	int i = 0;
	SC_BulletSet_List tbBulletList;
	for (auto& iter : CGameConfig::GetInstance()->BulletVector){
		SC_BulletSet* tb = tbBulletList.add_pb_bullets();
		tb->set_first(i == 0 ? 1 : 0);
		tb->set_bullet_size(iter.nBulletSize);
		tb->set_cannon_type(iter.nCannonType);
		tb->set_catch_radio(iter.nCatchRadio);
		tb->set_max_catch(iter.nMaxCatch);
		tb->set_mulriple(iter.nMulriple);
		tb->set_speed(iter.nSpeed);
		//if (i == 0)
		//{
		//	printf("mulriple %d\n", tb->mulriple());
		//}
	}
	
	SendTo_pb(guid, tbBulletList);
}

void catch_fish_logic::SendSystemMsg(int guid,int type, const std::string& msg){
	//std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> mb_conv_ucs;
	//std::wstring test = mb_conv_ucs.from_bytes(msg);

	//std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	//std::string narrowStr = conv.to_bytes(test);

	//lua_tinker::table table(catchfish_dll::sLuaState);
	//table.set("wType", SMT_CHAT);
	//table.set("szString", narrowStr.c_str());
	//SendTo(guid, "SC_SystemMessage", table);
}

//���������Ϣ
void catch_fish_logic::SendPlayerInfo(int TargetGuid){
	for (auto iter:m_ChairPlayers){
		SC_UserInfo tinfo;
		tinfo.set_chair_id(iter.first);
		tinfo.set_score(iter.second->GetScore());
		tinfo.set_cannon_mul(iter.second->GetMultiply());
		tinfo.set_cannon_type(iter.second->GetCannonType());
		tinfo.set_wastage(iter.second->GetWastage());
		SendTo_pb(TargetGuid, tinfo);
	}
}


int    catch_fish_logic::GetFirstPlayerGuID(){
	if (m_GuidPlayers.empty()){
		return 0;
	}

	return m_GuidPlayers.begin()->second.GetGuid();
}
//���ͳ�����Ϣ
void catch_fish_logic::SendSceneInfo(int GuID)
{
	uint32_t wChairID = m_GuidPlayers[GuID].GetChairID();
    {
		SC_SwitchScene tinfo;
		tinfo.set_switching(0);
		tinfo.set_nst(m_nCurScene);
		SendTo_pb(GuID, tinfo);
    }

	m_BulletManager.Lock();
	for (auto iter = m_BulletManager.Begin(); iter != m_BulletManager.End(); ++iter){
		SendBullet((CBullet*)iter->second);
	}
	m_BulletManager.Unlock();

	m_FishManager.Lock();
	for (auto iter = m_FishManager.Begin(); iter != m_FishManager.End();++iter){
		SendFish((CFish*)iter->second, wChairID);
	}
	m_FishManager.Unlock();
}

//�����Ƿ�������
void catch_fish_logic::SendAllowFire(int GuID)
{
	SC_AllowFire tinfo;
	tinfo.set_allow_fire(m_bAllowFire ? 1 : 0);
	SendTo_pb(GuID, tinfo);
}

void catch_fish_logic::delete_invalid_player()
{
	while (true)
	{
		bool find_nil = false;
		for (auto& iter : m_ChairPlayers){
			if (iter.second == nullptr){
				char bbb[128] = { 0 };
				sprintf(bbb, "OnGameUpdate  nil player, chair id %d", iter.first);
				lua_tinker::call<void>(catchfish_dll::sLuaState, "log_error", bbb);
				m_ChairPlayers.erase(iter.first);
				find_nil = true;
				break;
			}
		}
		if (!find_nil)
		{
			break;
		}
	}
}
//��Ϸ״̬����
void catch_fish_logic::OnGameUpdate()
{
	delete_invalid_player();
	uint32_t NowTime = timeGetTime();
	int ndt = NowTime - m_dwLastTick;
	float fdt = ndt / 1000.0f;

	bool hasR = HasRealPlayer();

	
	
	for (auto& iter:m_ChairPlayers){
		if (iter.second->GetGuid() == 0){
			continue;
		}

		//��������¼�
		iter.second->OnUpdate(ndt);
		//����Ҵ����������������
		if (iter.second->bLocking()){
			//�����������ʱ�ж���ID���Ƿ����
			if (iter.second->GetLockFishID() == 0){
				//ID= 0 ��������
				LockFish(iter.second->GetChairID());
				if (iter.second->GetLockFishID() == 0){
					iter.second->SetLocking(false);
				}
			}else{
				CFish* pFish = (CFish*)m_FishManager.Find(iter.second->GetLockFishID());
				if (pFish == NULL || !pFish->InSideScreen()){//���㲻���ڻ����Ѿ�����Ļ����������
					LockFish(iter.second->GetChairID());
					if (iter.second->GetLockFishID() == 0){
						iter.second->SetLocking(false);
					}
				}
			}
		}
	}
	//����������б�
	m_CanLockList.clear();
	//����������
	m_nFishCount = 0;

	//�Ƴ�����
	std::list<uint32_t> rmList;
	//��������0
	m_nSpecialCount = 0;

	m_FishManager.Lock();

	for (obj_table_iter ifs = m_FishManager.Begin(); ifs != m_FishManager.End(); ++ifs){
		CFish* pFish = (CFish*)ifs->second;
		//�������¼�
		pFish->OnUpdate(ndt);
		MoveCompent* pMove = (MoveCompent*)pFish->GetComponent(ECF_MOVE);
		if (pMove == NULL || pMove->IsEndPath()){//�ƶ����Ϊ�ջ� �Ѿ��ƶ�������
			if (pMove != NULL && pFish->InSideScreen()){//�ƶ���������ƶ���������������Ļ�� ��Ϊ��ָ�������ƶ�
				MoveCompent* pMove2 = (MoveCompent*)CreateComponent(EMCT_DIRECTION);
				if (pMove2 != NULL){
					pMove2->SetSpeed(pMove->GetSpeed());
					pMove2->SetDirection(pMove->GetDirection());
					pMove2->SetPosition(pMove->GetPostion());
					pMove2->InitMove();
					//SetComponent��������������
					pFish->SetComponent(pMove2);
				}
			}else{//������ӵ��Ƴ��б�
				rmList.push_back(pFish->GetId());
			}
		}else if (pFish->GetFishType() != ESFT_NORMAL){//Ǯ���Ͳ�������ͨ�� ������+1
			++m_nSpecialCount;
		}

		if (hasR && pFish->InSideScreen()){
			//������Ļ��
			if (pFish->GetLockLevel() > 0){//�����ȼ�����0 ����������б�
				m_CanLockList.push_back(pFish->GetId());
			}

			//������+1
			++m_nFishCount;
		}
	}

	m_FishManager.Unlock();

	//�����
	
	for (std::list<uint32_t>::iterator it = rmList.begin(); it != rmList.end(); it++){
		lua_tinker::call<void>(catchfish_dll::sLuaState, "on_fish_removed",m_RoomID,m_TableID, *it);
		m_FishManager.Remove(*it);
	}

	rmList.clear();

	m_BulletManager.Lock();
	for (obj_table_iter ibu = m_BulletManager.Begin(); ibu != m_BulletManager.End();++ibu){
		CBullet* pBullet = (CBullet*)ibu->second;
		//�����ӵ��¼�
		pBullet->OnUpdate(ndt);
		//��ȡ�ƶ����
		MoveCompent* pMove = (MoveCompent*)pBullet->GetComponent(ECF_MOVE);
		if (pMove == NULL || pMove->IsEndPath()){//��û���ƶ�������Ѿ��ƶ����յ� ���뵽����б�
			rmList.push_back(pBullet->GetId());
		}
		//����Ҫֱ���жϣ�
		else if (CGameConfig::GetInstance()->bImitationRealPlayer && !hasR){//�������ģ�� �� ����ң�
			int GuID = GetFirstPlayerGuID();
			for (auto ifs = m_FishManager.Begin(); ifs != m_FishManager.End();++ifs){
				CFish* pFish = (CFish*)ifs->second;
				//ֻҪ��û�� �ж� �Ƿ������
				if (pFish->GetState() < EOS_DEAD && pBullet->HitTest(pFish)){
					//��������ӵ�
                    if (GuID != 0){
						SC_KillBullet tinfo;
						tinfo.set_chair_id(pBullet->GetChairID());
						tinfo.set_bullet_id(pBullet->GetId());
						SendTo_pb(0, tinfo);
                    }
					//ץ����   //ץס�� Remove �����ƻ�ifs��
					CatchFish(pBullet, pFish, 1, 0);
					//�ӵ���������б�
					rmList.push_back(pBullet->GetId());
					break;
				}
			}
		}
	}

	m_BulletManager.Unlock();

	for (auto it = rmList.begin(); it != rmList.end();++it){
		m_BulletManager.Remove(*it);
	}
	rmList.clear();

	uint32_t tEvent = timeGetTime();
	CEventMgr::GetInstance()->Update(ndt);
	tEvent = timeGetTime() - tEvent;

	//�����������ˢ����
	DistrubFish(fdt);

	m_dwLastTick = NowTime;
	if (NowTime - m_dwLastSave > 10* 1000)
	{
		m_dwLastSave = NowTime;
		for (auto iter = m_GuidPlayers.begin(); iter != m_GuidPlayers.end(); iter++)
		{
			if (iter->first&&m_UserWinScore[iter->second.GetChairID()]!=0)
			{
				// �����û���Ϣ�����ݿ�
				ReturnBulletScore(iter->first);
				m_UserWinScore[iter->second.GetChairID()] = 0;
			}
		}
	}

}
//�ж��Ƿ��������
bool catch_fish_logic::HasRealPlayer(){
	return m_ChairPlayers.size() > 0;
}
//ץ����
void catch_fish_logic::CatchFish(CBullet* pBullet, CFish* pFish, int nCatch, int* nCatched)
{
	delete_invalid_player();
	//��ȡ�ӵ� �������͵ĸ���ֵ
	float pbb = pBullet->GetProbilitySet(pFish->GetTypeID()) / MAX_PROBABILITY;
	//��ȡ�㱻ץ������ֵ
	float pbf = pFish->GetProbability() / nCatch;
	//���ñ���
	float fPB = 1.0f;

	//��ȡ��׿����ֵ
	fPB = CGameConfig::GetInstance()->fAndroidProbMul;

	std::list<MyObject*> list;      //��ű���׽�� ����������������

	int64_t lScore = 0;           //��ֵ����
	auto chair_id = pBullet->GetChairID();  //��ȡ�ӵ��������

	static std::vector<int>	counts(MAX_PROBABILITY, 0);
	static int	randCount = 0;

	//�ж��Ƿ�ץ�����ӵ�ץ������ĸ���*�����㱻ץ�ĸ���*���� * �����Ʊ��ʣ�
	float probStorageControl = Earnings::getInstance().getProbabilityRatio((double)pFish->GetProbability(), m_ChairPlayers[chair_id]->GetGuid());
	int	 realProbV = (pbb * pbf * fPB * probStorageControl) ;

	int  randV = RandInt(0, MAX_PROBABILITY);
	bool bCatch = randV < realProbV;
	if (!bCatch){
		return;
	}

	//ץ����ִ���㱻ץЧ��
	lScore = CommonLogic::GetFishEffect(pBullet, pFish, list, false);

	if (m_ChairPlayers.find(pBullet->GetChairID()) == m_ChairPlayers.end()){
		std::cout << "�����ӵ�������" << std::endl;
		return;
	}


	Earnings::getInstance().onCatchFish(lScore);
	m_UserWinScore[chair_id] += lScore;
	m_ChairPlayers[chair_id]->AddScore(lScore);

	//������ �����ֵ/�ڵ�ֵ ���� �����ڻ��� �� ���ֵ С���������� Ϊ��һ�ȡ˫����BUFF
	if (lScore / pBullet->GetScore() > CGameConfig::GetInstance()->nIonMultiply && 
		RandInt(0, MAX_PROBABILITY) < CGameConfig::GetInstance()->nIonProbability){
		BufferMgr* pBMgr = (BufferMgr*)m_ChairPlayers[pBullet->GetChairID()]->GetComponent(ECF_BUFFERMGR);
		if (pBMgr != NULL && !pBMgr->HasBuffer(EBT_DOUBLE_CANNON)){
			pBMgr->Add(EBT_DOUBLE_CANNON, 0, CGameConfig::GetInstance()->fDoubleTime);
			SendCannonSet(pBullet->GetChairID());
		}
	}

	SendCatchFish(pBullet, pFish, lScore);
	char log_buff[256] = {0};
	sprintf(log_buff, "player %d catch fish %d,score is %d,prob is %f,storage is %ld, scale is %d",
		m_ChairPlayers[chair_id]->GetGuid(), pFish->GetTypeID(), int(lScore), probStorageControl, Earnings::getInstance().getEarnings(), lScore / pBullet->GetScore());
	lua_tinker::call<void>(catchfish_dll::sLuaState, "log_info", log_buff);

	//������������������
	for (std::list<MyObject*>::iterator im = list.begin(); im != list.end();++im){
		CFish* pf = (CFish*)*im;
		for (auto& iter:m_ChairPlayers){
			if (iter.second->GetLockFishID() == pf->GetId()){
				iter.second->SetLockFishID(0);
			}
		}

		if (pf != pFish){
			lua_tinker::call<void>(catchfish_dll::sLuaState, "on_fish_removed", m_RoomID, m_TableID, pf->GetId());
			m_FishManager.Remove(pf);
		}
	}

	lua_tinker::table table(catchfish_dll::sLuaState);
	table.set("table_id", m_TableID);
	table.set("room_id", m_RoomID);
	table.set("fish_id", pFish->GetId());
	table.set("multi", lScore / pBullet->GetScore());
	table.set("score", lScore);
	table.set("player_guid", m_ChairPlayers[chair_id]->GetGuid());
	lua_tinker::call<void>(catchfish_dll::sLuaState, "on_catch_fish",  table);

	//�Ƴ���
	lua_tinker::call<void>(catchfish_dll::sLuaState, "on_fish_removed", m_RoomID, m_TableID, pFish->GetId());
	m_FishManager.Remove(pFish);

	//�ô����� ����ȫΪ�� ����
	if (nCatched != NULL){
		*nCatched = *nCatched + 1;
	}
}
//�����㱻ץ
void catch_fish_logic::SendCatchFish(CBullet* pBullet, CFish*pFish, long long  score)
{
	int GuID = GetFirstPlayerGuID();
	if (GuID <= 0){
        return;
    }

	if (pBullet == NULL || pFish == NULL){
		return;
	}

	SC_KillFish tinfo;
	tinfo.set_chair_id(pBullet->GetChairID());
	tinfo.set_fish_id(pFish->GetId());
	tinfo.set_score(score);
	tinfo.set_bscoe(pBullet->GetScore());
	SendTo_pb(0, tinfo);
}
//�����������BUFF
void catch_fish_logic::AddBuffer(int btp, float parm, float ft)
{
	int GuID = GetFirstPlayerGuID();
	if (GuID > 0){
		SC_AddBuffer tinfo;
		tinfo.set_buffer_type(btp);
		tinfo.set_buffer_param(parm);
		tinfo.set_buffer_time(ft);
		SendTo_pb(0, tinfo);
    }

	m_FishManager.Lock();
	obj_table_iter ifs = m_FishManager.Begin();
	while (ifs != m_FishManager.End()){
		MyObject* pObj = ifs->second;
		BufferMgr* pBM = (BufferMgr*)pObj->GetComponent(ECF_BUFFERMGR);
		if (pBM != NULL){
			pBM->Add(btp, parm, ft);
		}
		++ifs;
	}
	m_FishManager.Unlock();
}
//�������� ������������ ��ˢ��
void catch_fish_logic::DistrubFish(float fdt)
{
	if (m_fPauseTime > 0.0f){
		m_fPauseTime -= fdt;
		return;
	}
	//����ʱ������
	m_fSceneTime += fdt;
	//ʱ����ڳ���׼��ʱ�䣬�Ҳ��ɿ��� INVALID_CHAIRȺ���ɿ������� ���ţ��Ƿ�Ӧ�ó����ڴ˴���Ϊʱ��ص�
	if (m_fSceneTime > SWITCH_SCENE_END && !m_bAllowFire){
		m_bAllowFire = true;
		SendAllowFire(-1);
	}

	//�жϵ�ǰ�����Ƿ����
	if (CGameConfig::GetInstance()->SceneSets.find(m_nCurScene) == CGameConfig::GetInstance()->SceneSets.end()){
		return;
	}

	//����ʱ���Ƿ�С�ڳ�������ʱ��
	if (m_fSceneTime < CGameConfig::GetInstance()->SceneSets[m_nCurScene].fSceneTime){
		int npos = 0;
		//��ȡ��ǰ������ˢ��ʱ���б�
		for (TroopSet &ts:CGameConfig::GetInstance()->SceneSets[m_nCurScene].TroopList){
			//�Ƿ�����Ҵ���
			if (!HasRealPlayer()){
				//������ʱ�䡡�Ƿ�Ϊˢ��ʱ�䡡
				if ((m_fSceneTime >= ts.fBeginTime) && (m_fSceneTime <= ts.fEndTime)){
					//������Ϊˢ�����ʱ��
					m_fSceneTime = ts.fEndTime + fdt;
				}
			}

			//������ʱ�䡡�Ƿ�Ϊˢ��ʱ�䡡
			if ((m_fSceneTime >= ts.fBeginTime) && (m_fSceneTime <= ts.fEndTime)){
				//��ѭ��С��ˢ������Ϣ����
				if (npos < m_vDistrubTroop.size()){
					int tid = ts.nTroopID;
					//�Ƿ������� ���� �������跢�Ͱ�
					if (!m_vDistrubTroop[npos].bSendDes){
						//����������ٶ�BUFF
						AddBuffer(EBT_CHANGESPEED, 5, 60);
						//��ȡˢ����Ⱥ������Ϣ
						Troop* ptp = PathManager::GetInstance()->GetTroop(tid);
						if (ptp != NULL){
							//��ȡ����������
							size_t nCount = ptp->Describe.size();
							//����4����ֻ����4��
							if (nCount > 4) nCount = 4;
							//����ˢ��ʱ�俪ʼʱ�� Ϊ 2��
							m_vDistrubTroop[npos].fBeginTime = nCount * 2.0f;//ÿ�����ַ���2�����ʾʱ��

                            //��������  ���� ��Ϊ����ID
							SC_SendDes tinfo;
							for (int i = 0; i < nCount; ++i){
								tinfo.add_des(ptp->Describe[i]);
							}
							BroadCast_pb(tinfo);
						}
						//����Ϊ�ѷ���
						m_vDistrubTroop[npos].bSendDes = true;
					}else if (!m_vDistrubTroop[npos].bSendTroop && 
						m_fSceneTime > (m_vDistrubTroop[npos].fBeginTime + ts.fBeginTime)){//���û�з��͹���Ⱥ�� ����ʱ�� ���� ˢ��ʱ�����������ʱ��
						m_vDistrubTroop[npos].bSendTroop = true;
						//��ȡˢ����Ⱥ������Ϣ
						Troop* ptp = PathManager::GetInstance()->GetTroop(tid);
						if (ptp == NULL){
							//���Ϊ�գ�����һ����
							m_fSceneTime += CGameConfig::GetInstance()->SceneSets[m_nCurScene].fSceneTime;
						}else{
							int n = 0;
							int ns = ptp->nStep.size();    //��ȡ���� ���岻��
							for (int i = 0; i < ns; ++i){
								//ˢ���ID
								int Fid = -1;
								//��ȡ�ܲ���
								int ncount = ptp->nStep[i];
								for (int j = 0; j < ncount; ++j){
									//n���� ����״��ʱ �˳�ѭ��
									if (n >= ptp->Shape.size()) break;
									//��ȡ��״��
									ShapePoint& tp = ptp->Shape[n++];
									//��Ȩ��
									int WeightCount = 0;
									//��ȡ�������б��Ȩ���б���Сֵ
									int nsz = min(tp.m_lTypeList.size(), tp.m_lWeight.size());
									//���Ϊ0����������
									if (nsz == 0) continue;
									//��ȡ��Ȩ��
									for (int iw = 0; iw < nsz; ++iw){
										WeightCount += tp.m_lWeight[iw];
									}

									for (int ni = 0; ni < tp.m_nCount; ++ni){
										if (Fid == -1 || !tp.m_bSame){
											//�ڼ�����Ŀ��
											int wpos = 0;
											//���Ȩ��
											int nf = RandInt(0, WeightCount);
											//����ƥ���Ȩ��
											while (nf > tp.m_lWeight[wpos]){
												//���ڻ����Ȩ�����ֵ������
												if (wpos >= tp.m_lWeight.size()) break;
												//���ֵ��ȥ��ǰȨ��
												nf -= tp.m_lWeight[wpos];
												//Ŀ���1
												++wpos;
												//��������������б� 
												if (wpos >= nsz){
													wpos = 0;
												}
											}
											//���λ��С�����б� ��ȡ ��ID
											if (wpos < tp.m_lTypeList.size()){
												Fid = tp.m_lTypeList[wpos];
											}
										}

										//������
										std::map<int, Fish>::iterator ift = CGameConfig::GetInstance()->FishMap.find(Fid);
										if (ift != CGameConfig::GetInstance()->FishMap.end()){
											Fish &finf = ift->second;
											CFish* pFish = CommonLogic::CreateFish(finf, tp.x, tp.y, 0.0f, ni*tp.m_fInterval, tp.m_fSpeed, tp.m_nPathID, true);
											if (pFish != NULL){
												m_FishManager.Add(pFish);
												SendFish(pFish);
											}
										}
									}
								}
							}
						}
					}
				}
				return;
			}

			++npos;
		}


		//�������ʱ����� ������ʼѡ��ʱ��
		if (m_fSceneTime > SWITCH_SCENE_END)
		{
			int nfpos = 0;
			//��ȡ�������б�
			std::list<DistrubFishSet>::iterator it = CGameConfig::GetInstance()->SceneSets[m_nCurScene].DistrubList.begin();
			while (it != CGameConfig::GetInstance()->SceneSets[m_nCurScene].DistrubList.end())
			{
				//��ǰ���� ������Ⱥ��
				DistrubFishSet &dis = *it;

				if (nfpos >= m_vDistrubFishTime.size())
				{
					break;
				}
				m_vDistrubFishTime[nfpos] += fdt;
				//[nfpos]������ˢ��ʱ�� ���� ��ǰʱ������ʱ�� ����ˢ��ʱ��
				if (m_vDistrubFishTime[nfpos] > dis.ftime)
				{
					//���һ��ˢ��ʱ��
					m_vDistrubFishTime[nfpos] -= dis.ftime;
					//�Ƿ�ǰ�������
					if (HasRealPlayer())
					{
						//��ȡȨ�غ����б���Сֵ
						int nsz = min(dis.Weight.size(), dis.FishID.size());
						//��Ȩ��
						int WeightCount = 0;
						//ˢ��������    ���һ��ˢ����Сֵ�����ֵ
						int nct = RandInt(dis.nMinCount, dis.nMaxCount);
						//��ˢ������
						int nCount = nct;
						//�����ͣ�
						int SnakeType = 0;
						//�����Ƿ���ڴ��� ˢ��������2
						if (dis.nRefershType == ERT_SNAK)
						{
							nCount += 2;
							nct += 2;
						}

						//��ȡһ��ˢ��ID
						uint32_t nRefershID = IDGenerator::GetInstance()->GetID64();

						//��ȡ��Ȩ��
						for (int wi = 0; wi < nsz; ++wi)
							WeightCount += dis.Weight[wi];

						//����Ȩ�ر������1
						if (nsz > 0)
						{
							//��ID
							int ftid = -1;
							//��ȡһ����ͨ·��ID
							int pid = PathManager::GetInstance()->GetRandNormalPathID();
							while (nct > 0)
							{
								//��ͨ��
								if (ftid == -1 || dis.nRefershType == ERT_NORMAL)
								{
									if (WeightCount == 0)
									{//Ȩ��Ϊ0 
										ftid = dis.FishID[0];
									}
									else
									{
										//Ȩ�����
										int wpos = 0, nw = RandInt(0, WeightCount);
										while (nw > dis.Weight[wpos])
										{
											if (wpos < 0 || wpos >= dis.Weight.size()) break;
											nw -= dis.Weight[wpos];
											++wpos;
											if (wpos >= nsz)
												wpos = 0;
										}
										if (wpos >= 0 || wpos < dis.FishID.size())
											ftid = dis.FishID[wpos];
									}

									SnakeType = ftid;
								}
								//�����ˢ���ߣ���ȡͷ��β
								if (dis.nRefershType == ERT_SNAK)
								{
									if (nct == nCount)
										ftid = CGameConfig::GetInstance()->nSnakeHeadType;
									else if (nct == 1)
										ftid = CGameConfig::GetInstance()->nSnakeTailType;
								}
								//������
								std::map<int, Fish>::iterator ift = CGameConfig::GetInstance()->FishMap.find(ftid);
								if (ift != CGameConfig::GetInstance()->FishMap.end())
								{
									Fish &finf = ift->second;
									//������ͨ
									int FishType = ESFT_NORMAL;
									//���ƫ��ֵ
									float xOffest = RandFloat(-dis.OffestX, dis.OffestX);
									float yOffest = RandFloat(-dis.OffestY, dis.OffestY);
									//�����ʱʱ��
									float fDelay = RandFloat(0.0f, dis.OffestTime);
									//������߻���� �����
									if (dis.nRefershType == ERT_LINE || dis.nRefershType == ERT_SNAK)
									{
										xOffest = dis.OffestX;
										yOffest = dis.OffestY;
										fDelay = dis.OffestTime * (nCount - nct);
									}
									else if (dis.nRefershType == ERT_NORMAL && m_nSpecialCount < CGameConfig::GetInstance()->nMaxSpecailCount)
									{
										std::map<int, SpecialSet>* pMap = NULL;
										//���������ıһ��������
										int nrand = rand() % 100;
										int fft = ESFT_NORMAL;

										if (nrand < CGameConfig::GetInstance()->nSpecialProb[ESFT_KING])
										{
											pMap = &(CGameConfig::GetInstance()->KingFishMap);
											fft = ESFT_KING;
										}
										else
										{
											nrand -= CGameConfig::GetInstance()->nSpecialProb[ESFT_KING];
										}

										if (nrand < CGameConfig::GetInstance()->nSpecialProb[ESFT_KINGANDQUAN])
										{
											pMap = &(CGameConfig::GetInstance()->KingFishMap);
											fft = ESFT_KINGANDQUAN;
										}
										else
										{
											nrand -= CGameConfig::GetInstance()->nSpecialProb[ESFT_KINGANDQUAN];
										}

										if (nrand < CGameConfig::GetInstance()->nSpecialProb[ESFT_SANYUAN])
										{
											pMap = &(CGameConfig::GetInstance()->SanYuanFishMap);
											fft = ESFT_SANYUAN;
										}
										else
										{
											nrand -= CGameConfig::GetInstance()->nSpecialProb[ESFT_SANYUAN];
										}

										if (nrand < CGameConfig::GetInstance()->nSpecialProb[ESFT_SIXI])
										{
											pMap = &(CGameConfig::GetInstance()->SiXiFishMap);
											fft = ESFT_SIXI;
										}
										//�ж��Ƿ������������
										if (pMap != NULL)
										{
											std::map<int, SpecialSet>::iterator ist = pMap->find(ftid);
											if (ist != pMap->end())
											{
												SpecialSet& kks = ist->second;
												//���������������ж��Ƿ�����
												if (RandFloat(0, MAX_PROBABILITY) < kks.fProbability)
													FishType = fft;
											}
										}
									}
									//������
									CFish* pFish = CommonLogic::CreateFish(finf, xOffest, yOffest, 0.0f, fDelay, finf.nSpeed, pid, false, FishType);
									if (pFish != NULL)
									{
										//������ID
										pFish->SetRefershID(nRefershID);
										m_FishManager.Add(pFish);
										SendFish(pFish);
									}
								}

								if (ftid == CGameConfig::GetInstance()->nSnakeHeadType)
									ftid = SnakeType;

								--nct;
							}
						}
					}
				}
				++it;
				++nfpos;
			}
		}
	}
	else
	{//������ʱ����ڳ�������ʱ�� �л�����
		//��ȡ��һ����ID ���ж��Ƿ����
		int nex = CGameConfig::GetInstance()->SceneSets[m_nCurScene].nNextID;
		if (CGameConfig::GetInstance()->SceneSets.find(nex) != CGameConfig::GetInstance()->SceneSets.end())
		{
			m_nCurScene = nex;
		}
		//���ó���
		ResetSceneDistrub();
		//������ ������ ������״̬ �ӵ�
        int GuID = 0;
		for (auto& iter:m_ChairPlayers)
		{
			iter.second->SetLocking(false);
			iter.second->SetLockFishID(0);
			iter.second->ClearBulletCount();
			if (iter.second->GetGuid() == 0){
                continue;
            }

			GuID = iter.second->GetGuid();
            //���� ������Ϣ
			SC_LockFish tinfo;
			tinfo.set_chair_id(iter.first);
			tinfo.set_lock_id(0);
			SendTo_pb(0, tinfo);
		}

		//�趨���ɿ��� ������
		m_bAllowFire = false;
        SendAllowFire(-1);

        //���ͳ����滻
		
		SC_SwitchScene tinfo;
		tinfo.set_nst(m_nCurScene);
		tinfo.set_switching(1);
		SendTo_pb(0, tinfo);

		//�����
		m_FishManager.Clear();

		m_fSceneTime = 0.0f;
	}
}
//��ȡ������� ���ţ�ÿ��ѭ����ȡ��
int	catch_fish_logic::CountPlayer()
{
	return m_ChairPlayers.size();
}



//����������
void catch_fish_logic::SendFish(CFish* pFish, uint32_t wChairID){
	auto ift = CGameConfig::GetInstance()->FishMap.find(pFish->GetTypeID());
	if (ift == CGameConfig::GetInstance()->FishMap.end()){
		return;
	}

	Fish finf = ift->second;
	MoveCompent* pMove = (MoveCompent*)pFish->GetComponent(ECF_MOVE);
	BufferMgr* pBM = (BufferMgr*)pFish->GetComponent(ECF_BUFFERMGR);


	SC_SendFish tinfo;
	tinfo.set_fish_id(pFish->GetId());
	
	tinfo.set_type_id(pFish->GetTypeID());
	tinfo.set_create_tick(pFish->GetCreateTick());
	tinfo.set_fis_type(pFish->GetFishType());
	tinfo.set_refersh_id(pFish->GetRefershID());

	if (pMove != NULL){
		tinfo.set_path_id(pMove->GetPathID());
		if (pMove->GetID() == EMCT_DIRECTION){
			tinfo.set_offest_x(pMove->GetPostion().x_);
			tinfo.set_offest_y(pMove->GetPostion().y_);
		}else{
			tinfo.set_offest_x(pMove->GetOffest().x_);
			tinfo.set_offest_y(pMove->GetOffest().y_);
		}
		tinfo.set_dir(pMove->GetDirection());
		tinfo.set_delay(pMove->GetDelay());
		tinfo.set_fish_speed(pMove->GetSpeed());
		tinfo.set_troop(pMove->bTroop());
	}

	if (pBM != NULL && pBM->HasBuffer(EBT_ADDMUL_BYHIT)){
		PostEvent("FishMulChange", pFish);
	}

	tinfo.set_server_tick(timeGetTime());

	if (wChairID == INVALID_CHAIR){
		SendTo_pb(0, tinfo);
	}else{
		SendTo_pb(m_ChairPlayers[wChairID]->GetGuid(), tinfo);
	}
}

void catch_fish_logic::SendFishList(uint32_t wChairID){
	if (m_FishManager.CountObject() == 0){
		return;
	}
	int fish_size = m_FishManager.CountObject();
	std::vector<SC_SendFishList> t_tinfo;
	t_tinfo.resize(8);

	int this_count = 0;
	for (auto& iter = m_FishManager.Begin(); iter != m_FishManager.End(); iter++){
		CFish* pFish = (CFish*)iter->second;
		MoveCompent* pMove = (MoveCompent*)pFish->GetComponent(ECF_MOVE);
		BufferMgr* pBM = (BufferMgr*)pFish->GetComponent(ECF_BUFFERMGR);
		
		SC_SendFish* ff = t_tinfo[this_count].add_pb_fishes();
		this_count++; 
		if (this_count >= 8){
			this_count = 0;
		}
		ff->set_fish_id(pFish->GetId());
		ff->set_type_id(pFish->GetTypeID());
		ff->set_create_tick(pFish->GetCreateTick());
		ff->set_fis_type(pFish->GetFishType());
		ff->set_refersh_id(pFish->GetRefershID());

		if (pMove != NULL){
			ff->set_path_id(pMove->GetPathID());
			if (pMove->GetID() == EMCT_DIRECTION){
				ff->set_offest_x(pMove->GetPostion().x_);
				ff->set_offest_y(pMove->GetPostion().y_);
			}else{
				ff->set_offest_x(pMove->GetOffest().x_);
				ff->set_offest_y(pMove->GetOffest().y_);
			}

			ff->set_dir(pMove->GetDirection());
			ff->set_delay(pMove->GetDelay());
			ff->set_fish_speed(pMove->GetSpeed());
			ff->set_troop(pMove->bTroop());
		}

		if (pBM != NULL && pBM->HasBuffer(EBT_ADDMUL_BYHIT)){
			PostEvent("FishMulChange", pFish);
		}

		ff->set_server_tick(timeGetTime());
	}

	for (auto& item : t_tinfo)
	{
		if (!item.pb_fishes().empty())
		{
			if (wChairID == INVALID_CHAIR){
				SendTo_pb(0, item);
			}
			else{
				SendTo_pb(m_ChairPlayers[wChairID]->GetGuid(), item);
			}
		}
	}
}

//�ı���ڼ�
bool catch_fish_logic::OnChangeCannonSet(lua_tinker::table player, int add)
{
	delete_invalid_player();
	int chair_id = player.get<int>("chair_id");

	if (chair_id >= GAME_PLAYER) return false;

	BufferMgr* pBMgr = (BufferMgr*)m_ChairPlayers[chair_id]->GetComponent(ECF_BUFFERMGR);
	if (pBMgr != NULL && (pBMgr->HasBuffer(EBT_DOUBLE_CANNON) || pBMgr->HasBuffer(EBT_ION_CANNON)))
	{
		return true;//�����ڻ�������ʱ��ֹ����
	}
	//��ȡ���ڼ�����
	int n = m_ChairPlayers[chair_id]->GetCannonSetType();

	do
	{
		if (add){
			if (n < CGameConfig::GetInstance()->CannonSetArray.size() - 1){
				++n;
			}else{
				n = 0;
			}
		}else{
			if (n >= 1){
				--n;
			}else{
				n = CGameConfig::GetInstance()->CannonSetArray.size() - 1;
			}
		}//����������ID ��˫��ID���˳�ѭ��
	} while (n == CGameConfig::GetInstance()->CannonSetArray[n].nIonID || n == CGameConfig::GetInstance()->CannonSetArray[n].nDoubleID);

	if (n < 0) n = 0;
	if (n >= CGameConfig::GetInstance()->CannonSetArray.size()){
		n = CGameConfig::GetInstance()->CannonSetArray.size() - 1;
	}

	//���ô��ڼ����� ��CacluteCannonPos ��ȡ���Ǵ������� m_nCannonType
	m_ChairPlayers[chair_id]->SetCannonSetType(n);
	//�����������
	m_ChairPlayers[chair_id]->CacluteCannonPos(chair_id);
	//���ʹ�����Ϣ
	SendCannonSet(chair_id);

	return true;
}
//����
bool catch_fish_logic::OnFire(lua_tinker::table player, lua_tinker::table msg)
{
	delete_invalid_player();
	int guid = player.get<int>("guid");
	auto chair_id = player.get<int>("chair_id");
	double Direction = msg.get<double>("direction");
	int ClientID = msg.get<int>("client_id");
	uint32_t FireTime = msg.get<uint32_t>("fire_time");
	MyPoint bullet_pos(msg.get<double>("pos_x"), msg.get<double>("pos_y"));

	//��ȡ�ӵ�����
	int mul = m_ChairPlayers[chair_id]->GetMultiply();
	if (mul < 0 || mul >= CGameConfig::GetInstance()->BulletVector.size()){
		std::cout << "invalid bullet multiple" << std::endl;
		return false;
	}

	//��������ҿ��Կ���
	if (m_bAllowFire && 
		(HasRealPlayer() || CGameConfig::GetInstance()->bImitationRealPlayer) && m_ChairPlayers[chair_id]->CanFire()){
		//��ȡ�ӵ�
		Bullet &binf = CGameConfig::GetInstance()->BulletVector[mul];
		//��ҽ�Ǯ�����ӵ�ֵ�� �� ������ӵ��� С������ӵ���
		if (m_ChairPlayers[chair_id]->GetScore() >= binf.nMulriple &&
			m_ChairPlayers[chair_id]->GetBulletCount() + 1 <= CGameConfig::GetInstance()->nMaxBullet){
			m_ChairPlayers[chair_id]->SetFired();
	
			// ����˰�պ������Ӯ����
			Earnings::getInstance().onUserFire(binf.nMulriple);
			m_UserWinScore[chair_id] -= binf.nMulriple;
			m_ChairPlayers[chair_id]->AddScore(-binf.nMulriple);

			//�����ӵ�
			CBullet* pBullet = CommonLogic::CreateBullet(binf, bullet_pos, Direction,
				m_ChairPlayers[chair_id]->GetCannonType(), m_ChairPlayers[chair_id]->GetMultiply(), false);
			if (pBullet != NULL){
				if (ClientID != 0){
					pBullet->SetId(ClientID);
				}

				pBullet->SetChairID(chair_id);       //��������
				pBullet->SetCreateTick(chair_id);   //���ÿ���ʱ�� ��ʱ����ЧУ��

				//�������BUFF�Ƿ���˫����BUFF
				BufferMgr* pBMgr = (BufferMgr*)m_ChairPlayers[chair_id]->GetComponent(ECF_BUFFERMGR);
				if (pBMgr != NULL && pBMgr->HasBuffer(EBT_DOUBLE_CANNON)){
					pBullet->setDouble(true);
				}

				//�Ƿ���������
				if (m_ChairPlayers[chair_id]->GetLockFishID() != 0){
					//��ȡ�ӵ��ƶ��ؼ�
					MoveCompent* pMove = (MoveCompent*)pBullet->GetComponent(ECF_MOVE);
					if (pMove != NULL){
						pMove->SetTarget(&m_FishManager, m_ChairPlayers[chair_id]->GetLockFishID());
					}
				}

				uint32_t now = timeGetTime();
				if (FireTime > now){
					//m_pITableFrame->SendTableData(pf->wChairID, SUB_S_FORCE_TIME_SYNC);
				}else{
					//����ӵ�����ʱ�����2��ִ�и����¼��������
					uint32_t delta = now - FireTime;
					if (delta > 2000) delta = 2000;
					pBullet->OnUpdate(delta);
				}

				//�����ӵ�
				m_ChairPlayers[chair_id]->ADDBulletCount(1);
				m_BulletManager.Add(pBullet);
				//�����ӵ�
				SendBullet(pBullet, true);
			}else{
				SC_KillBullet tinfo;
				tinfo.set_chair_id(chair_id);              //����ID
				tinfo.set_bullet_id(ClientID);
				SendTo_pb(0, tinfo);
				std::cout << "create bullet failed" << std::endl;
			}

			//������󿪻�ʱ��
			m_ChairPlayers[chair_id]->SetLastFireTick(timeGetTime());
		}else{
			SC_KillBullet tinfo;
			tinfo.set_chair_id(chair_id);              //����ID
			tinfo.set_bullet_id(ClientID);
			SendTo_pb(0, tinfo);

			std::cout << "Score less or reach max bullet count.guid:"<< guid << " count:" << m_ChairPlayers[chair_id]->GetBulletCount() 
				<< " max count:" << CGameConfig::GetInstance()->nMaxBullet << std::endl;
		}
	}
	else{
		SC_KillBullet tinfo;
		tinfo.set_chair_id(chair_id);              //����ID
		tinfo.set_bullet_id(ClientID);
		SendTo_pb(0, tinfo);
		std::cout <<"Do not allow fire,but fired." << std::endl;
	}

	return true;
}
//�����ӵ�
void catch_fish_logic::SendBullet(CBullet* pBullet, bool bNew)
{
	if (pBullet == NULL) return;

	if (!m_ChairPlayers[pBullet->GetChairID()])
	{
		return;
	}

	SC_SendBullet tinfo;
	tinfo.set_chair_id(pBullet->GetChairID());
	tinfo.set_id(pBullet->GetId());
	tinfo.set_cannon_type(pBullet->GetCannonType());
	tinfo.set_multiply(pBullet->GetTypeID());
	tinfo.set_direction(pBullet->GetDirection());
	tinfo.set_x_pos(pBullet->GetPosition().x_);
	tinfo.set_y_pos(pBullet->GetPosition().y_);
	tinfo.set_score(m_ChairPlayers[pBullet->GetChairID()]->GetScore());
	tinfo.set_is_new(bNew ? 1 : 0);
	tinfo.set_is_double(pBullet->bDouble() ? 1 : 0);
	tinfo.set_server_tick(timeGetTime());
	if (bNew){
		tinfo.set_create_tick(pBullet->GetCreateTick());
	}
	else{
		tinfo.set_create_tick(timeGetTime());
	}
	SendTo_pb(0, tinfo);
}

//����ϵͳʱ��
bool catch_fish_logic::OnTimeSync(lua_tinker::table player, int	client_tick)
{
	delete_invalid_player();
	int chair_id = player.get<int>("chair_id");
	int guid = player.get<int>("guid");
	if (guid != 0){
		SC_TimeSync tinfo;
		tinfo.set_chair_id(chair_id);
		tinfo.set_client_tick(client_tick);
		tinfo.set_server_tick(timeGetTime());
		SendTo_pb(guid, tinfo);
        return true;
    }

    return false;
}

//�任����
bool catch_fish_logic::OnChangeCannon(lua_tinker::table player, int add){
	delete_invalid_player();
	uint32_t ChairID = player.get<uint32_t>("chair_id");
	if (ChairID > GAME_PLAYER){
		return false;
	}


	//��ȡBuff������
	BufferMgr* pBMgr = (BufferMgr*)m_ChairPlayers[ChairID]->GetComponent(ECF_BUFFERMGR);
	//�鿴��ǰ�����Ƿ�Ϊ˫����������
	if (pBMgr != NULL && (pBMgr->HasBuffer(EBT_DOUBLE_CANNON) || pBMgr->HasBuffer(EBT_ION_CANNON))){
		return true;//�����ڻ�������ʱ��ֹ����
	}

	//��ȡ��ǰ�ӵ�����
	int mul = m_ChairPlayers[ChairID]->GetMultiply();

	if (add){
		++mul;
	}else{
		--mul;
	}
	//ѭ������
	if (mul < 0) mul = CGameConfig::GetInstance()->BulletVector.size() - 1;
	if (mul >= CGameConfig::GetInstance()->BulletVector.size()) mul = 0;
	//��������
	m_ChairPlayers[ChairID]->SetMultiply(mul);
	//��ȡ�ӵ���Ӧ��������
	int CannonType = CGameConfig::GetInstance()->BulletVector[mul].nCannonType;
	//������
	m_ChairPlayers[ChairID]->SetCannonType(CannonType);
	//����������
	SendCannonSet(ChairID);
	//�������һ�ο���ʱ��
	m_ChairPlayers[ChairID]->SetLastFireTick(timeGetTime());

	return true;
}
//���ʹ�������
void catch_fish_logic::SendCannonSet(int wChairID){

	auto iter = m_ChairPlayers.find(wChairID);
	if (iter == m_ChairPlayers.end()){
		return;
	}

	SC_CannonSet tinfo;
	tinfo.set_chair_id(iter->first);
	tinfo.set_cannon_mul(iter->second->GetMultiply());
	tinfo.set_cannon_type(iter->second->GetCannonType());
	tinfo.set_cannon_set(iter->second->GetCannonSetType());
	SendTo_pb(0, tinfo);
}
//�򿪱���
bool catch_fish_logic::OnTreasureEND(lua_tinker::table player, int64_t score)
{
	delete_invalid_player();
	int ChairID = player.get<int>("chair_id");
	int Guid = player.get<int>("guid");

	if (ChairID > 0 && ChairID <= GAME_PLAYER && Guid != 0){
		char szInfo[512] = {0};
		std::wstring str = TEXT("��ϲ%s��%d������ҡ�%s�����б���,�������л��%lld���!!!");
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		std::string narrowStr = conv.to_bytes(str);
		sprintf_s(szInfo, narrowStr.c_str(), "fishing",
			GetTableID(), m_GuidPlayers[Guid].GetNickname().c_str(), score);
		RaiseEvent("CatchFishBroadCast", szInfo, &m_GuidPlayers[Guid]);
	}

	return true;
}
//
void catch_fish_logic::ReturnBulletScore(int guid)
{
    {
		lua_tinker::call<void, int, int64_t>(
			catchfish_dll::sLuaState, "write_player_money", guid, m_UserWinScore[m_GuidPlayers[guid].GetChairID()]
			);
    }

#if 0
	if (wChairID >= GAME_PLAYER)
	{
		DebugString(TEXT("[Fish]ReturnBulletScore Err: wTableID %d wChairID %d"), m_pITableFrame->GetTableID(), wChairID);
		return;
	}
	try
	{
		IServerUserItem* pIServerUserItem = m_pITableFrame->GetTableUserItem(wChairID);
		if (pIServerUserItem != NULL)
		{
			// 			int64_t score = m_player[wChairID].GetScore();
			// 			if(score != 0)
			// 			{
			// 				long long  ls = score * CGameConfig::GetInstance()->nChangeRatioUserScore / CGameConfig::GetInstance()->nChangeRatioFishScore;
			// 				m_player[wChairID].AddWastage(-ls);
			// 			}
			// 
			// 			tagScoreInfo ScoreInfo;
			// 			ZeroMemory(&ScoreInfo, sizeof(tagScoreInfo));
			// 			score = -m_player[wChairID].GetWastage();
			// 			long long  lReve=0,cbRevenue=m_pGameServiceOption->wRevenueRatio;	
			// 			if (score > 0)
			// 			{	
			// 				float fRevenuePer = float(cbRevenue/1000);
			// 				lReve  = long long (score*fRevenuePer);
			// 				ScoreInfo.cbType = SCORE_TYPE_WIN;
			// 			}
			// 			else if (score < 0)
			// 				ScoreInfo.cbType = SCORE_TYPE_LOSE;
			// 			else
			// 				ScoreInfo.cbType = SCORE_TYPE_DRAW;
			// 			ScoreInfo.lScore = score;
			// 			ScoreInfo.lRevenue = lReve;
			// 
			// 			m_pITableFrame->WriteUserScore(wChairID, ScoreInfo);

			if (user_win_scores_[wChairID] != 0 || user_revenues_[wChairID] != 0) {// �з��ڹ�
				tagScoreInfo ScoreInfo = { 0 };
				ScoreInfo.cbType = (user_win_scores_[wChairID] > 0L) ? SCORE_TYPE_WIN : SCORE_TYPE_LOSE;
				ScoreInfo.lRevenue = user_revenues_[wChairID];
				ScoreInfo.lScore = user_win_scores_[wChairID];
				user_revenues_[wChairID] = 0;
				user_win_scores_[wChairID] = 0;
				m_pITableFrame->WriteUserScore(wChairID, ScoreInfo);
			}

			m_player[wChairID].ClearSet(wChairID);
		}
	}
	catch (...)
	{
		CTraceService::TraceString(TEXT("ReturnBulletScore����1"), TraceLevel_Exception);
		DebugString(TEXT("[Fish]ReturnBulletScore����1"));
	}

	std::list<uint32_t> rmList;
	m_BulletManager.Lock();
	try
	{
		obj_table_iter ibu = m_BulletManager.Begin();
		while (ibu != m_BulletManager.End())
		{
			CBullet* pBullet = (CBullet*)ibu->second;
			if (pBullet->GetChairID() == wChairID)
				rmList.push_back(pBullet->GetId());

			++ibu;
		}
	}
	catch (...)
	{
		CTraceService::TraceString(TEXT("ReturnBulletScore����2"), TraceLevel_Exception);
		DebugString(TEXT("[Fish]ReturnBulletScore����2"));
	}
	m_BulletManager.Unlock();

	std::list<uint32_t>::iterator it = rmList.begin();
	while (it != rmList.end())
	{
		m_BulletManager.Remove(*it);
		++it;
	}

	rmList.clear();
#endif
}
//�����¼�
void catch_fish_logic::OnAdwardEvent(CMyEvent* pEvent)
{
	//�ж��¼��Ƿ�Ϊ���¼�
	if (pEvent == NULL || pEvent->GetName() != "AdwardEvent") return;
	//�����¼�
	CEffectAward* pe = (CEffectAward*)pEvent->GetParam();
	//��
	CFish* pFish = (CFish*)pEvent->GetSource();
	//�ӵ�
	CBullet* pBullet = (CBullet*)pEvent->GetTarget();

	if (pe == NULL || pFish == NULL || pBullet == NULL) return;
	//������Ҳ��ɿ���
	m_ChairPlayers[pBullet->GetChairID()]->SetCanFire(false);

	long long  lScore = 0;
	//GetParam(1) ��������ʾʵ��Ч�� ���ӽ�ҡ������ӣ£գƣƣţ�
	if (pe->GetParam(1) == 0)
	{
		if (pe->GetParam(2) == 0)
			lScore = pe->GetParam(3);
		else
			lScore = pBullet->GetScore() * pe->GetParam(3);
	}
	else
	{
		//��ʹ�ӵ���BUFF
		BufferMgr* pBMgr = (BufferMgr*)m_ChairPlayers[pBullet->GetChairID()]->GetComponent(ECF_BUFFERMGR);
		if (pBMgr != NULL && !pBMgr->HasBuffer(pe->GetParam(2)))
		{
			//GetParam(2)���� GetParam(3)����ʱ��
			pBMgr->Add(pe->GetParam(2), 0, pe->GetParam(3));
		}
	}
	//��Ҽ�Ǯ
	m_ChairPlayers[pBullet->GetChairID()]->AddScore(lScore);
}
//������BUFF
void catch_fish_logic::OnAddBuffer(CMyEvent* pEvent)
{
	if (pEvent == NULL || pEvent->GetName() != "AddBuffer") return;
	CEffectAddBuffer* pe = (CEffectAddBuffer*)pEvent->GetParam();

	CFish* pFish = (CFish*)pEvent->GetSource();
	if (pFish == NULL) return;

	if (pFish->GetMgr() != &m_FishManager) return;

	//��Ŀ����ȫ����������Ϊ�ı��ٶ� �ı�ֵΪ0ʱ ���� ʱ��Ϊpe->GetParam(4)
	if (pe->GetParam(0) == 0 && pe->GetParam(2) == EBT_CHANGESPEED && pe->GetParam(3) == 0)//����
	{//��ֹֻͣ��ˢ��?
		m_fPauseTime = pe->GetParam(4);
	}
}
//ִ��������Ч��
void catch_fish_logic::OnMulChange(CMyEvent* pEvent)
{
	int GuID = GetFirstPlayerGuID();
	if (GuID <= 0){
        return;
    }

	if (pEvent == NULL || pEvent->GetName() != "FishMulChange") return;

	CFish* pFish = (CFish*)pEvent->GetParam();
	if (pFish != NULL)
	{
		m_FishManager.Lock();
		obj_table_iter ifs = m_FishManager.Begin();
		while (ifs != m_FishManager.End())
		{

			CFish* pf = (CFish*)ifs->second;
			//�ҵ�һ��ͬ����㣬Ȼ��ִ������Ч��
			if (pf != NULL && pf->GetTypeID() == pFish->GetTypeID())
			{
				CBullet bt;
				bt.SetScore(1);
				std::list<MyObject*> llt;
				llt.clear();
				//����ҵ������������� 
				EffectMgr* pEM = (EffectMgr*)pf->GetComponent(ECF_EFFECTMGR);
                int multemp = 0;
				if (pEM != NULL)
				{//ִ������Ч��
                    multemp = pEM->Execute(&bt, llt, true);
				}

				SC_FishMul tinfo;
				tinfo.set_fish_id(pf->GetId());
				tinfo.set_mul(multemp);
				SendTo_pb(0, tinfo);
			}

			++ifs;

		}
		m_FishManager.Unlock();
	}
}
//��һ�ο��� Ϊɶ��������� ��һ����������
void catch_fish_logic::OnFirstFire(CMyEvent* pEvent)
{
	if (pEvent == NULL || pEvent->GetName() != "FirstFire") return;

	CPlayer* pPlayer = (CPlayer*)pEvent->GetParam();

	if (m_ChairPlayers.find(pPlayer->GetChairID()) == m_ChairPlayers.end()){
		return;
	}

	int npos = 0;
	npos = CGameConfig::GetInstance()->FirstFireList.size() - 1;
	FirstFire& ff = CGameConfig::GetInstance()->FirstFireList[npos];
	//����������Ȩ����ȡ���ֵ
	int nsz = min(ff.FishTypeVector.size(), ff.WeightVector.size());

	if (nsz <= 0) return;

	//��Ȩ��
	int WeightCount = 0;
	for (int iw = 0; iw < nsz; ++iw){
		WeightCount += ff.WeightVector[iw];
	}

	//��ȡ����λ��
	MyPoint pt = pPlayer->GetCannonPos();
	//��ȡ���ڷ���
	float dir = CGameConfig::GetInstance()->CannonPos[pPlayer->GetChairID()].m_Direction;
	//������
	for (int nc = 0; nc < ff.nCount; ++nc)
	{
		//�۸������
		for (int ni = 0; ni < ff.nPriceCount; ++ni)
		{
			//��ȡ һ����
			int Fid = ff.FishTypeVector[RandInt(0, nsz)];
			//���һ��Ȩ��
			int nf = RandInt(0, WeightCount);
			int wpos = 0;
			//ƥ��һ��Ȩ��
			for (; wpos < nsz; ++wpos)
			{
				if (nf > ff.WeightVector[wpos])
				{
					nf -= ff.WeightVector[wpos];
				}
				else
				{
					Fid = ff.FishTypeVector[wpos];
					break;;
				}
			}
			//���û��ƥ�䵽��ƥ���һ��
			if (wpos >= nsz)
			{
				Fid = ff.FishTypeVector[0];
			}

			//�������սǶȣ�
			dir = CGameConfig::GetInstance()->CannonPos[pPlayer->GetChairID()].m_Direction - M_PI_2 + M_PI / ff.nPriceCount * ni;

			//����ƥ�䵽����
			std::map<int, Fish>::iterator ift = CGameConfig::GetInstance()->FishMap.find(Fid);
			if (ift != CGameConfig::GetInstance()->FishMap.end())
			{
				Fish& finf = ift->second;

				//������
				CFish* pFish = CommonLogic::CreateFish(finf, pt.x_, pt.y_, dir, RandFloat(0.0f, 1.0f) + nc, finf.nSpeed, -2);
				if (pFish != NULL)
				{
					m_FishManager.Add(pFish);
					SendFish(pFish);
				}
			}
		}
	}
}
//������
void catch_fish_logic::OnProduceFish(CMyEvent* pEvent)
{
	if (pEvent == NULL || pEvent->GetName() != "ProduceFish") return;

	CEffectProduce* pe = (CEffectProduce*)pEvent->GetParam();
	//SourceΪ��
	CFish* pFish = (CFish*)pEvent->GetSource();
	if (pFish == NULL) return;

	if (pFish->GetMgr() != &m_FishManager) return;
	//��ȡ����
	MyPoint& pt = pFish->GetPosition();
    list<SC_stSendFish> msg;
	//ͨ��ID������
	std::map<int, Fish>::iterator ift = CGameConfig::GetInstance()->FishMap.find(pe->GetParam(0));
	if (ift != CGameConfig::GetInstance()->FishMap.end()){
		Fish finf = ift->second;
		float fdt = M_PI * 2.0f / (float)pe->GetParam(2);
		//����Ϊ��ͨ
		int fishtype = ESFT_NORMAL;
		int ndif = -1;
		//����ѭ��
		for (int i = 0; i < pe->GetParam(1); ++i){
			//�����һ�����������δ���2 ˢ����������10ֻʱ ���һ����ˢ��Ϊ����
			if ((i == pe->GetParam(1) - 1) && (pe->GetParam(1) > 2) && (pe->GetParam(2) > 10)){
				ndif = RandInt(0, pe->GetParam(2));
			}

			//ˢ������
			for (int j = 0; j < pe->GetParam(2); ++j){
				if (j == ndif){
					fishtype = ESFT_KING;
				}else{
					fishtype = ESFT_NORMAL;
				}

				//������
				CFish* pf = CommonLogic::CreateFish(finf, pt.x_, pt.y_, fdt*j, 1.0f + pe->GetParam(3)*i, finf.nSpeed, -2, false, fishtype);
				if (pf != NULL){
					m_FishManager.Add(pf);
					//����ֻ��������
                    SC_stSendFish fish;
					fish.fish_id = pf->GetId();
                    fish.type_id = pf->GetTypeID();
                    fish.create_tick = pf->GetCreateTick();
                    fish.fis_type = pf->GetFishType();
                    fish.refersh_id = pf->GetRefershID();
					//����ƶ����
					MoveCompent* pMove = (MoveCompent*)pf->GetComponent(ECF_MOVE);
					if (pMove != NULL){
						fish.path_id = pMove->GetPathID();
						fish.offest_x = pMove->GetOffest().x_;
						fish.offest_y = pMove->GetOffest().y_;
						if (pMove->GetID() == EMCT_DIRECTION){
							fish.offest_x = pMove->GetPostion().x_;
							fish.offest_y = pMove->GetPostion().y_;
						}
						fish.dir = pMove->GetDirection();
						fish.delay = pMove->GetDelay();
						fish.fish_speed = pMove->GetSpeed();
						fish.troop = pMove->bTroop() ? 1 : 0;
					}

					BufferMgr* pBM = (BufferMgr*)pf->GetComponent(ECF_BUFFERMGR);
					if (pBM != NULL && pBM->HasBuffer(EBT_ADDMUL_BYHIT)){//�ҵ�BUFF������������BUFF ���� ���ӵ� ����¼�
						PostEvent("FishMulChange", pf);
					}

					fish.server_tick = timeGetTime();
                    msg.push_back(fish);
				}
			}
		}
	}

	if (msg.size() == 0){
		return;
	}


	int fish_size = msg.size();
	std::vector<SC_SendFishList> t_tinfo;
	t_tinfo.resize(8);

	int this_count = 0;
	for (list<SC_stSendFish>::iterator it = msg.begin(); it != msg.end(); ++it)
	{
		SC_SendFish* pff = t_tinfo[this_count].add_pb_fishes();
		this_count++;
		if (this_count >= 8){
			this_count = 0;
		}
		SC_stSendFish &temp = *it;
		pff->set_fish_id(temp.fish_id); //��ID
		pff->set_type_id(temp.type_id);  //���ͣ�
		pff->set_path_id(temp.path_id);  //·��ID
		pff->set_create_tick(temp.create_tick);  //����ʱ��
		pff->set_offest_x(temp.offest_x);  //X����
		pff->set_offest_y(temp.offest_y);  //Y����
		pff->set_dir(temp.dir);  //����
		pff->set_delay(temp.delay);  //��ʱ
		pff->set_server_tick(temp.server_tick);  //ϵͳʱ��
		pff->set_fish_speed(temp.fish_speed);  //���ٶ�
		pff->set_fis_type(temp.fis_type);  //�����ͣ�
		pff->set_troop(temp.troop);      //�Ƿ���Ⱥ
		pff->set_refersh_id(temp.refersh_id);  //��ȡˢ��ID��
	}

	for (auto& item : t_tinfo)
	{
		if (!item.pb_fishes().empty())
		{
			SendTo_pb(0, item);
		}
	}

}
//������
void catch_fish_logic::LockFish(unsigned int wChairID)
{
	uint32_t dwFishID = 0;

	CFish* pf = NULL;
	//��ȡ��ǰ����ID
	dwFishID = m_ChairPlayers[wChairID]->GetLockFishID();
	if (dwFishID != 0){
		pf = (CFish*)m_FishManager.Find(dwFishID);
	}

	if (pf != NULL){
		//�жϵ�ǰ������ �Ƿ��Ѿ�����������
		MoveCompent* pMove = (MoveCompent*)pf->GetComponent(ECF_MOVE);
		if (pf->GetState() >= EOS_DEAD || pMove == NULL || pMove->IsEndPath()){
			pf = NULL;
		}
	}

	dwFishID = 0;

	CFish* pLock = NULL;

	//��ѯ�������б�
	for (std::list<uint32_t>::iterator iw = m_CanLockList.begin(); iw != m_CanLockList.end();++iw){
		//������
		CFish* pFish = (CFish*)m_FishManager.Find(*iw);
		//��ǰ����Ч �� û���� �� �����ȼ�����0 �� û���γ���Ļ
		if (pFish != NULL && pFish->GetState() < EOS_DEAD && pFish->GetLockLevel() > 0 && pFish->InSideScreen()){
			//��ȡ�����������ȼ�����
			if (pf == NULL || (pf != pFish && !m_ChairPlayers[wChairID]->HasLocked(pFish->GetId()))){
				pf = pFish;

				if (pLock == NULL){
					pLock = pf;
				}else if (pf->GetLockLevel() > pLock->GetLockLevel()){
					pLock = pf;
				}
			}
		}
	}

	if (pLock != NULL){
		dwFishID = pLock->GetId();
	}

	//��������ID 
	m_ChairPlayers[wChairID]->SetLockFishID(dwFishID);
	if (m_ChairPlayers[wChairID]->GetLockFishID() == 0){
		return;
	}

	SC_LockFish tinfo;
	tinfo.set_chair_id(wChairID);
	tinfo.set_lock_id(dwFishID);
	SendTo_pb(0, tinfo);
}
//������
bool catch_fish_logic::OnLockFish(lua_tinker::table player, int isLock)
{
	int guid = player.get<int>("guid");
	int chair_id = player.get<int>("chair_id");
	//������λ���Ƿ����
	//���û������˳�
	if (!HasRealPlayer()) return true;

	if (isLock){
		//�����������
		m_GuidPlayers[guid].SetLocking(true);
		//������
		LockFish(chair_id);
	}else{
		m_GuidPlayers[guid].SetLocking(false);
		m_GuidPlayers[guid].SetLockFishID(0);

		SC_LockFish tinfo;
		tinfo.set_chair_id(chair_id);
		tinfo.set_lock_id(0);
		SendTo_pb(0, tinfo);
	}
	m_GuidPlayers[guid].SetLastFireTick(timeGetTime());

	return true;

}

bool catch_fish_logic::OnLockSpecFish(lua_tinker::table player, int fishID){
	delete_invalid_player();
	int guid = player.get<int>("guid");
	int chair_id = player.get<int>("chair_id");
	if (!HasRealPlayer()) return true;

	if (fishID > 0){
		CFish* pLockFish = (CFish*)m_FishManager.Find(fishID);
		if (!pLockFish){
			m_GuidPlayers[guid].SetLocking(false);
			m_GuidPlayers[guid].SetLockFishID(0);
			fishID = 0;
		}else{
			m_GuidPlayers[guid].SetLocking(true);
			m_GuidPlayers[guid].SetLockFishID(fishID);
		}
	}else{
		m_GuidPlayers[guid].SetLocking(false);
		m_GuidPlayers[guid].SetLockFishID(0);
	}

	SC_LockFish tinfo;
	tinfo.set_chair_id(chair_id);
	tinfo.set_lock_id(fishID);
	SendTo_pb(0, tinfo);

	m_GuidPlayers[guid].SetLastFireTick(timeGetTime());

	return true;
}

//���� ��Ҵ�������    �����Ǹı���ڣ�
void catch_fish_logic::OnCannonSetChange(CMyEvent* pEvent)
{
	if (pEvent == NULL || pEvent->GetName() != "CannonSetChanaged"){
		return;
	}

	CPlayer* pp = (CPlayer*)pEvent->GetParam();
	if (!pp){
		return;
	}

	SendCannonSet(pp->GetChairID());
}
//����
bool catch_fish_logic::OnNetCast(lua_tinker::table player, int bullet_id, int data, int fish_id)
{
	delete_invalid_player();
	int chair_id = player.get<int>("chair_id");
	int guid = player.get<int>("guid");

	if (m_ChairPlayers.find(chair_id) == m_ChairPlayers.end()){
		std::cout << "can not find chair id:" << chair_id << std::endl;
		return true;
	}

	m_BulletManager.Lock();
	//��ȡ�ӵ�
	CBullet* pBullet = (CBullet*)m_BulletManager.Find(bullet_id);
	if (pBullet != NULL){
		int bulletChairID = pBullet->GetChairID();
		//��ȡ�ӵ����������λ
		if (m_ChairPlayers.find(bulletChairID) == m_ChairPlayers.end()){
			return true;
		}

		m_FishManager.Lock();
		CFish* pFish = (CFish*)m_FishManager.Find(fish_id);
		if (pFish != NULL){
			CatchFish(pBullet, pFish, 1, 0);
		}

		m_FishManager.Unlock();

		//�����ӵ���ʧ
        {
			SC_KillBullet tinfo;
			tinfo.set_chair_id(chair_id);
			tinfo.set_bullet_id(bullet_id);
			SendTo_pb(0, tinfo);
        }

		//����ӵ�-1
		m_ChairPlayers[bulletChairID]->ADDBulletCount(-1);
		//�Ƴ��ӵ�
		m_BulletManager.Remove(bullet_id);
	}
	else
	{
		//std::cout << "invalid bullet id:" << bullet_id << std::endl;
		{
			SC_KillBullet tinfo;
			tinfo.set_chair_id(chair_id);
			tinfo.set_bullet_id(bullet_id);
			SendTo_pb(0, tinfo);
		}
		// TODO: ����ӵ������ڣ�Ҳ���ܵ���һЩ����,�п�������һ������Ѿ����ĳ������ӵ��Ѿ������ˣ��������յ����������Ϣ
	}
	m_BulletManager.Unlock();

	return true;
}

//������㲥 �޴���ֻ���ͣ� ����
void catch_fish_logic::OnCatchFishBroadCast(CMyEvent* pEvent)
{
	if (pEvent != NULL && pEvent->GetName() == "CatchFishBroadCast"){
		//��ȡ���
		CPlayer* pp = (CPlayer*)pEvent->GetSource();
		if (pp != NULL){
			SC_SystemMessage tinfo;
			tinfo.set_wtype(SMT_TABLE_ROLL);
			tinfo.set_szstring((char*)pEvent->GetParam());
			SendTo_pb(0, tinfo);
		}
	}
}
// �������� ���� ���ٹ������ز�
void catch_fish_logic::SetGuidAndGateID(int chair_id, int guid, int gate_id)
{
	if (chair_id >= 0 && chair_id < (int)m_ChairPlayers.size()){
		m_ChairPlayers[chair_id]->SetGuidGateID(guid, gate_id);
		m_ChairPlayers[chair_id]->SetChairID(chair_id);
	}
}


int catch_fish_logic::GetTableID(){
	return m_TableID;
}



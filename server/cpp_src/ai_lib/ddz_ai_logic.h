#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#pragma once
#include <wtypes.h>
#include <assert.h>
#define ASSERT assert
#define CountArray(x) sizeof(x)/sizeof(x[0])
#define INVALID_CHAIR 10
#define SCORE						LONGLONG							//��������
#pragma pack(1)

//////////////////////////////////////////////////////////////////////////////////
//������

//��Ϸ����
#define KIND_ID						200									//��Ϸ I D
#define GAME_NAME					TEXT("������")						//��Ϸ����

//�������
#define GAME_PLAYER					3									//��Ϸ����
#define VERSION_SERVER				PROCESS_VERSION(6,0,3)				//����汾
#define VERSION_CLIENT				PROCESS_VERSION(6,0,3)				//����汾

//////////////////////////////////////////////////////////////////////////////////

//��Ŀ����
#define MAX_COUNT					20									//�����Ŀ
#define FULL_COUNT					54									//ȫ����Ŀ

//�߼���Ŀ
#define NORMAL_COUNT				17									//������Ŀ
#define DISPATCH_COUNT				51									//�ɷ���Ŀ
#define GOOD_CARD_COUTN				38									//������Ŀ

//��ֵ����
#define	MASK_COLOR					0xF0								//��ɫ����
#define	MASK_VALUE					0x0F								//��ֵ����

//�߼�����
#define CT_ERROR					0									//��������
#define CT_SINGLE					1									//��������
#define CT_DOUBLE					2									//��������
#define CT_THREE					3									//��������
#define CT_SINGLE_LINE				4									//��������
#define CT_DOUBLE_LINE				5									//��������
#define CT_THREE_LINE				6									//��������
#define CT_THREE_TAKE_ONE			7									//����һ��
#define CT_THREE_TAKE_TWO			8									//����һ��
#define CT_FOUR_TAKE_ONE			9									//�Ĵ�����
#define CT_FOUR_TAKE_TWO			10									//�Ĵ�����
#define CT_BOMB_CARD				11									//ը������
#define CT_MISSILE_CARD				12									//�������

//////////////////////////////////////////////////////////////////////////////////

//����״̬
struct CMD_S_StatusFree
{
	//��Ϸ����
	LONG							lCellScore;							//��������

	//ʱ����Ϣ
	BYTE							cbTimeOutCard;						//����ʱ��
	BYTE							cbTimeCallScore;					//�з�ʱ��
	BYTE							cbTimeStartGame;					//��ʼʱ��
	BYTE							cbTimeHeadOutCard;					//�׳�ʱ��

	//��ʷ����
	SCORE							lTurnScore[GAME_PLAYER];			//������Ϣ
	SCORE							lCollectScore[GAME_PLAYER];			//������Ϣ
};

//�з�״̬
struct CMD_S_StatusCall
{
	//ʱ����Ϣ
	BYTE							cbTimeOutCard;						//����ʱ��
	BYTE							cbTimeCallScore;					//�з�ʱ��
	BYTE							cbTimeStartGame;					//��ʼʱ��
	BYTE							cbTimeHeadOutCard;					//�׳�ʱ��

	//��Ϸ��Ϣ
	LONG							lCellScore;							//��Ԫ����
	WORD							wCurrentUser;						//��ǰ���
	BYTE							cbBankerScore;						//ׯ�ҽз�
	BYTE							cbScoreInfo[GAME_PLAYER];			//�з���Ϣ
	BYTE							cbHandCardData[NORMAL_COUNT];		//�����˿�

	//��ʷ����
	SCORE							lTurnScore[GAME_PLAYER];			//������Ϣ
	SCORE							lCollectScore[GAME_PLAYER];			//������Ϣ
};

//��Ϸ״̬
struct CMD_S_StatusPlay
{
	//ʱ����Ϣ
	BYTE							cbTimeOutCard;						//����ʱ��
	BYTE							cbTimeCallScore;					//�з�ʱ��
	BYTE							cbTimeStartGame;					//��ʼʱ��
	BYTE							cbTimeHeadOutCard;					//�׳�ʱ��

	//��Ϸ����
	LONG							lCellScore;							//��Ԫ����
	BYTE							cbBombCount;						//ը������
	WORD							wBankerUser;						//ׯ���û�
	WORD							wCurrentUser;						//��ǰ���
	BYTE							cbBankerScore;						//ׯ�ҽз�

	//������Ϣ
	WORD							wTurnWiner;							//ʤ�����
	BYTE							cbTurnCardCount;					//������Ŀ
	BYTE							cbTurnCardData[MAX_COUNT];			//��������

	//�˿���Ϣ
	BYTE							cbBankerCard[3];					//��Ϸ����
	BYTE							cbHandCardData[MAX_COUNT];			//�����˿�
	BYTE							cbHandCardCount[GAME_PLAYER];		//�˿���Ŀ

	//��ʷ����
	SCORE							lTurnScore[GAME_PLAYER];			//������Ϣ
	SCORE							lCollectScore[GAME_PLAYER];			//������Ϣ
};

//////////////////////////////////////////////////////////////////////////////////
//�����

#define SUB_S_GAME_START			100									//��Ϸ��ʼ
#define SUB_S_CALL_SCORE			101									//�û��з�
#define SUB_S_BANKER_INFO			102									//ׯ����Ϣ
#define SUB_S_OUT_CARD				103									//�û�����
#define SUB_S_PASS_CARD				104									//�û�����
#define SUB_S_GAME_CONCLUDE			105									//��Ϸ����
#define SUB_S_SET_BASESCORE			106									//���û���

//�����˿�
struct CMD_S_GameStart
{
	WORD							wStartUser;							//��ʼ���
	WORD				 			wCurrentUser;						//��ǰ���
	BYTE							cbValidCardData;					//�����˿�
	BYTE							cbValidCardIndex;					//����λ��
	BYTE							cbCardData[NORMAL_COUNT];			//�˿��б�
};

//�������˿�
struct CMD_S_AndroidCard
{
	BYTE							cbHandCard[GAME_PLAYER][NORMAL_COUNT];//�����˿�
	WORD							wCurrentUser;						//��ǰ���
};

//�û��з�
struct CMD_S_CallScore
{
	WORD				 			wCurrentUser;						//��ǰ���
	WORD							wCallScoreUser;						//�з����
	BYTE							cbCurrentScore;						//��ǰ�з�
	BYTE							cbUserCallScore;					//�ϴνз�
};

//ׯ����Ϣ
struct CMD_S_BankerInfo
{
	WORD				 			wBankerUser;						//ׯ�����
	WORD				 			wCurrentUser;						//��ǰ���
	BYTE							cbBankerScore;						//ׯ�ҽз�
	BYTE							cbBankerCard[3];					//ׯ���˿�
};

//�û�����
struct CMD_S_OutCard
{
	BYTE							cbCardCount;						//������Ŀ
	WORD				 			wCurrentUser;						//��ǰ���
	WORD							wOutCardUser;						//�������
	BYTE							cbCardData[MAX_COUNT];				//�˿��б�
};

//��������
struct CMD_S_PassCard
{
	BYTE							cbTurnOver;							//һ�ֽ���
	WORD				 			wCurrentUser;						//��ǰ���
	WORD				 			wPassCardUser;						//�������
};

//��Ϸ����
struct CMD_S_GameConclude
{
	//���ֱ���
	LONG							lCellScore;							//��Ԫ����
	SCORE							lGameScore[GAME_PLAYER];			//��Ϸ����

	//�����־
	BYTE							bChunTian;							//�����־
	BYTE							bFanChunTian;						//�����־

	//ը����Ϣ
	BYTE							cbBombCount;						//ը������
	BYTE							cbEachBombCount[GAME_PLAYER];		//ը������

	//��Ϸ��Ϣ
	BYTE							cbBankerScore;						//�з���Ŀ
	BYTE							cbCardCount[GAME_PLAYER];			//�˿���Ŀ
	BYTE							cbHandCardData[FULL_COUNT];			//�˿��б�
};

//////////////////////////////////////////////////////////////////////////////////
//�����

#define SUB_C_CALL_SCORE			1									//�û��з�
#define SUB_C_OUT_CARD				2									//�û�����
#define SUB_C_PASS_CARD				3									//�û�����

//�û��з�
struct CMD_C_CallScore
{
	BYTE							cbCallScore;						//�з���Ŀ
};

//�û�����
struct CMD_C_OutCard
{
	BYTE							cbCardCount;						//������Ŀ
	BYTE							cbCardData[MAX_COUNT];				//�˿�����
};

//////////////////////////////////////////////////////////////////////////////////

#pragma pack()


//////////////////////////////////////////////////////////////////////////

//��������
#define ST_ORDER					0									//��С����
#define ST_COUNT					1									//��Ŀ����
#define ST_CUSTOM					2									//�Զ�����

//////////////////////////////////////////////////////////////////////////

//�����ṹ
struct tagAnalyseResult
{
	BYTE 							cbBlockCount[4];					//�˿���Ŀ
	BYTE							cbCardData[4][MAX_COUNT];			//�˿�����
};

//���ƽ��
struct tagOutCardResult
{
	BYTE							cbCardCount;						//�˿���Ŀ
	BYTE							cbResultCard[MAX_COUNT];			//����˿�
};

//�ֲ���Ϣ
struct tagDistributing
{
	BYTE							cbCardCount;						//�˿���Ŀ
	BYTE							cbDistributing[15][6];				//�ֲ���Ϣ
};

#define MAX_TYPE_COUNT 254
struct tagOutCardTypeResult 
{
	BYTE							cbCardType;							//�˿�����
	BYTE							cbCardTypeCount;					//������Ŀ
	BYTE							cbEachHandCardCount[MAX_TYPE_COUNT];//ÿ�ָ���
	BYTE							cbCardData[MAX_TYPE_COUNT][MAX_COUNT];//�˿�����
};

//////////////////////////////////////////////////////////////////////////

//��Ϸ�߼���
class ddz_ai_logic
{
	//��������
protected:
	static const BYTE				m_cbCardData[FULL_COUNT];			//�˿�����

	//AI����
public:
	static const BYTE				m_cbGoodcardData[GOOD_CARD_COUTN];	//��������
	BYTE							m_cbAllCardData[GAME_PLAYER][MAX_COUNT];//�����˿�
	BYTE							m_cbLandScoreCardData[MAX_COUNT];	//�����˿�
	BYTE							m_cbUserCardCount[GAME_PLAYER];		//�˿���Ŀ
	WORD							m_wBankerUser;						//�������

	//��������
public:
	//���캯��
	ddz_ai_logic();
	//��������
	virtual ~ddz_ai_logic();

	//���ͺ���
public:
	//��ȡ����
	BYTE GetCardType(const BYTE cbCardData[], BYTE cbCardCount);
	//��ȡ��ֵ
	BYTE GetCardValue(BYTE cbCardData) { return cbCardData&MASK_VALUE; }
	//��ȡ��ɫ
	BYTE GetCardColor(BYTE cbCardData) { return cbCardData&MASK_COLOR; }

	//���ƺ���
public:
	//�����˿�
	VOID RandCardList(BYTE cbCardBuffer[], BYTE cbBufferCount);
	//�����˿�
	VOID SortCardList(BYTE cbCardData[], BYTE cbCardCount, BYTE cbSortType);
	//ɾ���˿�
	bool RemoveCard(const BYTE cbRemoveCard[], BYTE cbRemoveCount, BYTE cbCardData[], BYTE cbCardCount);

	//�߼�����
public:
	//��Ч�ж�
	bool IsValidCard(BYTE cbCardData);
	//�߼���ֵ
	BYTE GetCardLogicValue(BYTE cbCardData);
	//�Ա��˿�
	bool CompareCard(const BYTE cbFirstCard[], const BYTE cbNextCard[], BYTE cbFirstCount, BYTE cbNextCount);
	//��������
	bool SearchOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, const BYTE cbTurnCardData[], BYTE cbTurnCardCount, tagOutCardResult & OutCardResult);

	//�ڲ�����
public:
	//�����˿�
	BYTE MakeCardData(BYTE cbValueIndex, BYTE cbColorIndex);
	//�����˿�
	VOID AnalysebCardData(const BYTE cbCardData[], BYTE cbCardCount, tagAnalyseResult & AnalyseResult);
	//�����ֲ�
	VOID AnalysebDistributing(const BYTE cbCardData[], BYTE cbCardCount, tagDistributing & Distributing);



	//////////////////////////////////////////////////////////////////////////
	//AI����

	//���ú���
public:
	//�����˿�
	VOID SetUserCard(WORD wChairID, BYTE cbCardData[], BYTE cbCardCount) ;
	//���õ���
	VOID SetBackCard(WORD wChairID, BYTE cbBackCardData[], BYTE cbCardCount) ;
	//����ׯ��
	VOID SetBanker(WORD wBanker) ;
	//�����˿�
	VOID SetLandScoreCardData(BYTE cbCardData[], BYTE cbCardCount) ;
	//ɾ���˿�
	VOID RemoveUserCardData(WORD wChairID, BYTE cbRemoveCardData[], BYTE cbRemoveCardCount) ;

	//��������
public:
	//����㷨
	VOID Combination(BYTE cbCombineCardData[], BYTE cbResComLen,  BYTE cbResultCardData[254][5], BYTE &cbResCardLen,BYTE cbSrcCardData[] , BYTE cbCombineLen1, BYTE cbSrcLen, const BYTE cbCombineLen2);
	//�����㷨
	VOID Permutation(BYTE *list, INT m, INT n, BYTE result[][4], BYTE &len) ;
	//����ը��
	VOID GetAllBomCard(BYTE const cbHandCardData[], BYTE const cbHandCardCount, BYTE cbBomCardData[], BYTE &cbBomCardCount);
	//����˳��
	VOID GetAllLineCard(BYTE const cbHandCardData[], BYTE const cbHandCardCount, BYTE cbLineCardData[], BYTE &cbLineCardCount);
	//��������
	VOID GetAllThreeCard(BYTE const cbHandCardData[], BYTE const cbHandCardCount, BYTE cbThreeCardData[], BYTE &cbThreeCardCount);
	//��������
	VOID GetAllDoubleCard(BYTE const cbHandCardData[], BYTE const cbHandCardCount, BYTE cbDoubleCardData[], BYTE &cbDoubleCardCount);
	//��������
	VOID GetAllSingleCard(BYTE const cbHandCardData[], BYTE const cbHandCardCount, BYTE cbSingleCardData[], BYTE &cbSingleCardCount);

	//��Ҫ����
public:
	//�������ͣ�����Ƶ��ã�
	VOID AnalyseOutCardType(BYTE const cbHandCardData[], BYTE const cbHandCardCount, BYTE const cbTurnCardData[], BYTE const cbTurnCardCount, tagOutCardTypeResult CardTypeResult[12+1]);
	//�������ƣ��ȳ��Ƶ��ã�
	VOID AnalyseOutCardType(BYTE const cbHandCardData[], BYTE const cbHandCardCount, tagOutCardTypeResult CardTypeResult[12+1]);
	//���Ƹ���
	BYTE AnalyseSinleCardCount(BYTE const cbHandCardData[], BYTE const cbHandCardCount, BYTE const cbWantOutCardData[], BYTE const cbWantOutCardCount, BYTE cbSingleCardData[]=NULL);

	//���ƺ���
public:
	//�������ƣ��ȳ��ƣ�
	VOID BankerOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, tagOutCardResult & OutCardResult) ;
	//�������ƣ�����ƣ�
	VOID BankerOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, WORD wOutCardUser, const BYTE cbTurnCardData[], BYTE cbTurnCardCount, tagOutCardResult & OutCardResult) ;
	//�����ϼң��ȳ��ƣ�
	VOID UpsideOfBankerOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, WORD wMeChairID,tagOutCardResult & OutCardResult) ;
	//�����ϼң�����ƣ�
	VOID UpsideOfBankerOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, WORD wOutCardUser,  const BYTE cbTurnCardData[], BYTE cbTurnCardCount, tagOutCardResult & OutCardResult) ;
	//�����¼ң��ȳ��ƣ�
	VOID UndersideOfBankerOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, WORD wMeChairID,tagOutCardResult & OutCardResult) ;
	//�����¼ң�����ƣ�
	VOID UndersideOfBankerOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, WORD wOutCardUser, const BYTE cbTurnCardData[], BYTE cbTurnCardCount, tagOutCardResult & OutCardResult) ;
	//��������
	bool SearchOutCard(const BYTE cbHandCardData[], BYTE cbHandCardCount, const BYTE cbTurnCardData[], BYTE cbTurnCardCount, WORD wOutCardUser, WORD wMeChairID, tagOutCardResult & OutCardResult);

	//�зֺ���
public:
	//�з��ж�
	BYTE LandScore(WORD wMeChairID, BYTE cbCurrentLandScore, BYTE landcards[]);

};

//////////////////////////////////////////////////////////////////////////

#endif
#include "EffectManager.h"
#include "EffectFactory.h"
#include "MyObject.h"

EffectMgr::EffectMgr()
{

}

EffectMgr::~EffectMgr()
{
	Clear();
}

void EffectMgr::Add(CEffect* pObj)
{
	if(pObj != NULL)
		m_effects.push_back(pObj);
}

void EffectMgr::Clear()
{
	obj_table_iter it = m_effects.begin();
	while(it != m_effects.end())
	{
		EffectFactory::GetInstance()->Recovery((*it)->GetEffectType(), *it);
		++it;
	}
	m_effects.clear();
}

int64_t  EffectMgr::Execute(MyObject* pTarget, std::list<MyObject*>& list, bool bPretreating){
	int64_t  Score = 0;
	MyObject* pOwner = GetOwner();
    // ���� Ҳ���Բ��ţ���Χ���붼�ǿ�list ��ѭ���ô�������Ƕ�ף�
	for (std::list<MyObject*>::iterator io = list.begin(); io != list.end();++io){
		if (pOwner == *io){
			return Score;
		}
	}

	//ֻҪ��û��������ִ��
	if(pOwner->GetState() < EOS_DEAD){
		list.push_back(pOwner);
		for (obj_table_iter it = m_effects.begin(); it != m_effects.end();++it){
			CEffect* eff = *it;
			Score += eff->Execute(pOwner, pTarget, list, bPretreating);
		}
	}

	return Score;
}

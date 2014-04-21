//AIStateMachine.h

#pragma once
#include <assert.h>
#include "AIStates.h"
#include "Stalker.h"

template <class entity_type>
class AIStateMachine
{
public:
	AIStateMachine(entity_type* owner) : m_pOwner(owner),
		m_pCurrentState(nullptr),
		m_pPreviousState(nullptr),
		m_pGlobalState(nullptr)
	{}

	void SetCurrentState(AIState<entity_type>* s)		{ m_pCurrentState = s; }
	void SetPreviousState(AIState<entity_type>* s)	{ m_pPreviousState = s; }
	void SetGlobalState(AIState<entity_type>* s)		{ m_pGlobalState = s; }

	void Update() const
	{
		//if (m_pGlobalState) m_pGlobalState->Execute(m_pOwner);

		if (m_pCurrentState) m_pCurrentState->Execute(m_pOwner);
	}
	void ChangeState(AIState<entity_type>* pNewState)
	{
		assert(pNewState && "<StateMachine::ChangeState>: trying to change to a nullptr State");
		m_pPreviousState = m_pCurrentState;
		m_pCurrentState->Exitstate(m_pOwner);
		m_pCurrentState = pNewState;
		m_pCurrentState->Enterstate(m_pOwner);
	}
	void RevertToPreviousState()
	{
		ChangeState(m_pPreviousState);
	}

	AIState<entity_type>* CurrentState() const { return m_pCurrentState; }
	AIState<entity_type>* PreviousState() const { return m_pPreviousState; }
	AIState<entity_type>* GlobalState() const { return m_pGlobalState; }

	bool isInState(const AIState<entity_type>& st)const;
private:
	entity_type* m_pOwner;
	AIState<entity_type>* m_pCurrentState;
	AIState<entity_type>* m_pPreviousState;
	AIState<entity_type>* m_pGlobalState;
};

//OnCollision()
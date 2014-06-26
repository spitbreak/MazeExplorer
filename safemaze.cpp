#include "safemaze.h"

SafeMaze::SafeMaze(IMazeInterface* pIMazeInterface) :
m_pIMazeInterface(pIMazeInterface),
m_ppMazeArch(0),
m_uiX(0),
m_uiY(0),
m_uiSleepInterval(3)
{

}

SafeMaze::~SafeMaze()
{
	delete m_ppMazeArch;
}

int SafeMaze::InitMaze(const unsigned int& uiX, const unsigned int& uiY)
{
	if(uiX < MIN_X || uiY < MIN_Y)
	{
		std::cout << "size error" << std::endl;
		return -1;
	}

	m_uiX = uiX;
	m_uiY = uiY;

	m_ppMazeArch = (char**)malloc(uiY * sizeof(char*));
	m_ppObjsPos = (long**)malloc(uiY * sizeof(long*));
	m_ppObjsMutex = (pthread_rwlock_t**)malloc(uiY * sizeof(pthread_rwlock_t*));	
	
	if(m_ppMazeArch == NULL || m_ppObjsPos == NULL || m_ppObjsMutex == NULL)
	{
		std::cout << "memory error" << std::endl;
		return -1;
	}

	if(m_pIMazeInterface->GenerateMaze(m_ppMazeArch, m_ppObjsPos, m_ppObjsMutex, uiX, uiY, 0))
	{
		std::cout << "memory error in generate maze" << std::endl;
		return -1;
	}

	m_ppMazeArch[0][0] = 0;
	m_ppMazeArch[m_uiY - 1][0] = 0;
	m_ppMazeArch[0][m_uiX - 1] = 0;
	m_ppMazeArch[m_uiY - 1][m_uiX - 1] = 0;
	return 0;
}

int SafeMaze::InitEmptyMaze(const unsigned int& uiX, const unsigned int& uiY)
{
	if(uiX < MIN_X || uiY < MIN_Y)
	{
		std::cout << "size error" << std::endl;
		return -1;
	}

	m_uiX = uiX;
	m_uiY = uiY;

	m_ppMazeArch = (char**)malloc(uiY * sizeof(char*));
	m_ppObjsPos = (long**)malloc(uiY * sizeof(long*));
	m_ppObjsMutex = (pthread_rwlock_t**)malloc(uiY * sizeof(pthread_rwlock_t*));	
	
	if(m_ppMazeArch == NULL || m_ppObjsPos == NULL || m_ppObjsMutex == NULL)
	{
		std::cout << "memory error" << std::endl;
		return -1;
	}

	if(m_pIMazeInterface->GenerateEmptyMaze(m_ppMazeArch, m_ppObjsPos, m_ppObjsMutex, uiX, uiY, 0))
	{
		std::cout << "memory error in generate maze" << std::endl;
		return -1;
	}

	return 0;
}

int SafeMaze::StartExplore()
{
	for(int i = 0; i < 4; i ++)
	{
		if(m_szpExplorers[i] != NULL)
		{
			pthread_t tr;
			long buff[2];
			buff[0] = (long)this;
			buff[1] = i;
//			char szpBuff[sizeof(SafeMaze) + 4];
//			memcpy(szpBuff, this, sizeof(SafeMaze));
//			memcpy(szpBuff + sizeof(SafeMaze), &i, sizeof(i));
			pthread_create(&tr,NULL, ExplrThrd, buff);
		}
	}

	return 0;
}

void SafeMaze::Display()
{
	m_pIMazeInterface->DisplayMaze(m_ppMazeArch, m_ppObjsPos, m_uiX, m_uiY);
}

int SafeMaze::SetExplorer(const int& idx, Explorer* pExplorer)
{
	if(idx < 0 || idx >= 4)
	{
		return 1;
	}
	else if(m_szpExplorers[idx] != 0)
	{
		return 2;
	}
	
	m_szpExplorers[idx] = pExplorer;
	switch(idx)
	{
	case 0:
		m_ppObjsPos[0][0] = (long)pExplorer;
		pExplorer->SetCurX(0);
		pExplorer->SetCurY(0);
		break;
	case 1:
		m_ppObjsPos[0][m_uiX - 1] = (long)pExplorer;
		pExplorer->SetCurX((int)(m_uiX - 1));
		pExplorer->SetCurY(0);
		break;
	case 2:
		m_ppObjsPos[m_uiY - 1][0] = (long)pExplorer;
		pExplorer->SetCurX(0);
		pExplorer->SetCurY((int)(m_uiY - 1));
		break;
	case 3:
		m_ppObjsPos[m_uiY - 1][m_uiX - 1] = (long)pExplorer;
		pExplorer->SetCurX((int)(m_uiX - 1));
		pExplorer->SetCurY((int)(m_uiY - 1));
		break;
	}
	return 0;
}

int SafeMaze::SetExplorer(const int& idx, Explorer* pExplr, const unsigned int& uiX, const unsigned int& uiY)
{
	if(idx < 0 || idx >= 4)
	{
		return 1;
	}
	else if(m_szpExplorers[idx] != 0)
	{
		return 2;
	}

	if(IsPosInMaze(uiX, uiY))
	{
		m_szpExplorers[idx] = pExplr;
		pExplr->SetCurX(uiX);
		pExplr->SetCurY(uiY);
		m_ppObjsPos[uiY][uiX] = (long)pExplr;
		return 0;
	}
	return 3;
}

void* SafeMaze::ExplrThrd(void* param)
{
	srand(time(NULL));
	long* lpArgv = (long*)param;
	SafeMaze* pMaze = (SafeMaze*)(lpArgv[0]);
	int iExplrNo = (int)(lpArgv[1]);
//	int 3 = *(int*)((char*)param + sizeof(SafeMaze));

	while(1)
	{
		sleep(pMaze->m_uiSleepInterval);
		unsigned int uiDestX = 0;
		unsigned int uiDestY = 0;
		const unsigned int iPrevX = pMaze->m_szpExplorers[iExplrNo]->CurX();
		const unsigned int iPrevY = pMaze->m_szpExplorers[iExplrNo]->CurY();
		if(pMaze->m_szpExplorers[iExplrNo]->Walk(uiDestX, uiDestY))
		{
			continue;
		}

		switch(pMaze->MoveExplorer(uiDestX, uiDestY, pMaze->m_szpExplorers[iExplrNo]))
		{
		case 0:
			std::cout << iExplrNo << " explorer, x: " << iPrevX << " y: " << iPrevY << " ==> x: " << uiDestX << " y: " << uiDestY << " success." << std::endl;
			std::cout << "CUR POSITION" << std::endl;
			pMaze->Display();
			break;
		case 1:
			std::cout << iExplrNo << " explorer, x: " << iPrevX << " y: " << iPrevY << " ==> x: " << uiDestX << " y: " << uiDestY << " failed. maze pos can't be access" << std::endl;
			break;
		case 2:
			std::cout << iExplrNo << " explorer, x: " << iPrevX << " y: " << iPrevY << " ==> x: " << uiDestX << " y: " << uiDestY << " failed. pos occupied when explore want read pos status." << std::endl;
			break;
		case 3:
			std::cout << iExplrNo << " explorer, x: " << iPrevX << " y: " << iPrevY << " ==> x: " << uiDestX << " y: " << uiDestY << " failed. pos occupied when explore want to move." << std::endl;
			break;
		}
	}

	return NULL;
}

int SafeMaze::MoveExplorer(const unsigned int& uiX, const unsigned int& uiY, Explorer* pExplr)
{
	if(IsPosInMaze(uiX, uiY))
	{
		pthread_rwlock_rdlock(&m_ppObjsMutex[uiY][uiX]);
		if(m_ppObjsPos[uiY][uiX])
		{
			pthread_rwlock_unlock(&m_ppObjsMutex[uiY][uiX]);
			return 2;
		}
		pthread_rwlock_unlock(&m_ppObjsMutex[uiY][uiX]);

		pthread_rwlock_wrlock(&m_ppObjsMutex[uiY][uiX]);
		pthread_rwlock_wrlock(&m_ppObjsMutex[pExplr->CurY()][pExplr->CurX()]);
		if(m_ppObjsPos[uiY][uiX] != 0)
		{
			pthread_rwlock_unlock(&m_ppObjsMutex[uiY][uiX]);
			pthread_rwlock_unlock(&m_ppObjsMutex[pExplr->CurY()][pExplr->CurX()]);
			return 3;
		}

		int iPrevX = pExplr->CurX();
		int iPrevY = pExplr->CurY();
		m_ppObjsPos[uiY][uiX] = (long)pExplr;
		m_ppObjsPos[iPrevY][iPrevX] = 0;
		pExplr->SetCurX(uiX);
		pExplr->SetCurY(uiY);
		pthread_rwlock_unlock(&m_ppObjsMutex[uiY][uiX]);
		pthread_rwlock_unlock(&m_ppObjsMutex[iPrevY][iPrevX]);
		return 0;
	}
	return 1;
}

bool SafeMaze::IsPosInMaze(const unsigned int& uiX, const unsigned int& uiY)
{
	return ((uiX >= 0 && uiX <= m_uiX - 1) && (uiY >= 0 && uiY <= m_uiY - 1) && m_ppMazeArch[uiY][uiX] == 0);
}

bool SafeMaze::TestMazeValIsBinary()
{
	for(unsigned int i = 0; i < m_uiX; i++)
	{
		for(unsigned int j = 0; j < m_uiY; j++)
		{
			if(m_ppMazeArch[j][i] != 1 && m_ppMazeArch[j][i] != 0)
			{
				std::cout << "Do not pass binary test" << std::endl;
				return false;
			}
		}
	}

	std::cout << "Binary test passed" << std::endl;
	return true;
}

bool SafeMaze::TestMazeCornValIsZero()
{
	return m_ppMazeArch[0][0] == 0 && 
		   m_ppMazeArch[m_uiY - 1][0] == 0 &&
		   m_ppMazeArch[0][m_uiX - 1] == 0 &&
		   m_ppMazeArch[m_uiY - 1][m_uiX - 1] == 0;
}

bool SafeMaze::TestObjsPosAllZero()
{
	for(unsigned int i = 0; i < m_uiX; i++)
	{
		for(unsigned int j = 0; j < m_uiY; j++)
		{
			if(m_ppObjsPos[j][i] != 0)
			{
				std::cout << "x: " << i << " y: " << j << " is not zero" << std::endl;
				return false;
			}
		}
	}
	return true;
} 

bool SafeMaze::TestMazeExplrNull()
{
	for(int i = 0; i < 4; i ++)
	{
		if(m_szpExplorers[i] != NULL)
		{
			std::cout << "i: " << i << "not null" << std::endl;
			return false;
		}
	}
	return true;
}

/**
bool SafeMaze::TestMutex()
{
	for(unsigned int i = 0; i < m_uiY; i++)
	{
		for(unsigned int j = 0; j < m_uiX; j++)
			pthread_mutex_lock(&m_ppObjsMutex[i][j]);
	}

	int ret = 0;
	for(unsigned int i = 0; i < m_uiY; i++)
	{
		for(unsigned int j = 0; j < m_uiX; j++)
		{
			ret = pthread_mutex_trylock(&m_ppObjsMutex[i][j]);
			if(ret)
				continue;
			else
				break;
		}
		if(ret == 0)
			break;
	}

	for(unsigned int i = 0; i < m_uiY; i++)
	{
		for(unsigned int j = 0; j < m_uiX; j++)
		{
			pthread_mutex_unlock(&m_ppObjsMutex[i][j]);
		}
	}
	return ret;
}
**/

bool SafeMaze::TestExplrInPos()
{
	bool ret = true;
	for(int i = 0; i < 4; i++)
	{
		if(m_szpExplorers[i])
		{
			switch(i)
			{
			case 0:
				ret = (m_szpExplorers[i]->CurX() == 0) && 
					  (m_szpExplorers[i]->CurY() == 0) && 
					  ((long)m_szpExplorers[i] == m_ppObjsPos[0][0]);
				break;		
			case 1:
				ret = (m_szpExplorers[i]->CurX() == (int)(m_uiX - 1)) && 
					  (m_szpExplorers[i]->CurY() == 0) && 
        			  ((long)m_szpExplorers[i] == m_ppObjsPos[0][m_uiX - 1]);
				break;
			case 2:
				ret = (m_szpExplorers[i]->CurX() == 0) && 
					  (m_szpExplorers[i]->CurY() == (int)(m_uiY - 1)) &&
					  ((long)m_szpExplorers[i] == m_ppObjsPos[m_uiY - 1][0]);
				break;
			case 3:
				ret = (m_szpExplorers[i]->CurX() == (long)(m_uiX - 1)) && 
					  (m_szpExplorers[i]->CurY() == (int)(m_uiY - 1)) &&
					  ((long)m_szpExplorers[i] == m_ppObjsPos[m_uiY - 1][m_uiX - 1]);
				break;
			}
		}

		if(!ret)
			break;
	}
	return ret;
}

bool SafeMaze::TestIsPosInMaze(const unsigned int& uiX, const unsigned int& uiY)
{
	return IsPosInMaze(uiX, uiY);
}
/**
void SafeMaze::GenerateMaze()
{

}

void SafeMaze::ChangeArch()
{

}

void SafeMaze::PutMonsterIn()
{

}
**/

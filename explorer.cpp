#include "explorer.h"


Explorer::Explorer() :
m_iCurX(-1),
m_iCurY(-1)
{

}

Explorer::~Explorer()
{
	std::cout << "explorer destroyed" << std::endl;
}

int Explorer::Walk(unsigned int& uiDestX, unsigned int& uiDestY)
{
	int b = (int)(log(E_Directors - 1) / log(2)) + 1;
	int c = random() % b;
	int d = pow(2, c);
	EDirector eDirector = (EDirector)d;
//	EDirector eDirector = (EDirector)pow(random() % (int)(log((double)(E_Directors - 1)) / log(2.0)), 2);
	uiDestX = m_iCurX;
	uiDestY = m_iCurY;
	switch(eDirector)
	{
	case E_Up:
		uiDestY -= 1;
		break;
	case E_Down:
		uiDestY += 1;
		break;
	case E_Left:
		uiDestX -= 1;
		break;
	case E_Right:
		uiDestX += 1;
		break;
	default:
		return 1;
	}
	return 0;
}

int Explorer::Walk(unsigned int& uiDestX, unsigned int& uiDestY, const EDirector& eDirector)
{
	uiDestX = m_iCurX;
	uiDestY = m_iCurY;
	switch(eDirector)
	{
	case E_Up:
		uiDestY -= 1;
		break;
	case E_Down:
		uiDestY += 1;
		break;
	case E_Left:
		uiDestX -= 1;
		break;
	case E_Right:
		uiDestX += 1;
		break;
	default:
		return 1;
	}
	return 0;
}

void Explorer::AddPath(const unsigned int& uiX, const unsigned int& uiY)
{

}

bool Explorer::IsPosInPath(const unsigned int& uiX, const unsigned int& uiY)
{
	std::vector<Position>::iterator it_start = m_Path.begin();
	while(it_start != m_Path.end())
	{
		Position pos = *it_start;
		if(pos.m_uiX == uiX && pos.m_uiY == uiY)
		{
			return true;
		}
		it_start++;
	}
	return false;
}

int Explorer::ActivatePower(AbsPower* pPower)
{
	pPower->ActivateInMaze(0);
	return 0;
}

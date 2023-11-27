#pragma once

float GetHypotenuseFromCenter(const D3DXVECTOR3& Point, LPDIRECT3DDEVICE9 pDevice)
{
	float PositionX = 0;
	float PositionY = 0;
	float ScreenCenterX, ScreenCenterY;
	D3DXVECTOR3 Screen;

	HWND hWindow = Overlay::GetInstance()->targetWindow;

	ScreenCenterX = GetWindowSize(hWindow).x / 2;
	ScreenCenterY = GetWindowSize(hWindow).y / 2;

	if (ProjectToScreen(Point, &Screen, pDevice) == FALSE)
		return 9999.0f;

	PositionX = Screen.x > ScreenCenterX ? Screen.x - ScreenCenterX : ScreenCenterX - Screen.x;
	PositionY = Screen.y > ScreenCenterY ? Screen.y - ScreenCenterY : ScreenCenterY - Screen.y;

	return sqrt(PositionX * PositionX + PositionY * PositionY);
}

DWORD GetAimKey()
{
	switch (Function[22])
	{
	case 0:
		return 0x02;
		break;
	case 1:
		return 0x01;
		break;
	}
}

int GetAimHeight()
{
	switch (Function[23])
	{
	case 0:
		return Bip01_Head;
		break;
	case 1:
		return Bip01_Spine2;
		break;
	}
}

void AimAtPos(float x, float y, LPDIRECT3DDEVICE9 pDevice)
{
	HWND hWindow = Overlay::GetInstance()->targetWindow;

	DWORD ScreenCenterX = GetWindowSize(hWindow).x / 2;
	DWORD ScreenCenterY = GetWindowSize(hWindow).y / 2;

	float AimSpeed;

	float TargetX = 0;
	float TargetY = 0;

	if (Function[24] == 0)
		AimSpeed = 4;
	else if (Function[24] == 1)
		AimSpeed = 2;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}
	if (y != 0)
	{
		if (y > ScreenCenterX)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterX)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	if (x && y && GetAsyncKeyState(GetAimKey()))
	{
		mouse_event(MOUSEEVENTF_MOVE, (DWORD)TargetX, (DWORD)TargetY, NULL, NULL);
	}
}

PObject_Player* GetTargetByCrosshair()
{
	PObject_Player* pEntity = NULL;
	float fAngleLength = FLT_MAX;


	if (pGameWorld && pGameWorld->bInited)
	{
		for (int i = 0; i < pGameWorld->getMaxObjects(); i++)
		{
			GameObject* object = pGameWorld->getObject(i);

			if (object == NULL || object == (GameObject*)localPlayer)
				continue;// Nope

			if (object && object->getObjType() == OBJTYPE_HUMAN || object && object->getObjType() == OBJTYPE_HUMAN1)
			{
				PObject_Player* pTarget = (PObject_Player*)object;

				if (pTarget)
				{
					D3DXVECTOR3 lPos, vHead;

					pGetBonePos(pTarget, GetAimHeight(), &vHead);

					float AngleLength = GetHypotenuseFromCenter(vHead, Overlay::GetInstance()->pDevice);

					if (AngleLength < fAngleLength) // Nearer than last nearest enemy
					{
						pEntity = pTarget;
						fAngleLength = AngleLength;
					}
				}
			}
		}
	}
	return pEntity;
}

PObject_Player* GetNextTarget(void)
{
	return GetTargetByCrosshair();
}

void AimBone()
{
	PObject_Player* pEntity = NULL;
	pEntity = GetNextTarget();

	if (!pEntity)
		return;

	D3DXVECTOR3 lPos, vHead;

	pGetBonePos(pEntity, GetAimHeight(), &vHead);

	D3DXVECTOR3 screenPos;

	if (ProjectToScreen(vHead, &screenPos, Overlay::GetInstance()->pDevice))
	{
		if (Function[20])
		{
			AimAtPos(screenPos.x, screenPos.y, Overlay::GetInstance()->pDevice);

			if (Function[21])
			{
				PrintText(screenPos.x, screenPos.y, red, "***");
			}
		}
	}
}
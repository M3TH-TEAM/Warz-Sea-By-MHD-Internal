#pragma once

obj_Player* localPlayer = NULL;


static ClientGameLogic* pGetGame()
{
	ClientGameLogic* pGamePlus = (ClientGameLogic*)(*(DWORD*)((DWORD)hGameBase + pGetGame_Adr));

	pGamePlus = (ClientGameLogic*)((DWORD)pGamePlus ^ 0x0);

	return pGamePlus;
}

static ObjectManager* GameWorld()
{
	ObjectManager* pGamePlus = (ObjectManager*)(*(DWORD*)((DWORD)hGameBase + pGameWorld_Adr));

	pGamePlus = (ObjectManager*)((DWORD)pGamePlus ^ 0x0);

	return pGamePlus;
}

static tRender* Render1()
{
	return (tRender*)(*(DWORD*)((DWORD)hGameBase + dwRenderAddress));
}

void Engine()
{
	GetRender = (tRender)((DWORD)Render1);
	pRenderer = GetRender();

	while (pGame == 0)
	{
		GetGame = (tGetGame)((DWORD)pGetGame);
		pGame = GetGame();

	}

	while (pGameWorld == 0)
	{
		GetGameWorld = (tGetGameWorld)((DWORD)GameWorld);
		pGameWorld = GetGameWorld();

	}
}
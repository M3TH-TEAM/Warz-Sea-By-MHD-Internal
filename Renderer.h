class r3dRenderLayer* pRenderer = NULL;
class ClientGameLogic* pGame = NULL;
class ObjectManager* pGameWorld = NULL;

#include "AllOffset.h"
#include "Class.h"
#include "Call.h"
#include "DirectK.h"

int Skeleton;
int Aimbot;
int Aimbotx2;

void BeginScene(int a);
void DrawScene();
void EndScene(int A);

LPD3DXFONT pFont;

void DrawRect(IDirect3DDevice9* m_pD3Ddev, float x, float y, float w, float h, D3DCOLOR Color)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	};
	Vertex qV[4] = {
		{ (float)x, (float)(y + h), 0.0f, 1.0f, Color },
		{ (float)x, (float)y, 0.0f, 1.0f, Color },
		{ (float)(x + w), (float)(y + h), 0.0f, 1.0f, Color },
		{ (float)(x + w), (float)y, 0.0f, 1.0f, Color }
	};
	const DWORD D3DFVF_TL = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	m_pD3Ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pD3Ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pD3Ddev->SetFVF(D3DFVF_TL);
	m_pD3Ddev->SetTexture(0, NULL);
	m_pD3Ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, qV, sizeof(Vertex));
}

bool m_bIsInitalized = false;

void PrintText(int x, int y, DWORD color, const char *fmt, ...)
{
	char draw[99] = "";
	va_list va_alist;
	va_start(va_alist, fmt);
	RECT Rectangle = { x, y, 1000 + x, 1000 + y };
	_vsnprintf_s(draw + strlen(draw), 512, 511, fmt, va_alist);
	va_end(va_alist);
	// menu->drawText(x,y, r,g,b,    draw);
	pFont->DrawText(NULL, draw, -1, &Rectangle, 0, color);
}

bool ComparePattern(DWORD address, DWORD patternAddress, char * mask) {
	int patternLen = strlen(mask);

	for (auto i = 1; i < patternLen; i++) {
		if (mask[i] != *"?" && *(char*)(address + i) != *(char*)(patternAddress + i)) {  // Compare each byte of the pattern with each byte after the current scanning address
			return false;
		}
	}
	if (address != patternAddress) {  // Make sure we aren't returning a match for the pattern defined within your DLLMain
		return true;
	}
	return false;
}

DWORD FindPattern(DWORD patternAddress, char * mask) {
	SYSTEM_INFO sysInfo; // Holds System Information
	GetSystemInfo(&sysInfo);
	DWORD procMin = (DWORD)sysInfo.lpMinimumApplicationAddress;  // Minimum memory address of process
	DWORD procMax = (DWORD)sysInfo.lpMaximumApplicationAddress;  // Maximum memory address of process

	MEMORY_BASIC_INFORMATION mBI, mBINext;

	DWORD firstOldProtect = NULL;
	DWORD secondOldProtect = NULL;

	DWORD patternSize = (DWORD)strlen(mask);

	std::vector<DWORD> matches;  // Holds all pattern matches

	while (procMin < procMax) {  // While still scanning memory

		VirtualQueryEx(GetCurrentProcess(), (LPVOID)procMin, &mBI, sizeof(MEMORY_BASIC_INFORMATION));  // Get memory page details

		if ((mBI.State == MEM_COMMIT) && (mBI.Type == MEM_PRIVATE)) {  // Check state of current page

			VirtualProtect((LPVOID)procMin, mBI.RegionSize, PAGE_EXECUTE_READWRITE, &firstOldProtect);  // Set page to read/write/execute

			for (auto n = (DWORD)mBI.BaseAddress; n < (DWORD)mBI.BaseAddress + mBI.RegionSize; n += 0x01) {  // For each byte in this page

				if (n + patternSize > procMax) {  // If our pattern will extend past the maximum memory address, break
					break;
				}

				if (*(char*)n == (*(char*)patternAddress)) {  // If first byte of pattern matches current byte

					if (n + patternSize < (UINT)mBI.BaseAddress + mBI.RegionSize) {  // If entire length of pattern is within this page

						if (ComparePattern((DWORD)n, patternAddress, mask)) {  // Test if full pattern matches
							matches.push_back((DWORD)n);  // If it does, add it to the vector
						}
					}
					else {  // If it isn't within the same page
						VirtualQueryEx(GetCurrentProcess(), (LPVOID)(procMin + mBI.RegionSize), &mBINext, sizeof(MEMORY_BASIC_INFORMATION));  // Same memory page stuff with next page

						if ((mBINext.State == MEM_COMMIT) && (mBINext.Type == MEM_PRIVATE)) {
							VirtualProtect((LPVOID)(procMin + mBI.RegionSize), mBINext.RegionSize, PAGE_EXECUTE_READWRITE, &secondOldProtect);

							if (ComparePattern((DWORD)n, patternAddress, mask)) {
								matches.push_back((DWORD)n);

							}
						}

					}

				}
			}


			VirtualProtect((LPVOID)procMin, mBI.RegionSize, firstOldProtect, NULL);  // Reset memory page state of first page

			if (secondOldProtect) {  // If we scanned into the second page
				VirtualProtect((LPVOID)procMin, mBINext.RegionSize, secondOldProtect, NULL);  // Reset memory page state of second page
				secondOldProtect = NULL;
			}
		}
		procMin += mBI.RegionSize;  // Start scanning next page
	}


	if (!matches.empty()) {
		return matches[0];  // If we had some matches, return the first. -- Change this and return type of functon if you need full list of matches
	}
	else {
		return NULL;  // Return NULL if no matches
	}
}

#include "Menu.h"


void PickupDraw()
{
	if (!m_bIsInitalized)
	{
		Engine();
		D3DXCreateFontA(Overlay::GetInstance()->pDevice, 14, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &pFont);
		m_bIsInitalized = true;
	}
	Overlay::GetInstance()->pDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
	Overlay::GetInstance()->pDevice->BeginScene();

	if (GetForegroundWindow() == Overlay::GetInstance()->targetWindow)
	{
		__try
		{
			BeginScene(NULL);
			DrawScene();
			EndScene(NULL);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			ResumeThread(GetCurrentThread());
		}
	}

	Overlay::GetInstance()->pDevice->EndScene();
	Overlay::GetInstance()->pDevice->Present(0, 0, 0, 0);
}

D3DXVECTOR3 pGetBonePos(PObject_Player* pPlayer, DWORD i, D3DXVECTOR3* Pos)
{
	if (!pPlayer || !pPlayer->m_Skeleton())
		return D3DXVECTOR3(0, 0, 0);

	if (!pPlayer->m_Skeleton()->BonePtr)
		return D3DXVECTOR3(0, 0, 0);

	D3DXMATRIX v6, v7, BoneMatrix;

	D3DXMatrixRotationYawPitchRoll(&v6, 0.0, 1.570796251296997, 0.0);
	BoneMatrix = pPlayer->m_Skeleton()->BonePtr->pBones[i].BoneMatrix;
	D3DXMatrixMultiply(&BoneMatrix, &BoneMatrix, &pPlayer->m_BoneCoeff());
	D3DXMatrixMultiply(&v7, &v6, &BoneMatrix);

	Pos->x = v7._41;
	Pos->y = v7._42;
	Pos->z = v7._43;

	return v7;
}

D3DXVECTOR3 GetBoneOrigin(PObject_Player* pPlayer, int iBoneID)
{
	if (!pPlayer != NULL || !pPlayer->m_Skeleton() != NULL)
		return D3DXVECTOR3(0, 0, 0);

	if (!pPlayer->m_Skeleton()->BonePtr != NULL)
		return D3DXVECTOR3(0, 0, 0);

	D3DXMATRIX v6;
	D3DXMATRIX v7;
	D3DXMATRIX BoneMatrix;
	D3DXVECTOR3 Pos;

	D3DXMatrixRotationYawPitchRoll(&v6, 0.0, 1.570796251296997, 0.0);
	BoneMatrix = pPlayer->m_Skeleton()->BonePtr->pBones[iBoneID].BoneMatrix;
	D3DXMatrixMultiply(&BoneMatrix, &BoneMatrix, &pPlayer->m_BoneCoeff());
	D3DXMatrixMultiply(&v7, &v6, &BoneMatrix);

	Pos.x = v7._41;
	Pos.y = v7._42;
	Pos.z = v7._43;

	return Pos;
}

void DrawSkeleton(PObject_Player* player, D3DCOLOR dwColour, LPDIRECT3DDEVICE9 pDevice)
{
	int aSkeleton[][2] = {
	{ 1, 2 },{ 2, 3 },{ 3, 4 },{ 4, 5 },{ 5, 6 },
	{ 5, 7 },{ 7, 8 },{ 8, 9 },{ 9, 10 },{ 10, 11 },
	{ 11, 12 },{ 12, 13 },{ 10, 14 },{ 14, 15 },{ 15, 16 },
	{ 10,17 },{ 17,18 },{ 18,19 },{ 10,20 },{ 20,21 },{ 21,22 },
	{ 10,23 },{ 23,24 },{ 24,25 },
	{ 5, 28 },{ 28, 29 },{ 29, 30 },
	{ 30, 31 },{ 31, 32 },{ 31, 35 },{ 35, 36 },
	{ 36, 37 },{ 31, 38 },{ 38, 39 },{ 39, 40 },{ 31, 41 },
	{ 31, 42 },{ 42, 43 },{ 31, 44 },{ 44, 45 },{ 45, 46 },
	{ 53, 54 },{ 54, 55 },{ 55, 56 },
	{ 58, 59 },{ 59, 60 },{ 60, 61 },
	{ 53, 58 },
	};

	for (int i = 0; i < 48; ++i)
	{
		D3DXVECTOR3 Bone1;
		D3DXVECTOR3 Bone2;
		pGetBonePos(player, aSkeleton[i][0], &Bone1);
		pGetBonePos(player, aSkeleton[i][1], &Bone2);
		D3DXVECTOR3 Out1, Out2;

		ProjectToScreen(Bone1, &Out1, pDevice);
		ProjectToScreen(Bone2, &Out2, pDevice);
		DrawLine(pDevice, Out1.x, Out1.y, Out2.x, Out2.y, dwColour);
	}
}

#include < Map >


class cItems {
public:
	const char* name;
	D3DCOLOR typeColor;
	int type;

	cItems()
	{
	};

	cItems(const char* name, D3DCOLOR typeColor, int type)
	{
		this->name = name;
		this->typeColor = typeColor;
		this->type = type;
	}
};



static enum eTypes { Unknown1, Throwable };
static cItems getObjectFromId(int ID)
{
	std::map <int, cItems> items;
	bool init = false;

	if (!init) {
		init = !init;

		items[101306] = cItems("Flashlight", White, Unknown1);
		items[101307] = cItems("Hammer", White, Unknown1);
		items[101309] = cItems("Pickaxe", White, Unknown1);
		items[101267] = cItems("Tactical Knife", White, Unknown1);
		items[101278] = cItems("Bat", White, Unknown1);
		items[101313] = cItems("Spiked Bat", White, Unknown1);
		items[101314] = cItems("Metal Bat", White, Unknown1);
		items[101335] = cItems("Kandy Kane", White, Unknown1);
		items[101336] = cItems("Katana", White, Unknown1);
		items[101339] = cItems("Machete", White, Unknown1);
		items[101344] = cItems("Canoe Paddle", White, Unknown1);
		items[101385] = cItems("Garden Shears", White, Unknown1);
		items[101386] = cItems("Golf Club", White, Unknown1);
		items[101345] = cItems("Cricket Bat", White, Unknown1);
		items[101382] = cItems("Crowbar", White, Unknown1);
		items[101391] = cItems("Wrench", White, Unknown1);
		items[101383] = cItems("Fire Axe", White, Unknown1);
		items[101381] = cItems("Butterfly Knif", White, Unknown1);
		items[101343] = cItems("Brass Knuckles", White, Unknown1);
		items[101346] = cItems("Shovel", White, Unknown1);
		items[101338] = cItems("Wakizashi", White, Unknown1);
		items[101337] = cItems("Jokoto Katan", White, Unknown1);
		items[101308] = cItems("Hatchet", White, Unknown1);
		items[101389] = cItems("Police Baton", White, Unknown1);
		items[101386] = cItems("Police Bato", White, Unknown1);	//old?
		items[101388] = cItems("Pitchfork", White, Unknown1);
		items[101384] = cItems("Frying Pan", White, Unknown1);
		items[101390] = cItems("Power Drill", White, Unknown1);
		items[101068] = cItems("SVD", Purple, Unknown1);
		items[101084] = cItems("VSS", Purple, Unknown1);
		items[101085] = cItems("Mauser", Purple, Unknown1);
		items[101087] = cItems("AWM", Purple, Unknown1);
		items[101088] = cItems("M107", Purple, Unknown1);
		items[101217] = cItems("MAURSER DESERT", Purple, Unknown1);
		items[101247] = cItems("BLASER R93", Purple, Unknown1);
		items[101002] = cItems("M16", Red, Unknown1);
		items[101332] = cItems("Kruger .22 Rifle", Red, Unknown1);
		items[101005] = cItems("G36", Red, Unknown1);
		items[101022] = cItems("AK47M", Red, Unknown1);
		items[101032] = cItems("AKM", Red, Unknown1);
		items[101035] = cItems("AKS74U", Red, Unknown1);
		items[101040] = cItems("M4 SEMI", Red, Unknown1);
		items[101055] = cItems("M4", Red, Unknown1);
		items[101172] = cItems("SIG556 ASR", Red, Unknown1);
		items[101173] = cItems("IMITAR21", Red, Unknown1);
		items[101106] = cItems("HONEYBADGER", Red, Unknown1);
		items[101169] = cItems("MASADA", Red, Unknown1);
		items[101193] = cItems("FN SCAR", Red, Unknown1);
		items[101210] = cItems("FNSCARNS", Red, Unknown1);
		items[101334] = cItems("KRUGER MINI14", Red, Unknown1);
		items[101060] = cItems("PKM", Red, Unknown1);
		items[101093] = cItems("RPK", Red, Unknown1);
		items[101095] = cItems("FN M249", Red, Unknown1);
		items[101197] = cItems("RA H23", Red, Unknown1);
		items[101103] = cItems("MP5", Red, Unknown1);
		items[101107] = cItems("P90", Red, Unknown1);
		items[101108] = cItems("EVO3", Red, Unknown1);
		items[101109] = cItems("BIZON", Red, Unknown1);
		items[101063] = cItems("MP7", Red, Unknown1);
		items[101064] = cItems("UZI", Red, Unknown1);
		items[101201] = cItems("SR1 VERSEK", Red, Unknown1);
		items[101246] = cItems("P90S", Red, Unknown1);
		items[101004] = cItems("FN Five7", Red, Unknown1);
		items[101111] = cItems("B92", Red, Unknown1);
		items[101112] = cItems("B93R", Red, Unknown1);
		items[101115] = cItems("JERICHO", Red, Unknown1);
		items[101120] = cItems("SIG P226", Red, Unknown1);
		items[101180] = cItems("DESERT EAGLE", Red, Unknown1);
		items[101224] = cItems("STI EAGLE ELITE", Red, Unknown1);
		items[101331] = cItems("ANACONDA", Red, Unknown1);
		items[101330] = cItems("KRUGER .22", Red, Unknown1);
		items[101098] = cItems("SAIGA", Red, Unknown1);
		items[101158] = cItems("MOSSBERG", Red, Unknown1);
		items[101183] = cItems("KTDECIDER", Red, Unknown1);
		items[101200] = cItems("AA-12", Red, Unknown1);
		items[101321] = cItems("DOUBLE BARREL", Red, Unknown1);
		items[101322] = cItems("CROSS BOW", Red, Unknown1);
		items[101320] = cItems("FLARE GUN", Red, Unknown1);
		items[101310] = cItems("Frag Grenade", Red, Unknown1);
		items[101392] = cItems("Nail Gun", Red, Unknown1);
		items[101341] = cItems("Mosin Rifle", Red, Unknown1);
		items[101342] = cItems("1911", Red, Unknown1);
		items[800011] = cItems("FAMAS", Red, Unknown1);
		items[800030] = cItems("QBZ", Red, Unknown1);
		items[800032] = cItems("PECHENEG", Red, Unknown1);
		items[800027] = cItems("LS 90", Red, Unknown1);
		items[700001] = cItems("SCAR WORLD", Red, Unknown1);
		items[101256] = cItems("Antibiotics", White, Unknown1);
		items[101261] = cItems("Bandages", White, Unknown1);
		items[101262] = cItems("Bandages DX", White, Unknown1);
		items[101300] = cItems("Pain killers", White, Unknown1);
		items[101301] = cItems("Zombie Repellent", White, Unknown1);
		items[101302] = cItems("C01-Vaccine", White, Unknown1);
		items[101303] = cItems("C04-Vaccine", White, Unknown1);
		items[101304] = cItems("Medkit", White, Unknown1);
		items[101399] = cItems("Repair Kit", White, Unknown1);
		items[101312] = cItems("Flare", White, Unknown1);
		items[101305] = cItems("Time Capsule", White, Unknown1);
		items[101324] = cItems("Sandbag Barricade", White, Unknown1);
		items[101316] = cItems("Barb Wire Barricade", White, Unknown1);
		items[101317] = cItems("Wood Shield Barricade", White, Unknown1);
		items[800066] = cItems("Desert Riot", White, Unknown1);
		items[101318] = cItems("Riot Shield", White, Unknown1);
		items[101323] = cItems("Air Horn", White, Unknown1);
		items[101315] = cItems("Binoculars", Green, Unknown1);
		items[101319] = cItems("Range Finder", Green, Unknown1);
		items[400000] = cItems("Forward Grip", Green, Unknown1);
		items[400001] = cItems("5.45 AK 30", Green, Unknown1);
		items[400003] = cItems("ACOG", Green, Unknown1);
		items[400004] = cItems("Rifle Laser", Green, Unknown1);
		items[400005] = cItems("Holographic", Green, Unknown1);
		items[400006] = cItems("SCAR IS", Green, Unknown1);
		items[400007] = cItems("Kobra", Green, Unknown1);
		items[400008] = cItems("Tactical Sniper Scope", Green, Unknown1);
		items[400009] = cItems("SMG Grip", Green, Unknown1);
		items[400010] = cItems("STANAG 60", Green, Unknown1);
		items[400012] = cItems("Flash Hider", Green, Unknown1);
		items[400013] = cItems("Silencer", Green, Unknown1);
		items[400015] = cItems("STANAG 45", Green, Unknown1);
		items[400016] = cItems("STANAG 30", Green, Unknown1);
		items[400017] = cItems("STANAG C-Mag", Green, Unknown1);
		items[400018] = cItems("Rifle Flashlight", Green, Unknown1);
		items[400019] = cItems("Compact Scope", Green, Unknown1);
		items[400020] = cItems("Red Dot SP", Green, Unknown1);
		items[400021] = cItems("Pistol laser", Green, Unknown1);
		items[400022] = cItems("Pistol Flashlight", Green, Unknown1);
		items[400023] = cItems("Reflex Sight", Green, Unknown1);
		items[400024] = cItems("M4 IS", Green, Unknown1);
		items[400025] = cItems("SIG 556", Green, Unknown1);
		items[400026] = cItems("MP7 IS", Green, Unknown1);
		items[400027] = cItems("PSO-1", Green, Unknown1);
		items[400029] = cItems("G36 ammo", Green, Unknown1);
		items[400030] = cItems("VSS-20", Green, Unknown1);
		items[400031] = cItems("VSS-10", Green, Unknown1);
		items[400032] = cItems("MP7 40", Green, Unknown1);
		items[400033] = cItems("MP7 30", Green, Unknown1);
		items[400034] = cItems("9x19 Para Mag", Green, Unknown1);
		items[400035] = cItems("M249 IS", Green, Unknown1);
		items[400036] = cItems("KT IS", Green, Unknown1);
		items[400038] = cItems("Blackwater Long Range", Green, Unknown1);
		items[400039] = cItems("Swiss Arms Scope 8x", Green, Unknown1);
		items[400040] = cItems("Iron AK74M", Green, Unknown1);
		items[400042] = cItems("Iron G36", Green, Unknown1);
		items[400043] = cItems("AWM .338 Magnum ammo", Green, Unknown1);
		items[400046] = cItems("P90 50 rounds", Green, Unknown1);
		items[400047] = cItems("Bizon 64 ammo", Green, Unknown1);
		items[400048] = cItems("SVD ammo", Green, Unknown1);
		items[400049] = cItems("CZ Scorpion EVO-3 ammo", Green, Unknown1);
		items[400050] = cItems("AA-12 Drum", Green, Unknown1);
		items[400051] = cItems("EVO-3 IS", Green, Unknown1);
		items[400052] = cItems("Bizon IS", Green, Unknown1);
		items[400053] = cItems("USS-12 IS", Green, Unknown1);
		items[400054] = cItems("P90 IS", Green, Unknown1);
		items[400055] = cItems("Pecheneg IS", Green, Unknown1);
		items[400056] = cItems("PKM IS", Green, Unknown1);
		items[400058] = cItems("SIG516 IS", Green, Unknown1);
		items[400059] = cItems("TAR21 IS", Green, Unknown1);
		items[400060] = cItems("RPK IS", Green, Unknown1);
		items[400061] = cItems("RPO IS", Green, Unknown1);
		items[400062] = cItems("AN94 IS", Green, Unknown1);
		items[400065] = cItems("AT4 IS", Green, Unknown1);
		items[400066] = cItems("M4 IS2", Green, Unknown1);
		items[400069] = cItems("5.45 AK 45", Green, Unknown1);
		items[400070] = cItems(".308 Winchester Sniper rifle ammo", Green, Unknown1);
		items[400071] = cItems("9mm Mag", Green, Unknown1);
		items[400073] = cItems("Saiga 10 ammo", Green, Unknown1);
		items[400074] = cItems("standard muzzle", Green, Unknown1);
		items[400079] = cItems("MP5 10mm Mag", Green, Unknown1);
		items[400080] = cItems("SAIGA IS", Green, Unknown1);
		items[400081] = cItems("XA5 IS", Green, Unknown1);
		items[400082] = cItems("M590 Is", Green, Unknown1);
		items[400083] = cItems("Veresk IS", Green, Unknown1);
		items[400084] = cItems("SMG-20 ammo", Green, Unknown1);
		items[400085] = cItems("SMG-40 ammo", Green, Unknown1);
		items[400086] = cItems("Desert Eagle ammo", Green, Unknown1);
		items[400087] = cItems("5.7 FN M240 Mag", Green, Unknown1);
		items[400088] = cItems("SMG Grip 1", Green, Unknown1);
		items[400099] = cItems("G36 C-Mag", Green, Unknown1);
		items[400100] = cItems("5.45 AK Drum", Green, Unknown1);
		items[400101] = cItems("7.62 AKM clip", Green, Unknown1);
		items[400119] = cItems("MASADA IS", Green, Unknown1);
		items[400121] = cItems("USS-12 IS", Green, Unknown1);
		items[400127] = cItems("M16 IS", Green, Unknown1);
		items[400128] = cItems("AKM IS", Green, Unknown1);
		items[400129] = cItems("AKS IS", Green, Unknown1);
		items[400133] = cItems(".50 BMG", Green, Unknown1);
		items[400134] = cItems("UZI IS", Green, Unknown1);
		items[400135] = cItems(".45 ACP STI Eagle Elite ammo", Green, Unknown1);
		items[400136] = cItems("12 Gauge Ammo", Green, Unknown1);
		items[400137] = cItems("2x 12gauge", Green, Unknown1);
		items[400139] = cItems("Arrow Explosive", Green, Unknown1);
		items[400140] = cItems("Arrow", Green, Unknown1);
		items[400141] = cItems("Shotgun shell 2x", Green, Unknown1);
		items[400142] = cItems("Shotgun shell 8x", Green, Unknown1);
		items[400143] = cItems("M249 Ammo Box", Green, Unknown1);
		items[400144] = cItems(".22 Caliber Clip", Green, Unknown1);
		items[400145] = cItems("Anaconda clip", Green, Unknown1);
		items[400146] = cItems("Large Kruger Rifle Clip", Green, Unknown1);
		items[400147] = cItems("Medium Kruger Rifle Clip", Green, Unknown1);
		items[400148] = cItems("Stardard Kruger .22 Rifle Clip", Green, Unknown1);
		items[400149] = cItems("Kruger Rifle IS", Green, Unknown1);
		items[400150] = cItems("Kruger Mini-14 clip", Green, Unknown1);
		items[400151] = cItems("Kruger Mini-14 IS", Green, Unknown1);
		items[400154] = cItems(".40 Caliber 1911MAG", Green, Unknown1);
		items[400153] = cItems("Standart Mosin Magazine", Green, Unknown1);
		items[400152] = cItems("Flare Clip", Green, Unknown1);
		items[400157] = cItems("Nail Strip", Green, Unknown1);
		items[20015] = cItems("Custom Guerilla", Grey, Unknown1);
		items[20054] = cItems("IBA Sand", Grey, Unknown1);
		items[20056] = cItems("MTV Forest", Grey, Unknown1);
		items[20059] = cItems("Light Gear Forest", Grey, Unknown1);
		items[800014] = cItems("Midium VDM", Grey, Unknown1);
		items[20175] = cItems("Medium Backpack", Grey, Unknown1);
		items[20176] = cItems("Small Backpack", Grey, Unknown1);
		items[20179] = cItems("Large Backpack", Grey, Unknown1);
		items[20180] = cItems("Military Ruck", Grey, Unknown1);
		items[20181] = cItems("Teddy Bear backpack", Grey, Unknown1);
		items[20185] = cItems("ALICE Rucksack", Grey, Unknown1);
		items[20196] = cItems("GameSpot BackPack", Grey, Unknown1);
		items[20200] = cItems("Santa's Sack", Grey, Unknown1);
		items[20006] = cItems("K.Style Helmet", Grey, Unknown1);
		items[20022] = cItems("Beret Cover", Grey, Unknown1);
		items[20023] = cItems("Boonie Cover", Grey, Unknown1);
		items[20025] = cItems("Shadow", Grey, Unknown1);
		items[20032] = cItems("Black Mask", Grey, Unknown1);
		items[20033] = cItems("Clown Mask", Grey, Unknown1);
		items[20034] = cItems("Jason Mask", Grey, Unknown1);
		items[20035] = cItems("Skull Mask", Grey, Unknown1);
		items[20036] = cItems("Slash Mask", Grey, Unknown1);
		items[20041] = cItems("Boonie Desert", Grey, Unknown1);
		items[20042] = cItems("Boonie Green", Grey, Unknown1);
		items[20043] = cItems("M9 helmet black", Grey, Unknown1);
		items[20046] = cItems("M9 Helmet with Goggles", Grey, Unknown1);
		items[20047] = cItems("M9 Helmet Green", Grey, Unknown1);
		items[20048] = cItems("M9 Helmet Urban", Grey, Unknown1);
		items[20049] = cItems("M9 Helmet Forest 1", Grey, Unknown1);
		items[20050] = cItems("M9 Helmet Goggles 1", Grey, Unknown1);
		items[20067] = cItems("NVG Goggles", Grey, Unknown1);
		items[20096] = cItems("Boonie Hat Leather", Grey, Unknown1);
		items[20097] = cItems("Fireman Helmet", Grey, Unknown1);
		items[20098] = cItems("Hard Hat", Grey, Unknown1);
		items[20177] = cItems("Gas Mask", Grey, Unknown1);
		items[20178] = cItems("Gas Mask 2", Grey, Unknown1);
		items[20187] = cItems("Night vision military", Grey, Unknown1);
		items[20188] = cItems("Night vision civilian", Grey, Unknown1);
		items[20192] = cItems("Helloween Special!", Grey, Unknown1);
		items[20197] = cItems("Christmas Special", Grey, Unknown1);
		items[20198] = cItems("Santa's Lil Helper", Grey, Unknown1);
		items[20199] = cItems("Captain Jack Frost", Grey, Unknown1);
		items[20211] = cItems("Dakotaz Hat", Grey, Unknown1);
		items[101286] = cItems("Coconut Water", White, Unknown1);
		items[101294] = cItems("Energy drink", White, Unknown1);
		items[101295] = cItems("Electro-AID", White, Unknown1);
		items[101296] = cItems("Can of soda", White, Unknown1);
		items[101297] = cItems("Juice", White, Unknown1);
		items[101298] = cItems("Water 1L", White, Unknown1);
		items[101299] = cItems("Water 375ml", White, Unknown1);
		items[101283] = cItems("Bag of Chips", White, Unknown1);
		items[101284] = cItems("Bag MRE", White, Unknown1);
		items[101285] = cItems("Instant Oatmeal", White, Unknown1);
		items[101288] = cItems("Chocolate Bar", White, Unknown1);
		items[101290] = cItems("Can of Pasta", White, Unknown1);
		items[101291] = cItems("Can of Soup", White, Unknown1);
		items[101292] = cItems("Can of Ham", White, Unknown1);
		items[101293] = cItems("Can of Tuna", White, Unknown1);
		items[101340] = cItems("MiniSaints", White, Unknown1);
		items[101289] = cItems("Granola Bar", White, Unknown1);
		items[101340] = cItems("Chemlight White", White, Unknown1);
		items[101340] = cItems("Flare", White, Unknown1);
		items[101340] = cItems("Chemlight Blue", White, Unknown1);
		items[101340] = cItems("Chemlight Green", White, Unknown1);
		items[101340] = cItems("Chemlight Orange", White, Unknown1);
		items[101340] = cItems("Chemlight Red", White, Unknown1);
		items[101340] = cItems("Chemlight Yellow", White, Unknown1);
		items[301370] = cItems("Rope", White, Unknown1);
		items[301319] = cItems("Duct Tape", White, Unknown1);
		items[301357] = cItems("Charcoal", White, Unknown1);
		items[301320] = cItems("Empty Bottle", White, Unknown1);
		items[101408] = cItems("Tool", White, Unknown1);
		items[301321] = cItems("Gasoline", White, Unknown1);
		items[301366] = cItems("Ointment", White, Unknown1);
		items[301335] = cItems("Scissors", White, Unknown1);
		items[301318] = cItems("Empty Can", White, Unknown1);
		items[301319] = cItems("Duct Tape", White, Unknown1);
		items[301324] = cItems("Gun Powder", White, Unknown1);
		items[301320] = cItems("Empty Bottle", White, Unknown1);
		items[301328] = cItems("Nails", White, Unknown1);
		items[301331] = cItems("Rag", White, Unknown1);
		items[301327] = cItems("Metal Scrap", White, Unknown1);
		items[301355] = cItems("Broom", White, Unknown1);
		items[301332] = cItems("Barbed Wire", White, Unknown1);
		items[0x474F4C44] = cItems("Money", White, Unknown1);
		items[0x4C4F4F54] = cItems("Mystery Crate", Purple, Unknown1);
		items[1280266066] = cItems("Mystery Crate", Purple, Unknown1);
	
	}
	if (items.count(ID) > 0)
	{
		return items[ID];
	}
	else
	{
		return cItems("_ITEM_", D3DCOLOR_ARGB(255, 255, 255, 255), Throwable);
	}
}

void BeginScene(int a)
{

}


void LoopHack()
{
	if (Function[40])
	{
		if (GetAsyncKeyState('G') & 1)
		{
			*(BOOL*)((DWORD)pGame + Disconnect) = 0;
		}
	}

	if (Function[41])
	{
		//*(float*)(GetloaclplayerZZ() + PickupIteam_off) = 1.2;

		float instantp = *(FLOAT*)((DWORD)localPlayer + PickupIteam_off);
		if (instantp >= 0.2f)
		{
			*(FLOAT*)((DWORD)localPlayer + PickupIteam_off) = 1.1f;
		}

	}

	if (Function[42])
	{
		if (GetAsyncKeyState(VK_SPACE))
		{
			*(float*)((DWORD)localPlayer + Superjumping) = 13;  //17A4
		}
	}

	if (Function[43])
	{
		*(float*)((DWORD)localPlayer + Stamina_offset) = 75;
	}
}

#include "Aim.h"

void RenderObjects()
{
	char txt[99];
	if (pGameWorld && pGameWorld->bInited)
	{
		for (int i = 0; i < pGameWorld->getMaxObjects(); i++)
		{
			GameObject* object = pGameWorld->getObject(i);

			if (object == NULL || object == (GameObject*)localPlayer)
				continue;// Nope

			D3DXVECTOR3 headVec, screen, box, screenHead, vTargetDistance,XfootVec,XheadVec;
			if (object && object->getObjType() == OBJTYPE_HUMAN || object && object->getObjType() == OBJTYPE_HUMAN1)
			{
				PObject_Player* player = (PObject_Player*)object;

				if (player)
				{
					D3DXVec3Subtract(&vTargetDistance, &player->Position(), &localPlayer->Position());
					float flDistance = D3DXVec3Length(&vTargetDistance);
					DWORD flHealth = *(DWORD*)((DWORD)player + GetHealth_off);
				
					pGetBonePos(player, Bip01_L_Foot, &XfootVec);
					pGetBonePos(player, Bip01_Head, &XheadVec);

					if (ProjectToScreen(XfootVec, &screen, Overlay::GetInstance()->pDevice))
					{
						if (ProjectToScreen(XheadVec, &screenHead, Overlay::GetInstance()->pDevice))
						{
							if (Function[10])
							{
								box.y = ((screen.y - screenHead.y) > 1.f) ? screen.y - screenHead.y : 1.f;
								box.x = box.y / 2.75f;

								DrawLine(Overlay::GetInstance()->pDevice, screen.x - (box.x / 2), screen.y - box.y, screen.x - (box.x / 2) + box.x, screen.y - box.y, red);
								DrawLine(Overlay::GetInstance()->pDevice, screen.x - (box.x / 2) + box.x, screen.y - box.y, screen.x - (box.x / 2) + box.x, screen.y - box.y + box.y, red);
								DrawLine(Overlay::GetInstance()->pDevice, screen.x - (box.x / 2) + box.x, screen.y - box.y + box.y, screen.x - (box.x / 2), screen.y - box.y + box.y, red);
								DrawLine(Overlay::GetInstance()->pDevice, screen.x - (box.x / 2), screen.y - box.y + box.y, screen.x - (box.x / 2), screen.y - box.y, red);
							}
						}

						if (Function[11])
						{
							DrawSkeleton(player, red, Overlay::GetInstance()->pDevice);
						}

						if (Function[12])
						{
							float phealth;
							memcpy(&phealth, &flHealth, sizeof(flHealth));
							if (phealth > 100)
								phealth = 100;

							PrintText(screen.x, screen.y + 10, red, "%.f", phealth);
						}

						if (Function[13])
						{
							PrintText(screen.x, screen.y + 20, blue, "[%0.1f]", flDistance);
						}

						//PrintText(screen.x, screen.y, red, "Player");
					}
				}
			}

			if (object && object->getObjType() == OBJTYPE_ZOMBIE1)
			{
				CObject_Zombie* zombie = (CObject_Zombie*)object;

				if (VALID(zombie))
				{
					D3DXVECTOR3 ZombieScreen;

					if (ProjectToScreen(zombie->Position(), &ZombieScreen, Overlay::GetInstance()->pDevice))
					{
						if (Function[3])
						{
							PrintText(ZombieScreen.x, ZombieScreen.y, orange, "Zombies");
						}
					}
				}
			}

			if (object && object->getObjType() == OBJTYPE_DroppedItem)
			{
				obj_DroppedItem* item = (obj_DroppedItem*)(object);
				int itemID = item->ItemId();
				cItems itemz = getObjectFromId(item->ItemId());

				if (VALID(item))
				{
					D3DXVECTOR3 ItemScreen;

					if (ProjectToScreen(item->Position(), &ItemScreen, Overlay::GetInstance()->pDevice))
					{
						if (Function[30])
						{
							if (itemz.type == Unknown1, Throwable)
							{
								char txt[1024] = { '\0' };
								sprintf(txt, "%s", itemz.name);
								PrintText(ItemScreen.x, ItemScreen.y, Green, txt);
							}
						}
					}
				}
			}
		}
	}
}

void DrawScene()
{
	D3DDEVICE_CREATION_PARAMETERS d3dcp;
	Overlay::GetInstance()->pDevice->GetCreationParameters(&d3dcp);

	/*SetWindowDisplayAffinity(d3dcp.hFocusWindow, 0x11);*/

	RenderMenu(Overlay::GetInstance()->pDevice);

	localPlayer = pGame->GetLocalPlayer();

	if (localPlayer)
	{
		MatrixToScreen();
		RenderObjects();
		AimBone();

		LoopHack();
	}

	if (Function[1])
	{
		D3DVIEWPORT9 viewPort;
		Overlay::GetInstance()->pDevice->GetViewport(&viewPort);

		static DWORD center_x = (float)viewPort.Width / 2;
		static DWORD center_y = (float)viewPort.Height / 2;
		DrawLine(Overlay::GetInstance()->pDevice, center_x - 4, center_y, center_x + 5, center_y, red);
		DrawLine(Overlay::GetInstance()->pDevice, center_x, center_y - 4, center_x, center_y + 5, red);
	}
}

void EndScene(int a)
{

}
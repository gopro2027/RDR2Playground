/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#include "pch.h"
#include <string>
#include <vector>

#pragma region shittyCodeOrig

DWORD	vehUpdateTime;
DWORD	pedUpdateTime;

struct text_box_t
{
	std::string text;
	float x, y;
	byte r, g, b, a;
};

void entity_draw_info_add(std::vector<text_box_t> &textOnScreen, Entity entity, float mindist, float maxdist, std::string type, byte r, byte g, byte b, byte a)
{
	Vector3 v = ENTITY::GET_ENTITY_COORDS(entity, TRUE, FALSE);
	float x, y;
	if (GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(v.x, v.y, v.z, &x, &y))
	{
		// draw bounds
		if (x < 0.01 || y < 0.01 || x > 0.93 || y > 0.93)
			return;
		// get coords
		Vector3 plv = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE, FALSE);
		float dist = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(plv.x, plv.y, plv.z, v.x, v.y, v.z, TRUE);
		// draw text if entity isn't close to the player
		if (dist > mindist && dist < maxdist)
		{
			// check if the text fits on screen
			bool bFitsOnscreen = true;
			for each (auto &iter in textOnScreen)
			{
				float textDist = sqrtf((iter.x - x)*(iter.x - x) + (iter.y - y)*(iter.y - y));
				if (textDist < 0.05)
				{
					bFitsOnscreen = false;
					break;
				}
			}
			// if text doesn't fit then skip draw
			if (!bFitsOnscreen) return;
			// add info to the vector
			int health = ENTITY::GET_ENTITY_HEALTH(entity);
			Hash model = ENTITY::GET_ENTITY_MODEL(entity);
			char text[256];
			sprintf_s(text, "^\n| %s %08X\n| Distance %.02f\n| Health %d", type.c_str(), model, dist, health);
			textOnScreen.push_back({ text, x, y, r, g, b, a });
		}
	}
}













void update()
{
	// player
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	// check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player))
		return;

	// there are lots of entities in some places so we need to
	// remove possibilty of text being drawn on top of another text
	// thats why we will check distance between text on screen
	std::vector<text_box_t> textOnScreen;

	// get all vehicles
	const int ARR_SIZE = 1024;
	Vehicle vehicles[ARR_SIZE];
	int count = worldGetAllVehicles(vehicles, ARR_SIZE);

	// let's track all existing vehicles
	for (int i = 0; i < count; i++)
		entity_draw_info_add(textOnScreen, vehicles[i], 15, 9999, "V", 75, 75, 75, 75);

	/*	
	// delete all vehicles
	for (int i = 0; i < count; i++)
	{		
		if (!ENTITY::IS_ENTITY_A_MISSION_ENTITY(vehicles[i]))
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(vehicles[i], TRUE, TRUE);
		VEHICLE::DELETE_VEHICLE(&vehicles[i]);
	}
	*/

	// get all peds
	Ped peds[ARR_SIZE];
	count = worldGetAllPeds(peds, ARR_SIZE);

	// let's track all animals
	for (int i = 0; i < count; i++)
	{
		if (PED::IS_PED_HUMAN(peds[i]))
			continue;
		entity_draw_info_add(textOnScreen, peds[i], 15, 9999, "A", 75, 110, 75, 75);
	}

	// get all objects
	Object objects[ARR_SIZE];
	count = worldGetAllObjects(objects, ARR_SIZE);

	// track all near objects
	for (int i = 0; i < count; i++)
		entity_draw_info_add(textOnScreen, objects[i], 2, 200, "O", 75, 75, 110, 75);

	// get all pickups
	Pickup pickups[ARR_SIZE];
	count = worldGetAllPickups(pickups, ARR_SIZE);

	// track all pickups
	for (int i = 0; i < count; i++)
		entity_draw_info_add(textOnScreen, pickups[i], 2, 9999, "P", 75, 75, 75, 75);

	// draw
	for each (auto &iter in textOnScreen)
	{
		UI::SET_TEXT_SCALE(0.2, 0.2);
		UI::SET_TEXT_COLOR_RGBA(255, 255, 255, 255);
		UI::SET_TEXT_CENTRE(0);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		UI::DRAW_TEXT(GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", const_cast<char *>(iter.text.c_str())), iter.x, iter.y);
		// box
		GRAPHICS::DRAW_RECT(iter.x + 0.028f, iter.y + 0.033f, 0.058f, 0.041f, iter.r, iter.g, iter.b, iter.a, 0, 0);
	}
}

#pragma endregion








// dllmain.cpp : Script entry point

#include "pch.h"
#include "SimpleMenu.h"
#include "NativeUtil.h"

static std::wstring GetModulePath(HMODULE module)
{
	DWORD size = MAX_PATH;
	std::vector<wchar_t> buffer(size);

	do
	{
		buffer.resize(size);
		GetModuleFileNameW(module, buffer.data(), size);
		size *= 1.5;
	} while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	std::wstring modulePath = std::wstring(buffer.begin(), buffer.end());

	size_t slashPos = modulePath.size();
	for (int i = modulePath.size() - 1; i >= 0; --i)
	{
		if (modulePath[i] == L'/' || modulePath[i] == L'\\') {
			slashPos = i;
			break;
		}
	}

	std::wstring moduleDir = modulePath.substr(0, slashPos);
	return moduleDir;
}







int entityToIndex(void *entityAddress);



Noclip noclipResource;
SimpleMenu simpleMenu(0.7, 0.2);

int consolePrintCount = 0;
bool shouldPrintConsole = true;
void consolePrint(char *text) {
	if (shouldPrintConsole)
		DrawGameText(0.00f, 0.00 + 0.025f * consolePrintCount, text, 255, 255, 255, 255, 0.35f, 0.35f);
	consolePrintCount++;
}
void consolePrintReset() {
	consolePrintCount = 0;
}

int printTime = 0;
char *printText;
void printloop() {
	if (printTime > 0) {
		printTime--;
		//DrawGameText(0.05,0.05,printText,255,255,255,255,1,1);
		consolePrint(printText);
	}
}
void print(char *text, int time) {
	printText = text;
	printTime = time;
}



//48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 54 41 55 41 56 41 57 48 8D 68 D1 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 44 0F B7 E2 4C 8B F9 BA ? ? ? ? 4D 8B E9 41 8B F0 48 8B 88 ? ? ? ? E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 48 8B 7D 57 48 8B CF E8 ? ? ? ? 8B 0F 66 89 44 24 ? 81 E1 ? ? ? ? 0F B7 C0 0B C8 89 4C 24 54 48 8D 4C 24 ? E8 ? ? ? ? 48 8B D8 83 B8 ? ? ? ? ? 75 0D E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 80 64 24 ? ? 48 8B CB E8 ? ? ? ? 48 8B CF 8A D8 E8 ? ? ? ? 44 8A 75 77 4C 8D 4C 24 ? 84 DB 8B D0 48 8B 5D 67 44 8B C6 74 39 8A 8D ? ? ? ? 41 0F B7 D4 80 F1 01 88 4C 24 48 8A 4D 7F 88 4C 24 40 48 8B 4D 5F 44 88 74 24 ? 48 89 5C 24 ? 48 89 4C 24 ? 49 8B CF 89 44 24 20 E8 ? ? ? ? EB 2A 8A 45 7F 49 8B CF 88 44 24 40 48 8B 45 5F 44 88 74 24 ? 48 89 5C 24 ? 48 89 44 24 ? 89 54 24 20 41 0F B7 D4 E8 ? ? ? ? 48 8B F8 48 85 C0 0F 84 ? ? ? ? 8B 80 ? ? ? ? 41 BC ? ? ? ? 4C 8B 05 ? ? ? ? 41 23 C4 2B 05 ? ? ? ? 48 69 D0 ? ? ? ? 4A 8B 84 02 ? ? ? ? 48 8B C8 48 83 E1 FE 48 F7 D8 48 1B F6 48 23 F1 45 84 F6 74 1D 4A 8B 84 02 ? ? ? ? 48 8B C8 48 83 E1 FE 48 F7 D8 48 1B D2 48 23 D1 0F BA 72 ? ? 8B 95 ? ? ? ? 48 B8 ? ? ? ? ? ? ? ? 48 09 86 ? ? ? ? 48 8B CE E8 ? ? ? ? 83 BE ? ? ? ? ? 75 08 48 8B CE E8 ? ? ? ? 40 8A 75 6F 40 84 F6 74 6B 80 3D ? ? ? ? ? 74 62 44 8A 8D ? ? ? ? 48 8D 05 ? ? ? ? 45 0F B6 C6 48 8D 4C 24 ? 41 C1 E0 02 33 D2 48 89 44 24 ? 48 89 7D 87 E8 ? ? ? ? 84 C0 75 33 45 84 F6 75 2E 48 8B D7 49 8B CF E8 ? ? ? ? 33 C0 4C 8D 9C 24 ? ? ? ? 49 8B 5B 30 49 8B 73 38 49 8B 7B 40 49 8B E3 41 5F 41 5E 41 5D 41 5C 5D C3 0F BA B7 ? ? ? ? ? 48 8B CF 40 0F B6 C6 83 F0 01 C1 E0 0A 09 87 ? ? ? ? E8 ? ? ? ? 80 7D 6D 03 74 1B B8 ? ? ? ? 66 3B D8 74 11 41 B1 01 45 33 C0 48 8B D3 48 8B CF E8 ? ? ? ? 0F BA AF ? ? ? ? ? 48 8B CF C6 87 ? ? ? ? ? E8 ? ? ? ? 48 85 C0 74 0B B9 ? ? ? ? 66 39 48 10 72 08 48 8B CF E8 ? ? ? ? 41 F6 45 ? ? 75 16 48 8B 0D ? ? ? ? 48 8B D7 48 81 C1 ? ? ? ? E8 ? ? ? ? 8B 87 ? ? ? ? 41 23 C4 2B 05 ? ? ? ? 48 69 C8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 54 01 ? 48 8B C2 48 83 E0 FE 48 F7 DA 48 1B C9 48 23 C8 8B 05 ? ? ? ? 89 41 4C 8B 87 ? ? ? ? 41 23 C4 2B 05 ? ? ? ? 48 69 C8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 5C 01 ? E8 ? ? ? ? 48 85 C0 48 8B CB 0F 95 C2 48 83 E1 FE 48 F7 DB 48 1B C0 48 23 C1 48 8D 4D 97 88 50 50 48 8B D7 E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 75 0C E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 48 8D 55 97 45 33 C0 FF 50 08 48 8D 4D 97 E8 ? ? ? ? 8A 95 ? ? ? ? 4C 8D 05 ? ? ? ? 48 8B CF E8 ? ? ? ? 48 8B C7 E9 ? ? ? ? 

#define ui64 uint64_t
/*ui64(*CPedFactoryCreatePed_orig)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	print((char*)"Hook Ped Spawned",10);
	Log::Info << "Ped Spawn: " << *(ui64*)r3 << ", " << r4 << ", " << r5 << ", " << *(ui64*)r6 << ", " << *(ui64*)r7 << ", " << *(ui64*)r8 << ", " << r9 << ", " << r10 << ", " << r11 << ", " << r12 << ", " << r13 << ", " << r14 << Log::Endl;
	return CPedFactoryCreatePed_orig(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
}*/


#ifdef failedhookscreateped
ui64(*CPedFactoryCreatePed_orig0)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed0(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	print((char*)"Hook Called 0", 10);
	Log::Info << "Hook " << 0 << ": " << (ui64*)r3 << ", " << (ui64*)r4 << ", " << (ui64*)r5 << ", " << (ui64*)r6 << ", " << (ui64*)r7 << ", " << (ui64*)r8 << ", " << (ui64*)r9 << ", " << (ui64*)r10 << ", " << (ui64*)r11 << ", " << (ui64*)r12 << ", " << (ui64*)r13 << ", " << (ui64*)r14 << Log::Endl;
	return CPedFactoryCreatePed_orig0(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
}

ui64(*CPedFactoryCreatePed_orig1)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed1(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	print((char*)"Hook Called 1", 10);
	Log::Info << "Hook " << 1 << ": " << (ui64*)r3 << ", " << (ui64*)r4 << ", " << (ui64*)r5 << ", " << (ui64*)r6 << ", " << (ui64*)r7 << ", " << (ui64*)r8 << ", " << (ui64*)r9 << ", " << (ui64*)r10 << ", " << (ui64*)r11 << ", " << (ui64*)r12 << ", " << (ui64*)r13 << ", " << (ui64*)r14 << Log::Endl;
	return CPedFactoryCreatePed_orig1(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
}

ui64(*CPedFactoryCreatePed_orig2)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed2(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	print((char*)"Hook Called 2", 10);
	Log::Info << "Hook " << 2 << ": " << (ui64*)r3 << ", " << (ui64*)r4 << ", " << (ui64*)r5 << ", " << (ui64*)r6 << ", " << (ui64*)r7 << ", " << (ui64*)r8 << ", " << (ui64*)r9 << ", " << (ui64*)r10 << ", " << (ui64*)r11 << ", " << (ui64*)r12 << ", " << (ui64*)r13 << ", " << (ui64*)r14 << Log::Endl;
	return CPedFactoryCreatePed_orig2(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
}

ui64(*CPedFactoryCreatePed_orig3)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed3(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	print((char*)"Hook Called 3", 10);
	Log::Info << "Hook " << 3 << ": " << (ui64*)r3 << ", " << (ui64*)r4 << ", " << (ui64*)r5 << ", " << (ui64*)r6 << ", " << (ui64*)r7 << ", " << (ui64*)r8 << ", " << (ui64*)r9 << ", " << (ui64*)r10 << ", " << (ui64*)r11 << ", " << (ui64*)r12 << ", " << (ui64*)r13 << ", " << (ui64*)r14 << Log::Endl;
	return CPedFactoryCreatePed_orig3(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
}

ui64(*CPedFactoryCreatePed_orig4)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed4(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	print((char*)"Hook Called 4", 10);
	Log::Info << "Hook " << 4 << ": " << (ui64*)r3 << ", " << (ui64*)r4 << ", " << (ui64*)r5 << ", " << (ui64*)r6 << ", " << (ui64*)r7 << ", " << (ui64*)r8 << ", " << (ui64*)r9 << ", " << (ui64*)r10 << ", " << (ui64*)r11 << ", " << (ui64*)r12 << ", " << (ui64*)r13 << ", " << (ui64*)r14 << Log::Endl;
	return CPedFactoryCreatePed_orig4(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
}

ui64(*CPedFactoryCreatePed_orig5)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed5(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	print((char*)"Hook Called 5", 10);
	Log::Info << "Hook " << 5 << ": " << (ui64*)r3 << ", " << (ui64*)r4 << ", " << (ui64*)r5 << ", " << (ui64*)r6 << ", " << (ui64*)r7 << ", " << (ui64*)r8 << ", " << (ui64*)r9 << ", " << (ui64*)r10 << ", " << (ui64*)r11 << ", " << (ui64*)r12 << ", " << (ui64*)r13 << ", " << (ui64*)r14 << Log::Endl;
	return CPedFactoryCreatePed_orig5(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
}


void HookCPedFactoryCreatePed()
{
	//Log::Info << "Creating Hook" << Log::Endl;
	constexpr CMemory::Pattern createped0("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 54 41 55 41 56 41 57 48 8D 68 D1 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 44 0F B7 E2 4C 8B F9 BA ? ? ? ? ");
	//static auto gtaThreadKill = gtaThreadKillPat.Search().Get<void(*)(int)>();//gtaThreadKill is now a function
	//Log::Info << "Found address, info next" << Log::Endl;
	//ui64 *addr = createped.Search().Get<ui64*>();
	//Log::Info << "Found Hook Location " << addr << " value: " << *addr << Log::Endl;
	createped0.Search().Detour(CPedFactoryCreatePed0, &CPedFactoryCreatePed_orig0);

	constexpr CMemory::Pattern createped1("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 50 48 8B 9C 24 ? ? ? ? 49 8B F1 41 8B F8 44 0F B7 F2 48 8B E9 4C 8B FB 48 85 DB 0F 85 ? ? ? ? 38 1D ? ? ? ? 74 1C 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B D8 E9 ? ? ? ? 8A 05 ? ? ? ? ");
	createped1.Search().Detour(CPedFactoryCreatePed1, &CPedFactoryCreatePed_orig1);

	constexpr CMemory::Pattern createped2("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 55 41 56 41 57 48 83 EC 50 8B 82 ? ? ? ? 41 BD ? ? ? ? 41 23 C5 48 8B DA 2B 05 ? ? ? ? 4C 8B F1 4C 69 C0 ? ? ? ? 48 8B 05 ? ? ? ? 4D 8B 4C 00 ? 4D 8B C1 49 83 E0 FE 49 F7 D9 48 1B C0 45 33 FF 49 23 C0 44 39 78 44 7E 13 45 33 C0 33 D2 ");
	createped2.Search().Detour(CPedFactoryCreatePed2, &CPedFactoryCreatePed_orig2);

	constexpr CMemory::Pattern createped3("48 8B C4 48 89 58 08 48 89 70 18 48 89 78 20 55 41 56 41 57 48 8D 68 A1 48 81 EC ? ? ? ? 33 FF 45 8A F1 45 8A F8 48 8B F2 48 85 D2 0F 84 ? ? ? ? 0F 28 05 ? ? ? ? 48 8D 4D F7 80 65 6F FC 0F 57 C9 0F 14 C8 0F 57 D2 0F 29 45 07 F3 0F 6F 42 ? 0F 29 4D 17 0F 14 CA 0F 29 4D 27 66 0F 69 C0 66 0F 72 E0 ? ");
	createped3.Search().Detour(CPedFactoryCreatePed3, &CPedFactoryCreatePed_orig3);

	constexpr CMemory::Pattern createped4("48 85 D2 74 4A 48 89 5C 24 ? 57 48 83 EC 20 49 8B D8 48 8B FA 48 85 DB 74 2B 48 8B D3 48 8B CF E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 48 85 C0 74 08 48 8B C8 E8 ? ? ? ? 48 8B D3 48 8B CF E8 ? ? ? ? 48 8B 5C 24 ? 48 83 C4 20 5F ");
	createped4.Search().Detour(CPedFactoryCreatePed4, &CPedFactoryCreatePed_orig4);

	//constexpr CMemory::Pattern createped5("");
	//createped5.Search().Detour(CPedFactoryCreatePed5, &CPedFactoryCreatePed_orig5);

	Log::Info << "Hooked Ped Create" << Log::Endl;
}
#endif

#ifdef brokengetCPedFactory
void *getCPedFactoryOLLLLLDDD() {
	constexpr CMemory::Pattern getCPlayerInfoSelf("48 83 EC 28 8A 05 ? ? ? ? 33 D2 84 C0 75 05 4C 8B C2 EB 36 48 8B 0D ? ? ? ? 4C 8B 05 ? ? ? ? 48 C1 C9 05 48 C1 C1 20 4C 33 C1 8B C1 83 E0 1F 49 C1 C0 20 FF C0 8A C8 8A 05 ? ? ? ? 49 D3 C0 84 C0 74 C8 49 F7 D0 49 8B 48 08 48 85 C9 74 08 E8 ? ? ? ? 48 8B D0 48 8B C2 48 83 C4 28 C3 ");
	uint64_t address = getCPlayerInfoSelf.Search().Get<uint64_t>();//this will return 0x7FF6B4E8C450

	if (*(unsigned int*)address != 0x4883EC28) {
		unsigned int actual = *(unsigned int*)address;
		Log::Info << "Mis Match: " << (void*)actual << Log::Endl;//0000000028EC8348
	}

	Log::Info << "About to unlock function" << Log::Endl;

	MemUnlock lock(address, 0x64);

	uint64_t patchStart = address + 0x4B;
	//overrite with nop
	for (int i = 0; i < 0xE; i++) {
		*(unsigned char*)(patchStart + i) = 0x90;
	}

	Log::Info << "Overwrote with NOP's" << Log::Endl;
	//put in a small patch
	//{ 0x4C, 0x89, 0xC0 }
	//  0x49  0x8B  0xC0
	*(unsigned char*)(patchStart + 0) = 0x49;
	*(unsigned char*)(patchStart + 1) = 0x8B;
	*(unsigned char*)(patchStart + 2) = 0xC0;

	Log::Info << "About to call CPedFactory Call" << Log::Endl;

	void *CPedFactoryValue = 0;//CMemory((void*)address).Call<void*>();

	Log::Info << "CPedFactory: " << CPedFactoryValue << Log::Endl;

	unsigned char unpatch[] = { 0x49, 0x8B, 0x48, 0x08, 0x48, 0x85, 0xC9, 0x74, 0x08, 0xE8, 0x0F, 0xB3, 0xD9, 0xFF };
	for (int i = 0; i < 0xE; i++) {
		*(unsigned char*)(patchStart + i) = unpatch[i];
	}

	return CPedFactoryValue;

}
#endif

#include <vector> 
//std::vector<int> spawnedPeds;

void teleportPedToMe(int ped) {
	int myped = PLAYER::PLAYER_PED_ID();

	Vector3 myPos = ENTITY::GET_ENTITY_COORDS( myped, false, false);

	ENTITY::SET_ENTITY_COORDS( ped, myPos.x, myPos.y, myPos.z, true, true, true, false);
}

int getCVehicleModelInfo(int hash, int *d) {
	static uint64_t address = 0;
	if (address == 0) {
		constexpr CMemory::Pattern pattern("48 89 5C 24 ? 89 4C 24 08 57 48 83 EC 20 33 DB 48 8B FA 85 C9 74 73 48 8D 54 24 ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 48 04 48 F7 D8 48 1B D2 48 23 D1 74 53 8B 0A 0F B7 C1 48 85 FF 74 02 89 07 3B 05 ? ? ? ? 7D 2F 83 E1 1F 4C 8B C0 8B D1 48 8B C8 48 8B 05 ? ? ? ? 48 C1 E9 05 8B 0C 88 0F A3 D1 73 11 4C 0F AF 05 ? ? ? ? 4C 03 05 ? ? ? ? EB 03 4C 8B C3 4D 85 ");
		address = pattern.Search().Get<uint64_t>();
	}
	return CMemory((void*)address).Call<int>(hash, d);
}

unsigned short getWeirdVal(int hash) {
	int val = 0xFFFF;
	getCVehicleModelInfo(hash, &val);//getCVehicleModelInfo
	return (unsigned short)(val);
}

/*
0x15
0x6   Spawned peds from script
0x7
0xB   Caused a crash once
0x10  People sitting at table in saloon from this I think
0x11  More people in valentine outside (maybe some horses too?)
*/

bool zombieMode = false;
bool giantsMode = false;
float giantsModeScale = 3.0f;
float SizeOfMyPed = 1.0f;
bool neverWanted = false;
ui64(*CPedFactoryCreatePed_orig0)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14) = nullptr;
ui64 CPedFactoryCreatePed0(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9, ui64 r10, ui64 r11, ui64 r12, ui64 r13, ui64 r14)
{
	//print((char*)"Hook Called 0", 50);
	//Log::Info << "Hook " << 0 << ": " << (ui64*)r3 << ", " << (ui64*)r4 << ", " << (ui64*)r5 << ", " << (ui64*)r6 << "(short in r6): " << (ui64*)r6 << ", " << (ui64*)r7 << ", " << (ui64*)r8 << ", " << (ui64*)r9 << ", " << (ui64*)r10 << ", " << (ui64*)r11 << ", " << (ui64*)r12 << ", " << (ui64*)r13 << ", " << (ui64*)r14 << Log::Endl;

	if (zombieMode) {// && (/*r4 == 0x15 || */r4 == 0x6 || r4 == 0x7 || r4 == 0x10 || r4 == 0x11)

		char* h = (char*)zombiearr[rand() % 5];
		int hash = GAMEPLAY::GET_HASH_KEY(h);//random zombie hash

		/*bool dontUse = false;
		if (hash == 0x53367A8A) {
			dontUse = true;
			print("BAD VALUE FOUND!",5000);
		}*/

		unsigned short val = getWeirdVal(hash);

		//Log::Info << "Trying to change to this data (hash): " << (void*)hash << "  short: " << (void*)val << Log::Endl;

		if (STREAMING::HAS_MODEL_LOADED( hash)/* && dontUse == false*/) {
			//r4 = 0x15;//this seems to be what I need also, might also be 0x6
			*(short*)r6 = val;
		}
	}

	ui64 pedaddress = CPedFactoryCreatePed_orig0(r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);

	int ped = -1;
	if (pedaddress != 0) {
		//teleportPedToMe(ped);
		if (zombieMode) {// && (/*r4 == 0x15 || */r4 == 0x6 || r4 == 0x7 || r4 == 0x10 || r4 == 0x11)
			ped = entityToIndex((void*)pedaddress);
			if (ENTITY::DOES_ENTITY_EXIST(ped))
				setPedToCombatJustMe(ped);
		}
		/*if (giantsMode) {
			ped = entityToIndex((void*)pedaddress);
			if (ENTITY::DOES_ENTITY_EXIST(ped))
				PED::_SET_PED_SCALE(ped, 2.0f);
		}*/
	}
	else {
		//Log::Info << "Entity address zero!" << Log::Endl;
	}



	//Log::Info << "Entity data in hook: 0x" << (void*)pedaddress << ", index: " << ped << Log::Endl;

	//spawnedPeds.push_back(ped);


	return pedaddress;
}

ui64(*readFileOfType_orig)(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9) = nullptr;
ui64 readFileOfTypeDetour(ui64 r3, ui64 r4, ui64 r5, ui64 r6, ui64 r7, ui64 r8, ui64 r9) {

	char *str = (char*)r4;

	//Log::Info << "Loading file: " << str << Log::Endl;

	ui64 retval = readFileOfType_orig(r3, r4, r5, r6, r7, r8, r9);

	return retval;
}

//constexpr CMemory::Pattern CPedFactorycreateped0("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 54 41 55 41 56 41 57 48 8D 68 D1 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 44 0F B7 E2 4C 8B F9 BA ? ? ? ? ");
constexpr CMemory::Pattern CPedFactorycreateped0("48 89 5C 24 ? 66 89 4C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 66 41 81 39 ? ? 49 8B F1 4D 8B F0 44 8B EA 0F 84 ? ? ? ? 49 8B C9 E8 ? ? ? ? 44 8B 16 48 8D 8C 24 ? ? ? ? 66 89 84 24 ? ? ? ? 41 81 E2 ? ? ? ? 0F B7 C0 44 0B D0 44 89 94 24 ? ? ? ? E8 ? ? ? ? F6 40 6B 08 0F 84 ? ? ? ? 41 F6 06 02 8A 1D ? ? ? ? 74 07 B8 ");
//constexpr CMemory::Pattern CPedFactorycreateped0("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 70 0F B7 E9 49 8B D9 49 8B C9 49 8B F8 8B F2 E8 ? ? ? ? 44 8B 0B 4C 8B C7 66 89 84 24 ? ? ? ? 41 81 E1 ? ? ? ? 0F B7 C0 8B D6 44 0B C8 0F B7 CD 8A 84 24 ? ? ? ? 88 44 24 60 48 8B 84 24 ? ? ? ? 48 89 44 24 ? 8A 84 24 ? ? ? ? 88 44 24 50 83 64 24 ? ? 8A 84 24 ? ? ? ? 88 44 24 40 8A 84 24 ");//this one does not work 

uint64_t getCPedFactoryCreateFromSig() {
	//createped0.Search().Detour(CPedFactoryCreatePed0, &CPedFactoryCreatePed_orig0);
	return CPedFactorycreateped0.Search().Get<uint64_t>();
}

void* indexToEntity(int index) {
	static uint64_t address = 0;
	if (address == 0) {
		constexpr CMemory::Pattern pattern("44 8B C9 83 F9 FF 74 6F 8A 05 ? ? ? ? 84 C0 75 04 33 D2 EB 37 48 8B 0D ? ? ? ? 48 8B 15 ? ? ? ? 48 C1 C1 20 48 C1 C9 02 48 33 D1 8B C1 83 E0 1F 83 C0 03 8A C8 8A 05 ? ? ? ? 48 D3 C2 48 C1 C2 20 84 C0 74 C8 48 F7 D2 48 8B 42 10 45 8B C1 41 C1 F8 08 45 38 0C 00 75 0D 8B 42 1C 41 0F AF C0 48 03 42 08 EB 02 33 C0 48 85 C0 74 05 48 8B 40 08 C3 33 C0 C3 ");
		address = pattern.Search().Get<uint64_t>();
		//Log::Info << "indexToEntity address: 0x" << (void*)address << Log::Endl;
	}
	return CMemory((void*)address).Call<void*>(index);
}

int entityToIndex(void *entityAddress) {

	//address of ped pool
	//&getEntityFromIndex + 0x1D + 0x2DAE0A3/*&getEntityFromIndex+0x16   int there*/
	//0x2DB2B57 + 0x1D + 0x7FF66DC0E344


	static uint64_t address = 0;
	if (address == 0) {
		constexpr CMemory::Pattern pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B 15 ? ? ? ? 48 8B F1 48 83 C1 10 33 FF 32 DB E8 ? ? ? ? 48 85 C0 75 65 8A 05 ? ? ? ? 84 C0 75 05 45 33 C0 EB 37 48 8B 0D ? ? ? ? 4C 8B 05 ? ? ? ? 48 C1 C1 20 48 C1 C9 02 4C 33 C1 8B C1 83 E0 1F 83 C0 03 8A C8 8A 05 ? ? ? ? 49 D3 C0 49 C1 C0 20 84 C0 74 C7 49 F7 D0 BA ? ? ? ? 49 8B C8 E8 ");
		address = pattern.Search().Get<uint64_t>();
		//Log::Info << "entityToIndex address: 0x" << (void*)address << Log::Endl;
	}
	return CMemory((void*)address).Call<int>(entityAddress);
}



/*
constexpr CMemory::Pattern pattern("");
address = pattern.Search().Get<uint64_t>();
*/

void *getCPedFactory() {

	static void *addressSaved = 0;
	if (addressSaved != 0)
		return addressSaved;


	constexpr CMemory::Pattern getCPlayerInfoSelf("48 83 EC 28 8A 05 ? ? ? ? 33 D2 84 C0 75 05 4C 8B C2 EB 36 48 8B 0D ? ? ? ? 4C 8B 05 ? ? ? ? 48 C1 C9 05 48 C1 C1 20 4C 33 C1 8B C1 83 E0 1F 49 C1 C0 20 FF C0 8A C8 8A 05 ? ? ? ? 49 D3 C0 84 C0 74 C8 49 F7 D0 49 8B 48 08 48 85 C9 74 07 E8 ? ? ? ? 8B D0 8B C2 48 83 C4 28 C3 ");
	uint64_t address = getCPlayerInfoSelf.Search().Get<uint64_t>();//PLAYER_PED_ID

	//Log::Info << "About to unlock function" << Log::Endl;

	MemUnlock lock(address, 0x84);//0x64 old

	uint64_t patchStart = address + 0x4B;
	//overrite with nop

	unsigned char origData[0x12];
	for (int i = 0; i < 0x12; i++) {
		origData[i] = *(unsigned char*)(patchStart + i);
		*(unsigned char*)(patchStart + i) = 0x90;
	}

	//Log::Info << "Overwrote with NOP's" << Log::Endl;
	//put in a small patch
	//mov    rax, r8
	//{ 0x4C, 0x89, 0xC0 }
	//  0x49  0x8B  0xC0
	*(unsigned char*)(patchStart + 0) = 0x49;
	*(unsigned char*)(patchStart + 1) = 0x8B;
	*(unsigned char*)(patchStart + 2) = 0xC0;

	//Log::Info << "About to call CPedFactory Call" << Log::Endl;

	void *CPedFactoryValue = CMemory((void*)address).Call<void*>();

	//Log::Info << "CPedFactory: " << CPedFactoryValue << Log::Endl;

	for (int i = 0; i < 0x12; i++) {
		*(unsigned char*)(patchStart + i) = origData[i];
	}


	//test if called function is same as native call
	int pedIDFromAddr = CMemory((void*)address).Call<int>();
	int pedIDFromNative = PLAYER::PLAYER_PED_ID();

	//Log::Info << "Ped ID's: " << pedIDFromAddr << " real: " << pedIDFromNative << Log::Endl;

	addressSaved = CPedFactoryValue;

	return CPedFactoryValue;

}

struct Vector3f {
	float x, y, z;
};

struct PositionStruct {
	char unk0[0x70];//0x0
	Vector3f position;//0x70 change this one and you teleport, aka this one will teleport u after u move. Change this and origin for full teleport
};

//Reversed from CPed (my ped_
struct CEntity {
	char unk0[0x38];//0x0
	PositionStruct *positonData;//0x38
	char unk1[0x50 - 0x40];//0x40
	float unkRender1;//0x50
	float unkRender2;//0x54
	float scaleWidthA;//0x58
	float scaleWidth;//0x5C
	float tiltFowardBack;//0x60
	float tiltLeftRight;//0x64
	float scaleHeightA;//0x68
	float scaleHeight;//0x6C
	Vector3f origin;//0x70  this one teleports you until you move
};

bool WIDEMOD = false;
void WIDEMODtick() {
	if (WIDEMOD) {
		int myPed = PLAYER::PLAYER_PED_ID();
		CEntity *meEnt = (CEntity *)indexToEntity(myPed);
		if (meEnt != 0) {
			meEnt->scaleWidth = 2.0f;
			//*(float*)((uint64_t)meEnt + 0x620) = 2.0f;
			consolePrint((char*)"CHONKIFIED");
		}
	}
}
void toggleWIDEMOD() {
	//WIDEMOD = !WIDEMOD;
	if (WIDEMOD == false) {
		int myPed = PLAYER::PLAYER_PED_ID();
		CEntity *meEnt = (CEntity *)indexToEntity(myPed);
		if (meEnt != 0) {
			meEnt->scaleWidth = 0.0f;
		}
	}
}


uint64_t *getMountOfPed(int ped) {

	uint64_t myPedAddress = (uint64_t)indexToEntity(ped);

	static uint64_t address = 0;
	if (address == 0) {
		constexpr CMemory::Pattern getCPlayerInfoSelf("8B 81 ? ? ? ? 25 ? ? ? ? 2B 05 ? ? ? ? 48 69 C8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 94 01 ? ? ? ? 48 8B C2 48 83 E0 FE 48 F7 DA 48 1B C9 48 23 C8 74 12 8B 41 10 C1 E8 04 A8 01 74 08 48 8B 81 ? ? ? ? C3 33 C0 C3 ");
		address = getCPlayerInfoSelf.Search().Get<uint64_t>();//PLAYER_PED_ID
		DWORD rights;
		VirtualProtect((LPVOID)address, 0x50, PAGE_EXECUTE_READWRITE, &rights);
	}

	unsigned char patch[] = { 0x48, 0x89, 0xC8 };//mov     rax, rcx

	//MemUnlock lock(address, 0x50);//0x4E actual

	uint64_t patchStart = address + 0x41;
	//overrite with nop

	unsigned char origData[9];
	for (int i = 0; i < sizeof(origData); i++) {
		origData[i] = *(unsigned char*)(patchStart + i);
		*(unsigned char*)(patchStart + i) = 0x90;
	}


	*(unsigned char*)(patchStart + 0) = patch[0];
	*(unsigned char*)(patchStart + 1) = patch[1];
	*(unsigned char*)(patchStart + 2) = patch[2];

	uint64_t mountAddress = CMemory((void*)address).Call<uint64_t>(myPedAddress);
	mountAddress = mountAddress + 0x178;

	uint64_t *retVal = (uint64_t *)mountAddress;

	for (int i = 0; i < sizeof(origData); i++) {
		*(unsigned char*)(patchStart + i) = origData[i];
	}

	return retVal;
}

void changePedMount(int newEntity) {
	uint64_t *pedMounted = getMountOfPed(PLAYER::PLAYER_PED_ID());
	if (pedMounted != 0) {
		void *forgeEntity = indexToEntity(newEntity);
		if (forgeEntity != 0) {
			*pedMounted = (uint64_t)forgeEntity;
			print((char*)"Mounted ped :)");
		}
		else {
			print((char*)"Error getting address of forge entity");
		}
	}
	else {
		print((char*)"pedMounted is 0");
	}
}

bool enableLocalGamemodeBool = false;
void localEntityManager() {
	if (enableLocalGamemodeBool) {
		Vector3 myPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false);
		runOnAllPeds([myPos](auto ped) {
			if (ENTITY::IS_ENTITY_DEAD(ped) || ENTITY::GET_ENTITY_COORDS(ped, true, false).sub(myPos).magnitude() > 1000.0f) {
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, false, false);
			}
			else {
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, true, true);
			}
		});
	}
}
void enableLocalGamemode() {
	enableLocalGamemodeBool = true;
}
void disableLocalGamemode() {
	enableLocalGamemodeBool = false;
	runOnAllPeds([](auto ped) {
		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, false, false);
	});
}


int snowCoverageLevel = 0;
bool customCameraZoom = false;
float customCameraZoomFloat = 1.0f;

void TickCpp()
{
	runlater::loop();

	LoadModelTick();
	noclipResource.Tick();
	forge::tick();
	simpleMenu.render();
	shootiportTick();
	explosiveAmmoTick();
	lightningAmmoTick();
	forceFieldTick();
	infiniteAmmoTick();
	RapidFireLoop();
	WIDEMODtick();
	shootAnimalsTick();
	predatorMissile::loop();
	circlePeds();
	ufomod::loop();
	PortalGun::loop();
	localEntityManager();
	if (snowCoverageLevel != 0) {
		GRAPHICS::_0xF02A9C330BBFC5C7(snowCoverageLevel);
	}

	if (customCameraZoom)
		CAM::_ANIMATE_GAMEPLAY_CAM_ZOOM(1.0f, customCameraZoomFloat);

	camShit::loop();//prefer to have this after any functions that may set position/rotation

	if (giantsMode)
		runOnAllPeds([](auto ped) {PED::_SET_PED_SCALE(ped, giantsModeScale); });

	//Hash WeaponID = 0;//w_rifle_boltaction01
	//WEAPON::GET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), &WeaponID, 0, 0, 0);

	//char bsdjgbsdjv[100];
	//snprintf(bsdjgbsdjv, sizeof(bsdjgbsdjv), "Weapon: 0x%X", WeaponID);
	//consolePrint(bsdjgbsdjv);


	//char buf[50];
	//sprintf(buf, "Speed: %f", Native::Invoke<float>(N::GET_ENTITY_SPEED, PLAYER::PLAYER_PED_ID()));
	//consolePrint(buf);

	for (int i = 0; i < 5; i++)
		RequestModel(GAMEPLAY::GET_HASH_KEY((char*)zombiearr[i]));

	if (zombieMode) {
		PLAYER::SET_WANTED_LEVEL_MULTIPLIER( 0.0f);
		//consolePrint((char*)"Zombie Apocalypse");
	}

	if (neverWanted) {
		PLAYER::SET_WANTED_LEVEL_MULTIPLIER( 0.0f);
		//consolePrint((char*)"Never Wanted");
	}

	/*if (forcePushBool) {
		consolePrint((char*)"Force Push (Press F)");
	}*/


	//char tbuf[50];
	//sprintf(tbuf,"Options To Render: %i", simpleMenu.maxOptionsToRender);
	//consolePrint(tbuf);

	//for (auto i = spawnedPeds.begin(); i != spawnedPeds.end(); ++i)
	//	teleportPedToMe(*i);

	switch (simpleMenu.getCurrentMenu()) {
	case MENU_MAIN:
		switch (simpleMenu.clickOption()) {
		case 0:
			noclipResource.Toggle();
			print((char*)"Noclip Toggled");
			break;
		case 1:
			simpleMenu.openSubmenu(MENU_SPAWN_VEHICLES);
			break;
		case 2:
			simpleMenu.openSubmenu(MENU_SPAWN_PEDS);
			break;
		case 3:
			simpleMenu.openSubmenu(MENU_CHANGE_PED);
			break;
		case 4:
			simpleMenu.openSubmenu(MENU_TELEPORTS);
			break;
		case 5:
			simpleMenu.openSubmenu(MENU_WEATHER);
			break;
		case 6:
			simpleMenu.openSubmenu(MENU_TIME);
			break;
		case 7:
			simpleMenu.openSubmenu(MENU_SELF);
			break;
		case 8:
			simpleMenu.openSubmenu(MENU_WEAPONS);
			break;
		case 9:
			simpleMenu.openSubmenu(MENU_FUN);
			break;
			
		

	//	case 13:
			//shootiportBool = !shootiportBool;
		//	break;
	//	case 14:
			//explosiveAmmoBool = !explosiveAmmoBool;
			//break;
		//case 15:
			//infiniteAmmoBool = !infiniteAmmoBool;
			//break;
		
		
		//case 17:
			//RapidfireActive = !RapidfireActive;
		//	break;
		//case 19:
			//forceFieldBool = !forceFieldBool;
			//break;
		//case 20:
			//forcePushBool = !forcePushBool;
			//break;
	

		//case 22:
			//shootAnimals = !shootAnimals;
			//break;
		//case 23:
			//lightningAmmoBool = !lightningAmmoBool;
			//break;
		
		/*case 27:
			if (1 == 1) {
				float *level = getSnowLevel();
				//char buf[100];
				//snprintf(buf,sizeof(buf),"Snow level: %f",*level);
				//print(buf);
			}
			break;
		case 28:
			//*getSnowLevel() = 5.0f;
			//print("Set snow level to 5");
			setSnowLevelAdvanced(5.0f,false);
			break;
		case 29:
			setSnowLevelAdvanced(5.0f, true);
			break;*/
		
		/*case 28:
		{
			static Vector3 loc = { 0,0,0 };
			loc = GET_COORDS_INFRONT(10).add(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), false, false));
			SpawnObject(0xB72F3DA7,&loc);//ufo above shack
			//loc = loc.add({ 10,0,0 });
			SpawnObject(0xC92962E3,&loc);//ufo above mt shawn
			break;
		}*/
		

		case 2500:
			if (forge::objectSelected != -1) {
				testMount(forge::objectSelected);
				print((char*)"Mounted ped");
			}
			else {
				print((char*)"No selected ped");
			}
			break;
		case 2600:
		{
			/*if (forge::objectSelected != -1) {
				testMount2(forge::objectSelected);
				print((char*)"Mounted ped 2");
			}
			else {
				print((char*)"No selected ped 2");
			}*/
			uint64_t *pedMounted = getMountOfPed(PLAYER::PLAYER_PED_ID());
			if (pedMounted != 0) {
				//Log::Info << "Ped Mounted Ptr: " << pedMounted << "  Ped Addr:" << *pedMounted << Log::Endl;
				print((char*)"Printed info to log");
			}
			else {
				print((char*)"pedMounted is 0");
			}
		}
		break;

		case 2700:
		{
			if (forge::objectSelected != -1) {

				uint64_t *pedMounted = getMountOfPed(PLAYER::PLAYER_PED_ID());
				if (pedMounted != 0) {
					void *forgeEntity = indexToEntity(forge::objectSelected);
					if (forgeEntity != 0) {
						*pedMounted = (uint64_t)forgeEntity;
						print((char*)"Mounted ped :)");
					}
					else {
						print((char*)"Error getting address of forge entity");
					}
				}
				else {
					print((char*)"pedMounted is 0");
				}
			}
			else {
				print((char*)"No selected ped");
			}

		}


		break;

		}
		break;

	case MENU_WEAPONS:
		switch (simpleMenu.clickOption()) {
		case 0:
			giveAllWeapons();
			print((char*)"All weapons given");
			break;
		case 7:
			/*if (forge::enabled == false) {
				print((char*)"Please enable forge first!");
				break;
			}*/
			//forge::gravityGun = !forge::gravityGun;
			if (forge::gravityGun)
				forge::enabled = true;
			else
				forge::enabled = false;
			break;
		case 9:
			PortalGun::deletePortals();
			print("Portals Removed");
			break;
		}
		break;

	case MENU_FUN:
		switch (simpleMenu.clickOption()) {
		case 0:
			ufomod::toggle(true);
			break;
		case 1:
			ufomod::deleteUfo();
			break;
		case 2:

			break;
		case 3:
			predatorMissile::startMissile();
			break;
		case 4:
			toggleWIDEMOD();
			break;
		case 5:
			//zombieMode = !zombieMode;
			if (zombieMode == false) {
				PLAYER::SET_WANTED_LEVEL_MULTIPLIER(1.0f);
			}
			break;
		case 6:
			spawnZombieTest();
			print((char*)"Zombie Spawned");
			break;
		case 9:
			if (giantsMode == false)
				runOnAllPeds([](auto ped) {PED::_SET_PED_SCALE(ped, 1.0f); });//reset size
			break;
		/*case 10:
			keyboardInput("1.5", &giantsModeScale);
			break;*/
		case 11:
			//print((char*)"Currently broken, can't shoot micah so other ped was spawned :(");
			SpawnPed("CS_MicahBell", [](auto ped) {//CS_MicahBell
				/*
				PED::SET_PED_MAX_HEALTH( ped, 0x7FFFFFFF);
				ENTITY::SET_ENTITY_MAX_HEALTH( ped, 0x7FFFFFFF);
				ENTITY::SET_ENTITY_HEALTH( ped, 0x7FFFFFFF, 0);
				Native::Invoke<void>(N::SET_PED_CAN_BE_TARGETTED( ped, true);
				Native::Invoke<void>(N::SET_PED_CAN_BE_TARGETTED_BY_PLAYER( ped, 0, true);
				Native::Invoke<void>(N::SET_PED_CAN_BE_TARGETTED_BY_TEAM( ped, Native::Invoke<int>(N::GET_PLAYER_TEAM, 0), true);
				*/
				//setPedToCombatJustMe(ped);
				setPedKillable(ped);
				//Native::Invoke<void>(N::REMOVE_ALL_PED_WEAPONS, ped, true);
			});
			break;
		case 12:
			GRAPHICS::_0xF02A9C330BBFC5C7(snowCoverageLevel);
			break;
		case 13:
			//enableLocalGamemode();
			SpawnPed("CS_JackMarston", [](auto ped) {
				
				//addPedToHumanGroup(PLAYER::PLAYER_PED_ID());
				
				runOnAllPeds([](auto pedid) {
					//ENTITY::SET_ENTITY_AS_MISSION_ENTITY(pedid, true, true);
					addPedToHumanGroup(pedid);
				});
				
				//setPedToCombatGroup(ped);

				addPedToZombieGroup(ped);
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, true, true);

				Hash hash = GAMEPLAY::GET_HASH_KEY("WEAPON_SHOTGUN_SEMIAUTO");
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(ped, hash, -1, true, 0x2cd419dc);
				WEAPON::SET_PED_AMMO(ped, hash, 1000);
				WEAPON::SET_CURRENT_PED_WEAPON(ped, hash, 1, 0, 0, 0);

				AI::TASK_COMBAT_HATED_TARGETS(ped, 1000.0f);//unsure  0x8182B561A29BD597
				PED::SET_PED_FLEE_ATTRIBUTES(ped, 0, 0);//unsure
				PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, 1);//unsure
				PED::SET_PED_KEEP_TASK(ped, true);

				

				//Health stuff does nothing for the kid. they can't die
				//ENTITY::SET_ENTITY_PROOFS(ped, true, true, true, true, true, true, true, true);
				//ENTITY::SET_ENTITY_INVINCIBLE(ped, true);
				
				int newHealth = 100;//ENTITY::GET_ENTITY_MAX_HEALTH(ped, false) * 5;
				ENTITY::SET_ENTITY_MAX_HEALTH(ped, newHealth);
				ENTITY::SET_ENTITY_HEALTH(ped, newHealth, false);
				//setPedKillable(ped);

				ENTITY::SET_ENTITY_INVINCIBLE(ped, false);
				ENTITY::SET_ENTITY_PROOFS(ped, false, false, false, false, false, false, false, false);
				
				//static char buf[50];
				//snprintf(buf, sizeof(buf), "Max Health: %i", ENTITY::GET_ENTITY_MAX_HEALTH(ped, false));
				//print(buf, 5000);
				

				
			});
			break;
		case 14:
			PED::_SET_PED_SCALE(PLAYER::PLAYER_PED_ID(), SizeOfMyPed);
			break;
		}
		break;
	
	case MENU_SELF:
		switch (simpleMenu.clickOption()) {
		case 1:
			if (neverWanted == false) {
				PLAYER::SET_WANTED_LEVEL_MULTIPLIER(1.0f);
			}
			break;
		case 2:
			tpIntoClosestVehicle();
			break;
		case 3:
			intoTheSky();
			break;
		case 5:
			forge::toggle();
			forge::gravityGun = false;
			break;
		case 6:
			if (customCameraZoom == false) {
				CAM::_ANIMATE_GAMEPLAY_CAM_ZOOM(1.0f, 1.0f);
			}
			break;
		
		/*case 6:
		{
			//PED::_SET_PED_SCALE(forge::objectSelected, 1.0f);
			CEntity *meEnt = (CEntity *)indexToEntity(forge::objectSelected);
			if (meEnt != 0) {
				meEnt->scaleWidth = 0.0f;
				meEnt->scaleHeight = 0.0f;
			}
		}
			break;
		case 7:
		{
			PED::_SET_PED_SCALE(forge::objectSelected, 2.0f);
			CEntity *meEnt = (CEntity *)indexToEntity(forge::objectSelected);
			if (meEnt != 0) {
				meEnt->scaleWidth = 2.0f;
				meEnt->scaleHeight = 2.0f;
			}
		}
			break;*/
		}
		break;

	case MENU_SPAWN_VEHICLES:
	{
		static char *lastVehicleInput = (char*)"stagecoach001x";
		const char *vehiclesArray[] = { (char*)"privateopensleeper02x",(char*)"privateopensleeper01x",(char*)"steamerDummy",(char*)"armoredCar01x",(char*)"armoredCar03x",(char*)"privatebaggage01x",(char*)"smuggler02",(char*)"keelboat",(char*)"boatSteam02x",(char*)"midlandrefrigeratorCar",(char*)"midlandboxcar05x",(char*)"caboose01x",(char*)"canoe",(char*)"canoeTreeTrunk",(char*)"cart01",(char*)"cart02",(char*)"cart03",(char*)"cart04",(char*)"cart05",(char*)"cart06",(char*)"cart07",(char*)"cart08",(char*)"coach2",(char*)"coach3",(char*)"coach3_cutscene",(char*)"coach4",(char*)"coach5",(char*)"coach6",(char*)"buggy01",(char*)"buggy02",(char*)"buggy03",(char*)"ArmySupplyWagon",(char*)"chuckwagon000x",(char*)"supplywagon",(char*)"supplywagon2",(char*)"logwagon",(char*)"logwagon2",(char*)"coal_wagon",(char*)"chuckwagon002x",(char*)"gatling_gun",(char*)"gatlingMaxim02",(char*)"handcart",(char*)"horseBoat",(char*)"hotAirBalloon01",(char*)"hotchkiss_cannon",(char*)"mineCart01x",(char*)"northflatcar01x",(char*)"privateflatcar01x",(char*)"northpassenger01x",(char*)"northpassenger03x",(char*)"privatepassenger01x",(char*)"oilWagon01x",(char*)"oilWagon02x",(char*)"pirogue",(char*)"pirogue2",(char*)"policeWagon01x",(char*)"policeWagongatling01x",(char*)"privateCoalCar01x",(char*)"NorthCoalCar01x",(char*)"winterSteamer",(char*)"wintercoalcar",(char*)"privateboxcar04x",(char*)"privateboxcar02x",(char*)"privateboxcar01x",(char*)"coalHopper01x",(char*)"privateObservationcar",(char*)"privateArmoured",(char*)"privateDining01x",(char*)"privateRooms01x",(char*)"privateSteamer01x",(char*)"northSteamer01x",(char*)"GhostTrainSteamer",(char*)"GhostTrainCoalCar",(char*)"GhostTrainPassenger",(char*)"GhostTrainCaboose",(char*)"rcBoat",(char*)"rowboat",(char*)"rowboatSwamp",(char*)"rowboatSwamp02",(char*)"ship_guama02",(char*)"turbineboat",(char*)"ship_nbdGuama",(char*)"ship_nbdGuama2",(char*)"skiff",(char*)"stagecoach001x",(char*)"stagecoach002x",(char*)"stagecoach003x",(char*)"stagecoach004x",(char*)"stagecoach005x",(char*)"stagecoach006x",(char*)"trolley01x",(char*)"TugBoat2",(char*)"wagon02x",(char*)"wagon03x",(char*)"wagon04x",(char*)"wagon05x",(char*)"wagon06x",(char*)"wagonCircus01x",(char*)"wagonCircus02x",(char*)"wagonDoc01x",(char*)"wagonPrison01x",(char*)"wagonWork01x",(char*)"wagonDairy01x",(char*)"wagonTraveller01x",(char*)"breach_cannon",(char*)"utilliwag",(char*)"gatchuck",(char*)"gatchuck_2" };
		int optionClicked = simpleMenu.clickOption();
		switch (optionClicked) {
		case 0:
			KeyboardInput([](auto input) {//stagecoach001x
				lastVehicleInput = input;//input was strdup'ed
				SpawnVehicle(input, [](auto veh) {
					int myPed = PLAYER::PLAYER_PED_ID();
					PED::SET_PED_INTO_VEHICLE( myPed, veh, -1);//set into the vehicle
					//print((char*)"Vehicle Spawned");
				});
			}, lastVehicleInput);
			break;
		default:
			if (optionClicked != -1) {
				SpawnVehicle(vehiclesArray[optionClicked - 1], [](auto veh) {
					int myPed = PLAYER::PLAYER_PED_ID();
					PED::SET_PED_INTO_VEHICLE(myPed, veh, -1);
				});
			}
			break;
		}
	}
	break;

	case MENU_SPAWN_PEDS:
	{
		static char *lastVehicleInput = (char*)"A_M_M_ARMCHOLERACORPSE_01";
		const char *pedsArray[] = { (char*)"CS_BILLWILLIAMSON",(char*)"CS_MICAHBELL",(char*)"CS_UNCLE",(char*)"CS_SEAN",(char*)"CS_KIERAN",(char*)"A_M_M_WapWarriors_01",(char*)"A_M_O_WapTownfolk_01",(char*)"A_F_O_WapTownfolk_01",(char*)"U_M_M_WAPOFFICIAL_01",(char*)"A_M_M_WAPWARRIORS_01",(char*)"A_M_O_WAPTOWNFOLK_01",(char*)"S_M_M_UNITRAINENGINEER_01",(char*)"S_M_M_UNITRAINGUARDS_01",(char*)"A_M_M_MiddleTrainPassengers_01",(char*)"A_F_M_MiddleTrainPassengers_01",(char*)"A_M_M_LowerTrainPassengers_01",(char*)"A_F_M_LowerTrainPassengers_01",(char*)"A_M_M_UpperTrainPassengers_01",(char*)"A_F_M_UpperTrainPassengers_01",(char*)"A_M_M_NBXUPPERCLASS_01",(char*)"A_F_M_NBXUPPERCLASS_01",(char*)"G_M_M_UNIDUSTER_02",(char*)"G_M_M_UNIDUSTER_03",(char*)"CS_EDMUNDLOWRY",(char*)"G_M_M_UniBanditos_01",(char*)"CS_POISONWELLSHAMAN",(char*)"S_M_M_RACRAILGUARDS_01",(char*)"U_M_M_valbarber_01",(char*)"U_M_M_ValBartender_01",(char*)"U_M_M_ValDoctor_01",(char*)"U_M_M_ValGunsmith_01",(char*)"CS_VALSHERIFF",(char*)"G_M_M_UNIDUSTER_01",(char*)"S_M_M_SDTICKETSELLER_01",(char*)"S_M_M_MagicLantern_01",(char*)"CS_GrizzledJon",(char*)"A_M_M_VALCRIMINALS_01",(char*)"A_M_M_ASBMINER_02",(char*)"A_M_Y_ASBMINER_02",(char*)"A_M_M_ASBMINER_04",(char*)"A_M_Y_ASBMINER_04",(char*)"A_M_M_ASBMINER_03",(char*)"A_M_Y_ASBMINER_03",(char*)"A_M_M_JamesonGuard_01",(char*)"U_M_O_ASBSHERIFF_01",(char*)"A_M_M_RkrFancyDRIVERS_01",(char*)"A_M_M_RkrFancyTravellers_01",(char*)"A_F_M_RkrFancyTravellers_01",(char*)"S_M_M_CornwallGuard_01",(char*)"G_M_M_UniCornwallGoons_01",(char*)"U_M_M_CRDHOMESTEADTENANT_01",(char*)"U_M_M_CRDHOMESTEADTENANT_02",(char*)"U_M_O_PSHRANCHER_01",(char*)"U_M_O_CAJHOMESTEAD_01",(char*)"U_M_Y_CAJHOMESTEAD_01",(char*)"U_M_M_CAJHOMESTEAD_01",(char*)"CS_DOROETHEAWICKLOW",(char*)"S_M_M_AmbientSDPolice_01",(char*)"A_M_M_SDObeseMen_01",(char*)"A_F_M_SDObeseWomen_01",(char*)"U_M_M_NBXShadyDealer_01",(char*)"A_F_M_WapTownfolk_01",(char*)"A_M_M_SDLABORERS_02",(char*)"A_M_Y_NBXSTREETKIDS_01",(char*)"A_M_M_LowerSDTownfolk_02",(char*)"A_M_M_LowerSDTownfolk_01",(char*)"A_M_Y_NbxStreetKids_Slums_01",(char*)"A_M_Y_SDStreetKids_Slums_02",(char*)"A_M_M_SDDockWorkers_02",(char*)"A_M_M_SDDockForeman_01",(char*)"A_F_M_NbxSlums_01",(char*)"A_F_M_SDSlums_02",(char*)"A_M_M_NbxSlums_01",(char*)"A_M_M_SDSlums_02",(char*)"A_M_O_SDUpperClass_01",(char*)"A_F_M_NbxUpperClass_01",(char*)"A_F_O_SDUpperClass_01",(char*)"A_M_M_MiddleSDTownfolk_01",(char*)"A_M_M_MiddleSDTownfolk_02",(char*)"A_M_M_MiddleSDTownfolk_03",(char*)"A_F_M_MiddleSDTownfolk_01",(char*)"A_F_M_MiddleSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_01",(char*)"A_F_M_LowerSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_03",(char*)"A_F_M_SDChinatown_01",(char*)"A_F_O_SDChinatown_01",(char*)"A_F_M_SDFancyWhore_01",(char*)"G_M_M_UNIBRONTEGOONS_01",(char*)"A_M_M_SDChinatown_01",(char*)"A_M_O_SDChinatown_01",(char*)"U_M_M_VALGUNSMITH_01",(char*)"U_M_M_VALGENSTOREOWNER_01",(char*)"U_M_M_ValButcher_01",(char*)"A_M_M_VALTOWNFOLK_01",(char*)"U_M_M_VALDOCTOR_01",(char*)"S_M_M_BANKCLERK_01",(char*)"U_M_M_ValHotelOwner_01",(char*)"U_M_M_VALBARTENDER_01",(char*)"U_M_M_VALBARBER_01",(char*)"U_M_O_ValBartender_01",(char*)"U_M_M_NBXBARTENDER_02",(char*)"A_M_M_BLWUPPERCLASS_01",(char*)"U_M_M_NBXBARTENDER_01",(char*)"A_M_M_MIDDLESDTOWNFOLK_01",(char*)"U_M_M_NBXGENERALSTOREOWNER_01",(char*)"U_M_M_NBXSHADYDEALER_01",(char*)"S_M_M_TAILOR_01",(char*)"U_M_M_NBXGUNSMITH_01",(char*)"CS_SDDOCTOR_01",(char*)"S_M_M_LIVERYWORKER_01",(char*)"S_M_M_MarketVendor_01",(char*)"U_M_M_SDPhotographer_01",(char*)"U_M_M_STRGENSTOREOWNER_01",(char*)"U_M_M_STRFREIGHTSTATIONOWNER_01",(char*)"U_M_M_ASBGUNSMITH_01",(char*)"U_M_M_SDTRAPPER_01",(char*)"S_M_M_TRAINSTATIONWORKER_01",(char*)"A_F_M_LAGTOWNFOLK_01",(char*)"S_M_M_SKPGUARD_01",(char*)"A_M_M_SKPPRISONER_01",(char*)"U_F_M_LagMother_01",(char*)"A_F_O_LagTownfolk_01",(char*)"A_M_O_LagTownfolk_01",(char*)"A_M_M_RHDFOREMAN_01",(char*)"U_M_M_ORPGUARD_01",(char*)"CS_ABE",(char*)"CS_DavidGeddes",(char*)"CS_ANGUSGEDDES",(char*)"CS_DUNCANGEDDES",(char*)"CS_MRSGEDDES",(char*)"G_M_M_UNIMOUNTAINMEN_01",(char*)"A_F_M_LagTownfolk_01",(char*)"CS_ThomasDown",(char*)"CS_ArchieDown",(char*)"CS_EDITHDOWN",(char*)"S_F_M_MaPWorker_01",(char*)"A_M_M_StrTownfolk_01",(char*)"A_M_M_STRLABORER_01",(char*)"A_M_M_BynRoughTravellers_01",(char*)"A_C_DOGPOODLE_01",(char*)"S_M_M_RHDCOWPOKE_01",(char*)"S_M_M_UniButchers_01",(char*)"S_M_Y_NewspaperBoy_01",(char*)"A_F_M_RhdUpperClass_01",(char*)"A_F_M_RhdTownfolk_02",(char*)"a_m_m_rhdtownfolk_01_laborer",(char*)"A_F_M_RHDTOWNFOLK_01",(char*)"U_M_M_RHDBARTENDER_01",(char*)"U_M_M_RHDGUNSMITH_01",(char*)"U_M_M_RHDGENSTOREOWNER_01",(char*)"U_M_M_RHDGENSTOREOWNER_02",(char*)"U_M_M_RHDSHERIFF_01",(char*)"U_M_M_RHDTRAINSTATIONWORKER_01",(char*)"A_F_M_RHDPROSTITUTE_01",(char*)"A_M_M_RHDObeseMen_01",(char*)"A_M_M_RhdUpperClass_01",(char*)"A_M_M_RkrRoughTravellers_01",(char*)"A_C_DOGCATAHOULACUR_01",(char*)"A_F_M_VHTTOWNFOLK_01",(char*)"A_M_M_VHTTHUG_01",(char*)"U_M_M_VHTSTATIONCLERK_01",(char*)"U_M_O_VHTEXOTICSHOPKEEPER_01",(char*)"U_F_M_VhTBartender_01",(char*)"A_M_M_HtlFancyDRIVERS_01",(char*)"A_M_M_HtlFancyTravellers_01",(char*)"A_F_M_HtlFancyTravellers_01",(char*)"S_M_M_BARBER_01",(char*)"S_M_M_UNIBUTCHERS_01",(char*)"S_M_M_BLWCOWPOKE_01",(char*)"A_M_M_BLWObeseMen_01",(char*)"A_M_O_BlWUpperClass_01",(char*)"A_F_M_BlWUpperClass_01",(char*)"A_F_O_BlWUpperClass_01",(char*)"A_M_M_BlWLaborer_02",(char*)"A_M_M_BlWLaborer_01",(char*)"A_M_M_BlWForeman_01",(char*)"A_F_M_BlWTownfolk_02",(char*)"A_F_M_BlWTownfolk_01",(char*)"S_M_M_AmbientBlWPolice_01",(char*)"S_M_M_FussarHenchman_01",(char*)"A_F_M_GuaTownfolk_01",(char*)"A_F_O_GuaTownfolk_01",(char*)"A_M_M_GuaTownfolk_01",(char*)"A_M_O_GuaTownfolk_01",(char*)"CS_NicholasTimmins",(char*)"A_M_M_STRFANCYTOURIST_01",(char*)"A_F_M_BiVFancyTravellers_01",(char*)"A_F_M_STRTOWNFOLK_01",(char*)"A_M_M_UNIGUNSLINGER_01",(char*)"U_M_M_StrGenStoreOwner_01",(char*)"U_M_M_STRWELCOMECENTER_01",(char*)"U_M_M_STRSHERRIFF_01",(char*)"U_M_M_StrFreightStationOwner_01",(char*)"G_M_M_UNICRIMINALS_01",(char*)"G_M_M_UNICRIMINALS_02",(char*)"A_M_M_BTCObeseMen_01",(char*)"A_F_M_BTCObeseWomen_01",(char*)"A_M_M_AsbTownfolk_01",(char*)"A_F_M_AsbTownfolk_01",(char*)"A_F_M_BTCHILLBILLY_01",(char*)"A_F_O_BTCHILLBILLY_01",(char*)"A_M_M_BTCHILLBILLY_01",(char*)"G_M_M_UniBraithwaites_01",(char*)"U_M_O_CmrCivilwarcommando_01",(char*)"S_M_M_Army_01",(char*)"U_M_M_BWMSTABLEHAND_01",(char*)"S_M_M_CGHWORKER_01",(char*)"G_M_M_UniGrays_01",(char*)"G_M_O_UniExConfeds_01",(char*)"G_M_Y_UniExConfeds_01",(char*)"G_M_M_UNIINBRED_01",(char*)"CS_AberdeenSister",(char*)"A_M_M_GriRoughTravellers_01",(char*)"A_F_M_ValTownfolk_01",(char*)"U_F_O_WTCTOWNFOLK_01",(char*)"U_M_M_BLWTRAINSTATIONWORKER_01",(char*)"CS_BLWPHOTOGRAPHER",(char*)"A_M_M_BlWTownfolk_01",(char*)"U_M_O_BLWPOLICECHIEF_01",(char*)"S_M_M_Tailor_01",(char*)"S_M_M_BankClerk_01",(char*)"U_M_O_BLWBARTENDER_01",(char*)"U_M_O_BLWGENERALSTOREOWNER_01",(char*)"S_M_M_TumDeputies_01",(char*)"CS_SHERIFFFREEMAN",(char*)"A_M_M_NEAROUGHTRAVELLERS_01",(char*)"A_M_M_TumTownfolk_01",(char*)"A_F_M_TumTownfolk_01",(char*)"A_M_M_TumTownfolk_02",(char*)"A_F_M_TumTownfolk_02",(char*)"U_M_M_TUMBARTENDER_01",(char*)"U_M_M_TUMBUTCHER_01",(char*)"U_M_M_TUMGUNSMITH_01",(char*)"U_F_M_TUMGENERALSTOREOWNER_01",(char*)"A_M_M_ARMTOWNFOLK_01",(char*)"A_M_M_RANCHERTRAVELERS_WARM_01",(char*)"S_M_M_AmbientLawRural_01",(char*)"A_M_M_ArmCholeraCorpse_01",(char*)"A_F_M_ArmCholeraCorpse_01",(char*)"U_M_M_ARMGENERALSTOREOWNER_01",(char*)"U_M_O_ARMBARTENDER_01",(char*)"U_M_M_ARMTRAINSTATIONWORKER_01",(char*)"U_M_M_ARMUNDERTAKER_01",(char*)"RE_DEADBODIES_MALE",(char*)"RE_DEADBODIES_MALES_01",(char*)"A_F_M_ArmTownfolk_01",(char*)"A_F_M_ArmTownfolk_02",(char*)"A_M_M_ArmTownfolk_01",(char*)"A_M_M_ArmTownfolk_02",(char*)"A_F_M_FAMILYTRAVELERS_WARM_01",(char*)"CS_WROBEL",(char*)"U_M_M_CZPHOMESTEADFATHER_01",(char*)"U_F_Y_CZPHOMESTEADDAUGHTER_01",(char*)"U_M_Y_CZPHOMESTEADSON_01",(char*)"U_M_Y_CZPHOMESTEADSON_02",(char*)"U_M_Y_CZPHOMESTEADSON_03",(char*)"U_M_M_LRSHOMESTEADTENANT_01",(char*)"U_F_M_RKSHOMESTEADTENANT_01",(char*)"U_M_M_UNIEXCONFEDSBOUNTY_01",(char*)"U_M_M_WALGENERALSTOREOWNER_01",(char*)"S_M_M_TrainStationWorker_01",(char*)"p_m_zero",(char*)"P_M_THREE",(char*)"A_C_HORSE_GANG_KIERAN",(char*)"CS_Dutch",(char*)"CS_JackMarston",(char*)"CS_JACKMARSTON_TEEN",(char*)"CS_EagleFlies",(char*)"CS_Cleet",(char*)"CS_Joe",(char*)"CS_CREOLECAPTAIN",(char*)"CS_JosiahTrelawny",(char*)"CS_RevSwanson",(char*)"CS_LeoStrauss",(char*)"CS_MrPearson",(char*)"CS_Lenny",(char*)"CS_Sean",(char*)"CS_HoseaMatthews",(char*)"CS_JavierEscuella",(char*)"CS_CharlesSmith",(char*)"CS_Uncle",(char*)"CS_BillWilliamson",(char*)"CS_MicahBell",(char*)"CS_JohnMarston",(char*)"CS_AbigailRoberts",(char*)"CS_MaryBeth",(char*)"CS_MollyOshea",(char*)"CS_SusanGrimshaw",(char*)"CS_Karen",(char*)"CS_Tilly",(char*)"CS_MrsAdler",(char*)"A_M_M_HtlRoughTravellers_01",(char*)"A_M_M_SclRoughTravellers_01",(char*)"U_M_M_LnSWorker_01",(char*)"U_M_M_LnSWorker_02",(char*)"U_M_M_LnSWorker_03",(char*)"U_M_M_LnSWorker_04",(char*)"U_M_M_RKFRANCHER_01",(char*)"A_M_M_Rancher_01",(char*)"U_M_M_MFRRANCHER_01",(char*)"EA_LCMP_DOGS",(char*)"EA_LCMP_FOREMAN",(char*)"U_M_M_BiVForeman_01",(char*)"EA_HMSTD_FOREMAN",(char*)"U_M_M_HtlForeman_01",(char*)"EA_HMSTD_WORKER_1",(char*)"EA_HMSTD_WORKER_2",(char*)"EA_HMSTD_MALE",(char*)"U_M_M_HTLHusband_01",(char*)"EA_HMSTD_FEMALE",(char*)"U_F_M_HTLWife_01",(char*)"EA_RCAMP_FOREMAN",(char*)"U_M_M_RaCForeman_01",(char*)"A_M_M_NbxUpperClass_01",(char*)"A_M_M_BlWUpperClass_01",(char*)"CS_Kieran",(char*)"A_M_M_RHDTOWNFOLK_02",(char*)"S_M_M_CKTWORKER_01",(char*)"S_M_M_STRLUMBERJACK_01",(char*)"S_M_M_RaCRailWorker_01",(char*)"S_M_Y_RACRAILWORKER_01",(char*)"A_M_M_BIVWORKER_01",(char*)"A_M_M_BLWTOWNFOLK_01",(char*)"S_M_M_MAPWORKER_01",(char*)"U_M_Y_HTLWORKER_01",(char*)"U_M_Y_HTLWORKER_02",(char*)"A_M_M_ASBMINER_01",(char*)"A_M_Y_ASBMINER_01",(char*)"CS_AberdeenPigFarmer",(char*)"U_M_M_DORHOMESTEADHUSBAND_01",(char*)"A_M_M_EmRFarmHand_01",(char*)"U_M_M_EMRFATHER_01",(char*)"U_M_Y_EMRSON_01",(char*)"A_M_M_ValTownfolk_01",(char*)"A_M_M_ValFarmer_01",(char*)"A_M_M_ValTownfolk_02",(char*)"S_M_M_ValCowpoke_01",(char*)"A_M_M_ValLaborer_01",(char*)"S_M_M_LiveryWorker_01",(char*)"A_M_M_NBXDOCKWORKERS_01",(char*)"A_M_M_NBXLABORERS_01",(char*)"A_M_M_LagTownfolk_01",(char*)"A_M_M_BtcHillbilly_01",(char*)"A_M_O_BTCHILLBILLY_01",(char*)"A_M_M_VhtTownfolk_01",(char*)"CS_TomDickens",(char*)"A_M_M_STRTOWNFOLK_01",(char*)"A_M_M_RHDTOWNFOLK_01",(char*)"A_M_M_RhdForeman_01",(char*)"S_M_Y_Army_01",(char*)"A_F_M_NbxWhore_01",(char*)"A_F_M_VHTPROSTITUTE_01",(char*)"A_F_M_VALPROSTITUTE_01",(char*)"A_C_Cat_01",(char*)"A_C_CedarWaxwing_01",(char*)"A_C_Chipmunk_01",(char*)"A_C_CRAB_01",(char*)"A_C_Cormorant_01",(char*)"A_C_CarolinaParakeet_01",(char*)"A_C_DogCatahoulaCur_01",(char*)"A_C_DOGHOBO_01",(char*)"A_C_DOGHOUND_01",(char*)"A_C_DogHusky_01",(char*)"A_C_DOGRUFUS_01",(char*)"A_C_DOGBLUETICKCOONHOUND_01",(char*)"A_C_Donkey_01",(char*)"A_C_Elk_01",(char*)"A_C_FISHBULLHEADCAT_01_SM",(char*)"A_C_FISHNORTHERNPIKE_01_LG",(char*)"A_C_FISHRAINBOWTROUT_01_MS",(char*)"A_C_FISHSALMONSOCKEYE_01_MS",(char*)"A_C_GILAMONSTER_01",(char*)"A_C_Goat_01",(char*)"A_C_HORSE_AMERICANPAINT_GREYOVERO",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_PALOMINODAPPLE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_SILVERTAILBUCKSKIN",(char*)"A_C_HORSE_ANDALUSIAN_DARKBAY",(char*)"A_C_HORSE_ANDALUSIAN_ROSEGRAY",(char*)"A_C_HORSE_APPALOOSA_BROWNLEOPARD",(char*)"A_C_HORSE_APPALOOSA_LEOPARD",(char*)"A_C_HORSE_ARABIAN_BLACK",(char*)"A_C_HORSE_ARABIAN_ROSEGREYBAY",(char*)"A_C_HORSE_ARDENNES_BAYROAN",(char*)"A_C_HORSE_ARDENNES_STRAWBERRYROAN",(char*)"A_C_HORSE_BELGIAN_BLONDCHESTNUT",(char*)"A_C_HORSE_BELGIAN_MEALYCHESTNUT",(char*)"A_C_HORSE_DUTCHWARMBLOOD_CHOCOLATEROAN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SEALBROWN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SOOTYBUCKSKIN",(char*)"A_C_HORSE_HUNGARIANHALFBRED_DARKDAPPLEGREY",(char*)"A_C_HORSE_HUNGARIANHALFBRED_PIEBALDTOBIANO",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_AMBERCHAMPAGNE",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_SILVERDAPPLEPINTO",(char*)"A_C_HORSE_NOKOTA_REVERSEDAPPLEROAN",(char*)"A_C_HORSE_SHIRE_DARKBAY",(char*)"A_C_HORSE_SHIRE_LIGHTGREY",(char*)"A_C_HORSE_SUFFOLKPUNCH_SORREL",(char*)"A_C_HORSE_SUFFOLKPUNCH_REDCHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_FLAXENROAN",(char*)"A_C_HORSE_THOROUGHBRED_BRINDLE",(char*)"A_C_HORSE_TURKOMAN_DARKBAY",(char*)"A_C_HORSE_TURKOMAN_GOLD",(char*)"A_C_HORSE_TURKOMAN_SILVER",(char*)"A_C_HORSE_GANG_BILL",(char*)"A_C_HORSE_GANG_CHARLES",(char*)"A_C_HORSE_GANG_DUTCH",(char*)"A_C_HORSE_GANG_HOSEA",(char*)"A_C_HORSE_GANG_JAVIER",(char*)"A_C_HORSE_GANG_JOHN",(char*)"A_C_HORSE_GANG_KAREN",(char*)"A_C_HORSE_GANG_LENNY",(char*)"A_C_HORSE_GANG_MICAH",(char*)"A_C_HORSE_GANG_SADIE",(char*)"A_C_HORSE_GANG_SEAN",(char*)"A_C_HORSE_GANG_TRELAWNEY",(char*)"A_C_HORSE_GANG_UNCLE",(char*)"A_C_HORSE_GANG_SADIE_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_CHARLES_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_UNCLE_ENDLESSSUMMER",(char*)"A_C_HORSE_AMERICANPAINT_OVERO",(char*)"A_C_HORSE_AMERICANPAINT_TOBIANO",(char*)"A_C_HORSE_AMERICANPAINT_SPLASHEDWHITE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BLACK",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BUCKSKIN",(char*)"A_C_HORSE_APPALOOSA_BLANKET",(char*)"A_C_HORSE_APPALOOSA_LEOPARDBLANKET",(char*)"A_C_HORSE_ARABIAN_WHITE",(char*)"A_C_HORSE_HUNGARIANHALFBRED_FLAXENCHESTNUT",(char*)"A_C_HORSE_MUSTANG_GRULLODUN",(char*)"A_C_HORSE_MUSTANG_WILDBAY",(char*)"A_C_HORSE_MUSTANG_TIGERSTRIPEDBAY",(char*)"A_C_HORSE_NOKOTA_BLUEROAN",(char*)"A_C_HORSE_NOKOTA_WHITEROAN",(char*)"A_C_HORSE_THOROUGHBRED_BLOODBAY",(char*)"A_C_HORSE_THOROUGHBRED_DAPPLEGREY",(char*)"A_C_HorseMule_01",(char*)"A_C_Ox_01",(char*)"A_C_REDFOOTEDBOOBY_01",(char*)"A_C_SHARKHAMMERHEAD_01",(char*)"A_C_SharkTiger",(char*)"A_C_SHEEP_01",(char*)"A_C_SNAKEBLACKTAILRATTLE_01",(char*)"A_C_snakeferdelance_01",(char*)"A_C_ROSEATESPOONBILL_01",(char*)"A_C_TURKEY_01",(char*)"A_C_WOODPECKER_02",(char*)"A_C_PIG_01",(char*)"A_C_SNAKEWATER_01",(char*)"A_C_SPARROW_01",(char*)"A_C_Egret_01",(char*)"A_C_PARROT_01",(char*)"A_C_PRAIRIECHICKEN_01",(char*)"A_C_BAT_01",(char*)"A_C_Heron_01",(char*)"A_C_Oriole_01",(char*)"A_C_BLUEJAY_01",(char*)"A_C_CARDINAL_01",(char*)"A_C_Duck_01",(char*)"A_C_PIGEON",(char*)"A_C_ROBIN_01",(char*)"A_C_SONGBIRD_01",(char*)"A_C_Woodpecker_01",(char*)"A_C_IGUANA_01",(char*)"A_C_ROOSTER_01",(char*)"A_C_SNAKEFERDELANCE_01",(char*)"A_C_Armadillo_01",(char*)"A_C_IGUANADESERT_01",(char*)"A_C_Quail_01",(char*)"A_C_SNAKEREDBOA_01",(char*)"A_C_Chicken_01",(char*)"A_C_FROGBULL_01",(char*)"A_C_SQUIRREL_01",(char*)"A_C_Toad_01",(char*)"A_C_TURTLESNAPPING_01",(char*)"A_C_Bull_01",(char*)"A_C_Cow",(char*)"A_C_MUSKRAT_01",(char*)"A_C_RAT_01",(char*)"A_C_SNAKE_01",(char*)"A_C_Beaver_01",(char*)"A_C_PHEASANT_01",(char*)"A_C_Rabbit_01",(char*)"A_C_TurkeyWild_01",(char*)"A_C_BigHornRam_01",(char*)"A_C_Buck_01",(char*)"A_C_Buffalo_01",(char*)"A_C_DEER_01",(char*)"A_C_ELK_01",(char*)"A_C_MOOSE_01",(char*)"A_C_ProngHorn_01",(char*)"A_C_HORSE_MORGAN_BAY",(char*)"A_C_HORSE_MORGAN_BAYROAN",(char*)"A_C_HORSE_MORGAN_FLAXENCHESTNUT",(char*)"A_C_HORSE_MORGAN_PALOMINO",(char*)"A_C_HORSE_KENTUCKYSADDLE_BLACK",(char*)"A_C_HORSE_KENTUCKYSADDLE_CHESTNUTPINTO",(char*)"A_C_HORSE_KENTUCKYSADDLE_GREY",(char*)"A_C_HORSE_KENTUCKYSADDLE_SILVERBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_BLACKRABICANO",(char*)"A_C_HORSE_TENNESSEEWALKER_CHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_DAPPLEBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_REDROAN",(char*)"A_C_BEAR_01",(char*)"A_C_DOGAMERICANFOXHOUND_01",(char*)"A_C_DOGCOLLIE_01",(char*)"A_C_DogAustralianSheperd_01",(char*)"A_C_DogChesBayRetriever_01",(char*)"A_C_DogLab_01",(char*)"A_C_DogStreet_01",(char*)"A_C_DogBlueTickCoonHound_01",(char*)"A_C_DogHound_01",(char*)"A_C_TURTLESEA_01",(char*)"A_C_HAWK_01",(char*)"A_C_DUCK_01",(char*)"A_C_LOON_01",(char*)"A_C_CORMORANT_01",(char*)"A_C_GOOSECANADA_01",(char*)"A_C_HERON_01",(char*)"A_C_EGRET_01",(char*)"A_C_CraneWhooping_01",(char*)"A_C_PELICAN_01",(char*)"A_C_Badger_01",(char*)"A_C_BOAR_01",(char*)"A_C_JAVELINA_01",(char*)"A_C_OWL_01",(char*)"A_C_PANTHER_01",(char*)"A_C_POSSUM_01",(char*)"A_C_Raccoon_01",(char*)"A_C_SKUNK_01",(char*)"A_C_Alligator_01",(char*)"A_C_Alligator_02",(char*)"A_C_Alligator_03",(char*)"A_C_Bear_01",(char*)"A_C_BearBlack_01",(char*)"A_C_Boar_01",(char*)"A_C_CALIFORNIACONDOR_01",(char*)"A_C_Cougar_01",(char*)"A_C_Coyote_01",(char*)"A_C_Crow_01",(char*)"A_C_Eagle_01",(char*)"A_C_Fox_01",(char*)"A_C_Hawk_01",(char*)"A_C_Owl_01",(char*)"A_C_Panther_01",(char*)"A_C_RAVEN_01",(char*)"A_C_SEAGULL_01",(char*)"A_C_Vulture_01",(char*)"A_C_Wolf",(char*)"A_C_Wolf_Medium",(char*)"A_C_Wolf_Small",(char*)"A_C_FishMuskie_01_lg",(char*)"A_C_FISHLAKESTURGEON_01_LG",(char*)"A_C_FISHLONGNOSEGAR_01_LG",(char*)"A_C_FISHCHANNELCATFISH_01_LG",(char*)"A_C_FISHBLUEGIL_01_SM",(char*)"A_C_FISHPERCH_01_SM",(char*)"A_C_FISHCHAINPICKEREL_01_SM",(char*)"A_C_FISHROCKBASS_01_SM",(char*)"A_C_FISHREDFINPICKEREL_01_SM",(char*)"A_C_FISHBLUEGIL_01_MS",(char*)"A_C_FISHPERCH_01_MS",(char*)"A_C_FISHREDFINPICKEREL_01_MS",(char*)"A_C_FISHROCKBASS_01_MS",(char*)"A_C_FISHSMALLMOUTHBASS_01_MS",(char*)"A_C_FISHBULLHEADCAT_01_MS",(char*)"A_C_FISHLARGEMOUTHBASS_01_MS" };
		int optionClicked = simpleMenu.clickOption();
		switch (optionClicked) {
		case 0:
			KeyboardInput([](auto input) {
				lastVehicleInput = input;
				SpawnPed(input, [](auto ped) {
					int myPed = PLAYER::PLAYER_PED_ID();
					//Native::Invoke<int>(N::SET_PED_INTO_VEHICLE, myPed, ped, -1);
					if (forge::enabled)
						forge::objectSelected = ped;
				});
			}, lastVehicleInput);
			break;
		default:
			if (optionClicked != -1) {
				SpawnPed(pedsArray[optionClicked - 1], [](auto ped) {
					//int myPed = PLAYER::PLAYER_PED_ID();
					//Native::Invoke<int>(N::SET_PED_INTO_VEHICLE, myPed, ped, -1);
					//changePedMount(ped);
					if (forge::enabled)
						forge::objectSelected = ped;
					/*bool mis = Native::Invoke<bool>(N::IS_ENTITY_A_MISSION_ENTITY, ped);
					if (mis) {
						print((char*)"IT IS MISSION ENTITY");
					}
					else {
						print((char*)"IT IS NOT MISSION ENTITY");
					}*/
				});
			}
			break;
		}
	}
	break;

	case MENU_CHANGE_PED:
	{
		static char *lastVehicleInput = (char*)"A_F_M_ARMCHOLERACORPSE_01";
		const char *pedsArray[] = { (char*)"CS_BILLWILLIAMSON",(char*)"CS_MICAHBELL",(char*)"CS_UNCLE",(char*)"CS_SEAN",(char*)"CS_KIERAN",(char*)"A_M_M_WapWarriors_01",(char*)"A_M_O_WapTownfolk_01",(char*)"A_F_O_WapTownfolk_01",(char*)"U_M_M_WAPOFFICIAL_01",(char*)"A_M_M_WAPWARRIORS_01",(char*)"A_M_O_WAPTOWNFOLK_01",(char*)"S_M_M_UNITRAINENGINEER_01",(char*)"S_M_M_UNITRAINGUARDS_01",(char*)"A_M_M_MiddleTrainPassengers_01",(char*)"A_F_M_MiddleTrainPassengers_01",(char*)"A_M_M_LowerTrainPassengers_01",(char*)"A_F_M_LowerTrainPassengers_01",(char*)"A_M_M_UpperTrainPassengers_01",(char*)"A_F_M_UpperTrainPassengers_01",(char*)"A_M_M_NBXUPPERCLASS_01",(char*)"A_F_M_NBXUPPERCLASS_01",(char*)"G_M_M_UNIDUSTER_02",(char*)"G_M_M_UNIDUSTER_03",(char*)"CS_EDMUNDLOWRY",(char*)"G_M_M_UniBanditos_01",(char*)"CS_POISONWELLSHAMAN",(char*)"S_M_M_RACRAILGUARDS_01",(char*)"U_M_M_valbarber_01",(char*)"U_M_M_ValBartender_01",(char*)"U_M_M_ValDoctor_01",(char*)"U_M_M_ValGunsmith_01",(char*)"CS_VALSHERIFF",(char*)"G_M_M_UNIDUSTER_01",(char*)"S_M_M_SDTICKETSELLER_01",(char*)"S_M_M_MagicLantern_01",(char*)"CS_GrizzledJon",(char*)"A_M_M_VALCRIMINALS_01",(char*)"A_M_M_ASBMINER_02",(char*)"A_M_Y_ASBMINER_02",(char*)"A_M_M_ASBMINER_04",(char*)"A_M_Y_ASBMINER_04",(char*)"A_M_M_ASBMINER_03",(char*)"A_M_Y_ASBMINER_03",(char*)"A_M_M_JamesonGuard_01",(char*)"U_M_O_ASBSHERIFF_01",(char*)"A_M_M_RkrFancyDRIVERS_01",(char*)"A_M_M_RkrFancyTravellers_01",(char*)"A_F_M_RkrFancyTravellers_01",(char*)"S_M_M_CornwallGuard_01",(char*)"G_M_M_UniCornwallGoons_01",(char*)"U_M_M_CRDHOMESTEADTENANT_01",(char*)"U_M_M_CRDHOMESTEADTENANT_02",(char*)"U_M_O_PSHRANCHER_01",(char*)"U_M_O_CAJHOMESTEAD_01",(char*)"U_M_Y_CAJHOMESTEAD_01",(char*)"U_M_M_CAJHOMESTEAD_01",(char*)"CS_DOROETHEAWICKLOW",(char*)"S_M_M_AmbientSDPolice_01",(char*)"A_M_M_SDObeseMen_01",(char*)"A_F_M_SDObeseWomen_01",(char*)"U_M_M_NBXShadyDealer_01",(char*)"A_F_M_WapTownfolk_01",(char*)"A_M_M_SDLABORERS_02",(char*)"A_M_Y_NBXSTREETKIDS_01",(char*)"A_M_M_LowerSDTownfolk_02",(char*)"A_M_M_LowerSDTownfolk_01",(char*)"A_M_Y_NbxStreetKids_Slums_01",(char*)"A_M_Y_SDStreetKids_Slums_02",(char*)"A_M_M_SDDockWorkers_02",(char*)"A_M_M_SDDockForeman_01",(char*)"A_F_M_NbxSlums_01",(char*)"A_F_M_SDSlums_02",(char*)"A_M_M_NbxSlums_01",(char*)"A_M_M_SDSlums_02",(char*)"A_M_O_SDUpperClass_01",(char*)"A_F_M_NbxUpperClass_01",(char*)"A_F_O_SDUpperClass_01",(char*)"A_M_M_MiddleSDTownfolk_01",(char*)"A_M_M_MiddleSDTownfolk_02",(char*)"A_M_M_MiddleSDTownfolk_03",(char*)"A_F_M_MiddleSDTownfolk_01",(char*)"A_F_M_MiddleSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_01",(char*)"A_F_M_LowerSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_03",(char*)"A_F_M_SDChinatown_01",(char*)"A_F_O_SDChinatown_01",(char*)"A_F_M_SDFancyWhore_01",(char*)"G_M_M_UNIBRONTEGOONS_01",(char*)"A_M_M_SDChinatown_01",(char*)"A_M_O_SDChinatown_01",(char*)"U_M_M_VALGUNSMITH_01",(char*)"U_M_M_VALGENSTOREOWNER_01",(char*)"U_M_M_ValButcher_01",(char*)"A_M_M_VALTOWNFOLK_01",(char*)"U_M_M_VALDOCTOR_01",(char*)"S_M_M_BANKCLERK_01",(char*)"U_M_M_ValHotelOwner_01",(char*)"U_M_M_VALBARTENDER_01",(char*)"U_M_M_VALBARBER_01",(char*)"U_M_O_ValBartender_01",(char*)"U_M_M_NBXBARTENDER_02",(char*)"A_M_M_BLWUPPERCLASS_01",(char*)"U_M_M_NBXBARTENDER_01",(char*)"A_M_M_MIDDLESDTOWNFOLK_01",(char*)"U_M_M_NBXGENERALSTOREOWNER_01",(char*)"U_M_M_NBXSHADYDEALER_01",(char*)"S_M_M_TAILOR_01",(char*)"U_M_M_NBXGUNSMITH_01",(char*)"CS_SDDOCTOR_01",(char*)"S_M_M_LIVERYWORKER_01",(char*)"S_M_M_MarketVendor_01",(char*)"U_M_M_SDPhotographer_01",(char*)"U_M_M_STRGENSTOREOWNER_01",(char*)"U_M_M_STRFREIGHTSTATIONOWNER_01",(char*)"U_M_M_ASBGUNSMITH_01",(char*)"U_M_M_SDTRAPPER_01",(char*)"S_M_M_TRAINSTATIONWORKER_01",(char*)"A_F_M_LAGTOWNFOLK_01",(char*)"S_M_M_SKPGUARD_01",(char*)"A_M_M_SKPPRISONER_01",(char*)"U_F_M_LagMother_01",(char*)"A_F_O_LagTownfolk_01",(char*)"A_M_O_LagTownfolk_01",(char*)"A_M_M_RHDFOREMAN_01",(char*)"U_M_M_ORPGUARD_01",(char*)"CS_ABE",(char*)"CS_DavidGeddes",(char*)"CS_ANGUSGEDDES",(char*)"CS_DUNCANGEDDES",(char*)"CS_MRSGEDDES",(char*)"G_M_M_UNIMOUNTAINMEN_01",(char*)"A_F_M_LagTownfolk_01",(char*)"CS_ThomasDown",(char*)"CS_ArchieDown",(char*)"CS_EDITHDOWN",(char*)"S_F_M_MaPWorker_01",(char*)"A_M_M_StrTownfolk_01",(char*)"A_M_M_STRLABORER_01",(char*)"A_M_M_BynRoughTravellers_01",(char*)"A_C_DOGPOODLE_01",(char*)"S_M_M_RHDCOWPOKE_01",(char*)"S_M_M_UniButchers_01",(char*)"S_M_Y_NewspaperBoy_01",(char*)"A_F_M_RhdUpperClass_01",(char*)"A_F_M_RhdTownfolk_02",(char*)"a_m_m_rhdtownfolk_01_laborer",(char*)"A_F_M_RHDTOWNFOLK_01",(char*)"U_M_M_RHDBARTENDER_01",(char*)"U_M_M_RHDGUNSMITH_01",(char*)"U_M_M_RHDGENSTOREOWNER_01",(char*)"U_M_M_RHDGENSTOREOWNER_02",(char*)"U_M_M_RHDSHERIFF_01",(char*)"U_M_M_RHDTRAINSTATIONWORKER_01",(char*)"A_F_M_RHDPROSTITUTE_01",(char*)"A_M_M_RHDObeseMen_01",(char*)"A_M_M_RhdUpperClass_01",(char*)"A_M_M_RkrRoughTravellers_01",(char*)"A_C_DOGCATAHOULACUR_01",(char*)"A_F_M_VHTTOWNFOLK_01",(char*)"A_M_M_VHTTHUG_01",(char*)"U_M_M_VHTSTATIONCLERK_01",(char*)"U_M_O_VHTEXOTICSHOPKEEPER_01",(char*)"U_F_M_VhTBartender_01",(char*)"A_M_M_HtlFancyDRIVERS_01",(char*)"A_M_M_HtlFancyTravellers_01",(char*)"A_F_M_HtlFancyTravellers_01",(char*)"S_M_M_BARBER_01",(char*)"S_M_M_UNIBUTCHERS_01",(char*)"S_M_M_BLWCOWPOKE_01",(char*)"A_M_M_BLWObeseMen_01",(char*)"A_M_O_BlWUpperClass_01",(char*)"A_F_M_BlWUpperClass_01",(char*)"A_F_O_BlWUpperClass_01",(char*)"A_M_M_BlWLaborer_02",(char*)"A_M_M_BlWLaborer_01",(char*)"A_M_M_BlWForeman_01",(char*)"A_F_M_BlWTownfolk_02",(char*)"A_F_M_BlWTownfolk_01",(char*)"S_M_M_AmbientBlWPolice_01",(char*)"S_M_M_FussarHenchman_01",(char*)"A_F_M_GuaTownfolk_01",(char*)"A_F_O_GuaTownfolk_01",(char*)"A_M_M_GuaTownfolk_01",(char*)"A_M_O_GuaTownfolk_01",(char*)"CS_NicholasTimmins",(char*)"A_M_M_STRFANCYTOURIST_01",(char*)"A_F_M_BiVFancyTravellers_01",(char*)"A_F_M_STRTOWNFOLK_01",(char*)"A_M_M_UNIGUNSLINGER_01",(char*)"U_M_M_StrGenStoreOwner_01",(char*)"U_M_M_STRWELCOMECENTER_01",(char*)"U_M_M_STRSHERRIFF_01",(char*)"U_M_M_StrFreightStationOwner_01",(char*)"G_M_M_UNICRIMINALS_01",(char*)"G_M_M_UNICRIMINALS_02",(char*)"A_M_M_BTCObeseMen_01",(char*)"A_F_M_BTCObeseWomen_01",(char*)"A_M_M_AsbTownfolk_01",(char*)"A_F_M_AsbTownfolk_01",(char*)"A_F_M_BTCHILLBILLY_01",(char*)"A_F_O_BTCHILLBILLY_01",(char*)"A_M_M_BTCHILLBILLY_01",(char*)"G_M_M_UniBraithwaites_01",(char*)"U_M_O_CmrCivilwarcommando_01",(char*)"S_M_M_Army_01",(char*)"U_M_M_BWMSTABLEHAND_01",(char*)"S_M_M_CGHWORKER_01",(char*)"G_M_M_UniGrays_01",(char*)"G_M_O_UniExConfeds_01",(char*)"G_M_Y_UniExConfeds_01",(char*)"G_M_M_UNIINBRED_01",(char*)"CS_AberdeenSister",(char*)"A_M_M_GriRoughTravellers_01",(char*)"A_F_M_ValTownfolk_01",(char*)"U_F_O_WTCTOWNFOLK_01",(char*)"U_M_M_BLWTRAINSTATIONWORKER_01",(char*)"CS_BLWPHOTOGRAPHER",(char*)"A_M_M_BlWTownfolk_01",(char*)"U_M_O_BLWPOLICECHIEF_01",(char*)"S_M_M_Tailor_01",(char*)"S_M_M_BankClerk_01",(char*)"U_M_O_BLWBARTENDER_01",(char*)"U_M_O_BLWGENERALSTOREOWNER_01",(char*)"S_M_M_TumDeputies_01",(char*)"CS_SHERIFFFREEMAN",(char*)"A_M_M_NEAROUGHTRAVELLERS_01",(char*)"A_M_M_TumTownfolk_01",(char*)"A_F_M_TumTownfolk_01",(char*)"A_M_M_TumTownfolk_02",(char*)"A_F_M_TumTownfolk_02",(char*)"U_M_M_TUMBARTENDER_01",(char*)"U_M_M_TUMBUTCHER_01",(char*)"U_M_M_TUMGUNSMITH_01",(char*)"U_F_M_TUMGENERALSTOREOWNER_01",(char*)"A_M_M_ARMTOWNFOLK_01",(char*)"A_M_M_RANCHERTRAVELERS_WARM_01",(char*)"S_M_M_AmbientLawRural_01",(char*)"A_M_M_ArmCholeraCorpse_01",(char*)"A_F_M_ArmCholeraCorpse_01",(char*)"U_M_M_ARMGENERALSTOREOWNER_01",(char*)"U_M_O_ARMBARTENDER_01",(char*)"U_M_M_ARMTRAINSTATIONWORKER_01",(char*)"U_M_M_ARMUNDERTAKER_01",(char*)"RE_DEADBODIES_MALE",(char*)"RE_DEADBODIES_MALES_01",(char*)"A_F_M_ArmTownfolk_01",(char*)"A_F_M_ArmTownfolk_02",(char*)"A_M_M_ArmTownfolk_01",(char*)"A_M_M_ArmTownfolk_02",(char*)"A_F_M_FAMILYTRAVELERS_WARM_01",(char*)"CS_WROBEL",(char*)"U_M_M_CZPHOMESTEADFATHER_01",(char*)"U_F_Y_CZPHOMESTEADDAUGHTER_01",(char*)"U_M_Y_CZPHOMESTEADSON_01",(char*)"U_M_Y_CZPHOMESTEADSON_02",(char*)"U_M_Y_CZPHOMESTEADSON_03",(char*)"U_M_M_LRSHOMESTEADTENANT_01",(char*)"U_F_M_RKSHOMESTEADTENANT_01",(char*)"U_M_M_UNIEXCONFEDSBOUNTY_01",(char*)"U_M_M_WALGENERALSTOREOWNER_01",(char*)"S_M_M_TrainStationWorker_01",(char*)"p_m_zero",(char*)"P_M_THREE",(char*)"A_C_HORSE_GANG_KIERAN",(char*)"CS_Dutch",(char*)"CS_JackMarston",(char*)"CS_JACKMARSTON_TEEN",(char*)"CS_EagleFlies",(char*)"CS_Cleet",(char*)"CS_Joe",(char*)"CS_CREOLECAPTAIN",(char*)"CS_JosiahTrelawny",(char*)"CS_RevSwanson",(char*)"CS_LeoStrauss",(char*)"CS_MrPearson",(char*)"CS_Lenny",(char*)"CS_Sean",(char*)"CS_HoseaMatthews",(char*)"CS_JavierEscuella",(char*)"CS_CharlesSmith",(char*)"CS_Uncle",(char*)"CS_BillWilliamson",(char*)"CS_MicahBell",(char*)"CS_JohnMarston",(char*)"CS_AbigailRoberts",(char*)"CS_MaryBeth",(char*)"CS_MollyOshea",(char*)"CS_SusanGrimshaw",(char*)"CS_Karen",(char*)"CS_Tilly",(char*)"CS_MrsAdler",(char*)"A_M_M_HtlRoughTravellers_01",(char*)"A_M_M_SclRoughTravellers_01",(char*)"U_M_M_LnSWorker_01",(char*)"U_M_M_LnSWorker_02",(char*)"U_M_M_LnSWorker_03",(char*)"U_M_M_LnSWorker_04",(char*)"U_M_M_RKFRANCHER_01",(char*)"A_M_M_Rancher_01",(char*)"U_M_M_MFRRANCHER_01",(char*)"EA_LCMP_DOGS",(char*)"EA_LCMP_FOREMAN",(char*)"U_M_M_BiVForeman_01",(char*)"EA_HMSTD_FOREMAN",(char*)"U_M_M_HtlForeman_01",(char*)"EA_HMSTD_WORKER_1",(char*)"EA_HMSTD_WORKER_2",(char*)"EA_HMSTD_MALE",(char*)"U_M_M_HTLHusband_01",(char*)"EA_HMSTD_FEMALE",(char*)"U_F_M_HTLWife_01",(char*)"EA_RCAMP_FOREMAN",(char*)"U_M_M_RaCForeman_01",(char*)"A_M_M_NbxUpperClass_01",(char*)"A_M_M_BlWUpperClass_01",(char*)"CS_Kieran",(char*)"A_M_M_RHDTOWNFOLK_02",(char*)"S_M_M_CKTWORKER_01",(char*)"S_M_M_STRLUMBERJACK_01",(char*)"S_M_M_RaCRailWorker_01",(char*)"S_M_Y_RACRAILWORKER_01",(char*)"A_M_M_BIVWORKER_01",(char*)"A_M_M_BLWTOWNFOLK_01",(char*)"S_M_M_MAPWORKER_01",(char*)"U_M_Y_HTLWORKER_01",(char*)"U_M_Y_HTLWORKER_02",(char*)"A_M_M_ASBMINER_01",(char*)"A_M_Y_ASBMINER_01",(char*)"CS_AberdeenPigFarmer",(char*)"U_M_M_DORHOMESTEADHUSBAND_01",(char*)"A_M_M_EmRFarmHand_01",(char*)"U_M_M_EMRFATHER_01",(char*)"U_M_Y_EMRSON_01",(char*)"A_M_M_ValTownfolk_01",(char*)"A_M_M_ValFarmer_01",(char*)"A_M_M_ValTownfolk_02",(char*)"S_M_M_ValCowpoke_01",(char*)"A_M_M_ValLaborer_01",(char*)"S_M_M_LiveryWorker_01",(char*)"A_M_M_NBXDOCKWORKERS_01",(char*)"A_M_M_NBXLABORERS_01",(char*)"A_M_M_LagTownfolk_01",(char*)"A_M_M_BtcHillbilly_01",(char*)"A_M_O_BTCHILLBILLY_01",(char*)"A_M_M_VhtTownfolk_01",(char*)"CS_TomDickens",(char*)"A_M_M_STRTOWNFOLK_01",(char*)"A_M_M_RHDTOWNFOLK_01",(char*)"A_M_M_RhdForeman_01",(char*)"S_M_Y_Army_01",(char*)"A_F_M_NbxWhore_01",(char*)"A_F_M_VHTPROSTITUTE_01",(char*)"A_F_M_VALPROSTITUTE_01",(char*)"A_C_Cat_01",(char*)"A_C_CedarWaxwing_01",(char*)"A_C_Chipmunk_01",(char*)"A_C_CRAB_01",(char*)"A_C_Cormorant_01",(char*)"A_C_CarolinaParakeet_01",(char*)"A_C_DogCatahoulaCur_01",(char*)"A_C_DOGHOBO_01",(char*)"A_C_DOGHOUND_01",(char*)"A_C_DogHusky_01",(char*)"A_C_DOGRUFUS_01",(char*)"A_C_DOGBLUETICKCOONHOUND_01",(char*)"A_C_Donkey_01",(char*)"A_C_Elk_01",(char*)"A_C_FISHBULLHEADCAT_01_SM",(char*)"A_C_FISHNORTHERNPIKE_01_LG",(char*)"A_C_FISHRAINBOWTROUT_01_MS",(char*)"A_C_FISHSALMONSOCKEYE_01_MS",(char*)"A_C_GILAMONSTER_01",(char*)"A_C_Goat_01",(char*)"A_C_HORSE_AMERICANPAINT_GREYOVERO",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_PALOMINODAPPLE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_SILVERTAILBUCKSKIN",(char*)"A_C_HORSE_ANDALUSIAN_DARKBAY",(char*)"A_C_HORSE_ANDALUSIAN_ROSEGRAY",(char*)"A_C_HORSE_APPALOOSA_BROWNLEOPARD",(char*)"A_C_HORSE_APPALOOSA_LEOPARD",(char*)"A_C_HORSE_ARABIAN_BLACK",(char*)"A_C_HORSE_ARABIAN_ROSEGREYBAY",(char*)"A_C_HORSE_ARDENNES_BAYROAN",(char*)"A_C_HORSE_ARDENNES_STRAWBERRYROAN",(char*)"A_C_HORSE_BELGIAN_BLONDCHESTNUT",(char*)"A_C_HORSE_BELGIAN_MEALYCHESTNUT",(char*)"A_C_HORSE_DUTCHWARMBLOOD_CHOCOLATEROAN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SEALBROWN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SOOTYBUCKSKIN",(char*)"A_C_HORSE_HUNGARIANHALFBRED_DARKDAPPLEGREY",(char*)"A_C_HORSE_HUNGARIANHALFBRED_PIEBALDTOBIANO",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_AMBERCHAMPAGNE",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_SILVERDAPPLEPINTO",(char*)"A_C_HORSE_NOKOTA_REVERSEDAPPLEROAN",(char*)"A_C_HORSE_SHIRE_DARKBAY",(char*)"A_C_HORSE_SHIRE_LIGHTGREY",(char*)"A_C_HORSE_SUFFOLKPUNCH_SORREL",(char*)"A_C_HORSE_SUFFOLKPUNCH_REDCHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_FLAXENROAN",(char*)"A_C_HORSE_THOROUGHBRED_BRINDLE",(char*)"A_C_HORSE_TURKOMAN_DARKBAY",(char*)"A_C_HORSE_TURKOMAN_GOLD",(char*)"A_C_HORSE_TURKOMAN_SILVER",(char*)"A_C_HORSE_GANG_BILL",(char*)"A_C_HORSE_GANG_CHARLES",(char*)"A_C_HORSE_GANG_DUTCH",(char*)"A_C_HORSE_GANG_HOSEA",(char*)"A_C_HORSE_GANG_JAVIER",(char*)"A_C_HORSE_GANG_JOHN",(char*)"A_C_HORSE_GANG_KAREN",(char*)"A_C_HORSE_GANG_LENNY",(char*)"A_C_HORSE_GANG_MICAH",(char*)"A_C_HORSE_GANG_SADIE",(char*)"A_C_HORSE_GANG_SEAN",(char*)"A_C_HORSE_GANG_TRELAWNEY",(char*)"A_C_HORSE_GANG_UNCLE",(char*)"A_C_HORSE_GANG_SADIE_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_CHARLES_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_UNCLE_ENDLESSSUMMER",(char*)"A_C_HORSE_AMERICANPAINT_OVERO",(char*)"A_C_HORSE_AMERICANPAINT_TOBIANO",(char*)"A_C_HORSE_AMERICANPAINT_SPLASHEDWHITE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BLACK",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BUCKSKIN",(char*)"A_C_HORSE_APPALOOSA_BLANKET",(char*)"A_C_HORSE_APPALOOSA_LEOPARDBLANKET",(char*)"A_C_HORSE_ARABIAN_WHITE",(char*)"A_C_HORSE_HUNGARIANHALFBRED_FLAXENCHESTNUT",(char*)"A_C_HORSE_MUSTANG_GRULLODUN",(char*)"A_C_HORSE_MUSTANG_WILDBAY",(char*)"A_C_HORSE_MUSTANG_TIGERSTRIPEDBAY",(char*)"A_C_HORSE_NOKOTA_BLUEROAN",(char*)"A_C_HORSE_NOKOTA_WHITEROAN",(char*)"A_C_HORSE_THOROUGHBRED_BLOODBAY",(char*)"A_C_HORSE_THOROUGHBRED_DAPPLEGREY",(char*)"A_C_HorseMule_01",(char*)"A_C_Ox_01",(char*)"A_C_REDFOOTEDBOOBY_01",(char*)"A_C_SHARKHAMMERHEAD_01",(char*)"A_C_SharkTiger",(char*)"A_C_SHEEP_01",(char*)"A_C_SNAKEBLACKTAILRATTLE_01",(char*)"A_C_snakeferdelance_01",(char*)"A_C_ROSEATESPOONBILL_01",(char*)"A_C_TURKEY_01",(char*)"A_C_WOODPECKER_02",(char*)"A_C_PIG_01",(char*)"A_C_SNAKEWATER_01",(char*)"A_C_SPARROW_01",(char*)"A_C_Egret_01",(char*)"A_C_PARROT_01",(char*)"A_C_PRAIRIECHICKEN_01",(char*)"A_C_BAT_01",(char*)"A_C_Heron_01",(char*)"A_C_Oriole_01",(char*)"A_C_BLUEJAY_01",(char*)"A_C_CARDINAL_01",(char*)"A_C_Duck_01",(char*)"A_C_PIGEON",(char*)"A_C_ROBIN_01",(char*)"A_C_SONGBIRD_01",(char*)"A_C_Woodpecker_01",(char*)"A_C_IGUANA_01",(char*)"A_C_ROOSTER_01",(char*)"A_C_SNAKEFERDELANCE_01",(char*)"A_C_Armadillo_01",(char*)"A_C_IGUANADESERT_01",(char*)"A_C_Quail_01",(char*)"A_C_SNAKEREDBOA_01",(char*)"A_C_Chicken_01",(char*)"A_C_FROGBULL_01",(char*)"A_C_SQUIRREL_01",(char*)"A_C_Toad_01",(char*)"A_C_TURTLESNAPPING_01",(char*)"A_C_Bull_01",(char*)"A_C_Cow",(char*)"A_C_MUSKRAT_01",(char*)"A_C_RAT_01",(char*)"A_C_SNAKE_01",(char*)"A_C_Beaver_01",(char*)"A_C_PHEASANT_01",(char*)"A_C_Rabbit_01",(char*)"A_C_TurkeyWild_01",(char*)"A_C_BigHornRam_01",(char*)"A_C_Buck_01",(char*)"A_C_Buffalo_01",(char*)"A_C_DEER_01",(char*)"A_C_ELK_01",(char*)"A_C_MOOSE_01",(char*)"A_C_ProngHorn_01",(char*)"A_C_HORSE_MORGAN_BAY",(char*)"A_C_HORSE_MORGAN_BAYROAN",(char*)"A_C_HORSE_MORGAN_FLAXENCHESTNUT",(char*)"A_C_HORSE_MORGAN_PALOMINO",(char*)"A_C_HORSE_KENTUCKYSADDLE_BLACK",(char*)"A_C_HORSE_KENTUCKYSADDLE_CHESTNUTPINTO",(char*)"A_C_HORSE_KENTUCKYSADDLE_GREY",(char*)"A_C_HORSE_KENTUCKYSADDLE_SILVERBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_BLACKRABICANO",(char*)"A_C_HORSE_TENNESSEEWALKER_CHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_DAPPLEBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_REDROAN",(char*)"A_C_BEAR_01",(char*)"A_C_DOGAMERICANFOXHOUND_01",(char*)"A_C_DOGCOLLIE_01",(char*)"A_C_DogAustralianSheperd_01",(char*)"A_C_DogChesBayRetriever_01",(char*)"A_C_DogLab_01",(char*)"A_C_DogStreet_01",(char*)"A_C_DogBlueTickCoonHound_01",(char*)"A_C_DogHound_01",(char*)"A_C_TURTLESEA_01",(char*)"A_C_HAWK_01",(char*)"A_C_DUCK_01",(char*)"A_C_LOON_01",(char*)"A_C_CORMORANT_01",(char*)"A_C_GOOSECANADA_01",(char*)"A_C_HERON_01",(char*)"A_C_EGRET_01",(char*)"A_C_CraneWhooping_01",(char*)"A_C_PELICAN_01",(char*)"A_C_Badger_01",(char*)"A_C_BOAR_01",(char*)"A_C_JAVELINA_01",(char*)"A_C_OWL_01",(char*)"A_C_PANTHER_01",(char*)"A_C_POSSUM_01",(char*)"A_C_Raccoon_01",(char*)"A_C_SKUNK_01",(char*)"A_C_Alligator_01",(char*)"A_C_Alligator_02",(char*)"A_C_Alligator_03",(char*)"A_C_Bear_01",(char*)"A_C_BearBlack_01",(char*)"A_C_Boar_01",(char*)"A_C_CALIFORNIACONDOR_01",(char*)"A_C_Cougar_01",(char*)"A_C_Coyote_01",(char*)"A_C_Crow_01",(char*)"A_C_Eagle_01",(char*)"A_C_Fox_01",(char*)"A_C_Hawk_01",(char*)"A_C_Owl_01",(char*)"A_C_Panther_01",(char*)"A_C_RAVEN_01",(char*)"A_C_SEAGULL_01",(char*)"A_C_Vulture_01",(char*)"A_C_Wolf",(char*)"A_C_Wolf_Medium",(char*)"A_C_Wolf_Small",(char*)"A_C_FishMuskie_01_lg",(char*)"A_C_FISHLAKESTURGEON_01_LG",(char*)"A_C_FISHLONGNOSEGAR_01_LG",(char*)"A_C_FISHCHANNELCATFISH_01_LG",(char*)"A_C_FISHBLUEGIL_01_SM",(char*)"A_C_FISHPERCH_01_SM",(char*)"A_C_FISHCHAINPICKEREL_01_SM",(char*)"A_C_FISHROCKBASS_01_SM",(char*)"A_C_FISHREDFINPICKEREL_01_SM",(char*)"A_C_FISHBLUEGIL_01_MS",(char*)"A_C_FISHPERCH_01_MS",(char*)"A_C_FISHREDFINPICKEREL_01_MS",(char*)"A_C_FISHROCKBASS_01_MS",(char*)"A_C_FISHSMALLMOUTHBASS_01_MS",(char*)"A_C_FISHBULLHEADCAT_01_MS",(char*)"A_C_FISHLARGEMOUTHBASS_01_MS" };
		int optionClicked = simpleMenu.clickOption();
		switch (optionClicked) {
		case 0:
			KeyboardInput([](auto input) {
				lastVehicleInput = input;
				ChangePlayerModel(input);
			}, lastVehicleInput);
			break;
		default:
			if (optionClicked != -1) {
				ChangePlayerModel(pedsArray[optionClicked - 1]);//this crashed for some reason
			}
			break;
		}
	}
	break;

	case MENU_TELEPORTS:
		switch (simpleMenu.clickOption()) {
		case 0: TeleportPlayer(2700.1, -1403.39, 46.6373); break;
		case 1: TeleportPlayer(2962.82, 583.162, 44.2948); break;
		case 2: TeleportPlayer(2941.17, 1358.08, 44.0665); break;
		case 3: TeleportPlayer(2252.3, -135.647, 46.2262); break;
		case 4: TeleportPlayer(2069.06, -861.575, 42.4468); break;
		case 5: TeleportPlayer(1797.61, -1352.64, 43.8385); break;
		case 6: TeleportPlayer(889.869, -1910.26, 45.2703); break;
		case 7: TeleportPlayer(1424.09, 316.904, 88.6065); break;
		case 8: TeleportPlayer(1211.79, -201.815, 101.436); break;
		case 9: TeleportPlayer(491.451, -307.623, 143.75); break;
		case 10: TeleportPlayer(-344.127, -363.465, 88.0377); break;
		case 11: TeleportPlayer(-1308.38, 400.834, 95.3829); break;
		case 12: TeleportPlayer(-1087.97, -585.786, 81.4831); break;
		case 13: TeleportPlayer(-262.849, 793.404, 118.587); break;
		case 14: TeleportPlayer(-391.389, 916.105, 117.644); break;
		case 15: TeleportPlayer(366.483, 1456.73, 178.916); break;
		case 16: TeleportPlayer(-822.636, 325.741, 95.2731); break;
		case 17: TeleportPlayer(-1815.63, -396.749, 161.602); break;
		case 18: TeleportPlayer(-1819.9, -599.968, 154.616); break;
		case 19: TeleportPlayer(-1588.19, -936.054, 84.1072); break;
		case 20: TeleportPlayer(-858.065, -1337.73, 44.4866); break;
		case 21: TeleportPlayer(-898.997, -1654.69, 68.5928); break;
		case 22: TeleportPlayer(-1191.3, -1950.71, 43.5789); break;
		case 23: TeleportPlayer(-1452.17, -2329.4, 42.9603); break;
		case 24: TeleportPlayer(1458.7f, 815.033f, 101.3335f); print((char*)"Test Location"); break;
		}
		break;
	case MENU_WEATHER:
	{
		const char *weatherArray[] = { (char*)"OVERCAST", (char*)"RAIN", (char*)"FOG", (char*)"SNOWLIGHT", (char*)"THUNDER", (char*)"BLIZZARD", (char*)"SNOW", (char*)"MISTY", (char*)"SUNNY", (char*)"HIGHPRESSURE", (char*)"CLEARING", (char*)"SLEET", (char*)"DRIZZLE", (char*)"SHOWER", (char*)"SNOWCLEARING", (char*)"OVERCASTDARK", (char*)"THUNDERSTORM", (char*)"SANDSTORM", (char*)"HURRICANE", (char*)"HAIL", (char*)"WHITEOUT", (char*)"GROUNDBLIZZARD" };
		int optionClicked = simpleMenu.clickOption();
		if (optionClicked != -1) {
			SetWeather((char*)weatherArray[optionClicked]);
		}
	}
	break;

	case MENU_TIME:

		switch (simpleMenu.clickOption()) {
		case 0: AddToClockTime(4, 0, 0); break;
		case 1: AddToClockTime(1, 0, 0); break;
		case 2: AddToClockTime(0, 15, 0); break;
		case 3: AddToClockTime(-4, 0, 0); break;
		case 4: AddToClockTime(-1, 0, 0); break;
		case 5: AddToClockTime(0, -15, 0); break;
		}

		break;



	}//end of menu switch case

	printloop();//this has to be first or laat depending on where I want the print to show up at
	consolePrintReset();//this has to be last
}

uint64_t getWaterAddress() {
	constexpr CMemory::Pattern pattern("89 5D 58 C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? 48 C7 45 ? ? ? ? ? 48 89 5D 70 E8 ? ? ? ? 48 89 05 ? ? ? ? EB 4F 83 F9 02 75 1C 48 8B 0D ? ? ? ? E8 ? ? ? ? 33 DB 89 1D ? ? ? ? 89 1D ? ? ? ? EB 2E 83 F9 04 75 29 33 D2 33 C9 ");
	uint64_t addrSearch = pattern.Search().Get<uint64_t>();//0x7FF6B4594D14

	uint64_t addrOffset = addrSearch + 0x64;//0x7FF6B4594D78
	unsigned int valueAdd = *(unsigned int*)addrOffset;

	uint64_t water_data = addrOffset + 0x4 + valueAdd;

	return water_data;

}

uint64_t(*formatTMT_orig)(char *buf, char *fmt, char *a, char *b, uint64_t unk1) = nullptr;
uint64_t formatTMT(char *buf, char *fmt, char *a, char *b, uint64_t unk1) {

	char buffer[500];
	snprintf(buffer, sizeof(buffer), fmt, a, b);
	//Log::Info << "Format Func Called: " << buffer << Log::Endl;

	return formatTMT_orig(buf, fmt, a, b, unk1);
}

uint64_t(*n_TASK_MOUNT_ANIMAL_orig)(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f, uint64_t g, uint64_t h);
uint64_t n_TASK_MOUNT_ANIMAL(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f, uint64_t g, uint64_t h) {

	//fix the weird issue...
	//e = 0x3F800000;
	//f = 1;

	//Log::Raw << "TASK_MOUNT_ANIMAL: " << a << ", " << b << ", " << c << ", " << d << ", " << e << ", " << f << ", " << g << ", " << h << Log::Endl;

	return n_TASK_MOUNT_ANIMAL_orig(a, b, c, d, e, f, g, h);
}


float *getSnowLevel() {
	uint64_t funcAddr = GetNativeAddress(0x1E5D727041BE1709);
	uint64_t baseAddr = funcAddr + 0xB;
	uint64_t addAddr = *(uint32_t*)(baseAddr-0x4);
	uint64_t finalAddr = (baseAddr + addAddr);
	static char buf[500];
	snprintf(buf,sizeof(buf),"Snow info: 0x%p, 0x%p, 0x%p, 0x%p, %f",funcAddr,baseAddr,addAddr,finalAddr,*(float*)finalAddr);
	print(buf, 5000);
	return (float*)finalAddr;
}

uint64_t getSnowGlobalPointer() {
	//48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F 28 CE 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 63 15 ? ? ? ? 48 8D 05 ? ? ? ? 48 C1 E2 04 44 8B C6 48 03 D0 39 72 08 76 1A 
	constexpr CMemory::Pattern pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F 28 CE 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 63 15 ? ? ? ? 48 8D 05 ? ? ? ? 48 C1 E2 04 44 8B C6 48 03 D0 39 72 08 76 1A ");
	return pattern.Search().GetOffset().Get<uint64_t>();//I'm guessing this is correct
}

void setSnowLevelAdvanced(float level, bool a) {
	uint64_t r9 = getSnowGlobalPointer();
	uint64_t rax = *(uint32_t*)(r9 + 0x3F18);
	uint64_t rcx = rax * 0x1E0;
	rax = *(uint32_t*)(r9 + 0x3F1C);
	uint64_t r8 = rax * 0x1E0;

	float *xmm2 = (float*)(rcx + r9 + 0x30);
	float *xmm3 = (float*)(r8 + r9 + 0x30);
	if (a == false) {
		static char buf[100];
		snprintf(buf, sizeof(buf), "%f %f", *xmm2, *xmm3);
		print(buf,5000);
	}

	uint64_t snowAddress = *(uint64_t*)(r9 + 0x3F64);//should be the same as (uint64_t)getSnowLevel()
	if (a == true) {
		uint64_t actualAddr = (uint64_t)getSnowLevel();
		if (snowAddress == actualAddr) {
			print((char*)"Same address!!!");
		}
		if (r9 + 0x3F64 == actualAddr) {
			print((char*)"Same address   not pointer!!!");
		}
	}
	if (a == true) {
		//set values
		*xmm2 = level;
		*xmm3 = level;
	}
}



uint64_t(*REQUEST_STREAMED_TEXTURE_DICT_orig)(uint64_t data) = nullptr;
uint64_t REQUEST_STREAMED_TEXTURE_DICT_hook(uint64_t data) {
	static std::vector<int> textureList;
	uint64_t loc = *(uint64_t*)(data + 0x10);//array of arguments
	char *textureDict = (char*)*(uint64_t*)(loc);
	char *textureName = (char*)*(uint64_t*)(loc + 0x8);

	int hash = GAMEPLAY::GET_HASH_KEY( textureName);

	bool textureAlreadyLogged = false;
	for (auto i = textureList.begin(); i != textureList.end(); ++i)
		if (*i == hash)
			textureAlreadyLogged = true;

	if (textureAlreadyLogged == false) {
		textureList.push_back(hash);
		//Log::Info << "Texture dict: " << textureDict << "  Texture name: " << textureName << Log::Endl;
	}
	return REQUEST_STREAMED_TEXTURE_DICT_orig(data);
}





















































constexpr CMemory::Pattern getNativeAddressPat("0F B6 C1 48 8D 15 ? ? ? ? 4C 8B C9");
uintptr_t(*GetNativeAddress)(uint64_t hash) = nullptr;

void Init()
	{
		srand(time(NULL));

		//GetGlobalPtr = getGlobal;
		//Native::SetEssentialFunction(getAddress);



		GetNativeAddress = getNativeAddressPat.Search().Get<decltype(GetNativeAddress)>();

		void *cPedFactory = getCPedFactory();

		uint64_t cPedFactoryVtable = *(uint64_t*)(((uint64_t)cPedFactory) + 0x0);
		uint64_t vtablelocdif = cPedFactoryVtable - CMemory::Base();

		//Log::Info << "CPedFactory Vtable Offset: " << (void*)vtablelocdif << " Base address: " << (void*)CMemory::Base() << Log::Endl;

		int myPed = PLAYER::PLAYER_PED_ID();
		uint64_t myPedAddressA = (uint64_t)indexToEntity(myPed);
		uint64_t myPedAddressB = *(uint64_t*)(((uint64_t)cPedFactory) + 0x8);

		//Log::Info << "addresses of my ped: 0x" << (void*)myPedAddressA << " 0x" << (void*)myPedAddressB << Log::Endl;

		//Log::Info << "Ped ID From The address: " << entityToIndex((void*)myPedAddressA) << Log::Endl;



		//CMemory((void*)getAddress(N::DRAW_SPRITE)).Detour(REQUEST_STREAMED_TEXTURE_DICT_hook, &REQUEST_STREAMED_TEXTURE_DICT_orig);
		//Log::Info << "Hooked DRAW_SPRITE" << Log::Endl;


		//mountPatch();//patches the mount tast function to follow through more

		uint64_t cpfcSig = getCPedFactoryCreateFromSig();
		uint64_t cpfcB = *(uint64_t*)(((uint64_t)cPedFactoryVtable) + 0x8);
		//Log::Info << "addresses of create ped: 0x" << (void*)cpfcSig << ", Old one in vtable: 0x" << (void*)cpfcB << Log::Endl;
		//if (cpfcSig - cpfcB == 0) {
			//100% verified correct address now, create hook
		CPedFactorycreateped0.Search().Detour(CPedFactoryCreatePed0, &CPedFactoryCreatePed_orig0);
		//}

		//constexpr CMemory::Pattern readFileOfTypePattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 40 65 48 8B 04 25 ? ? ? ? 4D 8B F1 8B 2D ? ? ? ? 49 8B F8 41 BF ? ? ? ? 48 8B F2 48 8B D9 4C 8B 14 E8 4B 8B 04 17 48 85 C0 75 04 48 8D 41 78 8B 40 0C C1 E8 1C A8 01 74 07 C6 05 ? ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 41 B1 01 4C 8B C7 48 8B D6 48 8B CB E8");
		//readFileOfTypePattern.Search().Detour(readFileOfTypeDetour, &readFileOfType_orig);


		constexpr CMemory::Pattern pattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 56 41 57 48 8D 68 A8 48 81 EC ? ? ? ? F3 0F 10 0A 33 DB F3 0F 10 42 ? 41 8A F1 F3 0F 10 5A ? 45 8A F9 40 22 B5 ? ? ? ? 8B F9 0F 29 70 D8 44 8B F3 0F 14 D9 0F 28 F1 0F 14 F0 0F 14 F3 0F 57 DB 0F 2F F3 C7 44 24 ");
		//Log::Info << "addresses of CREATE_PED: 0x" << (void*)pattern.Search().Get<uint64_t>() << Log::Endl;

		constexpr CMemory::Pattern Mount_Animal_pattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 D8 41 83 F9 FE 0F 29 78 C8 41 BE ? ? ? ? 41 8D 41 01 41 8B F0 44 0F 45 F0 8B DA 44 8B F9 41 83 F8 FF 74 09 45 85 C0 0F 8E ? ? ? ? 8B CB E8 ? ? ? ? 41 B8 ? ? ? ? 8B D3 48 8B C8 48 8B F8 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 85 FF 0F 84 ");
		//Log::Info << "addresses of TASK_MOUNT_ANIMAL: 0x" << (void*)Mount_Animal_pattern.Search().Get<uint64_t>() << Log::Endl;

		uint64_t waterAddress = getWaterAddress();

		//Log::Info << "Watter address: 0x" << (void*)waterAddress << Log::Endl;

		//Log::Info << "Watter address value: 0x" << (void*)*(uint64_t*)waterAddress << Log::Endl;

		unsigned int startOffsetWater = 0x20;//also could maybe be 0x18
		unsigned int waterSize = 0x1FC0 - startOffsetWater;
		//uint64_t waterStart = *(uint64_t*)waterAddress + startOffsetWater;
		for (uint64_t i = 0; i < waterSize; i++) {
			//*(char*)(waterStart + i) = 0;
		}

		//Log::Info << "Water overritten" << Log::Endl;


		getWeirdVal(0);//find the sig so it doesn't when zombies spawn
		entityToIndex((void*)myPedAddressA);//find the sig so it doesn't when zombies spawn



		//constexpr CMemory::Pattern formatYMTpattern("48 8B C4 48 89 50 10 4C 89 40 18 4C 89 48 20 53 48 83 EC 20 48 8B D9 48 85 D2 74 20 4C 8D 48 18 48 85 C9 74 0D 4C 8B C2 BA ? ? ? ? E8 ? ? ? ? 48 8B CB E8 ? ? ? ? EB 03 C6 01 00 48 83 C4 20 5B ");
		//formatYMTpattern.Search().Detour(formatTMT, &formatTMT_orig);

		//constexpr CMemory::Pattern n_TASK_MOUNT_ANIMALpattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 D8 41 83 F9 FE 0F 29 78 C8 41 BE ? ? ? ? 41 8D 41 01 41 8B F0 44 0F 45 F0 8B DA 44 8B F9 41 83 F8 FF 74 09 45 85 C0 0F 8E ? ? ? ? 8B CB E8 ? ? ? ? 41 B8 ? ? ? ? 8B D3 48 8B C8 48 8B F8");
		//n_TASK_MOUNT_ANIMALpattern.Search().Detour(n_TASK_MOUNT_ANIMAL, &n_TASK_MOUNT_ANIMAL_orig);


		bool* canChangeModel = (bool*)getGlobalPtr(1835009);
		*canChangeModel = true;

		//MENU_MAIN setup
		static char *listMain[] = {
			(char*)"Toggle Noclip",
			(char*)"Spawn Vehicle >",
			(char*)"Spawn Ped >",
			(char*)"Change Model >",
			(char*)"Teleports >",
			(char*)"Weather >",
			(char*)"Time >",
			(char*)"Self Options >",
			(char*)"Weapon Options >",
			(char*)"Fun Options >",
			//(char*)"Spawn Single Zombie",
			//(char*)"Toggle Forge Gun",
			//(char*)"Gravity Gun",
			//(char*)"Need For Speed",
			//(char*)"God Mode",
			//(char*)"Into The Sky",
			//(char*)"Shootiport",
			//(char*)"Explosive Ammo",
			//(char*)"Infinite Ammo",
			//(char*)"Zombie Apocalypse",
			//(char*)"Infinite Stabbable Micah",
			//(char*)"Rapid Fire",
			//(char*)"Never Wanted",
			//(char*)"Force Field",
			//(char*)"Force Push",
			//(char*)"CHONKY MORGAN",
			//(char*)"Teleport Into Closest Vehicle",
			//(char*)"Animal Gun",
			//(char*)"Lightning Ammo",
			//(char*)"Predator Missile",
			//(char*)"Give All Weapons",
			//(char*)"Print snow level from native",
			//(char*)"View snow level values",
			//(char*)"Set snow level to 5",
			//(char*)"Font Style",
			//(char*)"Force Ring Thing",
			//(char*)"Spawn UFO Objects (test function)",
			//(char*)"Spawn Driveable UFO",
			//(char*)"Despawn UFO"
			//(char*)"Ufo Speed"
			//(char*)"Test For Beam Up Flag For Cows"
			//(char*)"Mount Forge Selected Ped",
			//(char*)"Mount 2",
			//(char*)"Mount Ontol Selected Ped"
		};
		int szMain = sizeof(listMain) / sizeof(char*);
		simpleMenu.setOptionList(MENU_MAIN, szMain, listMain);

		//simpleMenu.setOptionData(MENU_MAIN, 13, &shootiportBool);
		//simpleMenu.setOptionData(MENU_MAIN, 14, &explosiveAmmoBool);
	//	simpleMenu.setOptionData(MENU_MAIN, 15, &infiniteAmmoBool);
		//simpleMenu.setOptionData(MENU_MAIN, 17, &RapidfireActive);
		//simpleMenu.setOptionData(MENU_MAIN, 18, &neverWanted);
		//simpleMenu.setOptionData(MENU_MAIN, 19, &forceFieldBool);
		//simpleMenu.setOptionData(MENU_MAIN, 20, &forcePushBool);//add option info "press F"
		//simpleMenu.setOptionData(MENU_MAIN, 23, &shootAnimals);
		//simpleMenu.setOptionData(MENU_MAIN, 24, &lightningAmmoBool);
		//simpleMenu.setOptionData(MENU_MAIN, 30, &textFont, 0, 10);
		//simpleMenu.setOptionData(MENU_MAIN, 27, &circlePedsForceEnabled);

		//simpleMenu.setOptionInfoText(MENU_MAIN, 20, "Press F to Push");
		//simpleMenu.setOptionInfoText(MENU_MAIN, 27, "Press F to Use");

		//simpleMenu.setOptionData(MENU_MAIN, 30, &explosionID, 0);
		//simpleMenu.setOptionData(MENU_MAIN, 31, &ufomod::ammoType, 0, 5);
		//simpleMenu.setOptionData(MENU_MAIN, 29, &TEST_FLAG, 0, 31);
		//simpleMenu.setOptionData(MENU_MAIN, 29, &ufomod::skySpeed, 0.2f, 1.0f, 0.01f);

		//MENU_FUN setuo
		static char *listMENU_FUN[] = {
			"Spawn Driveable UFO",
			"Despawn UFO",
			"Force Ring Thing",
			"Predator Missile",
			"CHONKY MORGAN",
			"Zombie Apocalypse",
			"Spawn Single Zombie",
			"Force Field",
			"Force Push",
			"They Might Be Giants",
			"Size Of Giants",
			"Spawn Killable Micah",
			"Set Snow Coverage",
			"Spawn Hitler",
			"Player Ped Size"
		};
		int szMENU_FUN = sizeof(listMENU_FUN) / sizeof(char*);
		simpleMenu.setOptionList(MENU_FUN, szMENU_FUN, listMENU_FUN);

		simpleMenu.setOptionData(MENU_FUN, 2, &circlePedsForceEnabled);
		simpleMenu.setOptionData(MENU_FUN, 4, &WIDEMOD);
		simpleMenu.setOptionData(MENU_FUN, 5, &zombieMode);
		simpleMenu.setOptionData(MENU_FUN, 7, &forceFieldBool);
		simpleMenu.setOptionData(MENU_FUN, 8, &forcePushBool);//add option info "press F"
		simpleMenu.setOptionData(MENU_FUN, 9, &giantsMode);
		simpleMenu.setOptionData(MENU_FUN, 10, &giantsModeScale, 0, 100.0f, 0.1f);
		simpleMenu.setOptionData(MENU_FUN, 12, &snowCoverageLevel,0,3);
		simpleMenu.setOptionData(MENU_FUN, 14, &SizeOfMyPed, 0.0f, 100.0f, 0.1f);

		simpleMenu.setOptionClickOnScroll(MENU_FUN, 12, true);
		simpleMenu.setOptionClickOnScroll(MENU_FUN, 14, true);

		simpleMenu.setOptionInfoText(MENU_FUN, 2, "Press F or Square to Use");//force ring thing
		simpleMenu.setOptionInfoText(MENU_FUN, 5, "This is experimental and replaces all spawned peds with zombies\n Please try other zombie mods too to see what you like best");
																												//   Please use other mods if you want well made zombies gameplay
		simpleMenu.setOptionInfoText(MENU_FUN, 8, "Press F or Square to Push");
		simpleMenu.setOptionInfoText(MENU_FUN, 13, "Its 1899 and 10 year old Hitler has HAD ENOUGH");
		//simpleMenu.setOptionInfoText(MENU_FUN, 10, "Press N to input custom value");
		//simpleMenu.setOptionInfoText(MENU_FUN, 14, "Press N to input custom value");


		//MENU_WEAPONS setup
		static char *listMENU_WEAPONS[] = {
			"Give All Weapons",
			"Infinite Ammo",
			"Lightning Ammo",
			"Animal Gun",
			"Rapid Fire",
			"Explosive Ammo",
			"Teleport Ammo",
			"Gravity Gun",
			"Portal Gun",
			"Delete Portals"

		};
		int szMENU_WEAPONS = sizeof(listMENU_WEAPONS) / sizeof(char*);
		simpleMenu.setOptionList(MENU_WEAPONS, szMENU_WEAPONS, listMENU_WEAPONS);

		simpleMenu.setOptionData(MENU_WEAPONS, 1, &infiniteAmmoBool);
		simpleMenu.setOptionData(MENU_WEAPONS, 2, &lightningAmmoBool);
		simpleMenu.setOptionData(MENU_WEAPONS, 3, &shootAnimals);
		simpleMenu.setOptionData(MENU_WEAPONS, 4, &RapidfireActive);
		simpleMenu.setOptionData(MENU_WEAPONS, 5, &explosiveAmmoBool);
		simpleMenu.setOptionData(MENU_WEAPONS, 6, &shootiportBool);
		simpleMenu.setOptionData(MENU_WEAPONS, 7, &forge::gravityGun);
		simpleMenu.setOptionData(MENU_WEAPONS, 8, &PortalGun::enabled);
		//simpleMenu.setOptionData(MENU_WEAPONS, 10, &testValueThingy,0,10);


		//MENU_SELF setup
		static char *listSELF_OPTIONS[] = {
			"Godmode",
			"Never Wanted",
			"Teleport Into Closest Vehicle",
			"Into The Sky",
			"Need For Speed",
			"Forge Gun",
			"Custom Camera Zoom",
			"Custom Camera Zoom Value"
		};
		int szSELF_OPTIONS = sizeof(listSELF_OPTIONS) / sizeof(char*);
		simpleMenu.setOptionList(MENU_SELF, szSELF_OPTIONS, listSELF_OPTIONS);

		simpleMenu.setOptionData(MENU_SELF, 0, &godmode);
		simpleMenu.setOptionData(MENU_SELF, 1, &neverWanted);
		simpleMenu.setOptionData(MENU_SELF, 4, &fastHorseEnabled);
		simpleMenu.setOptionData(MENU_SELF, 5, &forge::enabled);
		simpleMenu.setOptionData(MENU_SELF, 6, &customCameraZoom);
		simpleMenu.setOptionData(MENU_SELF, 7, &customCameraZoomFloat,0,100,0.1f);
		simpleMenu.setOptionClickOnScroll(MENU_SELF, 7, true);




		//MENU_SPAWN_VEHICLES setup
		static char *listMENU_SPAWN_VEHICLES[] = {
			(char*)"Custom Input",
			(char*)"privateopensleeper02x",(char*)"privateopensleeper01x",(char*)"steamerDummy",(char*)"armoredCar01x",(char*)"armoredCar03x",(char*)"privatebaggage01x",(char*)"smuggler02",(char*)"keelboat",(char*)"boatSteam02x",(char*)"midlandrefrigeratorCar",(char*)"midlandboxcar05x",(char*)"caboose01x",(char*)"canoe",(char*)"canoeTreeTrunk",(char*)"cart01",(char*)"cart02",(char*)"cart03",(char*)"cart04",(char*)"cart05",(char*)"cart06",(char*)"cart07",(char*)"cart08",(char*)"coach2",(char*)"coach3",(char*)"coach3_cutscene",(char*)"coach4",(char*)"coach5",(char*)"coach6",(char*)"buggy01",(char*)"buggy02",(char*)"buggy03",(char*)"ArmySupplyWagon",(char*)"chuckwagon000x",(char*)"supplywagon",(char*)"supplywagon2",(char*)"logwagon",(char*)"logwagon2",(char*)"coal_wagon",(char*)"chuckwagon002x",(char*)"gatling_gun",(char*)"gatlingMaxim02",(char*)"handcart",(char*)"horseBoat",(char*)"hotAirBalloon01",(char*)"hotchkiss_cannon",(char*)"mineCart01x",(char*)"northflatcar01x",(char*)"privateflatcar01x",(char*)"northpassenger01x",(char*)"northpassenger03x",(char*)"privatepassenger01x",(char*)"oilWagon01x",(char*)"oilWagon02x",(char*)"pirogue",(char*)"pirogue2",(char*)"policeWagon01x",(char*)"policeWagongatling01x",(char*)"privateCoalCar01x",(char*)"NorthCoalCar01x",(char*)"winterSteamer",(char*)"wintercoalcar",(char*)"privateboxcar04x",(char*)"privateboxcar02x",(char*)"privateboxcar01x",(char*)"coalHopper01x",(char*)"privateObservationcar",(char*)"privateArmoured",(char*)"privateDining01x",(char*)"privateRooms01x",(char*)"privateSteamer01x",(char*)"northSteamer01x",(char*)"GhostTrainSteamer",(char*)"GhostTrainCoalCar",(char*)"GhostTrainPassenger",(char*)"GhostTrainCaboose",(char*)"rcBoat",(char*)"rowboat",(char*)"rowboatSwamp",(char*)"rowboatSwamp02",(char*)"ship_guama02",(char*)"turbineboat",(char*)"ship_nbdGuama",(char*)"ship_nbdGuama2",(char*)"skiff",(char*)"stagecoach001x",(char*)"stagecoach002x",(char*)"stagecoach003x",(char*)"stagecoach004x",(char*)"stagecoach005x",(char*)"stagecoach006x",(char*)"trolley01x",(char*)"TugBoat2",(char*)"wagon02x",(char*)"wagon03x",(char*)"wagon04x",(char*)"wagon05x",(char*)"wagon06x",(char*)"wagonCircus01x",(char*)"wagonCircus02x",(char*)"wagonDoc01x",(char*)"wagonPrison01x",(char*)"wagonWork01x",(char*)"wagonDairy01x",(char*)"wagonTraveller01x",(char*)"breach_cannon",(char*)"utilliwag",(char*)"gatchuck",(char*)"gatchuck_2" };
		int szMENU_SPAWN_VEHICLES = sizeof(listMENU_SPAWN_VEHICLES) / sizeof(char*);
		simpleMenu.setOptionList(MENU_SPAWN_VEHICLES, szMENU_SPAWN_VEHICLES, listMENU_SPAWN_VEHICLES);

		//MENU_SPAWN_PEDS setip
		static char *listMENU_SPAWN_PEDS[] = {
			(char*)"Custom Input",
			(char*)"CS_BILLWILLIAMSON",(char*)"CS_MICAHBELL",(char*)"CS_UNCLE",(char*)"CS_SEAN",(char*)"CS_KIERAN",(char*)"A_M_M_WapWarriors_01",(char*)"A_M_O_WapTownfolk_01",(char*)"A_F_O_WapTownfolk_01",(char*)"U_M_M_WAPOFFICIAL_01",(char*)"A_M_M_WAPWARRIORS_01",(char*)"A_M_O_WAPTOWNFOLK_01",(char*)"S_M_M_UNITRAINENGINEER_01",(char*)"S_M_M_UNITRAINGUARDS_01",(char*)"A_M_M_MiddleTrainPassengers_01",(char*)"A_F_M_MiddleTrainPassengers_01",(char*)"A_M_M_LowerTrainPassengers_01",(char*)"A_F_M_LowerTrainPassengers_01",(char*)"A_M_M_UpperTrainPassengers_01",(char*)"A_F_M_UpperTrainPassengers_01",(char*)"A_M_M_NBXUPPERCLASS_01",(char*)"A_F_M_NBXUPPERCLASS_01",(char*)"G_M_M_UNIDUSTER_02",(char*)"G_M_M_UNIDUSTER_03",(char*)"CS_EDMUNDLOWRY",(char*)"G_M_M_UniBanditos_01",(char*)"CS_POISONWELLSHAMAN",(char*)"S_M_M_RACRAILGUARDS_01",(char*)"U_M_M_valbarber_01",(char*)"U_M_M_ValBartender_01",(char*)"U_M_M_ValDoctor_01",(char*)"U_M_M_ValGunsmith_01",(char*)"CS_VALSHERIFF",(char*)"G_M_M_UNIDUSTER_01",(char*)"S_M_M_SDTICKETSELLER_01",(char*)"S_M_M_MagicLantern_01",(char*)"CS_GrizzledJon",(char*)"A_M_M_VALCRIMINALS_01",(char*)"A_M_M_ASBMINER_02",(char*)"A_M_Y_ASBMINER_02",(char*)"A_M_M_ASBMINER_04",(char*)"A_M_Y_ASBMINER_04",(char*)"A_M_M_ASBMINER_03",(char*)"A_M_Y_ASBMINER_03",(char*)"A_M_M_JamesonGuard_01",(char*)"U_M_O_ASBSHERIFF_01",(char*)"A_M_M_RkrFancyDRIVERS_01",(char*)"A_M_M_RkrFancyTravellers_01",(char*)"A_F_M_RkrFancyTravellers_01",(char*)"S_M_M_CornwallGuard_01",(char*)"G_M_M_UniCornwallGoons_01",(char*)"U_M_M_CRDHOMESTEADTENANT_01",(char*)"U_M_M_CRDHOMESTEADTENANT_02",(char*)"U_M_O_PSHRANCHER_01",(char*)"U_M_O_CAJHOMESTEAD_01",(char*)"U_M_Y_CAJHOMESTEAD_01",(char*)"U_M_M_CAJHOMESTEAD_01",(char*)"CS_DOROETHEAWICKLOW",(char*)"S_M_M_AmbientSDPolice_01",(char*)"A_M_M_SDObeseMen_01",(char*)"A_F_M_SDObeseWomen_01",(char*)"U_M_M_NBXShadyDealer_01",(char*)"A_F_M_WapTownfolk_01",(char*)"A_M_M_SDLABORERS_02",(char*)"A_M_Y_NBXSTREETKIDS_01",(char*)"A_M_M_LowerSDTownfolk_02",(char*)"A_M_M_LowerSDTownfolk_01",(char*)"A_M_Y_NbxStreetKids_Slums_01",(char*)"A_M_Y_SDStreetKids_Slums_02",(char*)"A_M_M_SDDockWorkers_02",(char*)"A_M_M_SDDockForeman_01",(char*)"A_F_M_NbxSlums_01",(char*)"A_F_M_SDSlums_02",(char*)"A_M_M_NbxSlums_01",(char*)"A_M_M_SDSlums_02",(char*)"A_M_O_SDUpperClass_01",(char*)"A_F_M_NbxUpperClass_01",(char*)"A_F_O_SDUpperClass_01",(char*)"A_M_M_MiddleSDTownfolk_01",(char*)"A_M_M_MiddleSDTownfolk_02",(char*)"A_M_M_MiddleSDTownfolk_03",(char*)"A_F_M_MiddleSDTownfolk_01",(char*)"A_F_M_MiddleSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_01",(char*)"A_F_M_LowerSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_03",(char*)"A_F_M_SDChinatown_01",(char*)"A_F_O_SDChinatown_01",(char*)"A_F_M_SDFancyWhore_01",(char*)"G_M_M_UNIBRONTEGOONS_01",(char*)"A_M_M_SDChinatown_01",(char*)"A_M_O_SDChinatown_01",(char*)"U_M_M_VALGUNSMITH_01",(char*)"U_M_M_VALGENSTOREOWNER_01",(char*)"U_M_M_ValButcher_01",(char*)"A_M_M_VALTOWNFOLK_01",(char*)"U_M_M_VALDOCTOR_01",(char*)"S_M_M_BANKCLERK_01",(char*)"U_M_M_ValHotelOwner_01",(char*)"U_M_M_VALBARTENDER_01",(char*)"U_M_M_VALBARBER_01",(char*)"U_M_O_ValBartender_01",(char*)"U_M_M_NBXBARTENDER_02",(char*)"A_M_M_BLWUPPERCLASS_01",(char*)"U_M_M_NBXBARTENDER_01",(char*)"A_M_M_MIDDLESDTOWNFOLK_01",(char*)"U_M_M_NBXGENERALSTOREOWNER_01",(char*)"U_M_M_NBXSHADYDEALER_01",(char*)"S_M_M_TAILOR_01",(char*)"U_M_M_NBXGUNSMITH_01",(char*)"CS_SDDOCTOR_01",(char*)"S_M_M_LIVERYWORKER_01",(char*)"S_M_M_MarketVendor_01",(char*)"U_M_M_SDPhotographer_01",(char*)"U_M_M_STRGENSTOREOWNER_01",(char*)"U_M_M_STRFREIGHTSTATIONOWNER_01",(char*)"U_M_M_ASBGUNSMITH_01",(char*)"U_M_M_SDTRAPPER_01",(char*)"S_M_M_TRAINSTATIONWORKER_01",(char*)"A_F_M_LAGTOWNFOLK_01",(char*)"S_M_M_SKPGUARD_01",(char*)"A_M_M_SKPPRISONER_01",(char*)"U_F_M_LagMother_01",(char*)"A_F_O_LagTownfolk_01",(char*)"A_M_O_LagTownfolk_01",(char*)"A_M_M_RHDFOREMAN_01",(char*)"U_M_M_ORPGUARD_01",(char*)"CS_ABE",(char*)"CS_DavidGeddes",(char*)"CS_ANGUSGEDDES",(char*)"CS_DUNCANGEDDES",(char*)"CS_MRSGEDDES",(char*)"G_M_M_UNIMOUNTAINMEN_01",(char*)"A_F_M_LagTownfolk_01",(char*)"CS_ThomasDown",(char*)"CS_ArchieDown",(char*)"CS_EDITHDOWN",(char*)"S_F_M_MaPWorker_01",(char*)"A_M_M_StrTownfolk_01",(char*)"A_M_M_STRLABORER_01",(char*)"A_M_M_BynRoughTravellers_01",(char*)"A_C_DOGPOODLE_01",(char*)"S_M_M_RHDCOWPOKE_01",(char*)"S_M_M_UniButchers_01",(char*)"S_M_Y_NewspaperBoy_01",(char*)"A_F_M_RhdUpperClass_01",(char*)"A_F_M_RhdTownfolk_02",(char*)"a_m_m_rhdtownfolk_01_laborer",(char*)"A_F_M_RHDTOWNFOLK_01",(char*)"U_M_M_RHDBARTENDER_01",(char*)"U_M_M_RHDGUNSMITH_01",(char*)"U_M_M_RHDGENSTOREOWNER_01",(char*)"U_M_M_RHDGENSTOREOWNER_02",(char*)"U_M_M_RHDSHERIFF_01",(char*)"U_M_M_RHDTRAINSTATIONWORKER_01",(char*)"A_F_M_RHDPROSTITUTE_01",(char*)"A_M_M_RHDObeseMen_01",(char*)"A_M_M_RhdUpperClass_01",(char*)"A_M_M_RkrRoughTravellers_01",(char*)"A_C_DOGCATAHOULACUR_01",(char*)"A_F_M_VHTTOWNFOLK_01",(char*)"A_M_M_VHTTHUG_01",(char*)"U_M_M_VHTSTATIONCLERK_01",(char*)"U_M_O_VHTEXOTICSHOPKEEPER_01",(char*)"U_F_M_VhTBartender_01",(char*)"A_M_M_HtlFancyDRIVERS_01",(char*)"A_M_M_HtlFancyTravellers_01",(char*)"A_F_M_HtlFancyTravellers_01",(char*)"S_M_M_BARBER_01",(char*)"S_M_M_UNIBUTCHERS_01",(char*)"S_M_M_BLWCOWPOKE_01",(char*)"A_M_M_BLWObeseMen_01",(char*)"A_M_O_BlWUpperClass_01",(char*)"A_F_M_BlWUpperClass_01",(char*)"A_F_O_BlWUpperClass_01",(char*)"A_M_M_BlWLaborer_02",(char*)"A_M_M_BlWLaborer_01",(char*)"A_M_M_BlWForeman_01",(char*)"A_F_M_BlWTownfolk_02",(char*)"A_F_M_BlWTownfolk_01",(char*)"S_M_M_AmbientBlWPolice_01",(char*)"S_M_M_FussarHenchman_01",(char*)"A_F_M_GuaTownfolk_01",(char*)"A_F_O_GuaTownfolk_01",(char*)"A_M_M_GuaTownfolk_01",(char*)"A_M_O_GuaTownfolk_01",(char*)"CS_NicholasTimmins",(char*)"A_M_M_STRFANCYTOURIST_01",(char*)"A_F_M_BiVFancyTravellers_01",(char*)"A_F_M_STRTOWNFOLK_01",(char*)"A_M_M_UNIGUNSLINGER_01",(char*)"U_M_M_StrGenStoreOwner_01",(char*)"U_M_M_STRWELCOMECENTER_01",(char*)"U_M_M_STRSHERRIFF_01",(char*)"U_M_M_StrFreightStationOwner_01",(char*)"G_M_M_UNICRIMINALS_01",(char*)"G_M_M_UNICRIMINALS_02",(char*)"A_M_M_BTCObeseMen_01",(char*)"A_F_M_BTCObeseWomen_01",(char*)"A_M_M_AsbTownfolk_01",(char*)"A_F_M_AsbTownfolk_01",(char*)"A_F_M_BTCHILLBILLY_01",(char*)"A_F_O_BTCHILLBILLY_01",(char*)"A_M_M_BTCHILLBILLY_01",(char*)"G_M_M_UniBraithwaites_01",(char*)"U_M_O_CmrCivilwarcommando_01",(char*)"S_M_M_Army_01",(char*)"U_M_M_BWMSTABLEHAND_01",(char*)"S_M_M_CGHWORKER_01",(char*)"G_M_M_UniGrays_01",(char*)"G_M_O_UniExConfeds_01",(char*)"G_M_Y_UniExConfeds_01",(char*)"G_M_M_UNIINBRED_01",(char*)"CS_AberdeenSister",(char*)"A_M_M_GriRoughTravellers_01",(char*)"A_F_M_ValTownfolk_01",(char*)"U_F_O_WTCTOWNFOLK_01",(char*)"U_M_M_BLWTRAINSTATIONWORKER_01",(char*)"CS_BLWPHOTOGRAPHER",(char*)"A_M_M_BlWTownfolk_01",(char*)"U_M_O_BLWPOLICECHIEF_01",(char*)"S_M_M_Tailor_01",(char*)"S_M_M_BankClerk_01",(char*)"U_M_O_BLWBARTENDER_01",(char*)"U_M_O_BLWGENERALSTOREOWNER_01",(char*)"S_M_M_TumDeputies_01",(char*)"CS_SHERIFFFREEMAN",(char*)"A_M_M_NEAROUGHTRAVELLERS_01",(char*)"A_M_M_TumTownfolk_01",(char*)"A_F_M_TumTownfolk_01",(char*)"A_M_M_TumTownfolk_02",(char*)"A_F_M_TumTownfolk_02",(char*)"U_M_M_TUMBARTENDER_01",(char*)"U_M_M_TUMBUTCHER_01",(char*)"U_M_M_TUMGUNSMITH_01",(char*)"U_F_M_TUMGENERALSTOREOWNER_01",(char*)"A_M_M_ARMTOWNFOLK_01",(char*)"A_M_M_RANCHERTRAVELERS_WARM_01",(char*)"S_M_M_AmbientLawRural_01",(char*)"A_M_M_ArmCholeraCorpse_01",(char*)"A_F_M_ArmCholeraCorpse_01",(char*)"U_M_M_ARMGENERALSTOREOWNER_01",(char*)"U_M_O_ARMBARTENDER_01",(char*)"U_M_M_ARMTRAINSTATIONWORKER_01",(char*)"U_M_M_ARMUNDERTAKER_01",(char*)"RE_DEADBODIES_MALE",(char*)"RE_DEADBODIES_MALES_01",(char*)"A_F_M_ArmTownfolk_01",(char*)"A_F_M_ArmTownfolk_02",(char*)"A_M_M_ArmTownfolk_01",(char*)"A_M_M_ArmTownfolk_02",(char*)"A_F_M_FAMILYTRAVELERS_WARM_01",(char*)"CS_WROBEL",(char*)"U_M_M_CZPHOMESTEADFATHER_01",(char*)"U_F_Y_CZPHOMESTEADDAUGHTER_01",(char*)"U_M_Y_CZPHOMESTEADSON_01",(char*)"U_M_Y_CZPHOMESTEADSON_02",(char*)"U_M_Y_CZPHOMESTEADSON_03",(char*)"U_M_M_LRSHOMESTEADTENANT_01",(char*)"U_F_M_RKSHOMESTEADTENANT_01",(char*)"U_M_M_UNIEXCONFEDSBOUNTY_01",(char*)"U_M_M_WALGENERALSTOREOWNER_01",(char*)"S_M_M_TrainStationWorker_01",(char*)"p_m_zero",(char*)"P_M_THREE",(char*)"A_C_HORSE_GANG_KIERAN",(char*)"CS_Dutch",(char*)"CS_JackMarston",(char*)"CS_JACKMARSTON_TEEN",(char*)"CS_EagleFlies",(char*)"CS_Cleet",(char*)"CS_Joe",(char*)"CS_CREOLECAPTAIN",(char*)"CS_JosiahTrelawny",(char*)"CS_RevSwanson",(char*)"CS_LeoStrauss",(char*)"CS_MrPearson",(char*)"CS_Lenny",(char*)"CS_Sean",(char*)"CS_HoseaMatthews",(char*)"CS_JavierEscuella",(char*)"CS_CharlesSmith",(char*)"CS_Uncle",(char*)"CS_BillWilliamson",(char*)"CS_MicahBell",(char*)"CS_JohnMarston",(char*)"CS_AbigailRoberts",(char*)"CS_MaryBeth",(char*)"CS_MollyOshea",(char*)"CS_SusanGrimshaw",(char*)"CS_Karen",(char*)"CS_Tilly",(char*)"CS_MrsAdler",(char*)"A_M_M_HtlRoughTravellers_01",(char*)"A_M_M_SclRoughTravellers_01",(char*)"U_M_M_LnSWorker_01",(char*)"U_M_M_LnSWorker_02",(char*)"U_M_M_LnSWorker_03",(char*)"U_M_M_LnSWorker_04",(char*)"U_M_M_RKFRANCHER_01",(char*)"A_M_M_Rancher_01",(char*)"U_M_M_MFRRANCHER_01",(char*)"EA_LCMP_DOGS",(char*)"EA_LCMP_FOREMAN",(char*)"U_M_M_BiVForeman_01",(char*)"EA_HMSTD_FOREMAN",(char*)"U_M_M_HtlForeman_01",(char*)"EA_HMSTD_WORKER_1",(char*)"EA_HMSTD_WORKER_2",(char*)"EA_HMSTD_MALE",(char*)"U_M_M_HTLHusband_01",(char*)"EA_HMSTD_FEMALE",(char*)"U_F_M_HTLWife_01",(char*)"EA_RCAMP_FOREMAN",(char*)"U_M_M_RaCForeman_01",(char*)"A_M_M_NbxUpperClass_01",(char*)"A_M_M_BlWUpperClass_01",(char*)"CS_Kieran",(char*)"A_M_M_RHDTOWNFOLK_02",(char*)"S_M_M_CKTWORKER_01",(char*)"S_M_M_STRLUMBERJACK_01",(char*)"S_M_M_RaCRailWorker_01",(char*)"S_M_Y_RACRAILWORKER_01",(char*)"A_M_M_BIVWORKER_01",(char*)"A_M_M_BLWTOWNFOLK_01",(char*)"S_M_M_MAPWORKER_01",(char*)"U_M_Y_HTLWORKER_01",(char*)"U_M_Y_HTLWORKER_02",(char*)"A_M_M_ASBMINER_01",(char*)"A_M_Y_ASBMINER_01",(char*)"CS_AberdeenPigFarmer",(char*)"U_M_M_DORHOMESTEADHUSBAND_01",(char*)"A_M_M_EmRFarmHand_01",(char*)"U_M_M_EMRFATHER_01",(char*)"U_M_Y_EMRSON_01",(char*)"A_M_M_ValTownfolk_01",(char*)"A_M_M_ValFarmer_01",(char*)"A_M_M_ValTownfolk_02",(char*)"S_M_M_ValCowpoke_01",(char*)"A_M_M_ValLaborer_01",(char*)"S_M_M_LiveryWorker_01",(char*)"A_M_M_NBXDOCKWORKERS_01",(char*)"A_M_M_NBXLABORERS_01",(char*)"A_M_M_LagTownfolk_01",(char*)"A_M_M_BtcHillbilly_01",(char*)"A_M_O_BTCHILLBILLY_01",(char*)"A_M_M_VhtTownfolk_01",(char*)"CS_TomDickens",(char*)"A_M_M_STRTOWNFOLK_01",(char*)"A_M_M_RHDTOWNFOLK_01",(char*)"A_M_M_RhdForeman_01",(char*)"S_M_Y_Army_01",(char*)"A_F_M_NbxWhore_01",(char*)"A_F_M_VHTPROSTITUTE_01",(char*)"A_F_M_VALPROSTITUTE_01",(char*)"A_C_Cat_01",(char*)"A_C_CedarWaxwing_01",(char*)"A_C_Chipmunk_01",(char*)"A_C_CRAB_01",(char*)"A_C_Cormorant_01",(char*)"A_C_CarolinaParakeet_01",(char*)"A_C_DogCatahoulaCur_01",(char*)"A_C_DOGHOBO_01",(char*)"A_C_DOGHOUND_01",(char*)"A_C_DogHusky_01",(char*)"A_C_DOGRUFUS_01",(char*)"A_C_DOGBLUETICKCOONHOUND_01",(char*)"A_C_Donkey_01",(char*)"A_C_Elk_01",(char*)"A_C_FISHBULLHEADCAT_01_SM",(char*)"A_C_FISHNORTHERNPIKE_01_LG",(char*)"A_C_FISHRAINBOWTROUT_01_MS",(char*)"A_C_FISHSALMONSOCKEYE_01_MS",(char*)"A_C_GILAMONSTER_01",(char*)"A_C_Goat_01",(char*)"A_C_HORSE_AMERICANPAINT_GREYOVERO",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_PALOMINODAPPLE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_SILVERTAILBUCKSKIN",(char*)"A_C_HORSE_ANDALUSIAN_DARKBAY",(char*)"A_C_HORSE_ANDALUSIAN_ROSEGRAY",(char*)"A_C_HORSE_APPALOOSA_BROWNLEOPARD",(char*)"A_C_HORSE_APPALOOSA_LEOPARD",(char*)"A_C_HORSE_ARABIAN_BLACK",(char*)"A_C_HORSE_ARABIAN_ROSEGREYBAY",(char*)"A_C_HORSE_ARDENNES_BAYROAN",(char*)"A_C_HORSE_ARDENNES_STRAWBERRYROAN",(char*)"A_C_HORSE_BELGIAN_BLONDCHESTNUT",(char*)"A_C_HORSE_BELGIAN_MEALYCHESTNUT",(char*)"A_C_HORSE_DUTCHWARMBLOOD_CHOCOLATEROAN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SEALBROWN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SOOTYBUCKSKIN",(char*)"A_C_HORSE_HUNGARIANHALFBRED_DARKDAPPLEGREY",(char*)"A_C_HORSE_HUNGARIANHALFBRED_PIEBALDTOBIANO",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_AMBERCHAMPAGNE",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_SILVERDAPPLEPINTO",(char*)"A_C_HORSE_NOKOTA_REVERSEDAPPLEROAN",(char*)"A_C_HORSE_SHIRE_DARKBAY",(char*)"A_C_HORSE_SHIRE_LIGHTGREY",(char*)"A_C_HORSE_SUFFOLKPUNCH_SORREL",(char*)"A_C_HORSE_SUFFOLKPUNCH_REDCHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_FLAXENROAN",(char*)"A_C_HORSE_THOROUGHBRED_BRINDLE",(char*)"A_C_HORSE_TURKOMAN_DARKBAY",(char*)"A_C_HORSE_TURKOMAN_GOLD",(char*)"A_C_HORSE_TURKOMAN_SILVER",(char*)"A_C_HORSE_GANG_BILL",(char*)"A_C_HORSE_GANG_CHARLES",(char*)"A_C_HORSE_GANG_DUTCH",(char*)"A_C_HORSE_GANG_HOSEA",(char*)"A_C_HORSE_GANG_JAVIER",(char*)"A_C_HORSE_GANG_JOHN",(char*)"A_C_HORSE_GANG_KAREN",(char*)"A_C_HORSE_GANG_LENNY",(char*)"A_C_HORSE_GANG_MICAH",(char*)"A_C_HORSE_GANG_SADIE",(char*)"A_C_HORSE_GANG_SEAN",(char*)"A_C_HORSE_GANG_TRELAWNEY",(char*)"A_C_HORSE_GANG_UNCLE",(char*)"A_C_HORSE_GANG_SADIE_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_CHARLES_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_UNCLE_ENDLESSSUMMER",(char*)"A_C_HORSE_AMERICANPAINT_OVERO",(char*)"A_C_HORSE_AMERICANPAINT_TOBIANO",(char*)"A_C_HORSE_AMERICANPAINT_SPLASHEDWHITE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BLACK",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BUCKSKIN",(char*)"A_C_HORSE_APPALOOSA_BLANKET",(char*)"A_C_HORSE_APPALOOSA_LEOPARDBLANKET",(char*)"A_C_HORSE_ARABIAN_WHITE",(char*)"A_C_HORSE_HUNGARIANHALFBRED_FLAXENCHESTNUT",(char*)"A_C_HORSE_MUSTANG_GRULLODUN",(char*)"A_C_HORSE_MUSTANG_WILDBAY",(char*)"A_C_HORSE_MUSTANG_TIGERSTRIPEDBAY",(char*)"A_C_HORSE_NOKOTA_BLUEROAN",(char*)"A_C_HORSE_NOKOTA_WHITEROAN",(char*)"A_C_HORSE_THOROUGHBRED_BLOODBAY",(char*)"A_C_HORSE_THOROUGHBRED_DAPPLEGREY",(char*)"A_C_HorseMule_01",(char*)"A_C_Ox_01",(char*)"A_C_REDFOOTEDBOOBY_01",(char*)"A_C_SHARKHAMMERHEAD_01",(char*)"A_C_SharkTiger",(char*)"A_C_SHEEP_01",(char*)"A_C_SNAKEBLACKTAILRATTLE_01",(char*)"A_C_snakeferdelance_01",(char*)"A_C_ROSEATESPOONBILL_01",(char*)"A_C_TURKEY_01",(char*)"A_C_WOODPECKER_02",(char*)"A_C_PIG_01",(char*)"A_C_SNAKEWATER_01",(char*)"A_C_SPARROW_01",(char*)"A_C_Egret_01",(char*)"A_C_PARROT_01",(char*)"A_C_PRAIRIECHICKEN_01",(char*)"A_C_BAT_01",(char*)"A_C_Heron_01",(char*)"A_C_Oriole_01",(char*)"A_C_BLUEJAY_01",(char*)"A_C_CARDINAL_01",(char*)"A_C_Duck_01",(char*)"A_C_PIGEON",(char*)"A_C_ROBIN_01",(char*)"A_C_SONGBIRD_01",(char*)"A_C_Woodpecker_01",(char*)"A_C_IGUANA_01",(char*)"A_C_ROOSTER_01",(char*)"A_C_SNAKEFERDELANCE_01",(char*)"A_C_Armadillo_01",(char*)"A_C_IGUANADESERT_01",(char*)"A_C_Quail_01",(char*)"A_C_SNAKEREDBOA_01",(char*)"A_C_Chicken_01",(char*)"A_C_FROGBULL_01",(char*)"A_C_SQUIRREL_01",(char*)"A_C_Toad_01",(char*)"A_C_TURTLESNAPPING_01",(char*)"A_C_Bull_01",(char*)"A_C_Cow",(char*)"A_C_MUSKRAT_01",(char*)"A_C_RAT_01",(char*)"A_C_SNAKE_01",(char*)"A_C_Beaver_01",(char*)"A_C_PHEASANT_01",(char*)"A_C_Rabbit_01",(char*)"A_C_TurkeyWild_01",(char*)"A_C_BigHornRam_01",(char*)"A_C_Buck_01",(char*)"A_C_Buffalo_01",(char*)"A_C_DEER_01",(char*)"A_C_ELK_01",(char*)"A_C_MOOSE_01",(char*)"A_C_ProngHorn_01",(char*)"A_C_HORSE_MORGAN_BAY",(char*)"A_C_HORSE_MORGAN_BAYROAN",(char*)"A_C_HORSE_MORGAN_FLAXENCHESTNUT",(char*)"A_C_HORSE_MORGAN_PALOMINO",(char*)"A_C_HORSE_KENTUCKYSADDLE_BLACK",(char*)"A_C_HORSE_KENTUCKYSADDLE_CHESTNUTPINTO",(char*)"A_C_HORSE_KENTUCKYSADDLE_GREY",(char*)"A_C_HORSE_KENTUCKYSADDLE_SILVERBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_BLACKRABICANO",(char*)"A_C_HORSE_TENNESSEEWALKER_CHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_DAPPLEBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_REDROAN",(char*)"A_C_BEAR_01",(char*)"A_C_DOGAMERICANFOXHOUND_01",(char*)"A_C_DOGCOLLIE_01",(char*)"A_C_DogAustralianSheperd_01",(char*)"A_C_DogChesBayRetriever_01",(char*)"A_C_DogLab_01",(char*)"A_C_DogStreet_01",(char*)"A_C_DogBlueTickCoonHound_01",(char*)"A_C_DogHound_01",(char*)"A_C_TURTLESEA_01",(char*)"A_C_HAWK_01",(char*)"A_C_DUCK_01",(char*)"A_C_LOON_01",(char*)"A_C_CORMORANT_01",(char*)"A_C_GOOSECANADA_01",(char*)"A_C_HERON_01",(char*)"A_C_EGRET_01",(char*)"A_C_CraneWhooping_01",(char*)"A_C_PELICAN_01",(char*)"A_C_Badger_01",(char*)"A_C_BOAR_01",(char*)"A_C_JAVELINA_01",(char*)"A_C_OWL_01",(char*)"A_C_PANTHER_01",(char*)"A_C_POSSUM_01",(char*)"A_C_Raccoon_01",(char*)"A_C_SKUNK_01",(char*)"A_C_Alligator_01",(char*)"A_C_Alligator_02",(char*)"A_C_Alligator_03",(char*)"A_C_Bear_01",(char*)"A_C_BearBlack_01",(char*)"A_C_Boar_01",(char*)"A_C_CALIFORNIACONDOR_01",(char*)"A_C_Cougar_01",(char*)"A_C_Coyote_01",(char*)"A_C_Crow_01",(char*)"A_C_Eagle_01",(char*)"A_C_Fox_01",(char*)"A_C_Hawk_01",(char*)"A_C_Owl_01",(char*)"A_C_Panther_01",(char*)"A_C_RAVEN_01",(char*)"A_C_SEAGULL_01",(char*)"A_C_Vulture_01",(char*)"A_C_Wolf",(char*)"A_C_Wolf_Medium",(char*)"A_C_Wolf_Small",(char*)"A_C_FishMuskie_01_lg",(char*)"A_C_FISHLAKESTURGEON_01_LG",(char*)"A_C_FISHLONGNOSEGAR_01_LG",(char*)"A_C_FISHCHANNELCATFISH_01_LG",(char*)"A_C_FISHBLUEGIL_01_SM",(char*)"A_C_FISHPERCH_01_SM",(char*)"A_C_FISHCHAINPICKEREL_01_SM",(char*)"A_C_FISHROCKBASS_01_SM",(char*)"A_C_FISHREDFINPICKEREL_01_SM",(char*)"A_C_FISHBLUEGIL_01_MS",(char*)"A_C_FISHPERCH_01_MS",(char*)"A_C_FISHREDFINPICKEREL_01_MS",(char*)"A_C_FISHROCKBASS_01_MS",(char*)"A_C_FISHSMALLMOUTHBASS_01_MS",(char*)"A_C_FISHBULLHEADCAT_01_MS",(char*)"A_C_FISHLARGEMOUTHBASS_01_MS"
		};
		int szMENU_SPAWN_PEDS = sizeof(listMENU_SPAWN_PEDS) / sizeof(char*);
		simpleMenu.setOptionList(MENU_SPAWN_PEDS, szMENU_SPAWN_PEDS, listMENU_SPAWN_PEDS);

		//MENU_CHANGE_PED setup
		simpleMenu.setOptionList(MENU_CHANGE_PED, szMENU_SPAWN_PEDS, listMENU_SPAWN_PEDS);//same as MENU_SPAWN_PEDS



		//MENU_SPAWN_PEDS setip
		static char *listMENU_TELEPORTS[] = {
			(char*)"Saint Denis",
			(char*)"Van Horn",
			(char*)"Annesburg",
			(char*)"Maclean's House",
			(char*)"Hagen Orchards",
			(char*)"Caliga Hall",
			(char*)"Braithwaite Manor",
			(char*)"Emerald Ranch",
			(char*)"Scarlett Meadows",
			(char*)"Alone Tree",
			(char*)"Flatneck Station",
			(char*)"Wallace Station",
			(char*)"Riggs Station",
			(char*)"Valentine",
			(char*)"Chadwick Farm",
			(char*)"Fort Wallace",
			(char*)"Downes Ranch",
			(char*)"Strawberry",
			(char*)"Big Valley",
			(char*)"Holding Camp",
			(char*)"Blackwater",
			(char*)"Great Plains",
			(char*)"Quaker's Cove",
			(char*)"Thieves Landing"
		};
		int szMENU_TELEPORTS = sizeof(listMENU_TELEPORTS) / sizeof(char*);
		simpleMenu.setOptionList(MENU_TELEPORTS, szMENU_TELEPORTS, listMENU_TELEPORTS);

		//MENU_WEATHER
		static char *listMENU_WEATHER[] = { (char*)"OVERCAST", (char*)"RAIN", (char*)"FOG", (char*)"SNOWLIGHT", (char*)"THUNDER", (char*)"BLIZZARD", (char*)"SNOW", (char*)"MISTY", (char*)"SUNNY", (char*)"HIGHPRESSURE", (char*)"CLEARING", (char*)"SLEET", (char*)"DRIZZLE", (char*)"SHOWER", (char*)"SNOWCLEARING", (char*)"OVERCASTDARK", (char*)"THUNDERSTORM", (char*)"SANDSTORM", (char*)"HURRICANE", (char*)"HAIL", (char*)"WHITEOUT", (char*)"GROUNDBLIZZARD" };
		int szMENU_WEATHER = sizeof(listMENU_WEATHER) / sizeof(char*);
		simpleMenu.setOptionList(MENU_WEATHER, szMENU_WEATHER, listMENU_WEATHER);

		//MENU_TIME
		static char *listMENU_TIME[] = {
			(char*)"Add 4 hours",
			(char*)"Add 1 hour",
			(char*)"Add 15 minutes",
			(char*)"Back 4 hours",
			(char*)"Back 1 hour",
			(char*)"Back 15 minutes",
		};
		int szMENU_TIME = sizeof(listMENU_TIME) / sizeof(char*);
		simpleMenu.setOptionList(MENU_TIME, szMENU_TIME, listMENU_TIME);


		//HookCPedFactoryCreatePed();
		//Log::Info << "Natives hecking registered" << Log::Endl;
	}
	
	void Tick()
	{
		TickCpp();

		static bool lpressed = false;//this is basically for 'just pressed' detection in this function only, might want to expand it later on
		if ((GetKeyState(VK_LBUTTON) & 0x80) != 0 || (controllerMode && CONTROLS::IS_CONTROL_PRESSED(INPUT_GROUP_CONTROLLER, Buttons::Button_R2))) {
			LCJustPressed = true;
			if (LCpressed == true) {
				LCJustPressed = false;
			}

			LCpressed = true;
			if (lpressed == false) {
				if (forge::enabled) {
					forge::blacklistItem = forge::objectSelected;//make it so we can't pick up that previous one until we unaim
					if (forge::gravityGun) {
						//shoot object
						//Native::Invoke<void>(N::CLEAR_PED_TASKS_IMMEDIATELY, forge::objectSelected);
						forge::initiateGravityPulse(forge::objectSelected);
					}
					forge::forgetObject();
				}
				lpressed = true;
			}
		}
		else {
			LCJustPressed = false;
			LCpressed = false;
			lpressed = false;
		}
		if ((GetKeyState(VK_RBUTTON) & 0x80) != 0 || (controllerMode && CONTROLS::IS_CONTROL_PRESSED(INPUT_GROUP_CONTROLLER, Buttons::Button_L2))) {
			RCJustPressed = true;
			if (RCpressed == true) {
				RCJustPressed = false;
			}

			RCpressed = true;
		}
		else {
			RCpressed = false;
			RCJustPressed = false;
			//not pressed
			forge::blacklistItem = -1;
		}
	}




	//https://github.com/esc0rtd3w/illicit-sprx/blob/master/main/illicit/buttons.h
	float getLeftAxisX() {
		int value = CONTROLS::GET_CONTROL_VALUE(INPUT_GROUP_CONTROLLER, INPUT_MOVE_LR);
		value = value - 127;
		float v = (float)value / 127.0f;
		//-1 is fully left
		//0 is center
		//1 is fully right
		//static char buf[100];
		//snprintf(buf, sizeof(buf), "X: %f", v);
		//consolePrint(buf);
		return v;
		//return CONTROLS::GET_CONTROL_NORMAL(0, INPUT_SCRIPT_LEFT_AXIS_X);
	}
	float getLeftAxisY() {
		int value = CONTROLS::GET_CONTROL_VALUE(INPUT_GROUP_CONTROLLER, INPUT_MOVE_UD);
		value = value - 127;
		float v = (float)value / 127.0f;
		v = v * -1;//flip it so we get standard postitive is up and negative is down (like normal graphs)
		//-1 is fully down
		//0 is center
		//1 is fully up
		//static char buf[100];
		//snprintf(buf, sizeof(buf), "Y: %f", v);
		//consolePrint(buf);
		return v;
		//return CONTROLS::GET_CONTROL_NORMAL(0, INPUT_SCRIPT_LEFT_AXIS_Y)*-1;
	}

	void getAxisValues(float *rotation, float *magnitude) {
		float x = getLeftAxisX();
		float y = getLeftAxisY();
		float mag = sqrt(x * x + y * y);
		float rot = atan2(y, x) * 180 / 3.14159265358979323846264f;
		*rotation = rot;
		*magnitude = mag;
	}

	void enableDisableControls() {

		/*
		CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, INPUT_MOVE_LR, true);
		CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, INPUT_MOVE_UD, true);

		CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, INPUT_MOVE_LR);
		CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, INPUT_MOVE_UD);*/

		if (simpleMenu.isOpen()) {
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Up);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Down);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Left);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Right);

			
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_Circle);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_X);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_Square);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_Triangle);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_L1);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_L2);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_L3);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_R1);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_R2);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_R3);

			/*CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Up, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Down, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Left, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Right, true);

			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, INPUT_SCRIPT_LEFT_AXIS_X, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, INPUT_SCRIPT_LEFT_AXIS_Y, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_Circle, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_X, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_Square, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_Triangle, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_L1, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_L2, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_L3, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_R1, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_R2, true);
			CONTROLS::DISABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_R3, true);*/
			/*CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Up, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Down, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Left, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Dpad_Right, true);

			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, INPUT_SCRIPT_LEFT_AXIS_X, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, INPUT_SCRIPT_LEFT_AXIS_Y, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_Circle, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_X, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_Square, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_Triangle, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_L1, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_L2, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_L3, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_R1, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_R2, true);
			CONTROLS::ENABLE_CONTROL_ACTION(INPUT_GROUP_CONTROLLER, Buttons::Button_R3, true);*/
		}
	}

	void OnKeyDown(uint32_t key);
	void OnKeyUp(uint32_t key);
	void controllerInputKeyDownWrapper() {
		enableDisableControls();
		//F2 = r1 and square
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_R1) && CONTROLS::IS_CONTROL_PRESSED(INPUT_GROUP_CONTROLLER, Button_Square)) {
			OnKeyDown(0x71);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_R1) && CONTROLS::IS_CONTROL_RELEASED(INPUT_GROUP_CONTROLLER, Button_Square)) {
			OnKeyUp(0x71);
		}

		//back key VK_BACK = INPUT_FRONTEND_CANCEL
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_Circle)) {
			//print("\n\n\n          Circle Pressed");
			OnKeyDown(VK_BACK);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_Circle)) {
			OnKeyUp(VK_BACK);
		}


		//shift key = INPUT_SPRINT
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_R1)) {
			OnKeyDown(0x10);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_R1)) {
			OnKeyUp(0x10);
		}

		//VK_UP
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Dpad_Up)) {
			OnKeyDown(VK_UP);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Dpad_Up)) {
			OnKeyUp(VK_UP);
		}

		//VK_DOWN
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Dpad_Down)) {
			OnKeyDown(VK_DOWN);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Dpad_Down)) {
			OnKeyUp(VK_DOWN);
		}

		//VK_LEFT
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Dpad_Left)) {
			OnKeyDown(VK_LEFT);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Dpad_Left)) {
			OnKeyUp(VK_LEFT);
		}

		//VK_RIGHT
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Dpad_Right)) {
			OnKeyDown(VK_RIGHT);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Dpad_Right)) {
			OnKeyUp(VK_RIGHT);
		}

		//N key (for keyboard input) = square
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_Square)) {
			OnKeyDown(0x4E);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_Square)) {
			OnKeyUp(0x4E);
		}

		//VK_RETURN (select) = X
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_X)) {
			OnKeyDown(VK_RETURN);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_X)) {
			OnKeyUp(VK_RETURN);
		}

		//Wpressed 0x57
		if (getLeftAxisY() > 0.5f) {
			OnKeyDown(0x57);
		}
		if (getLeftAxisY() < 0.5f) {
			OnKeyUp(0x57);
		}

		//Spressed 0x53
		if (getLeftAxisY() < -0.5f) {
			OnKeyDown(0x53);
		}
		if (getLeftAxisY() > -0.5f) {
			OnKeyUp(0x53);
		}

		//Apressed 0x41
		if (getLeftAxisX() < -0.5f) {
			OnKeyDown(0x41);
		}
		if (getLeftAxisX() > -0.5f) {
			OnKeyUp(0x41);
		}

		//Dpressed 0x44
		if (getLeftAxisX() > 0.5f) {
			OnKeyDown(0x44);
		}
		if (getLeftAxisX() < 0.5f) {
			OnKeyUp(0x44);
		}

		//VK_CONTROL for go down in ufo
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_L3)) {
			//print("\n\n\n          L3 Pressed");
			OnKeyDown(VK_CONTROL);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_L3)) {
			OnKeyUp(VK_CONTROL);
		}

		//F key 0x46
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_Square)) {
			//print("\n\n\n          Square Pressed");
			OnKeyDown(0x46);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_Square)) {
			OnKeyUp(0x46);
		}

		//VK_SPACE for go up in ufo
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_X)) {
			//print("\n\n\n          X Pressed");
			OnKeyDown(VK_SPACE);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_X)) {
			OnKeyUp(VK_SPACE);
		}

		//0x45 E key (get in and exit ufo)
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(INPUT_GROUP_CONTROLLER, Button_Triangle)) {
			//print("\n\n\n          Triangle Pressed");
			OnKeyDown(0x45);
		}
		if (CONTROLS::IS_CONTROL_JUST_RELEASED(INPUT_GROUP_CONTROLLER, Button_Triangle)) {
			OnKeyUp(0x45);
		}
	}



	void OnKeyDown(uint32_t key)
	{
		if (isKeyboardOpen() == false) {
			if (key < 0xFF)
				keyPressed[key] = true;
			if (key == 0x71)
			{
				//noclipResource.Toggle();
				simpleMenu.toggle();
			}
			if (key == VK_BACK) {
				simpleMenu.back();
			}
			if (key == 0x10)
			{
				noclipResource.SetHighSpeed(true);
				shiftKeyPressed = true;
			}
			if (key == VK_UP) {
				if (shiftKeyPressed) {
					for (int i = 0; i < 5; i++)
						simpleMenu.down();
				}
				else {
					simpleMenu.down();
				}
			}
			if (key == VK_DOWN) {
				if (shiftKeyPressed) {
					for (int i = 0; i < 5; i++)
						simpleMenu.up();
				}
				else {
					simpleMenu.up();
				}
			}
			if (key == VK_RIGHT) {
				simpleMenu.right();
			}
			if (key == VK_LEFT) {
				simpleMenu.left();
			}
			if (key == 0x4E) {//N key
				simpleMenu.customInput();
			}

			if (key == VK_RETURN) {
				simpleMenu.select();
			}
			if (key == 0x57) {
				Wpressed = true;
			}
			if (key == 0x53) {
				Spressed = true;
			}
			if (key == 0x41) {
				Apressed = true;
			}
			if (key == 0x44) {
				Dpressed = true;
			}
			if (key == VK_SHIFT) {
				ShiftPressed = true;
			}
			if (key == VK_CONTROL) {
				CtrlPressed = true;
			}
			if (key == VK_RETURN) {
				EnterPressed = true;
			}
			
			if (key == 0x46) {//F Key
				if (forcePushBool)
					forcePush(100);//ORIGINALLY 50
				if (circlePedsForceEnabled)
					toggleCirclePeds();
				if (ufomod::enabled)
					ufomod::toggleWeapon();
			}
		}
	}

	void OnKeyUp(uint32_t key)
	{
		if (isKeyboardOpen() == false) {
			if (key < 0xFF)
				keyPressed[key] = false;
			if (key == 0x10)
			{
				noclipResource.SetHighSpeed(false);
				shiftKeyPressed = false;
			}
			if (key == 0x57) {
				Wpressed = false;
			}
			if (key == 0x53) {
				Spressed = false;
			}
			if (key == 0x41) {
				Apressed = false;
			}
			if (key == 0x44) {
				Dpressed = false;
			}
			if (key == VK_SHIFT) {
				ShiftPressed = false;
			}
			if (key == VK_CONTROL) {
				CtrlPressed = false;
			}
			if (key == VK_RETURN) {
				EnterPressed = false;
			}
		}

	}



void main()
{
	while (true)
	{

		//cheap fix for my button inputs from previous script injector

		/*if (IsKeyJustDown(0x74)) {//f5
			controllerMode = !controllerMode;
			if (controllerMode)
				print("Controller Mode ENABLED");
			else
				print("Controller Mode DISABLED");
		}*/

		bool previousInput = controllerMode; 

		controllerMode = !CONTROLS::_IS_INPUT_DISABLED(2);

		if (controllerMode != previousInput)
			inputModeChanged = true;
		else
			inputModeChanged = false;

		if (!controllerMode) {
			//consolePrint("Keyboard mode"); 
			for (int i = 0; i < KEYS_SIZE; i++) {
				if (IsKeyJustDown(i)) {
					OnKeyDown(i);
				}
				if (IsKeyJustUp(i)) {
					OnKeyUp(i);
				}
			}
		}
		else
			controllerInputKeyDownWrapper();//check for controller inputsss

		Tick();
		//update(); this is for those dumb entity boxes
		WAIT(0);
	}
}

void ScriptMain()
{
	Init();
	srand(GetTickCount());
	main();
}

void printDebugData() {

}
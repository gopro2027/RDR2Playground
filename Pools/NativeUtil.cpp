#include "pch.h"
#include "NativeUtil.h"


const double PI = 3.141592653589793238463;

Noclip::Noclip()
{
	enabled = false;
	speed = 1.0;
}

Vector3 GetCamDirection()
{
	uint32_t localPed = PLAYER::GET_PLAYER_PED(0);

	float pedHeading = ENTITY::GET_ENTITY_HEADING(localPed);
	float camHeading = CAM::GET_GAMEPLAY_CAM_RELATIVE_HEADING();
	float camPitch = CAM::GET_GAMEPLAY_CAM_RELATIVE_PITCH();

	float heading = (camHeading + pedHeading);

	float x = -sin(heading * PI / 180.f);
	float y = cos(heading * PI / 180.f);
	float z = sin(camPitch * PI / 180.f);

	float length = sqrt(x * x + y * y + z * z);
	if (length != 0)
	{
		x /= length;
		y /= length;
		z /= length;
	}

	return { x, y, z };
}

void Noclip::Tick()
{



	if (!enabled)
	{
		return;
	}

	uint32_t localPed = PLAYER::GET_PLAYER_PED(0);

	int VehicleHandle = getVehiclePedIsOn(localPed);//GET_VEHICLE_PED_IS_USING( myPed);
	if (VehicleHandle != -1)
		localPed = VehicleHandle;

	Vector3 curPosition = ENTITY::GET_ENTITY_COORDS(localPed, false, false);
	Vector3 camDirection = GetCamDirection();

	ENTITY::SET_ENTITY_VELOCITY(localPed, 0.f, 0.f, 0.f); 

	if (Wpressed/*CONTROLS::IS_CONTROL_PRESSED(0, GAMEPLAY::GET_HASH_KEY("input_move_up_only"))*/)
	{
		curPosition.x += speed * camDirection.x * (highSpeed ? 5 : 1);
		curPosition.y += speed * camDirection.y * (highSpeed ? 5 : 1);
		curPosition.z += speed * camDirection.z * (highSpeed ? 5 : 1);
	}

	if (Spressed/*CONTROLS::IS_CONTROL_PRESSED(0, GAMEPLAY::GET_HASH_KEY("input_move_down_only"))*/)
	{
		curPosition.x -= speed * camDirection.x * (highSpeed ? 5 : 1);
		curPosition.y -= speed * camDirection.y * (highSpeed ? 5 : 1);
		curPosition.z -= speed * camDirection.z * (highSpeed ? 5 : 1);
	}

	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(localPed, curPosition.x, curPosition.y, curPosition.z, true, true, true);
}

void Noclip::Toggle()
{
	auto flag = !enabled;

	uint32_t localPed = PLAYER::GET_PLAYER_PED(0);


	enabled = flag;
}

void Noclip::SetHighSpeed(bool enable)
{
	highSpeed = enable;
}




//We will imagine that our screen is 1920x1080. Default FullHD
void DrawGameRect(float x, float y, float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	//float fWidth = width / (float)1920;
	//float fHeight = height / (float)1080;
	//float fX = (x + width / 2) / (float)1920;
	//float fY = (y + height / 2) / (float)1080;

	GRAPHICS::DRAW_RECT(x, y, width, height, r, g, b, a, true, true/*confused screaming*/);
}

int* getTextJustification() {//0 = left, 1 = center, 2 = right
	static int* val = 0;
	if (val == 0) {
		constexpr CMemory::Pattern pattern("48 8B 41 10 33 D2 0F B6 0D");
		val = pattern.Search().GetOffset(9).Get<int*>();//from native SET_TEXT_CENTRE
	}
	return val;
}
float* getTextWrapMin() {
	return (float*)((uint64_t)getTextJustification() - 0x10);//0x10     0x64
}
float* getTextWrapMax() {
	return (float*)((uint64_t)getTextJustification() - 0xC);//0xC     0x60
}

struct rgba { char r, g, b, a; };
struct TextStyle
{
	rgba Color; // 0x00
	float FontScale; // 0x04
	float FontSize; // 0x08
	float TextWrapStart; // 0x0C
	float TextWrapEnd; // 0x10
	uint32_t Font; // 0x14
	uint32_t unk; // 0x18 Something related to widescreens
	uint16_t Align; // 0x1C
	bool DropShadow; // 0x20
	bool TextOutline; // 0x24
	char pad[0x3]; // 0x25
	bool unk2; // 0x28 Something related to widescreens
};
TextStyle* getTextProperties() {
	return (TextStyle*)((uint64_t)getTextJustification() - 0x1C);
}

int textFont = 2;//2 is also ver readable
void DrawGameText(float x, float y, char* text, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float scaleX, float scaleY, int justification, int font)
{
	//float fX = x / (float)1920;
	//float fY = y / (float)1080;
	UI::SET_TEXT_SCALE(scaleX, scaleY);//0.342f
	UI::SET_TEXT_COLOR_RGBA(r, g, b, a);
	//UI::SET_TEXT_CENTRE( center);
	*getTextJustification() = justification;
	if (justification == JUSTIFICATION_RIGHT) {
		//UI::SET_TEXT_WRAP(0.0f, x);
		//*getTextWrapMin() = 0.0f;
		//*getTextWrapMax() = x;
		getTextProperties()->TextWrapStart = 0.0f;
		getTextProperties()->TextWrapEnd = x;
	}
	else {
		//*getTextWrapMin() = 0.0f;
		//*getTextWrapMax() = 1.0f;
		getTextProperties()->TextWrapStart = 0.0f;
		getTextProperties()->TextWrapEnd = 1.0f;
	}
	//getTextProperties()->TextOutline = true;
	getTextProperties()->Font = font;
	char* varString = GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", text);
	UI::DRAW_TEXT(varString, x, y);
}

void DrawGameSprite(float x, float y, float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	//const char* textureDict = _CREATE_VAR_STRING( 10, "LITERAL_STRING", "boot_flow");
	//const char* textureName = _CREATE_VAR_STRING( 10, "LITERAL_STRING", "selection_box_bg_1d");
	//REQUEST_STREAMED_TEXTURE_DICT( textureDict);
	//DRAW_SPRITE( textureDict, textureName, x, y, width, height, 0.0f/*rot*/, r, g, b, a, true);

	TEXTURE::REQUEST_STREAMED_TEXTURE_DICT("boot_flow", false);
	GRAPHICS::DRAW_SPRITE("boot_flow", "selection_box_bg_1d", x, y, width, height, 0.0f/*rot*/, r, g, b, a, true);
}

/*float getTextWidth(char * text, int font, float scalex, float scaley)
{
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(scalex, scaley);
	UI::_0x51E7A037("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	return UI::_0xD12A643A(1);
}*/

void playSond(char* sound, char* ref) {
	AUDIO::PLAY_SOUND_FRONTEND(sound, ref, 1, 0);
}

bool shiftKeyPressed = false;

int myLastVehicle = 0;

using mlDeque = std::deque<std::pair<uint32_t, std::function<void(void)>>>;
mlDeque modelLoadQueue;

void LoadModel(uint32_t model, std::function<void(void)> callback)
{
	bool modelExist = STREAMING::IS_MODEL_VALID(model);
	if (modelExist)
	{
		modelLoadQueue.push_back({ model, callback });
	}
}

void RequestModel(uint32_t model)
{
	STREAMING::REQUEST_MODEL(model, false);
}

bool HasModelLoaded(uint32_t model)
{
	return STREAMING::HAS_MODEL_LOADED(model);
}

int SpawnLoadedVehicle(char* vehName)
{
	uint32_t model = GAMEPLAY::GET_HASH_KEY(vehName);
	if (myLastVehicle)
	{
		VEHICLE::DELETE_VEHICLE(&myLastVehicle);
		myLastVehicle = 0;
	}
	uint32_t myPed = PLAYER::GET_PLAYER_PED(0);
	Vector3 myPos = ENTITY::GET_ENTITY_COORDS(myPed, false, false /*idk*/);

	myLastVehicle = VEHICLE::CREATE_VEHICLE(model, myPos.x + 2, myPos.y + 2, myPos.z + 0.5, 49.29f, true, true, false, false);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
	return myLastVehicle;
}

int SpawnLoadedPed(char* pedName, Vector3 myPos)
{
	uint32_t model = GAMEPLAY::GET_HASH_KEY(pedName);

	uint32_t createdPed = PED::CREATE_PED(model, myPos.x, myPos.y, myPos.z, 0.f, false, false, false, false);
	ENTITY::SET_ENTITY_VISIBLE(createdPed, true);
	ENTITY::SET_ENTITY_ALPHA(createdPed, 255, false);
	PED::SET_PED_VISIBLE(createdPed, true);
	ENTITY::SET_ENTITY_AS_MISSION_ENTITY(createdPed, false, false);//make it despawn
	int createdPedCopy = createdPed;
	ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&createdPedCopy);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);

	return createdPed;
}

int SpawnLoadedObject(int objectHash, Vector3 myPos)
{
	uint32_t model = objectHash;

	uint32_t createdPed = OBJECT::CREATE_OBJECT(model, myPos.x, myPos.y, myPos.z, 1, 1, 0, 0, 1);
	ENTITY::SET_ENTITY_VISIBLE(createdPed, true);
	ENTITY::SET_ENTITY_ALPHA(createdPed, 255, false);
	PED::SET_PED_VISIBLE(createdPed, true);
	ENTITY::SET_ENTITY_AS_MISSION_ENTITY(createdPed, false, false);//make it despawn
	int createdPedCopy = createdPed;
	ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&createdPedCopy);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);

	return createdPed;
}

void ChangeLoadedPlayerModel(char* modelName)
{
	uint32_t model = GAMEPLAY::GET_HASH_KEY(modelName);

	PLAYER::SET_PLAYER_MODEL(0, model, true);

	uint32_t myPed = PLAYER::GET_PLAYER_PED(0);

	ENTITY::SET_ENTITY_VISIBLE(myPed, true);
	ENTITY::SET_ENTITY_ALPHA(myPed, 255, false);
	PED::SET_PED_VISIBLE(myPed, true);

	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
}

void TeleportPlayer(float x, float y, float z)
{
	uint32_t myPed = PLAYER::GET_PLAYER_PED(0);
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(myPed, x, y, z, false, false, false);
}

void SetWeather(char* weather)
{
	GAMEPLAY::_SET_WEATHER_TYPE_TRANSITION(GAMEPLAY::GET_HASH_KEY(weather), GAMEPLAY::GET_HASH_KEY(weather), 0.5f, true);
}

void AddToClockTime(int hours, int minutes, int seconds)
{
	TIME::ADD_TO_CLOCK_TIME(hours, minutes, seconds);
}

void SpawnPed(const char* model, std::function<void(int)> callback, Vector3* location)
{
	LoadModel(GAMEPLAY::GET_HASH_KEY((char*)model), [model, callback, location] {
		uint32_t myPed = PLAYER::GET_PLAYER_PED(0);
		static Vector3 myPos = { 0,0,0 };
		myPos = ENTITY::GET_ENTITY_COORDS(myPed, false, false/*unknown*/);
		Vector3* loc = location;
		if (loc == 0)
			loc = &myPos;
		int ped = SpawnLoadedPed((char*)model, *loc);
		if (callback != nullptr)
			callback(ped);
		});
}

void SpawnObject(int model, Vector3* location, std::function<void(int)> callback)
{
	LoadModel(model, [model, callback, location] {
		uint32_t myPed = PLAYER::GET_PLAYER_PED(0);
		static Vector3 myPos = { 0,0,0 };
		myPos = ENTITY::GET_ENTITY_COORDS(myPed, false, false/*unknown*/);
		Vector3* loc = location;
		if (loc == 0)
			loc = &myPos;
		int ped = SpawnLoadedObject(model, *loc);
		if (callback != nullptr)
			callback(ped);
		});
}

void ChangePlayerModel(const char* model)
{
	LoadModel(GAMEPLAY::GET_HASH_KEY((char*)model), [model] {
		ChangeLoadedPlayerModel((char*)model);
		});
}


void SpawnVehicle(const char* vehName, std::function<void(int)> callback)
{
	LoadModel(GAMEPLAY::GET_HASH_KEY((char*)vehName), [vehName, callback] {
		int veh = SpawnLoadedVehicle((char*)vehName);
		if (callback != nullptr)
			callback(veh);
		});
}

bool keyboardOpen = false;
std::function<void(char*)> keyboardcallback = nullptr;
void KeyboardInput(std::function<void(char*)> callback, char* defaultText) {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(1, "FMMC_MPM_NA", "", defaultText, "", "", "", 256); // Display the dialog text
	keyboardOpen = true;
	keyboardcallback = callback;

}

void KeyboardTick() {
	if (keyboardOpen) {
		CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(0);
		/*
		0 - User still editing
		1 - User has finished editing
		2 - User has canceled editing
		3 - Keyboard isn't active
		*/
		int keyboardID = GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD();
		if (keyboardID != 0) {
			keyboardOpen = false;
			if (keyboardID == 1) {
				//successful finish editing
				char* text = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
				keyboardcallback(_strdup(text));
			}
		}
	}
}

char* keyboardInput(char* defaultText) {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(1, "FMMC_MPM_NA", "", defaultText, "", "", "", 256);
	int keyboardID = 0;
	while ((keyboardID = GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD()) == 0) WAIT(1);
	if (keyboardID == 1) {
		return _strdup(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
	}
	return nullptr;//not a success
}
bool keyboardInput(char* defaultText, int* ptr) {
	char* text = keyboardInput(defaultText);
	if (text == nullptr)
		return false;
	*ptr = atoi(text);
	return true;
}
bool keyboardInput(char* defaultText, float* ptr) {
	char* text = keyboardInput(defaultText);
	if (text == nullptr)
		return false;
	*ptr = atof(text);
	return true;
}

bool isKeyboardOpen() {
	return keyboardOpen;
}

int getVehiclePedIsOn(int ped) {
	if (PED::IS_PED_ON_MOUNT(ped)) {
		return PED::GET_MOUNT(ped);
	}
	if (PED::IS_PED_IN_ANY_VEHICLE(ped, true))
		return PED::GET_VEHICLE_PED_IS_USING(ped);
	return -1;
}

bool godmode = false;
void setInvincible(bool inv) {
	int myPed = PLAYER::PLAYER_PED_ID();
	int VehicleHandle = getVehiclePedIsOn(myPed);//GET_VEHICLE_PED_IS_USING( myPed);
	if (VehicleHandle != -1)
		ENTITY::SET_ENTITY_INVINCIBLE(VehicleHandle, inv);
	PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), inv);

	PED::SET_PED_CAN_RAGDOLL(myPed, !inv);
	//SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT( myPed, !inv);
	//SET_PED_RAGDOLL_ON_COLLISION( myPed, !inv);
	ENTITY::SET_ENTITY_PROOFS(myPed, inv, inv, inv, inv, inv, inv, inv, inv);

	PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(myPed, false);//I would think it would be false

	//SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE( myPed, !inv);//doesn't seem to work :/
}
void toggleGodmode() {
	int myPed = PLAYER::PLAYER_PED_ID();
	if (godmode == false) {
		setInvincible(false);
		PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(myPed, true);
		//SET_ENTITY_PROOFS( myPed, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	else {
		//SET_ENTITY_PROOFS( myPed, 1, 1, 1, 1, 1, 1, 1, 1);
	}
}
void invincibilityTick() {
	if (godmode) {
		//consolePrint((char*)"God Mode");
		setInvincible(true);
	}
}

#define PI 3.14159265



#define nearbyPedArraySize 100
struct nearbyEnts {
	int size;
	int64_t entities[nearbyPedArraySize];
};


void runOnAllNearbyPedsToPlayer(std::function<void(int)> callback) {
	nearbyEnts arr;
	arr.size = nearbyPedArraySize;
	int ped = PLAYER::PLAYER_PED_ID();
	int size = PED::GET_PED_NEARBY_PEDS(ped, (int*)&arr, ped, 0/*unk*/);
	for (int i = 0; i < size; i++) {
		callback(arr.entities[i]);
	}
}

void runOnAllNearbyVehiclesToPlayer(std::function<void(int)> callback) {
	nearbyEnts arr;
	arr.size = nearbyPedArraySize;
	int ped = PLAYER::PLAYER_PED_ID();
	int size = PED::GET_PED_NEARBY_VEHICLES(ped, (int*)&arr);
	for (int i = 0; i < size; i++) {
		callback(arr.entities[i]);
	}
}

void runOnAllPeds(std::function<void(int)> callback, bool includeSelf) {
	int ents[0xFF];
	int ppid = PLAYER::PLAYER_PED_ID();
	if (includeSelf)
		ppid = 0;
	int size = worldGetAllPeds(ents, sizeof(ents) / sizeof(int));
	for (int i = 0; i < size; i++) {
		if (ents[i] != ppid)
			callback(ents[i]);
	}
}

void runOnAllVehicles(std::function<void(int)> callback) {
	int ents[0xFF];
	int size = worldGetAllVehicles(ents, sizeof(ents) / sizeof(int));
	for (int i = 0; i < size; i++) {
		callback(ents[i]);
	}
}

void runOnAllObjects(std::function<void(int)> callback) {
	int ents[0xFF];//might be more for this
	int size = worldGetAllObjects(ents, sizeof(ents) / sizeof(int));
	for (int i = 0; i < size; i++) {
		callback(ents[i]);
	}
}

void runOnAllPickups(std::function<void(int)> callback) {
	int ents[0xFF];//might be more for this
	int size = worldGetAllPickups(ents, sizeof(ents) / sizeof(int));
	for (int i = 0; i < size; i++) {
		callback(ents[i]);
	}
}

void runOnAllNearbyPedsAndVehiclesToPlayer(std::function<void(int)> callback) {
	runOnAllNearbyPedsToPlayer(callback);
	runOnAllNearbyVehiclesToPlayer(callback);
}

void runOnAllPedsAndVehicles(std::function<void(int)> callback, bool includeSelf) {
	runOnAllPeds(callback, includeSelf);
	runOnAllVehicles(callback);
}

bool forcePushBool = false;
void forcePush(float force, float maxDistanceAway) {
	runOnAllNearbyPedsAndVehiclesToPlayer([force, maxDistanceAway](auto ped) {
		Vector3 me = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false /*idk*/);
		Vector3 them = ENTITY::GET_ENTITY_COORDS(ped, true, false/*idk*/);
		Vector3 sub = them.sub(me);
		float distanceAway = sub.magnitude();
		//const float maxDistanceAway = 10.0f;
		if (distanceAway <= maxDistanceAway) {

			PED::SET_PED_TO_RAGDOLL(ped, 1000, 1000, 0, 0, 0, 0);//have to ragdoll before it can push them

			float multiplier = (maxDistanceAway - distanceAway) / maxDistanceAway;//so examples:
														 //7 = 30% push, (10-7)/10 = 0.3
														 //0 = 100% push, (10-0)/10 = 1

			Vector3 norm = sub.normalize();
			Vector3 Force = norm.scale(force * multiplier);

			ENTITY::SET_ENTITY_VELOCITY(ped, Force.x, Force.y, Force.z);
		}
		});
}



void APPLY_FORCE(int Entity, Vector3 Force, Vector3 Position, int type = 1, bool relative = true) {
	//APPLY_FORCE_TO_ENTITY(Entity, 5, Force.x, Force.y, Force.z, Position.x, Position.y, Position.z, 0, relative, true/*ignore up vec, set to true in the past*/, true/*true makes it have same accelleration for all objects, ie mass dependent or not*/, 0, 1);
	//SET_OBJECT_PHYSICS_PARAMS(Entity, 1000.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0);//last 3 0xbf800000
	/*SET_OBJECT_PHYSICS_PARAMS( Entity, 200000000.0, 1, 1000, 1, 0, 0, 0, 0, 0, 0, 0);
	SET_ACTIVATE_OBJECT_PHYSICS_AS_SOON_AS_IT_IS_UNFROZEN(Entity,true);
	APPLY_FORCE_TO_ENTITY( Entity, 1, Force.x * 1, Force.y * 1, Force.z * 1, 0.0f, 0.0f, 0.0f, 1, false, true, true, true, true);*/
	//APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS( Entity, 1, Force.x, Force.y, Force.z, false, false/*relative dire*/, true, false);
	//SET_OBJECT_PHYSICS_PARAMS( Entity, 200000000.0, 1, 1000, 1, 0, 0, 0, 0, 0, 0, 0);

	PED::SET_PED_TO_RAGDOLL(Entity, 1000, 1000, 0, 0, 0, 0);//have to ragdoll before it can push them

	//SET_ACTIVATE_OBJECT_PHYSICS_AS_SOON_AS_IT_IS_UNFROZEN( Entity, true);

	//APPLY_FORCE_TO_ENTITY( Entity, 1, Force.x, Force.y, Force.z, 0.0f,0.0f,0.0f, 0, 0, 1, 1, 1, 1);
	ENTITY::SET_ENTITY_VELOCITY(Entity, Force.x, Force.y, Force.z);//this working fucking beautifully

}
void forceEntity(int entity, float x, float y, float z, float rotx, float roty, float rotz, int type, bool relative) {
	Vector3 force = { x,y,z };
	Vector3 rot = { rotz,roty,rotz };
	APPLY_FORCE(entity, force, rot, type, relative);
}
void forceEntity(int entity, float x, float y, float z) {
	Vector3 force = { x,y,z };
	Vector3 rot = { 0,0,0 };
	APPLY_FORCE(entity, force, rot, 0, 0);
}
void intoTheSky() {
	int VehicleHandle = getVehiclePedIsOn(PLAYER::PLAYER_PED_ID());//GET_VEHICLE_PED_IS_USING( PLAYER_PED_ID());
	if (!ENTITY::DOES_ENTITY_EXIST(VehicleHandle))
		VehicleHandle = PLAYER::PLAYER_PED_ID();
	forceEntity(VehicleHandle, 0, 0, 500, 0, 0, 0, 1/*type*/, false);//non relative so straight up hopefully?
}

/*
uint64_t getMountOfPed(uint64_t myPedAddress) {
	uint64_t eax = *(uint64_t*)(myPedAddress+0x9C);
	eax = eax & 0x1FFFF;
	//fuk it
}*/

void testMount(int targetPed) {
	int myPed = PLAYER::PLAYER_PED_ID();
	//TASK_MOUNT_ANIMAL: 0, 97288, 20000, 4294967295, 2, 1, 0, 0
	//TASK_MOUNT_ANIMAL: 0, 267015, 4294967295, 4294967295, 1, 1, 0, 0
	//AI::TASK_MOUNT_ANIMAL(myPed,targetPed, 20000,-1,  2.0f,1,0,0);//5th arg is a float

	/*
	//these 3 natives return 0
	uint64_t a = Native::Invoke<uint64_t>(0x95CBC65780DE7EB1,myPed,targetPed);//sometimes returns 1
	uint64_t b = Native::Invoke<uint64_t>(0x4C8B59171957BCF7, targetPed);
	uint64_t c = Native::Invoke<uint64_t>(0xAAB0FE202E9FC9F0, myPed, targetPed);


	//all these 4 natives return my player ped id
	uint64_t d = Native::Invoke<uint64_t>(0xED1C764997A86D5A, myPed, targetPed);
	uint64_t e = Native::Invoke<uint64_t>(0xB8AB265426CFE6DD, myPed, targetPed);

	uint64_t f = Native::Invoke<uint64_t>(0xA691C10054275290, myPed, targetPed, -1);
	uint64_t g = Native::Invoke<uint64_t>(0x931B241409216C1F, myPed, targetPed, -1);


	Log::Info << a << ", " << b << ", " << c << ", " << d << ", " << e << ", " << f << ", " << g << Log::Endl;
	*/
}

void mountPatch() {
	constexpr CMemory::Pattern pattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 D8 41 83 F9 FE 0F 29 78 C8 41 BE ? ? ? ? 41 8D 41 01 41 8B F0 44 0F 45 F0 8B DA 44 8B F9 41 83 F8 FF 74 09 45 85 C0");
	CMemory m = pattern.Search().Add(0x14D);
	m.Nop(6);
	m.Add(0x4F);
	m.Nop(6);
}

void testMount2(int targetPed) {
	int myPed = PLAYER::PLAYER_PED_ID();
	PED::_0xAAB0FE202E9FC9F0(myPed, targetPed);
}


bool keyPressed[0xFF] = { false };
bool Wpressed, Spressed, Apressed, Dpressed = false;
bool RCpressed, LCpressed, RCJustPressed, LCJustPressed = false;
bool ShiftPressed, CtrlPressed, EnterPressed = false;
bool fastHorseEnabled = false;

void fastHorse() {
	if (fastHorseEnabled) {
		//consolePrint((char*)"Need For Speed");
		int VehicleHandle = getVehiclePedIsOn(PLAYER::PLAYER_PED_ID());// GET_VEHICLE_PED_IS_USING( PLAYER_PED_ID());
		if (VehicleHandle == -1)
			return;
		//consolePrint((char*)"We have your vehicle...");
		int speed = ENTITY::GET_ENTITY_SPEED(VehicleHandle);
		int newspeed = speed + 1;

		float trainSpeed = 1000;
		bool inTrain = PLAYER::IS_PLAYER_RIDING_TRAIN(0);
		if (Wpressed) {
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(VehicleHandle, newspeed);

			if (inTrain) {
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(VehicleHandle, trainSpeed);
				VEHICLE::SET_TRAIN_SPEED(VehicleHandle, trainSpeed);
				VEHICLE::SET_TRAIN_CRUISE_SPEED(VehicleHandle, trainSpeed);
			}
		}
		if (Spressed) {
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(VehicleHandle, -newspeed);

			if (inTrain) {
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(VehicleHandle, -trainSpeed);
				VEHICLE::SET_TRAIN_SPEED(VehicleHandle, -trainSpeed);
				VEHICLE::SET_TRAIN_CRUISE_SPEED(VehicleHandle, -trainSpeed);
			}
		}
		if (Wpressed && Spressed) {
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(VehicleHandle, 0);
			VEHICLE::SET_TRAIN_SPEED(VehicleHandle, 0);
			VEHICLE::SET_TRAIN_CRUISE_SPEED(VehicleHandle, 0);
		}
	}
}


void tpIntoClosestVehicle() {
	int myPed = PLAYER::PLAYER_PED_ID();
	Vector3 tmp = ENTITY::GET_ENTITY_COORDS(myPed, true, false /*idk*/);
	int foundVehicle = VEHICLE::GET_CLOSEST_VEHICLE(tmp.x, tmp.y, tmp.z, 1000000.0f, 0, 0/*flag*/);
	if (foundVehicle != 0) {
		PED::SET_PED_INTO_VEHICLE(myPed, foundVehicle, -1);
	}
	else {
		print((char*)"Could not find a vehicle nearby");
	}
}

void LoadModelTick()
{
	KeyboardTick();//just throw this in here...
	fastHorse();
	invincibilityTick();


	for (int i = 0; i < modelLoadQueue.size(); ++i)
	{
		auto& it = modelLoadQueue[i];
		if (HasModelLoaded(it.first))
		{
			auto callbackFunc = it.second;
			callbackFunc();
			modelLoadQueue.erase(modelLoadQueue.begin() + i, modelLoadQueue.begin() + i + 1);
			--i;
		}
		else
			RequestModel(it.first);
	}
}


struct WeaponInfo
{
	std::string name;
	std::string uiname;
};

WeaponInfo weaponInfos[]{
	{ "LASSO",                      "LASSO"             },
	{ "FISHINGROD",					"FISHINGROD"        },
	{ "MOONSHINEJUG",				"MOONSHINEJUG"      },
	{ "MELEE_CLEAVER",				"CLEAVER"           },
	{ "MELEE_HATCHET",				"HATCHET"           },
	{ "MELEE_KNIFE",				"KNIFE"             },
	{ "MELEE_MACHETE",				"MACHETE"           },
	{ "MELEE_TORCH",				"TORCH"             },
	{ "MELEE_LANTERN",				"LANTERN"           },
	{ "MELEE_DAVY_LANTERN",			"DAVY LANTERN"      },
	{ "MELEE_LANTERN_ELECTRIC",		"LANTERN ELECTRIC"  },
	{ "MELEE_BROKEN_SWORD",			"BROKEN SWORD"      },
	{ "MELEE_KNIFE_CIVIL_WAR",		"KNIFE CIVIL WAR"   },
	{ "MELEE_ANCIENT_HATCHET",		"ANCIENT HATCHET"   },
	{ "REVOLVER_CATTLEMAN",			"CATTLEMAN"         },
	{ "REVOLVER_DOUBLEACTION",		"DOUBLEACTION"      },
	{ "REVOLVER_SCHOFIELD",			"SCHOFIELD"         },
	{ "PISTOL_VOLCANIC",			"VOLCANIC"          },
	{ "PISTOL_MAUSER",				"MAUSER"            },
	{ "PISTOL_SEMIAUTO",			"SEMIAUTO"          },
	{ "REPEATER_CARBINE",			"CARBINE"           },
	{ "REPEATER_WINCHESTER",		"WINCHESTER"        },
	{ "REPEATER_HENRY",				"HENRY"             },
	{ "RIFLE_SPRINGFIELD",			"SPRINGFIELD"       },
	{ "RIFLE_BOLTACTION",			"BOLTACTION"        },
	{ "RIFLE_VARMINT",				"VARMINT"           },
	{ "SHOTGUN_DOUBLEBARREL",		"DOUBLEBARREL"      },
	{ "SHOTGUN_SAWEDOFF",			"SAWEDOFF"          },
	{ "SHOTGUN_SEMIAUTO",			"SEMIAUTO"          },
	{ "SHOTGUN_REPEATING",			"REPEATING"         },
	{ "SHOTGUN_PUMP",				"PUMP"              },
	{ "SNIPERRIFLE_CARCANO",		"CARCANO"           },
	{ "SNIPERRIFLE_ROLLINGBLOCK",	"ROLLINGBLOCK"      },
	{ "BOW",						"BOW"               },
	{ "THROWN_DYNAMITE",			"DYNAMITE"          },
	{ "THROWN_DYNAMITE_VOLATILE",	"DYNAMITE VOLATILE" },
	{ "THROWN_MOLOTOV",				"MOLOTOV"           },
	{ "THROWN_MOLOTOV_VOLATILE",	"MOLOTOV VOLATILE"  },
	{ "THROWN_THROWING_KNIVES",		"THROWING KNIVES"   },
	{ "THROWN_TOMAHAWK",			"TOMAHAWK"          },
	{ "KIT_BINOCULARS",				"BINOCULARS"        },
	{ "KIT_CAMERA",              	"CAMERA"            }
};
//char *allWeapons[] = { "p_camerabox01x","s_interact_jug_pickup","s_interact_lantern03x_pickup","s_interact_torch","w_melee_brokenSword01","w_melee_fishingpole02","w_melee_hatchet01","w_melee_hatchet02","w_melee_hatchet03","w_melee_hatchet04","w_melee_hatchet05","w_melee_hatchet06","w_melee_hatchet06","w_melee_hatchet07","w_melee_hatchet07","w_melee_knife01","w_melee_knife02","w_melee_knife03","w_melee_knife05","w_melee_knife14","w_melee_knife16","w_melee_knife17","w_melee_knife18","w_melee_lasso01","w_melee_machete01","w_melee_tomahawk01","w_melee_tomahawk02","w_pistol_m189901","w_pistol_mauser01","w_pistol_mauser01","w_pistol_semiauto01","w_pistol_volcanic01","w_repeater_carbine01","w_repeater_evans01","w_repeater_henry01","w_repeater_pumpaction01","w_repeater_winchester01","w_revolver_cattleman01","w_revolver_cattleman01","w_revolver_cattleman02","w_revolver_cattleman03","w_revolver_doubleaction01","w_revolver_doubleaction02","w_revolver_doubleaction04","w_revolver_doubleaction06","w_revolver_lemat01","w_revolver_schofield01","w_revolver_schofield03","w_revolver_schofield04","w_rifle_boltaction01","w_rifle_carcano01","w_rifle_rollingblock01","w_rifle_rollingblock01","w_rifle_springfield01","w_shotgun_doublebarrel01","w_shotgun_doublebarrel01","w_shotgun_pumpaction01","w_shotgun_repeating01","w_shotgun_sawed01","w_shotgun_semiauto01","w_sp_bowarrow","w_throw_dynamite01","w_throw_molotov01" };
void giveAllWeapons() {
	//for (int i = 0; i < sizeof(allWeapons)/sizeof(const char*); i++) {
	//	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(PLAYER::PLAYER_PED_ID(), GAMEPLAY::GET_HASH_KEY(allWeapons[i]), -1, true, 0x2cd419dc);//-1 may be 9999
	//}
	for each (auto info in weaponInfos) {
		Hash hash = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(("WEAPON_" + info.name).c_str()));
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(PLAYER::PLAYER_PED_ID(), hash, -1, true, 0x2cd419dc);
		WEAPON::SET_PED_AMMO(PLAYER::PLAYER_PED_ID(), hash, 1000);
		WEAPON::SET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), hash, 1, 0, 0, 0);
	}
}
Hash zombieGroup = 0;
Hash humanGroup = 0;
void addPedToHumanGroup(int human) {
	if (zombieGroup == 0 || humanGroup == 0) {
		PED::ADD_RELATIONSHIP_GROUP("ZOMBIE_GROUP", &zombieGroup);
		PED::ADD_RELATIONSHIP_GROUP("HUMAN_GROUP", &humanGroup);

		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, zombieGroup, humanGroup);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, humanGroup, zombieGroup);
	}

	//SET_RELATIONSHIP_BETWEEN_GROUPS( 5, zombieGroup, String::Hash("PLAYER"));
	//SET_RELATIONSHIP_BETWEEN_GROUPS( 5, String::Hash("PLAYER"), zombieGroup);

	PED::SET_PED_RELATIONSHIP_GROUP_DEFAULT_HASH(human, humanGroup);
	PED::SET_PED_RELATIONSHIP_GROUP_HASH(human, humanGroup);
}
void addPedToZombieGroup(int zombie) {
	if (zombieGroup == 0 || humanGroup == 0) {
		PED::ADD_RELATIONSHIP_GROUP("ZOMBIE_GROUP", &zombieGroup);
		PED::ADD_RELATIONSHIP_GROUP("HUMAN_GROUP", &humanGroup);

		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, zombieGroup, humanGroup);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, humanGroup, zombieGroup);
	}

	//SET_RELATIONSHIP_BETWEEN_GROUPS( 5, zombieGroup, String::Hash("PLAYER"));
	//SET_RELATIONSHIP_BETWEEN_GROUPS( 5, String::Hash("PLAYER"), zombieGroup);

	PED::SET_PED_RELATIONSHIP_GROUP_DEFAULT_HASH(zombie, zombieGroup);
	PED::SET_PED_RELATIONSHIP_GROUP_HASH(zombie, zombieGroup);
}

void setPedToCombatGroup(int ped) {

	//consolePrint((char*)"Making Zombie...");
	Hash zombieGroup = 0;
	PED::ADD_RELATIONSHIP_GROUP("ZOMBIE_GROUP", &zombieGroup);
	Hash humanGroup = 0;
	PED::ADD_RELATIONSHIP_GROUP("HUMAN_GROUP", &humanGroup);

	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, zombieGroup, humanGroup);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, humanGroup, zombieGroup);

	//SET_RELATIONSHIP_BETWEEN_GROUPS( 5, zombieGroup, String::Hash("PLAYER"));
	//SET_RELATIONSHIP_BETWEEN_GROUPS( 5, String::Hash("PLAYER"), zombieGroup);

	PED::SET_PED_RELATIONSHIP_GROUP_DEFAULT_HASH(ped, zombieGroup);
	PED::SET_PED_RELATIONSHIP_GROUP_HASH(ped, zombieGroup);
	PED::SET_PED_RELATIONSHIP_GROUP_DEFAULT_HASH(PLAYER::PLAYER_PED_ID(), humanGroup);
	PED::SET_PED_RELATIONSHIP_GROUP_HASH(PLAYER::PLAYER_PED_ID(), humanGroup);
	//addPedToHumanGroup(PLAYER::PLAYER_PED_ID());
	//addPedToZombieGroup(ped);


	AI::TASK_COMBAT_HATED_TARGETS(ped, 1000.0f);//unsure  0x8182B561A29BD597

	PED::SET_PED_FLEE_ATTRIBUTES(ped, 0, 0);//unsure
	PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, 1);//unsure

	PED::SET_PED_KEEP_TASK(ped, true);//unsure

	PED::_0x89F5E7ADECCCB49C(ped, "very_drunk");

	ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, true, true);//required to work for this method

	return;

}

void setPedToCombatJustMe(int ped) {



	//SET_ENTITY_AS_MISSION_ENTITY(ped,true,true);//new one... apparently makes it so the game will no longer control it, and I will

	//SET_ENTITY_MAX_SPEED(ped,1.3f);

	AI::CLEAR_PED_TASKS_IMMEDIATELY(ped, true, true);//kick them from vehicle and reset
	//GIVE_WEAPON_TO_PED(ped, GAMEPLAY::GET_HASH_KEY("WEAPON_KNIFE"), 1, false, false);

	if (PED::IS_PED_IN_ANY_VEHICLE(ped, true)) {
		int veh = PED::GET_VEHICLE_PED_IS_IN(ped, false);
		VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, 0);//stop vehicle
	}

	PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);
	AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);
	PED::SET_PED_FLEE_ATTRIBUTES(ped, 0, 0);
	PED::SET_PED_COMBAT_ATTRIBUTES(ped, 17, 1);
	PED::SET_PED_TARGET_LOSS_RESPONSE(ped, 2);//search again if target is lost
	PED::SET_PED_COMBAT_RANGE(ped, 2);//far range
	PED::SET_PED_SEEING_RANGE(ped, 10000000.0f);//see far away
	PED::SET_PED_VISUAL_FIELD_PERIPHERAL_RANGE(ped, 10000000.0f);
	PED::SET_PED_COMBAT_ABILITY(ped, 2);//set to professional
	PED::SET_PED_COMBAT_MOVEMENT(ped, 3);//set them to be offensive

	AI::TASK_LEAVE_VEHICLE(ped, PED::GET_VEHICLE_PED_IS_IN(ped, false), 0, 0);//256

//	REGISTER_TARGET( ped, PLAYER_PED_ID());//this doesn't seem to do anything

	AI::TASK_COMBAT_PED(ped, PLAYER::PLAYER_PED_ID(), 0, 16);//this is what does the trick

	//PED::SET_PED_IS_DRUNK(ped,true);//make them walk slow and stuff

	int BF[] = { 0,1,2,3,5,20,46,52 };
	for (int i = 0; i < sizeof(BF) / 4; i++)
		PED::SET_PED_COMBAT_ATTRIBUTES(ped, BF[i], true);

	PED::_0x89F5E7ADECCCB49C(ped, "very_drunk");

	ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, false, false);

}

const char* zombiearr[] = { "A_M_M_ARMCHOLERACORPSE_01","A_F_M_ARMCHOLERACORPSE_01","A_F_M_UniCorpse_01","A_M_M_UniCorpse_01","U_M_M_APFDeadMan_01" };

void spawnZombieTest() {
	//A_M_M_ARMCHOLERACORPSE_01
	//A_F_M_ARMCHOLERACORPSE_01
	//A_F_M_UniCorpse_01
	//A_M_M_UniCorpse_01
	//A_M_Y_UniCorpse_01
	//CS_crackpotRobot
	//U_M_M_APFDeadMan_01
	//U_M_M_CKTManager_01
	static int i = 0;
	SpawnPed((char*)zombiearr[i], [](auto ped) {
		setPedToCombatJustMe(ped);
		});

	i++;
	if (i >= 5)
		i = 0;
}

int getEntityAimedAt() {
	int ent = -1;
	bool found = PLAYER::GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(0, &ent);
	if (found) {
		return ent;
	}
	return -1;
}

int getEntityModel(int object) {
	return ENTITY::GET_ENTITY_MODEL(object);
}

Vector3 GET_COORDS_INFRONT_OF_ANGLE(Vector3 ro, float d, float offset) {
	Vector3 rot;
	float x = ro.z + 90 + offset;
	while (x < 0) x = x + 360;
	while (x > 360) {
		x = x - 360;
	}
	bool fix = false;
	if (x > 180) {
		fix = true;
	}
	rot.x = x;
	rot.z = ro.x;
	float cosrotz = cos(rot.z * PI / 180.0);
	float x1 = 0;
	float y1 = 0;
	float z1 = d * sin(rot.z * PI / 180.0);
	if (fix == true) {
		rot.x = (rot.x - 180);
		x1 = -d * cos(rot.x * PI / 180.0) * cosrotz;
		y1 = -d * sin(rot.x * PI / 180.0) * cosrotz;
	}
	else {
		x1 = d * cos(rot.x * PI / 180.0) * cosrotz;
		y1 = d * sin(rot.x * PI / 180.0) * cosrotz;
	}
	Vector3 Coords = { x1,y1,z1 };
	return Coords;
}

Vector3 GET_COORDS_INFRONT(float d, float offset) {
	Vector3 ro = CAM::GET_GAMEPLAY_CAM_ROT(2);
	return GET_COORDS_INFRONT_OF_ANGLE(ro, d, offset);
}


namespace forge {
	bool enabled = false;
	void toggle() {
		if (enabled == false)
			forgetObject();
	}
	int objectSelected = -1;
	int blacklistItem = -1;
	void forgetObject() {
		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(objectSelected, false, false);
		objectSelected = -1;
	}
	void setObject(int obj) {
		objectSelected = obj;
		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(objectSelected, true, true);

	}
	bool gravityGun = false;
	int gravityGunObjectID = -1;
	int gravityGunTimer = 0;

	void gravityGunWaiter() {
		if (gravityGunObjectID != -1) {
			if (gravityGunTimer <= 0) {

				Vector3 v = GET_COORDS_INFRONT(400);
				//static char buf[100];
				//sprintf(buf, "%f %f %f", v.x, v.y, v.z);
				//print(buf);
				forceEntity(gravityGunObjectID, v.x, v.y, v.z, 0, 0, 0, 1, false);

				gravityGunObjectID = -1;
				gravityGunTimer = 0;
			}
			gravityGunTimer--;
		}
	}

	void initiateGravityPulse(int obj) {
		gravityGunTimer = 0;//maybe not necessary
		gravityGunObjectID = obj;
	}

	void tick() {
		if (enabled) {
			if (gravityGun) {
				gravityGunWaiter();
				consolePrint((char*)"Gravity Gun");
			}
			else {
				consolePrint((char*)"Forge Gun");
			}

			int entLookingAt = getEntityAimedAt();
			if (entLookingAt != -1 && entLookingAt != blacklistItem && RCpressed == true) {
				if (objectSelected == -1)//I wanna make sure we don't already have an object selected... makes it so you can't just keep picking up objects you don't want
					setObject(entLookingAt);
			}

			if (objectSelected != -1) {
				char buf[50];
				sprintf(buf, "Object ID: 0x%X", objectSelected);
				consolePrint(buf);
				char buf1[50];
				sprintf(buf1, "Model Hash: 0x%X", getEntityModel(objectSelected));
				consolePrint(buf1);

				//set position to where I am looking at
				if (PLAYER::IS_PLAYER_FREE_AIMING(0) == true) {
					Vector3 tmp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false/*idk*/);
					Vector3 c = GET_COORDS_INFRONT(10);
					float x = tmp.x + c.x;
					float y = tmp.y + c.y;
					float z = tmp.z + c.z;
					PED::SET_PED_TO_RAGDOLL(objectSelected, 1000, 1000, 0, 0, 0, 0);
					//maybe make it so when you press middle mouse button it sets coordinates
					if (gravityGun) {
						pushEntityToLocation(objectSelected, { x,y,z }, 2.0f);
					}
					else {
						ENTITY::SET_ENTITY_COORDS(objectSelected, x, y, z, true, true, true, false);
					}
					PED::SET_PED_TO_RAGDOLL(objectSelected, 1000, 1000, 0, 0, 0, 0);
				}
			}
		}
	}

};


void runAtWeaponShot(std::function<void(Vector3)> callback) {
	//GET_PED_LAST_WEAPON_IMPACT_COORD ped vec3*
	int ped = PLAYER::PLAYER_PED_ID();
	if (PED::IS_PED_SHOOTING(ped))
	{
		//float p[6];
		Vector3 loc = { 0,0,0 };
		if (WEAPON::GET_PED_LAST_WEAPON_IMPACT_COORD(ped, &loc))
		{
			//Vector3 loc = { p[0],p[2],p[4] };
			callback(loc);
		}
	}

}


bool shootiportBool = false;
void shootiportTick() {
	if (shootiportBool) {
		//consolePrint((char*)"Shootiport");
		runAtWeaponShot([](auto loc) {
			int ped = PLAYER::PLAYER_PED_ID();
			//static char buf[100];
			//sprintf(buf, "%f %f %f", loc.x, loc.y, loc.z);
			//print(buf,500);
			ENTITY::SET_ENTITY_COORDS(ped, loc.x, loc.y, loc.z, true, true, true, false);
			});
	}
}

bool explosiveAmmoBool = false;
void explosiveAmmoTick() {
	if (explosiveAmmoBool) {
		//consolePrint((char*)"Explosive Ammo");
		runAtWeaponShot([](auto loc) {
			int ped = PLAYER::PLAYER_PED_ID();
			FIRE::ADD_OWNED_EXPLOSION(ped, loc.x, loc.y, loc.z, 0/*type*/, 100.0f/*damage*/, true/*audible*/, false/*invisible*/, 1.0f/*cam shake*/);
			});
	}
}

bool lightningAmmoBool = false;
void lightningAmmoTick() {
	if (lightningAmmoBool) {
		//consolePrint((char*)"Lightning Ammo (Must be a rainy weather!!!) (Idea from Devil)");
		runAtWeaponShot([](auto loc) {
			GAMEPLAY::_FORCE_LIGHTNING_FLASH_AT_COORDS(loc.x, loc.y, loc.z);
			});
	}
}

bool forceFieldBool = false;
void forceFieldTick() {
	if (forceFieldBool) {
		//if (godmode) {
			//consolePrint((char*)"Force Field");
		forcePush(10);
		//int ped = PLAYER_PED_ID();
		//Vector3 loc = GET_ENTITY_COORDS( ped, false);
		//ADD_OWNED_EXPLOSION( ped, loc.x, loc.y, loc.z, 0/*type*/, 5.0f/*damage*/, false/*audible*/, true/*invisible*/, 0.0f/*cam shake*/);
	//}
	//else {
	//	consolePrint((char*)"Please enable godmode to use Force Field");
	//}
	}
}

bool infiniteAmmoBool = false;
void infiniteAmmoTick() {
	if (infiniteAmmoBool) {
		//consolePrint((char*)"Inf Ammo");
		int ped = PLAYER::PLAYER_PED_ID();
		Hash wep = 0;
		WEAPON::GET_CURRENT_PED_WEAPON(ped, &wep, 1, 0, 0);
		int maxammo = WEAPON::GET_MAX_AMMO_IN_CLIP(ped, wep, 1);
		WEAPON::SET_AMMO_IN_CLIP(ped, wep, maxammo);
	}
}

Vector3 GetCoordsInfrontOfCam(float distance)//should be the exact same as my noclip coordinate code
{
	Vector3 GameplayCamRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
	Vector3 GameplayCamCoord = CAM::GET_GAMEPLAY_CAM_COORD();

	float tan = cos(GameplayCamRot.x * PI / 180.0) * distance;
	float xPlane = sin(GameplayCamRot.z * PI / 180.0 * -1.0f) * tan + GameplayCamCoord.x;
	float yPlane = cos(GameplayCamRot.z * PI / 180.0 * -1.0f) * tan + GameplayCamCoord.y;
	float zPlane = sin(GameplayCamRot.x * PI / 180.0) * distance + GameplayCamCoord.z;

	Vector3 ret = { xPlane, yPlane, zPlane };

	return ret;
}
bool RapidfireActive = false;
void RapidFireLoop()
{

	//if (RapidfireActive)
	//	consolePrint((char*)"Rapid Fire");
	static int Timeout = 0;
	if (RapidfireActive == true && LCpressed == true && GAMEPLAY::GET_GAME_TIMER() >= Timeout)
	{
		Vector3 FromCoord = PED::GET_PED_BONE_COORDS(PLAYER::PLAYER_PED_ID(), 57005, 0, 0, 0);//probably a bad bone index
		Vector3 ToCoord = GetCoordsInfrontOfCam(75);
		Hash WeaponID = 0;//w_rifle_boltaction01
		WEAPON::GET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), &WeaponID, 0, 0, 0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(FromCoord.x, FromCoord.y, FromCoord.z, ToCoord.x, ToCoord.y, ToCoord.z, 250, 0, /*GAMEPLAY::GET_HASH_KEY("WEAPON_RIFLE_SPRINGFIELD")*//*this weapon makes a better noise*/WeaponID, PLAYER::PLAYER_PED_ID(), 1, 0, 1, true/*idk*/);
		Timeout = GAMEPLAY::GET_GAME_TIMER() + 50;
	}
}





bool shootAnimals = false;
void shootAnimalsTick() {
	char* pedsArray[] = { (char*)"CS_BILLWILLIAMSON",(char*)"CS_MICAHBELL",(char*)"CS_UNCLE",(char*)"CS_SEAN",(char*)"CS_KIERAN",(char*)"A_M_M_WapWarriors_01",(char*)"A_M_O_WapTownfolk_01",(char*)"A_F_O_WapTownfolk_01",(char*)"U_M_M_WAPOFFICIAL_01",(char*)"A_M_M_WAPWARRIORS_01",(char*)"A_M_O_WAPTOWNFOLK_01",(char*)"S_M_M_UNITRAINENGINEER_01",(char*)"S_M_M_UNITRAINGUARDS_01",(char*)"A_M_M_MiddleTrainPassengers_01",(char*)"A_F_M_MiddleTrainPassengers_01",(char*)"A_M_M_LowerTrainPassengers_01",(char*)"A_F_M_LowerTrainPassengers_01",(char*)"A_M_M_UpperTrainPassengers_01",(char*)"A_F_M_UpperTrainPassengers_01",(char*)"A_M_M_NBXUPPERCLASS_01",(char*)"A_F_M_NBXUPPERCLASS_01",(char*)"G_M_M_UNIDUSTER_02",(char*)"G_M_M_UNIDUSTER_03",(char*)"CS_EDMUNDLOWRY",(char*)"G_M_M_UniBanditos_01",(char*)"CS_POISONWELLSHAMAN",(char*)"S_M_M_RACRAILGUARDS_01",(char*)"U_M_M_valbarber_01",(char*)"U_M_M_ValBartender_01",(char*)"U_M_M_ValDoctor_01",(char*)"U_M_M_ValGunsmith_01",(char*)"CS_VALSHERIFF",(char*)"G_M_M_UNIDUSTER_01",(char*)"S_M_M_SDTICKETSELLER_01",(char*)"S_M_M_MagicLantern_01",(char*)"CS_GrizzledJon",(char*)"A_M_M_VALCRIMINALS_01",(char*)"A_M_M_ASBMINER_02",(char*)"A_M_Y_ASBMINER_02",(char*)"A_M_M_ASBMINER_04",(char*)"A_M_Y_ASBMINER_04",(char*)"A_M_M_ASBMINER_03",(char*)"A_M_Y_ASBMINER_03",(char*)"A_M_M_JamesonGuard_01",(char*)"U_M_O_ASBSHERIFF_01",(char*)"A_M_M_RkrFancyDRIVERS_01",(char*)"A_M_M_RkrFancyTravellers_01",(char*)"A_F_M_RkrFancyTravellers_01",(char*)"S_M_M_CornwallGuard_01",(char*)"G_M_M_UniCornwallGoons_01",(char*)"U_M_M_CRDHOMESTEADTENANT_01",(char*)"U_M_M_CRDHOMESTEADTENANT_02",(char*)"U_M_O_PSHRANCHER_01",(char*)"U_M_O_CAJHOMESTEAD_01",(char*)"U_M_Y_CAJHOMESTEAD_01",(char*)"U_M_M_CAJHOMESTEAD_01",(char*)"CS_DOROETHEAWICKLOW",(char*)"S_M_M_AmbientSDPolice_01",(char*)"A_M_M_SDObeseMen_01",(char*)"A_F_M_SDObeseWomen_01",(char*)"U_M_M_NBXShadyDealer_01",(char*)"A_F_M_WapTownfolk_01",(char*)"A_M_M_SDLABORERS_02",(char*)"A_M_Y_NBXSTREETKIDS_01",(char*)"A_M_M_LowerSDTownfolk_02",(char*)"A_M_M_LowerSDTownfolk_01",(char*)"A_M_Y_NbxStreetKids_Slums_01",(char*)"A_M_Y_SDStreetKids_Slums_02",(char*)"A_M_M_SDDockWorkers_02",(char*)"A_M_M_SDDockForeman_01",(char*)"A_F_M_NbxSlums_01",(char*)"A_F_M_SDSlums_02",(char*)"A_M_M_NbxSlums_01",(char*)"A_M_M_SDSlums_02",(char*)"A_M_O_SDUpperClass_01",(char*)"A_F_M_NbxUpperClass_01",(char*)"A_F_O_SDUpperClass_01",(char*)"A_M_M_MiddleSDTownfolk_01",(char*)"A_M_M_MiddleSDTownfolk_02",(char*)"A_M_M_MiddleSDTownfolk_03",(char*)"A_F_M_MiddleSDTownfolk_01",(char*)"A_F_M_MiddleSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_01",(char*)"A_F_M_LowerSDTownfolk_02",(char*)"A_F_M_LowerSDTownfolk_03",(char*)"A_F_M_SDChinatown_01",(char*)"A_F_O_SDChinatown_01",(char*)"A_F_M_SDFancyWhore_01",(char*)"G_M_M_UNIBRONTEGOONS_01",(char*)"A_M_M_SDChinatown_01",(char*)"A_M_O_SDChinatown_01",(char*)"U_M_M_VALGUNSMITH_01",(char*)"U_M_M_VALGENSTOREOWNER_01",(char*)"U_M_M_ValButcher_01",(char*)"A_M_M_VALTOWNFOLK_01",(char*)"U_M_M_VALDOCTOR_01",(char*)"S_M_M_BANKCLERK_01",(char*)"U_M_M_ValHotelOwner_01",(char*)"U_M_M_VALBARTENDER_01",(char*)"U_M_M_VALBARBER_01",(char*)"U_M_O_ValBartender_01",(char*)"U_M_M_NBXBARTENDER_02",(char*)"A_M_M_BLWUPPERCLASS_01",(char*)"U_M_M_NBXBARTENDER_01",(char*)"A_M_M_MIDDLESDTOWNFOLK_01",(char*)"U_M_M_NBXGENERALSTOREOWNER_01",(char*)"U_M_M_NBXSHADYDEALER_01",(char*)"S_M_M_TAILOR_01",(char*)"U_M_M_NBXGUNSMITH_01",(char*)"CS_SDDOCTOR_01",(char*)"S_M_M_LIVERYWORKER_01",(char*)"S_M_M_MarketVendor_01",(char*)"U_M_M_SDPhotographer_01",(char*)"U_M_M_STRGENSTOREOWNER_01",(char*)"U_M_M_STRFREIGHTSTATIONOWNER_01",(char*)"U_M_M_ASBGUNSMITH_01",(char*)"U_M_M_SDTRAPPER_01",(char*)"S_M_M_TRAINSTATIONWORKER_01",(char*)"A_F_M_LAGTOWNFOLK_01",(char*)"S_M_M_SKPGUARD_01",(char*)"A_M_M_SKPPRISONER_01",(char*)"U_F_M_LagMother_01",(char*)"A_F_O_LagTownfolk_01",(char*)"A_M_O_LagTownfolk_01",(char*)"A_M_M_RHDFOREMAN_01",(char*)"U_M_M_ORPGUARD_01",(char*)"CS_ABE",(char*)"CS_DavidGeddes",(char*)"CS_ANGUSGEDDES",(char*)"CS_DUNCANGEDDES",(char*)"CS_MRSGEDDES",(char*)"G_M_M_UNIMOUNTAINMEN_01",(char*)"A_F_M_LagTownfolk_01",(char*)"CS_ThomasDown",(char*)"CS_ArchieDown",(char*)"CS_EDITHDOWN",(char*)"S_F_M_MaPWorker_01",(char*)"A_M_M_StrTownfolk_01",(char*)"A_M_M_STRLABORER_01",(char*)"A_M_M_BynRoughTravellers_01",(char*)"A_C_DOGPOODLE_01",(char*)"S_M_M_RHDCOWPOKE_01",(char*)"S_M_M_UniButchers_01",(char*)"S_M_Y_NewspaperBoy_01",(char*)"A_F_M_RhdUpperClass_01",(char*)"A_F_M_RhdTownfolk_02",(char*)"a_m_m_rhdtownfolk_01_laborer",(char*)"A_F_M_RHDTOWNFOLK_01",(char*)"U_M_M_RHDBARTENDER_01",(char*)"U_M_M_RHDGUNSMITH_01",(char*)"U_M_M_RHDGENSTOREOWNER_01",(char*)"U_M_M_RHDGENSTOREOWNER_02",(char*)"U_M_M_RHDSHERIFF_01",(char*)"U_M_M_RHDTRAINSTATIONWORKER_01",(char*)"A_F_M_RHDPROSTITUTE_01",(char*)"A_M_M_RHDObeseMen_01",(char*)"A_M_M_RhdUpperClass_01",(char*)"A_M_M_RkrRoughTravellers_01",(char*)"A_C_DOGCATAHOULACUR_01",(char*)"A_F_M_VHTTOWNFOLK_01",(char*)"A_M_M_VHTTHUG_01",(char*)"U_M_M_VHTSTATIONCLERK_01",(char*)"U_M_O_VHTEXOTICSHOPKEEPER_01",(char*)"U_F_M_VhTBartender_01",(char*)"A_M_M_HtlFancyDRIVERS_01",(char*)"A_M_M_HtlFancyTravellers_01",(char*)"A_F_M_HtlFancyTravellers_01",(char*)"S_M_M_BARBER_01",(char*)"S_M_M_UNIBUTCHERS_01",(char*)"S_M_M_BLWCOWPOKE_01",(char*)"A_M_M_BLWObeseMen_01",(char*)"A_M_O_BlWUpperClass_01",(char*)"A_F_M_BlWUpperClass_01",(char*)"A_F_O_BlWUpperClass_01",(char*)"A_M_M_BlWLaborer_02",(char*)"A_M_M_BlWLaborer_01",(char*)"A_M_M_BlWForeman_01",(char*)"A_F_M_BlWTownfolk_02",(char*)"A_F_M_BlWTownfolk_01",(char*)"S_M_M_AmbientBlWPolice_01",(char*)"S_M_M_FussarHenchman_01",(char*)"A_F_M_GuaTownfolk_01",(char*)"A_F_O_GuaTownfolk_01",(char*)"A_M_M_GuaTownfolk_01",(char*)"A_M_O_GuaTownfolk_01",(char*)"CS_NicholasTimmins",(char*)"A_M_M_STRFANCYTOURIST_01",(char*)"A_F_M_BiVFancyTravellers_01",(char*)"A_F_M_STRTOWNFOLK_01",(char*)"A_M_M_UNIGUNSLINGER_01",(char*)"U_M_M_StrGenStoreOwner_01",(char*)"U_M_M_STRWELCOMECENTER_01",(char*)"U_M_M_STRSHERRIFF_01",(char*)"U_M_M_StrFreightStationOwner_01",(char*)"G_M_M_UNICRIMINALS_01",(char*)"G_M_M_UNICRIMINALS_02",(char*)"A_M_M_BTCObeseMen_01",(char*)"A_F_M_BTCObeseWomen_01",(char*)"A_M_M_AsbTownfolk_01",(char*)"A_F_M_AsbTownfolk_01",(char*)"A_F_M_BTCHILLBILLY_01",(char*)"A_F_O_BTCHILLBILLY_01",(char*)"A_M_M_BTCHILLBILLY_01",(char*)"G_M_M_UniBraithwaites_01",(char*)"U_M_O_CmrCivilwarcommando_01",(char*)"S_M_M_Army_01",(char*)"U_M_M_BWMSTABLEHAND_01",(char*)"S_M_M_CGHWORKER_01",(char*)"G_M_M_UniGrays_01",(char*)"G_M_O_UniExConfeds_01",(char*)"G_M_Y_UniExConfeds_01",(char*)"G_M_M_UNIINBRED_01",(char*)"CS_AberdeenSister",(char*)"A_M_M_GriRoughTravellers_01",(char*)"A_F_M_ValTownfolk_01",(char*)"U_F_O_WTCTOWNFOLK_01",(char*)"U_M_M_BLWTRAINSTATIONWORKER_01",(char*)"CS_BLWPHOTOGRAPHER",(char*)"A_M_M_BlWTownfolk_01",(char*)"U_M_O_BLWPOLICECHIEF_01",(char*)"S_M_M_Tailor_01",(char*)"S_M_M_BankClerk_01",(char*)"U_M_O_BLWBARTENDER_01",(char*)"U_M_O_BLWGENERALSTOREOWNER_01",(char*)"S_M_M_TumDeputies_01",(char*)"CS_SHERIFFFREEMAN",(char*)"A_M_M_NEAROUGHTRAVELLERS_01",(char*)"A_M_M_TumTownfolk_01",(char*)"A_F_M_TumTownfolk_01",(char*)"A_M_M_TumTownfolk_02",(char*)"A_F_M_TumTownfolk_02",(char*)"U_M_M_TUMBARTENDER_01",(char*)"U_M_M_TUMBUTCHER_01",(char*)"U_M_M_TUMGUNSMITH_01",(char*)"U_F_M_TUMGENERALSTOREOWNER_01",(char*)"A_M_M_ARMTOWNFOLK_01",(char*)"A_M_M_RANCHERTRAVELERS_WARM_01",(char*)"S_M_M_AmbientLawRural_01",(char*)"A_M_M_ArmCholeraCorpse_01",(char*)"A_F_M_ArmCholeraCorpse_01",(char*)"U_M_M_ARMGENERALSTOREOWNER_01",(char*)"U_M_O_ARMBARTENDER_01",(char*)"U_M_M_ARMTRAINSTATIONWORKER_01",(char*)"U_M_M_ARMUNDERTAKER_01",(char*)"RE_DEADBODIES_MALE",(char*)"RE_DEADBODIES_MALES_01",(char*)"A_F_M_ArmTownfolk_01",(char*)"A_F_M_ArmTownfolk_02",(char*)"A_M_M_ArmTownfolk_01",(char*)"A_M_M_ArmTownfolk_02",(char*)"A_F_M_FAMILYTRAVELERS_WARM_01",(char*)"CS_WROBEL",(char*)"U_M_M_CZPHOMESTEADFATHER_01",(char*)"U_F_Y_CZPHOMESTEADDAUGHTER_01",(char*)"U_M_Y_CZPHOMESTEADSON_01",(char*)"U_M_Y_CZPHOMESTEADSON_02",(char*)"U_M_Y_CZPHOMESTEADSON_03",(char*)"U_M_M_LRSHOMESTEADTENANT_01",(char*)"U_F_M_RKSHOMESTEADTENANT_01",(char*)"U_M_M_UNIEXCONFEDSBOUNTY_01",(char*)"U_M_M_WALGENERALSTOREOWNER_01",(char*)"S_M_M_TrainStationWorker_01",(char*)"p_m_zero",(char*)"P_M_THREE",(char*)"A_C_HORSE_GANG_KIERAN",(char*)"CS_Dutch",(char*)"CS_JackMarston",(char*)"CS_JACKMARSTON_TEEN",(char*)"CS_EagleFlies",(char*)"CS_Cleet",(char*)"CS_Joe",(char*)"CS_CREOLECAPTAIN",(char*)"CS_JosiahTrelawny",(char*)"CS_RevSwanson",(char*)"CS_LeoStrauss",(char*)"CS_MrPearson",(char*)"CS_Lenny",(char*)"CS_Sean",(char*)"CS_HoseaMatthews",(char*)"CS_JavierEscuella",(char*)"CS_CharlesSmith",(char*)"CS_Uncle",(char*)"CS_BillWilliamson",(char*)"CS_MicahBell",(char*)"CS_JohnMarston",(char*)"CS_AbigailRoberts",(char*)"CS_MaryBeth",(char*)"CS_MollyOshea",(char*)"CS_SusanGrimshaw",(char*)"CS_Karen",(char*)"CS_Tilly",(char*)"CS_MrsAdler",(char*)"A_M_M_HtlRoughTravellers_01",(char*)"A_M_M_SclRoughTravellers_01",(char*)"U_M_M_LnSWorker_01",(char*)"U_M_M_LnSWorker_02",(char*)"U_M_M_LnSWorker_03",(char*)"U_M_M_LnSWorker_04",(char*)"U_M_M_RKFRANCHER_01",(char*)"A_M_M_Rancher_01",(char*)"U_M_M_MFRRANCHER_01",(char*)"EA_LCMP_DOGS",(char*)"EA_LCMP_FOREMAN",(char*)"U_M_M_BiVForeman_01",(char*)"EA_HMSTD_FOREMAN",(char*)"U_M_M_HtlForeman_01",(char*)"EA_HMSTD_WORKER_1",(char*)"EA_HMSTD_WORKER_2",(char*)"EA_HMSTD_MALE",(char*)"U_M_M_HTLHusband_01",(char*)"EA_HMSTD_FEMALE",(char*)"U_F_M_HTLWife_01",(char*)"EA_RCAMP_FOREMAN",(char*)"U_M_M_RaCForeman_01",(char*)"A_M_M_NbxUpperClass_01",(char*)"A_M_M_BlWUpperClass_01",(char*)"CS_Kieran",(char*)"A_M_M_RHDTOWNFOLK_02",(char*)"S_M_M_CKTWORKER_01",(char*)"S_M_M_STRLUMBERJACK_01",(char*)"S_M_M_RaCRailWorker_01",(char*)"S_M_Y_RACRAILWORKER_01",(char*)"A_M_M_BIVWORKER_01",(char*)"A_M_M_BLWTOWNFOLK_01",(char*)"S_M_M_MAPWORKER_01",(char*)"U_M_Y_HTLWORKER_01",(char*)"U_M_Y_HTLWORKER_02",(char*)"A_M_M_ASBMINER_01",(char*)"A_M_Y_ASBMINER_01",(char*)"CS_AberdeenPigFarmer",(char*)"U_M_M_DORHOMESTEADHUSBAND_01",(char*)"A_M_M_EmRFarmHand_01",(char*)"U_M_M_EMRFATHER_01",(char*)"U_M_Y_EMRSON_01",(char*)"A_M_M_ValTownfolk_01",(char*)"A_M_M_ValFarmer_01",(char*)"A_M_M_ValTownfolk_02",(char*)"S_M_M_ValCowpoke_01",(char*)"A_M_M_ValLaborer_01",(char*)"S_M_M_LiveryWorker_01",(char*)"A_M_M_NBXDOCKWORKERS_01",(char*)"A_M_M_NBXLABORERS_01",(char*)"A_M_M_LagTownfolk_01",(char*)"A_M_M_BtcHillbilly_01",(char*)"A_M_O_BTCHILLBILLY_01",(char*)"A_M_M_VhtTownfolk_01",(char*)"CS_TomDickens",(char*)"A_M_M_STRTOWNFOLK_01",(char*)"A_M_M_RHDTOWNFOLK_01",(char*)"A_M_M_RhdForeman_01",(char*)"S_M_Y_Army_01",(char*)"A_F_M_NbxWhore_01",(char*)"A_F_M_VHTPROSTITUTE_01",(char*)"A_F_M_VALPROSTITUTE_01",(char*)"A_C_Cat_01",(char*)"A_C_CedarWaxwing_01",(char*)"A_C_Chipmunk_01",(char*)"A_C_CRAB_01",(char*)"A_C_Cormorant_01",(char*)"A_C_CarolinaParakeet_01",(char*)"A_C_DogCatahoulaCur_01",(char*)"A_C_DOGHOBO_01",(char*)"A_C_DOGHOUND_01",(char*)"A_C_DogHusky_01",(char*)"A_C_DOGRUFUS_01",(char*)"A_C_DOGBLUETICKCOONHOUND_01",(char*)"A_C_Donkey_01",(char*)"A_C_Elk_01",(char*)"A_C_FISHBULLHEADCAT_01_SM",(char*)"A_C_FISHNORTHERNPIKE_01_LG",(char*)"A_C_FISHRAINBOWTROUT_01_MS",(char*)"A_C_FISHSALMONSOCKEYE_01_MS",(char*)"A_C_GILAMONSTER_01",(char*)"A_C_Goat_01",(char*)"A_C_HORSE_AMERICANPAINT_GREYOVERO",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_PALOMINODAPPLE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_SILVERTAILBUCKSKIN",(char*)"A_C_HORSE_ANDALUSIAN_DARKBAY",(char*)"A_C_HORSE_ANDALUSIAN_ROSEGRAY",(char*)"A_C_HORSE_APPALOOSA_BROWNLEOPARD",(char*)"A_C_HORSE_APPALOOSA_LEOPARD",(char*)"A_C_HORSE_ARABIAN_BLACK",(char*)"A_C_HORSE_ARABIAN_ROSEGREYBAY",(char*)"A_C_HORSE_ARDENNES_BAYROAN",(char*)"A_C_HORSE_ARDENNES_STRAWBERRYROAN",(char*)"A_C_HORSE_BELGIAN_BLONDCHESTNUT",(char*)"A_C_HORSE_BELGIAN_MEALYCHESTNUT",(char*)"A_C_HORSE_DUTCHWARMBLOOD_CHOCOLATEROAN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SEALBROWN",(char*)"A_C_HORSE_DUTCHWARMBLOOD_SOOTYBUCKSKIN",(char*)"A_C_HORSE_HUNGARIANHALFBRED_DARKDAPPLEGREY",(char*)"A_C_HORSE_HUNGARIANHALFBRED_PIEBALDTOBIANO",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_AMBERCHAMPAGNE",(char*)"A_C_HORSE_MISSOURIFOXTROTTER_SILVERDAPPLEPINTO",(char*)"A_C_HORSE_NOKOTA_REVERSEDAPPLEROAN",(char*)"A_C_HORSE_SHIRE_DARKBAY",(char*)"A_C_HORSE_SHIRE_LIGHTGREY",(char*)"A_C_HORSE_SUFFOLKPUNCH_SORREL",(char*)"A_C_HORSE_SUFFOLKPUNCH_REDCHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_FLAXENROAN",(char*)"A_C_HORSE_THOROUGHBRED_BRINDLE",(char*)"A_C_HORSE_TURKOMAN_DARKBAY",(char*)"A_C_HORSE_TURKOMAN_GOLD",(char*)"A_C_HORSE_TURKOMAN_SILVER",(char*)"A_C_HORSE_GANG_BILL",(char*)"A_C_HORSE_GANG_CHARLES",(char*)"A_C_HORSE_GANG_DUTCH",(char*)"A_C_HORSE_GANG_HOSEA",(char*)"A_C_HORSE_GANG_JAVIER",(char*)"A_C_HORSE_GANG_JOHN",(char*)"A_C_HORSE_GANG_KAREN",(char*)"A_C_HORSE_GANG_LENNY",(char*)"A_C_HORSE_GANG_MICAH",(char*)"A_C_HORSE_GANG_SADIE",(char*)"A_C_HORSE_GANG_SEAN",(char*)"A_C_HORSE_GANG_TRELAWNEY",(char*)"A_C_HORSE_GANG_UNCLE",(char*)"A_C_HORSE_GANG_SADIE_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_CHARLES_ENDLESSSUMMER",(char*)"A_C_HORSE_GANG_UNCLE_ENDLESSSUMMER",(char*)"A_C_HORSE_AMERICANPAINT_OVERO",(char*)"A_C_HORSE_AMERICANPAINT_TOBIANO",(char*)"A_C_HORSE_AMERICANPAINT_SPLASHEDWHITE",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BLACK",(char*)"A_C_HORSE_AMERICANSTANDARDBRED_BUCKSKIN",(char*)"A_C_HORSE_APPALOOSA_BLANKET",(char*)"A_C_HORSE_APPALOOSA_LEOPARDBLANKET",(char*)"A_C_HORSE_ARABIAN_WHITE",(char*)"A_C_HORSE_HUNGARIANHALFBRED_FLAXENCHESTNUT",(char*)"A_C_HORSE_MUSTANG_GRULLODUN",(char*)"A_C_HORSE_MUSTANG_WILDBAY",(char*)"A_C_HORSE_MUSTANG_TIGERSTRIPEDBAY",(char*)"A_C_HORSE_NOKOTA_BLUEROAN",(char*)"A_C_HORSE_NOKOTA_WHITEROAN",(char*)"A_C_HORSE_THOROUGHBRED_BLOODBAY",(char*)"A_C_HORSE_THOROUGHBRED_DAPPLEGREY",(char*)"A_C_HorseMule_01",(char*)"A_C_Ox_01",(char*)"A_C_REDFOOTEDBOOBY_01",(char*)"A_C_SHARKHAMMERHEAD_01",(char*)"A_C_SharkTiger",(char*)"A_C_SHEEP_01",(char*)"A_C_SNAKEBLACKTAILRATTLE_01",(char*)"A_C_snakeferdelance_01",(char*)"A_C_ROSEATESPOONBILL_01",(char*)"A_C_TURKEY_01",(char*)"A_C_WOODPECKER_02",(char*)"A_C_PIG_01",(char*)"A_C_SNAKEWATER_01",(char*)"A_C_SPARROW_01",(char*)"A_C_Egret_01",(char*)"A_C_PARROT_01",(char*)"A_C_PRAIRIECHICKEN_01",(char*)"A_C_BAT_01",(char*)"A_C_Heron_01",(char*)"A_C_Oriole_01",(char*)"A_C_BLUEJAY_01",(char*)"A_C_CARDINAL_01",(char*)"A_C_Duck_01",(char*)"A_C_PIGEON",(char*)"A_C_ROBIN_01",(char*)"A_C_SONGBIRD_01",(char*)"A_C_Woodpecker_01",(char*)"A_C_IGUANA_01",(char*)"A_C_ROOSTER_01",(char*)"A_C_SNAKEFERDELANCE_01",(char*)"A_C_Armadillo_01",(char*)"A_C_IGUANADESERT_01",(char*)"A_C_Quail_01",(char*)"A_C_SNAKEREDBOA_01",(char*)"A_C_Chicken_01",(char*)"A_C_FROGBULL_01",(char*)"A_C_SQUIRREL_01",(char*)"A_C_Toad_01",(char*)"A_C_TURTLESNAPPING_01",(char*)"A_C_Bull_01",(char*)"A_C_Cow",(char*)"A_C_MUSKRAT_01",(char*)"A_C_RAT_01",(char*)"A_C_SNAKE_01",(char*)"A_C_Beaver_01",(char*)"A_C_PHEASANT_01",(char*)"A_C_Rabbit_01",(char*)"A_C_TurkeyWild_01",(char*)"A_C_BigHornRam_01",(char*)"A_C_Buck_01",(char*)"A_C_Buffalo_01",(char*)"A_C_DEER_01",(char*)"A_C_ELK_01",(char*)"A_C_MOOSE_01",(char*)"A_C_ProngHorn_01",(char*)"A_C_HORSE_MORGAN_BAY",(char*)"A_C_HORSE_MORGAN_BAYROAN",(char*)"A_C_HORSE_MORGAN_FLAXENCHESTNUT",(char*)"A_C_HORSE_MORGAN_PALOMINO",(char*)"A_C_HORSE_KENTUCKYSADDLE_BLACK",(char*)"A_C_HORSE_KENTUCKYSADDLE_CHESTNUTPINTO",(char*)"A_C_HORSE_KENTUCKYSADDLE_GREY",(char*)"A_C_HORSE_KENTUCKYSADDLE_SILVERBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_BLACKRABICANO",(char*)"A_C_HORSE_TENNESSEEWALKER_CHESTNUT",(char*)"A_C_HORSE_TENNESSEEWALKER_DAPPLEBAY",(char*)"A_C_HORSE_TENNESSEEWALKER_REDROAN",(char*)"A_C_BEAR_01",(char*)"A_C_DOGAMERICANFOXHOUND_01",(char*)"A_C_DOGCOLLIE_01",(char*)"A_C_DogAustralianSheperd_01",(char*)"A_C_DogChesBayRetriever_01",(char*)"A_C_DogLab_01",(char*)"A_C_DogStreet_01",(char*)"A_C_DogBlueTickCoonHound_01",(char*)"A_C_DogHound_01",(char*)"A_C_TURTLESEA_01",(char*)"A_C_HAWK_01",(char*)"A_C_DUCK_01",(char*)"A_C_LOON_01",(char*)"A_C_CORMORANT_01",(char*)"A_C_GOOSECANADA_01",(char*)"A_C_HERON_01",(char*)"A_C_EGRET_01",(char*)"A_C_CraneWhooping_01",(char*)"A_C_PELICAN_01",(char*)"A_C_Badger_01",(char*)"A_C_BOAR_01",(char*)"A_C_JAVELINA_01",(char*)"A_C_OWL_01",(char*)"A_C_PANTHER_01",(char*)"A_C_POSSUM_01",(char*)"A_C_Raccoon_01",(char*)"A_C_SKUNK_01",(char*)"A_C_Alligator_01",(char*)"A_C_Alligator_02",(char*)"A_C_Alligator_03",(char*)"A_C_Bear_01",(char*)"A_C_BearBlack_01",(char*)"A_C_Boar_01",(char*)"A_C_CALIFORNIACONDOR_01",(char*)"A_C_Cougar_01",(char*)"A_C_Coyote_01",(char*)"A_C_Crow_01",(char*)"A_C_Eagle_01",(char*)"A_C_Fox_01",(char*)"A_C_Hawk_01",(char*)"A_C_Owl_01",(char*)"A_C_Panther_01",(char*)"A_C_RAVEN_01",(char*)"A_C_SEAGULL_01",(char*)"A_C_Vulture_01",(char*)"A_C_Wolf",(char*)"A_C_Wolf_Medium",(char*)"A_C_Wolf_Small",(char*)"A_C_FishMuskie_01_lg",(char*)"A_C_FISHLAKESTURGEON_01_LG",(char*)"A_C_FISHLONGNOSEGAR_01_LG",(char*)"A_C_FISHCHANNELCATFISH_01_LG",(char*)"A_C_FISHBLUEGIL_01_SM",(char*)"A_C_FISHPERCH_01_SM",(char*)"A_C_FISHCHAINPICKEREL_01_SM",(char*)"A_C_FISHROCKBASS_01_SM",(char*)"A_C_FISHREDFINPICKEREL_01_SM",(char*)"A_C_FISHBLUEGIL_01_MS",(char*)"A_C_FISHPERCH_01_MS",(char*)"A_C_FISHREDFINPICKEREL_01_MS",(char*)"A_C_FISHROCKBASS_01_MS",(char*)"A_C_FISHSMALLMOUTHBASS_01_MS",(char*)"A_C_FISHBULLHEADCAT_01_MS",(char*)"A_C_FISHLARGEMOUTHBASS_01_MS" };
	static bool chickenbob = 0;
	if (shootAnimals) {
		//consolePrint((char*)"Animal Gun");
		uint32_t myPed = PLAYER::PLAYER_PED_ID();
		bool Iamshooting = PED::IS_PED_SHOOTING(myPed)/* && LCpressed*/;
		if (Iamshooting) {
			if (!chickenbob) {
				chickenbob = true;
				static Vector3 loc = { 0,0,0 };
				loc = GET_COORDS_INFRONT(2).add(ENTITY::GET_ENTITY_COORDS(myPed, false, false/*idk*/));
				char* chosen = pedsArray[rand() % (sizeof(pedsArray) / sizeof(char*))];//random ped
				//print(chosen);
				SpawnPed(chosen, [](auto ped) {
					//print((char*)"Ped Spawned");
					runlater::runlater(1, [ped]() {
						//print((char*)"Done the loop!");
						Vector3 v = GET_COORDS_INFRONT(400);
						forceEntity(ped, v.x, v.y, v.z, 0, 0, 0, 1, false);
						});
					}, &loc);
			}
		}
		else {
			chickenbob = false;
		}
	}
}


namespace runlater {


	using deque = std::deque<std::tuple<int, std::function<void(void)>>>;
	deque queue;

	void loop() {
		for (int i = 0; i < queue.size(); ++i)
		{
			auto& it = queue[i];
			if (std::get<0>(it) <= 0)
			{
				auto callbackFunc = std::get<1>(it);
				callbackFunc();//std::get<2>(it)
				queue.erase(queue.begin() + i, queue.begin() + i + 1);
				--i;
			}
			else
				std::get<0>(it)--;
		}
	}
	void runlater(int interval, std::function<void(void)> callback) {
		queue.push_back({ interval, callback });
	}
}

float clamp360(float value) {
	while (value < 0)
		value += 360;
	return value;
}


namespace camShit {
	int cam = 0;
	Vector3 location = { 0,0,0 };
	Vector3 rotation = { 0,0,0 };//GET_GAMEPLAY_CAM_ROT(2);
	Vector3 velocityForFocus = { 0, 0, 0 };
	int entity = 0;
	bool doCamRot = true;
	bool doCamPos = true;
	bool doFocus = true;
	bool doLookat = true;
	void loop() {
		if (CAM::DOES_CAM_EXIST(cam)) {
			if (doCamRot)
				CAM::SET_CAM_ROT(cam, rotation.x, rotation.y, rotation.z, 2);
			if (doCamPos)
				CAM::SET_CAM_COORD(cam, location.x, location.y, location.z);
			if (doFocus)
				STREAMING::SET_FOCUS_POS_AND_VEL(location.x, location.y, location.z, velocityForFocus.x, velocityForFocus.y, velocityForFocus.z);
			if (doLookat) {
				CAM::POINT_CAM_AT_COORD(cam, rotation.x, rotation.y, rotation.z);
				Vector3 r = CAM::GET_CAM_ROT(cam, 2);
				r.x = clamp360(r.x);
				r.y = clamp360(r.y);
				r.z = clamp360(r.z);
				CAM::SET_CAM_ROT(cam, r.x, 0/*r.y*/, r.z, 2);
			}
		}
	}

	void attachToLocation(float x, float y, float z, bool setFocusOnArea = true) {
		doCamRot = true;//no need to set rot... we are doing lookat
		doCamPos = true;
		doFocus = true;
		doLookat = false;
		if (setFocusOnArea) {
			STREAMING::CLEAR_FOCUS();
			STREAMING::SET_FOCUS_POS_AND_VEL(x, y, z, 0, 0, 0);//should be right I think
		}
		entity = 0;
		if (!CAM::DOES_CAM_EXIST(cam))
			cam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);
		CAM::SET_CAM_COORD(cam, x, y, z);
		CAM::SET_CAM_ACTIVE(cam, 1);
		CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0, false/*idk*/);
		//Vector3 v = GET_GAMEPLAY_CAM_ROT(2);
		//SET_CAM_ROT( cam, rot.x, rot.y, rot.z, 2);
	}

	/*void attachToEntityHead(int ent, float offsetz = 0, bool setFocusOnEntity = true, bool justEntityAttach = false, bool doCamRot_ = true) {
		doCamRot = doCamRot_;
		if (setFocusOnEntity) {
			STREAMING::CLEAR_FOCUS();
			STREAMING::SET_FOCUS_ENTITY(ent);
		}
		entity = ent;
		if (!CAM::DOES_CAM_EXIST(cam))
			cam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);
		Vector3 coords = { 0.0,0.15,0.07 + offsetz };
		if (justEntityAttach == false)
			CAM::ATTACH_CAM_TO_PED_BONE(cam, ent, 31086, coords.x, coords.y, coords.z, 1);
		else
			CAM::ATTACH_CAM_TO_ENTITY(cam, ent, 0, 0, 0, true);
		//CAM::SET_CAM_COORD(cam,x,y,z);
		SET_CAM_NEAR_CLIP(cam, 0.225f);
		CAM::SET_CAM_ACTIVE(cam, 1);
		CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
		Vector3 v = CAM::GET_GAMEPLAY_CAM_ROT(2);
		float v1[] = { v.x,v.y,v.z };
		SET_CAM_ROT(cam, v1, 2);
	}*/



	void resetCam(bool clearFocus = true) {
		if (CAM::DOES_CAM_EXIST(cam)) {
			if (clearFocus)
				STREAMING::CLEAR_FOCUS();
			CAM::SET_CAM_ACTIVE(cam, 0);
			CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0, false/*idk*/);
			CAM::DESTROY_CAM(cam, 0);
			cam = 0;
		}
		cam = 0;
	}
	bool enabled() {

		return CAM::DOES_CAM_EXIST(cam);
	}
}

float getFPS() {
	static float fps = 0;
	fps = 1.0f / GAMEPLAY::GET_FRAME_TIME();
	return fps;
}

float secondsPerFrame() {
	return GAMEPLAY::GET_FRAME_TIME();
}

float gravityPerFrame() {
	return -9.81f * secondsPerFrame();
}



float randomFloatInRange(float smal, float large) {
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);//from 0 to 1
	float range = large - smal;
	r = r * range;
	r = r + smal;//get to small value
	return r;
}

float dx, dy, dz;
float GetDistance(Vector3 c1, Vector3 c2)
{
	float dx = c2.x - c1.x;
	float dy = c2.y - c1.y;
	float dz = c2.z - c1.z;

	return sqrt((float)((dx * dx) + (dy * dy) + (dz * dz)));
}

//bool testVal = false;

void drawBoxOutline(float x, float y, float radius, float thickness, int r, int g, int b, int a) {

	int rx, ry;//originally float but int does make more sense...
	GRAPHICS::GET_SCREEN_RESOLUTION(&rx, &ry);
	float aspectRatio = (float)ry / (float)rx;//want y to be the standard

	float xLeft = x - radius * aspectRatio;
	float xRight = x + radius * aspectRatio;
	float yTop = y - radius;
	float yBottom = y + radius;
	float length = radius * 2;
	DrawGameRect(xLeft + radius * aspectRatio, yTop + thickness / 2.0f, length * aspectRatio, thickness, r, g, b, a);//top bar
	DrawGameRect(xLeft + radius * aspectRatio, yBottom - thickness / 2.0f, length * aspectRatio, thickness, r, g, b, a);//bottom bar
	DrawGameRect(xLeft + thickness * aspectRatio / 2.0f, yTop + radius, thickness * aspectRatio, length, r, g, b, a);//left bar
	DrawGameRect(xRight - thickness * aspectRatio / 2.0f, yTop + radius, thickness * aspectRatio, length, r, g, b, a);//right bar
}

namespace predatorMissile {

	void drawRedboxes() {
		runOnAllNearbyPedsAndVehiclesToPlayer([](auto ped) {
			//GET_SCREEN_COORD_FROM_WORLD_COORD
			Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, false, false/*idk*/);
			float x, y;
			GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(pos.x, pos.y, pos.z, &x, &y);
			drawBoxOutline(x, y, 0.01f, 0.002f, 255, 0, 0, 255);
			});
	}

	bool missileActive;
	Vector3 location = { 0,0,0 };
	Vector3 rotation = { 0,0,0 };
	Vector3 velocity = { 0,0,0 };
	float speed = 1.0f;
	float initialZForGroundTest = 0;
	void loop() {
		if (missileActive) {

			DrawGameRect(0.5f, 0.5f, 1.0f, 1.0f, 8, 50, 5, 150);//green background
			//DrawGameRect(0.5f, 0.5f, 0.1f, 0.1f, 255, 255, 255, 100);
			drawBoxOutline(0.5f, 0.5f, 0.05f, 0.005f, 255, 255, 255, 255);
			drawRedboxes();
			/*
			//Old where it uses velocity and generates rotation from that, but it doesn't feel right to control
			//uint32_t myPed = GET_PLAYER_PED( 0);
			//Vector3 myPos = GET_ENTITY_COORDS( myPed, false);
			float groundZ = 0;




			//calculate new position first...
			//velocity.z = -1.0f;//+= gravityPerFrame();
			float fastSpeed = -5.0f;
			if (shiftKeyPressed)
				velocity.z = fastSpeed;

			if (velocity.z != fastSpeed) {
				consolePrint((char*)"~r~Press Shift For Speed Boost!");
			}

			if (Dpressed)
				velocity.x += 0.1f;
			if (Apressed)
				velocity.x -= 0.1f;
			if (Wpressed)
				velocity.y += 0.1f;
			if (Spressed)
				velocity.y -= 0.1f;

			location.x += velocity.x;
			location.y += velocity.y;
			location.z += velocity.z;// * secondsPerFrame()

			camShit::velocityForFocus.x = velocity.x;
			camShit::velocityForFocus.y = velocity.y;
			camShit::velocityForFocus.z = velocity.z;


			GET_GROUND_Z_FOR_3D_COORD( location.x, location.y, initialZForGroundTest, &groundZ, true);//true makes water counted as ground

			float heightAboveGround = (location.z) - groundZ;
			camShit::location.x = location.x;
			camShit::location.y = location.y;
			camShit::location.z = location.z;

			//camShit::rotation.x = -90;//pitch
			//camShit::rotation.y = 0;//roll
			//camShit::rotation.z = 0;//yaw


			float q = sqrt(velocity.x*velocity.x + velocity.y*velocity.y);
			float a = atan2(velocity.x, velocity.y) * 180.0f / PI;//yaw
			float o = atan2(velocity.z,q) * 180.0f / PI;//pitch
			a = clamp360(-a);
			o = clamp360(o);
			camShit::rotation.x = o;//pitch
			camShit::rotation.y = 0;//roll  -- nobody likes you
			camShit::rotation.z = a;//yaw
			*/




			float groundZ = 0;
			float maxRot = 45.0f;
			if (Wpressed) {
				rotation.x += 1.0f;
				if (rotation.x > maxRot)
					rotation.x = maxRot;
			}
			if (Spressed) {
				rotation.x -= 1.0f;
				if (rotation.x < -maxRot)
					rotation.x = -maxRot;
			}
			if (Apressed) {
				rotation.z += 1.0f;
				if (rotation.z > maxRot)
					rotation.z = maxRot;
			}
			if (Dpressed) {
				rotation.z -= 1.0f;
				if (rotation.z < -maxRot)
					rotation.z = -maxRot;
			}


			float fastSpeed = 10.0f;
			if (shiftKeyPressed) {
				speed = fastSpeed;
				rotation.x = 0;
				rotation.z = 0;
			}

			if (speed != fastSpeed) {
				DrawGameText(0.5f, 0.15, controllerMode?"Press R1 For Speed Boost":"Press SHIFT For Speed Boost", 255, 0, 0, 255, 1.0, 1.0, JUSTIFICATION_CENTER);
			}

			Vector3 velocityFinal = GET_COORDS_INFRONT_OF_ANGLE(rotation, speed);
			/*location.x += velocityFinal.x;
			location.y += velocityFinal.y;
			location.z += velocityFinal.z;*/
			//shift coordinate system
			//Vector3 previousLocation = {location.x,location.y,location.z};

			location.x += velocityFinal.x;
			location.y += velocityFinal.z;
			location.z += -velocityFinal.y;

			//Vector3 directionDif = location.sub(previousLocation);
			//Vector3 predictedLocation = location.add(directionDif);

			/*
			float q = sqrt(direction.x*direction.x + direction.y*direction.y);
			float a = atan2(direction.x, direction.y) * 180.0f / PI;//yaw
			float o = atan2(direction.z, q) * 180.0f / PI;//pitch
			a = clamp360(-a);//yaw
			o = clamp360(o);//pitch
			*/

			camShit::velocityForFocus.x = velocityFinal.x;
			camShit::velocityForFocus.y = velocityFinal.y;
			camShit::velocityForFocus.z = velocityFinal.z;

			camShit::location.x = location.x;
			camShit::location.y = location.y;
			camShit::location.z = location.z;






			//Vector3 coords = { predictedLocation.x - location.x,predictedLocation.y - location.y,predictedLocation.z - location.z };
			//float dist = GetDistance(predictedLocation, location);
			//float x = *(float*)read_process(ClActive() + BaseAngle,4);
			//float y = *(float*)read_process(ClActive() + BaseAngle + 0x4,4);
			//float z = *(float*)read_process(ClActive() + BaseAngle + 0x8,4);
			//vec3 base = {x,y,z};
			//vec3 cangle = vectoangles(angles);
			//Vector3 cangle = { 0,0,0 };
			//float x = atan(coords.x/coords.y) * 180.00 / PI;//this is 100% the x rotation no matter what the view angle is. this does not
			//float x = acos(coords.x / (dist != 0 ? dist : 0.000001f)) * 180.00 / PI;//probably the right one
			//float z = asin(coords.z / dist) * 180.00 / PI;//this is 100% the y rotation no matter what the view angle is. this works.
			//if (z < -90 && testVal)
			//	z = clamp360(z);
			//x = clamp360(x - 90);
			//camShit::rotation.x = z;//working! this works 100% for sure y axis         
			//camShit::rotation.y = 0;//cangle.y is your turning rotation. turns your screen sideways
			//camShit::rotation.z = clamp360(x);//working! cangle.z is x direction





			//camShit::rotation.x = clamp360(rotation.x + randomFloatInRange(-1, 1));//pitch
			//camShit::rotation.y = 0;//roll
			//camShit::rotation.z = clamp360(rotation.z + randomFloatInRange(-1, 1));//yaw

			camShit::rotation.x = clamp360(-90.0f + randomFloatInRange(-1, 1));//pitch
			camShit::rotation.y = 0;//roll
			camShit::rotation.z = clamp360(randomFloatInRange(-1, 1));//yaw

			//Vector3 lookat = location.add(directionDif);//predicted next look at location, should be close enough
			//camShit::rotation.x = lookat.x;
			//camShit::rotation.y = lookat.y;
			//camShit::rotation.z = lookat.z;

			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(location.x, location.y, initialZForGroundTest, &groundZ, true);//true makes water counted as ground

			float heightAboveGround = (location.z) - groundZ;



			//char buf[200];
			//snprintf(buf, sizeof(buf), "z: %f,   rx: %f, rz: %f,   vel: %f %f %f", groundZ, camShit::rotation.x, camShit::rotation.z, velocity.x, velocity.y, velocity.z);
			//consolePrint(buf);
			if (heightAboveGround <= 0) {
				//make explosion and delete
				FIRE::ADD_OWNED_EXPLOSION(PLAYER::PLAYER_PED_ID(), location.x, location.y, groundZ, 0/*type*/, /*200 originally*/500.0f/*damage*/, true/*audible*/, false/*invisible*/, 100.0f/*cam shake*/);
				camShit::resetCam();
				ENTITY::FREEZE_ENTITY_POSITION(PLAYER::PLAYER_PED_ID(), false);
				missileActive = false;
				simpleMenu.toggle();
			}
		}
	}
	void startMissile() {
		uint32_t myPed = PLAYER::GET_PLAYER_PED(0);
		Vector3 myPos = ENTITY::GET_ENTITY_COORDS(myPed, false, false/*idk*/);
		missileActive = true;
		location.x = myPos.x;
		location.y = myPos.y;
		location.z = myPos.z + 400;
		rotation.x = 0;//pitch -90 usually
		rotation.y = 0;//roll   -- dont touch
		rotation.z = 0;//yaw
		velocity.x = 0;
		velocity.y = 0;
		velocity.z = 0;
		initialZForGroundTest = location.z;
		speed = 2.0f;
		camShit::attachToLocation(location.x, location.y, location.z);
		ENTITY::FREEZE_ENTITY_POSITION(PLAYER::PLAYER_PED_ID(), true);
		simpleMenu.toggle();
	}
};























void setPedKillable(int micah) {
	AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(micah, true);
	auto micahGroup = PED::CREATE_GROUP(0);
	PED::SET_PED_AS_GROUP_LEADER(micah, micahGroup, true);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(
		255,
		PED::GET_PED_RELATIONSHIP_GROUP_HASH(micah),
		PED::GET_PED_RELATIONSHIP_GROUP_HASH(PLAYER::PLAYER_PED_ID())
	);

	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(
		255,
		PED::GET_PED_RELATIONSHIP_GROUP_HASH(PLAYER::PLAYER_PED_ID()),
		PED::GET_PED_RELATIONSHIP_GROUP_HASH(micah)
	);
}






bool circlePedsForceEnabled = false;
bool circlePedsForceOtherEnabled = false;
bool circlePedsBool = false;
//int circlePedsArray[100];
void pushEntityToLocation(int entity, Vector3 destination, float forceMultiplier) {
	Vector3 dif = destination.sub(ENTITY::GET_ENTITY_COORDS(entity, false, false));
	dif = dif.scale(forceMultiplier);
	forceEntity(entity, dif.x, dif.y, dif.z);
}
void toggleCirclePeds() {
	if (circlePedsForceEnabled || circlePedsForceOtherEnabled) {
		circlePedsBool = !circlePedsBool;
		if (circlePedsBool == true) {
			/*worldGetAllPeds(circlePedsArray, sizeof(circlePedsArray) / sizeof(int));
			for (int i = 0; i < sizeof(circlePedsArray) / sizeof(int); i++) {
				if (circlePedsArray[i] == PLAYER::PLAYER_PED_ID()) {
					circlePedsArray[i] = 0;//remove my player from it
				}
				else {
					//ENTITY::SET_ENTITY_COLLISION(circlePedsArray[i], false, false);
				}
			}*/
		}
		else {
			//for (int i = 0; i < sizeof(circlePedsArray) / sizeof(int); i++) {
				//ENTITY::SET_ENTITY_COLLISION(circlePedsArray[i], true, true);
			runOnAllPedsAndVehicles([](auto ent) {
				Vector3 dif = ENTITY::GET_ENTITY_COORDS(ent/*circlePedsArray[i]*/, true, false).sub(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false)).scale(20);
				forceEntity(ent/*circlePedsArray[i]*/, dif.x, dif.y, dif.z);
				});
			//}
		}
	}
}
void circlePeds() {
	if (circlePedsForceEnabled || circlePedsForceOtherEnabled) {
		static int rotation = 0;
		rotation++;
		if (rotation > 360)
			rotation = 0;
		if (circlePedsBool) {
			int ppid = PLAYER::PLAYER_PED_ID();
			Vector3 myCoords = ENTITY::GET_ENTITY_COORDS(ppid, true, false);
			//for (int i = 0; i < sizeof(circlePedsArray) / sizeof(int); i++) {
			runOnAllPedsAndVehicles([ppid, myCoords](auto ent) {
				//int ent = circlePedsArray[i];
				if (ent != ppid) {
					if (ENTITY::DOES_ENTITY_EXIST(ent)) {
						Vector3 position = GET_COORDS_INFRONT_OF_ANGLE({ 0,0,0 }, 7, rotation + (ent % 360)).add(myCoords).add({ 0,0,2 });
						//ENTITY::SET_ENTITY_COORDS(ent,position.x,position.y,position.z,true,true,true,false);
						pushEntityToLocation(ent, position);
					}
				}
				});
			//}
		}
	}
}


class AdvancedGravityGun {
public:
	std::vector<int> entities;
	int entityCap = 1000;
	AdvancedGravityGun() {}
	AdvancedGravityGun(int entityCap) : entityCap(entityCap) {}
	void addEntity(int ent) {
		if (!containsEntity(ent))
			if (entities.size() < entityCap)
				entities.push_back(ent);
	}
	bool containsEntity(int ent) {
		if (std::find(entities.begin(), entities.end(), ent) != entities.end()) {
			return true;
		}
		return false;
	}
	//the shoot pivots around sorucePosition for each entity unless asDirectionIntead is true then it goes in the same direction for all of them
	void shootAll(Vector3 sourcePosition, float power = 100.0f, bool asDirectionInstead = false) {
		for (auto i = entities.begin(); i != entities.end(); ++i) {
			int ent = *i;
			Vector3 entityCoords = ENTITY::GET_ENTITY_COORDS(ent, true, false);
			Vector3 dif = entityCoords.sub(sourcePosition);
			if (asDirectionInstead)
				dif = sourcePosition;//direction instead
			Vector3 direction = dif.normalize().scale(power);
			Vector3 pushTo = direction.add(entityCoords);
			pushEntityToLocation(ent, pushTo);
		}
		entities.clear();
	}
	void loop(Vector3 position, float power) {
		//Vector3 position = GET_COORDS_INFRONT(10).add(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false));
		for (auto i = entities.begin(); i != entities.end(); ++i) {
			int ent = *i;
			if (ENTITY::DOES_ENTITY_EXIST(ent)) {
				pushEntityToLocation(ent, position, power);
			}
			else {
				entities.erase(i);//remove entity if it no longer exists
				i--;
			}
		}
	}

};

enum RaycastFlags {
	INTERSECT_MAP_ANY = (1 << 0),
	INTERSECT_VEHICLES = (1 << 1),
	INTERSECT_PEDS = (1 << 2), //player pedestrians. does not include horses
	INTERSECT_PEDS2 = (1 << 3),//player pedestrians. does not include horses
	INTERSECT_PROPS = (1 << 4),
	UNK_5 = (1 << 5),
	INTERSECT_GLASS = (1 << 6),//windows ?
	UNK_7 = (1 << 7),
	INTERSECT_GREENERY = (1 << 8),//plants and trees
	INTERSECT_ROAD = (1 << 9),
	INTERSECT_ANIMALS = (1 << 10),
	UNK_11 = (1 << 11),
	UNK_12 = (1 << 12),
	UNK_13 = (1 << 13),
	UNK_14 = (1 << 14),
	UNK_15 = (1 << 15),
	UNK_16 = (1 << 16),
	UNK_17 = (1 << 17),
	UNK_18 = (1 << 18),
	UNK_19 = (1 << 19),
	UNK_20 = (1 << 20),
	UNK_21 = (1 << 21),
	UNK_22 = (1 << 22),
	UNK_23 = (1 << 23),
	UNK_24 = (1 << 24),
	UNK_25 = (1 << 25),
	UNK_26 = (1 << 26),
	UNK_27 = (1 << 27),
	UNK_28 = (1 << 28),
	UNK_29 = (1 << 29),
	UNK_30 = (1 << 30),
	UNK_31 = (1 << 31),
	INTERSECT_ALL = 0x7FFFFFFF
};
int createRaycast(Vector3 start, Vector3 end, unsigned int flag = 0x7FFFFFFF, float radius = 0.1f) {
	return SHAPETEST::START_SHAPE_TEST_CAPSULE(start.x, start.y, start.z, end.x, end.y, end.z, radius, flag, PLAYER::PLAYER_PED_ID(), 4);
	/*BOOL hit = false;
	Vector3 endCoords = { 0,0,0 };
	Vector3 surfaceNormal = { 0,0,0 };
	int entityHit = 0;
	int result = 1;
	while (result == 1) {
		result = SHAPETEST::GET_SHAPE_TEST_RESULT(handle, &hit, &endCoords, &surfaceNormal, &entityHit);//will return 2 when it is finished, whether it hit or not
		WAIT(1);
	}
	static char buf[200];
	snprintf(buf, sizeof(buf), "Shape test: ret: %i, hit: %i, hitat: %f %f %f, enthit: %i", result, hit, endCoords.x, endCoords.y, endCoords.z, entityHit);
	print(buf);*/
}

//returns true if it has the results, false if you need to wait another tick to check again
int entityHitTempValue = 0;
bool getRaycastResult(int handle, BOOL* hit, Vector3* endCoords, int* entityHit = &entityHitTempValue) {
	Vector3 surfaceNormal = { 0,0,0 };
	if (int result = SHAPETEST::GET_SHAPE_TEST_RESULT(handle, hit, endCoords, &surfaceNormal, entityHit) != 1) {
		//static char buf[200];
		//snprintf(buf, sizeof(buf), "Shape test: ret: %i, hit: %i, hitat: %f %f %f, enthit: %i", result, *hit, endCoords->x, endCoords->y, endCoords->z, *entityHit);
		//print(buf);
		return true;
	}
	return false;
}



int explosionID = 22;
int TEST_FLAG = 0;

namespace ufomod {
	bool enabled = false;
	int ufoID = -1;
	int weapon = 0;
	//int ammoType = 0;
	bool canExitUFO = false;//just here for the rendering controls
	float minHeight = 0.0f;
	Vector3 position = { 0,0,0 };
	Vector3 velocity = { 0,0,0 };
	AdvancedGravityGun gravityGun;
	AdvancedGravityGun gravityGunSingle(1);
	bool gravityGunEnabled = false;//show a red x when this is false
	//bool gravityGunModeToggle = false;//false for flying saucer pickup, true for gun pickup
	float maxHeightToLane = 20.0f;
	float slowModeHeight = 40.0f;
	//float skySpeed = 0.35f;
	enum Weapons {
		WEAPON_PASSIVE,
		WEAPON_MG,
		WEAPON_BOMBER,
		WEAPON_GRAVITYWELL,
		WEAPON_GRAVITYGUN,
		WEAPON_GRAVITYGUN_SINGLE,
		WEAPON_COUNT
	};
	void toggle(bool startFresh) {
		enabled = !enabled;
		int ppid = PLAYER::PLAYER_PED_ID();
		static bool dontSetOutAfter = false;
		promptUFO(false, false, false);//rid of any controls on toggle
		if (enabled) {

			if (ufoID != -1 && ENTITY::DOES_ENTITY_EXIST(ufoID) && startFresh == true) {
				ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ufoID, 0, 1);
				ENTITY::DELETE_ENTITY(&ufoID);
				//keep previous position, spawn new one at it
			}

			velocity = { 0,0,0 };
			ENTITY::SET_ENTITY_VISIBLE(ppid, false);
			setInvincible(true);
			if (startFresh == true) {
				//spawn 
				position = GET_COORDS_INFRONT(5).add(ENTITY::GET_ENTITY_COORDS(ppid, true, false)).add({ 0,0,400 });
				SpawnObject(0xB72F3DA7, &position, [](auto ent) {
					ufoID = ent;
					ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ent, true, true);

					//add sound
					//while it's nice to keep it on the ufo, the sound doesn't have distance scaling for whatever reason
					/*static bool swi = false;
					swi = !swi;
					if (swi) {
						AUDIO::PLAY_SOUND_FROM_ENTITY("Loop_A", ent, "Ufos_Sounds", 0, 0);
						print("A");
					}
					else {
						AUDIO::PLAY_SOUND_FROM_ENTITY("Loop_B", ent, "Ufos_Sounds", 0, 0);//this one is better
						print("B");
					}*/
					//AUDIO::PLAY_SOUND_FROM_ENTITY("Loop_B", ent, "Ufos_Sounds", 0, 0);


					//add blip
					int blip = RADAR::_0x23F74C2FDA6E7C61(-515518185, ent);
					//-1327110633 is saddle bag
					//RADAR::SET_BLIP_SPRITE(blip, -1739686743, 1);
					//RADAR::SET_BLIP_NAME_FROM_TEXT_FILE(blip, GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", "UFO"));
					RADAR::SET_BLIP_NAME_FROM_TEXT_FILE(blip, "BLIP_DEBUG");//interesitng one

					//one of these makes the icon pulse when it is first created
					//another makes it so you can't click on it in the map (which is fine until I figure out how to set the name)
					//idk what the other one does yet
					RADAR::_0x662D364ABF16DE2F(blip, 580546400);//I beleive this one is pulsing
					RADAR::_0x662D364ABF16DE2F(blip, -1878373110);//makes it not show up on menu list??? might actually have to do with the name
					RADAR::_0x662D364ABF16DE2F(blip, 231194138);
					});
			}
			weapon = Weapons::WEAPON_PASSIVE;
		}
		else {
			circlePedsForceOtherEnabled = false;
			ENTITY::SET_ENTITY_VISIBLE(ppid, true);
			//ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ufoID, 0, 1);
			//ENTITY::DELETE_ENTITY(&ufoID);
			setInvincible(false);
			if (dontSetOutAfter == false)
				CAM::_ANIMATE_GAMEPLAY_CAM_ZOOM(1.0f, 1.0f);
			//ufoID = -1;

			float groundZ = 0;
			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y + 2.0f, 1000.0f, &groundZ, true);
			if (dontSetOutAfter == false)
				ENTITY::SET_ENTITY_COORDS(ppid, position.x, position.y + 2.0f, groundZ, false, false, false, false);
			dontSetOutAfter = false;
		}

		//quick trick to make ufo spawn in front of me on ground and not actually be inside of it
		if (enabled == true && startFresh) {
			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y, 1000.0f, &position.z, true);//basically set ufo on ground
			position.z += 1.0f;//move up 1 so it's not like in the gorund

			dontSetOutAfter = true;
			toggle();
		}
	}

	void deleteUfo() {
		if (enabled)
			toggle();//get out of ufo

		promptUFO(false, false, false);//rid of any controls on toggle, this is here so the E button gets disabled properly iif you are sitting just outside of the ufo

		//delete ufo
		if (ufoID != -1 && ENTITY::DOES_ENTITY_EXIST(ufoID)) {
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ufoID, 0, 1);
			ENTITY::DELETE_ENTITY(&ufoID);
		}
		enabled = false;

	}

	float getHeightAboveGround() {
		float groundZ = 0;
		GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y, 1000.0f, &groundZ, true);
		return position.z - groundZ;
	}

	void doPhysics() {

		//add velocity to position
		position.x += velocity.x;//*(30.0f / getFPS());//I get roughly 30fps I beleive so scale it to that
		position.y += velocity.y;//*(30.0f / getFPS());
		position.z += velocity.z;//*(30.0f / getFPS());

		float groundZ = 0;
		GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y, 1000.0f, &groundZ, true);//shoudl always return ground coord
		groundZ += minHeight;
		if (position.z < groundZ)
			position.z = groundZ;

		bool slowMode = false;
		float heightAboveGround = getHeightAboveGround();
		if (heightAboveGround < slowModeHeight)
			slowMode = true;

		//slow down velocity with air friction
		float accelVal = slowMode ? 0.8f : 0.95f;//originally 0.8

		accelVal = accelVal;

		//we want it to be 0 when magnitude gets down to 1 so it scales to 0, should give us a realistic decelleration without too much math

		float mag = velocity.magnitude();
		if (mag < 0.3f)//needs to be smaller than the smallest speed or it will always clamp to 0
			accelVal = 0.0f;

		velocity = velocity.scale(accelVal);

	}

	void addVelocityInDirection(float direction, float speed = 1.0f) {
		Vector3 ro = CAM::GET_GAMEPLAY_CAM_ROT(2);//only want the z rotation
		ro.x = 0;
		ro.y = 0;
		velocity = velocity.add(GET_COORDS_INFRONT_OF_ANGLE(ro, speed, direction));
	}

	void addVelocityUpDown(float amount = 1.0f) {
		velocity.z += amount;
	}
	void controls() {

		bool slowMode = false;
		float heightAboveGround = getHeightAboveGround();
		if (heightAboveGround < slowModeHeight)
			slowMode = true;

		float speed = 0.42f;//0.7 was nice accel/decel but just a little too fast
		if (!slowMode)
			speed = 0.36f;//0.6f;
		if (keyPressed[VK_SHIFT] && !slowMode) {
			speed = 1.0f;
		}

		if (controllerMode == false) {
			if (Wpressed) {
				addVelocityInDirection(0, speed);
			}
			if (Spressed) {
				addVelocityInDirection(180, speed);
			}
			if (Dpressed) {
				addVelocityInDirection(-90, speed);
			}
			if (Apressed) {
				addVelocityInDirection(90, speed);
			}
		}
		else {
			float rotation = 0;
			float magnitude = 0;
			getAxisValues(&rotation, &magnitude);
			rotation = rotation - 90;//get it aligned with this weirdness
			//static char buf[100];
			//snprintf(buf, sizeof(buf), "rot: %f, mag: %f", rotation, magnitude);
			//consolePrint(buf);
			if (magnitude >= 0.2f)
				addVelocityInDirection(rotation, speed * magnitude);
		}

		if (CtrlPressed) {
			addVelocityUpDown(-speed);
		}
		if (keyPressed[VK_SPACE]) {
			addVelocityUpDown(speed);
		}

		if (/*keyPressed[0x46]*/LCpressed) {//F key originally
			if (weapon == Weapons::WEAPON_MG) {
				Vector3 FromCoord = GET_COORDS_INFRONT(5).add(position);//GetCoordsInfrontOfCam(5);  this gets offset from the camera position which doesn't give a good value since we move the cam so far back
				Vector3 ToCoord = GET_COORDS_INFRONT(75).add(position)/*.add({0,0,5})*/;
				//char buf[50];
				//snprintf(buf, sizeof(buf), "WEAPON_RIFLE_SPRINGFIELD^%i", ammoType);
				Hash WeaponID = GAMEPLAY::GET_HASH_KEY("WEAPON_RIFLE_SPRINGFIELD");
				GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(FromCoord.x, FromCoord.y, FromCoord.z, ToCoord.x, ToCoord.y, ToCoord.z, 250, 0, WeaponID, PLAYER::PLAYER_PED_ID(), 1, 0, 1, true/*idk*/);

			}
			if (weapon == Weapons::WEAPON_BOMBER) {
				float groundZ = 0;
				GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y, 1000.0f, &groundZ, true);
				FIRE::ADD_OWNED_EXPLOSION(PLAYER::PLAYER_PED_ID(), position.x, position.y, groundZ, 22/*type*/, 100.0f/*damage*/, true/*audible*/, false/*invisible*/, 0.1f/*cam shake*/);
			}
			if (weapon == Weapons::WEAPON_GRAVITYWELL) {
				if (LCJustPressed)
					toggleCirclePeds();
				//gravity
			}
			if (weapon == Weapons::WEAPON_GRAVITYGUN) {
				if (LCJustPressed) {

					//gravityGunEnabled = !gravityGunEnabled;
					//if (gravityGunEnabled == false) {//shoot them I just turned it off
					gravityGun.shootAll(GET_COORDS_INFRONT(15).add(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false)).sub(position), 400.0f, true);
					//}

					//quick patch...
					//gravityGunEnabled = true;//we don't actually need to worry about the toggle since it's grabbing from below and not our crisshair anymore
				}
			}
			if (weapon == Weapons::WEAPON_GRAVITYGUN_SINGLE) {
				if (gravityGunEnabled == true) {
					gravityGunSingle.shootAll(GET_COORDS_INFRONT(15).add(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false)).sub(position), 400.0f, true);
					gravityGunEnabled = false;//set so it doesn't pick up the entity we just threw
				}
			}
			if (weapon == Weapons::WEAPON_PASSIVE) {
				//landing mode
			}
		}
		if (RCpressed) {
			if (weapon == Weapons::WEAPON_GRAVITYGUN_SINGLE) {
				if (RCJustPressed) {
					gravityGunEnabled = !gravityGunEnabled;
					//toggle between aim mode and such from below mode
					//if (gravityGunEnabled == false)
					gravityGunSingle.entities.clear();//remove all when dropped the entities

					//gravityGunModeToggle = false;//keep as beam up mode... I don't really like the aim at mode
				}
			}
		}
		if (keyPressed[0x45]) {//E key
			if (weapon == Weapons::WEAPON_PASSIVE) {
				if (getHeightAboveGround() < maxHeightToLane) {
					keyPressed[0x45] = false;//temporarily disable until next press
					toggle();
					if (enabled) {
						//AUDIO::PLAY_SOUND_FRONTEND("Arrive", "Ufos_Sounds", 1, 0);
						//AUDIO::PLAY_SOUND_FRONTEND("Loop_B", "Ufos_Sounds", 1, 0);
						////actually this is never true here. The enter code is down below in loop
					}
					else {
						AUDIO::_STOP_SOUND_FRONTED("Loop_A", "Ufos_Sounds");//this made it stop when I got out... nice
					}
				}
			}
		}



		//gravity gun loop... not actually a control
		if (weapon == Weapons::WEAPON_GRAVITYGUN) {
			//if (gravityGunEnabled) {
			static int raycastHandle = -1;
			if (raycastHandle != -1) {
				BOOL hit = false;
				Vector3 endCoords = { 0,0,0 };
				int hitEnt = -1;
				if (getRaycastResult(raycastHandle, &hit, &endCoords, &hitEnt)) {
					if (hit) {
						//if (ENTITY::DOES_ENTITY_EXIST(hitEnt) && (ENTITY::IS_ENTITY_A_PED(hitEnt) || ENTITY::IS_ENTITY_A_VEHICLE(hitEnt)))
						gravityGun.addEntity(hitEnt);
					}
					raycastHandle = -1;
				}
			}
			if (raycastHandle == -1) {
				//This one is to pick up stuff in my look direction... but ufo's don't work like that ;)
				/*if (gravityGunModeToggle == true) {
					//pick up like a gun
					raycastHandle = createRaycast(GET_COORDS_INFRONT(5).add(position), GET_COORDS_INFRONT(500).add(position), RaycastFlags::INTERSECT_PEDS | RaycastFlags::INTERSECT_PEDS2 | RaycastFlags::INTERSECT_VEHICLES | RaycastFlags::INTERSECT_ANIMALS, 1.0f);//outputs debug info for raycast
				}
				else {*/
				//pick up like a flying saucer
				raycastHandle = createRaycast(position.sub({ 0,0,5 }), position.sub({ 0,0,1000 }), RaycastFlags::INTERSECT_PEDS | RaycastFlags::INTERSECT_PEDS2 | RaycastFlags::INTERSECT_VEHICLES | RaycastFlags::INTERSECT_ANIMALS, 4.0f);//outputs debug info for raycast
			//}
			}
			gravityGun.loop(GET_COORDS_INFRONT(15).add(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false)), 1.5f);//pull entities to this location
		//}
		}


		if (weapon == Weapons::WEAPON_GRAVITYGUN_SINGLE) {
			if (gravityGunEnabled) {
				static int raycastHandle = -1;
				if (raycastHandle != -1) {
					BOOL hit = false;
					Vector3 endCoords = { 0,0,0 };
					int hitEnt = -1;
					if (getRaycastResult(raycastHandle, &hit, &endCoords, &hitEnt)) {
						if (hit) {
							//if (ENTITY::DOES_ENTITY_EXIST(hitEnt) && (ENTITY::IS_ENTITY_A_PED(hitEnt) || ENTITY::IS_ENTITY_A_VEHICLE(hitEnt)))
							gravityGunSingle.addEntity(hitEnt);
						}
						raycastHandle = -1;
					}
				}
				if (raycastHandle == -1) {
					//pick up like a gun
					raycastHandle = createRaycast(GET_COORDS_INFRONT(5).add(position), GET_COORDS_INFRONT(500).add(position), RaycastFlags::INTERSECT_PEDS | RaycastFlags::INTERSECT_PEDS2 | RaycastFlags::INTERSECT_VEHICLES | RaycastFlags::INTERSECT_ANIMALS, 1.0f);//outputs debug info for raycast


				}
				gravityGunSingle.loop(GET_COORDS_INFRONT(15).add(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false)), 1.5f);//pull entities to this location
			}
		}


	}

	void toggleWeapon() {
		weapon++;
		if (weapon > Weapons::WEAPON_COUNT - 1)
			weapon = 0;
	}

	char* getWeaponText() {
		switch (weapon) {
		case Weapons::WEAPON_MG:
			return "MACHINE GUN";
		case Weapons::WEAPON_BOMBER:
			return "BOMBER";
		case Weapons::WEAPON_GRAVITYWELL:
			return "GRAVITY WELL";
		case Weapons::WEAPON_PASSIVE:
			return "PASSIVE";
		case Weapons::WEAPON_GRAVITYGUN:
			//if (gravityGunEnabled)
			//	return "GRAVITY GUN {ACTIVE}";
			//return "GRAVITY GUN {OFF}";
			return "BEAM UP";
		case Weapons::WEAPON_GRAVITYGUN_SINGLE:
			return "PROBING DEVICE";
		default:
			return "INVALID MODE";
		}
	}

	void drawUI() {
		//char *weaponText = "INVALID WEAPON";
		if (weapon == Weapons::WEAPON_MG) {
			//weaponText = "MACHINE GUN";

			static Vector3 pos = { 0,0,0 };
			static bool render = false;
			/*static int raycastHandle = -1;
			if (raycastHandle != -1) {
				BOOL hit = false;
				Vector3 endCoords = { 0,0,0 };
				if (getRaycastResult(raycastHandle, &hit, &endCoords)) {
					if (hit) {
						render = true;
						pos = endCoords;
					}
					else {
						render = false;//didn't hit anything so dont bother rendering crosshairs

						//could also set it to use the coords in front since they are the same now as if it hit something
						//render = true;
						//pos = GET_COORDS_INFRONT(75).add(position).add({ 0,0,5 });//set position to be this
					}
					raycastHandle = -1;
				}
			}
			if (raycastHandle == -1) {
				raycastHandle = createRaycast(GET_COORDS_INFRONT(5).add(position), GET_COORDS_INFRONT(75).add(position).add({ 0,0,5 }));//outputs debug info for raycast
			}*/

			pos = GET_COORDS_INFRONT(75).add(position);
			render = true;
			if (render) {
				float x, y;
				GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(pos.x, pos.y, pos.z, &x, &y);
				drawBoxOutline(x, y, 0.01f, 0.002f, 255, 0, 0, 255);
			}

		}
		if (weapon == Weapons::WEAPON_BOMBER) {
			//weaponText = "BOMBER";

			float groundZ = 0;
			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y, 1000.0f, &groundZ, true);
			float x, y;
			GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(position.x, position.y, groundZ, &x, &y);
			drawBoxOutline(x, y, 0.01f, 0.002f, 255, 0, 0, 255);

		}
		if (weapon == Weapons::WEAPON_GRAVITYWELL) {
			//weaponText = "GRAVITY WELL";
			circlePedsForceOtherEnabled = true;
		}
		else {
			circlePedsForceOtherEnabled = false;
		}
		if (weapon == Weapons::WEAPON_PASSIVE) {
			//weaponText = "PASSIVE LANDING MODE";
			canExitUFO = true;
			minHeight = 1.0f;
		}
		else {
			minHeight = 10.0f;
		}

		if (weapon == Weapons::WEAPON_GRAVITYGUN || weapon == Weapons::WEAPON_GRAVITYGUN_SINGLE) {//raycast based hitbox
			//weaponText = "GRAVITY GUN";
			static Vector3 pos = { 0,0,0 };
			static bool render = false;
			static int raycastHandle = -1;
			if (raycastHandle != -1) {
				BOOL hit = false;
				Vector3 endCoords = { 0,0,0 };
				if (getRaycastResult(raycastHandle, &hit, &endCoords)) {
					if (hit) {
						render = true;
						pos = endCoords;
					}
					else {
						//render = false;//didn't hit anything so dont bother rendering crosshairs
						render = true;
						pos = GET_COORDS_INFRONT(500).add(position);
					}
					raycastHandle = -1;
				}
			}
			if (raycastHandle == -1) {
				raycastHandle = createRaycast(GET_COORDS_INFRONT(5).add(position), GET_COORDS_INFRONT(500).add(position));//outputs debug info for raycast
			}

			if (render) {
				float x, y;
				GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(pos.x, pos.y, pos.z, &x, &y);
				if (weapon == Weapons::WEAPON_GRAVITYGUN_SINGLE && gravityGunEnabled == false) {
					drawBoxOutline(x, y, 0.01f, 0.002f, 120, 120, 120, 255);//draw grey box cuz pickup is disabled
				}
				else {
					drawBoxOutline(x, y, 0.01f, 0.002f, 255, 0, 0, 255);
				}
			}
		}

		//char text[200];
		//snprintf(text, sizeof(text), "Press Enter To Change Weapon\nPress F to Shoot\n%s", weaponText);
		//DrawGameText(0.5f, 0.0f, weaponText/*text*/, 128, 255, 128, 255, 0.35f, 0.35f, JUSTIFICATION_CENTER, 0);//don't render the controls text at top of screen

	}
	void loop() {
		static unsigned int counter = 0;
		counter += 3;
		bool canEnterUFO = false;
		canExitUFO = false;
		if (enabled) {
			doPhysics();
			drawUI();
			int ppid = PLAYER::PLAYER_PED_ID();
			ENTITY::SET_ENTITY_VISIBLE(ppid, false);
			setInvincible(true);
			ENTITY::SET_ENTITY_COORDS(ppid, position.x, position.y, position.z, false, false, false, false);
			CAM::_ANIMATE_GAMEPLAY_CAM_ZOOM(1.0f, 30.0f);
			controls();//controls need to go down here due to the E to exit feature

			Vector3 ro = CAM::GET_GAMEPLAY_CAM_ROT(2);
			ENTITY::SET_ENTITY_ROTATION(ppid, 0, 0, ro.z, 2, true);//fix head in first persn

			//char buf[100];
			//snprintf(buf, sizeof(buf), "%f %f %f   mag: %f", velocity.x, velocity.y, velocity.z, velocity.magnitude());
			//consolePrint(buf);
		}
		else {
			if (ufoID != -1 && ENTITY::DOES_ENTITY_EXIST(ufoID)) {
				//check if nearby and pressing E to get back in
				float groundZ = 0;
				GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y, 1000.0f, &groundZ, true);
				Vector3 enterCoords = { position.x, position.y , groundZ };
				float distance = enterCoords.sub(ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, false)).magnitude();
				if (distance < 10.0f) {
					canEnterUFO = true;
					//DrawGameText(0.5f, 0.0f, "Press E to get into UFO", 128, 255, 128, 255, 0.35f, 0.35f, JUSTIFICATION_CENTER, 0);
					if (keyPressed[0x45]) {//E key
						keyPressed[0x45] = false;
						toggle();
						if (enabled)
							AUDIO::PLAY_SOUND_FROM_ENTITY("Loop_A", ufoID, "Ufos_Sounds", 0, 0);
					}
				}
				/*
				no longer needed since I added the blip
				if (CAM::IS_SPHERE_VISIBLE(enterCoords.x, enterCoords.y, enterCoords.z, 1.0f) && distance < 50) {
					float x, y;
					GRAPHICS::GET_SCREEN_COORD_FROM_WORLD_COORD(position.x, position.y, enterCoords.z+2.0f, &x, &y);
					DrawGameText(x,y,"Enter UFO\nV",255,255,255,255,0.15,0.15,JUSTIFICATION_CENTER);
				}*/
			}
		}
		if (ufoID != -1 && ENTITY::DOES_ENTITY_EXIST(ufoID)) {
			ENTITY::SET_ENTITY_COORDS(ufoID, position.x, position.y, position.z, false, false, false, false);


			//set rotation of the ufo
			float spin = counter % 360;
			Vector3 ro = CAM::GET_GAMEPLAY_CAM_ROT(2);
			float difAngleFromVel = (atan2(velocity.y, velocity.x) * 180.0f / PI) - ro.z;
			//difAngleFromVel -= 90.0f;//cancels out with B
			ro.x = 0;
			ro.y = 0;
			ro.z -= spin;//sub off out spin
			//ro.z += 90;//calcels out with A
			ro.z += difAngleFromVel;
			float tilt = velocity.sub({ 0,0,velocity.z }/*only want horizontal velicity, up and down stays flat*/).magnitude() * 2;
			if (tilt > 30.0f)
				tilt = 30.0f;

			Vector3 rotcoords = GET_COORDS_INFRONT_OF_ANGLE(ro, tilt, 0);
			//if (TEST_FLAG == 1)
			//	Vector3 rotcoords = GET_COORDS_INFRONT_OF_ANGLE(ro, tilt, atan2(velocity.x, velocity.y) - ro.z + spin);

			ENTITY::SET_ENTITY_ROTATION(ufoID, rotcoords.x, rotcoords.y, spin, 2, true);
			promptUFO(enabled, canExitUFO, canEnterUFO);
		}
	}
};


int func_496(int iParam0)
{
	switch (iParam0)
	{
	case 0:
		return -875187602;//Q

	case 1:
		return 285921746;//E

	case 2:
		return -69749786;//Spacebar

	case 3:
		return 1138488863;//Enter

	//case 4:
	//	return 1138488863;//Enter

	case 5:
		return 814057702;//backspace

	//case 6:
	//	return 814057702;

	case 7:
		return -711536720;//F

	case 8:
		return -1932931774;//X
		return 0;
	}
}


bool isPromptValid(int prompt) {
	return UI::_0x347469FBDD1589A9(prompt);
}



void setPromptEnabled(int* prompt, bool enabled) {
	if (isPromptValid(*prompt)) {
		/*if (enabled == true && !UI::_0x0D00EDDFB58B7F28(*prompt)) {//enabled is true and the prompt is currently disabled
			UI::_0x8A0FB4D03A630D21(*prompt, true);
			UI::_0x71215ACCFDE075EE(*prompt, true);
		}
		else if (enabled == false && UI::_0x0D00EDDFB58B7F28(*prompt)) {//enabled is false (turn off) and prompt is currently enabled
			UI::_0x8A0FB4D03A630D21(*prompt, false);
			UI::_0x71215ACCFDE075EE(*prompt, false);
		}*/
		UI::_0x8A0FB4D03A630D21(*prompt, enabled);//enable
		//UI::_0x06565032897BA861(*prompt);//enabled this frame
		UI::_0x71215ACCFDE075EE(*prompt, enabled);//visible
		//deletePrompt(prompt);
	}
}

void deletePrompt(int* prompt) {
	setPromptEnabled(prompt,false);
	UI::_0x00EDE88D4D13CF59(*prompt);//kinda useless for us since we are keeping it always created for simplicity
	*prompt = -1;
}

//F4A5C4509BF923B1 is set prompt type
bool isPromptJustPressed(int prompt) {
	return UI::_0x2787CC611D3FACC5(prompt);
}
bool isPromptJustReleased(int prompt) {
	return UI::_0x635CC82FA297A827(prompt);
}

void createPrompt(int* prompt, int button, char* text, int secondButton = 0) {
	if (!isPromptValid(*prompt)) {
		*prompt = UI::_0x04F97DE45A519419();//_PROMPT_REGISTER_BEGIN()
		//UI::_0xF4A5C4509BF923B1(*prompt, 0);//unknown
		//UI::_0xB5352B7494A08258(globalPrompt, func_496(0));//_PROMPT_SET_CONTROL_ACTION   Q button
		//UI::_0xB5352B7494A08258(globalPrompt, func_496(1));//_PROMPT_SET_CONTROL_ACTION   E button
		//UI::_0xB5352B7494A08258(globalPrompt, func_496(3));//Enter key?
		//UI::_0xB5352B7494A08258(globalPrompt, func_496(7));//F key?
		//UI::_0xB5352B7494A08258(*prompt, func_496(2));//space
		//UI::_0xB5352B7494A08258(*prompt, func_496(4));//enter
		//UI::_0xB5352B7494A08258(*prompt, func_496(5));//backspace
		//UI::_0xB5352B7494A08258(*prompt, func_496(8));//X

		UI::_0xB5352B7494A08258(*prompt, button);
		if (secondButton != 0)
			UI::_0xB5352B7494A08258(*prompt, secondButton);

		//_UIPROMPT_SET_ALLOWED_ACTION
		/*UI::_0x565C1CE183CB0EAF(*prompt, button);
		if (secondButton != 0)
			UI::_0x565C1CE183CB0EAF(*prompt, secondButton);*/

		//UI::_0x5DD02A8318420DD7(globalPrompt, "TEST TEST LOL");//_PROMPT_SET_TEXT
		//UI::_0x560E76D5E2E1803F(*prompt, 13, 1);//_PROMPT_SET_ATTRIBUTE   this might be the pressing for q and e... idk
		//UI::_0x560E76D5E2E1803F(*prompt, 13, 1);
		//UI::_0x560E76D5E2E1803F(*prompt, 10, 1);
		//UI::_0x560E76D5E2E1803F(*prompt, 18, 1);

		//UI::_0x565C1CE183CB0EAF(*prompt, -2018477760);
		//UI::_0x565C1CE183CB0EAF(*prompt, 74488305);

		/*UI::_0x560E76D5E2E1803F(*prompt, 10, 1);//ui add attribute
		UI::_0x560E76D5E2E1803F(*prompt, 11, 1);

		UI::_0xF4A5C4509BF923B1(*prompt, 0);*/

		//UI::_0xCA24F528D0D16289(uParam0->f_1190, iParam6);//_PROMPT_SET_PRIORITY
		UI::_0x71215ACCFDE075EE(*prompt, false);//_PROMPT_SET_VISIBLE
		UI::_0x8A0FB4D03A630D21(*prompt, false);//_PROMPT_SET_ENABLED
		UI::_0xF7AA2696A22AD8B9(*prompt);
	}
	//update text if necessary
	//UI::_0xEA5CCF4EEB2F82D1(*prompt);
	UI::_0x5DD02A8318420DD7(*prompt, GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", text)/*"IB_SELECT"*/);//_PROMPT_SET_TEXT
}

static int promptF = -1;
static int promptLeftClick = -1;
static int promptE = -1;
static int promptSpace = -1;
static int promptShift = -1;
static int promptRightClick = -1;

bool controllerMode = false;
bool inputModeChanged = false;
void promptUFO(bool insideUFO, bool canLandUFO, bool canEnterUFO) {


	if (inputModeChanged) {
		//print("Deleted prompts");
		deletePrompt(&promptF);
		deletePrompt(&promptLeftClick);
		deletePrompt(&promptE);
		deletePrompt(&promptSpace);
		deletePrompt(&promptShift);
		deletePrompt(&promptRightClick);
	}

	if (insideUFO)
		canEnterUFO = false;
	
	bool runRightClick = (ufomod::weapon == ufomod::Weapons::WEAPON_GRAVITYGUN_SINGLE) && insideUFO;

	if (controllerMode) {
		if (insideUFO) {
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_L3);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_R2);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_X);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_R1);
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_Square);

		}
		if (canEnterUFO || canLandUFO)
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_Triangle);
		if (runRightClick)
			CONTROLS::SET_INPUT_EXCLUSIVE(INPUT_GROUP_CONTROLLER, Buttons::Button_L2);
	}

	


	createPrompt(&promptShift, controllerMode ? Buttons::Button_R1 : 0x8FFC75D6, "Boost");
	createPrompt(&promptLeftClick, controllerMode ? Buttons::Button_R2 : 0x07CE1E61, "Shoot Weapon");//Left Click

	char* ggmode = "Pickup Disabled";
	if (ufomod::gravityGunEnabled)
		ggmode = "Pickup Enabled";
	if (ufomod::gravityGunEnabled && ufomod::gravityGunSingle.entities.size() == 1)
		ggmode = "Drop Probee";


	createPrompt(&promptRightClick, controllerMode ? Buttons::Button_L2 : 0xF84FA74F, ggmode);//right click

	static char WeaponText[100];
	snprintf(WeaponText, sizeof(WeaponText), "Mode [%s]", ufomod::getWeaponText());

	createPrompt(&promptF, controllerMode ? Buttons::Button_Square : func_496(7) /*0xCEFD9220*//*func_496(3)*/, WeaponText);//Changed to F    //enter  1138488863 0x43DBF61F
	createPrompt(&promptSpace, controllerMode ? Buttons::Button_X : 0xD9D0E1C0/*func_496(2)*/, "Ascend", controllerMode ? Button_L3 : 0x339F3730/*descend button*/);//space
	//createPrompt(&promptCtrl, 0x339F3730, "Descend");//ctrl

	char* txt = "Exit UFO";
	if (canEnterUFO)
		txt = "Enter UFO";

	createPrompt(&promptE, controllerMode ? Buttons::Button_Triangle : 285921746/*0xCEFD9220 this value is also E*//*0xA4F1006B*/, txt);//e

	//this code doesn't work cuz u can't register 2 apparently
	//createPrompt(&promptExitUfo, 285921746/*0xCEFD9220 this value is also E*//*0xA4F1006B*/, "Exit UFO");//e
	//createPrompt(&promptEnterUfo, 285921746/*0xCEFD9220*//*0xA4F1006B*/, "Enter UFO");//e
	//promptE = promptExitUfo;
	//if (canEnterUFO)
	//	promptE = promptEnterUfo;

	//enable them
	//setPromptEnabled(&promptCtrl, insideUFO);//this shows up at the top (ctrl)
	setPromptEnabled(&promptF, insideUFO);//this shows up at bottom (enter)
	if (canLandUFO || canEnterUFO) {
		setPromptEnabled(&promptLeftClick, false); //this shows up third to bottom (F)

		setPromptEnabled(&promptE, true);

		if (!(ufomod::getHeightAboveGround() < ufomod::maxHeightToLane)) {
			UI::_0x8A0FB4D03A630D21(promptE, false);//disable the prompt because you are not flying low enough
		}

	}
	else {
		setPromptEnabled(&promptLeftClick, insideUFO);
		setPromptEnabled(&promptE, false);
	}
	setPromptEnabled(&promptSpace, insideUFO);//this shows up second to bottom (space)

	setPromptEnabled(&promptShift, insideUFO);
	if (insideUFO) {
		float heightAboveGround = ufomod::getHeightAboveGround();
		if (heightAboveGround < ufomod::slowModeHeight) {
			UI::_0x8A0FB4D03A630D21(promptShift, false);//disable the prompt because you are not flying high enough
		}
	}

	setPromptEnabled(&promptRightClick, runRightClick);//we are gonna always keep beam up mode
}

struct unksize2 { int a; int b; };
unksize2 func_1299(int iParam0)
{
	unksize2 ret = { iParam0 ,iParam0 };
	return ret;
}

void setCampFunds(int amt) {
	//Global_37.f_4248.f_324
	int* moneh = (int*)getGlobalPtr(37 + 4248 + 324);
	*moneh = amt;
	//Var0 = { func_1299(1736503291) };
	//unk_0xA4DDF5DF95E65EEE(&Var0, iParam0, 1);
	//unksize2 Var0 = { func_1299(1736503291) };
	unksize2 Var0 = { 1736503291 };
	STATS::_0xA4DDF5DF95E65EEE(&Var0, amt, 1);
}



int func_1433(int v)
{
	switch (v) {
	case 0: return -1880054030;//ret
	case 1: return 105540869;//ret
	case 2: return -1390686937;//ret
	case 3: return -841046000;//ret
	case 4: return -1132780182;
	case 5: return 493104853;
	case 6: return -1148661087;//9 but with arrow too
	case 7: return -1577441307;
	case 8: return -2038043934;
	case 9: return -993457185;
	case 10: return -1928689334;//used for sure
	}
}



namespace particles {
	//scr_winter4 scr_wnt4_private_car_breach
	//"scr_distance_smoke","scr_campfire_distance_smoke_sma"
	int createParticleAtCoord(float x, float y, float z, char* dictionary, char* name) {
		STREAMING::_REQUEST_NAMED_PTFX_ASSET(-458373790);
		STREAMING::REQUEST_NAMED_PTFX_ASSET(dictionary);
		GRAPHICS::USE_PARTICLE_FX_ASSET(dictionary);
		return GRAPHICS::START_PARTICLE_FX_LOOPED_AT_COORD(name, x, y, z, 0, 0, 0, 1, false, false, false, false);
	}
	void deleteParticle(int particleID) {
		GRAPHICS::STOP_PARTICLE_FX_LOOPED(particleID, false);
		GRAPHICS::REMOVE_PARTICLE_FX(particleID, false);
	}
};

int testValueThingy = 10;
namespace checkpoint {

	int createCheckpointAtCoord(float x, float y, float z, int checkpointID = -993457185) {
		//func_1433(testValueThingy)
		return GRAPHICS::_0x175668836B44CBB0(checkpointID, x, y, z, x, y, z - 1.0f, 1.0f, 0, 0, 255, 255, 0);//	_CREATE_CHECKPOINT
	}
	void deleteCheckpoint(int checkpointID) {
		GRAPHICS::DELETE_CHECKPOINT(checkpointID);
	}
};

class Portal {
public:
	Vector3 position = { 0,0,0 };
	float portalRadius = 2.0f;
	int checkpoint = 0;
	Portal(Vector3 pos) : position(pos) {
		checkpoint = checkpoint::createCheckpointAtCoord(pos.x, pos.y, pos.z, -1148661087);
	}
	~Portal() {
		checkpoint::deleteCheckpoint(checkpoint);
	}
};

class PortalPair {
public:
	Portal* a = nullptr;
	Portal* b = nullptr;
	void loop() {
		if (a != nullptr && b != nullptr) {
			runOnAllPedsAndVehicles([=](auto ped) {
				Vector3 pedloc = ENTITY::GET_ENTITY_COORDS(ped, false, false);
				float da = pedloc.sub(a->position).magnitude();
				float db = pedloc.sub(b->position).magnitude();
				if (da < a->portalRadius) {
					//teleport to b
					Vector3 p = b->position.add({ b->portalRadius + 0.1f,0,0 });
					ENTITY::SET_ENTITY_COORDS(ped, p.x, p.y, p.z, false, false, false, false);
				}
				if (db < b->portalRadius) {
					//teleport to a
					Vector3 p = a->position.add({ a->portalRadius + 0.1f,0,0 });
					ENTITY::SET_ENTITY_COORDS(ped, p.x, p.y, p.z, false, false, false, false);
				}
				}, true);//include myself in it
		}
	}
	void deleteA() {
		if (a != nullptr)
			delete a;
		a = nullptr;
	}
	void createA(Vector3 pos) {
		deleteA();
		a = new Portal(pos);
	}
	void deleteB() {
		if (b != nullptr)
			delete b;
		b = nullptr;
	}
	void createB(Vector3 pos) {
		deleteB();
		b = new Portal(pos);
	}
	void deletePortals() {
		deleteA();
		deleteB();
	}
};

namespace PortalGun {
	PortalPair portals;
	bool enabled = false;
	bool portalChoice = false;
	void deletePortals() {
		portals.deletePortals();
	}
	void loop() {
		portals.loop();
		static int promptLC = -1;
		static int promptF = -1;
		if (enabled) {
			runAtWeaponShot([=](Vector3 hitLoc) {
				if (portalChoice) {
					portals.createA(hitLoc);
					print("Portal A created");
				}
				else {
					portals.createB(hitLoc);
					print("Portal B created");
				}
				});

			/*if (portalChoice)
				consolePrint("Shoot To Place Portal A");
			else
				consolePrint("Shoot To Place Portal B");*/

			if (keyPressed[0x46]) {//F key
				keyPressed[0x46] = false;
				portalChoice = !portalChoice;
			}
			/*if (keyPressed[0x47]) {//G key
				keyPressed[0x47] = false;
				portals.deletePortals();
				print("Portals Deleted!");
			}*/

			if (inputModeChanged) {
				deletePrompt(&promptLC);
				deletePrompt(&promptF);
			}

			createPrompt(&promptLC, /*controllerMode ? Buttons::Button_R2 this caused issues but worked before so idk anymore lol : */0x07CE1E61, "Create Portal");//Left Click   INPUT_ATTACK
			char* pchoiceText = "Portal A Selected";
			if (portalChoice == false)
				pchoiceText = "Portal B Selected"; 
			createPrompt(&promptF, controllerMode ? Buttons::Button_Square : func_496(7), pchoiceText);//F    INPUT_GAME_MENU_EXTRA_OPTION

			setPromptEnabled(&promptLC, true);
			setPromptEnabled(&promptF, true);

		}
		else {
			if (promptLC != -1) {
				setPromptEnabled(&promptLC, false);
				promptLC = -1;
			}
			if (promptF != -1) {
				setPromptEnabled(&promptF, false);
				promptF = -1;
			}
		}


	}
};
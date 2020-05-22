#pragma once
#include <functional>
#include <deque>
#include <math.h>
#include "SimpleMenu.h"


enum Justification {
	JUSTIFICATION_CENTER,
	JUSTIFICATION_LEFT,
	JUSTIFICATION_RIGHT
};

enum Font {
	FONT_DEFAULT,
	FONT_RDR,
	FONT_CAPS,
	FONT_BOXES,
	FONT_CONSOLE,
	_FONT_CONSOLE,
	FONT_RDR2,
	_FONT_RDR2,
	FONT_UNKNOWN,
	FONT_CAPS2,
	_FONT_RDR
};

void DrawGameRect(float x, float y, float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void DrawGameText(float x, float y, char* text, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float scaleX = 0.342f, float scaleY = 0.342f, int justification = JUSTIFICATION_LEFT, int font = FONT_DEFAULT);
void DrawGameSprite(float x, float y, float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void LoadModelTick();
void LoadModel(uint32_t model, std::function<void(void)> callback);
void RequestModel(uint32_t model);
bool HasModelLoaded(uint32_t model);
int SpawnLoadedVehicle(char* vehName);
int SpawnLoadedPed(char* pedName, Vector3 myPos);
void ChangeLoadedPlayerModel(char* modelName);
void TeleportPlayer(float x, float y, float z);
void SetWeather(char* weather);
void AddToClockTime(int hours, int minutes, int seconds);
void SpawnPed(const char* model, std::function<void(int)> callback = nullptr, Vector3 *loc = 0);
void SpawnObject(int model, Vector3 *location = 0, std::function<void(int)> callback = nullptr);
void ChangePlayerModel(const char* model);
void SpawnVehicle(const char* vehName, std::function<void(int)> callback = nullptr);
void spawnZombieTest();
void KeyboardInput(std::function<void(char*)> callback, char *defaultText = (char*)"");
void toggleFastHorse();
void toggleGodmode();
void intoTheSky();
void forceEntity(int entity, float x, float y, float z, float rotx, float roty, float rotz, int type = 1, bool relative = true);
Vector3 GET_COORDS_INFRONT(float d, float offset = 0);
Vector3 GET_COORDS_INFRONT_OF_ANGLE(Vector3 ro, float d, float offset = 0);
void runAtWeaponShot(std::function<void(Vector3)> callback);
void shootiportTick();
void explosiveAmmoTick();
void lightningAmmoTick();
void infiniteAmmoTick();
void setPedToCombatGroup(int ped);
void setPedToCombatJustMe(int ped);
int getVehiclePedIsOn(int ped);
void RapidFireLoop();
bool isKeyboardOpen();
void testMount(int targetPed);
void testMount2(int targetPed);
void mountPatch();
void forceFieldTick();
void forcePush(float force, float maxDistanceAway = 10.0f);
void tpIntoClosestVehicle();
void shootAnimalsTick();
void giveAllWeapons();
void circlePeds();
void toggleCirclePeds();
void promptUFO(bool insideUFO, bool canLandUFO, bool canEnterUFO);
void pushEntityToLocation(int entity, Vector3 destination, float forceMultiplier = 1.0f);
void runOnAllPeds(std::function<void(int)> callback, bool includeSelf = false);
void runOnAllVehicles(std::function<void(int)> callback);
void runOnAllPedsAndVehicles(std::function<void(int)> callback, bool includeSelf = false);
void setPedKillable(int micah);
void addPedToHumanGroup(int human);
void addPedToZombieGroup(int zombie);
//returns null pointer if calcelled
char *keyboardInput(char *defaultText);
bool keyboardInput(char *defaultText, int *ptr);
bool keyboardInput(char *defaultText, float *ptr);
class ZombieArena {
public:
	ZombieArena() {

	}

	void Tick() {

	}
};

namespace runlater {
	void loop();
	void runlater(int interval, std::function<void(void)> callback);
};

extern int testValueThingy;
namespace PortalGun {
	void loop();
	void deletePortals();
	extern bool enabled;
};

namespace forge {
	void toggle();
	void forgetObject();
	void tick();
	void initiateGravityPulse(int obj);
	extern int blacklistItem;
	extern int objectSelected;
	extern bool gravityGun;
	extern bool enabled;
}
namespace camShit {
	void loop();
}
namespace predatorMissile {
	void loop();
	void startMissile();
};

namespace ufomod {
	void toggle(bool startFresh = false);
	void loop();
	void toggleWeapon();
	void deleteUfo();
	extern bool enabled;
	extern int ammoType;
	//extern float skySpeed;
};

class Noclip
{
public:
	Noclip();
	void Tick();
	void Toggle();
	void SetHighSpeed(bool enable);
private:
	bool highSpeed = false;
	bool enabled;
	float speed;
};

extern bool keyPressed[0xFF];
extern bool Wpressed;
extern bool Spressed;
extern bool Apressed;
extern bool Dpressed;
extern bool RCpressed;
extern bool LCpressed;
extern bool RCJustPressed;
extern bool LCJustPressed;
extern bool ShiftPressed;
extern bool CtrlPressed;
extern bool EnterPressed;
extern bool shootiportBool;
extern bool explosiveAmmoBool;
extern bool infiniteAmmoBool;
extern bool lightningAmmoBool;
extern const char *zombiearr[];
extern bool RapidfireActive;
extern bool forceFieldBool;
extern bool forcePushBool;
extern bool shootAnimals;
extern bool shiftKeyPressed;
extern SimpleMenu simpleMenu;
extern int textFont;
extern bool circlePedsForceEnabled;
//extern int explosionID;
extern int TEST_FLAG;
extern bool godmode;
extern bool fastHorseEnabled;
extern bool WIDEMOD;
extern bool controllerMode;
extern bool inputModeChanged;

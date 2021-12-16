#pragma once
char *ftoa(float n, char* res, int afterpoint); //from SimpleMenu.cpp
bool keyboardInput(char *defaultText, int *ptr); //from NativeUtil.h
bool keyboardInput(char *defaultText, float *ptr); //from NativeUtil.h

enum Submenus {
	MENU_MAIN,
	MENU_SPAWN_VEHICLES,
	MENU_SPAWN_PEDS,
	MENU_CHANGE_PED,
	MENU_TELEPORTS,
	MENU_WEATHER,
	MENU_TIME,
	MENU_FUN,
	MENU_WEAPONS,
	MENU_SELF,
	COUNT_OF_MENUS
};

enum OptionType {
	OPTIONTYPE_DEFAULT,
	OPTIONTYPE_INT,
	OPTIONTYPE_FLOAT,
	OPTIONTYPE_BOOL
};

union AnyValue {
	float f;
	int i;
	bool b;
};
struct OptionData {
	OptionType type = OPTIONTYPE_DEFAULT;
	void *value = nullptr;
	AnyValue step;
	AnyValue min;
	AnyValue max;
	char *infoText = nullptr;
	bool clickOnScroll = false;
	void setClickOnScroll(bool clickOnScroll) {
		this->clickOnScroll = clickOnScroll;
	}
	void setInfoText(char *text) {
		this->infoText = text;
	}
	void set(bool *value) {
		type = OPTIONTYPE_BOOL;
		this->step.i = 1;
		this->value = value;
		this->min.i = 0;
		this->max.i = 1;
	}
	void set(int *value, int min, int max, int step) {
		type = OPTIONTYPE_INT;
		this->step.i = step;
		this->value = value;
		this->min.i = min;
		this->max.i = max;
	}
	void set(float *value, float min, float max, float step) {
		type = OPTIONTYPE_FLOAT;
		this->step.f = step;
		this->value = value;
		this->min.f = min;
		this->max.f = max;
	}
	void handleIncrement() {
		/*if (type == OPTIONTYPE_BOOL) {
			bool *val = (bool*)this->value;
			*val = !*val;
		}*/
		if (type == OPTIONTYPE_INT) {
			int *val = (int*)this->value;
			*val += this->step.i;
			if (*val > this->max.i)
				*val = this->min.i;
			if (*val < this->min.i)
				*val = this->max.i;
		}
		if (type == OPTIONTYPE_FLOAT) {
			float *val = (float*)this->value;
			*val += this->step.f;
			if (*val > this->max.f)
				*val = this->min.f;
			if (*val < this->min.f)
				*val = this->max.f;
		}
	}
	void handleDecrement() {
		/*if (type == OPTIONTYPE_BOOL) {
			bool *val = (bool*)this->value;
			*val = !*val;
		}*/
		if (type == OPTIONTYPE_INT) {
			int *val = (int*)this->value;
			*val -= this->step.i;
			if (*val > this->max.i)
				*val = this->min.i;
			if (*val < this->min.i)
				*val = this->max.i;
		}
		if (type == OPTIONTYPE_FLOAT) {
			float *val = (float*)this->value;
			*val -= this->step.f;
			if (*val > this->max.f)
				*val = this->min.f;
			if (*val < this->min.f)
				*val = this->max.f;
		}
	}
	void handleSwap() {
		if (type == OPTIONTYPE_BOOL) {
			bool *val = (bool*)this->value;
			*val = !*val;
		}
	}


	void customInput() {
		static char buf[50];
		if (type == OPTIONTYPE_INT) {
			int *val = (int*)this->value;
			keyboardInput(_itoa(*val,buf,10),val);
		}
		if (type == OPTIONTYPE_FLOAT) {
			float *val = (float*)this->value;
			keyboardInput(ftoa(*val, buf, 2), val);
		}
	}
	char *toString() {
		static char buf[100];
		if (type == OPTIONTYPE_DEFAULT)
			return nullptr;
		if (type == OPTIONTYPE_INT) {
			snprintf(buf,sizeof(buf),"<%i>",*(int*)this->value);
		}
		if (type == OPTIONTYPE_FLOAT) {
			snprintf(buf, sizeof(buf), "<%.2f>", *(float*)this->value);
		}
		if (type == OPTIONTYPE_BOOL) {
			int val = *(bool*)this->value;
			if (val == 0)
				return nullptr;
			else
				return "*";
		}
		return buf;
	}
};

class SimpleMenu {
public:
	SimpleMenu(float x, float y);
	void setOptionList(int menu, int length, char **list);
	void setOptionData(int menu, int option, int *value, int min = (int)0x80000000, int max = 0x7FFFFFFF, int step = 1);
	void setOptionData(int menu, int option, float *value, float min = 0.0f, float max = 100.0f, float step = 1.0f);
	void setOptionData(int menu, int option, bool *value);
	void setOptionInfoText(int menu, int option, char *text);
	void setOptionClickOnScroll(int menu, int option, bool clickOnScroll);
	void render();
	void toggle();
	bool isOpen() { return open; };
	void select();
	void up();
	void down();
	void right();
	void left();
	void customInput();
	void openSubmenu(int submenu);
	void back();
	int getCurrentMenu();
	int clickOption();//returns -1 if not clicked, any other if item was selected
	int maxOptionsToRender = 25;
private:
	void renderFromAtoB(int startOption, int endOption, int selectedOption);
	float x,y;
	bool open;
	int optionCount[COUNT_OF_MENUS];
	int selectedOption[COUNT_OF_MENUS];
	bool clicked;
	char **optionList[COUNT_OF_MENUS];
	OptionData *optionData[COUNT_OF_MENUS];
	int menuTree[COUNT_OF_MENUS];//max it would ever be is COUNT_OF_MENUS but can be less
	int currentDepth;
};


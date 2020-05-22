#include "pch.h"
#include "SimpleMenu.h"
#include "NativeUtil.h"

SimpleMenu::SimpleMenu(float x, float y) {
	this->x = x;
	this->y = y;
}

void DrawHorizontalLine(float x, float y, float length) {
	DrawGameRect(x + length / 2, y + 0.001f / 2, length, 0.001f, 255, 255, 255, 128);
}
void DrawVerticalLine(float x, float y, float length) {
	DrawGameRect(x + 0.001f / 2, y + length / 2, 0.001f, length, 255, 255, 255, 128);
}

void SimpleMenu::renderFromAtoB(int startOption, int endOption, int selectedOption) {
	float height = 0.025f;
	float width = 0.25f;
	float widthOfScrollbar = 0.005f;
	float textPadding = 0.005f;
	int backgroundOpacity = 128;
	if (this->open) {
		//do this to width so it covers the top of the scrollbar
		width += widthOfScrollbar;
		DrawGameText(this->x + width / 2, this->y + height * -1, "gopro_2027's Playground", 255, 255, 255, 255, 0.35f, 0.35f, JUSTIFICATION_CENTER, FONT_RDR2);
		DrawGameRect(this->x + width / 2, this->y + height / 2 + height * -1, width, height, 0, 0, 0, 255);
		width -= widthOfScrollbar;

		//DrawGameRect(this->x + width / 2, this->y + 0.001f / 2/* + height * -1*/, width, 0.001f, 255, 255, 255, 255);//line underneath text
		



		for (int i = startOption; i < endOption; i++) {

			int textr, textg, textb;
			textr = textg = textb = 255;
			int rectr, rectg, rectb;
			rectr = rectg = rectb = 0;
			if (selectedOption == i) {
				textr = textg = textb = 0;
				//rectr = rectg = rectb = 255;//white
				rectr = 255;//red
			}

			//char buf[256];
			char *str = this->optionList[getCurrentMenu()][i];
			char *optionData = this->optionData[getCurrentMenu()][i].toString();
			/*strcpy(buf, str);
			if (optionData != nullptr) {
				strcat(buf, " ");
				strcat(buf, optionData);
			}*/

			DrawGameText(this->x + textPadding, this->y + height * (i - startOption), str, textr, textg, textb, 255, 0.35f, 0.35f, JUSTIFICATION_LEFT, FONT_CAPS);
			if (optionData != nullptr) {
				DrawGameText(this->x + width - textPadding, this->y + height * (i - startOption), optionData, textr, textg, textb, 255, 0.35f, 0.35f, JUSTIFICATION_RIGHT, FONT_RDR2);
			}
			DrawGameRect(this->x + width / 2, this->y + height / 2 + height * (i - startOption), width, height, rectr, rectg, rectb, backgroundOpacity);
		}

		char *optionInfoText = this->optionData[getCurrentMenu()][selectedOption].infoText;
		if (optionInfoText != nullptr) {
			width += widthOfScrollbar;
			DrawGameText(this->x + textPadding, this->y + height * (endOption - startOption), optionInfoText, 255, 255, 255, 255, 0.35f, 0.35f, JUSTIFICATION_LEFT, FONT_CAPS);
			DrawGameRect(this->x + width / 2, this->y + height / 2 + height * (endOption - startOption), width, height, 0, 0, 0, 255);
			width -= widthOfScrollbar;
		}


		//draws white lines around the menu
		bool drawSideLines = false;
		if (drawSideLines) {
			//Drawing side lines

			//DrawGameRect(this->x + width / 2, this->y + 0.001f / 2/* + height * -1*/, width, 0.001f, 255, 255, 255, 255);
			int oc = endOption - startOption + 1;//total length
			float heightOfMenu = oc * height;

			DrawHorizontalLine(this->x, this->y + height * -1, width);
			DrawHorizontalLine(this->x, this->y + height * -1 + heightOfMenu, width);//bottom

			DrawVerticalLine(this->x, this->y + height * -1, heightOfMenu);//left
			DrawVerticalLine(this->x + width, this->y + height * -1, heightOfMenu);//right

		}

		int oc = endOption - startOption;//total length (not including header for the scrollbar)
		float heightOfMenu = oc * height;
		
		//DrawGameRect(this->x + width + widthOfScrollbar / 2.0f, this->y + heightOfMenu / 2.0f, widthOfScrollbar, heightOfMenu, 0, 0, 0, backgroundOpacity);//full background rect

		int optionCount = this->optionCount[getCurrentMenu()];
		float lengthPerOption = heightOfMenu / (float)optionCount;
		float barHeight = lengthPerOption * (float)(endOption - startOption);
		float topYPos = lengthPerOption * startOption;

		//draw the scrollbar
		DrawGameRect(this->x + width + widthOfScrollbar / 2.0f, this->y + topYPos + barHeight / 2.0f, widthOfScrollbar, barHeight, 255, 0, 0, backgroundOpacity);//full background rect

		//draw the grey areas above and below
		DrawGameRect(this->x + width + widthOfScrollbar / 2.0f, this->y + topYPos / 2.0f, widthOfScrollbar, topYPos, 0, 0, 0, backgroundOpacity);//top
		DrawGameRect(this->x + width + widthOfScrollbar / 2.0f, this->y + topYPos + barHeight + (heightOfMenu - (topYPos + barHeight))/2.0f, widthOfScrollbar, (heightOfMenu - (topYPos + barHeight)), 0, 0, 0, backgroundOpacity);

	}
}

void SimpleMenu::render() {
	//render in here
	int optionCount = this->optionCount[getCurrentMenu()];
	int selectedOption = this->selectedOption[getCurrentMenu()];
	int maxSizeDiv2 = this->maxOptionsToRender / 2;
	if (optionCount > maxOptionsToRender) {
		int start = selectedOption - maxSizeDiv2;
		int end = selectedOption + maxSizeDiv2;// I cri sometimes
		if (this->maxOptionsToRender % 2 != 0) {
			//odd
			end++;//add 1 to end so it's in dead center
		}
		if (start < 0) {
			//end -= start;
			//start = 0;
			start = 0;
			end = this->maxOptionsToRender;
		}
		if (end > optionCount) {
			//start -= (end - (optionCount-1));
			//end = optionCount-1;
			end = optionCount;
			start = end - (this->maxOptionsToRender);
		}
		renderFromAtoB(start, end, selectedOption);
	}
	else {
		//just render like normal
		renderFromAtoB(0,optionCount,selectedOption);
	}

}

void SimpleMenu::toggle() {
	this->open = !this->open;
	this->menuTree[0] = MENU_MAIN;
}

int SimpleMenu::getCurrentMenu() {
	return this->menuTree[this->currentDepth];
}

void SimpleMenu::select() {
	if (this->open) {
		this->clicked = true;
	}
}

void SimpleMenu::up() {
	if (this->open) {
		selectedOption[getCurrentMenu()]++;
		if (selectedOption[getCurrentMenu()] >= optionCount[getCurrentMenu()])
			selectedOption[getCurrentMenu()] = 0;
	}
}

void SimpleMenu::down() {
	if (this->open) {
		selectedOption[getCurrentMenu()]--;
		if (selectedOption[getCurrentMenu()] < 0)
			selectedOption[getCurrentMenu()] = optionCount[getCurrentMenu()] - 1;
	}
}

int SimpleMenu::clickOption() {
	if (this->open) {
		if (this->clicked) {
			this->clicked = false;
			if (this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].type == OptionType::OPTIONTYPE_BOOL) {
				this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].handleSwap();
			}
			return this->selectedOption[this->getCurrentMenu()];
		}
	}
	return -1;
}

void SimpleMenu::openSubmenu(int submenu) {
	if (this->open) {
		currentDepth++;
		menuTree[currentDepth] = submenu;
	}
}

void SimpleMenu::back() {
	if (this->open) {
		currentDepth--;
		if (currentDepth == -1) {
			currentDepth = 0;
			toggle();//toggle it to closed
		}
	}
}

void SimpleMenu::setOptionList(int menu, int length, char **list) {
	this->optionCount[menu] = length;
	this->optionList[menu] = list;
	this->optionData[menu] = new OptionData[length];
}

void SimpleMenu::setOptionData(int menu, int option, int *value, int min, int max, int step) {
	this->optionData[menu][option].set(value, min, max, step);
	this->setOptionInfoText(menu, option, "Press N or Square to input custom value");
}
void SimpleMenu::setOptionData(int menu, int option, float *value, float min, float max, float step) {
	this->optionData[menu][option].set(value, min, max, step);
	this->setOptionInfoText(menu, option, "Press N or Square to input custom value");
}
void SimpleMenu::setOptionData(int menu, int option, bool *value) {
	this->optionData[menu][option].set(value);
}
void SimpleMenu::setOptionInfoText(int menu, int option, char *text) {
	this->optionData[menu][option].setInfoText(text);
}
void SimpleMenu::setOptionClickOnScroll(int menu, int option, bool clickOnScroll) {
	this->optionData[menu][option].setClickOnScroll(clickOnScroll);
}
//template <typename T> void SimpleMenu::setOptionData<float>(int menu, int option, float *value, float min, float max, float step);
//template <typename T> void SimpleMenu::setOptionData<int>(int menu, int option, T *value, T min, T max, T step);
//template <typename T> void SimpleMenu::setOptionData<int32_t>(int menu, int option, T *value, T min, T max, T step);
//template void SimpleMenu::setOptionData<bool>(int menu, int option, bool *value, bool min, bool max, bool step);

void SimpleMenu::right() {
	if (this->open) {
		this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].handleIncrement();
		if (this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].clickOnScroll) {
			//handle click
			select();
		}
	}
}
void SimpleMenu::left() {
	if (this->open) {
		this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].handleDecrement();
		if (this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].clickOnScroll) {
			//handle click
			select();
		}
	}
}
void SimpleMenu::customInput() {
	if (this->open) {
		this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].customInput();
		if (this->optionData[getCurrentMenu()][selectedOption[getCurrentMenu()]].clickOnScroll) {
			//handle click
			select();
		}
	}
}





//some functions required ftoa

// Reverses a string 'str' of length 'len' 
void reverse(char* str, int len)
{
	int i = 0, j = len - 1, temp;
	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

// Converts a given integer x to string str[].  
// d is the number of digits required in the output.  
// If d is more than the number of digits in x,  
// then 0s are added at the beginning. 
int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}

	// If number of digits required is more, then 
	// add 0s at the beginning 
	while (i < d)
		str[i++] = '0';

	reverse(str, i);
	str[i] = '\0';
	return i;
}
// Converts a floating-point/double number to a string. 
char *ftoa(float n, char* res, int afterpoint)
{
	// Extract integer part 
	int ipart = (int)n;

	// Extract floating part 
	float fpart = n - (float)ipart;

	// convert integer part to string 
	int i = intToStr(ipart, res, 0);

	// check for display option after point 
	if (afterpoint != 0) {
		res[i] = '.'; // add dot 

		// Get the value of fraction part upto given no. 
		// of points after dot. The third parameter  
		// is needed to handle cases like 233.007 
		fpart = fpart * pow(10, afterpoint);

		intToStr((int)fpart, res + i + 1, afterpoint);
	}
	return res;
}
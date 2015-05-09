/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gfx.h"

/**
 * This demo demonstrates many of the GWIN widgets.
 * On the "Radio" tab try playing with the color radio buttons.
 * On the "Checkbox" tab try playing with the "Disable All" checkbox.
 */

/**
 * The image files must be stored on a GFILE file-system.
 * Use either GFILE_NEED_NATIVEFS or GFILE_NEED_ROMFS (or both).
 *
 * The ROMFS uses the file "romfs_files.h" to describe the set of files in the ROMFS.
 */

/* Our custom yellow style */
static const GWidgetStyle YellowWidgetStyle = {
	Yellow,							// window background

	// enabled color set
	{
		HTML2COLOR(0x0000FF),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0xE0E0E0),		// progress - inactive area
	},

	// disabled color set
	{
		HTML2COLOR(0xC0C0C0),		// text
		HTML2COLOR(0x808080),		// edge
		HTML2COLOR(0xE0E0E0),		// fill
		HTML2COLOR(0xC0E0C0),		// progress - active area
	},

	// pressed color set
	{
		HTML2COLOR(0xFF00FF),		// text
		HTML2COLOR(0x404040),		// edge
		HTML2COLOR(0x808080),		// fill
		HTML2COLOR(0x00E000),		// progress - active area
	},
};

/* The variables we need */
static GListener	gl;
static GHandle		ghConsole;
static GHandle		ghTabButtons, ghTabSliders, ghTabCheckboxes, ghTabLabels, ghTabRadios, ghTabLists, ghTabImages, ghTabProgressbar;
static GHandle		ghButton1, ghButton2, ghButton3, ghButton4;
static GHandle		ghSlider1, ghSlider2, ghSlider3, ghSlider4;
static GHandle		ghCheckbox1, ghCheckbox2, ghCheckDisableAll;
static GHandle		ghLabel1;
static GHandle		ghRadio1, ghRadio2;
static GHandle		ghRadioBlack, ghRadioWhite, ghRadioYellow;
static GHandle		ghList1, ghList2, ghList3, ghList4;
static GHandle		ghImage1;
static GHandle		ghProgressbar1;
static gdispImage	imgYesNo;

/* Some useful macros */
#define	ScrWidth			gdispGetWidth()
#define	ScrHeight			gdispGetHeight()

#define TAB_HEIGHT			30
#define LABEL_HEIGHT		40
#define BUTTON_WIDTH		50
#define BUTTON_HEIGHT		30
#define LIST_WIDTH			100
#define LIST_HEIGHT			80
#define SLIDER_WIDTH		20
#define CHECKBOX_WIDTH		80
#define CHECKBOX_HEIGHT		20
#define RADIO_WIDTH			50
#define RADIO_HEIGHT		20
#define COLOR_WIDTH			80
#define DISABLEALL_WIDTH	100
#define GROUP_TABS			0
#define GROUP_YESNO			1
#define GROUP_COLORS		2

/**
 * Create all the widgets.
 * With the exception of the Tabs they are all created invisible.
 */
static void createWidgets(void) {
	GWidgetInit		wi;

	gwinWidgetClearInit(&wi);

	// Create the Tabs
	wi.g.show = TRUE; wi.customDraw = gwinRadioDraw_Tab;
	wi.g.width = ScrWidth/7; wi.g.height = TAB_HEIGHT; wi.g.y = 0;
	wi.g.x = 0*wi.g.width; wi.text = "Buttons";
	ghTabButtons     = gwinRadioCreate(0, &wi, GROUP_TABS);
	wi.g.x = 1*wi.g.width; wi.text = "Sliders";
	ghTabSliders     = gwinRadioCreate(0, &wi, GROUP_TABS);
	wi.g.x = 2*wi.g.width; wi.text = "Checkbox";
	ghTabCheckboxes  = gwinRadioCreate(0, &wi, GROUP_TABS);
	wi.g.x = 3*wi.g.width; wi.text = "Radios";
	ghTabRadios      = gwinRadioCreate(0, &wi, GROUP_TABS);
	wi.g.x = 4*wi.g.width; wi.text = "Lists";
	ghTabLists       = gwinRadioCreate(0, &wi, GROUP_TABS);
	wi.g.x = 5*wi.g.width; wi.text = "Labels";
	ghTabLabels      = gwinRadioCreate(0, &wi, GROUP_TABS);
	wi.g.x = 6*wi.g.width; wi.text = "Images";
	ghTabImages      = gwinRadioCreate(0, &wi, GROUP_TABS);
	wi.g.y = TAB_HEIGHT;
	wi.g.x = 0*wi.g.width; wi.text = "Progressbar";
	ghTabProgressbar = gwinRadioCreate(0, &wi, GROUP_TABS);

	// Buttons
	wi.g.show = FALSE; wi.customDraw = 0;
	wi.g.width = BUTTON_WIDTH; wi.g.height = BUTTON_HEIGHT; wi.g.y = 2*TAB_HEIGHT+10;
	wi.g.x = 0+0*(BUTTON_WIDTH+1); wi.text = "B1";
	ghButton1 = gwinButtonCreate(0, &wi);
	wi.g.x = 0+1*(BUTTON_WIDTH+1); wi.text = "B2";
	ghButton2 = gwinButtonCreate(0, &wi);
	wi.g.x = 0+2*(BUTTON_WIDTH+1); wi.text = "B3"; 
	ghButton3 = gwinButtonCreate(0, &wi);
	wi.g.x = 0+3*(BUTTON_WIDTH+1); wi.text = "B4";
	ghButton4 = gwinButtonCreate(0, &wi);

	// Horizontal Sliders
	wi.g.width = ScrWidth/2-2; wi.g.height = SLIDER_WIDTH; wi.g.x = ScrWidth/2+1;
	wi.g.y = ScrHeight/2-2*(SLIDER_WIDTH+1); wi.text = "S1";
	ghSlider1 = gwinSliderCreate(0, &wi);
	wi.g.y = ScrHeight/2-1*(SLIDER_WIDTH+1); wi.text = "S2";
	ghSlider2 = gwinSliderCreate(0, &wi);

	// Vertical Sliders
	wi.g.width = SLIDER_WIDTH; wi.g.height = ScrHeight/2-2; wi.g.y = ScrHeight/2+1;
	wi.g.x = 0+0*(SLIDER_WIDTH+1); wi.text = "S3";
	ghSlider3 = gwinSliderCreate(0, &wi);
	wi.g.x = 0+1*(SLIDER_WIDTH+1); wi.text = "S4";
	ghSlider4 = gwinSliderCreate(0, &wi);

	// Checkboxes - for the 2nd checkbox we apply special drawing before making it visible
	wi.g.width = CHECKBOX_WIDTH; wi.g.height = CHECKBOX_HEIGHT; wi.g.x = 0;
	wi.g.y = 2*TAB_HEIGHT+10+0*(CHECKBOX_HEIGHT+1); wi.text = "C1";
	ghCheckbox1 = gwinCheckboxCreate(0, &wi);
	wi.customDraw = gwinCheckboxDraw_CheckOnRight;
	wi.g.y = 2*TAB_HEIGHT+10+1*(CHECKBOX_HEIGHT+1); wi.text = "C2";
	ghCheckbox2 = gwinCheckboxCreate(0, &wi);
	wi.customDraw = 0; wi.g.width = DISABLEALL_WIDTH;
	wi.g.y = 2*TAB_HEIGHT+10+2*(CHECKBOX_HEIGHT+1); wi.text = "Disable All";
	ghCheckDisableAll = gwinCheckboxCreate(0, &wi);

    // Labels
	wi.g.width = 0;	wi.g.height = LABEL_HEIGHT;	// dynamic width, fixed height
	wi.g.y = 2*TAB_HEIGHT+10+2*(CHECKBOX_HEIGHT+1); wi.text = "Label";
	ghLabel1 = gwinLabelCreate(0, &wi);

	// Radio Buttons
	wi.g.width = RADIO_WIDTH; wi.g.height = RADIO_HEIGHT; wi.g.y = 2*TAB_HEIGHT+10;
	wi.g.x = 0*wi.g.width; wi.text = "Yes";
	ghRadio1 = gwinRadioCreate(0, &wi, GROUP_YESNO);
	wi.g.x = 1*wi.g.width; wi.text = "No";
	ghRadio2 = gwinRadioCreate(0, &wi, GROUP_YESNO);
	wi.g.width = COLOR_WIDTH; wi.g.y += RADIO_HEIGHT+5;
	wi.g.x = 0*wi.g.width; wi.text = "Black";
	ghRadioBlack = gwinRadioCreate(0, &wi, GROUP_COLORS);
	wi.g.x = 1*wi.g.width; wi.text = "White";
	ghRadioWhite = gwinRadioCreate(0, &wi, GROUP_COLORS);
	wi.g.x = 2*wi.g.width; wi.text = "Yellow";
	ghRadioYellow = gwinRadioCreate(0, &wi, GROUP_COLORS);
	gwinRadioPress(ghRadioWhite);

	// Lists
	wi.g.show = FALSE; wi.customDraw = 0;
	wi.g.width = LIST_WIDTH; wi.g.height = LIST_HEIGHT; wi.g.y = 2*TAB_HEIGHT+10;
	wi.g.x = 0+0*(LIST_WIDTH+5); wi.text = "L1";
	ghList1 = gwinListCreate(0, &wi, FALSE);
	gwinListAddItem(ghList1, "Item 0", FALSE);
	gwinListAddItem(ghList1, "Item 1", FALSE);
	gwinListAddItem(ghList1, "Item 2", FALSE);
	gwinListAddItem(ghList1, "Item 3", FALSE);
	gwinListAddItem(ghList1, "Item 4", FALSE);
	gwinListAddItem(ghList1, "Item 5", FALSE);
	gwinListAddItem(ghList1, "Item 6", FALSE);
	gwinListAddItem(ghList1, "Item 7", FALSE);
	gwinListAddItem(ghList1, "Item 8", FALSE);
	gwinListAddItem(ghList1, "Item 9", FALSE);
	gwinListAddItem(ghList1, "Item 10", FALSE);
	gwinListAddItem(ghList1, "Item 11", FALSE);
	gwinListAddItem(ghList1, "Item 12", FALSE);
	gwinListAddItem(ghList1, "Item 13", FALSE);
	wi.g.x = 0+1*(LIST_WIDTH+5); wi.text = "L2";
	ghList2 = gwinListCreate(0, &wi, TRUE);
	gwinListAddItem(ghList2, "Item 0", FALSE);
	gwinListAddItem(ghList2, "Item 1", FALSE);
	gwinListAddItem(ghList2, "Item 2", FALSE);
	gwinListAddItem(ghList2, "Item 3", FALSE);
	gwinListAddItem(ghList2, "Item 4", FALSE);
	gwinListAddItem(ghList2, "Item 5", FALSE);
	gwinListAddItem(ghList2, "Item 6", FALSE);
	gwinListAddItem(ghList2, "Item 7", FALSE);
	gwinListAddItem(ghList2, "Item 8", FALSE);
	gwinListAddItem(ghList2, "Item 9", FALSE);
	gwinListAddItem(ghList2, "Item 10", FALSE);
	gwinListAddItem(ghList2, "Item 11", FALSE);
	gwinListAddItem(ghList2, "Item 12", FALSE);
	gwinListAddItem(ghList2, "Item 13", FALSE);
	wi.g.x = 0+2*(LIST_WIDTH+5); wi.text = "L3";
	ghList3 = gwinListCreate(0, &wi, TRUE);
	gwinListAddItem(ghList3, "Item 0", FALSE);
	gwinListAddItem(ghList3, "Item 1", FALSE);
	gwinListAddItem(ghList3, "Item 2", FALSE);
	gwinListAddItem(ghList3, "Item 3", FALSE);
	gdispImageOpenFile(&imgYesNo, "image_yesno.gif");
	gwinListItemSetImage(ghList3, 1, &imgYesNo);
	gwinListItemSetImage(ghList3, 3, &imgYesNo);
	wi.g.x = 0+3*(LIST_WIDTH+5); wi.text = "L4";
	ghList4 = gwinListCreate(0, &wi, TRUE);
	gwinListAddItem(ghList4, "Item 0", FALSE);
	gwinListAddItem(ghList4, "Item 1", FALSE);
	gwinListAddItem(ghList4, "Item 2", FALSE);
	gwinListAddItem(ghList4, "Item 3", FALSE);
	gwinListAddItem(ghList4, "Item 4", FALSE);
	gwinListAddItem(ghList4, "Item 5", FALSE);
	gwinListAddItem(ghList4, "Item 6", FALSE);
	gwinListAddItem(ghList4, "Item 7", FALSE);
	gwinListAddItem(ghList4, "Item 8", FALSE);
	gwinListAddItem(ghList4, "Item 9", FALSE);
	gwinListAddItem(ghList4, "Item 10", FALSE);
	gwinListAddItem(ghList4, "Item 11", FALSE);
	gwinListAddItem(ghList4, "Item 12", FALSE);
	gwinListAddItem(ghList4, "Item 13", FALSE);
	gwinListSetScroll(ghList4, scrollSmooth);

	// Image
	wi.g.x = 20; wi.g.y = 2*TAB_HEIGHT+20; wi.g.width = 200; wi.g.height = 100;
	ghImage1 = gwinImageCreate(0, &wi.g);
	gwinImageOpenFile(ghImage1, "romfs_img_ugfx.gif");

	// Progressbar
	wi.g.show = FALSE; wi.customDraw = 0;
	wi.g.width = 200; wi.g.height = 20; wi.g.y = 2*TAB_HEIGHT+10;
	wi.g.x = 20; wi.text = "Progressbar 1";
	ghProgressbar1 = gwinProgressbarCreate(0, &wi);
	gwinProgressbarSetResolution(ghProgressbar1, 10);

	// Console - we apply some special colors before making it visible
	wi.g.width = ScrWidth/2-1; wi.g.height = ScrHeight/2-1;
	wi.g.x = ScrWidth/2+1; wi.g.y = ScrHeight/2+1;
	ghConsole = gwinConsoleCreate(0, &wi.g);
    gwinSetColor(ghConsole, Yellow);
    gwinSetBgColor(ghConsole, Black);
}

/**
 * Set the visibility of widgets based on which tab is selected.
 */
static void setTab(GHandle tab) {
	/* Make sure everything is invisible first */
	gwinSetVisible(ghButton1, FALSE);
	gwinSetVisible(ghButton2, FALSE);
	gwinSetVisible(ghButton3, FALSE);
	gwinSetVisible(ghButton4, FALSE);
	gwinSetVisible(ghSlider1, FALSE);
	gwinSetVisible(ghSlider2, FALSE);
	gwinSetVisible(ghSlider3, FALSE);
	gwinSetVisible(ghSlider4, FALSE);
	gwinSetVisible(ghCheckbox1, FALSE);
	gwinSetVisible(ghCheckbox2, FALSE);	
	gwinSetVisible(ghCheckDisableAll, FALSE);
	gwinSetVisible(ghLabel1, FALSE);
	gwinSetVisible(ghRadio1, FALSE);
	gwinSetVisible(ghRadio2, FALSE);
	gwinSetVisible(ghRadioWhite, FALSE);
	gwinSetVisible(ghRadioBlack, FALSE);
	gwinSetVisible(ghRadioYellow, FALSE);
	gwinSetVisible(ghList1, FALSE);
	gwinSetVisible(ghList2, FALSE);
	gwinSetVisible(ghList3, FALSE);
	gwinSetVisible(ghList4, FALSE);
	gwinSetVisible(ghImage1, FALSE);
	gwinSetVisible(ghProgressbar1, FALSE);

	// Stop the progress bar
	gwinProgressbarStop(ghProgressbar1);
	gwinProgressbarReset(ghProgressbar1);

	/* Turn on widgets depending on the tab selected */
	if (tab == ghTabButtons) {
		gwinSetVisible(ghButton1, TRUE);
		gwinSetVisible(ghButton2, TRUE);
		gwinSetVisible(ghButton3, TRUE);
		gwinSetVisible(ghButton4, TRUE);
	} else if (tab == ghTabSliders) {
		gwinSetVisible(ghSlider1, TRUE);
		gwinSetVisible(ghSlider2, TRUE);
		gwinSetVisible(ghSlider3, TRUE);
		gwinSetVisible(ghSlider4, TRUE);
	} else if (tab == ghTabCheckboxes) {
		gwinSetVisible(ghCheckbox1, TRUE);
		gwinSetVisible(ghCheckbox2, TRUE);
		gwinSetVisible(ghCheckDisableAll, TRUE);
	} else if (tab == ghTabLabels) {
		gwinSetVisible(ghLabel1, TRUE);
	} else if (tab == ghTabRadios) {
		gwinSetVisible(ghRadio1, TRUE);
		gwinSetVisible(ghRadio2, TRUE);
		gwinSetVisible(ghRadioWhite, TRUE);
		gwinSetVisible(ghRadioBlack, TRUE);
		gwinSetVisible(ghRadioYellow, TRUE);
	} else if (tab == ghTabLists) {
		gwinSetVisible(ghList1, TRUE);
		gwinSetVisible(ghList2, TRUE);
		gwinSetVisible(ghList3, TRUE);
		gwinSetVisible(ghList4, TRUE);
	} else if (tab == ghTabImages) {
		gwinSetVisible(ghImage1, TRUE);
	} else if (tab == ghTabProgressbar) {
		gwinSetVisible(ghProgressbar1, TRUE);

		// Start the progress bar
		gwinProgressbarStart(ghProgressbar1, 500);
	}
}

/**
 * Set the enabled state of every widget (except the tabs etc)
 */
static void setEnabled(bool_t ena) {
	gwinSetEnabled(ghButton1, ena);
	gwinSetEnabled(ghButton2, ena);
	gwinSetEnabled(ghButton3, ena);
	gwinSetEnabled(ghButton4, ena);
	gwinSetEnabled(ghSlider1, ena);
	gwinSetEnabled(ghSlider2, ena);
	gwinSetEnabled(ghSlider3, ena);
	gwinSetEnabled(ghSlider4, ena);
	gwinSetEnabled(ghCheckbox1, ena);
	gwinSetEnabled(ghCheckbox2, ena);
	//gwinSetEnabled(ghCheckDisableAll, TRUE);
	gwinSetEnabled(ghLabel1, ena);
	gwinSetEnabled(ghRadio1, ena);
	gwinSetEnabled(ghRadio2, ena);
	gwinSetEnabled(ghList1, ena);
	gwinSetEnabled(ghList2, ena);
	gwinSetEnabled(ghList3, ena);
	gwinSetEnabled(ghList4, ena);
	gwinSetEnabled(ghRadioWhite, ena);
	gwinSetEnabled(ghRadioBlack, ena);
	gwinSetEnabled(ghRadioYellow, ena);
	gwinSetEnabled(ghImage1, ena);
	gwinSetEnabled(ghProgressbar1, ena);
}

int main(void) {
	GEvent *			pe;

	// Initialize the display
	gfxInit();

	// Connect the mouse
	#if GINPUT_NEED_MOUSE
		gwinAttachMouse(0);
	#endif

	// Set the widget defaults
	gwinSetDefaultFont(gdispOpenFont("*"));
	gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
	gdispClear(White);

	// Create the gwin windows/widgets
	createWidgets();

    // Assign toggles and dials to specific buttons & sliders etc.
	#if GINPUT_NEED_TOGGLE
		gwinAttachToggle(ghButton1, 0, 0);
		gwinAttachToggle(ghButton2, 0, 1);
	#endif
	#if GINPUT_NEED_DIAL
		gwinAttachDial(ghSlider1, 0, 0);
		gwinAttachDial(ghSlider3, 0, 1);
	#endif

	// Make the console visible
	gwinSetVisible(ghConsole, TRUE);
	gwinClear(ghConsole);

    // We want to listen for widget events
	geventListenerInit(&gl);
	gwinAttachListener(&gl);

	// Press the Tab we want visible
	gwinRadioPress(ghTabButtons);

	while(1) {
		// Get an Event
		pe = geventEventWait(&gl, TIME_INFINITE);

		switch(pe->type) {
		case GEVENT_GWIN_BUTTON:
			gwinPrintf(ghConsole, "Button %s\n", gwinGetText(((GEventGWinButton *)pe)->button));
			break;

		case GEVENT_GWIN_SLIDER:
			gwinPrintf(ghConsole, "Slider %s=%d\n", gwinGetText(((GEventGWinSlider *)pe)->slider), ((GEventGWinSlider *)pe)->position);
			break;

		case GEVENT_GWIN_CHECKBOX:
			gwinPrintf(ghConsole, "Checkbox %s=%s\n", gwinGetText(((GEventGWinCheckbox *)pe)->checkbox), ((GEventGWinCheckbox *)pe)->isChecked ? "Checked" : "UnChecked");

			// If it is the Disable All checkbox then do that.
			if (((GEventGWinCheckbox *)pe)->checkbox == ghCheckDisableAll) {
				gwinPrintf(ghConsole, "%s All\n", ((GEventGWinCheckbox *)pe)->isChecked ? "Disable" : "Enable");
				setEnabled(!((GEventGWinCheckbox *)pe)->isChecked);
			}
			break;

		case GEVENT_GWIN_LIST:
			gwinPrintf(ghConsole, "List %s Item %d %s\n", gwinGetText(((GEventGWinList *)pe)->list), ((GEventGWinList *)pe)->item,
					gwinListItemIsSelected(((GEventGWinList *)pe)->list, ((GEventGWinList *)pe)->item) ? "Selected" : "Unselected");
			break;

		case GEVENT_GWIN_RADIO:
			gwinPrintf(ghConsole, "Radio Group %u=%s\n", ((GEventGWinRadio *)pe)->group, gwinGetText(((GEventGWinRadio *)pe)->radio));

			switch(((GEventGWinRadio *)pe)->group) {
			case GROUP_TABS:

				// Set control visibility depending on the tab selected
				setTab(((GEventGWinRadio *)pe)->radio);

				// Do some special animation for Label1 to demonstrate auto width sizing
				if (((GEventGWinRadio *)pe)->radio == ghTabLabels) {
					gwinPrintf(ghConsole, "Change Label Text\n");
					gfxSleepMilliseconds(1000);
					gwinSetText(ghLabel1, "Very Big Label", FALSE);

					gfxSleepMilliseconds(1000);
					gwinSetText(ghLabel1, "Label", FALSE);
				}
				break;

			case GROUP_COLORS:
				{
					const GWidgetStyle	*pstyle;

					gwinPrintf(ghConsole, "Change Color Scheme\n");

					if (((GEventGWinRadio *)pe)->radio == ghRadioYellow)
						pstyle = &YellowWidgetStyle;
					else if (((GEventGWinRadio *)pe)->radio == ghRadioBlack)
						pstyle = &BlackWidgetStyle;
					else
						pstyle = &WhiteWidgetStyle;

					// Clear the screen to the new color - we avoid the console area as it can't redraw itself
					#if GDISP_NEED_CLIP
						gdispUnsetClip();
					#endif
					gdispFillArea(0, 0, ScrWidth, ScrHeight/2, pstyle->background);
					gdispFillArea(0, ScrHeight/2, ScrWidth/2, ScrHeight/2, pstyle->background);

					// Update the style on all controls
					gwinSetDefaultStyle(pstyle, TRUE);
				}
				break;
			}
			break;

		default:
			gwinPrintf(ghConsole, "Unknown %d\n", pe->type);
			break;
		}
	}
	return 0;
}


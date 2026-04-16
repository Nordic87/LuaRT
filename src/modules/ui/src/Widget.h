/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Widget.h | LuaRT Widget object header
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <luart.h>


#include <windows.h>
#include <commctrl.h>


extern int UIWindow;
extern int UIButton;
extern int UIGroup;
extern int UICheck;
extern int UIRadio;
extern int UILabel;
extern int UIEntry;
extern int UIPicture;
extern int UIDate;
extern int UIMenu;
extern int UIMenuItem;
extern int UITab;
extern int UIList;
extern int UICombo;
extern int UITree;
extern int UIEdit;
extern int UIItem;
extern int UIProgressbar;
extern int UIPanel;

//--------------------------------------------------| Widget object definition

typedef int WidgetType;

typedef struct WidgetItem {
	WidgetType	itemtype;
	LRESULT		iconstyle;
	union {
		TVITEMW				*treeitem;
		LVITEMW 			*listitem;
		TCITEMW 			*tabitem;
		COMBOBOXEXITEMW 	*cbitem;
		MENUITEMINFOW		*mi;
	};
} WidgetItem;

typedef struct _Widget {
	luart_type	type;
	void		*handle;
	WidgetType	wtype;
	union {
		HANDLE		tooltip; //------ For standard Widgets, or Parent for Window:showmodal()
		HANDLE		parent;	 //------ For Menu
		int			menu;    //------ For Window
	};
	HANDLE		status;
	int			ref;
	HIMAGELIST	imglist;
	int 		index;
	WidgetItem	item;
	HFONT		font;
	HICON		icon;
	ACCEL		accel;
	HACCEL		accel_table;
	DWORD		style;
	int			cursor;
	HCURSOR  	hcursor;
	BOOL 		autosize;
	HBRUSH		brush;
	COLORREF	color;
	int			align;
	WINDOWPLACEMENT	wp;
	void		*user;
	BOOL		ismain;
	BOOL		isactive;
	BOOL 		allowdrop;
} Widget;

//--- Register a widget type
#define lua_regwidget(L, __typename, methods, mt, has_text, has_font, has_cursor, has_icon, has_tooltip) lua_registerwidget(L, &T##__typename, #__typename, __typename##_constructor, methods, mt, has_text, has_font, has_cursor, has_icon, FALSE, FALSE, has_tooltip, FALSE, FALSE)
#define lua_regwidgetmt(L, __typename, methods, has_text, has_font, has_cursor, has_icon, has_tooltip) lua_registerwidget(L, &T##__typename, #__typename, __typename##_constructor, methods, __typename##_metafields, has_text, has_font, has_cursor, has_icon, FALSE, FALSE, has_tooltip, FALSE, FALSE)
#define lua_regparentwidget(L, __typename, methods, mt, has_text, has_font, has_cursor, has_icon, has_tooltip, TRUE) lua_registerwidget(L, &T##__typename, #__typename, __typename##_constructor, methods, mt, has_text, has_font, has_cursor, has_icon, FALSE, FALSE, has_tooltip, TRUE, FALSE)
#define lua_regparentwidgetmt(L, __typename, methods, has_text, has_font, has_cursor, has_icon, has_tooltip, TRUE) lua_registerwidget(L, &T##__typename, #__typename, __typename##_constructor, methods, __typename##_metafields, has_text, has_font, has_cursor, has_icon, FALSE, FALSE, has_tooltip, TRUE, FALSE)

#define WM_LUAEVENTRESULT 0

#define	WM_LUAMIN			(WM_USER+1000)
#define WM_LUAHIDE 			(WM_LUAMIN)
#define WM_LUASHOW 			(WM_LUAMIN + 1)
#define WM_LUAMOVE 			(WM_LUAMIN + 2)
#define WM_LUARESIZE 		(WM_LUAMIN + 3)
#define WM_LUAHOVER 		(WM_LUAMIN + 4)
#define WM_LUALEAVE 		(WM_LUAMIN + 5)
#define WM_LUACLOSE 		(WM_LUAMIN + 6)
#define WM_LUACLICK 		(WM_LUAMIN + 7)
#define WM_LUADBLCLICK		(WM_LUAMIN + 8)
#define WM_LUACONTEXT 		(WM_LUAMIN + 9)
#define WM_LUACREATE	 	(WM_LUAMIN + 10)
#define WM_LUACARET 		(WM_LUAMIN + 11)
#define WM_LUACHANGE 		(WM_LUAMIN + 12)
#define WM_LUASELECT 		(WM_LUAMIN + 13)
#define WM_LUATRAYCLICK 	(WM_LUAMIN + 14)
#define WM_LUATRAYDBLCLICK	(WM_LUAMIN + 15)
#define WM_LUATRAYCONTEXT 	(WM_LUAMIN + 16)
#define WM_LUATRAYHOVER 	(WM_LUAMIN + 17)
#define WM_LUANOTIFYCLICK 	(WM_LUAMIN + 18)
#define WM_LUAMENU 			(WM_LUAMIN + 19)
#define WM_LUAKEY 			(WM_LUAMIN + 20)
#define WM_LUAMOUSEUP 		(WM_LUAMIN + 21)
#define WM_LUAMOUSEDOWN 	(WM_LUAMIN + 22)
#define WM_LUAMAXIMIZE  	(WM_LUAMIN + 23)
#define WM_LUAMINIMIZE   	(WM_LUAMIN + 24)
#define WM_LUARESTORE    	(WM_LUAMIN + 25)
#define WM_LUADROP    	    (WM_LUAMIN + 26)
#define WM_LUATHEMECHANGE   (WM_LUAMIN + 27)
#define WM_LUADROPITEM      (WM_LUAMIN + 28)

typedef enum {
	onHide			= WM_LUAHIDE,
    onShow 			= WM_LUASHOW, 
    onMove 			= WM_LUAMOVE, 
    onResize		= WM_LUARESIZE,
    onHover 		= WM_LUAHOVER,
    onLeave			= WM_LUALEAVE,
    onClose 		= WM_LUACLOSE,
    onClick 		= WM_LUACLICK,
    onDoubleClick 	= WM_LUADBLCLICK,
    onContext 		= WM_LUACONTEXT,
    onCreate		= WM_LUACREATE,
    onCaret 		= WM_LUACARET,
    onChange 		= WM_LUACHANGE,
    onSelect 		= WM_LUASELECT,
    onTrayClick		= WM_LUATRAYCLICK,
    onTrayDoubleClick=WM_LUATRAYDBLCLICK,
    onTrayContext 	= WM_LUATRAYCONTEXT,
    onTrayHover 	= WM_LUATRAYHOVER,
    onNotificationClick	= WM_LUANOTIFYCLICK,
	onMenu			= WM_LUAMENU,
	onKey			= WM_LUAKEY,
    onMouseUp       = WM_LUAMOUSEUP,
    onMouseDown     = WM_LUAMOUSEDOWN,
    onMaximize      = WM_LUAMAXIMIZE,
    onMinimize      = WM_LUAMINIMIZE,
    onRestore       = WM_LUARESTORE,
    onDrop          = WM_LUADROP,
    onThemeChange   = WM_LUATHEMECHANGE,
    onDropItem   	= WM_LUADROPITEM,
} WidgetEvent;

//---- call event e associated with widget w
#define lua_callevent(w, e) PostMessage((HWND)w->handle, e, 0, 0)
//---- call event e associated with widget w with parameters p and pp
#define lua_paramevent(w, e, p, pp) PostMessage((HWND)w->handle, e, (WPARAM)p, (LPARAM)pp)
//---- call event e associated with widget w with an index value i
#define lua_indexevent(w, e, i) PostMessage((HWND)w->handle, e, (WPARAM)(i), 0)

#ifdef __cplusplus
}
#endif


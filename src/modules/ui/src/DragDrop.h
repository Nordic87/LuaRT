#pragma once
#include <windows.h>
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ListViewDragStateHandle;

ListViewDragStateHandle ListView_CreateDragState(void);
void ListView_DestroyDragState(ListViewDragStateHandle handle);
HWND ListView_SourceDrag(void);
BOOL ListView_BeginDrag(HWND hListView, LPARAM lParam, ListViewDragStateHandle handle);
void ListView_UpdateDrag(HWND hListView, LPARAM lParam, ListViewDragStateHandle handle);
BOOL ListView_EndDrag(HWND hListView, LPARAM lParam, ListViewDragStateHandle handle, WORD* sourceIndex, WORD* targetIndex);
void ListView_CancelDrag(HWND hListView, ListViewDragStateHandle handle);

#ifdef __cplusplus
}
#endif

//
//	DATAOBJECT.CPP
//
//	Implementation of the IDataObject COM interface
//	Modified by Samir Tine 2026
//	By J Brown 2004 
//
//	www.catch22.net
//

#define STRICT

#include <windows.h>
#include <shlobj_core.h>
#include <vector>
#include <algorithm>
#include <string>
#include "ui.h"

CLIPFORMAT CF_PREFERREDDROPEFFECT = static_cast<CLIPFORMAT>(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT));

//
//---------------------------------------------- IDataObject implementation
//
class DataObject : public IDataObject
{
public:
    HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
    ULONG   __stdcall AddRef (void);
    ULONG   __stdcall Release (void);
		

    HRESULT __stdcall GetData				(FORMATETC *pFormatEtc,  STGMEDIUM *pMedium);
    HRESULT __stdcall GetDataHere			(FORMATETC *pFormatEtc,  STGMEDIUM *pMedium);
    HRESULT __stdcall QueryGetData			(FORMATETC *pFormatEtc);
	HRESULT __stdcall GetCanonicalFormatEtc (FORMATETC *pFormatEct,  FORMATETC *pFormatEtcOut);
    HRESULT __stdcall SetData				(FORMATETC *pFormatEtc,  STGMEDIUM *pMedium,  BOOL fRelease);
	HRESULT __stdcall EnumFormatEtc			(DWORD      dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
	HRESULT __stdcall DAdvise				(FORMATETC *pFormatEtc,  DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
	HRESULT __stdcall DUnadvise				(DWORD      dwConnection);
	HRESULT __stdcall EnumDAdvise			(IEnumSTATDATA **ppEnumAdvise);
	
    DataObject(FORMATETC *fmt, STGMEDIUM *stgmed);

	
private:

    LONG	   m_lRefCount;

	FORMATETC 	*fmtetc;
	STGMEDIUM 	*stgm;
};

DataObject::DataObject(FORMATETC *fmt_etc, STGMEDIUM *stgmed) 
{
	m_lRefCount  = 1;

	fmtetc  = fmt_etc;
	stgm  = stgmed;
}

ULONG __stdcall DataObject::AddRef(void) {
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall DataObject::Release(void) {
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if (count == 0)
		delete this;
	return count;
}

HRESULT __stdcall DataObject::QueryInterface(REFIID iid, void **ppvObject) {
    if(iid == IID_IDataObject || iid == IID_IUnknown) {
        AddRef();
        *ppvObject = this;
        return S_OK;
    } else { 
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

HRESULT __stdcall DataObject::GetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium) {	
	pMedium->tymed = 0;
	pMedium->pUnkForRelease = NULL;
	pMedium->hGlobal = NULL;
	if (fmtetc->cfFormat == pFormatEtc->cfFormat && fmtetc->tymed & TYMED_HGLOBAL) {
		pMedium->tymed	 = fmtetc->tymed;
		pMedium->hGlobal = (HGLOBAL)OleDuplicateData(stgm->hGlobal, fmtetc->cfFormat, NULL);
		return S_OK;
	 } else if ((pFormatEtc->tymed & TYMED_HGLOBAL) && (pFormatEtc->cfFormat == CF_PREFERREDDROPEFFECT)) {
        HGLOBAL data   = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, sizeof(DWORD));
        DWORD*  effect = static_cast<DWORD*>(GlobalLock(data));
        (*effect)      = DROPEFFECT_COPY;
        GlobalUnlock(data);
        pMedium->hGlobal = data;
        pMedium->tymed   = TYMED_HGLOBAL;
        return S_OK;
    }
	return DATA_E_FORMATETC;
}

HRESULT __stdcall DataObject::GetDataHere (FORMATETC *pFormatEtc, STGMEDIUM *pMedium) {
	return DATA_E_FORMATETC;
}

HRESULT __stdcall DataObject::QueryGetData (FORMATETC *pFormatEtc) {
	return ((pFormatEtc->cfFormat == fmtetc->cfFormat || pFormatEtc->cfFormat == CF_PREFERREDDROPEFFECT) && pFormatEtc->tymed & TYMED_HGLOBAL) ? S_OK : S_FALSE;
}

HRESULT __stdcall DataObject::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFEOut) {
    if (pFEOut == nullptr)
        return E_INVALIDARG;
    return DATA_S_SAMEFORMATETC;	
}

HRESULT __stdcall DataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
	*ppEnumFormatEtc = nullptr;
    if(dwDirection == DATADIR_GET)
		return SHCreateStdEnumFmtEtc(1, fmtetc, ppEnumFormatEtc);
	else if (dwDirection == DATADIR_SET)
		return E_NOTIMPL;
	return E_INVALIDARG;
}

HRESULT __stdcall DataObject::SetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease) {
	return E_NOTIMPL;
}

HRESULT __stdcall DataObject::DAdvise (FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection) {
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT __stdcall DataObject::DUnadvise (DWORD dwConnection) {
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT __stdcall DataObject::EnumDAdvise (IEnumSTATDATA **ppEnumAdvise) {
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT _CreateDataObject(FORMATETC *fmt_etc, STGMEDIUM *stgmeds, IDataObject **ppDataObject)
{
	if(ppDataObject == 0)
		return E_INVALIDARG;

	*ppDataObject = new DataObject(fmt_etc, stgmeds);

	return (*ppDataObject) ? S_OK : E_OUTOFMEMORY;
}

//
//---------------------------------------------- IDropSource implementation
//
class DropSource : public IDropSource
{
public:
    HRESULT __stdcall QueryInterface	(REFIID iid, void ** ppvObject);
    ULONG   __stdcall AddRef			(void);
    ULONG   __stdcall Release			(void);
		
    HRESULT __stdcall QueryContinueDrag	(BOOL fEscapePressed, DWORD grfKeyState);
	HRESULT __stdcall GiveFeedback		(DWORD dwEffect);
	
    DropSource();
	
private:
    LONG	   m_lRefCount;
};

DropSource::DropSource()  {
	m_lRefCount = 1;
}

ULONG __stdcall DropSource::AddRef(void) {
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall DropSource::Release(void) {
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
		delete this;	
	return count;
}

HRESULT __stdcall DropSource::QueryInterface(REFIID iid, void **ppvObject) {
    if(iid == IID_IDropSource || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    } else {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

HRESULT __stdcall DropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) {
	if(fEscapePressed == TRUE)
		return DRAGDROP_S_CANCEL;	

	if((grfKeyState & MK_LBUTTON) == 0)
		return DRAGDROP_S_DROP;

	return NOERROR;
}

HRESULT __stdcall DropSource::GiveFeedback(DWORD dwEffect) {
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

HRESULT _CreateDropSource(IDropSource **ppDropSource) {
	if(ppDropSource == 0)
		return E_INVALIDARG;

	*ppDropSource = new DropSource();

	return (*ppDropSource) ? S_OK : E_OUTOFMEMORY;
}

//
//---------------------------------------------- IDropTarget implementation
//

class DropTarget : public IDropTarget
{
public:
    HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
    ULONG __stdcall AddRef (void);
    ULONG __stdcall Release (void);
    HRESULT __stdcall DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    HRESULT __stdcall DragLeave(void);
    HRESULT __stdcall Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

    DropTarget(HWND h);

private:
    long m_lRefCount;
    HWND hwnd;
};

DropTarget::DropTarget(HWND h) : IDropTarget() {
	hwnd = h;
	m_lRefCount = 1;
}

ULONG __stdcall DropTarget::AddRef(void) {
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall DropTarget::Release(void) {
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
		delete this;	
	return count;
}

HRESULT __stdcall DropTarget::QueryInterface(REFIID iid, void **ppvObject) {
    if(iid == IID_IDropTarget || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    } else {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

HRESULT __stdcall DropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	DragOver(grfKeyState, pt, pdwEffect);	
	SetFocus(hwnd);
    return S_OK;
}

HRESULT __stdcall DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(grfKeyState & MK_CONTROL)
        *pdwEffect = *pdwEffect & DROPEFFECT_COPY;
    else if(grfKeyState & MK_SHIFT)
        *pdwEffect = *pdwEffect & DROPEFFECT_MOVE;
	else  {
        if(*pdwEffect & DROPEFFECT_COPY) *pdwEffect = DROPEFFECT_COPY;
        if(*pdwEffect & DROPEFFECT_MOVE) *pdwEffect = DROPEFFECT_MOVE;
    }
    return S_OK;
}

HRESULT __stdcall DropTarget::DragLeave(void)
{
    return S_OK;
}

HRESULT __stdcall DropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	FORMATETC fmt = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgm;
	
	if (pDataObject->QueryGetData(&fmt) != S_OK) {
		fmt.cfFormat = CF_UNICODETEXT;
		if (pDataObject->QueryGetData(&fmt) != S_OK) {
			fmt.cfFormat = CF_TEXT;
			if (pDataObject->QueryGetData(&fmt) != S_OK)
				fmt.cfFormat = 0;															
		}
	}
	if (fmt.cfFormat && SUCCEEDED(pDataObject->GetData(&fmt, &stgm))) {
		size_t size = GlobalSize(stgm.hGlobal);
		char *data = (char *)calloc(1, size+1);
		memcpy(data+1, GlobalLock(stgm.hGlobal), size);
		*data = fmt.cfFormat;
		GlobalUnlock(stgm.hGlobal);
		ReleaseStgMedium(&stgm);
		PostMessage(hwnd, WM_LUADROP, (WPARAM)data, (LPARAM)size);
	}
    return DragOver(grfKeyState, pt, pdwEffect);
}

HRESULT _CreateDropTarget(IDropTarget **ppDropTarget, HWND h) {
	if(ppDropTarget == 0)
		return E_INVALIDARG;

	*ppDropTarget = new DropTarget(h);

	return (*ppDropTarget != NULL) ? S_OK : E_OUTOFMEMORY;
}

//--------------------- C interface
extern "C" {
	HRESULT CreateDropTarget(IDropTarget **ppDropTarget, HWND h) {
		return _CreateDropTarget(ppDropTarget, h);
	}
	HRESULT CreateDropSource(IDropSource **ppDropSource) {
		return _CreateDropSource(ppDropSource);
	}
	HRESULT CreateDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmeds, IDataObject **ppDataObject) {
		return _CreateDataObject(fmtetc, stgmeds, ppDataObject);
	}
}

//
//---------------------------------------------- ListView DragDrop helper functions
//

#include "DragDrop.h"

struct ListViewDragState {
    BOOL isDragging = false;
    HIMAGELIST hDragImage = nullptr;
    POINT ptStart = {0, 0};
    int selectedIndex = -1;
    HWND sourceListView = nullptr;
    int lastDropHighlight = -1;
};

static struct {
    HWND source = nullptr;
    int itemIndex = -1;
    char text[256] = {};
} g_dragTransfer;

extern "C" {

ListViewDragStateHandle ListView_CreateDragState(void) {
    return new ListViewDragState();
}

void ListView_DestroyDragState(ListViewDragStateHandle handle) {
    delete static_cast<ListViewDragState*>(handle);
}

HWND ListView_SourceDrag(void) {
    return g_dragTransfer.source;
}

BOOL ListView_BeginDrag(HWND hListView, LPARAM lParam, ListViewDragStateHandle handle) {
    auto* state = static_cast<ListViewDragState*>(handle);
    POINT pt = { LOWORD(lParam), HIWORD(lParam) };

    LVHITTESTINFO hit = { 0 };
	hit.pt = pt;
    if (ListView_HitTest(hListView, &hit) < 0)
        return FALSE;

    state->ptStart = pt;
    state->selectedIndex = hit.iItem;
    state->sourceListView = hListView;
    state->lastDropHighlight = -1;

    if (state->selectedIndex >= 0) {
        ListView_GetItemText(hListView, state->selectedIndex, 0, g_dragTransfer.text, ARRAYSIZE(g_dragTransfer.text));
        g_dragTransfer.source = hListView;
        g_dragTransfer.itemIndex = state->selectedIndex;
        return TRUE;
    }

    return FALSE;
}

void ListView_UpdateDrag(HWND hListView, LPARAM lParam, ListViewDragStateHandle handle) {
    auto* state = static_cast<ListViewDragState*>(handle);
    POINT pt = { LOWORD(lParam), HIWORD(lParam) };
    
    if (!state->isDragging && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
		int dx = abs(pt.x - state->ptStart.x);
        int dy = abs(pt.y - state->ptStart.y);
        if ((dx > 4 || dy > 4) && state->selectedIndex >= 0) {
			state->hDragImage = ListView_CreateDragImage(hListView, state->selectedIndex, &state->ptStart);
            ImageList_BeginDrag(state->hDragImage, 0, 0, 0);
            ImageList_DragEnter(hListView, pt.x, pt.y);
            SetCapture(hListView);
            state->isDragging = true;
        }
    }

    if (state->isDragging) {
        ImageList_DragMove(LOWORD(lParam), HIWORD(lParam));

        LVHITTESTINFO hit = { 0 };
		hit.pt = pt;

        int dropIndex = ListView_HitTest(hListView, &hit);

        if (dropIndex != state->lastDropHighlight) {
            if (state->lastDropHighlight >= 0) {
                ListView_SetItemState(hListView, state->lastDropHighlight, 0, LVIS_DROPHILITED);
            }
            if (dropIndex >= 0) {
                ListView_SetItemState(hListView, dropIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);
            }
            state->lastDropHighlight = dropIndex;
        }
    }
}

BOOL ListView_EndDrag(HWND hListView, LPARAM lParam, ListViewDragStateHandle handle, WORD* sourceIndex, WORD* targetIndex) {
    auto* state = static_cast<ListViewDragState*>(handle);
    if (!state->isDragging)
        return FALSE;

    POINT pt = { LOWORD(lParam), HIWORD(lParam) };
    ScreenToClient(hListView, &pt);

    LVHITTESTINFO hit = { 0 };
	hit.pt = pt;
    int dropIndex = ListView_HitTest(hListView, &hit);
    if (dropIndex < 0)
        dropIndex = ListView_GetItemCount(hListView);

    ImageList_EndDrag();
    ImageList_Destroy(state->hDragImage);
    ReleaseCapture();
    state->isDragging = false;

    if (state->lastDropHighlight >= 0) {
        ListView_SetItemState(hListView, state->lastDropHighlight, 0, LVIS_DROPHILITED);
        state->lastDropHighlight = -1;
    }

    if (sourceIndex)     *sourceIndex = g_dragTransfer.itemIndex;
    if (targetIndex)     *targetIndex = dropIndex;

    BOOL valid = (g_dragTransfer.itemIndex >= 0 && g_dragTransfer.text[0] != L'\0');
    g_dragTransfer.source = nullptr;
    g_dragTransfer.itemIndex = -1;
    g_dragTransfer.text[0] = L'\0';
    state->selectedIndex = -1;

    return valid;
}

void ListView_CancelDrag(HWND hListView, ListViewDragStateHandle handle) {
    auto* state = static_cast<ListViewDragState*>(handle);
    if (!state->isDragging)
        return;

    ImageList_EndDrag();
    ImageList_Destroy(state->hDragImage);
    ReleaseCapture();
    state->isDragging = false;

    if (state->lastDropHighlight >= 0) {
        ListView_SetItemState(hListView, state->lastDropHighlight, 0, LVIS_DROPHILITED);
        state->lastDropHighlight = -1;
    }

    g_dragTransfer.source = nullptr;
    g_dragTransfer.itemIndex = -1;
    g_dragTransfer.text[0] = L'\0';
    state->selectedIndex = -1;
}

}
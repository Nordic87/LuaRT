/*
 | Webview for LuaRT - HTML/JS/CSS render Widget
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE
 |--------------------------------------------------------
 | handler.h | Webview2 c++ interface class wrapper header
*/

#pragma once
#include <functional>
#include <windows.h>
#include <Zip.h>
#include "WebView2.h"
#include <string>
#include <vector>
#include <memory>
#include "../../ui/src/ui.h"

extern UIInterface *ui;

extern UINT onReady, onMessage, onLoaded, onFullscreen;
extern wchar_t *ErrorFromHR(HRESULT errorCode);
wchar_t *toUTF16(const char *s);

typedef struct {
	int 	 ref;
	bool 	 done;
	wchar_t *result;
	EventRegistrationToken token;
} EventTask;

template <typename T>
static HRESULT STDMETHODCALLTYPE Null_QueryInterface(T* This, REFIID riid, void** ppvObject) {
	return E_NOINTERFACE;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_AddRef(T* This) {
	return 1;
}

template <typename T>
static ULONG STDMETHODCALLTYPE Null_Release(T* This) {
	return 1;
}

class WebviewHandler
{
public:
	WebviewHandler(HWND h, const char *URL, const char *args);
	virtual ~WebviewHandler();

	WebviewHandler(const WebviewHandler&) = delete;
	WebviewHandler(WebviewHandler&&) = delete;
	WebviewHandler& operator=(const WebviewHandler&) = delete;
	WebviewHandler& operator=(WebviewHandler&&) = delete;

	void Resize();

	HWND hwnd; 
	zip_t *archive = NULL;
	std::wstring url;

	ICoreWebView2Settings3* settings = nullptr;
	ICoreWebView2* webview2 = nullptr;
	ICoreWebView2_17* webview3 = nullptr;
	ICoreWebView2Controller* controller = nullptr;
};

class TaskCallback : public ICoreWebView2ExecuteScriptCompletedHandler,
					 public	ICoreWebView2CallDevToolsProtocolMethodCompletedHandler,
					 public ICoreWebView2PrintToPdfCompletedHandler,
					 public ICoreWebView2PrintCompletedHandler
{
public:
	TaskCallback() {};
	~TaskCallback() { 
		if (this->result)
			free(result);
	};

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) noexcept override {
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(ICoreWebView2ExecuteScriptCompletedHandler) ||
            riid == __uuidof(ICoreWebView2CallDevToolsProtocolMethodCompletedHandler)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() noexcept override { return 1; }
    ULONG STDMETHODCALLTYPE Release() noexcept override { return 1; }

	HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr, LPCWSTR resultObjectAsJson) noexcept override {
		if (SUCCEEDED(hr))
			this->result = _wcsdup(resultObjectAsJson);
		this->hr = hr;
		this->done = true;
		return S_OK;
	};

	HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, BOOL isSuccessful) noexcept override {
		if (isSuccessful)
			this->hr = S_OK;
		else {
			wchar_t *err = ErrorFromHR(errorCode);
			this->result = _wcsdup(err);
			this->hr = errorCode;
			LocalFree(err);
		}
		this->done = true;
		return S_OK;
	};

	HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, COREWEBVIEW2_PRINT_STATUS result) noexcept override {
		switch (result) {
			case COREWEBVIEW2_PRINT_STATUS_PRINTER_UNAVAILABLE: this->result = _wcsdup(L"Printer is unavailable"); break;
			case COREWEBVIEW2_PRINT_STATUS_OTHER_ERROR: {
				if (SUCCEEDED(errorCode))
					this->result = _wcsdup(L"Print operation failed");
				else {
					wchar_t *err = ErrorFromHR(errorCode);
					this->result = _wcsdup(err);
					LocalFree(err);
				}
			}
		}
		this->hr = this->result ? E_FAIL : S_OK;
		this->done = true;
		return S_OK;
	};
    
	wchar_t *result = NULL;
	bool done = false;
	HRESULT hr;
};
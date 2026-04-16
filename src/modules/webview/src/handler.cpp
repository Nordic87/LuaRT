/*
 | Webview for LuaRT - HTML/JS/CSS render Widget
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE
 |-------------------------------------------------
 | handler.cpp | Webview2 c++ interface class wrapper
*/

#include "handler.h"
#include <shlwapi.h>
#include <string>
#include <Zip.h>

#include <wrl.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <filesystem>

using namespace Microsoft::WRL;

wchar_t *toUTF16(const char *s) {
	DWORD size = MultiByteToWideChar(CP_UTF8, 0, s, -1, 0, 0);
	WCHAR *ws = (WCHAR *)GlobalAlloc(GMEM_FIXED, sizeof(WCHAR) * size);
	if (ws == NULL)
		return NULL;
	MultiByteToWideChar(CP_UTF8, 0, s, -1, ws, size);
	return ws;
}

static char *wchar_toutf8(const wchar_t *str) {
	char *buff;
	int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	buff = (char *)malloc(size);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, buff, size, NULL, NULL);
	return buff;
}

WebviewHandler::~WebviewHandler() {
	ICoreWebView2Environment *env = nullptr;
	this->controller->Release();

	if (this->webview3) {
		this->webview3->get_Environment(&env);
		this->webview3->Release();
	}
	if (this->webview2) {
		this->webview2->RemoveWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
		this->webview2->Release();
	}
	if (this->controller)
	if (env)
		env->Release();
}

WebviewHandler::WebviewHandler(HWND h, const char *URL, const char *args) {
	std::wstring dataPath;
	wchar_t temp[1024];
	wchar_t *wstr;
	ComPtr<ICoreWebView2EnvironmentOptions> env = Make<CoreWebView2EnvironmentOptions>();

	GetEnvironmentVariableW(L"AppData", temp, _countof(temp));
	dataPath = temp;
	GetModuleFileNameW(nullptr, temp, _countof(temp));
	wchar_t *szExeName = wcsrchr(temp, L'\\');
	szExeName = szExeName ? szExeName + 1 : temp;
	dataPath += L"\\";
	dataPath += szExeName;
	this->hwnd = h;
	wstr = toUTF16(URL);
	this->url = wstr;
	GlobalFree(wstr);
	if (args) {
		wstr = toUTF16(args);
		env->put_AdditionalBrowserArguments(wstr);
		GlobalFree(wstr);
	}
	CreateCoreWebView2EnvironmentWithOptions(nullptr, dataPath.substr(0, dataPath.find_last_of(L'.')).c_str(), args ? env.Get() : nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [h, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            env->CreateCoreWebView2Controller(h, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [h, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
					if (FAILED(result))
						return E_FAIL;
					EventRegistrationToken uritoken, msgtoken, navigtoken, fullscreentoken;
					ICoreWebView2Profile *profile; 
					this->controller = controller;
					this->controller->AddRef();
					this->controller->get_CoreWebView2(&this->webview2);
					this->webview2->AddRef();
					this->webview2->QueryInterface(IID_ICoreWebView2_17, (void**)&this->webview3);
					this->webview2->AddWebResourceRequestedFilter(NULL, COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
					if (SUCCEEDED(this->webview3->get_Profile(&profile))) {
						BOOL isDark;
						ui->lua_uigetinfo(NULL, &isDark);
						profile->put_PreferredColorScheme(isDark ? COREWEBVIEW2_PREFERRED_COLOR_SCHEME_DARK : COREWEBVIEW2_PREFERRED_COLOR_SCHEME_LIGHT);
						profile->Release();						
					}
					//--- onWebMessageReceived event
					this->webview2->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
						[this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
							LPWSTR webMessage;
							if (SUCCEEDED(args->get_WebMessageAsJson(&webMessage)))
								PostMessage(this->hwnd, onMessage, (WPARAM)_wcsdup(webMessage), 0);
							return S_OK;
					}).Get(), &msgtoken);
					//--- onNavigationCompleted event
					this->webview2->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
						[this](ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args) -> HRESULT {
							BOOL result;
							ICoreWebView2NavigationCompletedEventArgs2 *args2;
							int status;
							
							args->QueryInterface(IID_ICoreWebView2NavigationCompletedEventArgs2, (void**)&args2);
							args->get_IsSuccess(&result);
							args2->get_HttpStatusCode(&status);
							args2->Release();
							PostMessage(this->hwnd, onLoaded, (WPARAM)result, (LPARAM)status);
							return S_OK;
					}).Get(), &navigtoken);
						//--- onWebResourceRequested event
					this->webview2->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>(
						[this](ICoreWebView2 *sender, ICoreWebView2WebResourceRequestedEventArgs *args) -> HRESULT {
							ICoreWebView2WebResourceRequest *request;
							HRESULT result = E_FAIL;
							if (SUCCEEDED(args->get_Request(&request))) {
								LPWSTR wuri;
								if (SUCCEEDED(request->get_Uri(&wuri))) {
									char *uri = wchar_toutf8(wuri);
									char *entry = strstr(uri, "file:///") ? uri+8 : NULL;
									if (entry && this->archive) {
										if (zip_entry_open(this->archive, entry) == 0) {
											void *buffer;
											size_t buffsize;
											IStream *stream;
											LPWSTR mime = NULL;
											wchar_t content[128] = L"Content-Type: text/html";
											ICoreWebView2Environment *env;
											ICoreWebView2WebResourceResponse *response;
											zip_entry_read(this->archive, &buffer, &buffsize);
											zip_entry_close(this->archive);
											stream = SHCreateMemStream((const BYTE*)buffer, buffsize);   
											this->webview3->get_Environment(&env);
											if (SUCCEEDED(FindMimeFromData(NULL, wuri, NULL, 0, NULL, FMFD_URLASFILENAME, &mime, 0))) {
												_snwprintf(content, 128, L"Content-Type: %s", mime);
												CoTaskMemFree(mime);
											}
											env->CreateWebResourceResponse(stream, 200, L"OK", content, &response);
											args->put_Response(response);
											stream->Release();
											result = S_OK;
											free(buffer);
										}
									} else {
										if (entry && std::filesystem::exists(entry)) {
											void *buffer;
											size_t buffsize;
											IStream *stream;
											LPWSTR mime = NULL;
											wchar_t content[128] = L"Content-Type: text/html";
											ICoreWebView2Environment *env;
											ICoreWebView2WebResourceResponse *response;
											if (SUCCEEDED(SHCreateStreamOnFileA(entry, STGM_READ, &stream))) {   
												this->webview3->get_Environment(&env);
												if (SUCCEEDED(FindMimeFromData(NULL, wuri, NULL, 0, NULL, FMFD_URLASFILENAME, &mime, 0))) {
													_snwprintf(content, 128, L"Content-Type: %s", mime);
													CoTaskMemFree(mime);
												}
											}
											env->CreateWebResourceResponse(stream, 200, L"OK", content, &response);
											args->put_Response(response);
											stream->Release();
											result = S_OK;
										}  		
									}
									free(uri);
								}
								GlobalFree(wuri);
								request->Release();
							}
							return result;
						}).Get(), &uritoken);	
						//--- onContainsFullScreenElementChangedEvent event
						this->webview2->add_ContainsFullScreenElementChanged(Callback<ICoreWebView2ContainsFullScreenElementChangedEventHandler>(
							[this](ICoreWebView2 *sender, IUnknown *args) -> HRESULT {
								BOOL isFullscreen;
							
								if (SUCCEEDED(sender->get_ContainsFullScreenElement(&isFullscreen))) {
									PostMessage(this->hwnd, onFullscreen, (WPARAM)isFullscreen, 0);
									return S_OK;
								}
								return E_FAIL;
						}).Get(), &fullscreentoken);	
					this->webview2->Navigate(this->url.c_str());
					this->webview2->get_Settings((ICoreWebView2Settings**)&this->settings);	
					PostMessage(this->hwnd, onReady, 0, 0);
					this->Resize();
					return S_OK;
                }).Get());
            return S_OK;
        }).Get());
} 

void WebviewHandler::Resize() {
	if (!controller)
		return;

	RECT bounds;
	GetClientRect(this->hwnd, &bounds);
	controller->put_Bounds(bounds);
}
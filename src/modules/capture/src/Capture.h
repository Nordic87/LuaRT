#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d2d1.h>
#include <winrt/base.h>
#include <wincodec.h>
#include <winrt/Windows.Media.MediaProperties.h>
#include <winrt/Windows.Media.Capture.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Capture.Frames.h>
#include <winrt/Windows.Media.MediaProperties.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Media.Devices.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <luart.h>

enum class PreviewMode {
    AspectStretch,
    Aspect4x3,
    Aspect16x9
};

class Capture {
public:
    // Constructors
    Capture(HWND hwnd, wchar_t *video, wchar_t *audio);
    Capture(wchar_t *video, wchar_t *audio);
    ~Capture();

    // Initialization
    void InitializeAsync(
        const std::wstring& videoDeviceName,
        const std::wstring& audioDeviceName);

    // Check if camera is ready (has received at least one frame)    
    bool IsReady() const {
        return m_lastBitmap != nullptr; 
    }  
    // Recording
    void StartRecordingAsync(
        const std::wstring& filePath,
        winrt::Windows::Media::MediaProperties::VideoEncodingQuality quality =
            winrt::Windows::Media::MediaProperties::VideoEncodingQuality::HD720p,
        int audioChannels = 2,
        uint32_t audioSampleRate = 44100,
        uint32_t audioBitrate = 128000,
        uint32_t videoBitrate = 5000000);

    void StopRecordingAsync();

    // Snapshot
    void SnapshotAsync(const std::wstring& filePath);
    
    // Preview options
    void SetPreviewMode(PreviewMode mode);
    bool IsRecording() const;
    PreviewMode m_previewMode{ PreviewMode::AspectStretch };

    // Device enumeration
    static std::vector<std::wstring> GetVideoDeviceNames();
    static std::vector<std::wstring> GetAudioDeviceNames();

    // Resize event
    void OnWindowResize(UINT width, UINT height);

private:
    void VerifyWindowsVersion();
    void InitD3D(HWND hwnd);
    void RecreateDevice();
    void OnFrameArrived(winrt::Windows::Media::Capture::Frames::MediaFrameReader const& reader);
    D2D1_RECT_F GetPreviewRect(D2D1_SIZE_F targetSize, PreviewMode mode);

    // Members
    HWND m_hwnd{};
    bool m_recording{ false };
    CRITICAL_SECTION m_resizeLock;
    winrt::com_ptr<IWICImagingFactory> m_wicFactory;
    winrt::Windows::Graphics::Imaging::SoftwareBitmap m_lastBitmap;
    bool m_frameReaderActive;
    std::chrono::steady_clock::time_point m_lastFrameTime;
    UINT m_lastValidWidth;
    UINT m_lastValidHeight;

    winrt::com_ptr<ID3D11Device> m_d3dDevice;
    winrt::com_ptr<ID3D11DeviceContext> m_d3dContext;
    winrt::com_ptr<IDXGISwapChain1> m_swapChain;
    winrt::com_ptr<ID2D1Factory> m_d2dFactory;
    winrt::com_ptr<ID2D1RenderTarget> m_renderTarget;

    winrt::Windows::Media::Capture::MediaCapture m_mediaCapture{ nullptr };
    winrt::Windows::Media::Capture::Frames::MediaFrameReader m_frameReader{ nullptr };
    winrt::Windows::Storage::StorageFile m_outputFile{ nullptr };
};
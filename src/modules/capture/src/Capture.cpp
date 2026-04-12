#include "Capture.h"

#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Media.Capture.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d2d1.h>
#include <stdexcept>
#include <cassert>
#include <filesystem>

using namespace winrt;
using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;
using namespace Windows::Media::Devices;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Devices::Enumeration;

static const float ratio[] = { 1, 4.0f / 3.0f, 16.0f / 9.0f };

std::wstring ResolveFilePathToAbsolute(std::wstring const& filePath)
{
    std::filesystem::path path(filePath);

    if (path.is_absolute())
        return path.wstring();

    std::filesystem::path combined = std::filesystem::current_path() / path;
    return combined.wstring();
}


Capture::Capture(HWND hwnd, wchar_t *video, wchar_t *audio)
    : m_hwnd(hwnd), m_lastBitmap(nullptr) {
    VerifyWindowsVersion();
    InitD3D(hwnd);
    InitializeAsync(video ? video : L"", audio ? audio : L"");
    RECT rc{};
    if (GetClientRect(m_hwnd, &rc) && rc.right > rc.left && rc.bottom > rc.top) {
        m_lastValidWidth = rc.right - rc.left;
        m_lastValidHeight = rc.bottom - rc.top;
    }
}

Capture::Capture(wchar_t *video, wchar_t *audio)
: m_hwnd(nullptr), m_lastBitmap(nullptr) {
    VerifyWindowsVersion();
    InitializeAsync(video ? video : L"", audio ? audio : L"");
 }

Capture::~Capture() {
    if (m_recording) {
        try {
            StopRecordingAsync();
        } catch (...) {}
    }
    if (m_frameReader) {
        try {
            m_frameReader.StopAsync().get();
        } catch (...) {}
    }
    if (m_mediaCapture) {
        try {
            m_mediaCapture.Close();
        } catch (...) {}
    }
    if (m_wicFactory) m_wicFactory->Release();
    
    DeleteCriticalSection(&m_resizeLock);
}

bool Capture::IsRecording() const {
    return m_recording;
}

void Capture::SetPreviewMode(PreviewMode mode) {
    m_previewMode = mode;
}

void Capture::InitializeAsync(
    const std::wstring& videoDeviceName,
    const std::wstring& audioDeviceName) {
    DeviceInformation videoDev{ nullptr }, audioDev{ nullptr };

    InitializeCriticalSection(&m_resizeLock);
    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_wicFactory.put()));

    auto videoList = DeviceInformation::FindAllAsync(
        MediaDevice::GetVideoCaptureSelector()).get();
    if (videoDeviceName.empty())
        for (auto&& dev : videoList) {
            if (dev.Name() == videoDeviceName) {
                videoDev = dev;
                break;
            }
        }

    if (videoList == nullptr || videoList.Size() == 0)
        throw std::runtime_error("No video device found.");

    auto audioList = DeviceInformation::FindAllAsync(
        MediaDevice::GetAudioCaptureSelector()).get();
    if (audioDeviceName.empty())
        for (auto&& dev : audioList) {
            if (dev.Name() == audioDeviceName) {
                audioDev = dev;
                break;
            }
        }
    if (audioList == nullptr || audioList.Size() == 0)
        throw std::runtime_error("No audio device found.");

    m_mediaCapture = MediaCapture();
    MediaCaptureInitializationSettings settings;
    settings.VideoDeviceId(videoDev ? videoDev.Id() : L"");
    settings.AudioDeviceId(audioDev ? audioDev.Id() : L"");
    settings.StreamingCaptureMode(StreamingCaptureMode::AudioAndVideo);  
    settings.MemoryPreference(MediaCaptureMemoryPreference::Cpu);

    m_mediaCapture.InitializeAsync(settings).get();

    // Frame source
    for (auto&& pair : m_mediaCapture.FrameSources()) {
        auto src = pair.Value();
        if (src.Info().SourceKind() == MediaFrameSourceKind::Color) {
            m_frameReader = m_mediaCapture.CreateFrameReaderAsync(src).get();
            break;
        }
    }

    // Frame callback
    m_frameReader.FrameArrived([this](auto const& reader, auto const&) {
        OnFrameArrived(reader);
    });

    auto status = m_frameReader.StartAsync().get();
    assert(status == MediaFrameReaderStartStatus::Success);
}

void Capture::StopRecordingAsync() {
    if (m_mediaCapture)
        m_mediaCapture.StopRecordAsync().get();
    m_recording = false;
}

void Capture::StartRecordingAsync(
    const std::wstring& filePath,
    VideoEncodingQuality quality,
    int audioChannels,
    uint32_t audioSampleRate,
    uint32_t audioBitrate,
    uint32_t videoBitrate) {

    std::filesystem::path fullPath = ResolveFilePathToAbsolute(filePath);
    std::filesystem::path folderPath = fullPath.parent_path();
    std::wstring fileName = fullPath.filename().wstring();
    winrt::hstring folderH = winrt::hstring(folderPath.wstring());

    winrt::Windows::Storage::StorageFolder folder = winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(folderH).get();
    m_outputFile = folder.CreateFileAsync(
        winrt::hstring(fileName),
        winrt::Windows::Storage::CreationCollisionOption::ReplaceExisting
    ).get();
    MediaEncodingProfile profile = nullptr;
    if (filePath.ends_with(L".mp4")) {
        profile = MediaEncodingProfile::CreateMp4(quality);
    } else if (filePath.ends_with(L".wmv")) {
        profile = MediaEncodingProfile::CreateWmv(quality);
    } else if (filePath.ends_with(L".avi")) {
        profile = MediaEncodingProfile::CreateAvi(quality);
    } else {
        throw std::runtime_error("File format not available.");
    }

    // Audio
    auto audio = profile.Audio();
    audio.ChannelCount(audioChannels);
    audio.SampleRate(audioSampleRate);
    audio.Bitrate(audioBitrate);
    audio.BitsPerSample(16);

    // Vidéo
    auto video = profile.Video();
    video.Bitrate(videoBitrate);

    m_mediaCapture.StartRecordToStorageFileAsync(profile, m_outputFile).get();
    m_recording = true;
}

void Capture::SnapshotAsync(const std::wstring& filePath) {
    // 1. Check frame
    if (!m_lastBitmap)
        throw winrt::hresult_error(E_FAIL, L"No frame received.");

    // 2. Get raw bytes from WinRT SoftwareBitmap (Format: BGRA8 Premultiplied)
    auto converted = SoftwareBitmap::Convert(m_lastBitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);
    
    int width = converted.PixelWidth();
    int height = converted.PixelHeight();
    int stride = width * 4;
    int bufferSize = stride * height;

    Windows::Storage::Streams::Buffer buffer(static_cast<uint32_t>(bufferSize));
    converted.CopyToBuffer(buffer);
    auto rawBytes = winrt::array_view<uint8_t>(buffer.data(), buffer.data() + bufferSize);

    // 3. Create a temporary WIC Bitmap from memory (32bpp)
    winrt::com_ptr<IWICBitmap> inputWicBitmap;
    winrt::check_hresult(m_wicFactory->CreateBitmapFromMemory(
        width, height, 
        GUID_WICPixelFormat32bppPBGRA, // Must match SoftwareBitmap
        stride, bufferSize, rawBytes.data(), 
        inputWicBitmap.put()));

    // 4. Prepare File Stream
    winrt::com_ptr<IWICStream> stream;
    winrt::check_hresult(m_wicFactory->CreateStream(stream.put()));
    std::filesystem::path fullPath = ResolveFilePathToAbsolute(filePath);
    winrt::check_hresult(stream->InitializeFromFilename(fullPath.c_str(), GENERIC_WRITE));

    // 5. Determine Target Format (Fixes JPG issues)
    GUID containerFormat = GUID_ContainerFormatPng;
    WICPixelFormatGUID targetPixelFormat = GUID_WICPixelFormat32bppPBGRA; 

    if (filePath.ends_with(L".jpg") || filePath.ends_with(L".jpeg")) {
        containerFormat = GUID_ContainerFormatJpeg;
        targetPixelFormat = GUID_WICPixelFormat24bppBGR; // JPG must be 24-bit (No Alpha)
    } else if (filePath.ends_with(L".bmp")) {
        containerFormat = GUID_ContainerFormatBmp;
        targetPixelFormat = GUID_WICPixelFormat24bppBGR; // BMP is safest as 24-bit
    } else if (filePath.ends_with(L".png")) {
        containerFormat = GUID_ContainerFormatPng;
        targetPixelFormat = GUID_WICPixelFormat32bppPBGRA; // PNG supports Alpha
    } else {
        throw std::runtime_error("Unsupported image format.");
    }

    // 6. Initialize Encoder
    winrt::com_ptr<IWICBitmapEncoder> encoder;
    winrt::check_hresult(m_wicFactory->CreateEncoder(containerFormat, nullptr, encoder.put()));
    winrt::check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderNoCache));

    winrt::com_ptr<IWICBitmapFrameEncode> frameEncode;
    winrt::com_ptr<IPropertyBag2> propertyBag;
    winrt::check_hresult(encoder->CreateNewFrame(frameEncode.put(), propertyBag.put()));
    winrt::check_hresult(frameEncode->Initialize(propertyBag.get()));
    winrt::check_hresult(frameEncode->SetSize(width, height));
    
    // 7. Use a Converter to safely convert pixel formats (32bpp -> 24bpp for JPG)
    winrt::com_ptr<IWICFormatConverter> converter;
    winrt::check_hresult(m_wicFactory->CreateFormatConverter(converter.put()));
    
    winrt::check_hresult(converter->Initialize(
        inputWicBitmap.get(),           
        targetPixelFormat,              
        WICBitmapDitherTypeNone, 
        nullptr, 
        0.f, 
        WICBitmapPaletteTypeMedianCut
    ));

    // Set the format to the encoder
    WICPixelFormatGUID encodeFormat = targetPixelFormat;
    winrt::check_hresult(frameEncode->SetPixelFormat(&encodeFormat));

    // 8. Write the CONVERTED source
    // This handles the stride calculation automatically, preventing memory corruption
    winrt::check_hresult(frameEncode->WriteSource(converter.get(), nullptr));

    winrt::check_hresult(frameEncode->Commit());
    winrt::check_hresult(encoder->Commit());
}

// void Capture::SnapshotAsync(const std::wstring& filePath) {
//     using namespace winrt;
//     using namespace Windows::Graphics::Imaging;
//     using namespace Windows::Storage;
//     using namespace Windows::Storage::Streams;

//     if (!m_lastBitmap)
//         throw hresult_error(E_FAIL, L"No frame received within timeout.");

//     auto converted = SoftwareBitmap::Convert(m_lastBitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);
    
//     std::filesystem::path fullPath = ResolveFilePathToAbsolute(filePath);
//     std::filesystem::path folderPath = fullPath.parent_path();
//     std::wstring fileName = fullPath.filename().wstring();
//     winrt::hstring folderH = winrt::hstring(folderPath.wstring());
//     winrt::Windows::Storage::StorageFolder folder = winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(folderH).get();
//     auto file = folder.CreateFileAsync(
//         winrt::hstring(fileName),
//         winrt::Windows::Storage::CreationCollisionOption::ReplaceExisting
//     ).get();
//     auto stream = file.OpenAsync(FileAccessMode::ReadWrite).get();

//     BitmapEncoder encoder = nullptr;

//     if (filePath.ends_with(L".png")) {
//         encoder = BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), stream).get();
//     } else if (filePath.ends_with(L".jpg") || filePath.ends_with(L".jpeg")) {
//         encoder = BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId(), stream).get();
//     } else if (filePath.ends_with(L".bmp")) {
//         encoder = BitmapEncoder::CreateAsync(BitmapEncoder::BmpEncoderId(), stream).get();
//     } else {
//         throw std::runtime_error("Unsupported image format. Use .png, .jpg, or .bmp.");
//     }

//     encoder.SetSoftwareBitmap(converted);
//     encoder.FlushAsync().get();
// }

void Capture::VerifyWindowsVersion() {
    typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    RTL_OSVERSIONINFOW osvi = { sizeof(osvi) };
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (!hMod) throw std::runtime_error("Cannot check Windows version.");

    auto func = reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(hMod, "RtlGetVersion"));
    if (!func || func(&osvi) != 0) throw std::runtime_error("Cannot check Windows version.");

    if (osvi.dwMajorVersion < 10 || osvi.dwBuildNumber < 14393) {
        throw std::runtime_error("Windows 10 Anniversary Update (1607) or later is required.");
    }
}

void Capture::InitD3D(HWND hwnd) {
    D3D_FEATURE_LEVEL featureLevel;
    winrt::check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
        D3D11_SDK_VERSION, m_d3dDevice.put(), &featureLevel, m_d3dContext.put()));

    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    m_d3dDevice->QueryInterface(dxgiDevice.put());

    winrt::com_ptr<IDXGIAdapter> adapter;
    dxgiDevice->GetAdapter(adapter.put());

    winrt::com_ptr<IDXGIFactory2> factory;
    adapter->GetParent(__uuidof(IDXGIFactory2), factory.put_void());

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = 800;
    desc.Height = 600;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferCount = 2;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.SampleDesc.Count = 1;

    winrt::check_hresult(factory->CreateSwapChainForHwnd(
        m_d3dDevice.get(), hwnd, &desc, nullptr, nullptr, m_swapChain.put()));

    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.put());
}

static bool resizing = false;

void Capture::OnWindowResize(UINT width, UINT height) {
    EnterCriticalSection(&m_resizeLock);
    if (m_swapChain && !resizing && width > 0 && height > 0) {
        m_renderTarget = nullptr;
        if (SUCCEEDED(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0))) {
            m_lastValidWidth = width;
            m_lastValidHeight = height;
            resizing = true;
        } 
    }
    if (!m_swapChain)
        RecreateDevice();
    LeaveCriticalSection(&m_resizeLock);
}

void Capture::RecreateDevice() {
    EnterCriticalSection(&m_resizeLock);
    m_renderTarget = nullptr;
    m_swapChain = nullptr;
    m_d3dContext = nullptr;
    m_d3dDevice = nullptr;
    try {
        InitD3D(m_hwnd);
        resizing = true; 
    } catch (const std::exception& e) {
        resizing = false;
    }
    LeaveCriticalSection(&m_resizeLock);
}

D2D1_RECT_F ComputeAspectCorrectPreviewSize(const D2D1_SIZE_F& targetSize, float sourceAspect, PreviewMode mode) {
    D2D1_RECT_F result = {};
    float desiredAspect = sourceAspect;

    if (targetSize.width <= 0 || targetSize.height <= 0)
        return D2D1::RectF(0.0f, 0.0f, 0.0f, 0.0f);

    switch(mode) {
        case PreviewMode::AspectStretch:
            result =  D2D1::RectF(0.0f, 0.0f, targetSize.width, targetSize.height);
            break;
        case PreviewMode::Aspect4x3:
            desiredAspect = 4.0f / 3.0f;
            result.top = 0.0f;
            result.bottom = targetSize.height;
            result.right = targetSize.height * desiredAspect;
            result.left = (targetSize.width - result.right) / 2.0f;
            result.right += result.left; 
            break;
        default:
            desiredAspect = 16.0f / 9.0f;
            result.left = 0.0f;
            result.right = targetSize.width;
            result.bottom = targetSize.width / desiredAspect;
            result.top = (targetSize.height - result.bottom) / 2.0f;
            result.bottom += result.top; 
    }
    return result;
}

void Capture::OnFrameArrived(MediaFrameReader const& reader) {
    auto frame = reader.TryAcquireLatestFrame();
    if (!frame)
        return;
    auto videoFrame = frame.VideoMediaFrame();
    if (!videoFrame)
        return;

    auto inputBitmap = videoFrame.SoftwareBitmap();
    if (!inputBitmap)
        return;

    auto bitmap = SoftwareBitmap::Convert(inputBitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);
    m_lastBitmap = bitmap;
    m_frameReaderActive = true;

    int width = bitmap.PixelWidth();
    int height = bitmap.PixelHeight();
    float sourceAspect = static_cast<float>(width) / height;
    int stride = width * 4;
    int bufferSize = stride * height;

    Windows::Storage::Streams::Buffer buffer(static_cast<uint32_t>(bufferSize));
    bitmap.CopyToBuffer(buffer);
    auto rawBytes = winrt::array_view<uint8_t>(buffer.data(), buffer.data() + bufferSize);

    {
        EnterCriticalSection(&m_resizeLock);

        winrt::com_ptr<IWICBitmap> wicBitmap;
        HRESULT hr = m_wicFactory->CreateBitmapFromMemory(
            width, height, GUID_WICPixelFormat32bppPBGRA, stride, bufferSize, rawBytes.data(), wicBitmap.put());
        if (FAILED(hr)) {
            LeaveCriticalSection(&m_resizeLock);
            return;
        }

        if (m_swapChain) {
            if (!m_renderTarget || resizing) {
                RECT rc{};
                GetClientRect(m_hwnd, &rc);
                UINT windowWidth = rc.right - rc.left;
                UINT windowHeight = rc.bottom - rc.top;

                if (windowWidth <= 0 || windowHeight <= 0) {
                    resizing = false;
                    LeaveCriticalSection(&m_resizeLock);
                    return;
                }

                DXGI_SWAP_CHAIN_DESC1 desc;
                hr = m_swapChain->GetDesc1(&desc);
                if (FAILED(hr)) {
                    RecreateDevice();
                    LeaveCriticalSection(&m_resizeLock);
                    return;
                }
                if (desc.Width != windowWidth || desc.Height != windowHeight) {
                    m_renderTarget = nullptr;
                    hr = m_swapChain->ResizeBuffers(0, windowWidth, windowHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
                    if (FAILED(hr)) {
                        RecreateDevice();
                        LeaveCriticalSection(&m_resizeLock);
                        return;
                    }
                }

                winrt::com_ptr<ID3D11Texture2D> backBuffer;
                winrt::com_ptr<IDXGISurface> surface;
                hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), backBuffer.put_void());
                if (FAILED(hr)) {
                    RecreateDevice();
                    LeaveCriticalSection(&m_resizeLock);
                    return;
                }
                hr = backBuffer->QueryInterface(surface.put());
                if (FAILED(hr)) {
                    RecreateDevice();
                    LeaveCriticalSection(&m_resizeLock);
                    return;
                }

                D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
                    D2D1_RENDER_TARGET_TYPE_DEFAULT,
                    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
                hr = m_d2dFactory->CreateDxgiSurfaceRenderTarget(surface.get(), &props, m_renderTarget.put());
                if (FAILED(hr)) {
                    RecreateDevice();
                    LeaveCriticalSection(&m_resizeLock);
                    return;
                }
                resizing = false;
            }

            if (!m_renderTarget) {
                LeaveCriticalSection(&m_resizeLock);
                return;
            }

            winrt::com_ptr<ID2D1Bitmap> d2dBitmap;
            hr = m_renderTarget->CreateBitmapFromWicBitmap(wicBitmap.get(), nullptr, d2dBitmap.put());
            if (FAILED(hr)) {
                LeaveCriticalSection(&m_resizeLock);
                return;
            }

            if (!IsWindowEnabled(m_hwnd) || !IsWindowVisible(m_hwnd)) {
                hr = m_renderTarget->EndDraw();
                if (FAILED(hr)) {
                    if (hr == D2DERR_RECREATE_TARGET) {
                        m_renderTarget = nullptr;
                        resizing = true;
                    }
                    LeaveCriticalSection(&m_resizeLock);
                    return;
                }

                hr = m_swapChain->Present(1, 0);
                if (FAILED(hr)) {
                    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                        RecreateDevice();
                }
                LeaveCriticalSection(&m_resizeLock);
                return;
            }

            m_renderTarget->BeginDraw();
            m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

            RECT rc{};
            GetClientRect(m_hwnd, &rc);
            D2D1_SIZE_F targetSize = D2D1::SizeF(
                static_cast<float>(rc.right - rc.left),
                static_cast<float>(rc.bottom - rc.top)
            );

            if (targetSize.width <= 0 || targetSize.height <= 0) {
                if (m_lastValidWidth > 0 && m_lastValidHeight > 0) {
                    targetSize = D2D1::SizeF(static_cast<float>(m_lastValidWidth), static_cast<float>(m_lastValidHeight));
                } else {
                    m_renderTarget->EndDraw();
                    LeaveCriticalSection(&m_resizeLock);
                    return;
                }
            } else {
                m_lastValidWidth = static_cast<UINT>(targetSize.width);
                m_lastValidHeight = static_cast<UINT>(targetSize.height);
            }

            if (targetSize.width <= 0 || targetSize.height <= 0) {
                m_renderTarget->EndDraw();
                LeaveCriticalSection(&m_resizeLock);
                return;
            }

            D2D1_RECT_F previewRect = ComputeAspectCorrectPreviewSize(targetSize, sourceAspect, m_previewMode);
            D2D1_RECT_F sourceRect = D2D1::RectF(
                0.0f, 0.0f,
                static_cast<float>(width),
                static_cast<float>(height)
            );

            m_renderTarget->DrawBitmap(
                d2dBitmap.get(), previewRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRect);
            hr = m_renderTarget->EndDraw();
            if (FAILED(hr)) {
                if (hr == D2DERR_RECREATE_TARGET) {
                    m_renderTarget = nullptr;
                    resizing = true;
                }
                LeaveCriticalSection(&m_resizeLock);
                return;
            }

            hr = m_swapChain->Present(1, 0);
            if (FAILED(hr)) {
                if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                    RecreateDevice();
                LeaveCriticalSection(&m_resizeLock);
                return;
            }
        }
        LeaveCriticalSection(&m_resizeLock);
    }
}

// Static device listing
std::vector<std::wstring> Capture::GetVideoDeviceNames() {
    std::vector<std::wstring> result;
    auto devices = DeviceInformation::FindAllAsync(MediaDevice::GetVideoCaptureSelector()).get();
    for (auto&& dev : devices) result.push_back(dev.Name().c_str());
    return result;
}

std::vector<std::wstring> Capture::GetAudioDeviceNames() {
    std::vector<std::wstring> result;
    auto devices = DeviceInformation::FindAllAsync(MediaDevice::GetAudioCaptureSelector()).get();
    for (auto&& dev : devices) result.push_back(dev.Name().c_str());
    return result;
}
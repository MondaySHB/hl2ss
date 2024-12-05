
#pragma once

#include <MemoryBuffer.h>
#include <mfidl.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Media.Capture.Frames.h>
#include <winrt/Windows.Graphics.Imaging.h>

class SoftwareBitmapBuffer : public IMFMediaBuffer
{
private:
    ULONG m_nRefCount;

    winrt::Windows::Graphics::Imaging::SoftwareBitmap                  m_bmp;
    winrt::Windows::Graphics::Imaging::BitmapBuffer                    m_buf;
    winrt::Windows::Foundation::IMemoryBufferReference                 m_ref;
    winrt::impl::com_ref<Windows::Foundation::IMemoryBufferByteAccess> m_bba;

    BYTE* m_pBase;
    DWORD m_maxLength;
    DWORD m_curLength;

    SoftwareBitmapBuffer(winrt::Windows::Media::Capture::Frames::MediaFrameReference const& ref);
    ~SoftwareBitmapBuffer();

public:
    static HRESULT CreateInstance(SoftwareBitmapBuffer** ppBuffer, winrt::Windows::Media::Capture::Frames::MediaFrameReference const& ref);

    // IUnknown Methods
    ULONG   AddRef();
    ULONG   Release();
    HRESULT QueryInterface(REFIID iid, void** ppv);

    // IMFMediaBuffer Methods
    HRESULT GetCurrentLength(DWORD* pcbCurrentLength);
    HRESULT GetMaxLength(DWORD* pcbMaxLength);
    HRESULT Lock(BYTE** ppbBuffer, DWORD* pcbMaxLength, DWORD* pcbCurrentLength);
    HRESULT SetCurrentLength(DWORD cbCurrentLength);
    HRESULT Unlock();
};

class BufferBuffer : public IMFMediaBuffer
{
private:
    ULONG m_nRefCount;

    winrt::Windows::Storage::Streams::Buffer m_buf;

    BYTE* m_pBase;
    DWORD m_maxLength;
    DWORD m_curLength;

    BufferBuffer(winrt::Windows::Storage::Streams::Buffer const& ref);
    ~BufferBuffer();

public:
    static HRESULT CreateInstance(BufferBuffer** ppBuffer, winrt::Windows::Storage::Streams::Buffer const& ref);

    // IUnknown Methods
    ULONG   AddRef();
    ULONG   Release();
    HRESULT QueryInterface(REFIID iid, void** ppv);

    // IMFMediaBuffer Methods
    HRESULT GetCurrentLength(DWORD* pcbCurrentLength);
    HRESULT GetMaxLength(DWORD* pcbMaxLength);
    HRESULT Lock(BYTE** ppbBuffer, DWORD* pcbMaxLength, DWORD* pcbCurrentLength);
    HRESULT SetCurrentLength(DWORD cbCurrentLength);
    HRESULT Unlock();
};

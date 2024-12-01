
#include <MemoryBuffer.h>
#include "encoder_rm_zlt.h"
#include "research_mode.h"

#include <winrt/Windows.Foundation.Collections.h>

using namespace Windows::Foundation;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage::Streams;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// OK
void Encoder_RM_ZLT::ToBGRA8(uint8_t const* pSigma, uint16_t const* pDepth, uint16_t const* pAb, uint32_t* pBGRA8)
{
	uint16x8_t threshold = vdupq_n_u16(RM_ZLT_MASK);

	for (int i = 0; i < (RM_ZLT_PIXELS / 32); ++i)
	{
	uint8x8x4_t  s = vld1_u8_x4(pSigma);
	uint16x8x4_t d = vld1q_u16_x4(pDepth);

	d.val[0] = vandq_u16(d.val[0], vcltq_u16(vmovl_u8(s.val[0]), threshold));
	d.val[1] = vandq_u16(d.val[1], vcltq_u16(vmovl_u8(s.val[1]), threshold));
	d.val[2] = vandq_u16(d.val[2], vcltq_u16(vmovl_u8(s.val[2]), threshold));
	d.val[3] = vandq_u16(d.val[3], vcltq_u16(vmovl_u8(s.val[3]), threshold));

	vst1q_u16_x4(pBGRA8, d);

	pSigma += 32;
	pDepth += 32;
	pBGRA8 += 16;
	}

	memcpy(pBGRA8, pAb, RM_ZLT_ABSIZE);
}

// OK
Encoder_RM_ZLT::Encoder_RM_ZLT(HOOK_RM_ZLT_PROC pHookCallback, void* pHookParam, H26xFormat& format, ZABFormat& zabFormat) : m_softwareBitmap(nullptr)
{
    format.width     = RM_ZLT_WIDTH;
    format.height    = RM_ZLT_HEIGHT;
    format.framerate = RM_ZLT_FPS;

    m_pngProperties.Insert(L"FilterOption", BitmapTypedValue(winrt::box_value(zabFormat.filter), winrt::Windows::Foundation::PropertyType::UInt8));
    m_softwareBitmap = SoftwareBitmap(BitmapPixelFormat::Bgra8, RM_ZLT_WIDTH, RM_ZLT_HEIGHT, BitmapAlphaMode::Straight);
    m_pHookCallback = pHookCallback;
    m_pHookParam = pHookParam;
}

// OK
void Encoder_RM_ZLT::WriteSample(BYTE const* pSigma, UINT16 const* pDepth, UINT16 const* pAbImage, LONGLONG timestamp, UINT8* metadata, UINT32 metadata_size)
{
    BYTE* pixelBufferData;
    UINT32 pixelBufferDataLength;
    uint32_t streamSize;

    {
    auto bitmapBuffer = m_softwareBitmap.LockBuffer(BitmapBufferAccessMode::Write);
    auto spMemoryBufferByteAccess = bitmapBuffer.CreateReference().as<IMemoryBufferByteAccess>();

    spMemoryBufferByteAccess->GetBuffer(&pixelBufferData, &pixelBufferDataLength);

    ToBGRA8(pSigma, pDepth, pAbImage, (uint32_t*)pixelBufferData);
    }

    auto stream = InMemoryRandomAccessStream();
    auto encoder = BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), stream, m_pngProperties).get();

    encoder.SetSoftwareBitmap(m_softwareBitmap);
    encoder.FlushAsync().get();

    streamSize = (uint32_t)stream.Size();

    auto streamBuf = Buffer(streamSize);

    stream.ReadAsync(streamBuf, streamSize, InputStreamOptions::None).get();

    m_pHookCallback(streamBuf, timestamp, metadata, metadata_size, m_pHookParam);
}

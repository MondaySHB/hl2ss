
#include "microphone_capture.h"
#include "channel.h"
#include "ipc_sc.h"
#include "ports.h"
#include "encoder_mc.h"

class Channel_MC : public Channel
{
private:
	std::unique_ptr<Encoder_MC> m_pEncoder;

	bool Startup();
	void Run();
	void Cleanup();

	void Execute_Mode0();

	void OnFrameArrived(BYTE* data, UINT32 frames, bool silent, UINT64 timestamp);
	void OnEncodingComplete(void* encoded, DWORD encoded_size, LONGLONG sample_time, void* metadata, UINT32 metadata_size);

	static void Thunk_Sensor(BYTE* data, UINT32 frames, bool silent, UINT64 timestamp, void* self);
	static void Thunk_Encoder(void* encoded, DWORD encoded_size, LONGLONG sample_time, void* metadata, UINT32 metadata_size, void* self);

public:
	Channel_MC(char const* name, char const* port, uint32_t id);
};

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

static std::unique_ptr<Channel_MC> g_channel;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// OK
void Channel_MC::Thunk_Sensor(BYTE* data, UINT32 frames, bool silent, UINT64 timestamp, void* self)
{
	static_cast<Channel_MC*>(self)->OnFrameArrived(data, frames, silent, timestamp);
}

// OK
void Channel_MC::Thunk_Encoder(void* pBytes, DWORD cbData, LONGLONG sample_time, void* metadata, UINT32 metadata_size, void* self)
{
	static_cast<Channel_MC*>(self)->OnEncodingComplete(pBytes, cbData, sample_time, metadata, metadata_size);
}

// OK
void Channel_MC::OnFrameArrived(BYTE* data, UINT32 frames, bool silent, UINT64 timestamp)
{
	m_pEncoder->WriteSample(data, frames, silent, timestamp);
}

// OK
void Channel_MC::OnEncodingComplete(void* encoded, DWORD encoded_size, LONGLONG sample_time, void* metadata, UINT32 metadata_size)
{
	(void)metadata;
	(void)metadata_size;

	WSABUF wsaBuf[3];
	bool ok;

	pack_buffer(wsaBuf, 0, &sample_time,  sizeof(sample_time));
	pack_buffer(wsaBuf, 1, &encoded_size, sizeof(encoded_size));
	pack_buffer(wsaBuf, 2, encoded,       encoded_size);

	ok = send_multiple(m_socket_client, wsaBuf, sizeof(wsaBuf) / sizeof(WSABUF));
	if (!ok) { SetEvent(m_event_client); }
}

// OK
void Channel_MC::Execute_Mode0()
{
	AACFormat format;
	bool ok;

	ok = ReceiveAACFormat_Profile(m_socket_client, format);
	if (!ok) { return; }

	bool array_raw = (format.profile == AACProfile::AACProfile_None) && (format.level == AACLevel::AACLevel_L5);

	MicrophoneCapture_Open(array_raw);

	if (!MicrophoneCapture_Status()) { return; }

	Encoder_MC::SetAACFormat(format, array_raw);

	m_pEncoder = std::make_unique<Encoder_MC>(Thunk_Encoder, this, format);

	MicrophoneCapture_ExecuteSensorLoop(Thunk_Sensor, this, m_event_client);

	m_pEncoder.reset();

	MicrophoneCapture_Close();
}

// OK
Channel_MC::Channel_MC(char const* name, char const* port, uint32_t id) : 
Channel(name, port, id)
{
}

// OK
bool Channel_MC::Startup()
{
	return true;
}

// OK
void Channel_MC::Run()
{
	Execute_Mode0();
}

// OK
void Channel_MC::Cleanup()
{
}

// OK
void MC_Initialize()
{
	g_channel = std::make_unique<Channel_MC>("MC", PORT_NAME_MC, PORT_NUMBER_MC - PORT_NUMBER_BASE);
}

// OK
void MC_Cleanup()
{
	g_channel.reset();
}

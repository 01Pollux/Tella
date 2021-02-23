
#include "Main.h"
#include "NetMessage.h"
#include "Glow.h"

extern IGlowManager* GlowManager;

void Interfaces::Init()
{
    MIKUDebug::Init();

    Library::LoadLibraries();

    gpGlobals   = reinterpret_cast<CGlobalVarsBase*>(Library::enginelib.FindPattern("pGlobalVarBasePtr"));

    clientlist  = Library::clientlib.FindInterface<IClientEntityList>(VCLIENTENTITYLIST_INTERFACE_VERSION);

    clienttrace = Library::enginelib.FindInterface<IEngineTrace>(INTERFACEVERSION_ENGINETRACE_CLIENT);

    nstcontainer = Library::enginelib.FindInterface<INetworkStringTableContainer>(INTERFACENAME_NETWORKSTRINGTABLECLIENT);

    debugoverlay = Library::enginelib.FindInterface<IVDebugOverlay>(VDEBUG_OVERLAY_INTERFACE_VERSION);

    panel        = reinterpret_cast<vgui::IPanel*>(Library::clientlib.FindPattern("g_pVGUIPanel"));

    surface      = Library::vguimatsurfacelib.FindInterface<vgui::ISurface>(VGUI_SURFACE_INTERFACE_VERSION);

    engineclient = Library::enginelib.FindInterface<IVEngineClient>(VENGINE_CLIENT_INTERFACE_VERSION);

    eventmanager = Library::enginelib.FindInterface<IGameEventManager2>(INTERFACEVERSION_GAMEEVENTSMANAGER2);

    modelinfo   = Library::enginelib.FindInterface<IVModelInfo>(VMODELINFO_CLIENT_INTERFACE_VERSION);

    clientdll   = Library::clientlib.FindInterface<IBaseClientDLL>(CLIENT_DLL_INTERFACE_VERSION);

    g_pCVar     = Library::libvstd_lib.FindInterface<ICvar>(CVAR_INTERFACE_VERSION);

    clienttools = Library::enginelib.FindInterface<IClientTools>(VCLIENTTOOLS_INTERFACE_VERSION);

    GlowManager = reinterpret_cast<IGlowManager*>(Library::clientlib.FindPattern("GlowManager"));
}

bool CLC_RespondCvarValue::WriteToBuffer(bf_write& buffer) noexcept(false)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

    buffer.WriteSBitLong(m_iCookie, 32);
    buffer.WriteSBitLong(m_eStatusCode, 4);

    buffer.WriteString(m_szCvarName);
    buffer.WriteString(m_szCvarValue);

    return !buffer.IsOverflowed();
}

bool CLC_RespondCvarValue::ReadFromBuffer(bf_read& buffer) noexcept(false)
{
    m_iCookie = buffer.ReadSBitLong(32);
    m_eStatusCode = static_cast<EQueryCvarValueStatus>(buffer.ReadSBitLong(4));

    buffer.ReadString(m_szCvarNameBuffer, sizeof(m_szCvarNameBuffer));
    m_szCvarName = m_szCvarNameBuffer;

    buffer.ReadString(m_szCvarValueBuffer, sizeof(m_szCvarValueBuffer));
    m_szCvarValue = m_szCvarValueBuffer;

    return !buffer.IsOverflowed();
}

bool NET_SignonState::WriteToBuffer(bf_write& buffer) noexcept(false)
{
    buffer.WriteUBitLong(GetType(), 6);
    buffer.WriteByte(m_nSignonState);
    buffer.WriteLong(m_nSpawnCount);

    return !buffer.IsOverflowed();
}

bool NET_SignonState::ReadFromBuffer(bf_read& buffer) noexcept(false)
{
    m_nSignonState = buffer.ReadByte();
    m_nSpawnCount = buffer.ReadLong();

    return !buffer.IsOverflowed();
}

static constexpr int NUM_NEW_COMMAND_BITS = 4;
static constexpr int NUM_BACKUP_COMMAND_BITS = 3;

bool CLC_VoiceData::WriteToBuffer(bf_write& buffer) noexcept(false)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    m_nLength = m_DataOut.GetNumBitsWritten();
    buffer.WriteWord(m_nLength); // length in bits

    return buffer.WriteBits(m_DataOut.GetBasePointer(), m_nLength);
}

bool CLC_VoiceData::ReadFromBuffer(bf_read& buffer) noexcept(false)
{
    m_nLength = buffer.ReadWord(); // length in bits
    m_DataIn = buffer;

    return buffer.SeekRelative(m_nLength);
}

bool CLC_BaselineAck::WriteToBuffer(bf_write& buffer) noexcept(false)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    buffer.WriteLong(m_nBaselineTick);
    buffer.WriteUBitLong(m_nBaselineNr, 1);
    return !buffer.IsOverflowed();
}

bool CLC_BaselineAck::ReadFromBuffer(bf_read& buffer) noexcept(false)
{

    m_nBaselineTick = buffer.ReadLong();
    m_nBaselineNr = buffer.ReadUBitLong(1);
    return !buffer.IsOverflowed();
}

bool CLC_Move::WriteToBuffer(bf_write& buffer) noexcept(false)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    m_nLength = m_DataOut.GetNumBitsWritten();

    buffer.WriteUBitLong(m_nNewCommands, NUM_NEW_COMMAND_BITS);
    buffer.WriteUBitLong(m_nBackupCommands, NUM_BACKUP_COMMAND_BITS);

    buffer.WriteWord(m_nLength);

    return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool CLC_Move::ReadFromBuffer(bf_read& buffer) noexcept(false)
{

    m_nNewCommands = buffer.ReadUBitLong(NUM_NEW_COMMAND_BITS);
    m_nBackupCommands = buffer.ReadUBitLong(NUM_BACKUP_COMMAND_BITS);
    m_nLength = buffer.ReadWord();
    m_DataIn = buffer;
    return buffer.SeekRelative(m_nLength);
}

bool NET_SetConVar::WriteToBuffer(bf_write& buffer) noexcept(false)
{
    buffer.WriteUBitLong(GetType(), 6);
    buffer.WriteByte(1);
    buffer.WriteString(convar.name);
    buffer.WriteString(convar.value);
    return !buffer.IsOverflowed();
}

bool NET_SetConVar::ReadFromBuffer(bf_read& buffer) noexcept(false)
{
    int numvars = buffer.ReadByte();

    // m_ConVars.RemoveAll();

    for (int i = 0; i < numvars; i++)
    {
        cvar_t cvar;
        buffer.ReadString(cvar.name, sizeof(cvar.name));
        buffer.ReadString(cvar.value, sizeof(cvar.value));
        // m_ConVars.AddToTail( cvar );
    }
    return !buffer.IsOverflowed();
}

bool NET_StringCmd::WriteToBuffer(bf_write& buffer) noexcept(false)
{
    buffer.WriteUBitLong(GetType(), 6);
    return buffer.WriteString(m_szCommand ? m_szCommand : " NET_StringCmd NULL");
}

bool NET_StringCmd::ReadFromBuffer(bf_read& buffer) noexcept(false)
{
    m_szCommand = m_szCommandBuffer;
    return buffer.ReadString(m_szCommandBuffer, sizeof(m_szCommandBuffer));
}

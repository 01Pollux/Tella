#include "NetMessage.hpp"


bool CLC_VoiceData::WriteToBuffer(bf_write& buffer) 
{
    buffer.write_ubit(static_cast<uint32_t>(GetType()), 6);
    Length = DataOut.bits_written();
    buffer.write_word(Length); // length in bits

    return buffer.write_bits(DataOut.data(), Length);
}

bool CLC_VoiceData::ReadFromBuffer(bf_read& buffer) 
{
    Length = buffer.read_word(); // length in bits
    DataIn = buffer;

    return buffer.seek_relative(Length);
}

bool CLC_BaseLineAck::WriteToBuffer(bf_write& buffer)
{
    buffer.write_ubit(static_cast<uint32_t>(GetType()), 6);
    buffer.write_long(BaselineTick);
    buffer.write_ubit(BaselineNr, 1);
    return !buffer.has_overflown();
}

bool CLC_BaseLineAck::ReadFromBuffer(bf_read& buffer)
{
    BaselineTick = buffer.read_long();
    BaselineNr = buffer.read_ubit(1);
    return !buffer.has_overflown();
}

bool CLC_Move::WriteToBuffer(bf_write& buffer) 
{
    buffer.write_ubit(static_cast<uint32_t>(GetType()), 6);
    Length = DataOut.bits_written();

    buffer.write_ubit(NewCommands, 4);
    buffer.write_ubit(BackupCommands, 3);

    buffer.write_word(Length);

    return buffer.write_bits(DataOut.data(), Length);
}

bool CLC_Move::ReadFromBuffer(bf_read& buffer) 
{
    NewCommands = buffer.read_ubit(4);
    BackupCommands = buffer.read_ubit(3);
    Length = buffer.read_word();
    DataIn = buffer;
    return buffer.seek_relative(Length);
}

bool NET_SetConVar::WriteToBuffer(bf_write& buffer) 
{
    buffer.write_ubit(static_cast<uint32_t>(GetType()), 6);
    buffer.write_byte(1);
    for (int i = 0; i < ConVars.Count(); i++)
    {
        buffer.write_string(ConVars[i].Name);
        buffer.write_string(ConVars[i].Value);
    }
    return !buffer.has_overflown();
}

bool NET_SetConVar::ReadFromBuffer(bf_read& buffer)
{
    int numvars = buffer.read_byte();

    ConVars.Purge();
    cvar_t cvar;

    for (int i = 0; i < numvars; i++)
    {
        buffer.read_string(cvar.Name, sizeof(cvar.Name));
        buffer.read_string(cvar.Value, sizeof(cvar.Value));
        ConVars.AddToTail(cvar);
    }
    return !buffer.has_overflown();
}

bool NET_StringCmd::WriteToBuffer(bf_write& buffer)
{
    buffer.write_ubit(static_cast<uint32_t>(GetType()), 6);
    return buffer.write_string(Command ? Command : " NET_StringCmd NULL");
}

bool NET_StringCmd::ReadFromBuffer(bf_read& buffer)
{
    Command = CommandBuffer;
    return buffer.read_string(CommandBuffer, sizeof(CommandBuffer));
}
#pragma once

#ifdef M0PROTO_BUILD
#define M0PROTO _declspec(dllexport)
#else
#pragma comment(lib, "ProtoExport.lib")
#define M0PROTO _declspec(dllimport)
#endif

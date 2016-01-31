/*
*
*/
#include "HTTPNetworkManager.h"

#ifdef _WIN32
#include "WinHTTP.inl"
#else
#include "CurlHTTP.inl"
#endif

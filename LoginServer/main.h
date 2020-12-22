#ifndef MAIN_H
#define MAIN_H
#include <Windows.h>



constexpr const char *LoginServerUsage = R"(
-help		displays a list of options (this)
-headless		runs the LoginServer in headless mode, where it doesn't contain
			a Console. Use -logs or -IPC options for further communication.

-logs		not implemented yet
-IPC			not implemented yet)";


/* configuration of the host & port for the LoginServer */
const char *g_LoginServerHost = "localhost";
int g_LoginServerPort = 1115;

//							     17    bd    08    6b    1b    94    f0    2f    f0    ec    53    d7    63    58    9b    5f
BYTE g_LoginServerEncryptionKey[] = { 0x17, 0xBD, 0x08, 0x6b, 0x1B, 0x94, 0xF0, 0x2F, 0xF0, 0xEC, 0x53, 0xD7, 0x63, 0x58, 0x9B, 0x5F };


struct
{
	/*
	Result to IPC communications or logs instead of the Console.
	*/
	BOOL RunHeadless;
} LoginServerSettings;
#endif // MAIN_H
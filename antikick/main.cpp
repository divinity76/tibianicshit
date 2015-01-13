#include <iostream>
#include <assert.h>
#include <vector>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <sal.h>
#include <sstream>
#include <cstdint>
#include <algorithm>
#include <string>

//#include "AutoItX\AutoItX3_DLL.h"
std::vector<int> getPIDsByProcessName(_In_ const TCHAR* name, _Out_opt_ std::vector<std::string>* warnings=NULL){
	std::vector<int> ret;
//	std::vector<std::string> imanoob;
//	if (!errors){
//		erros = &imanoob;
//	}


	DWORD aProcesses[1024], cbNeeded, cProcesses,i;
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		throw new std::exception("unable to get list of PIDs!");
	}
	// Calculate how many process identifiers were returned.
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	cProcesses = cbNeeded / sizeof(DWORD);
	
	HANDLE hProcess = 0;
	for (i = 0; i < cProcesses; ++i)
	{

		if (aProcesses[i] == 0)
			{ 
				//???
				continue; 
		}
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, aProcesses[i]);
		if (NULL == hProcess){
			fprintf(stderr, "warning! could not get PROCESS_QUERY_INFORMATION of PID %i (if it is tibianic.exe\'s PID, you got a problem...)\n", aProcesses[i]);
			continue;
		}
		{
			
			HMODULE hMod;
			DWORD cbNeeded;
			TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded) == 0){
				fprintf(stderr, "warning! could not EnumProcessModules of PID %i (if it is tibianic.exe\'s PID, you got a problem...)\n", aProcesses[i]);
				goto end;
			}
			if (GetModuleBaseName(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR)) == 0)
			{
				fprintf(stderr, "warning! could not GetModuleBaseName of PID %i (if it is tibianic.exe\'s PID, you got a problem...)\n", aProcesses[i]);
				goto end;
			}
			
			if (_tcsicmp(szProcessName, name)==0)
			{
				ret.push_back(aProcesses[i]);
			}
			end:
			CloseHandle(hProcess);
		}
		
	}
	return ret;
}
template<typename T>
std::string hhb_tostring(T input){
	std::stringstream ss;
	ss << input;
	return ss.str();
}
std::string ReadProcessMemoryString(
	_In_   HANDLE hProcess,
	_In_   LPCVOID lpBaseAddress,
	_In_opt_ BYTE breakByte = 0x00
	){
	std::vector<BYTE> buf;
	buf.resize(1);
	SIZE_T bytesToRead = 0;//
	SIZE_T bytesRead = 0;
	while (1){
		ReadProcessMemory(hProcess, (void*)(int(lpBaseAddress) + bytesToRead), &buf[0], 1, &bytesRead);
		assert(bytesRead == 1);
		++bytesToRead;
		if (buf[0] == breakByte)break;
	}
	buf.resize(bytesToRead);
	ReadProcessMemory(hProcess, lpBaseAddress, &buf[0], bytesToRead, &bytesRead);
	assert(bytesRead == bytesToRead);
	std::string ret(buf.begin(), buf.end());
	return ret;
}


std::string hhb_strcat(std::initializer_list<std::string> values)
{
	//std::stringstream ret;
	std::string ret;
	for (auto& value : values) {
//		ret << value;
		ret += value;

	}
//	return ret.str();
	return ret;
}
template<typename T,typename TT>
void* getAbsoluteAddress(T relative,TT base=0){
	int64_t ret = int(base) + int(relative);
	return (void*)ret;
}
std::string getTibianicCharNameByPid(int pid){
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_VM_READ, true, pid);
	if (!hProcess){
		throw new std::exception(hhb_strcat({ "COULD NOT GET PROCESS_VM_READ ACCESS ON PID ", hhb_tostring(pid) }).c_str());
	}
	//std::string name = ReadProcessMemoryString(hProcess, getAbsoluteAddress(0x005C6950,0), 0x00);
	
	void* nameptr = (void*)0x1337;
	ReadProcessMemory(hProcess, (void*)0x0071C54C, &nameptr, sizeof(decltype(nameptr)), NULL);
	std::string name = ReadProcessMemoryString(hProcess, nameptr, 0x00);
	return name;
}
//HWND g_HWND = NULL; //TODO: lamda and nonglobal...
std::vector<HWND> g_HWND;
BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		//std::cout << "LOLz";
		//g_HWND = hwnd;
		//return FALSE;
		g_HWND.push_back(hwnd);
	}
	return TRUE;
}
std::vector<HWND> getHwndsFromPID(int pid){
	g_HWND.clear();
	EnumWindows(EnumWindowsProcMy, pid);

	return g_HWND;

}
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
struct hhb_vkey{
	const char* name;
	const uint8_t vkey;
	const char* description;
};
const hhb_vkey hhb_vkey_list[] = {
	{ "VK_LBUTTON", 0x01, "Left mouse button" },
	{ "VK_RBUTTON", 0x02, "Right mouse button" },
	{ "VK_CANCEL", 0x03, "Control-break processing" },
	{ "VK_MBUTTON", 0x04, "Middle mouse button (three-button mouse)" },
	{ "VK_XBUTTON1", 0x05, "X1 mouse button" },
	{ "VK_XBUTTON2", 0x06, "X2 mouse button" },
//	{ "-", 0x07, "Undefined" },
	{ "VK_BACK", 0x08, "BACKSPACE key" },
	{ "VK_TAB", 0x09, "TAB key" },
//	{ "-", 0x0A - 0B, "Reserved" },
	{ "VK_CLEAR", 0x0C, "CLEAR key" },
	{ "VK_RETURN", 0x0D, "ENTER key" },
//	{ "-", 0x0E - 0F, "Undefined" },
	{ "VK_SHIFT", 0x10, "SHIFT key" },
	{ "VK_CONTROL", 0x11, "CTRL key" },
	{ "VK_MENU", 0x12, "ALT key" },
	{ "VK_PAUSE", 0x13, "PAUSE key" },
	{ "VK_CAPITAL", 0x14, "CAPS LOCK key" },
	{ "VK_KANA", 0x15, "IME Kana mode" },
	{ "VK_HANGUEL", 0x15, "IME Hanguel mode (maintained for compatibility; use VK_HANGUL)" },
	{ "VK_HANGUL", 0x15, "IME Hangul mode" },
//	{ "-", 0x16, "Undefined" },
	{ "VK_JUNJA", 0x17, "IME Junja mode" },
	{ "VK_FINAL", 0x18, "IME final mode" },
	{ "VK_HANJA", 0x19, "IME Hanja mode" },
	{ "VK_KANJI", 0x19, "IME Kanji mode" },
//	{ "-", 0x1A, "Undefined" },
	{ "VK_ESCAPE", 0x1B, "ESC key" },
	{ "VK_CONVERT", 0x1C, "IME convert" },
	{ "VK_NONCONVERT", 0x1D, "IME nonconvert" },
	{ "VK_ACCEPT", 0x1E, "IME accept" },
	{ "VK_MODECHANGE", 0x1F, "IME mode change request" },
	{ "VK_SPACE", 0x20, "SPACEBAR" },
	{ "VK_PRIOR", 0x21, "PAGE UP key" },
	{ "VK_NEXT", 0x22, "PAGE DOWN key" },
	{ "VK_END", 0x23, "END key" },
	{ "VK_HOME", 0x24, "HOME key" },
	{ "VK_LEFT", 0x25, "LEFT ARROW key" },
	{ "VK_UP", 0x26, "UP ARROW key" },
	{ "VK_RIGHT", 0x27, "RIGHT ARROW key" },
	{ "VK_DOWN", 0x28, "DOWN ARROW key" },
	{ "VK_SELECT", 0x29, "SELECT key" },
	{ "VK_PRINT", 0x2A, "PRINT key" },
	{ "VK_EXECUTE", 0x2B, "EXECUTE key" },
	{ "VK_SNAPSHOT", 0x2C, "PRINT SCREEN key" },
	{ "VK_INSERT", 0x2D, "INS key" },
	{ "VK_DELETE", 0x2E, "DEL key" },
	{ "VK_HELP", 0x2F, "HELP key" },
	{ "0 key", 0x30, "0 key" },
	{ "1 key", 0x31, "1 key" },
	{ "2 key", 0x32, "2 key" },
	{ "3 key", 0x33, "3 key" },
	{ "4 key", 0x34, "4 key" },
	{ "5 key", 0x35, "5 key" },
	{ "6 key", 0x36, "6 key" },
	{ "7 key", 0x37, "7 key" },
	{ "8 key", 0x38, "8 key" },
	{ "9 key", 0x39, "9 key" },
//	{ "undefined", 0x3A - 40, "undefined" },
	{ "A key", 0x41, "A key" },
	{ "B key", 0x42, "B key" },
	{ "C key", 0x43, "C key" },
	{ "D key", 0x44, "D key" },
	{ "E key", 0x45, "E key" },
	{ "F key", 0x46, "F key" },
	{ "G key", 0x47, "G key" },
	{ "H key", 0x48, "H key" },
	{ "I key", 0x49, "I key" },
	{ "J key", 0x4A, "J key" },
	{ "K key", 0x4B, "K key" },
	{ "L key", 0x4C, "L key" },
	{ "M key", 0x4D, "M key" },
	{ "N key", 0x4E, "N key" },
	{ "O key", 0x4F, "O key" },
	{ "P key", 0x50, "P key" },
	{ "Q key", 0x51, "Q key" },
	{ "R key", 0x52, "R key" },
	{ "S key", 0x53, "S key" },
	{ "T key", 0x54, "T key" },
	{ "U key", 0x55, "U key" },
	{ "V key", 0x56, "V key" },
	{ "W key", 0x57, "W key" },
	{ "X key", 0x58, "X key" },
	{ "Y key", 0x59, "Y key" },
	{ "Z key", 0x5A, "Z key" },//hmm, add the lowercase a-z as well?
	{ "VK_LWIN", 0x5B, "Left Windows key (Natural keyboard)" },
	{ "VK_RWIN", 0x5C, "Right Windows key (Natural keyboard)" },
	{ "VK_APPS", 0x5D, "Applications key (Natural keyboard)" },
//	{ "-", 0x5E, "Reserved" },
	{ "VK_SLEEP", 0x5F, "Computer Sleep key" },
	{ "VK_NUMPAD0", 0x60, "Numeric keypad 0 key" },
	{ "VK_NUMPAD1", 0x61, "Numeric keypad 1 key" },
	{ "VK_NUMPAD2", 0x62, "Numeric keypad 2 key" },
	{ "VK_NUMPAD3", 0x63, "Numeric keypad 3 key" },
	{ "VK_NUMPAD4", 0x64, "Numeric keypad 4 key" },
	{ "VK_NUMPAD5", 0x65, "Numeric keypad 5 key" },
	{ "VK_NUMPAD6", 0x66, "Numeric keypad 6 key" },
	{ "VK_NUMPAD7", 0x67, "Numeric keypad 7 key" },
	{ "VK_NUMPAD8", 0x68, "Numeric keypad 8 key" },
	{ "VK_NUMPAD9", 0x69, "Numeric keypad 9 key" },
	{ "VK_MULTIPLY", 0x6A, "Multiply key" },
	{ "VK_ADD", 0x6B, "Add key" },
	{ "VK_SEPARATOR", 0x6C, "Separator key" },
	{ "VK_SUBTRACT", 0x6D, "Subtract key" },
	{ "VK_DECIMAL", 0x6E, "Decimal key" },
	{ "VK_DIVIDE", 0x6F, "Divide key" },
	{ "VK_F1", 0x70, "F1 key" },
	{ "VK_F2", 0x71, "F2 key" },
	{ "VK_F3", 0x72, "F3 key" },
	{ "VK_F4", 0x73, "F4 key" },
	{ "VK_F5", 0x74, "F5 key" },
	{ "VK_F6", 0x75, "F6 key" },
	{ "VK_F7", 0x76, "F7 key" },
	{ "VK_F8", 0x77, "F8 key" },
	{ "VK_F9", 0x78, "F9 key" },
	{ "VK_F10", 0x79, "F10 key" },
	{ "VK_F11", 0x7A, "F11 key" },
	{ "VK_F12", 0x7B, "F12 key" },
	{ "VK_F13", 0x7C, "F13 key" },
	{ "VK_F14", 0x7D, "F14 key" },
	{ "VK_F15", 0x7E, "F15 key" },
	{ "VK_F16", 0x7F, "F16 key" },
	{ "VK_F17", 0x80, "F17 key" },
	{ "VK_F18", 0x81, "F18 key" },
	{ "VK_F19", 0x82, "F19 key" },
	{ "VK_F20", 0x83, "F20 key" },
	{ "VK_F21", 0x84, "F21 key" },
	{ "VK_F22", 0x85, "F22 key" },
	{ "VK_F23", 0x86, "F23 key" },
	{ "VK_F24", 0x87, "F24 key" },
//	{ "-", 0x88 - 8F, "Unassigned" },
	{ "VK_NUMLOCK", 0x90, "NUM LOCK key" },
	{ "VK_SCROLL", 0x91, "SCROLL LOCK key" },
//	{ "-", 0x88 - 8F, "Unassigned" },
	{ "VK_NUMLOCK", 0x90, "NUM LOCK key" },
	{ "VK_SCROLL", 0x91, "SCROLL LOCK key" },
//	{"-", 0x92-96,"OEM specific"},
//	{ "-", 0x97 - 9F, "Unassigned" },
	{ "VK_LSHIFT", 0xA0, "Left SHIFT key" },
	{ "VK_RSHIFT", 0xA1, "Right SHIFT key" },
	{ "VK_LCONTROL", 0xA2, "Left CONTROL key" },
	{ "VK_RCONTROL", 0xA3, "Right CONTROL key" },
	{ "VK_LMENU", 0xA4, "Left MENU key" },
	{ "VK_RMENU", 0xA5, "Right MENU key" },
	{ "VK_BROWSER_BACK", 0xA6, "Browser Back key" },
	{ "VK_BROWSER_FORWARD", 0xA7, "Browser Forward key" },
	{ "VK_BROWSER_REFRESH", 0xA8, "Browser Refresh key" },
	{ "VK_BROWSER_STOP", 0xA9, "Browser Stop key" },
	{ "VK_BROWSER_SEARCH", 0xAA, "Browser Search key" },
	{ "VK_BROWSER_FAVORITES", 0xAB, "Browser Favorites key" },
	{ "VK_BROWSER_HOME", 0xAC, "Browser Start and Home key" },
	{ "VK_VOLUME_MUTE", 0xAD, "Volume Mute key" },
	{ "VK_VOLUME_DOWN", 0xAE, "Volume Down key" },
	{ "VK_VOLUME_UP", 0xAF, "Volume Up key" },
	{ "VK_MEDIA_NEXT_TRACK", 0xB0, "Next Track key" },
	{ "VK_MEDIA_PREV_TRACK", 0xB1, "Previous Track key" },
	{ "VK_MEDIA_STOP", 0xB2, "Stop Media key" },
	{ "VK_MEDIA_PLAY_PAUSE", 0xB3, "Play/Pause Media key" },
	{ "VK_LAUNCH_MAIL", 0xB4, "Start Mail key" },
	{ "VK_LAUNCH_MEDIA_SELECT", 0xB5, "Select Media key" },
	{ "VK_LAUNCH_APP1", 0xB6, "Start Application 1 key" },
	{ "VK_LAUNCH_APP2", 0xB7, "Start Application 2 key" },
//	{ "-", 0xB8 - B9, "Reserved" },
	{ "VK_OEM_1", 0xBA, "Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \';:\' key VK_OEM_PLUS" },
	{"VK_OEM_PLUS", 0xBB, "For any country/region, the \'+\' key"},
	{ "VK_OEM_COMMA", 0xBC, "For any country/region, the \',\' key" },
	{ "VK_OEM_MINUS", 0xBD, "For any country/region, the \'-\' key" },
	{ "VK_OEM_PERIOD", 0xBE, "For any country/region, the \'.\' key" },
	{ "VK_OEM_2", 0xBF, "Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \'/?\' key" },
	{ "VK_OEM_3", 0xC0, "Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \'`~\' key" },
//	{ "-", 0xC1 - D7, "Reserved" },
//	{ "-", 0xD8 - DA, "Unassigned" },
	{ "VK_OEM_4", 0xDB, "Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \'[{\' key" },
	{ "VK_OEM_5", 0xDC, "Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \'\\|\' key" },
	{ "VK_OEM_6", 0xDD, "Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \']}\' key" },
	{ "VK_OEM_7", 0xDE, "Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the \'single-quote/double-quote\' key" },
	{ "VK_OEM_8", 0xDF, "Used for miscellaneous characters; it can vary by keyboard." },
//	{ "-", 0xE0, "Reserved" },
//	{ "-", 0xE1, "OEM specific" },
	{ "VK_OEM_102", 0xE2, "Either the angle bracket key or the backslash key on the RT 102-key keyboard" },
//	{ "-", 0xE3 - E4, "OEM specific" },
	{ "VK_PROCESSKEY", 0xE5, "IME PROCESS key" },
//	{ "-", 0xE6, "OEM specific" },
	{ "VK_PACKET", 0xE7, "Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP" },
//	{ "-", 0xE8, "Unassigned" },
//  {"-",0xE6,"OEM specific"},
    {"VK_PACKET",0xE7,"Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP"},
//  {"-",0xE8,"Unassigned"},
//	{ "-", 0xE9 - F5, "OEM specific" },
	{ "VK_ATTN", 0xF6, "Attn key" },
	{ "VK_CRSEL", 0xF7, "CrSel key" },
	{ "VK_EXSEL", 0xF8, "ExSel key" },
	{ "VK_EREOF", 0xF9, "Erase EOF key" },
	{ "VK_PLAY", 0xFA, "Play key" },
	{ "VK_ZOOM", 0xFB, "Zoom key" },
	{ "VK_NONAME", 0xFC, "Reserved" },
	{ "VK_PA1", 0xFD, "PA1 key" },
	{ "VK_OEM_CLEAR", 0xFE, "Clear key" }
};

void SendToHwnd(_In_ HWND hwnd, _In_opt_ const std::string StringToSend){
	assert(hwnd != 0);//todo: validate hwnd?
	if (StringToSend.length() == 0)
	{ return; }
	//oookay, there's *probably* a *good* way to do this..
	std::vector<uint8_t> vkeys;
	uint32_t i = 0;
	const uint32_t length = StringToSend.length();
	//<parseString>
	bool isEscaping = false;
	for (i = 0; i < length; ++i){
		if (!isEscaping && StringToSend.at(i) == '\\'){
			isEscaping = true;
			continue;
		}
		if (!isEscaping && StringToSend.at(i)=='{')
		{
			std::string tmpstr = "";
			++i;
			while (StringToSend.at(i) != '}'){
				tmpstr += StringToSend.at(i);
				++i;
			}
			++i;
//			std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), ::toupper);
			bool found = false; 
			auto size = sizeof(hhb_vkey_list)/sizeof(hhb_vkey);
			for (unsigned int ii = 0; ii < size; ++ii){
//				int max = std::max(strlen(hhb_vkey_list[ii].name), tmpstr.length())
				uint32_t max = strlen(hhb_vkey_list[ii].name);
				if (max < tmpstr.length()){
					max = tmpstr.length(); }
					if (strncmp(hhb_vkey_list[ii].name, tmpstr.c_str(), max) == 0)
				{
					found = true;
					vkeys.push_back(hhb_vkey_list[ii].vkey);
					break;
				}
			}
			if (!found)
			{
				std::cerr << "Warning: unrecognized vkey code \"" << tmpstr << "\"" << std::endl;
			}
				continue;
		}
		vkeys.push_back(VkKeyScan(BYTE(StringToSend.at(i))));
		continue;
	}
	//</parseString>
	for (auto& vk : vkeys){
		auto scan = MapVirtualKey(vk, 0);
		auto lparam = 0x00000001 | (LPARAM)(scan << 16); //wtf? i have no clue.
		//memset(&lparam, 0x00, 2);
		PostMessage(hwnd, WM_KEYDOWN, vk, lparam);
	//	PostMessage(hwnd, WM_KEYUP, vk, lparam);
	}
	return;
}

//std::wstring stemp = s2ws(myString);
//LPCWSTR result = stemp.c_str();


void SetDebugPrivileges()
{
	void* tokenHandle;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tokenHandle);
	TOKEN_PRIVILEGES privilegeToken;
	LookupPrivilegeValue(0, SE_DEBUG_NAME, &privilegeToken.Privileges[0].Luid);
	privilegeToken.PrivilegeCount = 1;
	privilegeToken.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(tokenHandle, 0, &privilegeToken, sizeof(TOKEN_PRIVILEGES), 0, 0);
	CloseHandle(tokenHandle);
}



int TibianicPID = 0;
int main(int argc, char*argv[]){
	SetDebugPrivileges();
	auto tibianicPIDs = getPIDsByProcessName(TEXT("tibianic.exe"),0);
	for (auto& pid : tibianicPIDs){
		std::cout <<"pid "<< pid << ":"<<getTibianicCharNameByPid(pid) << std::endl;
	}
	std::cout << "enter PID to use..." << std::endl;
	std::cin >> TibianicPID;
	std::cout << "\nusing pid " << TibianicPID << std::endl;
	auto TibianicWindow = getHwndsFromPID(TibianicPID);
	//std::cout << "hwnd: " << TibianicWindow << std::endl;
///	while (1){ std::string tmpstr;  std::getline(std::cin, tmpstr);  SendToHwnd(TibianicWindow[0], tmpstr); }
	std::cout << "sending {VK_CONTROL}{VK_UP} and {VK_CONTROL}{VK_DOWN}{VK_DOWN}  every 7 minutes.. (antikick)" << std::endl;
	while (1){
		for (auto& hwnd : TibianicWindow){
			SendToHwnd(hwnd, "{VK_CONTROL}{VK_UP}{VK_UP}");
			SendToHwnd(hwnd, "{VK_CONTROL}{VK_DOWN}{VK_DOWN}");
			continue;
#ifdef NEGRO
			LRESULT WINAPI SendMessage(
				_In_  HWND hWnd,
				_In_  UINT Msg,
				_In_  WPARAM wParam,
				_In_  LPARAM lParam
				);

#endif // NEGRO
		}
		Sleep(7 * 60 * 1000);
			std::cout << "sending {VK_CONTROL}{VK_UP} and {VK_CONTROL}{VK_DOWN}{VK_DOWN}  every 7 minutes.. (antikick)" << std::endl;
	}
	system("pause");
	return 0;
}

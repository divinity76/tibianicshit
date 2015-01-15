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


std::vector<int> getPIDsByProcessName(_In_ const TCHAR* name, _Out_opt_ std::vector<std::string>* warnings = NULL){
	std::vector<int> ret;
	//	std::vector<std::string> imanoob;
	//	if (!errors){
	//		erros = &imanoob;
	//	}


	DWORD aProcesses[1024], cbNeeded, cProcesses, i;
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

			if (_tcsicmp(szProcessName, name) == 0)
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
template<typename T, typename TT>
void* getAbsoluteAddress(T relative, TT base = 0){
	int64_t ret = int(base) + int(relative);
	return (void*)ret;
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
	auto tibianicPIDs = getPIDsByProcessName(TEXT("tibianic.exe"), 0);
	for (auto& pid : tibianicPIDs){
		std::cout << "pid " << pid << ":" << getTibianicCharNameByPid(pid) << std::endl;
	}
	std::cout << "enter PID to use..." << std::endl;
	std::cin >> TibianicPID;
	std::cout << "\nusing pid " << TibianicPID << std::endl;
	auto hProcess = OpenProcess(
		//PROCESS_QUERY_INFORMATION |
		//PROCESS_VM_READ | PROCESS_VM_WRITE
		PROCESS_ALL_ACCESS
		,
		FALSE, TibianicPID);
	assert(hProcess != 0);
	void* buff = malloc(2);
	memset(buff, 0x90, 2);
	SIZE_T idgaf;
	WriteProcessMemory(hProcess, (void*)0x04BF94B, buff, 2, &idgaf);
	assert(idgaf == 2);
	memset(buff, 0xFF, 2);
	WriteProcessMemory(hProcess, (void*)0x4BF94E, buff,1, &idgaf);
	assert(idgaf == 1);
	free(buff);
	std::cout << "enabled lighthack! :D";
	return 0;
}

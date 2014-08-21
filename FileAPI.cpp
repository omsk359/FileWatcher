#include <windows.h>
#include <stdio.h>
#include <wincrypt.h>
#include <locale.h>
#include <string>
#include <iostream>

#include "FileAPI.h"

#define BUFSIZE 1024
#define MD5LEN  16

std::string FileAPI::md5(std::string fName, unsigned &status)
{
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE rgbFile[BUFSIZE];
    DWORD cbRead = 0;
    BYTE rgbHash[MD5LEN];
    DWORD cbHash = 0;
    CHAR hexDigits[] = "0123456789abcdef";

	std::wstring stemp = std::wstring(fName.begin(), fName.end());
	LPCWSTR filename = stemp.c_str();
//	std::wcout << L"File: " << filename << L"\n";

    HANDLE hFile = CreateFile(filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        status = GetLastError();
        printf("Error opening file %s\nError: %d\n", fName.c_str(), status);
        return std::string();
    }

    // Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT))
    {
        status = GetLastError();
        printf("CryptAcquireContext failed: %d\n", status); 
        CloseHandle(hFile);
		return std::string();
	}

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        status = GetLastError();
        printf("CryptAcquireContext failed: %d\n", status); 
        CloseHandle(hFile);
        CryptReleaseContext(hProv, 0);
		return std::string();
	}

    while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, &cbRead, NULL)) {
        if (!cbRead)
            break;

        if (!CryptHashData(hHash, rgbFile, cbRead, 0)) {
            status = GetLastError();
            printf("CryptHashData failed: %d\n", status); 
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
			return std::string();
		}
    }

    if (!bResult) {
        status = GetLastError();
        printf("ReadFile failed: %d\n", status); 
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
		return std::string();
	}

    cbHash = MD5LEN;
    if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
//        printf("MD5 hash of file %s is: ", filename);
//        for (DWORD i = 0; i < cbHash; i++) {
//            printf("%c%c", hexDigits[rgbHash[i] >> 4],
//                hexDigits[rgbHash[i] & 0xf]);
//        }
//        printf("\n");
    } else {
        status = GetLastError();
        printf("CryptGetHashParam failed: %d\n", status);
		return std::string();
	}

    std::string strHash(32, '0');
    for(size_t i = 0; i < cbHash; i++) {
        strHash[i*2]     = hexDigits[rgbHash[i] >> 4];
        strHash[(i*2)+1] = hexDigits[rgbHash[i] & 0xF];
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

    return strHash;    
}

/*std::time_t FileAPI::modifyTime(std::string fName, unsigned &status)
{
	std::wstring stemp = std::wstring(fName.begin(), fName.end());
	LPCWSTR filename = stemp.c_str();

	HANDLE hFile = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile) {
		status = GetLastError();
		printf("Error opening file %s\nError: %d\n", fName.c_str(), status);
		return time(NULL);
	}

	FILETIME fc, fa, fw;
	if (!GetFileTime(hFile, &fc, &fa, &fw)) {
		status = GetLastError();
		printf("Error get file time: %s\nError: %d\n", fName.c_str(), status);
	}
	return filetime_to_timet(fw);
}

std::time_t FileAPI::filetime_to_timet(FILETIME const& ft)
{
	ULARGE_INTEGER ull;
	ull.LowPart = ft.dwLowDateTime;
	ull.HighPart = ft.dwHighDateTime;
	return ull.QuadPart / 10000000ULL - 11644473600ULL;
}
*/
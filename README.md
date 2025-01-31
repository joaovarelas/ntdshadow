# NTDShadow

## Overview
**NTDShadow** is a helper tool designed to extract the **NTDS.dit** Active Directory database using **Volume Shadow Copies (VSS)**. This technique allows for stealthy credential extraction during penetration testing and red team engagements.

## Features
- Extract **NTDS.dit** via **VSS** without triggering traditional security defenses.
- Uses Windows operating system API's and interface **IVssBackupComponents**.
- Automate the enumeration and extraction process.
- Lightweight executable and efficient for red team operations.
- Recover AD hashed passwords in NT Hash format for complexity analysis & bruteforcing. 



## Screenshots





## References

- [https://learn.microsoft.com/en-us/windows/win32/api/vsbackup/nl-vsbackup-ivssbackupcomponents](https://learn.microsoft.com/en-us/windows/win32/api/vsbackup/nl-vsbackup-ivssbackupcomponents)
- [https://github.com/albertony/vss/](https://github.com/albertony/vss/)
- [https://jpcertcc.github.io/ToolAnalysisResultSheet/details/ntdsutil.htm](https://jpcertcc.github.io/ToolAnalysisResultSheet/details/ntdsutil.htm)


## Disclaimer
This tool is intended for legal security assessments and educational purposes only. Unauthorized use is strictly prohibited.


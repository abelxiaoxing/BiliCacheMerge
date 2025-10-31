#include <tchar.h>
#include <Windows.h>

int main()
{
	TCHAR strPath[MAX_PATH] = { 0 };

	if (GetModuleFileName(NULL, strPath, MAX_PATH)) {
		(_tcsrchr(strPath, _T('\\')))[1] = 0;
		_tcscat_s(strPath, MAX_PATH, _T("bin\\"));
		SetCurrentDirectory(strPath);
	};

	system("merger.exe");

	return 0;
}
#include "auxfunc.h"
//�ж��ļ����Ƿ����
//DWORD d = GetFileAttributesA(const char* filename); #include <windows.h> Ϊwindowsϵͳ�������ж��ļ�Ŀ¼�Ƿ����
bool dirExists(const std::string& dirpath)
{
    DWORD ftyp = GetFileAttributesA(dirpath.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!  

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!  

    return false;    // this is not a directory!  
}
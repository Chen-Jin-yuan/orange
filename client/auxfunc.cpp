#include "auxfunc.h"
//判断文件夹是否存在
//DWORD d = GetFileAttributesA(const char* filename); #include <windows.h> 为windows系统函数，判断文件目录是否存在
bool dirExists(const std::string& dirpath)
{
    DWORD ftyp = GetFileAttributesA(dirpath.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!  

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!  

    return false;    // this is not a directory!  
}
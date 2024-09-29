#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
 
int main() {
    // 创建一个 std::wstring，存储需要转换的字符串
    std::wstring utf16_string = L"你好，世界！";
 
    // 创建一个 std::wstring_convert，用于进行字符集转换
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_convert;
    std::string utf8_string = utf8_convert.to_bytes(utf16_string);
 
    // 输出 UTF-8 编码的字符串
    std::cout << "UTF-8: " << utf8_string << std::endl;
 
    // 创建一个 std::codecvt<wchar_t, char, mbstate_t>，用于进行 GBK 到 UTF-16 的转换
    std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> gbk_convert;
    std::wstring gbk_string = gbk_convert.from_bytes(utf8_string);
 
    // 输出 GBK 编码的字符串
    std::wcout.imbue(std::locale("zh_CN.GBK")); // 设置输出流的本地化
    std::wcout << "GBK: " << gbk_string << std::endl;
    
    
     //wstring 转 string
    std::wstring str = L"你好，世界！";
    std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter(new std::codecvt<wchar_t, char, std::mbstate_t>("CHS"));
    
    std::string narrowStr = converter.to_bytes(str);
    
     //string 转 wstring
    std::wstring wstr = converter.from_bytes(narrowStr);
    std::cout<<narrowStr<<'\n';
    std::wcout.imbue(std::locale("chs"));   //初始化cout为中文输出
    std::wcout<<wstr<<'\n';
 
    return 0;
}
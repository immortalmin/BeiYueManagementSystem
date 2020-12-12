#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include<sstream>
#include<fstream>

#include "../cgicc-3.2.16/cgicc/CgiDefs.h"
#include "../cgicc-3.2.16/cgicc/Cgicc.h"
#include "../cgicc-3.2.16/cgicc/HTTPHTMLHeader.h"
#include "../cgicc-3.2.16/cgicc/HTMLClasses.h"

using namespace std;
using namespace cgicc;

int main ()
{
    Cgicc cgi;

    cout << "Content-type:text/html\r\n\r\n";
    cout << "<html>\n";
    cout << "<head>\n";
    cout << "<title>CGI 中的文件上传</title>\n";
    cout << "</head>\n";
    cout << "<body>\n";

    // 获取要被上传的文件列表
    const_file_iterator file = cgi.getFile("wordFile");
    if(file != cgi.getFiles().end()) {
        // 在 cout 中发送数据类型
//        cout << HTTPContentHeader(file->getDataType());
        // 在 cout 中写入内容
//        file->writeToStream(cout);
        stringstream fileString(file->getData());
        string s;
        while(getline(fileString,s,'\n')){
            cout<<s<<"!"<<endl;
        }
    }
    cout << "<文件上传成功>\n";
    cout << "</body>\n";
    cout << "</html>\n";

    return 0;
}

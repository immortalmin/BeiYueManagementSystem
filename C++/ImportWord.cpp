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

/* MySQL Connector/C++ specific headers （注意：所有头文件都放在/usr/include/cppconn/下面）*/
#include "../include/cppconn/driver.h"
#include "../include/cppconn/connection.h"
#include "../include/cppconn/statement.h"
#include "../include/cppconn/prepared_statement.h"
#include "../include/cppconn/resultset.h"
#include "../include/cppconn/metadata.h"
#include "../include/cppconn/resultset_metadata.h"
#include "../include/cppconn/exception.h"
#include "../include/cppconn/warning.h"
#define PASSWORD "csk1314520"

using namespace std;
using namespace cgicc;
using namespace sql;

Driver *driver;
Connection *con;
Statement *stmt;
ResultSet *res;
PreparedStatement *prep_stmt;
Savepoint *savept;
int updatecount = 0;

string IntToString(int n);
int StringToInt(string s);
/**
 * 数据库连接
 * @return
 */
int ConnectDataBase(){
    /* initiate url, user, password and database variables */
    string url("localhost:3306");
    const string user("root");
    const string password(PASSWORD);
    const string database("word");
    try {
        driver = get_driver_instance();

        /* create a database connection using the Driver */
        con = driver -> connect(url, user, password);

        /* turn off the autocommit */
        con -> setAutoCommit(0);
        con -> setSchema(database);
    } catch (SQLException &e) {
        cout << "ERROR: SQLException in " << __FILE__;
        cout << " (" << __func__<< ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047) {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return EXIT_FAILURE;
    } catch (std::runtime_error &e) {

        cout << "ERROR: runtime_error in " << __FILE__;
        cout << " (" << __func__ << ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what() << endl;

        return EXIT_FAILURE;
    }
}

//判断字符串中是否包含中文
bool includeChinese(string s) {
    int flag=0;
    for(int i=0; i<s.size()-1; i++) {
        if(s[i]&0x80&&s[i]+1&0x80) {
            flag=1;
            break;
        }
    }
    if(flag) return true;
    return false;
}


/**
 * 插入单词
 * @param word_group
 * @param C_meaning
 * @param source
 * @return 插入的wid
 */
int InsertWord(string word_group,string C_meaning,string source){
//    cout<<word_group;
    try {
        /* insert couple of rows of data into City table using Prepared Statements */
        prep_stmt = con -> prepareStatement ("insert into words(word_group,C_meaning,source)values((?),(?),(?));");
        //插入数据
        prep_stmt -> setString (1, word_group);
        prep_stmt -> setString (2, C_meaning);
        prep_stmt -> setString (3, source);
        updatecount = prep_stmt -> executeUpdate();
        con -> commit();

        stmt = con -> createStatement();
        //获取刚插入的id
        res = stmt -> executeQuery ("select @@identity as id;");
        if(res->next()){
            return StringToInt(res->getString(1));
        }
    } catch (SQLException &e) {
        cout << "ERROR: SQLException in " << __FILE__;
        cout << " (" << __func__<< ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047) {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return EXIT_FAILURE;
    }
}

/**
 * 插入例句
 * @param word_meaning
 * @param E_sentence
 * @param C_translate
 * @param wid
 * @param source
 * @return
 */
int InsertExample(string word_meaning,string E_sentence,string C_translate,string wid,string source){
    try {
        /* insert couple of rows of data into City table using Prepared Statements */
        prep_stmt = con -> prepareStatement ("insert into example(word_meaning,E_sentence,C_translate,wid,source)values((?),(?),(?),(?),(?));");

        prep_stmt -> setString (1, word_meaning);
        prep_stmt -> setString (2, E_sentence);
        prep_stmt -> setString (3, C_translate);
        prep_stmt -> setString (4, wid);
        prep_stmt -> setString (5, source);
        updatecount = prep_stmt -> executeUpdate();
        con -> commit();
    } catch (SQLException &e) {
        cout << "ERROR: SQLException in " << __FILE__;
        cout << " (" << __func__<< ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047) {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return EXIT_FAILURE;
    }
}

/**
 * 关闭数据库
 */
int CloseDataBase(){
    try{
        /* Clean up */
        delete res;
        delete stmt;
        delete prep_stmt;
        con -> close();
        delete con;
    } catch (SQLException &e) {
        cout << "ERROR: SQLException in " << __FILE__;
        cout << " (" << __func__<< ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047) {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return EXIT_FAILURE;
    }
}


string IntToString(int n){
    if(n==0) return "0";
    int a;
    string s="";
    while(n){
        a=n%10;
        s=(char)(a+48)+s;
        n/=10;
    }
    return s;
}

int StringToInt(string s){
    int ans=0;
    for(int i=0;i<s.size();i++){
        ans=ans*10+(s[i]-'0');
    }
    return ans;

}
int main ()
{
    Cgicc cgi;
    cout << "Content-type:text/html\r\n\r\n";
    //连接数据库
    ConnectDataBase();

    //获取uid，代表来源
    form_iterator uid = cgi.getElement("uid");
//    cout<<"uid:"<<**uid;
    // 获取要被上传的文件列表
    // 上传的TXT文件中的中文可能会出现乱码的现象，可以引导用户将文件编码改成utf-8的
    const_file_iterator file = cgi.getFile("wordFile");
    if(file != cgi.getFiles().end()) {
        //将文件转换为字符流
        stringstream fileString(file->getData());
        string word_en,word_ch,word_meaning,E_sentence,C_translate;
        bool flag=false;
        int last_insert_id;
        //读入word_en
        getline(fileString,word_en,'\n');
        while(getline(fileString,word_ch),'\n') {
            //向数据库插入单词
            last_insert_id = InsertWord(word_en,word_ch,**uid);
            //判断是否还有数据
            if(!getline(fileString,word_en,'\n')){
                break;
            }
            //循环插入多个例句
            while(includeChinese(word_en)){//根据是否包含中文，来判断是例句还是单词
                word_meaning=word_en;
                getline(fileString,E_sentence,'\n');
                getline(fileString,C_translate,'\n');
                //insert a example sentence;
                InsertExample(word_meaning,E_sentence,C_translate,IntToString(last_insert_id),**uid);
                //判断是否还有数据
                if(!getline(fileString,word_en,'\n')){
                    flag=true;
                    break;
                }
            }
            if(flag) break;
        }
    }
    CloseDataBase();//关闭数据库
    return 0;
}

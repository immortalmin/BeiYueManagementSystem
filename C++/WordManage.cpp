//
// Created by Administrator on 2020/12/3.
//

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
//#include <windows.h>
#include <unistd.h>

#include "../cgicc-3.2.16/cgicc/CgiDefs.h"
#include "../cgicc-3.2.16/cgicc/Cgicc.h"
#include "../cgicc-3.2.16/cgicc/HTTPHTMLHeader.h"
#include "../cgicc-3.2.16/cgicc/HTMLClasses.h"

#include "../include/json/json.h"

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


#define NUMOFFSET 100
#define COLNAME 200

using namespace std;
using namespace cgicc;
using namespace sql;

Driver *driver;
Connection *con;
Statement *stmt;
ResultSet *res;
PreparedStatement *prep_stmt;
Savepoint *savept;

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

        /* alternate syntax using auto_ptr to create the db connection */
        //auto_ptr  con (driver -> connect(url, user, password));

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

/**
 * 查询当前页码的所有单词
 * @param curPage 当前页码
 * @param pageSize 每页数据的条数
 * @param dict_source 词典来源
 * @return
 */
int QueryAllWords(string curPage,string pageSize,string dict_source){
    try{
        /* create a statement object */
        stmt = con -> createStatement();
        if(dict_source=="0"){//恋练不忘
            /* run a query which returns exactly one result set */
            res = stmt -> executeQuery ("select wid,word_group,C_meaning from words where source=1 limit "+IntToString((StringToInt(curPage)-1)*StringToInt(pageSize))+","+pageSize);
            /* retrieve the data from the result set and display on stdout */
            Json::Value word_list;
            while(res->next()){
                Json::Value word;
                word["wid"] = (string)res->getString(1);
                word["word_group"] = (string)res->getString(2);
                word["C_meaning"] = (string)res->getString(3);
                word_list.append(word);
            }
            Json::StyledWriter sw;
            cout << sw.write(word_list) << endl << endl;
        }else if(dict_source=="1"){//柯林斯词典
            /* run a query which returns exactly one result set */
            res = stmt -> executeQuery ("select wid,word_en,word_ch,star from k_words limit "+IntToString((StringToInt(curPage)-1)*StringToInt(pageSize))+","+pageSize);
            /* retrieve the data from the result set and display on stdout */
            Json::Value word_list;
            while(res->next()){
                Json::Value word;
                word["wid"] = (string)res->getString(1);
                word["word_en"] = (string)res->getString(2);
                word["word_ch"] = (string)res->getString(3);
                word["star"] = (string)res->getString(4);
                word_list.append(word);
            }
            Json::StyledWriter sw;
            cout << sw.write(word_list) << endl << endl;
        }else if(dict_source=="2"){
            /* run a query which returns exactly one result set */
            res = stmt -> executeQuery ("select wid,word_group,C_meaning from words where source!=1 limit "+IntToString((StringToInt(curPage)-1)*StringToInt(pageSize))+","+pageSize);
            /* retrieve the data from the result set and display on stdout */
            Json::Value word_list;
            while(res->next()){
                Json::Value word;
                word["wid"] = (string)res->getString(1);
                word["word_group"] = (string)res->getString(2);
                word["C_meaning"] = (string)res->getString(3);
                word_list.append(word);
            }
            Json::StyledWriter sw;
            cout << sw.write(word_list) << endl << endl;
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
 * 查询单个单词的数据
 * @param dict_source 来源
 * @param wid 单词ID
 * @return
 */
int QuerySingleWord(string dict_source,string wid){
    try{
        /* create a statement object */
        stmt = con -> createStatement();
        if(dict_source=="0"){//恋练不忘&其他
            /* run a query which returns exactly one result set */
            res = stmt -> executeQuery ("select wid,word_group as 'word_en',C_meaning as 'word_ch',user.`username` as 'source' from words join user where words.`source`=user.`uid` and wid= "+wid);
            /* retrieve the data from the result set and display on stdout */
            Json::Value word;
            if(res->next()){
                word["wid"] = (string)res->getString(1);
                word["word_en"] = (string)res->getString(2);
                word["word_ch"] = (string)res->getString(3);
                word["source"] = (string)res->getString(4);
            }
            Json::StyledWriter sw;
            cout << sw.write(word) << endl << endl;
        }else if(dict_source=="1"){//柯林斯词典
            /* run a query which returns exactly one result set */
            res = stmt -> executeQuery ("select wid,word_en,star from k_words where wid= "+wid);
            /* retrieve the data from the result set and display on stdout */
            Json::Value word;
            if(res->next()){
                word["wid"] = (string)res->getString(1);
                word["word_en"] = (string)res->getString(2);
                word["star"] = (string)res->getString(3);
            }
            Json::StyledWriter sw;
            cout << sw.write(word) << endl << endl;
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
 * 查询例句
 * @param dict_source 词典来源
 * @param wid 单词ID
 * @return
 */
int QueryExampleSentence(string dict_source,string wid){
    try{
        /* create a statement object */
        stmt = con -> createStatement();
        if(dict_source=="0"){//恋练不忘or由用户添加的单词
            /* run a query which returns exactly one result set */
            res = stmt -> executeQuery ("select eid,word_meaning,E_sentence,C_translate,user.`username` as 'source' from example join user where example.`source`=user.`uid` and wid="+wid);
            /* retrieve the data from the result set and display on stdout */
            Json::Value sentences;
            while(res->next()){
                Json::Value sentence;
                sentence["eid"] = (string)res->getString(1);
                sentence["word_meaning"] = (string)res->getString(2);
                sentence["E_sentence"] = (string)res->getString(3);
                sentence["C_translate"] = (string)res->getString(4);
                sentence["source"] = (string)res->getString(5);
                sentences.append(sentence);
            }
            Json::StyledWriter sw;
            cout << sw.write(sentences) << endl << endl;
        }else if(dict_source=="1"){//柯林斯词典
            /* run a query which returns exactly one result set */
            res = stmt -> executeQuery ("select iid,number,label,word_ch,explanation,gram from k_items where wid="+wid);
            /* retrieve the data from the result set and display on stdout */
            Json::Value items;
            while(res->next()){
                Json::Value item;
                item["iid"] = (string)res->getString(1);
                item["number"] = (string)res->getString(2);
                item["label"] = (string)res->getString(3);
                item["word_ch"] = (string)res->getString(4);
                item["explanation"] = (string)res->getString(5);
                item["gram"] = (string)res->getString(6);
                ResultSet *res2 = stmt->executeQuery("select sid,sentence_ch,sentence_en from k_sentences where iid="+(string)res->getString(1));
                Json::Value sentences;
                while(res2->next()){
                    Json::Value sentence;
                    sentence["sid"] = (string)res2->getString(1);
                    sentence["sentence_ch"] = (string)res2->getString(2);
                    sentence["sentence_en"] = (string)res2->getString(3);
                    sentences.append(sentence);
                }
                item["sentences"]=sentences;
                items.append(item);
            }
            Json::StyledWriter sw;
            cout << sw.write(items) << endl << endl;
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
 * 查询共有多少条记录
 */
int QueryTotalCount(string dict_source){
    try{
        /* create a statement object */
        stmt = con -> createStatement();

        /* run a query which returns exactly one result set */
        if("0"==dict_source){
            res = stmt -> executeQuery ("select count(*) from words where source=1;");
        }else if("1"==dict_source){
            res = stmt -> executeQuery ("select count(*) from k_words;");
        }else if("2"==dict_source){
            res = stmt -> executeQuery ("select count(*) from words where source!=1;");
        }

        /* retrieve the data from the result set and display on stdout */
        while(res->next()){
            cout<<res->getString(1)<<endl;
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
    Cgicc formData;
    cout << "Content-type:text/html\r\n\r\n";
    //连接数据库
    ConnectDataBase();

    form_iterator fi = formData.getElement("what");
    if( !fi->isEmpty() && fi != (*formData).end()) {
        if("0"==**fi){//查询所有的单词
            form_iterator curPage = formData.getElement("curPage");
            form_iterator pageSize = formData.getElement("pageSize");
            form_iterator dict_source = formData.getElement("dict_source");
            QueryAllWords(**curPage,**pageSize,**dict_source);
        }else if("1"==**fi){//获取单词的数量
            form_iterator dict_source = formData.getElement("dict_source");
            QueryTotalCount(**dict_source);
        }else if("2"==**fi){//获取单个单词的数据
            form_iterator dict_source = formData.getElement("dict_source");
            form_iterator wid = formData.getElement("wid");
            QuerySingleWord(**dict_source,**wid);
        }else if("3"==**fi){//查询例句
            form_iterator dict_source = formData.getElement("dict_source");
            form_iterator wid = formData.getElement("wid");
            QueryExampleSentence(**dict_source,**wid);
        }
    }else{
        cout << "No text entered for first name" << endl;
    }



    //关闭数据库
    CloseDataBase();

    return 0;
}



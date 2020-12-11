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
 * 查询数据
 */
int QueryData(string curPage,string pageSize){
    try{
        /* create a statement object */
        stmt = con -> createStatement();

        /* run a query which returns exactly one result set */
        res = stmt -> executeQuery ("SELECT eid AS \"fid\",user.`uid`,username,profile_photo,phone_model,description,contact,add_time,progress,"
                                    "img_path,\"0\" as \"what\" FROM error_feedback INNER JOIN user where error_feedback.`uid`=user.`uid` "
                                    "UNION SELECT fid,user.`uid`,username,profile_photo,\"null\" AS \"phone_model\",description,contact,add_time,"
                                    "progress,img_path,\"1\" as \"what\" FROM feature_suggestions INNER JOIN user where "
                                    "feature_suggestions.`uid`=user.`uid` order by add_time desc limit "+IntToString((StringToInt(curPage)-1)*StringToInt(pageSize))+","+pageSize);
        /* retrieve the data from the result set and display on stdout */
        Json::Value word_list;
        while(res->next()){
            Json::Value word;
            word["fid"] = (string)res->getString(1);
            word["uid"] = (string)res->getString(2);
            word["username"] = (string)res->getString(3);
            word["profile_photo"] = (string)res->getString(4);
            word["phone_model"] = (string)res->getString(5);
            word["description"] = (string)res->getString(6);
            word["contact"] = (string)res->getString(7);
            word["add_time"] = (string)res->getString(8);
            word["progress"] = (string)res->getString(9);
            word["img_path"] = (string)res->getString(10);
            word["what"] = (string)res->getString(11);
            word_list.append(word);
        }
        Json::StyledWriter sw;
        cout << sw.write(word_list) << endl << endl;

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
int QueryTotalCount(){
    try{
        /* create a statement object */
        stmt = con -> createStatement();

        /* run a query which returns exactly one result set */
        res = stmt -> executeQuery ("select sum(table_rows) from information_schema.tables where table_name=\"error_feedback\" or table_name=\"feature_suggestions\";");
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
 * 修改进度
 * @param fid ID
 * @param type 0：错误反馈；1：功能建议
 * @param value 进度
 * @return
 */
void UpdateProgress(string fid,string type,string progress){
    try{
        /* create a statement object */
        stmt = con -> createStatement();

        /* run a query which returns exactly one result set */
        if(type=="0"){//错误反馈
            prep_stmt = con -> prepareStatement ("UPDATE error_feedback SET progress=(?) WHERE eid=(?)");
            prep_stmt -> setString (1, progress);
            prep_stmt -> setString (2, fid);
            updatecount = prep_stmt -> executeUpdate();
//            cout<<"受影响的行数："<<updatecount<<endl;
        }else{//功能建议
            prep_stmt = con -> prepareStatement ("UPDATE feature_suggestions SET progress=(?) WHERE fid=(?)");
            prep_stmt -> setString (1, progress);
            prep_stmt -> setString (2, fid);
            updatecount = prep_stmt -> executeUpdate();
//            cout<<"受影响的行数："<<updatecount<<endl;
        }
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

        return ;
    }
}


/**
 * 删除数据
 * @param fid
 */
void DeleteFeedback(string fid,string type){
    try{
        /* create a statement object */
        stmt = con -> createStatement();
        if("0"==type){
            prep_stmt = con -> prepareStatement ("DELETE FROM error_feedback WHERE eid=(?);");
            prep_stmt -> setString (1, fid);
        }else{
            prep_stmt = con -> prepareStatement ("DELETE FROM feature_suggestions WHERE fid=(?);");
            prep_stmt -> setString (1, fid);
        }
        updatecount = prep_stmt -> executeUpdate();
        con -> commit();
        //删除文件夹下的图片
        if("0"==type){
            int num=0;
            string s = "/var/www/word/img/feedback/e_"+fid+"_"+IntToString(num)+".jpg";
            const char* file = s.data();
            while(!access(file,0)){//access:文件存在返回0，不存在返回-1
                remove(file);
                num++;
                s = "/var/www/word/img/feedback/e_"+fid+"_"+IntToString(num)+".jpg";
                file = s.data();
            }
        }else{
            int num=0;
            string s = "/var/www/word/img/feedback/f_"+fid+"_"+IntToString(num)+".jpg";
            const char* file = s.data();
            while(!access(file,0)){//access:文件存在返回0，不存在返回-1
                remove(file);
                num++;
                s = "/var/www/word/img/feedback/f_"+fid+"_"+IntToString(num)+".jpg";
                file = s.data();
            }
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

        return ;
    }
}

int InsertData(){
    try {
        /* insert couple of rows of data into City table using Prepared Statements */
        prep_stmt = con -> prepareStatement ("INSERT INTO City (CityName) VALUES (?)");

        cout << "\tInserting \"London, UK\" into the table, City .." << endl;

        prep_stmt -> setString (1, "London, UK");
        updatecount = prep_stmt -> executeUpdate();

        cout << "\tCreating a save point \"SAVEPT1\" .." << endl;
        savept = con -> setSavepoint ("SAVEPT1");

        cout << "\tInserting \"Paris, France\" into the table, City .." << endl;

        prep_stmt -> setString (1, "Paris, France");
        updatecount = prep_stmt -> executeUpdate();

        cout << "\tRolling back until the last save point \"SAVEPT1\" .." << endl;
        con -> rollback (savept);
        con -> releaseSavepoint (savept);

        cout << "\tCommitting outstanding updates to the database .." << endl;
        con -> commit();

        cout << "\nQuerying the City table again .." << endl;

        /* re-use result set object */
        res = NULL;
        res = stmt -> executeQuery ("SELECT * FROM City");
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
        if("0"==**fi){//查询所有的反馈
            form_iterator curPage = formData.getElement("curPage");
            form_iterator pageSize = formData.getElement("pageSize");
            QueryData(**curPage,**pageSize);
        }else if("1"==**fi){//修改进度
            form_iterator fid = formData.getElement("fid");
            form_iterator type = formData.getElement("type");
            form_iterator progress = formData.getElement("progress");
            UpdateProgress(**fid,**type,**progress);
        }else if("2"==**fi){//删除反馈
            form_iterator fid = formData.getElement("fid");
            form_iterator type = formData.getElement("type");
            DeleteFeedback(**fid,**type);
        }else if("3"==**fi){
            QueryTotalCount();
        }
    }else{
        cout << "No text entered for first name" << endl;
    }



    //关闭数据库
    CloseDataBase();

    return 0;
}
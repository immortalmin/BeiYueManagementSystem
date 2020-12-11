#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "cgicc-3.2.16/cgicc/CgiDefs.h"
#include "cgicc-3.2.16/cgicc/Cgicc.h"
#include "cgicc-3.2.16/cgicc/HTTPHTMLHeader.h"
#include "cgicc-3.2.16/cgicc/HTMLClasses.h"

#include "include/json/json.h"

/* MySQL Connector/C++ specific headers （注意：所有头文件都放在/usr/include/cppconn/下面）*/
#include "include/cppconn/driver.h"
#include "include/cppconn/connection.h"
#include "include/cppconn/statement.h"
#include "include/cppconn/prepared_statement.h"
#include "include/cppconn/resultset.h"
#include "include/cppconn/metadata.h"
#include "include/cppconn/resultset_metadata.h"
#include "include/cppconn/exception.h"
#include "include/cppconn/warning.h"
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

void ShowJsonData(){
    //显示json格式的数据，返回给Ajax
    Json::Value root;
    Json::Value word_data;
    word_data["id"] = 0;
    word_data["eword"] = "zero";
    root.append(word_data);
    word_data["id"] = 1;
    word_data["eword"] = "one";
    root.append(word_data);
    word_data["id"] = 2;
    word_data["eword"] = "two";
    root.append(word_data);
    Json::StyledWriter sw;
    cout << sw.write(root) << endl << endl;
}

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
int QueryData(){
    try{
        /* select appropriate database schema */
//        con -> setSchema("user");

        /* create a statement object */
        stmt = con -> createStatement();

        /* run a query which returns exactly one result set */
        res = stmt -> executeQuery ("SELECT * FROM words limit 5");
        /* retrieve the data from the result set and display on stdout */
        Json::Value word_list;
        while(res->next()){
//            cout<<res->getString(1)<<" "<<res->getString(2)<<endl;
            Json::Value word;
            word["wid"] = (string)res->getString(1);
            word["word_group"] = (string)res->getString(2);
            word["C_meaning"] = (string)res->getString(3);
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


int main ()
{
//    Cgicc formData;
    cout << "Content-type:text/html\r\n\r\n";
    //连接数据库
    ConnectDataBase();

    QueryData();

    //关闭数据库
    CloseDataBase();

    return 0;
}
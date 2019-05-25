#include <iostream>
#include <cstring>
#include <exception>
#include <iterator>
#include <vector>

#include "nanodbc/nanodbc.h"
#include "utils.h"

#define NANODBC_TIMEOUT 10 // Timeout is 10 sec

#define ODBC_DRIVER	"SQL Server Native Client 11.0"
#define ODBC_SERVER	"SERVER_HERE"
#define ODBC_DB		"DBNAME_HERE"
#define ODBC_USER	"USERNAME_HERE"
#define ODBC_PASSWD	"PASSWORD_HERE"

using namespace std;

int main()
{
	/*
		ConnectionStrings.com

		Driver={SQL Server Native Client 11.0};Server=myServerAddress;
		Database=myDataBase;Uid=myUsername;Pwd=myPassword;
	*/

	setlocale(LC_ALL, "Russian");
	try {
		/* Connect to SQL Server */
		string connection_string = format("Driver={%s};Server={%s};Database={%s};Uid={%s};Pwd={%s};", ODBC_DRIVER, ODBC_SERVER, ODBC_DB, ODBC_USER, ODBC_PASSWD);
		nanodbc::connection connection(connection_string, NANODBC_TIMEOUT);

		if (connection.connected()) {
			printf("Connected!\n");
		}
		else {
			printf("Unable to connect to server\n");
			return EXIT_FAILURE;
		}

		/* Exec queries */
		nanodbc::result res;
		std::vector<std::string> received_tables;

		// get all tables from DB
		res = nanodbc::execute(connection, "SELECT * FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_TYPE='BASE TABLE';"); // execute query & place result to "(nanodbc::result) res"

		// retrieve available tables & save them in "received_tables"
		if (res.columns() > 0) {
			try {
				short selected_column = res.column("TABLE_NAME"); // there may be a error, so use try for handle it

				while (res.next()) {
					//printf("%s\n", res.get<nanodbc::string>(selected_column, "null").c_str());
					received_tables.push_back(res.get<std::string>(selected_column));
				}
			}
			catch (...) {
				printf("Error handled on %s:%d\n", __FILE__, __LINE__);
			}
			
			// print received columns from result
			/*for (short i = 0; i < res.columns(); i++) {
				printf("%s%s", res.column_name(i).c_str(), i != res.columns() - 1 ? ", " : "\n");
			}*/
		}
		else {
			printf("Not found available columns\n");
			return EXIT_FAILURE;
		}

		// print all available tables
		printf("Available tables: ");
		if (received_tables.size() <= 0) printf("tables not found\n");
		for (size_t i = 0; i < received_tables.size(); i++) {
			printf("%s%s", received_tables[i].c_str(), i != received_tables.size() - 1 ? ", " : "\n");
		}

		// get all rows from received tables
		const char* split_chars = "\t\t";
		string builded_query = "";
		int tmp_row_processed = 0;

		for (vector<string>::iterator it = received_tables.begin(); it != received_tables.end(); it++) {
			builded_query = format("SELECT * FROM [%s];", (*it).c_str()); // SELECT * FROM [Students];
			res = nanodbc::execute(connection, builded_query.c_str());

			printf("Available data for \"%s\":\n", (*it).c_str());
			for (short i = 0; i < res.columns(); i++) {
				printf("%s%s", res.column_name(i).c_str(), i != res.columns() - 1 ? split_chars : "\n");
			}

			tmp_row_processed = 0;
			while (true) {
				// Attention! Important to read the result to the end, else you can get the error
				if (res.next()) {
					tmp_row_processed++;
					for (short i = 0; i < res.columns(); i++) {
						printf("%ws%s", res.get<std::wstring>(i).c_str(), i != res.columns() - 1 ? split_chars : "\n"); // Attention! Here is an error, if the field has type "nvarchar(max)"
					}
				}
				else {
					if (!tmp_row_processed && res.rows() <= 0) printf("-- No data --\n");
					break;
				}
			}
			printf("\n");
		}

		/* Disconnect */
		connection.disconnect();
	}
	catch (exception& ex) {
		printf("%s\n", ex.what()); // ODBC Driver errors
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
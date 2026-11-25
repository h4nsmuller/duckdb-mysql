//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/mysql_table_set.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"

#include "storage/mysql_catalog_set.hpp"
#include "storage/mysql_table_entry.hpp"

namespace duckdb {
struct CreateTableInfo;
class MySQLConnection;
class MySQLResult;
class MySQLSchemaEntry;

class MySQLTableSet : public MySQLInSchemaSet {
public:
	explicit MySQLTableSet(MySQLSchemaEntry &schema);

public:
	optional_ptr<CatalogEntry> CreateTable(ClientContext &context, MySQLTransaction &transaction,
	                                       BoundCreateTableInfo &info);

	static unique_ptr<MySQLTableInfo> GetTableInfo(ClientContext &context, MySQLTransaction &transaction,
	                                               MySQLSchemaEntry &schema, const string &table_name);
	optional_ptr<CatalogEntry> RefreshTable(ClientContext &context, MySQLTransaction &transaction,
	                                        const string &table_name);

	void AlterTable(ClientContext &context, MySQLTransaction &transaction, AlterTableInfo &info);

protected:
	void LoadEntries(ClientContext &context, MySQLTransaction &transaction) override;

	void AlterTable(ClientContext &context, MySQLTransaction &transaction, RenameTableInfo &info);
	void AlterTable(ClientContext &context, MySQLTransaction &transaction, RenameColumnInfo &info);
	void AlterTable(ClientContext &context, MySQLTransaction &transaction, AddColumnInfo &info);
	void AlterTable(ClientContext &context, MySQLTransaction &transaction, RemoveColumnInfo &info);

	static void AddColumn(MySQLTransaction &transaction, MySQLResult &result, MySQLTableInfo &table_info,
	                      idx_t column_offset = 0);
};

} // namespace duckdb

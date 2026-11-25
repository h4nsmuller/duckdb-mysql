//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/mysql_index_set.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "storage/mysql_catalog_set.hpp"

namespace duckdb {
class MySQLSchemaEntry;

class MySQLIndexSet : public MySQLInSchemaSet {
public:
	MySQLIndexSet(MySQLSchemaEntry &schema);

	void DropEntry(ClientContext &context, MySQLTransaction &transaction, DropInfo &info) override;

protected:
	void LoadEntries(ClientContext &context, MySQLTransaction &transaction) override;
};

} // namespace duckdb

//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/mysql_catalog_set.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"
#include "duckdb/catalog/catalog.hpp"
#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/common/mutex.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/transaction/transaction.hpp"

namespace duckdb {
struct DropInfo;
class MySQLSchemaEntry;
class MySQLTransaction;

class MySQLCatalogSet {
public:
	MySQLCatalogSet(Catalog &catalog);

	optional_ptr<CatalogEntry> GetEntry(ClientContext &context, MySQLTransaction &transaction, const string &name);
	virtual void DropEntry(ClientContext &context, MySQLTransaction &transaction, DropInfo &info);
	void Scan(ClientContext &context, MySQLTransaction &transaction,
	          const std::function<void(CatalogEntry &)> &callback);
	virtual optional_ptr<CatalogEntry> CreateEntry(MySQLTransaction &transaction, shared_ptr<CatalogEntry> entry);
	void ClearEntries();

protected:
	virtual void LoadEntries(ClientContext &context, MySQLTransaction &transaction) = 0;

	void TryLoadEntries(ClientContext &context, MySQLTransaction &transaction);

	void EraseEntryInternal(const string &name);

protected:
	Catalog &catalog;

private:
	optional_ptr<CatalogEntry> TryGetEntry(MySQLTransaction &transaction, const string &name);

	void ClearEntriesNoLock();

private:
	// lock order -> clear, load, entry
	mutex entry_lock;
	mutex load_lock;
	mutex clear_lock;
	case_insensitive_map_t<shared_ptr<CatalogEntry>> entries;
	bool is_loaded = false;
};

class MySQLInSchemaSet : public MySQLCatalogSet {
public:
	MySQLInSchemaSet(MySQLSchemaEntry &schema);

	optional_ptr<CatalogEntry> CreateEntry(MySQLTransaction &transaction, shared_ptr<CatalogEntry> entry) override;

protected:
	MySQLSchemaEntry &schema;
};

} // namespace duckdb

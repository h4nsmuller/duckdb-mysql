#include "storage/mysql_transaction.hpp"

#include "storage/mysql_catalog.hpp"

namespace duckdb {

MySQLTransaction::MySQLTransaction(MySQLCatalog &mysql_catalog, TransactionManager &manager, Context &context)
    : Transaction(manager, context), catalog(mysql_catalog), transactions_enabled(GetTransactionsEnabled(context)),
      access_mode(mysql_catalog.access_mode), conn_init_opts(GetConnectionInitOptions(context)) {
}

void MySQLTransaction::Start() {
	// no-op
}

void MySQLTransaction::Commit(ClientContext &context) {
	if (!pooled_connection || !transactions_enabled) {
		return;
	}

	lock_guard<mutex> guard(transaction_lock);

	if (transaction_state == MySQLTransactionState::TRANSACTION_STARTED) {
		try {
			pooled_connection.GetConnection().Execute(context, "COMMIT");
			transaction_state = MySQLTransactionState::TRANSACTION_FINISHED;
		} catch (...) {
			pooled_connection.Invalidate();
			throw;
		}
	}
}

void MySQLTransaction::Rollback(ClientContext &context) {
	if (!pooled_connection || !transactions_enabled) {
		return;
	}

	lock_guard<mutex> guard(transaction_lock);

	if (transaction_state == MySQLTransactionState::TRANSACTION_STARTED) {
		try {
			pooled_connection.GetConnection().Execute(context, "ROLLBACK");
			transaction_state = MySQLTransactionState::TRANSACTION_FINISHED;
		} catch (...) {
			pooled_connection.Invalidate();
			throw;
		}
	}
}

MySQLConnection &MySQLTransaction::GetConnection(ClientContext &context) {
	lock_guard<mutex> guard(transaction_lock);

	if (!pooled_connection) {
		if (pooled_connection.Id() > 0) {
			throw IOException("Remote connection, ID: %llu, of this transaction was invalidated",
			                  pooled_connection.Id());
		}
		this->pooled_connection = catalog.GetConnectionPool().Acquire();
		try {
			pooled_connection.GetConnection().Initialize(context, conn_init_opts);
		} catch (...) {
			pooled_connection.Invalidate();
			throw;
		}
	}

	if (transactions_enabled && transaction_state == MySQLTransactionState::TRANSACTION_NOT_YET_STARTED) {
		string query = "START TRANSACTION";
		if (access_mode == AccessMode::READ_ONLY) {
			query += " READ ONLY";
		}
		try {
			pooled_connection.GetConnection().Execute(context, query);
			transaction_state = MySQLTransactionState::TRANSACTION_STARTED;
		} catch (...) {
			pooled_connection.Invalidate();
			throw;
		}
	}

	return pooled_connection.GetConnection();
}

uint64_t MySQLTransaction::GetConnectionId() {
	return pooled_connection.Id();
}

MySQLTransaction &MySQLTransaction::Get(ClientContext &context, Catalog &catalog) {
	return Transaction::Get(context, catalog).Cast<MySQLTransaction>();
}

bool MySQLTransaction::GetTransactionsEnabled(Context &context) {
	Value val;
	if (context.TryGetCurrentSetting("mysql_enable_transactions", val)) {
		return BooleanValue::Get(val);
	}
	return true;
}

MySQLConnectionInitOptions MySQLTransaction::GetConnectionInitOptions(Context &context) {
	MySQLConnectionInitOptions res;
	res.type_config = MySQLTypeConfig(context);
	Value val;
	if (context.TryGetCurrentSetting("mysql_session_time_zone", val)) {
		res.time_zone = StringValue::Get(val);
	}
	return res;
}

optional_ptr<CatalogEntry> MySQLTransaction::ReferenceEntry(shared_ptr<CatalogEntry> &entry) {
	auto &ref = *entry;
	referenced_entries.emplace(ref, entry);
	return ref;
}

} // namespace duckdb

#include "zenlang.hpp"
#if !defined(NOSQLITE3)

namespace zz {
    inline z::string error(const z::Sqlite3::Database& db) {
        return sqlite3_errmsg(db.impl()._db);
    }
}

void z::Sqlite3::Database::Impl::close() {
    if(_db) {
        int err = SQLITE_BUSY;
        for(int i = 0; ((i < 10) && (err == SQLITE_BUSY)); ++i) {
            err = sqlite3_close(_db);
        }
        if(err > 0)
            throw z::Exception("Sqlite", z::string("Error closing database"));
    }
    _db = 0;
}

int z::Sqlite3::Statement::Impl::open(const z::Sqlite3::Database& db, const z::string& sql) {
    int rc = sqlite3_prepare(db.impl()._db, z::s2e(sql).c_str(), (int)z::s2e(sql).size(), &(_stmt), 0);
    if(rc != SQLITE_OK) {
        throw z::Exception("Sqlite3", z::string("Statement():[%{t}] %{s}").arg("t", sql).arg("s", zz::error(db)));
    }
    return rc;
}

int z::Sqlite3::Statement::Impl::reset() {
    return sqlite3_reset(_stmt);
}

int z::Sqlite3::Statement::Impl::close() {
    if(_stmt) {
        sqlite3_reset(_stmt);
        sqlite3_clear_bindings(_stmt);
        sqlite3_finalize(_stmt);
    }
    _stmt = 0;
    _db = 0;
    return 0;
}

namespace zz {
    inline int execStmt(const z::Sqlite3::Database& db, const z::Sqlite3::Statement& stmt) {
        int rc = sqlite3_step(stmt.impl()._stmt);
        if((rc > 0) && (rc < 100)) {
            throw z::Exception("Sqlite3", z::string("execStmt(): (%{rc})%{s}").arg("rc",rc).arg("s", zz::error(db)));
        }
        return rc;
    }

    inline int execRaw(const z::Sqlite3::Database& db, const z::string& stmt) {
        z::Sqlite3::Statement s;
        z::Sqlite3::PrepareStatement(db, s, stmt);
        return execStmt(db, s);
    }
}

void z::Sqlite3::Transaction::Impl::begin(Database& db) {
    assert(_db == 0);
    if(SQLITE_DONE != zz::execRaw(db, "BEGIN EXCLUSIVE;")) {
        throw z::Exception("Sqlite3", z::string("begin(): %{s}").arg("s", zz::error(db)));
    }
    _db = z::ptr(db);
}

void z::Sqlite3::Transaction::Impl::commit() {
    assert(_db);
    if(SQLITE_DONE != zz::execRaw(z::ref(_db), "COMMIT;")) {
        throw z::Exception("Sqlite3", z::string("commit(): %{s}").arg("s", zz::error(z::ref(_db))));
    }
    _db = 0;
}

void z::Sqlite3::Transaction::Impl::rollback() {
    assert(_db);
    if(SQLITE_DONE != zz::execRaw(z::ref(_db), "ROLLBACK;")) {
        throw z::Exception("Sqlite3", z::string("rollback(): %{s}").arg("s", zz::error(z::ref(_db))));
    }
    _db = 0;
}

void z::Sqlite3::Open(z::Sqlite3::Database& db, const z::string& filename) {
    if(db.impl()._db != 0) {
        Close(db);
    }

    const char* vfs = 0;
    int f = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;

    sqlite3* sdb;
    int rc = sqlite3_open_v2(z::s2e(filename).c_str(), &sdb, f, vfs);
    if(rc != SQLITE_OK)
        throw z::Exception("Sqlite3", z::string("Error opening database %{s}").arg("s", filename));

    sqlite3_extended_result_codes(sdb, 1);
    sqlite3_busy_timeout(sdb, 300);
    db.impl()._db = sdb;
}

void z::Sqlite3::Create(z::Sqlite3::Database& db, const z::string& filename) {
    if(db.impl()._db != 0) {
        Close(db);
    }

    if(z::Url::FileExists(filename)) {
        if(!z::Dir::RemovePath(filename)) {
            z::mlog("Sqlite3", z::string("Unable to remove file: %{s}").arg("s", filename));
        }
    }
    Open(db, filename);
}

bool z::Sqlite3::isOpen(z::Sqlite3::Database& db) {
    if(db.impl()._db) {
        return true;
    }
    return false;
}

void z::Sqlite3::Close(z::Sqlite3::Database& db) {
    if(db.impl()._db) {
        int err = SQLITE_BUSY;
        for(int i = 0; ((i < 10) && (err == SQLITE_BUSY)); ++i) {
            err = sqlite3_close(db.impl()._db);
        }
        if(err > 0)
            throw z::Exception("Sqlite3", z::string("Error closing database"));
    }
    db.impl()._db = 0;
}

void z::Sqlite3::Begin(z::Sqlite3::Database& db, z::Sqlite3::Transaction& tr) {
    tr.impl().begin(db);
}

void z::Sqlite3::Commit(z::Sqlite3::Transaction& tr) {
    tr.impl().commit();
}

void z::Sqlite3::Rollback(z::Sqlite3::Transaction& tr) {
    tr.impl().rollback();
}

int z::Sqlite3::PrepareStatement(const z::Sqlite3::Database& db, z::Sqlite3::Statement& stmt, const z::string& sql) {
    return stmt.impl().open(db, sql);
}

int z::Sqlite3::ResetStatement(const z::Sqlite3::Statement& stmt) {
    return stmt.impl().reset();
}

int z::Sqlite3::CloseStatement(const z::Sqlite3::Statement& stmt) {
    return stmt.impl().close();
}

int32_t z::Sqlite3::ExecuteStatement(const z::Sqlite3::Database& db, const z::string& stmt) {
    return zz::execRaw(db, stmt);
}

int64_t z::Sqlite3::Insert(const z::Sqlite3::Database& db, const z::Sqlite3::Statement& stmt) {
    int rc = zz::execStmt(db, stmt);
    if(rc != SQLITE_DONE) {
        throw z::Exception("Sqlite3", z::string("Insert(): %{s}").arg("s", zz::error(db)));
    }
    return sqlite3_last_insert_rowid(db.impl()._db);
}

int32_t z::Sqlite3::SetParamInt(const z::Sqlite3::Statement& stmt, const z::string& key, const int64_t& v) {
    int idx = sqlite3_bind_parameter_index(stmt.impl()._stmt, z::s2e(key).c_str());
    if(idx == 0) {
        throw z::Exception("Sqlite3", z::string("SetParamInt(): unknown key: %{s}").arg("s", key));
    }

    return sqlite3_bind_int(stmt.impl()._stmt, idx, (int)v);
}

int32_t z::Sqlite3::SetParamText(const z::Sqlite3::Statement& stmt, const z::string& key, const z::string& v) {
    int idx = sqlite3_bind_parameter_index(stmt.impl()._stmt, z::s2e(key).c_str());
    if(idx == 0) {
        throw z::Exception("Sqlite3", z::string("SetParamString(): unknown key: %{s}").arg("s", key));
    }

    z::estring es = z::s2e(v);
    return sqlite3_bind_text(stmt.impl()._stmt, idx, es.c_str(), (int)es.length(), SQLITE_TRANSIENT);
}

bool z::Sqlite3::Select(const z::Sqlite3::Database& db, const z::Sqlite3::Statement& stmt) {
    int32_t rc = zz::execStmt(db, stmt);
    return (rc == SQLITE_ROW);
}

bool z::Sqlite3::Update(const z::Sqlite3::Database& db, const z::Sqlite3::Statement& stmt) {
    int32_t rc = zz::execStmt(db, stmt);
    return (rc == SQLITE_ROW);
}

int64_t z::Sqlite3::GetColumnInt(const z::Sqlite3::Statement& stmt, const int32_t& idx) {
    int32_t value = sqlite3_column_int(stmt.impl()._stmt, idx);
    return value;
}

z::string z::Sqlite3::GetColumnText(const z::Sqlite3::Statement& stmt, const int32_t& idx) {
    const char* str = (const char*)sqlite3_column_text(stmt.impl()._stmt, idx);
    z::estring value(str);
    return z::e2s(value);
}

#endif

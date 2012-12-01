#pragma once

struct z::Sqlite3::Database::Impl {
    sqlite3* _db;
    void close();
    inline Impl() : _db(0) {}
    inline ~Impl() {
        close();
    }
};

struct z::Sqlite3::Statement::Impl {
    sqlite3* _db;
    sqlite3_stmt* _stmt;
    int open(const z::Sqlite3::Database& db, const z::string& sql);
    int reset();
    int close();
    inline Impl() : _db(0), _stmt(0) {}
    inline ~Impl() {
        close();
    }
};

struct z::Sqlite3::Transaction::Impl {
    z::Sqlite3::Database* _db;
    void begin(z::Sqlite3::Database& db);
    void commit();
    void rollback();
    inline Impl() : _db(0) {}
    inline ~Impl() {
        // if _db is not zero, commit() wasn't called, so rollback()
        if(_db)
            rollback();
    }
};

#include <QMutex>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QVector>

namespace WORM {

struct DatabaseHelper {
    struct DB {
        QSqlDatabase db_;
        bool busy_{false};
        DB(QSqlDatabase db) : db_(db) {}
    };

    static QMutex mutex;
    static QVector<DB*> vec;

    class AutoRelease {
    public:
        AutoRelease(bool* status) { status_ = status; }
        ~AutoRelease() { *status_ = false; }

    private:
        bool* status_ = nullptr;
    };

    static inline auto reset(DB*& db) { return AutoRelease(&db->busy_); }

    static inline void checkError(const QSqlQuery& query) {
        Q_ASSERT(query.lastError().type() == QSqlError::NoError);
    }

    static inline DB* getDB() {
        QMutexLocker l(&mutex);
        while (true) {
            for (auto& it : vec) {
                if (!it->busy_) {
                    it->busy_ = true;
                    return it;
                }
            }
        }
        return nullptr;
    }
};

QMutex DatabaseHelper::mutex;
QVector<DatabaseHelper::DB*> DatabaseHelper::vec;

struct ConnectionHelper {
    static inline bool connect(const QString& name,
                               const QString& type = "QMYSQL",
                               const QString& username = "root",
                               const QString& password = "123456",
                               const QString& hostname = "localhost",
                               int port = 3306,
                               int count = 1) {
        Q_ASSERT(count > 0);
        for (int i = 0; i < count; ++i) {
            QSqlDatabase db = QSqlDatabase::addDatabase(type, QString("WORM%1").arg(i));
            db.setDatabaseName(name);
            db.setUserName(username);
            db.setPassword(password);
            db.setHostName(hostname);
            db.setPort(port);
            if (!db.open()) {
                return false;
            }
            DatabaseHelper::vec.push_back(new DatabaseHelper::DB(db));
        }
        return true;
    }
};

struct SerializationHelper {
    template <typename T> static inline void serialize(const T& property, QStringList& value) {
        QVariant var = property;
        if (var.type() == QVariant::String) {
            value << QString("\'%1\'").arg(var.toString());
        } else {
            value << var.toString();
        }
    }
};

struct DeserializationHelper {
    template <typename T> static inline void deserialize(T& property, const QVariant& value) {
        Q_ASSERT(value.canConvert<T>());
        property = value.value<T>();
    }
};

class InjectionHelper {
    static QStringList extractFieldName(std::string input) {
        QStringList ret;
        QString tmpStr;
        for (char ch : std::move(input) + ",") {
            if (isalnum(ch) || ch == '_')
                tmpStr += ch;
            else if (ch == ',') {
                if (!tmpStr.isEmpty()) ret << tmpStr;
                tmpStr.clear();
            }
        }
        return ret;
    }

public:
    template <typename C, typename Fn> static inline decltype(auto) visit(C& obj, Fn fn) {
        return obj.__accept(fn);
    }

    template <typename C> static inline const QStringList& fieldNames() {
        static const QStringList fieldNames = extractFieldName(C::__FieldNames);
        return fieldNames;
    }

    template <typename C> static inline const QString& tableName() {
        static const QString tableName{C::__TableName};
        return tableName;
    }

    template <typename C> static inline const QString& fields() {
        static const QString fields = extractFieldName(C::__FieldNames).join(',');
        return fields;
    }
};

class QueryHelper {
public:
    template <typename T>
    static inline void select(QVector<T>& vec, const QString& condition = "") {
        select(vec, condition, true);
    }

    template <typename T> static inline void execute(QVector<T>& vec, const QString& cmd = "") {
        select(vec, cmd, false);
    }

    template <typename T> static inline void remove(const QString& condition = "") {
        execute(condition.trimmed().isEmpty()
                        ? QString("truncate table %1;").arg(InjectionHelper::tableName<T>())
                        : QString("delete from %1 where %2;")
                                  .arg(InjectionHelper::tableName<T>())
                                  .arg(condition));
    }

    template <typename T> static inline void update(const T& t, const QString& condition = "") {
        QStringList values;
        InjectionHelper::visit(t, [&values](auto&... args) {
            int index = 0;
            std::initializer_list<int>{(SerializationHelper::serialize(args, values),
                                        values.last().prepend(QString("%1 = ").arg(
                                                InjectionHelper::fieldNames<T>().at(index++))),
                                        0)...};
        });
        auto db = DatabaseHelper::getDB();
        auto autoRelease = DatabaseHelper::reset(db);
        DatabaseHelper::checkError(db->db_.exec(
                QString("update %1 set %2 %3;")
                        .arg(InjectionHelper::tableName<T>())
                        .arg(values.join(','))
                        .arg(condition.trimmed().isEmpty() ? "" : "where " + condition)));
    }

    template <typename T> static inline void update(const QVector<T>& vec) { insert(vec, false); }

    template <typename T> static inline void insert(const QVector<T>& vec) { insert(vec, true); }

    static inline void execute(const QString& cmd = "") {
        auto db = DatabaseHelper::getDB();
        auto autoRelease = DatabaseHelper::reset(db);
        DatabaseHelper::checkError(db->db_.exec(cmd));
    }

private:
    template <typename T> static inline void select(QVector<T>& vec, const QString& cmd, bool q) {
        vec.clear();
        auto db = DatabaseHelper::getDB();
        auto autoRelease = DatabaseHelper::reset(db);
        QSqlQuery query(q ? QString("select %1 from %2 %3;")
                                        .arg(InjectionHelper::fields<T>())
                                        .arg(InjectionHelper::tableName<T>())
                                        .arg(cmd.trimmed().isEmpty() ? "" : "where " + cmd)
                          : cmd,
                        db->db_);
        DatabaseHelper::checkError(query);
        while (query.next()) {
            T t;
            InjectionHelper::visit(t, [q, query](auto&... args) {
                int index = 0;
                std::initializer_list<int>{
                        (DeserializationHelper::deserialize(
                                 args,
                                 q ? query.value(index++)
                                   : query.value(InjectionHelper::fieldNames<T>().at(index++))),
                         0)...};
            });
            vec.push_back(t);
        }
    }

    template <typename T> static inline void insert(const QVector<T>& vec, bool insert) {
        QStringList values;
        for (auto it : vec) {
            QStringList value;
            InjectionHelper::visit(it, [&value](auto&... args) {
                std::initializer_list<int>{(SerializationHelper::serialize(args, value), 0)...};
            });
            values << QString("(%1)").arg(value.join(','));
        }
        auto db = DatabaseHelper::getDB();
        auto autoRelease = DatabaseHelper::reset(db);
        DatabaseHelper::checkError(db->db_.exec(QString("%1 into %2 (%3) values %4;")
                                                        .arg(insert ? "insert" : "replace")
                                                        .arg(InjectionHelper::tableName<T>())
                                                        .arg(InjectionHelper::fields<T>())
                                                        .arg(values.join(','))));
    }
};
}  // namespace WORM

#define ORMAP(_TABLE_NAME_, ...)                                                                   \
private:                                                                                           \
    friend class WORM::InjectionHelper;                                                            \
    template <typename FN> inline decltype(auto) __accept(FN fn) { return fn(__VA_ARGS__); }       \
    template <typename FN> inline decltype(auto) __accept(FN fn) const { return fn(__VA_ARGS__); } \
    constexpr static const char* __FieldNames = #__VA_ARGS__;                                      \
    constexpr static const char* __TableName = _TABLE_NAME_;
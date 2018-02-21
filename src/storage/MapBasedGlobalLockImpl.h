#ifndef AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
#define AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation with global lock
 *
 *
 */
class MapBasedGlobalLockImpl : public Afina::Storage {
public:
    MapBasedGlobalLockImpl(size_t max_size = 1024) : _max_size(max_size) {}
    ~MapBasedGlobalLockImpl() {}

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) const override;

private:

    using Key = std::string;
    using Value = std::string;
    using KeyValue = std::pair<Key, Value>;
    using List = std::list<KeyValue>;
    using list_const_iterator = typename List::const_iterator;
    using Map = std::unordered_map<std::reference_wrapper<const Key>, list_const_iterator, std::hash<Key>, std::equal_to<Key>>;

    const size_t _max_size;
    mutable List _cache_list;
    Map _cache_map;
    mutable std::mutex _mutex;

    bool
    _Has(const Key &) const;

    void
    _MoveHead(const Key &) const;


    void
    _RemoveTail(void);

    bool
    _Push(const Key &key, const Value &value);
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H

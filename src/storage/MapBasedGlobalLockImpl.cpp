#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_Has(key)) {
        _MoveHead(key);
        _cache_list.front().second = value;
    } else {
        return _Push(key, value);
    }
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_Has(key)) {
        return false;
    } else {
        return _Push(key, value);
    }
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_Has(key)) {
        return false;
    } else {
        _MoveHead(key);
        _cache_list.front().second = value;
        return true;
    }
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
    std::lock_guard<std::mutex> lock(_mutex);

    _cache_list.erase(_cache_map[key]);
    _cache_map.erase(key);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_Has(key)) {
        return false;
    } else {
        _MoveHead(key);
        value = _cache_list.front().second;
        return true;
    }
}

bool MapBasedGlobalLockImpl::_Has(const Key &key) const { return _cache_map.find(key) != _cache_map.end(); }

void MapBasedGlobalLockImpl::_MoveHead(const Key &key) const {
    _cache_list.splice(_cache_list.begin(), _cache_list, _cache_map.find(key)->second);
}

void MapBasedGlobalLockImpl::_RemoveTail(void) {
    _cache_map.erase(_cache_list.back().first);
    _cache_list.pop_back();
}

bool MapBasedGlobalLockImpl::_Push(const Key &key, const Value &value) {
    if (_cache_list.size() == _max_size) {
        _RemoveTail();
    }

    _cache_list.emplace_front(key, value);
    _cache_map[_cache_list.front().first] = _cache_list.cbegin();
    return true;
}

} // namespace Backend
} // namespace Afina

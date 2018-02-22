#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

MapBasedGlobalLockImpl::MapBasedGlobalLockImpl(size_t max_size) : _max_size(max_size), _size(0) {}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_Has(key)) {
        _MoveHead(key);
        _cache_list.front().second = value;
    }

    return _Push(key, value);
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_Has(key)) {
        return false;
    }

    return _Push(key, value);
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_Has(key)) {
        return false;
    }

    _MoveHead(key);
    _cache_list.front().second = value;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto key_value = _cache_map.find(key);
    _cache_list.erase(key_value->second);
    _cache_map.erase(key_value);
    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_Has(key)) {
        return false;
    }

    _MoveHead(key);
    value = _cache_list.front().second;
    return true;
}

bool MapBasedGlobalLockImpl::_Has(const Key &key) const { return _cache_map.find(key) != _cache_map.end(); }

void MapBasedGlobalLockImpl::_MoveHead(const Key &key) const {
    _cache_list.splice(_cache_list.begin(), _cache_list, _cache_map.find(key)->second);
}

void MapBasedGlobalLockImpl::_RemoveTail(void) {
    const auto &back = _cache_list.back();
    _size -= back.first.size() + back.second.size();

    _cache_map.erase(back.first);
    _cache_list.pop_back();
}

bool MapBasedGlobalLockImpl::_Push(const Key &key, const Value &value) {
    auto elem_size = key.size() + value.size();

    if (elem_size > _max_size) {
        return false;
    }

    while (_size + elem_size > _max_size) {
        _RemoveTail();
    }

    _size += elem_size;
    _cache_list.emplace_front(key, value);
    _cache_map.emplace(_cache_list.front().first, _cache_list.cbegin());
    return true;
}

} // namespace Backend
} // namespace Afina

#include "rule_factory.hpp"

using namespace godot;

RuleFactory *RuleFactory::singleton = nullptr;

void RuleFactory::clear() {
    if (cache_mutex.is_valid()) {
        cache_mutex->lock();
    }

    rule_cache.clear();
    compiling_schemas.clear();
    custom_rule_factories.clear();

    if (cache_mutex.is_valid()) {
        cache_mutex->unlock();
        cache_mutex.unref();
    }
}
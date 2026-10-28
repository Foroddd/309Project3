// src/tools/tool_registry.cpp - hash table for tools (spec 4.3).
// Uses separate chaining, FNV-1a hashing, and power-of-two buckets.

#include "tools/tool_registry.h"

#include <cstdint>
#include <utility>

ToolRegistry::ToolRegistry() : buckets_(kInitialBuckets) {}

ToolRegistry::~ToolRegistry() = default;

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL;   // FNV-1a starting value
    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;                  // FNV-1a prime
    }
    return static_cast<std::size_t>(h);
}

std::size_t ToolRegistry::index_for(std::string_view key, std::size_t buckets) const noexcept {
    return hash(key) & (buckets - 1);   // Get the bucket index
}

void ToolRegistry::rehash(std::size_t new_bucket_count) {
    std::vector<std::vector<Entry>> fresh(new_bucket_count);
    for (auto& chain : buckets_) {
        for (auto& e : chain) {
            std::size_t i = index_for(e.name, new_bucket_count);
            fresh[i].push_back(std::move(e));   // Move entries to the new buckets
        }
    }
    buckets_ = std::move(fresh);
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (!tool) return false;

    std::string name = tool->name();
    if (find(name) != nullptr) return false;   // Don't add duplicate names

    // Double the bucket count if needed
    if (static_cast<double>(count_ + 1) > kMaxLoad * static_cast<double>(buckets_.size()))
        rehash(buckets_.size() * 2);

    std::size_t i = index_for(name, buckets_.size());
    buckets_[i].push_back(Entry{std::move(name), std::move(tool)});
    ++count_;
    return true;
}

Tool* ToolRegistry::find(std::string_view name) const {
    const auto& chain = buckets_[index_for(name, buckets_.size())];
    for (const auto& e : chain) {
        if (e.name == name) return e.tool.get();
    }
    return nullptr;
}

std::size_t ToolRegistry::size() const noexcept {
    return count_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& chain : buckets_)
        for (const auto& e : chain) fn(*e.tool);
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(count_) / static_cast<double>(buckets_.size());
}

std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;
    for (const auto& chain : buckets_)
        if (chain.size() > longest) longest = chain.size();
    return longest;
}
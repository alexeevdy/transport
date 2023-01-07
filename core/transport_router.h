#include "descriptions.h"

#include <optional>

class NameId {
public:
    size_t SetId(const std::string &name) {
        if (auto it = ids_.find(name); it != ids_.end()) {
            return it->second;
        }
        size_t id = names_.size();
        ids_.emplace(name, id);
        names_.push_back(name);
        return id;
    }

    size_t GetId(const std::string &name) const {
        return ids_.at(name);
    }

    std::string GetName(size_t id) const {
        if (id < names_.size()) {
            return names_.at(id);
        }
        throw std::runtime_error("id out of bounds: " + std::to_string(id));
    }

private:
    std::unordered_map<std::string, size_t> ids_;
    std::vector<std::string> names_;
};

class TransportRouter {
public:
    void UpdateDistance(const std::string &from, const std::string &to, int distance);

    int GetDistance(const std::string &from, const std::string &to) const;

private:
    void ResizeToFit(size_t column, size_t row);

private:
    NameId stops_;
    NameId buses_;
    std::vector<std::vector<std::optional<int>>> distance_table;
};

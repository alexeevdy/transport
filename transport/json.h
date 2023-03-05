#pragma once

#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Json {

    struct Int {
        int64_t value;

        explicit Int(int64_t v) : value(v) {}
    };

    struct Bool {
        bool flag;

        explicit Bool(bool f) : flag(f) {}
    };

    class Node : public std::variant<std::vector<Node>,
            std::map<std::string, Node>,
            Int,
            double,
            Bool,
            std::string> {
    public:
        using variant::variant;

        const auto &AsArray() const {
            return std::get<std::vector<Node>>(*this);
        }

        const auto &AsMap() const {
            return std::get<std::map<std::string, Node>>(*this);
        }

        int64_t AsInt() const {
            return std::get<Int>(*this).value;
        }

        double AsDouble() const {
            return std::get<double>(*this);
        }

        bool AsBool() const {
            return std::get<Bool>(*this).flag;
        }

        const auto &AsString() const {
            return std::get<std::string>(*this);
        }

    };

    class Document {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;

    private:
        Node root;
    };

    Document Load(std::istream &input);

    void Print(std::ostream &out, const Node &node, size_t level = 0);

}

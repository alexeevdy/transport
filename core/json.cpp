#include "json.h"

#include <iomanip>

namespace Json {

    Document::Document(Node root) : root(std::move(root)) {
    }

    const Node &Document::GetRoot() const {
        return root;
    }

    Node LoadNode(std::istream &input);

    Node LoadArray(std::istream &input) {
        std::vector<Node> result;

        for (char c; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(std::move(result));
    }

    Node LoadNumber(std::istream &input) {
        double result = .0, base = 1., factor = 1.;
        if (input.peek() == '-') {
            factor = -1.;
            input.ignore(1);
        } else if (input.peek() == '+') {
            input.ignore(1);
        }
        while (isdigit(input.peek())) {
            result *= 10;
            result += input.get() - '0';
        }
        if (input.peek() == '.') {
            input.ignore(1);
            while (isdigit(input.peek())) {
                base /= 10;
                result += base * (input.get() - '0');
            }
        }
        return Node(factor * result);
    }

    Node LoadBool(std::istream &input) {
        std::string flag;
        while (std::isalpha(input.peek())) {
            flag += static_cast<char>(input.get());
        }
        return Node(Bool(flag == "true"));
    }

    Node LoadString(std::istream &input) {
        std::string line;
        getline(input, line, '"');
        return Node(std::move(line));
    }

    Node LoadDict(std::istream &input) {
        std::map<std::string, Node> result;

        for (char c; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            std::string key = LoadString(input).AsString();
            input >> c;
            result.emplace(std::move(key), LoadNode(input));
        }

        return Node(std::move(result));
    }

    Node LoadNode(std::istream &input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else if (c == 't' || c == 'f') {
            input.putback(c);
            return LoadBool(input);
        } else {
            input.putback(c);
            return LoadNumber(input);
        }
    }

    Document Load(std::istream &input) {
        return Document{LoadNode(input)};
    }

    void Indent(std::ostream &out, int level) {
        for (int i = 0; i < level; ++i) {
            out << "\t";
        }
    }

    void PrintArray(std::ostream &out, const Node &root, int level) {
        out << "[";
        const auto &array = root.AsArray();
        if (!array.empty()) out << "\n";
        bool first = true;
        for (const auto &node: array) {
            if (!first) {
                out << ",\n";
            }
            first = false;
            Indent(out, level + 1);
            Print(out, node, level + 1);
        }
        if (!array.empty()) {
            out << "\n";
            Indent(out, level);
        }
        out << "]";
    }

    void PrintMap(std::ostream &out, const Node &root, int level) {
        out << "{";
        const auto &map = root.AsMap();
        if (!map.empty()) out << "\n";
        bool first = true;
        for (const auto &[key, node]: map) {
            if (!first) {
                out << ",\n";
            }
            first = false;
            Indent(out, level + 1);
            out << "\"" << key << "\": ";
            Print(out, node, level + 1);
        }
        if (!map.empty()) {
            out << "\n";
            Indent(out, level);
        }
        out << "}";
    }

    void Print(std::ostream &out, const Node &node, int level) {
        if (std::holds_alternative<std::vector<Node>>(node)) {
            PrintArray(out, node, level);
        } else if (std::holds_alternative<std::map<std::string, Node>>(node)) {
            PrintMap(out, node, level);
        } else if (std::holds_alternative<Int>(node)) {
            out << node.AsInt();
        } else if (std::holds_alternative<double>(node)) {
            out << std::fixed << std::setprecision(9) << node.AsDouble();
        } else if (std::holds_alternative<Bool>(node)) {
            out << (node.AsBool() ? "true" : "false");
        } else if (std::holds_alternative<std::string>(node)) {
            out << "\"" << node.AsString() << "\"";
        } else {
            throw std::runtime_error("unknown variant type");
        }
    }

}

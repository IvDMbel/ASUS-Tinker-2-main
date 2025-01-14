#include "json.h"
 
using namespace std;
 
namespace transport_catalogue {
namespace detail {
namespace json {
namespace {
    
Node LoadNode(istream& input);
 
std::string LoadLiteral(std::istream& input) {
    std::string str;
    
    while (std::isalpha(input.peek())) {
        str.push_back(static_cast<char>(input.get()));
    }
    return str;
}
 
Node LoadArray(std::istream& input) {
    std::vector<Node> array;
    
    for (char ch; input >> ch && ch != ']';) {
        if (ch != ',') {
            input.putback(ch);
        }
        
        array.push_back(LoadNode(input));
    }
 
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
 
    return Node(std::move(array));
}
 
Node LoadNull(std::istream& input) {
    if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node(nullptr);
    } else {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}
 
Node LoadBool(std::istream& input) {
    const auto str = LoadLiteral(input);
 
    if (str == "true"sv) {
        return Node(true);
    } else if (str == "false"sv) {
        return Node(false);
    } else {
        throw ParsingError("Failed to parse '"s + str + "' as bool"s);
    }
}
 
Node LoadNumber(std::istream& input) {
    std::string number;
 
    auto read_char = [&number, &input] {
        number += static_cast<char>(input.get());
        
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };
 
    auto read_digits = [&input, read_char] {
        
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is  expected"s);
        } else {
            while (std::isdigit(input.peek())) {
                read_char();
            }   
        }
    };
 
    if (input.peek() == '-') {
        read_char();
    }
 
    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }
 
    bool IsInt = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        IsInt = false;
    }
 
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
 
        read_digits();
        IsInt = false;
    }
 
    try {
        if (IsInt) {
            try {
                return Node(std::stoi(number));
                
            } catch (...) {}
        }
        return Node(std::stod(number));
        
    } catch (...) {
        throw ParsingError("Failed  to convert "s + number + " to number"s);
    }
}
 
Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string str;
    
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
            
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            
            const char esc_ch = *(it);
            switch (esc_ch) {
                case 'n':
                    str.push_back('\n');
                    break;
                case 't':
                    str.push_back('\t');
                    break;
                case 'r':
                    str.push_back('\r');
                    break;
                case '"':
                    str.push_back('"');
                    break;
                case '\\':
                    str.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized  escpe sequence \\"s + esc_ch);
            }
            
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            str.push_back(ch);
        }
        
        ++it;
    }
    
    return Node(str);
}
    
Node LoadDictionary(std::istream& input) {
    Dict dictionary;
 
    for (char ch; input >> ch && ch != '}';) {
        
        if (ch == '"') {
            std::string key = LoadString(input).AsString();
 
            if (input >> ch && ch == ':') {
                
                if (dictionary.find(key) != dictionary.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
 
                dictionary.emplace(std::move(key), LoadNode(input));
                
            } else {
                throw ParsingError(": is expected but '"s + ch + "' has been found"s);
            }
            
        } else if (ch != ',') {
            throw ParsingError(R"(',' expected. but ')"s + ch + "' has been found"s);
        }
    }
 
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    } else {
        return Node(dictionary);
    }
    
}
  
Node LoadNode(std::istream& input) {
    char ch;
    
    if (!(input >> ch)) {
        throw ParsingError("Unexpected EOF"s);
    } else {
        switch (ch) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDictionary(input);
        case '"':
            return LoadString(input);
        case 't': case 'f':
            input.putback(ch);
            return LoadBool(input);
        case 'n':
            input.putback(ch);
            return LoadNull(input);
        default:
            input.putback(ch);
            return LoadNumber(input);
        }
    }
} 
    
}//end namespace
   

const Array& Node::AsArray() const {
    using namespace std::literals;
    
    if (!IsArray()) {
        throw std::logic_error("value is not an array"s);
    } else {
        return std::get<Array>(*this); 
    }
}
 
const Dict& Node::AsDict() const {
    using namespace std::literals;
    
    if (!IsDict()) {
        throw std::logic_error("value is not a dictionary"s);
    } else {
        return std::get<Dict>(*this);  
    }
}
 
const std::string& Node::AsString() const {
    using namespace std::literals;
    
    if (!IsString()) {
        throw std::logic_error("value is not a string"s);
    } else {
        return std::get<std::string>(*this);        
    }
}
    
int Node::AsInt() const {
    using namespace std::literals; 
    
    if (!IsInt()) {
        throw std::logic_error("value is not an int"s);
    } else {
        return std::get<int>(*this);    
    }
}
 
double Node::AsDouble() const {
    using namespace std::literals;
    
    if (!IsDouble()) {
        throw std::logic_error("value is not a double"s);
    } else if (IsRealDouble()) {
        return std::get<double>(*this);
    } else {
        return AsInt();
    }   
}
 
bool Node::AsBool() const {
    using namespace std::literals;
    
    if (!IsBool()) {
        throw std::logic_error("value is not a bool"s);
    } else {
        return std::get<bool>(*this);   
    }
}
    
bool Node::IsNull() const {return std::holds_alternative<std::nullptr_t>(*this);}
bool Node::IsInt() const {return std::holds_alternative<int>(*this);}
bool Node::IsDouble() const {return IsRealDouble() || IsInt();}  
bool Node::IsRealDouble() const {return std::holds_alternative<double>(*this);}
bool Node::IsBool() const {return std::holds_alternative<bool>(*this);}
bool Node::IsString() const {return std::holds_alternative<std::string>(*this);}
bool Node::IsArray() const {return std::holds_alternative<Array>(*this);}
bool Node::IsDict() const {return std::holds_alternative<Dict>(*this);}
 
const Node::Value& Node::GetValue() const {return *this;}
   
Document::Document(Node root) : root_(std::move(root)) {}
const Node& Document::GetRoot() const {return root_;}
    
Document Load(istream& input) {return Document(LoadNode(input));}
 
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
 
    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
 
    [[nodiscard]] PrintContext Indented() const {
        return {out, 
                indent_step, 
                indent_step + indent};
    }
};
 
void PrintNode(const Node& node, const PrintContext& context);
 
void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');
    
    for (const char ch : value) {
        switch (ch) {
            case '\r':
                out << R"(\r)";
                break;
            case '\n':
                out << R"(\n)";
                break;
            case '"':
                out << R"(\")";
                break;
            case '\\':
                out << R"(\\)";
                break;
            default:
                out.put(ch);
                break;
        }
    }
    
    out.put('"');
}
 
template <typename Value>
void PrintValue(const Value& value, const PrintContext& context) {
    context.out << value;
}    
 
template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& context) {
    PrintString(value, context.out);
}
 
void PrintValue(const std::nullptr_t&, const PrintContext& context) {
    context.out << "null"s;
}
 
void PrintValue(const bool& value, const PrintContext& context) {
    context.out << std::boolalpha << value;
}
 
 void PrintValue(const Array& nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
 
        inner_context.PrintIndent();
        PrintNode(node, inner_context);
    }
 
    out.put('\n');
    context.PrintIndent();
    out.put(']');
}
 
 void PrintValue(const Dict& nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
 
        inner_context.PrintIndent();
        PrintString(key, context.out);
        out << ": "sv;
        PrintNode(node, inner_context);
    }
 
    out.put('\n');
    context.PrintIndent();
    out.put('}');
}
 
void PrintNode(const Node& node, const PrintContext& context) {
    std::visit([&context](const auto& value) {
            PrintValue(value, context);
        }, node.GetValue());
}
    
void Print(const Document& document, std::ostream& output) {
    PrintNode(document.GetRoot(), PrintContext{output});
}
 
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue

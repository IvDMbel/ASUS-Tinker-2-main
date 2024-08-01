#include "json_builder.h"
 
namespace transport_catalogue {
namespace detail {
namespace json {
namespace builder {
 
//---------------------BaseContext---------------------
 
Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}
 
Builder::KeyContext Builder::BaseContext::Key(const std::string& key) {return builder_.Key(key);}
Builder& Builder::BaseContext::Value(const Node::Value& value) {return builder_.Value(value);}
 
Builder::DictionaryContext Builder::BaseContext::StartDict() {return Builder::DictionaryContext(builder_.StartDict());}
Builder& Builder::BaseContext::EndDict() {return builder_.EndDict();}
 
Builder::ArrayContext Builder::BaseContext::StartArray() {return ArrayContext(builder_.StartArray());}
Builder& Builder::BaseContext::EndArray() {return builder_.EndArray();}
    
//---------------------KeyContext-----------------------
 
Builder::KeyContext::KeyContext(Builder& builder) : Builder::BaseContext(builder) {}
 
Builder::DictionaryContext Builder::KeyContext::Value(const Node::Value& value) {return BaseContext::Value(value);}
    
//---------------------DictionaryContext----------------
 
Builder::DictionaryContext::DictionaryContext(Builder& builder) : Builder::BaseContext(builder) {}
 
//---------------------ArrayContext---------------------
 
Builder::ArrayContext::ArrayContext(Builder& builder) : Builder::BaseContext(builder) {}
 
Builder::ArrayContext Builder::ArrayContext::Value(const Node::Value& value) {return Builder::BaseContext::Value(value); }
 
//---------------------Builder--------------------------

Node Builder::MakeNode(const Node::Value& value_) {
    Node node;
 
    if (std::holds_alternative<bool>(value_)) {
        bool bol = std::get<bool>(value_);
        node = Node(bol);
 
    } else if (std::holds_alternative<int>(value_)) {
        int intt = std::get<int>(value_);
        node = Node(intt);
 
    } else if (std::holds_alternative<double>(value_)) {
        double doble = std::get<double>(value_);
        node = Node(doble);
 
    } else if (std::holds_alternative<std::string>(value_)) {
        std::string str = std::get<std::string>(value_);
        node = Node(std::move(str));
 
    } else if (std::holds_alternative<Array>(value_)) {
        Array arr = std::get<Array>(value_);
        node = Node(std::move(arr));
 
    } else if (std::holds_alternative<Dict>(value_)) {
        Dict dictionary = std::get<Dict>(value_);
        node = Node(std::move(dictionary));
 
    } else {
        node = Node();
    }
 
    return node;
}
 
void Builder::AddNode(const Node& node) {
    if (nodes_stack_.empty()) {
 
        if (!root_.IsNull()) {
            throw std::logic_error("root has been added");
        }
 
        root_ = node;
        return;
 
    } else {
 
        if (!nodes_stack_.back()->IsArray()
            && !nodes_stack_.back()->IsString()) {
 
            throw std::logic_error("unable to create node");
        }
 
        if (nodes_stack_.back()->IsArray()) {
            Array arr = nodes_stack_.back()->AsArray();
            arr.emplace_back(node);
 
            nodes_stack_.pop_back();
            auto arr_ptr = std::make_unique<Node>(arr);
            nodes_stack_.emplace_back(std::move(arr_ptr));
 
            return;
        }
 
        if (nodes_stack_.back()->IsString()) {
            std::string str = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
 
            if (nodes_stack_.back()->IsDict()) {
                Dict dictionary = nodes_stack_.back()->AsDict();
                dictionary.emplace(std::move(str), node);
 
                nodes_stack_.pop_back();
                auto dictionary_ptr = std::make_unique<Node>(dictionary);
                nodes_stack_.emplace_back(std::move(dictionary_ptr));
            }
 
            return;
        }
    }
}
 
Builder::KeyContext Builder::Key(const std::string& key_) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to create key");
    }
 
    auto key_ptr = std::make_unique<Node>(key_);
 
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.emplace_back(std::move(key_ptr));
    }
 
    return KeyContext(*this);
}
 
Builder& Builder::Value(const Node::Value& value_) {
    AddNode(MakeNode(value_));
 
    return *this;
}
 
Builder::DictionaryContext Builder::StartDict() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));
 
    return DictionaryContext(*this);
}
 
Builder& Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close as without opening");
    }
 
    Node node = *nodes_stack_.back();
 
    if (!node.IsDict()) {
        throw std::logic_error("object isn't dictionary");
    }
 
    nodes_stack_.pop_back();
    AddNode(node);
 
    return *this;
}
 
Builder::ArrayContext Builder::StartArray() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));
 
    return ArrayContext(*this);
}
 
Builder& Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close without opening");
    }
 
    Node node = *nodes_stack_.back();
 
    if (!node.IsArray()) {
        throw std::logic_error("object isn't array");
    }
 
    nodes_stack_.pop_back();
    AddNode(node);
 
    return *this;
}
 
Node Builder::Build() {
    if (root_.IsNull()) {
        throw std::logic_error("empty json");
    }
 
    if (!nodes_stack_.empty()) {
        throw std::logic_error("invalid json");
    }
 
    return root_;
}
 
}//end namespace builder
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue

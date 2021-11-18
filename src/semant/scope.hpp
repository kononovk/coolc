#pragma once
#include <stack>
#include <vector>

constexpr int a = 0;

namespace coolc {

struct Scope {
  using ObjectName = std::string;
  using ClassName = std::string;
  using MethodName = std::string;
  using TypeName = std::string;

  using ObjectSet = std::unordered_set<ObjectName>;
  using TypeSet = std::unordered_set<TypeName>;

  struct MethodTypes {
    TypeName return_type;
    std::vector<TypeName> args_types;
    friend bool operator==(const MethodTypes& lhs, const MethodTypes& rhs) {
      return (lhs.return_type == rhs.return_type) && (lhs.args_types == rhs.args_types);
    }

    friend bool operator!=(const MethodTypes& lhs, const MethodTypes& rhs) {
      return !(lhs == rhs);
    }
  };

  std::vector<std::unordered_map<ObjectName, TypeName>> objects;
  std::stack<ObjectSet> methods;
  std::unordered_map<ClassName, std::unordered_map<ObjectName, TypeName>> attr_table;
  std::unordered_map<ClassName, std::unordered_map<MethodName, MethodTypes>> method_table;
  std::string current_class;
  const InheritanceGraph& _ig;

  Scope(const InheritanceGraph& ig) : _ig(ig) {
    method_table["String"]["length"] = {.return_type = "Int", .args_types = {}};
    method_table["String"]["substr"] = {.return_type = "String", .args_types = {"Int", "Int"}};
    method_table["String"]["concat"] = {.return_type = "String", .args_types = {"String"}};
    method_table["Object"]["abort"] = {.return_type = "Object", .args_types = {}};
    method_table["Object"]["type_name"] = {.return_type = "String", .args_types = {}};
    method_table["Object"]["copy"] = {.return_type = "SELF_TYPE", .args_types = {}};
    method_table["IO"]["out_string"] = {.return_type = "SELF_TYPE", .args_types = {"String"}};
    method_table["IO"]["in_string"] = {.return_type = "String", .args_types = {}};
    method_table["IO"]["out_int"] = {.return_type = "SELF_TYPE", .args_types = {"Int"}};
    method_table["IO"]["in_int"] = {.return_type = "Int", .args_types = {}};
  }

  void Push() {
    objects.push_back({});
    methods.push({});
    attr_table[current_class]["self"] = "SELF_TYPE";
  }

  void Pop() {
    assert(objects.size() > 0 && methods.size() > 0);
    objects.pop_back();
    methods.pop();
  }

  std::optional<MethodTypes> GetMethod(std::string name) {
    std::string curr = current_class;
    while (curr != "Object") {
      if (method_table[curr].contains(name)) {
        return method_table[curr][name];
      } else {
        curr = _ig.GetAncessor(curr);
      }
    }
    return {};
  }

  std::optional<MethodTypes> GetMethod(std::string cl, std::string symbol) {
    std::string curr = cl;
    while (curr != "Object") {
      if (method_table[curr].contains(symbol)) {
        return method_table[curr][symbol];
      } else {
        curr = _ig.GetAncessor(curr);
      }
    }
    CHECK_NULLOPT(method_table[curr].contains(symbol))
    return method_table[curr][symbol];
  }

  bool AddMethod(std::string name, std::string return_type, std::vector<std::string> arg_types) {
    CHECK_ERROR(!method_table[current_class].contains(name))
    CHECK_ERROR(!methods.top().contains(name))

    MethodTypes new_method = {.return_type = return_type, .args_types = arg_types};
    if (auto prev_method = GetMethod(name); prev_method && *prev_method != new_method) {
      if (prev_method->args_types.size() != arg_types.size()) {
        std::cerr << "Incompatible number of formal parameters in redefined method " << name << std::endl;
      } else if (prev_method->return_type != return_type) {
        std::cerr << "Incompatible return types in redefined method " << name << std::endl;
      }
      return false;
    }
    method_table[current_class][name] = std::move(new_method);
    methods.top().insert(name);
    return true;
  }

  bool AddAttribute(std::string name, std::string type) {
    CHECK_ERROR(name != "self")
    CHECK_ERROR(!GetAttrObject(name))
    objects.back().insert({name, type});
    attr_table[current_class][std::move(name)] = std::move(type);
    return true;
  }

  bool AddObject(ObjectName name, TypeName type) {
    assert(objects.size() > 0 && methods.size() > 0);
    CHECK_ERROR(name != "self")
    CHECK_ERROR(objects.back().insert({name, type}).second)
    if (type != "SELF_TYPE") {
      CHECK_ERROR(_ig.HasClass(type))
    }
    return true;
  }

  void EnterClass(std::string name) {
    current_class = std::move(name);
  }

  void ExitClass() {
    current_class.clear();
  }

  std::optional<TypeName> GetAttrObject(ObjectName name) {
    // get object
    for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
      if (it->contains(name)) {
        return it->at(name);
      }
    }

    // get attribute
    std::string curr = current_class;
    while (curr != "Object") {
      if (attr_table[curr].contains(name)) {
        return attr_table[curr][name];
      } else {
        curr = _ig.GetAncessor(curr);
      }
    }
    return {};
  }
};

struct ScopeGuard {
  explicit ScopeGuard(Scope* scope) : curr_scope{scope} {
    curr_scope->Push();
  }

  ScopeGuard(Scope* scope, const Class& cl) : curr_scope{scope}, new_class{true} {
    curr_scope->EnterClass(cl.type);
    curr_scope->Push();
  }

  ~ScopeGuard() {
    if (new_class) {
      curr_scope->ExitClass();
    }
    curr_scope->Pop();
  }

  Scope* curr_scope;
  bool new_class{false};
};

}  // namespace coolc

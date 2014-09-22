// Copyright (C) 2013 The University of Michigan
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Authors - Chun-Hung Hsiao (chhsiao@umich.edu)
//

#ifndef CANONICALAST_H
#define CANONICALAST_H

#include <set>
#include <ast.h>
#include <scopes.h>
#include <compiler.h>

using namespace v8::internal;

#if 0
#define CONTROL_NODE_LIST(V) \
    V(CanonicalFunctionEntry) \
    V(IfStatement) \
    V(SwitchStatement) \
    V(WhileStatement) \
    V(ForInStatement)

#define END_NODE_LIST(V) \
    V(CanonicalFunctionExit) \
    V(EndIfStatement) \
    V(EndSwitchStatement) \
    V(EndWhileStatement) \
    V(EndForInStatement)
#endif

#define CANONICAL_NODE_LIST(V) \
    V(CanonicalFunctionEntry) \
    V(CanonicalAssignment) \
    V(CanonicalPropertyAssignment) \
    V(CanonicalFunctionExit)

#define DECLARE_TYPE_ENUM(type) k##type,
enum CanonicalNodeType {
    kCanonicalBegin = AstNode::kThisFunction,
    CANONICAL_NODE_LIST(DECLARE_TYPE_ENUM)
	kCanonicalEnd = -1
};
#undef DECLARE_TYPE_ENUM

#define DECLARE_NODE_TYPE(type) \
	virtual void Accept(AstVisitor* v); \
	virtual AstNode::Type node_type() const { \
	    return static_cast<AstNode::Type>(k##type); \
	}

class CanonicalFunctionEntry : public Statement {
    public:
	DECLARE_NODE_TYPE(CanonicalFunctionEntry)

	inline FunctionLiteral* literal() const { return literal_; }
    	inline ZoneList<Variable*>* parameters() const { return parameters_; }
	inline ZoneList<Declaration*>* declarations() const { return declarations_; }
	inline ZoneList<Statement*>* body() const { return body_; }

    protected:
	template<class> friend class CanonicalNodeFactory;

	CanonicalFunctionEntry(FunctionLiteral* literal, ZoneList<Variable*>* parameters, ZoneList<Declaration*>* declarations, ZoneList<Statement*>* body)
	    : literal_(literal), parameters_(parameters), declarations_(declarations), body_(body) { }

    private:
	FunctionLiteral* literal_;
	ZoneList<Variable*>* parameters_;
	ZoneList<Declaration*>* declarations_;
	ZoneList<Statement*>* body_;
};

class CanonicalAssignment : public Statement {
    public:
	DECLARE_NODE_TYPE(CanonicalAssignment)

	inline VariableProxy* target() const { return target_; }
	inline Expression* value() const { return value_; }
	inline void setValue(Expression* value) { value_ = value; }

    protected:
	template<class> friend class CanonicalNodeFactory;

	CanonicalAssignment(VariableProxy* target, Expression* value)
	    : target_(target), value_(value) { }

    private:
	VariableProxy* target_;
	Expression* value_;
};

class CanonicalPropertyAssignment : public Statement {
    public:
	DECLARE_NODE_TYPE(CanonicalPropertyAssignment)

	inline Property* target() const { return target_; }
	inline Expression* value() const { return value_; }

    protected:
	template<class> friend class CanonicalNodeFactory;

	CanonicalPropertyAssignment(Property* target, Expression* value)
	    : target_(target), value_(value) { }

    private:
	Property* target_;
	Expression* value_;
};

class CanonicalFunctionExit : public Statement {
    public:
	DECLARE_NODE_TYPE(CanonicalFunctionExit)

	inline CanonicalFunctionEntry* entry() const { return entry_; }

    protected:
	template<class> friend class CanonicalNodeFactory;

	CanonicalFunctionExit(CanonicalFunctionEntry* entry) : entry_(entry) { }

    private:
	CanonicalFunctionEntry* entry_;
};

#if 0
#define DECLARE_END_NODE(type) \
class End##type : public Statement { \
    public: \
	DECLARE_NODE_TYPE(End##type) \
	type* begin() const { return begin_; } \
    protected: \
	template<class> friend class CanonicalNodeFactory; \
	End##type(type* begin) : begin_(begin) { } \
    private: \
	type* begin_; \
};
CONTROL_NODE_LIST(DECLARE_END_NODE)
#undef DECLARE_END_NODE
#endif

#undef DECLARE_NODE_TYPE

class CanonicalAstVisitor : public AstVisitor {
    public:
	void Visit(AstNode* node) { node->Accept(this); }
#define DECLARE_VISIT(type) \
	virtual void Visit##type(type* node) = 0;
	CANONICAL_NODE_LIST(DECLARE_VISIT)
#undef DECLARE_VISIT

    protected:
	inline Isolate* isolate() { return isolate_; }

    private:
	Isolate* isolate_;
};

template<class Visitor> class CanonicalNodeFactory : public AstNodeFactory<Visitor> {
    public:
	explicit CanonicalNodeFactory(Isolate *isolate)
	    : AstNodeFactory<Visitor>(isolate, isolate->runtime_zone()), isolate_(isolate), zone_(isolate->runtime_zone()) { }

	CanonicalFunctionEntry* NewCanonicalFunctionEntry(FunctionLiteral* literal, ZoneList<Statement*>* body) {
	    ZoneList<Variable*>* params = new(zone_) ZoneList<Variable*>(literal->scope()->num_parameters(), zone_);
	    for (int i = 0; i < literal->scope()->num_parameters(); ++i)
		params->Add(literal->scope()->parameter(i), zone_);
	    return new(zone_) CanonicalFunctionEntry(literal, params, literal->scope()->declarations(), body);
	}

	CanonicalAssignment* NewCanonicalAssignment(Expression* target, Expression* value) {
	    return new(zone_) CanonicalAssignment(reinterpret_cast<VariableProxy*>(target), value);
	}

	CanonicalPropertyAssignment* NewCanonicalPropertyAssignment(Expression* target, Expression* value) {
	    return new(zone_) CanonicalPropertyAssignment(reinterpret_cast<Property*>(target), value);
	}

	CanonicalFunctionExit* NewCanonicalFunctionExit(CanonicalFunctionEntry *entry) {
	    return new(zone_) CanonicalFunctionExit(entry);
	}

#if 0
#define DECLARE_NEW_END_NODE(type) \
	End##type* NewEnd##type(type* begin) { \
	    return new(zone_) End##type(begin); \
	}
	CONTROL_NODE_LIST(DECLARE_NEW_END_NODE)
#undef DECLARE_NEW_END_NODE
#endif

	VariableProxy* NewTemporary(Scope* scope) {
    	    static int count = 0;
	    char buf[16];
	    sprintf(buf, "$%d", count++);
	    Handle<String> name = isolate_->factory()->LookupAsciiSymbol(buf);
	    return this->NewVariableProxy(scope->NewTemporary(name));
	}

    private:
	Isolate* isolate_;
	Zone* zone_;
};

template <class T>
class ZoneListIterator {
    public:
	ZoneListIterator(Scope* scope, ZoneList<T*>* elems)
	    : scope_(scope), elements_(elems), index_(0) { }

	inline Scope* scope() const { return scope_; }
	inline T* Get() const { return elements_->at(index_); }
	inline bool IsDone() const { return index_ >= elements_->length(); }
	inline void Next() { ++index_; }
	inline void Insert(T* elem) { elements_->InsertAt(index_++, elem, scope_->zone()); }
	inline void Append(T* elem) { elements_->InsertAt(++index_, elem, scope_->zone()); }
	inline void Remove() { elements_->Remove(index_--); }

    private:
	Scope* scope_;
	ZoneList<T*>* elements_;
	int index_;
};

class CanonicalAstConverter : public AstVisitor {
    public:

	CanonicalAstConverter() : isolate_(Isolate::Current()), factory_(isolate_) { }

	void Visit(AstNode* node) { node->Accept(this); }
	// Individual nodes
#define DECLARE_VISIT(type) \
	void Visit##type(type* node);
	AST_NODE_LIST(DECLARE_VISIT)
#undef DECLARE_VISIT

	void Convert(CompilationInfo* info);
	void ConvertStatements(Scope* scope, ZoneList<Statement*>* statements);

    private:
	Isolate* isolate_;
	CanonicalNodeFactory<AstNullVisitor> factory_;
	Scope* global_scope_;
	ZoneListIterator<Statement>* iterator_;
	Expression* value_;
	std::set<FunctionLiteral*> functions_;

	inline Isolate* isolate() { return isolate_; }
	Expression* Canonicalize(Expression* expr);
	Block* Wrap(Statement* stmt);
};

#endif  // CANONICALAST_H

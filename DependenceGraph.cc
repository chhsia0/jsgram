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

#include "DependenceGraph.h"

#include <checks.h>
#include <queue>
#include <utility>
#include "Utility.h"

using std::make_pair;
using std::queue;

#define DEFINE_UNREACHABLE_VISIT(type) \
void DependenceGraphBuilder::Visit##type(type* node) { \
    UNREACHABLE(); \
}
MODULE_NODE_LIST(DEFINE_UNREACHABLE_VISIT)
DEFINE_UNREACHABLE_VISIT(ModuleDeclaration)
DEFINE_UNREACHABLE_VISIT(ImportDeclaration)
DEFINE_UNREACHABLE_VISIT(ExportDeclaration)
DEFINE_UNREACHABLE_VISIT(ModuleStatement)
DEFINE_UNREACHABLE_VISIT(ExpressionStatement)
DEFINE_UNREACHABLE_VISIT(EmptyStatement)
DEFINE_UNREACHABLE_VISIT(WithStatement)
DEFINE_UNREACHABLE_VISIT(DoWhileStatement)
DEFINE_UNREACHABLE_VISIT(ForStatement)
DEFINE_UNREACHABLE_VISIT(TryCatchStatement)
DEFINE_UNREACHABLE_VISIT(TryFinallyStatement)
DEFINE_UNREACHABLE_VISIT(ObjectLiteral)
DEFINE_UNREACHABLE_VISIT(ArrayLiteral)
DEFINE_UNREACHABLE_VISIT(Assignment)
DEFINE_UNREACHABLE_VISIT(CountOperation)
#undef DEFINE_UNREACHABLE_VISIT

#define DEFINE_EMPTY_VISIT(type) \
    void DependenceGraphBuilder::Visit##type(type* node) { \
}
DEFINE_EMPTY_VISIT(VariableDeclaration)
DEFINE_EMPTY_VISIT(DebuggerStatement)
DEFINE_EMPTY_VISIT(SharedFunctionInfoLiteral)
DEFINE_EMPTY_VISIT(Literal)
DEFINE_EMPTY_VISIT(RegExpLiteral)
DEFINE_EMPTY_VISIT(ThisFunction)
DEFINE_EMPTY_VISIT(CanonicalFunctionExit)
//DEFINE_EMPTY_VISIT(EndIfStatement)
//DEFINE_EMPTY_VISIT(EndSwitchStatement)
//DEFINE_EMPTY_VISIT(EndWhileStatement)
//DEFINE_EMPTY_VISIT(EndForInStatement)
#undef DEFINE_EMPTY_VISIT

void DependenceGraphBuilder::VisitFunctionDeclaration(FunctionDeclaration* node) {
    Visit(node->fun());
}

void DependenceGraphBuilder::VisitBlock(Block* node) {
    VisitStatements(node->statements());
}

void DependenceGraphBuilder::VisitContinueStatement(ContinueStatement* node) {
    // TODO: manage control dependence
}

void DependenceGraphBuilder::VisitBreakStatement(BreakStatement* node) {
    // TODO: manage control dependence
}

void DependenceGraphBuilder::VisitReturnStatement(ReturnStatement* node) {
    // TODO: manage control dependence
    visiting_ = node;
    Visit(node->expression());
}

void DependenceGraphBuilder::VisitFunctionLiteral(FunctionLiteral* node) {
    VisitStatements(node->body());
}

void DependenceGraphBuilder::VisitConditional(Conditional* node) {
    Visit(node->condition());
    Visit(node->then_expression());
    Visit(node->else_expression());
}

void DependenceGraphBuilder::VisitVariableProxy(VariableProxy* node) {
    Read(node->var());
}

void DependenceGraphBuilder::VisitThrow(Throw* node) {
    Visit(node->exception());
}

void DependenceGraphBuilder::VisitProperty(Property* node) {
    Visit(node->obj());
    Visit(node->key());
}

void DependenceGraphBuilder::VisitCall(Call* node) {
    // XXX: currently enforce Write on all arguments
    //for (int i = 0; i < node->arguments()->length(); ++i)
    //	Visit(node->arguments()->at(i));
    for (int i = 0; i < node->arguments()->length(); ++i)
	if (node->arguments()->at(i)->node_type() == AstNode::kVariableProxy)
	    Write(reinterpret_cast<VariableProxy*>(node->arguments()->at(i))->var());
    Visit(node->expression());
}

void DependenceGraphBuilder::VisitCallNew(CallNew* node) {
    // XXX: currently enforce Write on all arguments
    //for (int i = 0; i < node->arguments()->length(); ++i)
    //	Visit(node->arguments()->at(i));
    for (int i = 0; i < node->arguments()->length(); ++i)
	if (node->arguments()->at(i)->node_type() == AstNode::kVariableProxy)
	    Write(reinterpret_cast<VariableProxy*>(node->arguments()->at(i))->var());
    Visit(node->expression());
}

void DependenceGraphBuilder::VisitCallRuntime(CallRuntime* node) {
    // XXX: currently enforce Write on all arguments
    //for (int i = 0; i < node->arguments()->length(); ++i)
    //	Visit(node->arguments()->at(i));
    for (int i = 0; i < node->arguments()->length(); ++i)
	if (node->arguments()->at(i)->node_type() == AstNode::kVariableProxy)
	    Write(reinterpret_cast<VariableProxy*>(node->arguments()->at(i))->var());
}

void DependenceGraphBuilder::VisitUnaryOperation(UnaryOperation* node) {
    Visit(node->expression());
}

void DependenceGraphBuilder::VisitBinaryOperation(BinaryOperation* node) {
    Visit(node->left());
    Visit(node->right());
}

void DependenceGraphBuilder::VisitCompareOperation(CompareOperation* node) {
    Visit(node->left());
    Visit(node->right());
}

void DependenceGraphBuilder::VisitCanonicalAssignment(CanonicalAssignment* node) {
    visiting_ = node;
    Visit(node->value());
    visiting_ = node;
    Write(node->target()->var());
}

void DependenceGraphBuilder::VisitCanonicalPropertyAssignment(CanonicalPropertyAssignment* node) {
    visiting_ = node;
    Visit(node->value());
    visiting_ = node;
    Visit(node->target()->obj());
    Visit(node->target()->key());
    ASSERT(node->target()->obj()->node_type() == AstNode::kVariableProxy);
    Write(reinterpret_cast<VariableProxy*>(node->target()->obj())->var());
}

void DependenceGraphBuilder::VisitCanonicalFunctionEntry(CanonicalFunctionEntry* node) {
    Region* outer_region = region_;
    region_ = new Region(NULL, node);
    visiting_ = node;
    for (int i = 0; i < node->parameters()->length(); ++i)
	Write(node->parameters()->at(i));
    for (int i = 0; i < node->declarations()->length(); ++i)
	Visit(node->declarations()->at(i));
    VisitStatements(node->body());
    delete region_;
    region_ = outer_region;
}

void DependenceGraphBuilder::VisitIfStatement(IfStatement* node) {
    visiting_ = node;
    Visit(node->condition());
    Region* then_region = new Region(region_, node);
    Region* else_region = new Region(region_, node);
    region_ = then_region;
    Visit(node->then_statement());
    region_ = else_region;
    Visit(node->else_statement());
    region_ = then_region->outer_region();
    delete then_region;
    //delete else_region;
}

void DependenceGraphBuilder::VisitSwitchStatement(SwitchStatement* node) {
    visiting_ = node;
    Visit(node->tag());
    for (int i = 0; i < node->cases()->length(); ++i) {
	if (!node->cases()->at(i)->is_default()) {
	    visiting_ = node;
	    Visit(node->cases()->at(i)->label());
	}
    }
    region_ = new Region(region_, node);
    for (int i = 0; i < node->cases()->length(); ++i)
	VisitStatements(node->cases()->at(i)->statements());
    Region* inner_region = region_;
    region_ = inner_region->outer_region();
    delete inner_region;
}

void DependenceGraphBuilder::VisitWhileStatement(WhileStatement* node) {
    //for (int i = 0; i < 2; ++i) {
	visiting_ = node;
	Visit(node->cond());
	region_ = new Region(region_, node);
	Visit(node->body());
	Region* inner_region = region_;
	region_ = inner_region->outer_region();
	delete inner_region;
    //}
}

void DependenceGraphBuilder::VisitForInStatement(ForInStatement* node) {
    visiting_ = node;
    Visit(node->enumerable());
    //for (int i = 0; i < 2; ++i) {
	visiting_ = node;
	Write(reinterpret_cast<VariableProxy*>(node->each())->var());
	region_ = new Region(region_, node);
	Visit(node->body());
	Region* inner_region = region_;
	region_ = inner_region->outer_region();
	delete inner_region;
    //}
}

void DependenceGraphBuilder::Build(FunctionLiteral* program) {
    visiting_ = NULL;
    region_ = NULL;
    graph_.clear();
    Visit(program);
    for (value_iterator<DependenceGraph> iter = graph_.begin(); iter != graph_.end(); ++iter) {
    	iter->sort();
    	iter->unique();
    }
    for (DependenceGraph::iterator i = graph_.begin(); i != graph_.end(); ++i) {
	reverse_graph_.insert(make_pair(i->first, list<Statement*>()));
    	for (list<Statement*>::iterator j = i->second.begin(); j != i->second.end(); ++j)
    	    reverse_graph_[*j].push_back(i->first);
    }
}

void DependenceGraphBuilder::VisitStatements(ZoneList<Statement*>* stmts) {
    for (int i = 0; i < stmts->length(); ++i) {
        switch (static_cast<int>(stmts->at(i)->node_type())) {
            case kCanonicalFunctionEntry:
            case kCanonicalFunctionExit:
                graph_.insert(make_pair(stmts->at(i), list<Statement*>()));
            case AstNode::kBlock:
                break;
            default:
                if (region_)
                graph_[stmts->at(i)].push_back(region_->entry());
	}
	Visit(stmts->at(i));
    }
}

void DependenceGraphBuilder::Read(Variable* var) {
    for (pair<list<Statement*>::const_iterator,list<Statement*>::const_iterator> rdefs = region_->Find(var); rdefs.first != rdefs.second; ++rdefs.first)
	graph_[visiting_].push_back(*rdefs.first);
}

void DependenceGraphBuilder::Write(Variable* var) {
    region_->Define(var, visiting_);
}

DependenceGraphBuilder::Region::Region(Region* outer_region, Statement* entry) : outer_region_(outer_region), entry_(entry) {
}

DependenceGraphBuilder::Region::~Region() {
    if (outer_region_ != NULL)
	for (map<Variable*,list<Statement*> >::iterator iter = defs_.begin(); iter != defs_.end(); ++iter)
	    outer_region_->defs_[iter->first].insert(outer_region_->defs_[iter->first].end(), iter->second.begin(), iter->second.end());
}

pair<list<Statement*>::const_iterator,list<Statement*>::const_iterator> DependenceGraphBuilder::Region::Find(Variable* var) {
    if (defs_.find(var) != defs_.end())
	return make_pair(defs_[var].begin(), defs_[var].end());
    if (outer_region_ != NULL)
	return outer_region_->Find(var);
    Define(var, entry_);
    return make_pair(defs_[var].begin(), defs_[var].end());
}

void DependenceGraphBuilder::Region::Define(Variable* var, Statement* stmt) {
    defs_[var].clear();
    defs_[var].push_back(stmt);
}

// class DependenceGraph

const DependenceGraph DependenceGraph::GetNeighborhood(Statement* node, int radius) const {
    queue<Statement*> q;
    DependenceGraph neighborhood;
    neighborhood.insert(make_pair(node, list<Statement*>()));
    q.push(node);
    size_t r = 1;
    int d = 1;
    while (d < radius) {
	node = q.front();
	q.pop();
	for (list<Statement*>::const_iterator i = at(node).begin(); i != at(node).end(); ++i) {
	    if (neighborhood.insert(make_pair(*i, list<Statement*>())).second)
		q.push(*i);
	    neighborhood[node].push_back(*i);
	}
	if (q.empty())
	    break;
	if (--r == 0) {
	    ++d;
	    r = q.size();
	}
    }
    return neighborhood;
}

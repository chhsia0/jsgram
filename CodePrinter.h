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

#ifndef CODEPRINTER_H
#define CODEPRINTER_H

#include <list>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include "CanonicalAst.h"
#include "DependenceGraph.h"

using namespace v8::internal;
using std::list;
using std::map;
using std::set;
using std::stack;
using std::vector;

class CodePrinter: public CanonicalAstVisitor {
    public:
	CodePrinter(FunctionLiteral* program);
	virtual ~CodePrinter();

	// The following routines print a node into a string.
	// The result string is alive as long as the CodePrinter is alive.
	const char* PrintProgram();
	const char* Print(AstNode* node, const DependenceGraph& graph = DependenceGraph(), const list<Statement*>& succ = list<Statement*>());
	const char* PrintFunc(FunctionLiteral* node);

	void Print(const char* format, ...);

	// Print a node to stdout.
	//static void PrintOut(AstNode* node);

	// Individual nodes
#define DECLARE_VISIT(type) virtual void Visit##type(type* node);
	AST_NODE_LIST(DECLARE_VISIT)
	CANONICAL_NODE_LIST(DECLARE_VISIT)
#undef DECLARE_VISIT

	inline const char* GetOutput() const { return output_; }
	inline int GetLineNo(Statement* node) const { return lineno_.at(node); }
	inline int GetFuncNo(Statement* node) const { return funcno_.at(node); }
	inline int CompareNode(Statement* const& x, Statement* const& y) const { return lineno_.at(x) - lineno_.at(y); }
	inline Statement* GetLine(int lineno) const { return line_.at(lineno - 1); }
	inline Statement* GetFunc(int funcno) const { return func_.at(funcno); }
	inline const map<int,Statement*>& GetFuncList() const { return func_; }
	inline size_t NumLines() const { return line_.size(); }

    private:
	char* output_;  // output string buffer
	int size_;  // output_ size
	int pos_;  // current printing position
	int indent_;
	FunctionLiteral* program_;
	vector<Statement*> line_;
	map<int,Statement*> func_;
	map<Statement*,int> lineno_;
	map<Statement*,int> funcno_;
	const DependenceGraph* graph_;
	set<Statement*> successors_;
	stack<int> func_stack_;
	//stack<bool> flag_stack_;

    protected:
	void Init();

	virtual void PrintStatements(ZoneList<Statement*>* statements);
	void PrintLabels(ZoneStringList* labels);
	virtual void PrintArguments(ZoneList<Expression*>* arguments);
	void PrintLiteral(Handle<Object> value, bool quote);
	void PrintParameters(Scope* scope);
	void PrintDeclarations(ZoneList<Declaration*>* declarations);
	void PrintFunctionLiteral(FunctionLiteral* function);
	void PrintCaseClause(CaseClause* clause);
	void PrintDependence(Statement* node);
	void PrintLineNo(Statement* node);
	//void ExitStatement(Statement* node);
};

#endif  // CODEPRINTER_H

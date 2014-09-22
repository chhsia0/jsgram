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

#ifndef DEPENDENCEGRAPHBUILDER_H
#define DEPENDENCEGRAPHBUILDER_H

#include <list>
#include <map>
#include <utility>
#include "CanonicalAst.h"

using std::list;
using std::map;
using std::pair;

class DependenceGraph : public map<Statement*,list<Statement*> > {
    public:
	const DependenceGraph GetNeighborhood(Statement* node, int radius) const;
};

class DependenceGraphBuilder : public CanonicalAstVisitor {
    public:
	void Build(FunctionLiteral* program);
	inline const DependenceGraph& GetGraph() const { return graph_; }
	inline const list<Statement*>& GetSuccessors(Statement* node) const { return reverse_graph_.at(node); }

#define DECLARE_VISIT(type) \
	void Visit##type(type* node);
	AST_NODE_LIST(DECLARE_VISIT)
	CANONICAL_NODE_LIST(DECLARE_VISIT)
#undef DECLARE_VISIT

    private:
	class Region {
	    public:
		Region(Region* outer_region, Statement* entry);
		~Region();

		inline Statement* entry() const { return entry_; }
		inline Region* outer_region() const { return outer_region_; }
		pair<list<Statement*>::const_iterator,list<Statement*>::const_iterator> Find(Variable* var);
		void Define(Variable* var, Statement* stmt);

	    private:
		Region* outer_region_;
		Statement* entry_;
		map<Variable*,list<Statement*> > defs_;
	};

	Statement* visiting_;
	Region* region_;
	DependenceGraph graph_;
	DependenceGraph reverse_graph_;

	void VisitStatements(ZoneList<Statement*>* stmts);
	void Read(Variable* var);
	void Write(Variable* var);
};

#endif  // DEPENDENCEGRAPHBUILDER_H

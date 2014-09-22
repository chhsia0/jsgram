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

#ifndef PDGEXTRACTOR_H
#define PDGEXTRACTOR_H

#include <map>
#include <utility>
#include <vector>
#include "CanonicalAst.h"
#include "DependenceGraph.h"
#include "NgramExtractor.h"
#include "Utility.h"

using std::map;
using std::pair;
using std::string;
using std::vector;

class PDGExtractor : public NgramExtractor {
    public:
	template <class Compare> PDGExtractor(const DependenceGraph &graph, Compare cmp, size_t limit)
	    : graph_(graph), size_limit_(limit) {
	    list<Statement*> nodes;
	    for (key_iterator<DependenceGraph> i = graph_.begin(); i != graph_.end(); ++i)
	    	nodes.push_back(*i);
	    nodes.sort(cmp);
	    int rank = 0;
	    for (list<Statement*>::iterator i = nodes.begin(); i != nodes.end(); ++i)
	    	lexical_order_[*i] = rank++;
	}

	string Extract(Statement* node, int n, bool long_desc = false);

    private:
	struct Node {
	    Node() : parent(this), rank(0) { }

            Node* FindRoot() {
                if (parent != this)
                    parent = parent->FindRoot();
                return parent;
            }
            void Union(Node* that) {
                if (FindRoot() != that->FindRoot()) {
                    if (parent->rank < that->parent->rank)
                        parent->parent = that->parent;
                    else if (parent->rank > that->parent->rank)
                        that->parent->parent = parent;
                    else {
                        that->parent->parent = parent;
                        ++parent->rank;
                    }
                }
            }

	    Statement* statement;
	    list<Node*> predecessors;
	    list<Node*> successors;
	    pair<int,int> level;
	    vector<bool> adjacency;
	    string serialization;
            Node* parent;
            int rank;
	};

	int CompareNode(Node* const& x, Node* const& y) const;
	int CompareSymmetry(Node* const& x, Node* const& y) const;
	//int CompareSuccessors(Node* const& x, Node* const& y) const;
	const char* ToCString(size_t index);
	void SetMinLevel(Node* node);
	void FindMinimalPattern();
	void SearchOrder(size_t index);

	Statement* focal_;
	DependenceGraph graph_;
	DependenceGraph neighborhood_;
	map<Statement*,int> lexical_order_;
	vector<Node*> min_order_;
	vector<Node*> curr_order_;
	vector<size_t> range_;
	string min_pattern_;
	string curr_pattern_;
	const size_t size_limit_;
	size_t count_;
	map<int,map<Statement*,string> > cached_patterns_;
};

#endif // PDGEXTRACTOR_H

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

#include "PDGExtractor.h"

#include <algorithm>
#include <iostream>
#include <isolate.h>
#include <sstream>
#include <queue>

using std::cerr;
using std::cout;
using std::endl;
using std::make_pair;
using std::max;
using std::min;
using std::next_permutation;
using std::queue;
using std::random_shuffle;
using std::reverse;
using std::sort;
using std::ostringstream;
using std::swap;

string PDGExtractor::Extract(Statement* node, int n, bool long_desc) {
    focal_ = node;
    neighborhood_ = graph_.GetNeighborhood(node, n);
    if (neighborhood_.size() > size_limit_) {
    	return "";
    }
    if (cached_patterns_[n].count(node)) {
    	min_pattern_ = cached_patterns_[n][node];
    } else {
	FindMinimalPattern();
	cached_patterns_[n][node] = min_pattern_;
    }
    ostringstream out;
    out << min_pattern_;
    if (long_desc) {
    	out << '\t' << neighborhood_.size();
    	out << '\t' << Serialize(node);
    	for (DependenceGraph::iterator i = neighborhood_.begin(); i != neighborhood_.end(); ++i)
	    i->second.remove(node);
	//for (list<Statement*>::iterator i = reverse_neighborhood_[node].begin(); i != reverse_neighborhood_[node].end(); ++i)
	//    neighborhood_[*i].remove(node);
	//for (list<Statement*>::iterator i = neighborhood_[node].begin(); i != neighborhood_[node].end(); ++i)
	//    reverse_neighborhood_[*i].remove(node);
	neighborhood_.erase(node);
	//reverse_neighborhood_.erase(node);
	FindMinimalPattern();
	out << '\t' << min_pattern_;
    }
    return out.str();
}

/*void PDGExtractor::Count(int k) {
    map<size_t,size_t> count;
    for (key_iterator<DependenceGraph> i = graph_.begin(); i != graph_.end(); ++i)
	++count[FindNeighborhood(*i, k - 1)];
    for (map<size_t,size_t>::iterator iter = count.begin(); iter != count.end(); ++iter)
	cout << iter->first << '\t' << iter->second << endl;
}

void PDGExtractor::Print(Statement* node, int k) {
    FindNeighborhood(node, k - 1);
    cout << printer_.PrintFragment(set<Statement*>(key_iterator<DependenceGraph>(neighborhood_.begin()), key_iterator<DependenceGraph>(neighborhood_.end())), neighborhood_);
}*/

int PDGExtractor::CompareNode(Node* const& x, Node* const& y) const {
    int ret;
    int x_type = x->statement->node_type();
    int y_type = y->statement->node_type();
    if (x_type != y_type) {
	if (x_type == kCanonicalFunctionEntry || y_type == kCanonicalFunctionExit)
	    return -1;
	if (y_type == kCanonicalFunctionEntry || x_type == kCanonicalFunctionExit)
	    return 1;
    }
    if ((ret = x->level.first - y->level.first) != 0)
	return ret;
    if ((ret = x->level.second - y->level.second) != 0)
	return ret;
    if ((ret = x->predecessors.size() - y->predecessors.size()) != 0)
	return ret;
    if ((ret = x->successors.size() - y->successors.size()) != 0)
	return ret;
    if ((ret = x_type - y_type) != 0)
	return ret;
    if ((ret = x->serialization.compare(y->serialization)) != 0)
    	return ret;
    return x->adjacency <= y->adjacency ? x->adjacency < y->adjacency ? -1 : 0 : 1;
}

int PDGExtractor::CompareSymmetry(Node* const& x, Node* const& y) const {
    if (x->FindRoot() == y->FindRoot())
        return 0;
    return x <= y ? x < y ? -1 : 0 : 1;
}

/*int PDGExtractor::CompareSuccessors(Node* const& x, Node* const& y) const {
    int ret;
    list<Node*>::const_iterator i, j;
    for (i = x->successors.begin(), j = y->successors.begin();
         i != x->successors.end() && j != y->successors.end(); ++i, ++j) {
        if ((ret = *i - *j) != 0)
            return ret;
    }
    if (j != y->successors.end())
    	return -1;
    if (i != x->successors.end())
    	return 1;
    return 0;
}*/

void PDGExtractor::SetMinLevel(Node* node) {
    if (node->level.first)
    	return;
    node->level.first = 1;
    for (list<Node*>::iterator i = node->predecessors.begin(); i != node->predecessors.end(); ++i) {
	if (lexical_order_[(*i)->statement] < lexical_order_[node->statement]) {
	    SetMinLevel(*i);
	    node->level.first = max(node->level.first, (*i)->level.first + 1);
	}
    }
}

const char* PDGExtractor::ToCString(size_t index) {
    static char buf[16] = " ";
    char* ptr = buf + 1;
    do {
    	*ptr++ = index % 10 + '0';
    	index /= 10;
    } while (index);
    *ptr = '\0';
    reverse(buf + 1, ptr);
    return buf;
}

void PDGExtractor::FindMinimalPattern() {
    min_pattern_ = curr_pattern_ = "";

    // initialize nodes
    curr_order_.clear();
    map<Statement*,Node*> node_map;
    for (key_iterator<DependenceGraph> i = neighborhood_.begin(); i != neighborhood_.end(); ++i) {
        Node* node = new Node;
    	node->statement = *i;
    	node->level = make_pair(0, 0);
    	node->adjacency.assign(neighborhood_.size(), false);
    	node->serialization = Serialize(*i);
    	node_map[*i] = node;
    	curr_order_.push_back(node);
    }
    for (DependenceGraph::iterator i = neighborhood_.begin(); i != neighborhood_.end(); ++i) {
    	for (list<Statement*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
	    node_map[i->first]->predecessors.push_back(node_map[*j]);
	    node_map[*j]->successors.push_back(node_map[i->first]);
	}
    }

    // set min/max levels
    for (vector<Node*>::iterator i = curr_order_.begin(); i != curr_order_.end(); ++i)
	SetMinLevel(*i);
    sort(curr_order_.begin(), curr_order_.end(), mem_fun_less(this, &PDGExtractor::CompareNode));
    for (vector<Node*>::iterator i = curr_order_.begin(); i != curr_order_.end(); ++i)
	(*i)->level.second = curr_order_.back()->level.first;
    for (vector<Node*>::reverse_iterator i = curr_order_.rbegin(); i != curr_order_.rend(); ++i) {
    	for (list<Node*>::iterator j = (*i)->predecessors.begin(); j != (*i)->predecessors.end(); ++j) {
    	    if (lexical_order_[(*j)->statement] < lexical_order_[(*i)->statement])
		(*j)->level.second = min((*j)->level.second, (*i)->level.second - 1);
	}
    }
    sort(curr_order_.begin(), curr_order_.end(), mem_fun_less(this, &PDGExtractor::CompareNode));

    // set range
    range_.clear();
    for (size_t i = 0; i < curr_order_.size(); ) {
    	while (i < curr_order_.size() && !CompareNode(curr_order_[range_.size()], curr_order_[i]))
    	    ++i;
	while (range_.size() < i)
	    range_.push_back(i);
    }

    //count_ = 0;
    SearchOrder(0);

    for (vector<Node*>::iterator i = curr_order_.begin(); i != curr_order_.end(); ++i)
    	delete *i;
}

void PDGExtractor::SearchOrder(size_t index) {
    //if (count_ > count_limit_)
    //	return;

    if (index == curr_order_.size()) {
        if (min_pattern_.empty() || curr_pattern_ < min_pattern_) {
            min_pattern_ = curr_pattern_;
            min_order_ = curr_order_;
        } else if (curr_pattern_ == min_pattern_) {
            size_t i;
            for (i = 0; i < min_order_.size() && min_order_[i] == curr_order_[i]; ++i)
                ;
            if (i < min_order_.size() && CompareNode(min_order_[i], curr_order_[i]) == 0)
                min_order_[i]->Union(curr_order_[i]);
        }
    	//++count_;
    	return;
    }

    // reorder nodes according to their ordered predecessors
    size_t pivot = index + 1;
    for (size_t i = index + 1; i < range_[index]; ++i) {
    	if (curr_order_[i]->adjacency <= curr_order_[index]->adjacency) {
    	    if (curr_order_[i]->adjacency < curr_order_[index]->adjacency)
    	    	pivot = index;
    	    swap(curr_order_[pivot++], curr_order_[i]);
	}
    }

    sort(curr_order_.begin() + index, curr_order_.begin() + pivot);//, mem_fun_less(this, &PDGExtractor::CompareSymmetry));

    size_t len = curr_pattern_.size();
    //for (int t = (pivot - index) << 2; t; --t) { // enumerate all if n <= 3, o.w. 4n random ones
    while (true) { // enumerate all
    	curr_pattern_.resize(len);
    	for (size_t i = index; i < pivot; ++i) {
    	    for (list<Node*>::iterator k = curr_order_[i]->successors.begin(); k != curr_order_[i]->successors.end(); ++k)
    	    	(*k)->adjacency[i] = true;
    	    curr_pattern_ += curr_order_[i]->statement == focal_ ? "[" : "(";
    	    curr_pattern_ += curr_order_[i]->serialization;
    	    for (size_t j = 0; j < curr_order_[i]->adjacency.size(); ++j) {
    	    	if (curr_order_[i]->adjacency[j])
		    curr_pattern_ += ToCString(j);
	    }
    	    curr_pattern_ += curr_order_[i]->statement == focal_ ? "]" : ")";
	}

/*
	vector<Node*> old_order;
        if (pivot - index > 1) {
            old_order = curr_order_;
            cout << "(" << index << "," << pivot << ") before:";
            for (size_t i = index; i < pivot; ++i)
                cout << " " << ((long) curr_order_[i]->FindRoot());
            cout << endl;
        }
*/

        SearchOrder(pivot);

/*
        if (pivot - index > 1) {
            for (size_t i = index; i < pivot; ++i) {
                if (old_order[i] != curr_order_[i]) {
                    cout << "(" << index << "," << pivot << ") after :";
                    for (size_t i = index; i < pivot; ++i)
                        cout << " " << ((long) curr_order_[i]->FindRoot());
                    cout << endl;
                    break;
                }
            }
        }
*/

    	for (size_t i = index; i < pivot; ++i) {
    	    for (list<Node*>::iterator k = curr_order_[i]->successors.begin(); k != curr_order_[i]->successors.end(); ++k)
    	    	(*k)->adjacency[i] = false;
	}
    	/*if (pivot - index > 3) {
    	    random_shuffle(curr_order_.begin() + index, curr_order_.begin() + pivot);
	} else*/
	if (!next_permutation(curr_order_.begin() + index, curr_order_.begin() + pivot, mem_fun_less(this, &PDGExtractor::CompareSymmetry)))
	    break;
    }
}

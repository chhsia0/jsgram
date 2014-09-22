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

#ifndef OPERATIONPRINTER_H
#define OPERATIONPRINTER_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include "CanonicalAst.h"

using std::map;
using std::set;
using std::string;
using std::vector;

class OperationPrinter : public CanonicalAstVisitor {
    public:
	string Print(Statement* node);

#define DECLARE_VISIT(type) \
	void Visit##type(type* node);
	AST_NODE_LIST(DECLARE_VISIT)
	CANONICAL_NODE_LIST(DECLARE_VISIT)
#undef DECLARE_VISIT

    private:
	string value_;
};

#endif  // OPERATIONPRINTER_H

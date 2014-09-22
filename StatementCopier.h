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

#ifndef STATEMENTCOPIER_H
#define STATEMENTCOPIER_H

#include <ast.h>
#include <isolate.h>
#include <map>

using namespace v8::internal;
using std::map;

class StatementCopier : public AstVisitor {
	public:
		StatementCopier() : isolate_(Isolate::Current()), factory_(isolate_, isolate_->runtime_zone()) { }

		inline void Visit(AstNode* node) { node->Accept(this); }
#define DECLARE_VISIT(type) \
		void Visit##type(type* node);
		AST_NODE_LIST(DECLARE_VISIT)
#undef DECLARE_VISIT

		Statement* Copy(Statement* stmt);

	private:
		Isolate* isolate_;
		Statement* copy_;
		map<BreakableStatement*, BreakableStatement*> targets_;
		AstNodeFactory<AstNullVisitor> factory_;

		inline Isolate* isolate() { return isolate_; }
		CaseClause* CopyCaseClause(CaseClause* clause);
};

#endif  // STATEMENTCOPIER_H

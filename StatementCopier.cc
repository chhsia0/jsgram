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

#include "StatementCopier.h"

#define DEFINE_VISIT(type) \
void StatementCopier::Visit##type(type* node) { }
DECLARATION_NODE_LIST(DEFINE_VISIT)
MODULE_NODE_LIST(DEFINE_VISIT)
EXPRESSION_NODE_LIST(DEFINE_VISIT)
DEFINE_VISIT(ModuleStatement)
#undef DEFINE_VISIT

void StatementCopier::VisitBlock(Block* node) {
	Block* block = factory_.NewBlock(node->labels(), node->statements()->length(), node->is_initializer_block());
	for (int i = 0; i < node->statements()->length(); ++i) {
		Visit(node->statements()->at(i));
		block->AddStatement(copy_, isolate()->runtime_zone());
	}
	copy_ = block;
	targets_[node] = block;
}

void StatementCopier::VisitExpressionStatement(ExpressionStatement* node) {
	copy_ = factory_.NewExpressionStatement(node->expression());
}

void StatementCopier::VisitEmptyStatement(EmptyStatement* node) {
	copy_ = factory_.NewEmptyStatement();
}

void StatementCopier::VisitIfStatement(IfStatement* node) {
	Visit(node->then_statement());
	Statement* then_stmt = copy_;
	Visit(node->else_statement());
	Statement* else_stmt = copy_;
	copy_ = factory_.NewIfStatement(node->condition(), then_stmt, else_stmt);
}

void StatementCopier::VisitContinueStatement(ContinueStatement* node) {
	if (targets_.find(node->target()) == targets_.end())
		copy_ = factory_.NewContinueStatement(node->target());
	else
		copy_ = factory_.NewContinueStatement(targets_[node->target()]->AsIterationStatement());
}

void StatementCopier::VisitBreakStatement(BreakStatement* node) {
	if (targets_.find(node->target()) == targets_.end())
		copy_ = factory_.NewBreakStatement(node->target());
	else
		copy_ = factory_.NewBreakStatement(targets_[node->target()]);
}

void StatementCopier::VisitReturnStatement(ReturnStatement* node) {
	copy_ = factory_.NewReturnStatement(node->expression());
}

void StatementCopier::VisitWithStatement(WithStatement* node) {
	Visit(node->statement());
	copy_ = factory_.NewWithStatement(node->expression(), copy_);
}

void StatementCopier::VisitSwitchStatement(SwitchStatement* node) {
	SwitchStatement* switch_stmt = factory_.NewSwitchStatement(node->labels());
	ZoneList<CaseClause*>* cases = new(isolate()->runtime_zone()) ZoneList<CaseClause*>(node->cases()->length(), isolate()->runtime_zone());
	for (int i = 0; i < node->cases()->length(); ++i)
		cases->Add(CopyCaseClause(node->cases()->at(i)), isolate()->runtime_zone());
	switch_stmt->Initialize(node->tag(), cases);
	copy_ = switch_stmt;
	targets_[node] = switch_stmt;
}

void StatementCopier::VisitDoWhileStatement(DoWhileStatement* node) {
	DoWhileStatement* do_while_stmt = factory_.NewDoWhileStatement(node->labels());
	Visit(node->body());
	do_while_stmt->Initialize(node->cond(), copy_);
	copy_ = do_while_stmt;
	targets_[node] = do_while_stmt;
}

void StatementCopier::VisitWhileStatement(WhileStatement* node) {
	WhileStatement* while_stmt = factory_.NewWhileStatement(node->labels());
	Visit(node->body());
	while_stmt->Initialize(node->cond(), copy_);
	copy_ = while_stmt;
	targets_[node] = while_stmt;
}

void StatementCopier::VisitForStatement(ForStatement* node) {
	ForStatement* for_stmt = factory_.NewForStatement(node->labels());
	Visit(node->init());
	Statement* init = copy_;
	Visit(node->next());
	Statement* next = copy_;
	Visit(node->body());
	for_stmt->Initialize(init, node->cond(), next, copy_);
	copy_ = for_stmt;
	targets_[node] = for_stmt;
}

void StatementCopier::VisitForInStatement(ForInStatement* node) {
	ForInStatement* for_in_stmt = factory_.NewForInStatement(node->labels());
	Visit(node->body());
	for_in_stmt->Initialize(node->each(), node->enumerable(), copy_);
	copy_ = for_in_stmt;
	targets_[node] = for_in_stmt;
}

void StatementCopier::VisitTryCatchStatement(TryCatchStatement* node) {
	Visit(node->try_block());
	Block* try_block = reinterpret_cast<Block*>(copy_);
	Visit(node->catch_block());
	Block* catch_block = reinterpret_cast<Block*>(copy_);
	copy_ = factory_.NewTryCatchStatement(node->index(), try_block, node->scope(), node->variable(), catch_block); 
}

void StatementCopier::VisitTryFinallyStatement(TryFinallyStatement* node) {
	Visit(node->try_block());
	Block* try_block = reinterpret_cast<Block*>(copy_);
	Visit(node->finally_block());
	Block* finally_block = reinterpret_cast<Block*>(copy_);
	copy_ = factory_.NewTryFinallyStatement(node->index(), try_block, finally_block); 
}

void StatementCopier::VisitDebuggerStatement(DebuggerStatement* node) {
	copy_ = factory_.NewDebuggerStatement();
}

Statement* StatementCopier::Copy(Statement* stmt) {
	Visit(stmt);
	return copy_;
}

CaseClause* StatementCopier::CopyCaseClause(CaseClause* clause) {
	ZoneList<Statement*>* stmts = new(isolate()->runtime_zone()) ZoneList<Statement*>(clause->statements()->length(), isolate()->runtime_zone());
	for (int i = 0; i < clause->statements()->length(); ++i) {
		Visit(clause->statements()->at(i));
		stmts->Add(copy_, isolate()->runtime_zone());
	}
	return new(isolate()->runtime_zone()) CaseClause(isolate(), clause->label(), stmts, clause->position());
}

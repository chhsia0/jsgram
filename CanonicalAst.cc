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

#include "CanonicalAst.h"

#include <checks.h>
#include "StatementCopier.h"

#define DEFINE_ACCEPT(type) \
void type::Accept(AstVisitor* v) { \
    reinterpret_cast<CanonicalAstVisitor*>(v)->Visit##type(this); \
}
CANONICAL_NODE_LIST(DEFINE_ACCEPT)
#undef DEFINE_ACCEPT

void CanonicalAstConverter::VisitBlock(Block* node) {
    if (node->statements() == NULL)
	return;
    ConvertStatements(node->scope(), node->statements());
}

void CanonicalAstConverter::VisitVariableDeclaration(VariableDeclaration* node) {
}

void CanonicalAstConverter::VisitFunctionDeclaration(FunctionDeclaration* node) {
    VisitFunctionLiteral(node->fun());
}

void CanonicalAstConverter::VisitModuleDeclaration(ModuleDeclaration* node) {
    Visit(node->module());
}

void CanonicalAstConverter::VisitImportDeclaration(ImportDeclaration* node) {
    Visit(node->module());
}

void CanonicalAstConverter::VisitExportDeclaration(ExportDeclaration* node) {
}

void CanonicalAstConverter::VisitModuleLiteral(ModuleLiteral* node) {
    Visit(node->body());
}

void CanonicalAstConverter::VisitModuleVariable(ModuleVariable* node) {
    Visit(node->proxy());
}

void CanonicalAstConverter::VisitModulePath(ModulePath* node) {
    Visit(node->module());
    //PrintLiteral(node->name(), false);
}

void CanonicalAstConverter::VisitModuleUrl(ModuleUrl* node) {
    //PrintLiteral(node->url(), true);
}

void CanonicalAstConverter::VisitModuleStatement(ModuleStatement* node) {
    Visit(node->body());
}

void CanonicalAstConverter::VisitExpressionStatement(ExpressionStatement* node) {
    Visit(node->expression());
    AstNode::Type type = node->expression()->node_type();
    if (type != AstNode::kAssignment && type != AstNode::kCountOperation)
	Canonicalize(value_);
    if (node == iterator_->Get())
	iterator_->Remove();
}

void CanonicalAstConverter::VisitEmptyStatement(EmptyStatement* node) {
    iterator_->Remove();
}

void CanonicalAstConverter::VisitIfStatement(IfStatement* node) {
    Visit(node->condition());
    Expression* condition = Canonicalize(value_);
    Statement* then_statement = Wrap(node->then_statement());
    Visit(then_statement);
    Statement* else_statement = Wrap(node->else_statement());
    Visit(else_statement);
    IfStatement* if_stmt = factory_.NewIfStatement(condition, then_statement, else_statement);
    iterator_->Insert(if_stmt);
    iterator_->Remove();
}

void CanonicalAstConverter::VisitContinueStatement(ContinueStatement* node) {
    /*ZoneStringList* labels = node->target()->labels();
    if (labels != NULL) {
    	ASSERT(labels->length() > 0);  // guaranteed to have at least one entry
      	PrintLiteral(labels->at(0), false);  // any label from the list is fine
    }*/
}

void CanonicalAstConverter::VisitBreakStatement(BreakStatement* node) {
    /*ZoneStringList* labels = node->target()->labels();
    if (labels != NULL) {
    	ASSERT(labels->length() > 0);  // guaranteed to have at least one entry
      	PrintLiteral(labels->at(0), false);  // any label from the list is fine
    }*/
}

void CanonicalAstConverter::VisitReturnStatement(ReturnStatement* node) {
    Visit(node->expression());
    iterator_->Insert(factory_.NewReturnStatement(Canonicalize(value_)));
    iterator_->Remove();
}

void CanonicalAstConverter::VisitWithStatement(WithStatement* node) {
    Visit(node->expression());
    Canonicalize(value_);
    Block* body = Wrap(node->statement());
    Visit(body);
    iterator_->Insert(body);
    iterator_->Remove();
}

void CanonicalAstConverter::VisitSwitchStatement(SwitchStatement* node) {
    Visit(node->tag());
    Expression* tag = Canonicalize(value_);
    ZoneList<CaseClause*>* cases = node->cases();
    for (int i = 0; i < cases->length(); i++) {
	Expression* label = NULL;
	if (!cases->at(i)->is_default()) {
	    Visit(cases->at(i)->label());
	    label = Canonicalize(value_);
	}
	ConvertStatements(NULL, cases->at(i)->statements());
	cases->at(i) = new(isolate()->runtime_zone()) CaseClause(isolate(), label, cases->at(i)->statements(), cases->at(i)->position());
    }
    node->Initialize(tag, cases);
}

void CanonicalAstConverter::VisitDoWhileStatement(DoWhileStatement* node) {
    Block* body = Wrap(node->body());
    Expression* condition = node->cond();
    if (node->cond()->node_type() != AstNode::kLiteral && node->cond()->node_type() != AstNode::kVariableProxy) {
	condition = factory_.NewTemporary(iterator_->scope());
	Assignment* update = factory_.NewAssignment(Token::ASSIGN, condition, node->cond(), RelocInfo::kNoPosition);
	body->statements()->Add(factory_.NewExpressionStatement(update), isolate()->runtime_zone());
    }
    Block* block = reinterpret_cast<Block*>(StatementCopier().Copy(body));
    WhileStatement *loop = factory_.NewWhileStatement(NULL);
    loop->Initialize(condition, body);
    block->AddStatement(loop, isolate()->runtime_zone());
    Visit(block);
    iterator_->Insert(block);
    iterator_->Remove();
}

void CanonicalAstConverter::VisitWhileStatement(WhileStatement* node) {
    Visit(node->cond());
    Expression* condition = Canonicalize(value_);
    Block* body = Wrap(node->body());
    if (node->cond() != condition) {
	Assignment* update = factory_.NewAssignment(Token::ASSIGN, condition, node->cond(), RelocInfo::kNoPosition);
	body->statements()->Add(factory_.NewExpressionStatement(update), isolate()->runtime_zone());
    }
    Visit(body);
    node->Initialize(condition, body);
}

void CanonicalAstConverter::VisitForStatement(ForStatement* node) {
    if (node->init() != NULL)
	Visit(node->init());
    Expression* condition = factory_.NewLiteral(isolate()->factory()->true_value());
    if (node->cond() != NULL) {
	Visit(node->cond());
	condition = Canonicalize(value_);
    }
    Block* body = Wrap(node->body());
    if (node->next() != NULL)
	body->statements()->Add(node->next(), isolate()->runtime_zone());
    if (node->cond() != NULL && node->cond() != condition) {
	Assignment* update = factory_.NewAssignment(Token::ASSIGN, condition, node->cond(), RelocInfo::kNoPosition);
	body->statements()->Add(factory_.NewExpressionStatement(update), isolate()->runtime_zone());
    }
    Visit(body);
    WhileStatement *loop = factory_.NewWhileStatement(NULL);
    loop->Initialize(condition, body);
    iterator_->Insert(loop);
    iterator_->Remove();
}

void CanonicalAstConverter::VisitForInStatement(ForInStatement* node) {
    Expression* each = node->each();
    Block* body = Wrap(node->body());
    if (each->node_type() != AstNode::kVariableProxy) {
	each = factory_.NewTemporary(iterator_->scope());
	Assignment* update = factory_.NewAssignment(Token::ASSIGN, node->each(), each, RelocInfo::kNoPosition);
	body->statements()->InsertAt(0, factory_.NewExpressionStatement(update), isolate()->runtime_zone());
    }
    Visit(node->enumerable());
    Expression* enumerable = Canonicalize(value_);
    Visit(body);
    node->Initialize(each, enumerable, body);
}

void CanonicalAstConverter::VisitTryCatchStatement(TryCatchStatement* node) {
    Visit(node->try_block());
    iterator_->Insert(node->try_block());
    iterator_->Remove();
    //PrintLiteral(node->variable()->name(), false);
    //Visit(node->catch_block());
}

void CanonicalAstConverter::VisitTryFinallyStatement(TryFinallyStatement* node) {
    Visit(node->try_block());
    Visit(node->finally_block());
    iterator_->Insert(node->try_block());
    iterator_->Insert(node->finally_block());
    iterator_->Remove();
}

void CanonicalAstConverter::VisitDebuggerStatement(DebuggerStatement* node) {
}

void CanonicalAstConverter::VisitFunctionLiteral(FunctionLiteral* node) {
    if (functions_.count(node))
	return;
    functions_.insert(node);
    for (int i = 0; i < node->scope()->declarations()->length(); i++)
	Visit(node->scope()->declarations()->at(i));
    //ZoneList<Statement*>* body = new(isolate()->runtime_zone()) ZoneList<Statement*>(*node->body(), isolate()->runtime_zone());
    ZoneList<Statement*>* body = new(isolate()->runtime_zone()) ZoneList<Statement*>(node->body()->length(), isolate()->runtime_zone());
    body->AddAll(*node->body(), isolate()->runtime_zone());
    ConvertStatements(node->scope(), body);
    CanonicalFunctionEntry *func = factory_.NewCanonicalFunctionEntry(node, body);
    node->body()->Clear();
    node->body()->Add(func, isolate()->runtime_zone());
    node->body()->Add(factory_.NewCanonicalFunctionExit(func), isolate()->runtime_zone());
    value_ = node;
}

void CanonicalAstConverter::VisitSharedFunctionInfoLiteral(SharedFunctionInfoLiteral* node) {
    //PrintLiteral(node->shared_function_info(), true);
    value_ = node;
}

void CanonicalAstConverter::VisitConditional(Conditional* node) {
    Visit(node->condition());
    Expression* condition = Canonicalize(value_);
    Visit(node->then_expression());
    Expression* then_expr = Canonicalize(value_);
    Visit(node->else_expression());
    Expression* else_expr = Canonicalize(value_);
    value_ = factory_.NewConditional(condition, then_expr, else_expr, node->then_expression_position(), node->else_expression_position());
}

void CanonicalAstConverter::VisitLiteral(Literal* node) {
    value_ = node;
}

void CanonicalAstConverter::VisitRegExpLiteral(RegExpLiteral* node) {
    value_ = node;
}

void PrintLiteral(FILE* fp, Expression* node) {
    if (node->AsLiteral() != NULL) {
	String* str = String::cast(*node->AsLiteral()->handle());
	for (int i = 0; i < str->length(); ++i)
	    fputc(str->Get(i), fp);
	fputc('\n', fp);
    }
}

void CanonicalAstConverter::VisitObjectLiteral(ObjectLiteral* node) {
    VariableProxy* object = factory_.NewVariableProxy(global_scope_->DeclareDynamicGlobal(isolate()->factory()->Object_symbol()));
    CallNew* new_object = factory_.NewCallNew(object, new(isolate()->runtime_zone()) ZoneList<Expression*>(0, isolate()->runtime_zone()), RelocInfo::kNoPosition);
    VariableProxy* temp = factory_.NewTemporary(iterator_->scope());
    iterator_->Insert(factory_.NewCanonicalAssignment(temp, new_object));
    for (int i = 0; i < node->properties()->length(); i++) {
	ObjectLiteral::Property* property = node->properties()->at(i);
	Visit(property->key());
	Expression* key = Canonicalize(value_);
	Visit(property->value());
	Expression* value = Canonicalize(value_);
	iterator_->Insert(factory_.NewCanonicalPropertyAssignment(factory_.NewProperty(temp, key, RelocInfo::kNoPosition), value));
    }
    value_ = temp;
}

void CanonicalAstConverter::VisitArrayLiteral(ArrayLiteral* node) {
    VariableProxy* array = factory_.NewVariableProxy(global_scope_->DeclareDynamicGlobal(isolate()->factory()->Array_symbol()));
    CallNew* new_array = factory_.NewCallNew(array, new(isolate()->runtime_zone()) ZoneList<Expression*>(0, isolate()->runtime_zone()), RelocInfo::kNoPosition);
    VariableProxy* temp = factory_.NewTemporary(iterator_->scope());
    iterator_->Insert(factory_.NewCanonicalAssignment(temp, new_array));
    for (int i = 0; i < node->values()->length(); i++) {
	Expression* key = factory_.NewNumberLiteral(i);
	Visit(node->values()->at(i));
	Expression* value = Canonicalize(value_);
	iterator_->Insert(factory_.NewCanonicalPropertyAssignment(factory_.NewProperty(temp, key, RelocInfo::kNoPosition), value));
    }
    value_ = temp;
}

void CanonicalAstConverter::VisitVariableProxy(VariableProxy* node) {
    value_ = node;
}

void CanonicalAstConverter::VisitAssignment(Assignment* node) {
    Visit(node->target());
    Expression* target = value_;
    Visit(node->is_compound() ? node->binary_operation() : node->value());
    switch (target->node_type()) {
	case AstNode::kVariableProxy:
	    iterator_->Insert(factory_.NewCanonicalAssignment(target, value_));
	    value_ = target;
	    break;
	case AstNode::kProperty:
	    value_ = Canonicalize(value_);
	    iterator_->Insert(factory_.NewCanonicalPropertyAssignment(target, value_));
	    break;
	default:
	    UNREACHABLE();
    }
}

void CanonicalAstConverter::VisitThrow(Throw* node) {
    Visit(node->exception());
    Expression* exception = Canonicalize(value_);
    value_ = factory_.NewThrow(exception, node->position());
}

void CanonicalAstConverter::VisitProperty(Property* node) {
    Visit(node->obj());
    Expression* obj = Canonicalize(value_);	
    Visit(node->key());
    Expression* key = Canonicalize(value_);
    value_ = factory_.NewProperty(obj, key, node->position());
}

void CanonicalAstConverter::VisitCall(Call* node) {
    Visit(node->expression());
    Expression* expr = value_;
    for (int i = 0; i < node->arguments()->length(); i++) {
	Visit(node->arguments()->at(i));
	node->arguments()->at(i) = Canonicalize(value_);
    }
    value_ = factory_.NewCall(expr, node->arguments(), node->position());
}

void CanonicalAstConverter::VisitCallNew(CallNew* node) {
    Visit(node->expression());
    Expression* expr = value_;
    for (int i = 0; i < node->arguments()->length(); i++) {
	Visit(node->arguments()->at(i));
	node->arguments()->at(i) = Canonicalize(value_);
    }
    value_ = factory_.NewCallNew(expr, node->arguments(), node->position());
}

void CanonicalAstConverter::VisitCallRuntime(CallRuntime* node) {
    for (int i = 0; i < node->arguments()->length(); i++) {
	Visit(node->arguments()->at(i));
	node->arguments()->at(i) = Canonicalize(value_);
    }
    value_ = node;
}

void CanonicalAstConverter::VisitUnaryOperation(UnaryOperation* node) {
    Visit(node->expression());
    Expression* expression = Canonicalize(value_);
    value_ = factory_.NewUnaryOperation(node->op(), expression, node->position());
}

void CanonicalAstConverter::VisitCountOperation(CountOperation* node) {
    Visit(node->expression());
    Expression* expr = value_;
    Expression* target = Canonicalize(expr);
    if (node->is_postfix()) {
	VariableProxy* temp = factory_.NewTemporary(iterator_->scope());
	iterator_->Insert(factory_.NewCanonicalAssignment(temp, target));
	value_ = temp;
    }
    Expression* value = factory_.NewBinaryOperation(node->binary_op(), target, factory_.NewNumberLiteral(1), node->position());
    iterator_->Insert(factory_.NewCanonicalAssignment(target, value));
    if (node->is_prefix())
	value_ = target;
    if (expr->node_type() == AstNode::kProperty) {
	iterator_->Insert(factory_.NewCanonicalPropertyAssignment(expr, target));
    }
}

void CanonicalAstConverter::VisitBinaryOperation(BinaryOperation* node) {
    Visit(node->left());
    Expression* left = Canonicalize(value_);
    Visit(node->right());
    Expression* right = Canonicalize(value_);
    value_ = factory_.NewBinaryOperation(node->op(), left, right, node->position());
}

void CanonicalAstConverter::VisitCompareOperation(CompareOperation* node) {
    Visit(node->left());
    Expression* left = Canonicalize(value_);
    Visit(node->right());
    Expression* right = Canonicalize(value_);
    value_ = factory_.NewCompareOperation(node->op(), left, right, node->position());
}

void CanonicalAstConverter::VisitThisFunction(ThisFunction* node) {
}

void CanonicalAstConverter::Convert(CompilationInfo* info) {
    functions_.clear();
    global_scope_ = info->global_scope();
    Visit(info->function());
}

void CanonicalAstConverter::ConvertStatements(Scope* scope, ZoneList<Statement*>* statements) {
    ZoneListIterator<Statement>* parent = iterator_;
    if (scope == NULL)
	scope = parent->scope();
    iterator_ = new ZoneListIterator<Statement>(scope, statements);
    for (; !iterator_->IsDone(); iterator_->Next())
	Visit(iterator_->Get());
    delete iterator_;
    iterator_ = parent;
}

Expression* CanonicalAstConverter::Canonicalize(Expression* expr) {
    VariableProxy* temp;
    switch (expr->node_type()) {
	case AstNode::kVariableProxy:
	case AstNode::kLiteral:
	case AstNode::kRegExpLiteral:
	    return expr;
	default:
	    temp = factory_.NewTemporary(iterator_->scope());
	    iterator_->Insert(factory_.NewCanonicalAssignment(temp, expr));
	    return temp;
    }
}

Block* CanonicalAstConverter::Wrap(Statement* stmt) {
    if (stmt->node_type() == AstNode::kBlock)
	return reinterpret_cast<Block*>(stmt);
    Block* block = factory_.NewBlock(NULL, 16, false);
    block->AddStatement(stmt, isolate()->runtime_zone());
    return block;
}

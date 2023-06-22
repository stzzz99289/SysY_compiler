%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN LEQ GEQ EQ NEQ LAND LOR CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
// lv1&2
%type <int_val> Number
%type <ast_val> FuncDef FuncType Block 
%type <ast_val> Stmt

// lv3
%type <ast_val> ExpList Exp PrimaryExp UnaryExp UnaryOp 
%type <ast_val> MulExp AddExp
%type <ast_val> RelExp EqExp LAndExp LOrExp

// lv4
%type <ast_val> VarDecl VarDefList VarDef InitVal
%type <ast_val> ConstExp ConstDecl BType ConstDef ConstDefList ConstInitVal
%type <ast_val> BlockItem BlockItemList
%type <ast_val> LVal
%type <ast_val> Decl

// lv 5,6,7
%type <ast_val> MatchedStmt UnmatchedStmt

%%

// start symbols
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// basic symbols
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    // $$ = new string("int");
    auto ast = new FuncTypeAST();
    $$ = ast;
  }
  ;

Block
  : '{' BlockItemList '}' {
    auto ast = new BlockAST();
    ast->block_item_lst = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

BlockItemList
  : /* empty */ {
    auto ast = new BlockItemListAST_emp();
    $$ = ast;
  }
  | BlockItemList BlockItem {
    auto ast = new BlockItemListAST_lst();
    ast->block_item_lst = unique_ptr<BaseAST>($1);
    ast->block_item = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->some_block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->some_block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

MatchedStmt
  : RETURN ExpList ';' {
    auto ast = new MatchedStmtAST_ret();
    ast->exp_list = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new MatchedStmtAST_var();
    ast->l_val = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Block {
    auto ast = new MatchedStmtAST_blk();
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ExpList ';' {
    auto ast = new MatchedStmtAST_lst();
    ast->exp_list = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' MatchedStmt ELSE MatchedStmt {
    auto ast = new MatchedStmtAST_ifelse();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->matched_stmt_if = unique_ptr<BaseAST>($5);
    ast->matched_stmt_else = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' MatchedStmt {
    auto ast = new MatchedStmtAST_while();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->matched_stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new MatchedStmtAST_break();
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new MatchedStmtAST_continue();
    $$ = ast;
  }
  ;
UnmatchedStmt
  : IF '(' Exp ')' Stmt {
    auto ast = new UnmatchedStmtAST_if();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' MatchedStmt ELSE UnmatchedStmt {
    auto ast = new UnmatchedStmtAST_ifelse();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->matched_stmt = unique_ptr<BaseAST>($5);
    ast->unmatched_stmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  ;
Stmt
  : MatchedStmt {
    auto ast = new StmtAST();
    ast->some_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnmatchedStmt {
    auto ast = new StmtAST();
    ast->some_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// expression symbols
ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ExpList
  : /* empty */ {
    auto ast = new ExpListAST_emp();
    $$ = ast;
  }
  | ExpList Exp {
    auto ast = new ExpListAST_lst();
    ast->exp_list = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->some_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST_exp();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST_val();
    ast->l_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST_num();
    ast->number = $1;
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST_pri();
    ast->primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST_uop();
    ast->unary_op = unique_ptr<BaseAST>($1);
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

UnaryOp
  : '+' { 
    auto ast = new UnaryOpAST();
    ast->op = "+";
    $$ = ast;
  }
  | '-' { 
    auto ast = new UnaryOpAST();
    ast->op = "-";
    $$ = ast;
  }
  | '!' { 
    auto ast = new UnaryOpAST();
    ast->op = "!";
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new MulExpAST_una();
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    auto ast = new MulExpAST_mul();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = "*";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new MulExpAST_mul();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = "/";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    auto ast = new MulExpAST_mul();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = "%";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast = new AddExpAST_mul();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExpAST_add();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = "+";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new AddExpAST_add();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = "-";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    auto ast = new RelExpAST_add();
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp '<' AddExp {
    auto ast = new RelExpAST_rel();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = "<";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp '>' AddExp {
    auto ast = new RelExpAST_rel();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = ">";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp LEQ AddExp {
    auto ast = new RelExpAST_rel();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = "<=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp GEQ AddExp {
    auto ast = new RelExpAST_rel();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = ">=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    auto ast = new EqExpAST_rel();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQ RelExp {
    auto ast = new EqExpAST_eq();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->op = "==";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | EqExp NEQ RelExp {
    auto ast = new EqExpAST_eq();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->op = "!=";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new LAndExpAST_eq();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp LAND EqExp {
    auto ast = new LAndExpAST_and();
    ast->land_exp = unique_ptr<BaseAST>($1);
    ast->eq_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST_and();
    ast->land_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp LOR LAndExp {
    auto ast = new LOrExpAST_or();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    ast->land_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

// variable symbols
Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->some_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->some_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDefList ';' {
    auto ast = new ConstDeclAST();
    ast->b_type = unique_ptr<BaseAST>($2);
    ast->const_def_list = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstDefList
  : ConstDef {
    auto ast = new ConstDefListAST_def();
    ast->const_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ConstDefList ',' ConstDef {
    auto ast = new ConstDefListAST_lst();
    ast->const_def_list = unique_ptr<BaseAST>($1);
    ast->const_def = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

BType
  : INT {
    auto ast = new BTypeAST();
    $$ = ast;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);;
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);;
    $$ = ast;
  }
  ;
VarDecl
  : BType VarDefList ';' {
    auto ast = new VarDeclAST();
    ast->b_type = unique_ptr<BaseAST>($1);
    ast->var_def_list = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;
VarDefList
  : VarDef {
    auto ast = new VarDefListAST_def();
    ast->var_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDefList ',' VarDef {
    auto ast = new VarDefListAST_lst();
    ast->var_def_list = unique_ptr<BaseAST>($1);
    ast->var_def = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
VarDef
  : IDENT {
    auto ast = new VarDefAST_dec();
    ast->ident = *unique_ptr<std::string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST_def();
    ast->ident = *unique_ptr<std::string>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;
// const symbol
Number
  : INT_CONST {
    $$ = $1;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}

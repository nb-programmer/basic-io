#pragma once

#include "ast.h"
#include "basic_runner.h"

ASTNodeData basic_fn_print(BASICRuntime *runtime, ASTNode *args);
ASTNodeData basic_fn_max(BASICRuntime *runtime, ASTNode *args);
ASTNodeData basic_fn_min(BASICRuntime *runtime, ASTNode *args);
ASTNodeData basic_fn_sleep(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData basic_fn_toint(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData basic_fn_toflt(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData basic_fn_rand(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData basic_fn_irand(BASICRuntime *runtime, ASTNode *arg);

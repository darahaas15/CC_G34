#include "llvmcodegen.hh"
#include "ast.hh"
#include <iostream>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <vector>

#define MAIN_FUNC compiler->module.getFunction("main")

/*
The documentation for LLVM codegen, and how exactly this file works can be found
ins `docs/llvm.md`
*/

char *constants[13] = {"ifcont", "else", "then", "merge", "ifcond", "printi", "main", "entry", "addtmp", "minustmp", "multtmp", "divtmp", "iftmp"};

void LLVMCompiler::compile(Node *root)
{
    /* Adding reference to print_i in the runtime library */
    // void printi();
    FunctionType *printi_func_type = FunctionType::get(
        builder.getVoidTy(),
        {builder.getInt64Ty()},
        false);
    Function::Create(
        printi_func_type,
        GlobalValue::ExternalLinkage,
        "printi",
        &module);
    /* we can get this later
        module.getFunction("printi");
    */

    /* Main Function */
    // int main();
    FunctionType *main_func_type = FunctionType::get(
        builder.getInt64Ty(), {}, false /* is vararg */
    );
    Function *main_func = Function::Create(
        main_func_type,
        GlobalValue::ExternalLinkage,
        "main",
        &module);

    // create main function block
    BasicBlock *main_func_entry_bb = BasicBlock::Create(
        *context,
        "entry",
        main_func);

    // move the builder to the start of the main function block
    builder.SetInsertPoint(main_func_entry_bb);

    root->llvm_codegen(this);

    // return 0;
    builder.CreateRet(builder.getInt64(0));
}

void LLVMCompiler::dump()
{
    outs() << module;
}

void LLVMCompiler::write(std::string file_name)
{
    std::error_code EC;
    raw_fd_ostream fout(file_name, EC, sys::fs::OF_None);
    WriteBitcodeToFile(module, fout);
    fout.flush();
    fout.close();
}

//  ┌―――――――――――――――――――――┐  //
//  │ AST -> LLVM Codegen │  //
// └―――――――――――――――――――――┘   //

// codegen for statements
Value *NodeStmts::llvm_codegen(LLVMCompiler *compiler)
{
    Value *last = nullptr;
    for (auto node : list)
    {
        last = node->llvm_codegen(compiler);
    }

    return last;
}

Value *NodeDebug::llvm_codegen(LLVMCompiler *compiler)
{
    Value *expr = expression->llvm_codegen(compiler);

    Function *printi_func = compiler->module.getFunction("printi");
    compiler->builder.CreateCall(printi_func, {expr});

    return expr;
}

Value *NodeInt::llvm_codegen(LLVMCompiler *compiler)
{
    return compiler->builder.getInt64(value);
}

Value *NodeBinOp::llvm_codegen(LLVMCompiler *compiler)
{
    Value *left_expr = left->llvm_codegen(compiler);
    Value *right_expr = right->llvm_codegen(compiler);

    switch (op)
    {
    case PLUS:
        return compiler->builder.CreateAdd(left_expr, right_expr, "addtmp");
    case MINUS:
        return compiler->builder.CreateSub(left_expr, right_expr, "minustmp");
    case MULT:
        return compiler->builder.CreateMul(left_expr, right_expr, "multtmp");
    case DIV:
        return compiler->builder.CreateSDiv(left_expr, right_expr, "divtmp");
    }
}

Value *NodeDecl::llvm_codegen(LLVMCompiler *compiler)
{
    Value *expr = expression->llvm_codegen(compiler);

    int sc = compiler->scope;
    IRBuilder<> temp_builder(
        &MAIN_FUNC->getEntryBlock(),
        MAIN_FUNC->getEntryBlock().begin());

    AllocaInst *alloc = temp_builder.CreateAlloca(compiler->builder.getInt64Ty(), 0, identifier);

    compiler->localscope[sc][identifier] = alloc;

    return compiler->builder.CreateStore(expr, alloc);
}

Value *NodeIdent::llvm_codegen(LLVMCompiler *compiler)
{
    int sc = compiler->scope;

    while (sc > 0)
    {
        if (compiler->localscope[sc][identifier] != nullptr)
        {
            break;
        }
        sc--;
    }

    AllocaInst *alloc = compiler->localscope[sc][identifier];

    // if your LLVM_MAJOR_VERSION >= 14
    return compiler->builder.CreateLoad(compiler->builder.getInt64Ty(), alloc, identifier);
}

Value *NodeIfElse::llvm_codegen(LLVMCompiler *compiler)
{

    if (!if_cond && !else_cond)
    {
        return nullptr;
    }

    compiler->scope++;
    Value *cond = condition->llvm_codegen(compiler);

    // Compare cond with 0
    cond = compiler->builder.CreateICmpSLT(compiler->builder.getInt64(0), cond, "ifcond");

    Function *if_function = compiler->builder.GetInsertBlock()->getParent();

    BasicBlock *thenBlock, *elseBlock;

    if (if_cond)
    {
        thenBlock = BasicBlock::Create(*compiler->context, constants[2], if_function);
    }
    else
    {
        thenBlock = BasicBlock::Create(*compiler->context, "empty_then", if_function);
    }

    if (else_cond)
    {
        elseBlock = BasicBlock::Create(*compiler->context, constants[1]);
    }
    else
    {
        elseBlock = BasicBlock::Create(*compiler->context, "empty_else");
    }

    BasicBlock *mergeBlock = BasicBlock::Create(*compiler->context, constants[0]);

    // Branch to the then or else block depending on the condition
    compiler->builder.CreateCondBr(cond, thenBlock, elseBlock);

    // Then Block
    compiler->builder.SetInsertPoint(thenBlock);

    Value *ifConditionValue = if_cond ? if_cond->llvm_codegen(compiler) : nullptr;

    if (ifConditionValue)
    {
        StoreInst *then = dyn_cast<StoreInst>(ifConditionValue);
        if (then)
            ifConditionValue = then->getValueOperand();
    }
    compiler->builder.CreateBr(mergeBlock);
    thenBlock = compiler->builder.GetInsertBlock();

    // Else Block
    Value *elseConditionValue = else_cond ? else_cond->llvm_codegen(compiler) : nullptr;

    if (else_cond)
    {
        if_function->getBasicBlockList().push_back(elseBlock);
        compiler->builder.SetInsertPoint(elseBlock);
        if (elseConditionValue)
        {
            StoreInst *elseStoreValue = dyn_cast<StoreInst>(elseConditionValue);
            if (elseStoreValue)
                elseConditionValue = elseStoreValue->getValueOperand();
        }
        compiler->builder.CreateBr(mergeBlock);
        elseBlock = compiler->builder.GetInsertBlock();
    }
    else
    {
        if_function->getBasicBlockList().push_back(elseBlock);
        compiler->builder.SetInsertPoint(elseBlock);

        compiler->builder.CreateBr(mergeBlock);
    }

    // Merge Block
    if_function->getBasicBlockList().push_back(mergeBlock);
    compiler->builder.SetInsertPoint(mergeBlock);

    // Phi Node
    PHINode *phiNode = compiler->builder.CreatePHI(compiler->builder.getInt64Ty(), 2, "iftmp");

    if (ifConditionValue)
    {
        phiNode->addIncoming(ifConditionValue, thenBlock);
    }
    else
    {
        Constant *nullValue = ConstantInt::get(compiler->builder.getInt64Ty(), 0);
        phiNode->addIncoming(nullValue, thenBlock);
    }

    if (elseConditionValue)
    {
        phiNode->addIncoming(elseConditionValue, elseBlock);
    }
    else
    {
        Constant *nullValue = ConstantInt::get(compiler->builder.getInt64Ty(), 0);
        phiNode->addIncoming(nullValue, elseBlock);
    }

    return phiNode;
}

#undef MAIN_FUNC
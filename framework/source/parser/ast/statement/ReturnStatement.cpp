// Copyright 2025 solar-mist

#include "parser/ast/statement/ReturnStatement.h"

#include <vipir/IR/Instruction/RetInst.h>

#include <cmath>

namespace parser
{
    ReturnStatement::ReturnStatement(Scope* scope, ASTNodePtr value, SourcePair source)
        : ASTNode(scope, source)
        , mReturnValue(std::move(value))
    {
    }

    vipir::Value* ReturnStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        if (!mReturnValue)
        {
            return builder.CreateRet(nullptr);
        }

        auto returnValue = mReturnValue->dcodegen(builder, diBuilder, module, diag);

        return builder.CreateRet(returnValue);
    }

    std::vector<ASTNode*> ReturnStatement::getChildren()
    {
        if (mReturnValue)
            return {mReturnValue.get()};

        return {};
    }

    void ReturnStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        if (mReturnValue)
        {
            mReturnValue->typeCheck(diag, exit);
        }

        auto returnType = mScope->getCurrentReturnType();
        if (returnType->isVoidType())
        {
            if (mReturnValue != nullptr && !mReturnValue->getType()->isVoidType())
            {
                diag.reportCompilerError(
                    mReturnValue->getSourcePair().start,
                    mReturnValue->getSourcePair().end,
                    std::format("value of type '{}{}{}' is not compatible with return type '{}{}{}'",
                        fmt::bold, mReturnValue->getType()->getName(), fmt::defaults,
                        fmt::bold, returnType->getName(), fmt::defaults)
                );
                exit = true;
            }
        }
        else
        {
            if (!mReturnValue)
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("non-void function returning '{}{}{}' cannot return '{}void{}'",
                        fmt::bold, returnType->getName(), fmt::defaults,
                        fmt::bold, fmt::defaults)
                );
                exit = true;
            }
            else if (returnType != mReturnValue->getType())
            {
                if (mReturnValue->canImplicitCast(diag, returnType))
                {
                    mReturnValue = Cast(mReturnValue, returnType);
                }
                else
                {
                    diag.reportCompilerError(
                        mReturnValue->getSourcePair().start,
                        mReturnValue->getSourcePair().end,
                        std::format("value of type '{}{}{}' is not compatible with return type '{}{}{}'",
                            fmt::bold, mReturnValue->getType()->getName(), fmt::defaults,
                            fmt::bold, returnType->getName(), fmt::defaults)
                    );
                    exit = true;
                }
            }
        }
    }
}
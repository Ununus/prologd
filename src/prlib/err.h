#pragma once

enum class ErrorCode {
  NoErrors = 0,
  InvalidCharacter = 1,
  NotEnoughFreeMemory = 2,
  InvalidStringFormat = 3,
  InvalidNumberFormat = 4,
  AbsentSeparator = 5,
  AbsentMinus = 6,
  AbsentClosingBraceOrDef = 7,
  IncorrectUseDef = 8,
  IncorrectUseListAsPredicate = 9,
  IncorrectUseQuestion = 10,
  IncorrectUseUnderscore = 11,
  IncorrectUseExclamation = 12,
  SentenceOverflow = 13,
  UnpairedBracketsInListProcessing = 14,
  WrongList = 15,
  UnpairedBracketsInFunctionProcessing = 16,
  AbsentFunctorBeforeOpenBracket = 17,
  SignNotInArithmeticExpression = 18,
  ErrorInWritingAnArithmeticExpression = 19,
  ErrorWhileParsingSentence = 20,
  CannotOpenFileOut = 21,
  CannotOpenGraphics = 22,
  TooManyCharacterConstants = 23,
  UnknownError = 24,
  NotImplementedPredicate = 25,
  WrongNumberOfArgumentsInBuiltinPredicate = 26,
  ExecutionInterrupted = 27,
  StackOverflow = 28,
  FailedMultiplicationIntegerDivisionOfSubstancesOfNumbers = 29,
  FailedMultiplicationInvalidArguments = 30,
  FailedAddition = 31,
  ErrorInArithmeticExpression = 32,
  IOError = 33,
  ModOperationAppliedToRealOperand = 34,
  DivOperationAppliedToRealOperand = 35,
  ErrorInBuiltinPredicateInt = 36,
  ErrorInBuiltinPredicateFloat = 37,
  ErrorWhileParsingFunction = 38,
  NotEnoughMemoryToRunTheProgram = 39,
  StackOverflowWhileCalculatingArithmeticsExpressions = 40,
  DivisionByZeroWhileCalculatingArithmeticsExpressions = 41,
  CharacterConstantNameBufferOverflow = 42,
  AnExceptionOccurredDuringExecution = 43,
  TooLongList = 44
};

char *GetPrErrText(ErrorCode err);

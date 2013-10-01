
#ifndef __AstSpecificProcessing_H_LOADED__
#define __AstSpecificProcessing_H_LOADED__

#include "rose.h"

/*
 * This is the type of statement processor functions. Given a reference to a
 * statement, perform some action.
 */
typedef void (*StmtProcessorFunction) (SgStatement*);

/*
 * `StmtSimpleProcessing` is a sub-class of `AstSimpleProcessing` which
 * applies a given simple processor to every statement only.
 *
 * The statement processor function is associated with the instance object at
 * construction/instantiation -time. Users of this class need to:
 *
 *   1. Define a function of type `StmtProcessFunction`:
 *
 *      > void processor (SgStatement& stmt) {
 *      >   // analyze and / or transform stmt
 *      >   ...
 *      > }
 *
 *   2. Use it to build an instance of `StmtSimpleProcessing`:
 *
 *      > StmtSimpleProcessing* process = new StmtSimpleProcessing(processor);
 *
 * A common useage is to define the `visitStatement` method in
 * `FunctionBodyProcessing` (see below) via an instance of
 * `StmtSimpleProcessing`. This allows one to focus on the atomic statement
 * processor, leaving the plumbing -- such as handling substatements correctly
 * -- to ROSE.
 */
class StmtSimpleProcessing : public AstSimpleProcessing {
  private:
    StmtProcessorFunction _simple;
  public:
    void visit (SgNode* node) {
      if ( node ) {
        SgStatement* stmt = isSgStatement(node);
        if ( stmt ) {
          _simple(stmt);
        }
      }
    }

    StmtSimpleProcessing(StmtProcessorFunction simple)
        : _simple(simple) { }
};


/*
 * This class is a sub-class of `AstSimpleProcessing` intended for writing
 * analyses and transforms that are restricted to the body of any/all function
 * definitions. Its general `visit` function visits only function definitions,
 * applying the instance object's `visitStatement` to each statement in the
 * function's body.
 *
 * Care is taken so that `visitStatement` is invoked with the correct current
 * scope (i.e., that of the body of the function). This means that definitions
 * of `visitStatement` may safely build and graft ASTs.
 *
 * Sub-classes of `FunctionBodyProcessing` must define `visitStatement`. A
 * common idiom is to do so via an instance of `StmtSimpleProcessing` (see
 * above).
 *
 * `functionOfInterest` allows sub-classes to filter out irrelevant
 * definitions. By default, it accepts all functions.
 *
 */
class FunctionBodyProcessing : public AstSimpleProcessing {
  public:
    virtual bool functionOfInterest (SgFunctionDefinition* fun) {
      return true;
    }

    virtual void visitStatement (SgStatement* stmt) {
    }

    void visit (SgNode *node) {
      SgFunctionDefinition* fun = SageInterface::getEnclosingFunctionDefinition(node);
      if ( fun && this->functionOfInterest(fun) ) {
        SgStatement *stmt = isSgStatement(node);
        if ( stmt ) {
          this->visitStatement(stmt);
        }
      }
    }
};

#endif /* __AstSpecificProcessing_H_LOADED__ */


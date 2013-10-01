/*
 * memory skeleton generator prototype
 */
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <rose.h>
#include <staticSingleAssignment.h>

#include "processPragmas.h"
#include "MemoryAccessSignatures.h"
#include "AstSpecificProcessing.h"
#include "Utils.h"
#include "processPragmas.h"
#include "annotatePragmas.h"
#include "SageTools.h"

#ifdef PROFILING_ENABLED
#include <Profile/Profiler.h>
#endif /* PROFILING_ENABLED */

#ifdef PROFILING_ENABLED 
#define PROFILE_FUNC TAU_PROFILE(__func__, " ", TAU_USER);
#else
#define PROFILE_FUNC
#endif

/* Add an AstAttribute to node signifying that this node is a "generated"
 * access. As in, synthetic and not from the original program text.
 */

void markAsGenerated (SgNode* node) {
PROFILE_FUNC
  AstAttribute* code = new AstAttribute();
  if ( node ) {
    node->setAttribute(GeneratedAccess, code);
  }
}

/* Add an AstAtribute to node signifying that this node has already been
 * visited in the replacement pass.
 */

void markAsVisitedForReplacement(SgNode* node) {
PROFILE_FUNC
  AstAttribute* code = new AstAttribute();
  if ( node ) {
    node->setAttribute(VisitedForReplacement, code);
  }
}


/* Add an AstAttribute marking a node for replacement with 
 * a branch predictor.
 */

void markForBranchPredictor(SgNode* node) {
PROFILE_FUNC
  AstAttribute* code = new AstAttribute();
  if ( node ) {
    node->setAttribute(ReplaceWithBranchPredictor, code);
  }
}

/*

    1. Pragmas => list of pivots, list of control variables

    2. Traversal: mark all instances of same in code
        * AstAttribute
        * AstSimpleProcessing
       Add attribite of this kind: ( SgSymbol
                                   , {- read count -}  Int
                                   , {- write count -} Int
                                   )

    3. Slice: remove all code that doesn't refer to pivots, control vars, or
       changes them.

    4. Traversal: find all stmts with pivots and rewrite:

       4.1 Determine set of r/w accesses
       4.2 Generate statement sequence of read/write
       4.3 Replace original stmt with set of stmts

    5. Traversal: find all conditionals that don't mention control vars or
       pivots
       5.1. Replace conditional with appropriate branch predictor.

 */

// FIXME Global to store symbols known to be for loop indices
std::set<SgSymbol*> indices;

/***
 * passes:
 *   first, spin through every AST element to see whether or not it is a
 *   read or write.
 *
 *   second, for each AST element that is labeled as being read/written,
 *   check context (e.g., basic block vs for loop control).
 ***/


/* Determines if a compound statement is empty:
 *
 * The following statements are empty when their body is an empty compound
 * statement: while statement, do-while statement, for statement, switch
 * statement, case option statement, default option statement.
 *
 * If statements are empty when both branches are empty compound statements.
 *
 * Null statements are always considered empty.
 *
 * Basic blocks are empty when all the statements in the block are empty
 * statements.
 *
 * Otherwise, the statement is not considered empty.
 *
 */

bool
isEmptyCompoundStmt (SgStatement* stmt) {
PROFILE_FUNC
  if ( stmt ) {
    switch ( stmt->variantT() ) {
      case V_SgWhileStmt : {
        SgWhileStmt* ws  = isSgWhileStmt(stmt);
        return isEmptyCompoundStmt(ws->get_body());
      }

      case V_SgDoWhileStmt : {
        SgDoWhileStmt* dws = isSgDoWhileStmt(stmt);
        return isEmptyCompoundStmt(dws->get_body());
      }

      case V_SgSwitchStatement : {
        SgSwitchStatement* ss  = isSgSwitchStatement(stmt);
        return isEmptyCompoundStmt(ss->get_body());
      }

      case V_SgCaseOptionStmt : {
        SgCaseOptionStmt* cos = isSgCaseOptionStmt(stmt);
        return isEmptyCompoundStmt(cos->get_body());
      }

      case V_SgDefaultOptionStmt : {
        SgDefaultOptionStmt* dos = isSgDefaultOptionStmt(stmt);
        return isEmptyCompoundStmt(dos->get_body());
      }

      case V_SgForStatement : {
        SgForStatement* fs  = isSgForStatement(stmt);
        return isEmptyCompoundStmt(fs->get_loop_body());
      }

      case V_SgIfStmt : {
        SgIfStmt* ifs         = isSgIfStmt(stmt);
        return isEmptyCompoundStmt(ifs->get_true_body()) &&
               isEmptyCompoundStmt(ifs->get_false_body());
      }

      case V_SgNullStatement :
        return true;

      case V_SgBasicBlock : {
        SgBasicBlock* bb          = isSgBasicBlock(stmt);
        SgStatementPtrList& stmts = bb->get_statements();
        bool all_empty = true;
        foreach(SgStatement* s, stmts) {
          all_empty = all_empty && isEmptyCompoundStmt(s);
        }
        return all_empty;
      }

      default:
        return false;
    }
  }

  return false;
}

/* Returns true when,
 * 
 * a) node is of type variant v, AND;
 *
 * b) node does not have a parent node of type v
 *
 * We know we've hit the outermost part of the node when there is no parent or
 * the parent is an SgStatement.
 *
 */

bool
isOutermostVariant (SgNode* node, VariantT v) {
PROFILE_FUNC
  if ( node ) {
    if ( node->variantT() == v ) {
      SgNode *parent = node->get_parent();
      while ( parent && ! isSgStatement(parent) ) {
        if ( parent->variantT() == v ) {
          return false;
        }
        parent = parent->get_parent();
      }
      return true;
    }
  }
  return false;
}

/* Follows a path from sub to the root, checking if for dom along the path. If
 * we find it, we say that dom dominates sub.
 *
 * TODO: Can we ask ROSE for the answer of this based on SSA?
 *
 * TODO: Does this match the standard definition for Dominator?
 *
 * DELETEME: Looks like this is unused
 */

bool
isDominator (SgNode* dom, SgNode* sub) {
PROFILE_FUNC
  if ( dom && sub ) {
    SgNode *parent = sub->get_parent();
    while ( parent && parent != dom ) {
      parent = parent->get_parent();
    }
    return parent == dom;
  }
  return false;
}

/* Returns true for VarRefExps, PntrArrRefExps, and InitializedNames. Returns
 * false for everything else.
 *
 * TODO: Isn't this part of ROSE? Should we just use that? A quick glance says:
 * It's not here.
 *
 * TODO: SgRefExp?
 */

bool
isVarRef(SgExpression* expr) {
PROFILE_FUNC
  if ( expr ) {
    switch ( expr->variantT() ) {
    case V_SgVarRefExp:
    case V_SgPntrArrRefExp:
    case V_SgInitializedName:
      return true;
    default:
      return false;
    }
  }

  return false;
}

/* Fetches the the symbol from the given variable reference expression.
 *
 * Hint: If isVarRef returns false for this expression then you will always get
 * NULL from this function.
 */

SgSymbol*
getSymbolFromRefExp(SgExpression* expr) {
PROFILE_FUNC
  if ( expr ) {
    switch ( expr->variantT() ) {
      case V_SgVarRefExp: {
        SgVarRefExp* var = isSgVarRefExp(expr);         ROSE_ASSERT( var );
        return var->get_symbol();
      }

      case V_SgPntrArrRefExp: {
        SgPntrArrRefExp* par = isSgPntrArrRefExp(expr); ROSE_ASSERT( par );
        SgInitializedName* i = SageInterface::convertRefToInitializedName(par);
                                                        ROSE_ASSERT( i );
        return i->search_for_symbol_from_symbol_table();
      }

      case V_SgInitializedName: {
        SgInitializedName* i = isSgInitializedName(expr); ROSE_ASSERT( i );
        return i->search_for_symbol_from_symbol_table();
      }

      default:
        return NULL;
      }
  }

  return NULL;
}

/* Check if sym is present in the refs vector.
 *
 * Note: This has to handle the case where sym is NULL as well as fetching the
 * symbol inside each reference. In the absence of those two complications this
 * function is equivalent to:
 * 
 * std::find(refs::begin(), refs::end(), sym) != refs::end();
 */

bool
symbolIsElementOf( SgSymbol* sym, std::vector< SgNode * > refs) {
PROFILE_FUNC
  if ( sym ) {
    foreach(SgNode *node, refs) {
      SgExpression *ref = isSgExpression(node);
      if ( getSymbolFromRefExp(ref) == sym ) {
        return true;
      }
    }
  }

  return false;
}

/* Check if the expression was marked as a loop index.
 *
 * Note: This refers to the global variable indices
 */

bool
isLoopIndex(SgExpression* expr) {
PROFILE_FUNC
  if ( expr && isVarRef(expr) ) {
    SgSymbol* sym = getSymbolFromRefExp(expr);
    if ( sym ) {
      return indices.find(sym) != indices.end();
    }
  }
  return false;
}

/* A statement is crucial if it is a:
 *
 * break, continue, (computed) goto, label, or null statement.
 *
 * TODO: This used to check for return statements and function calls.  Why were
 * those removed?
 */

bool
isCrucial(SgStatement* stmt) {
PROFILE_FUNC
  if ( stmt ) {
    // if ( containsFunctionCall(stmt) ) {
    //   return true;
    // }

    switch ( stmt->variantT() ) { // any simple control flow
      case V_SgBreakStmt:
      case V_SgContinueStmt:
      case V_SgComputedGotoStatement:
      case V_SgGotoStatement:
      case V_SgLabelStatement:
      case V_SgNullStatement:
      // case V_SgReturnStmt:
        return true;

      default:
        return false;
    }
  }
  return false;
}

/* A statement is a single statement if it is an:
 *
 * assert, assignment, break, continue, (computed) goto, expression, label, or
 * null statment.
 *
 */

bool
isSingleStatement(SgStatement* stmt) {
PROFILE_FUNC
  if ( stmt ) {
    switch ( stmt->variantT() ) {
      case V_SgAssertStmt:
      case V_SgAssignStatement:
      case V_SgBreakStmt:
      case V_SgContinueStmt:
      case V_SgComputedGotoStatement:
      case V_SgGotoStatement:
      case V_SgExprStatement:
      case V_SgLabelStatement:
      case V_SgReturnStmt:
      case V_SgNullStatement:
        return true;

      default:
        return false;
    }
  }
  return false;
}

/* Collectable statements are either function declarations, scope statements,
 * or single statements.
 */

bool
isCollectable (SgStatement* stmt) {
PROFILE_FUNC
  if ( stmt ) {
    SgFunctionDeclaration* func  = isSgFunctionDeclaration(stmt);
    SgScopeStatement*      scope = isSgScopeStatement(stmt);
    return ( func || scope || isSingleStatement(stmt) );
  }
  return false;
}

/* Constant value expressions are one of:
 *
 * a) sizeof operation
 *
 * b) unary op applied to something where:
 * isConstantValueExp(something) == true
 *
 * c) binary op applied to things where:
 * isConstantValueExp(thing1) && isConstantValueExp(thing2) == true
 *
 * d) an SgValue expression
 *
 * TODO: Could we just check the result of SageInterface::constantFolding()?
 */

bool
isConstantValueExp(SgExpression* expr) {
PROFILE_FUNC
  if ( expr ) {
    if ( isSgSizeOfOp(expr) ) return true;
    SgUnaryOp*  unary  = isSgUnaryOp(expr);
    SgBinaryOp* binary = isSgBinaryOp(expr);
    if ( unary ) {
      SgExpression* operand = unary->get_operand();
      return isConstantValueExp(operand);
    } else if ( binary ) {
      SgExpression* lhs = binary->get_lhs_operand();
      SgExpression* rhs = binary->get_rhs_operand();
      return isConstantValueExp(lhs) && isConstantValueExp(rhs);
    } else {
      return isSgValueExp(expr) != NULL;
    }
  }
  return false;
}

/*
ProtectedFunctions protectedCalls;

bool
isProtectedCall(SgExpression* exp) {
  foreach(ProtectedFunction fun, protectedCalls) {
    if ( SageInterface::isCallToParticularFunction(fun.first, fun.second, exp) ) {
      return true;
    }
  }

  return false;
}
*/

/* Returns true when the expression is part of a function call, false
 * otherwise.
 *
 * The test recurses over the parent of the expression.
 *
 * TODO: Won't this always just percolate up the AST? It stops when
 * isSgExpression fails.  I believe this function traverses up the AST until
 * either: a) isSgExpression() returns NULL for the parent of expr, or; b) the
 * expression is equal to the function that is being called.
 *
 * TODO: I'm concerned this function is buggy. It's not obvious how (or when)
 * this function should abort its traversal. If you imagine a part of the AST like this:
 *
 *         p_n
 *        /
 *      ...
 *      /
 * e = p_0
 *
 * Where p_n means the nth parent of e, then the test for pointer equality has the form
 * call_i = p_{i+1}
 * ...
 * call_i->get_function() == p_i
 *
 * So if you expand out call_i:
 * p_{i+1}->get_function() == p_i
 *
 * So we're checking that the parent's get_function() is equal to the current expression.
 * That seems dubious.
 *
 */

bool
partOfFunctionHead( SgExpression* expr ) {
PROFILE_FUNC
  if ( expr ) {
    SgFunctionCallExp* call = isSgFunctionCallExp(expr->get_parent());
    if ( call ) {
      // TODO: What does this test, exactly?
      // call is the function call expression, so it would seem reasonable
      // that get_function() returns the SgNode* for the "to be called" function.
      // Why do we then compare it with expr? If the comparison is true
      // then expr is the function itself?
      return call->get_function() == expr;
    } else {
      return partOfFunctionHead(isSgExpression(expr->get_parent()));
    }
  }
  return false;
}

/* Checks for control flow statements:
 * 
 * Returns true for:
 * if statements, switch statements, while statements, do-while statements
 *
 * TODO: What about for statements? What about ternary operator? goto? others?
 * Is there a rose builtin we could use instead?
 */

bool
isBranch( SgStatement* stmt ) {
PROFILE_FUNC
  return isSgIfStmt(stmt)
      || isSgSwitchStatement(stmt)
      || isSgWhileStmt(stmt)
      || isSgDoWhileStmt(stmt);
}

///* This function ignores the following expression types: 
// *
// * a) a member function reference
// *
// * b) it's part of a function head
// *
// * c) constant values
// */
//
//void
//markReadWrite( SgExpression* expr
//             , std::vector< SgNode * >& reads
//             , std::vector< SgNode * >& writes
//             ) {
//PROFILE_FUNC
//  /* if expr is null, bail out early */
//  if ( expr == NULL ) return;
//
//  /* if it's a mumber function ref, function head, or constant bail early */
//  if ( expr->variantT() == V_SgMemberFunctionRefExp ||
//       partOfFunctionHead(expr)                     ||
//       isConstantValueExp(expr) )
//    return;
//
//  /* Now we have some work to do */
//  if ( isVarRef(expr) ) {
//    SageTools::collectReadWriteRefs(expr, reads, writes, true);
////    if ( expr->isUsedAsLValue() && expr->isLValue() ) {
////      if (debug) std::cout << "  (Used as L-Value)";
////      writes.push_back(expr);
////    } else {
////      reads.push_back(expr);
////    }
////  } else if ( expr->isLValue() ) {
////    // VarRefs may be marked with isLValue if used as an LValue elsewhere in
////    // the enclsoing statement, even if not being used /here/ as such.
////    if (debug) std::cout << "  (Non-VarRef L-Value)";
////    writes.push_back(expr);
////  } else {
////    reads.push_back(expr);
//  }
//}

/* Calls markReadWrite for expressions that are:
 *
 * 1) "atomic" meaning it has no subexpressions
 *
 * 2) Function call nodes
 *
 * 3) Not a compound expression, (eg., not a BinaryOp, UnaryOp, cast
 * expression, this expression, dot expression, arrow expression, size of op,
 * expression list), or a constant value. In the case of constants there is no
 * read/write at run-time and in the compound expression case the children will
 * be handled so we can ignore the expression at this level.
 *
 * This function delegates the marking to the markReadWrite() function.
 *
 */

//void
//collectReadWriteRefsInExpression( SgExpression* expr
//                                , std::vector< SgNode * >& reads
//                                , std::vector< SgNode * >& writes
//                                ) {
//PROFILE_FUNC
//  if ( expr == NULL ) {
//    return;
//  }
//
//  if (debug) 
//    std::cout << ">> Collecting r/w in "
//              << getSgVariant(expr->variantT())                   << std::endl;
//
//  NodeQuerySynthesizedAttributeType children =
//      NodeQuery::querySubTree(expr, V_SgExpression);
//  if ( children.empty() ) { // an atomic expression
//    if (debug) std::cout << ">>    atomic node"                            << std::endl;
//    markReadWrite(expr,reads,writes);
//  } else {
//    if (debug) std::cout << ">>    " << children.size() << " children:"    << std::endl;
//    foreach (SgNode *node, children) {
//      if ( node ) {
//        SgExpression* child = isSgExpression(node);
//        if(debug) std::cout << "\t\t" << getSgVariant(child->variantT());
//        if(debug) std::cout << " "    << child->unparseToString();
//
//        const bool isCompoundExpr =  isSgBinaryOp(child)
//                                  || isSgUnaryOp(child)
//                                  || isSgCastExp(child)
//                                  || isSgThisExp(child)
//                                  || isSgDotExp(child)
//                                  || isSgArrowExp(child)
//                                  || isSgSizeOfOp(child)
//                                  || isSgExprListExp(child)
//                                  || isConstantValueExp(child)
//                                  || SageInterface::isAssignmentStatement(child);
//
//        if ( isFunctionCall(child) || ! isCompoundExpr ) {
//          /*
//           * Don't add child when:
//           *   a) it's a compound expression, as its children will appear
//           *      elsewhere in this same iterator
//           *   b) it's equivalent to a constant (at compile-time, or
//           *      statically), as it has no accesses in that case
//           *
//           */
//          // TODO: um, this should be part of the conditional in the if-statement above
//          //if( !SageInterface::isAssignmentStatement(child) ){
//            //if(debug) std::cout << "isAssignmentStatement: " << child->unparseToString() << std::endl;
//            markReadWrite(child,reads,writes);
//          //}
//        }
//        if(debug) std::cout << std::endl;
//      }
//    }
//  }
//}

/* Remove all the elements of nodes that are dominated by dom.
 */

void
normalizeByDominance(SgNode* dom, std::vector<SgNode*>& nodes) {
PROFILE_FUNC
  for (std::vector<SgNode*>::iterator sub = nodes.begin() ;
       sub != nodes.end() ; ) {
    if ( SageInterface::isAncestor(dom,*sub) ) {
    // if ( isDominator(dom,*sub) ) {
      sub = nodes.erase(sub);
    } else {
      ++sub;
    }
  }
}

/* Take an arbitrary statement and find all the reads and writes to variables
 * and builds separate lists of each.
 *
 * Returns true when it is able to process the given statement and false
 * otherwise.
 *
 * TODO: The returned SgNodes will be of what types? It's hard to say without
 * reading the ROSE source code.
 *
 */

bool
collectReadsWrites( SgStatement* stmt
                  , std::vector< SgNode * >& reads
                  , std::vector< SgNode * >& writes
                  ) {
PROFILE_FUNC
  if ( isCollectable(stmt) && ! containsFunctionCall(stmt) ) {
    SageInterface::collectReadWriteRefs(stmt, reads, writes, true);
  } else {
    if(debug) 
      std::cout << "not collectable, or contains function call: "
                << getSgVariant(stmt->variantT())                 << std::endl;
    SgExprStatement* e_stmt   = isSgExprStatement(stmt);
    SgReturnStmt*    ret_stmt = isSgReturnStmt(stmt);
    if ( e_stmt ) {
      SageTools::collectReadWriteRefs( e_stmt->get_expression()
                                     , reads
                                     , writes
                                     , true );
    } else if ( ret_stmt ) {
      SageTools::collectReadWriteRefs( ret_stmt->get_expression()
                                     , reads
                                     , writes
                                     , true );
    } else {
      if(debug) std::cout << "\t\n!!! case not handled !!!\n"             << std::endl;
      return false;
    }
  }
  return true;
}

/* Process stmt and identify nodes that are read or writen within stmt.
 *
 * Subexpressions that are dominated by a read/write are removed.
 */

void
collectMemoryAccesses( SgStatement* stmt
                     , std::vector< SgNode * >& reads
                     , std::vector< SgNode * >& writes
                     ) {
PROFILE_FUNC
  if ( stmt == NULL ) {
    return;
  }

  /* If the more general collectReadsWrites is unable to process stmt we return
   * immediately.
   */
  if ( ! collectReadsWrites(stmt, reads, writes) ) { return; }

  std::vector < SgNode* > dominantReads;
  std::vector < SgNode* > dominantWrites;

  // Try to identify the outermost part of the read expression
  foreach(SgNode* r, reads) {
    if(debug) std::cout << "\t\tread " << getSgVariant(r->variantT());
    if(debug) std::cout << " "         << r->unparseToString();
    if ( isOutermostVariant(r, V_SgPntrArrRefExp)        ||
         isOutermostVariant(r, V_SgArrowExp)             ||
         // isOutermostVariant(r, V_SgFunctionCallExp)      ||
         isOutermostVariant(r, V_SgDotExp)               ||
         isFunctionCall(r) ) {
      dominantReads.push_back(r);
      if(debug) std::cout << " @";
    }
    if(debug) std::cout << std::endl;
  }

  // make sure to look at LHS too, since a write may induce reads that we
  // want to filter out as well
  foreach(SgNode* w, writes) {
    if(debug) std::cout << "\t\twrite " << getSgVariant(w->variantT());
    if(debug) std::cout << " "          << w->unparseToString();
    if ( isOutermostVariant(w, V_SgPntrArrRefExp)        ||
         isOutermostVariant(w, V_SgArrowExp)             ||
         // isOutermostVariant(w, V_SgFunctionCallExp)      ||
         isOutermostVariant(w, V_SgDotExp)               ||
         isFunctionCall(w) ) {
      dominantWrites.push_back(w);
      if(debug) std::cout << " @";
    }
    if(debug) std::cout << std::endl;
  }

  // Subtract the effect of sub-expressions, as calculated
  // above. The effect of their reads will now be accounted for in the
  // dominant expression.
  foreach(SgNode* dom, dominantReads) {
    normalizeByDominance(dom, reads);
  }

  // Subtract the effect of sub-expressions, as calculated
  // above. The effect of their reads and writes will now be accounted for in
  // the dominant expression.
  foreach(SgNode* dom, dominantWrites) {
    normalizeByDominance(dom, reads);
    normalizeByDominance(dom, writes);
  }
}

/*
 *
 * Inferring memory access signatures.
 *
 * TODO: There is an undocumented assumption here that collectMemoryAccesses
 * returns lists of SgExpressions.
 *
 */

void accessAssociator( bool isBranchPoint
                     , SgStatement *stmt
                     , SgNode *associatee ) {
PROFILE_FUNC
  std::vector< SgNode * > readRefs;
  std::vector< SgNode * > writeRefs;
  bool marked = false;
  bool keep = ! isBranchPoint;

  collectMemoryAccesses(stmt, readRefs, writeRefs);

  /* For each expression that is a write we annotate a copy of the expression,
   * ignoring non-expressions */

  foreach(SgNode *node, writeRefs) {
    SgExpression *expr = isSgExpression(node);

    if(debug) std::cout << "One write: " << getSgVariant(node->variantT()) << ": ";

    if ( expr ) {
      SgExpression* clone = SageInterface::copyExpression(expr);
                                                        ROSE_ASSERT ( clone );
      annotate(associatee, clone, oneWrite);
      marked = true;
      keep = keep || isLoopIndex(expr);

      if(debug) std::cout << "marked"                                     << std::endl;
    } else {
      if(debug) std::cout << "*not*"                                      << std::endl;
    }
  }

  /* For each expression that is a read we annotate a copy of the expression,
   * again ignoring non-expressions */

  foreach(SgNode *node, readRefs) {
    SgExpression *expr = isSgExpression(node);

    if(debug) std::cout << "One read: " << getSgVariant(node->variantT()) << ": ";

    if ( expr ) {
      SgExpression* clone = SageInterface::copyExpression(expr);
                                                        ROSE_ASSERT ( clone );
      annotate(associatee, clone, oneRead);
      marked = true;
      keep = keep || isLoopIndex(expr);

      if(debug) std::cout << "marked"                                     << std::endl;
    } else {
      if(debug) std::cout << "*not*"                                      << std::endl;
    }
  }

  if ( ! marked ) {
    if(debug) std::cout << "No reads nor writes"                          << std::endl;
    annotate(associatee, SageBuilder::buildNullExpression(), noAccess);
  }

  if ( ! keep ) {
    /* Mark this node for replacement with a branch predictor if the condition
     * has no read / write accesses to something that is a loop index.
     */
    if(debug) std::cout << " << to be replaced with branch predictor >>"
                                                                << std::endl;
    markForBranchPredictor(associatee);
  }
}

/* This associates an access with the statement unless:
 *
 * a) We've already visited this statement before
 * b) It's a BasicBlock, DeclarationStatement, CaseOptionStmt,
 *    DefaultOptionStmt or a FunctionDefinition
 *
 * Note: this function is careful to avoid "double counting", so
 * if you change the way things are counted you may have to
 * update here and elsewhere.
 */

void inferAccessInStatement(SgStatement* stmt) {
PROFILE_FUNC
  ROSE_ASSERT( stmt );

  if ( stmt ) {
    if(debug) std::cout << std::endl << std::endl << "Visiting a statement of kind "
              << getSgVariant(stmt->variantT())                 << std::endl;
    // check for types that we bail on -- these are ones that may cause
    // double counting, like basic blocks where we would visit the contents
    // twice - at the block level and at the statement level
    if (  isSgBasicBlock(stmt)
       || isSgDeclarationStatement(stmt)
       || isSgCaseOptionStmt(stmt)
       || isSgDefaultOptionStmt(stmt)
       || isSgFunctionDefinition(stmt)
        ) {
        if(debug) std::cout << "\tcompound but elsewise-covered statement" << std::endl;

        return;
    }

    // make sure we haven't been here before
    if ( stmt->attributeExists(MemoryAccessSignature) ) {
      if(debug) std::cout << "Multivisit :: " << stmt->unparseToString() << std::endl;
      return;
    }

    if(debug) std::cout << "============================================" << std::endl;
    if(debug) std::cout << "==> " << stmt->unparseToString() << std::endl;

    // if-statement: examine just the control portion
    SgIfStmt *ifs = isSgIfStmt(stmt);
    if ( ifs ) {
      if(debug) std::cout << "Handing if statement." << std::endl;

      SgStatement *s_cond = ifs->get_conditional();
      if ( s_cond ) accessAssociator(true, s_cond, stmt);

      if(debug) std::cout << "==========================================" << std::endl;
      return;
    }

    // TODO: can safely ignore For statements, as we're transforming them away.
#if 0
    // for statement: examine just the control portion
    SgForStatement *fs = isSgForStatement(stmt);
    if ( fs ) {
      std::cout << "Handing for loop." << std::endl;
      SgStatement *s_test = fs->get_test();
      //
      // TODO: how to handle s_incr since it is not an SgStatement
      //SgExpression *s_incr = fs->get_increment();
      //
      SgForInitStatement *s_init = fs->get_for_init_stmt();

      if ( s_test ) accessAssociator(s_test, stmt);
      if ( s_init ) accessAssociator(s_init, stmt);

      std::cout << "==========================================" << std::endl;
      return;
    }
#endif

    // while statements here too
    SgWhileStmt *ws = isSgWhileStmt(stmt);
    if ( ws ) {
      //if(debug) std::cout << "Handing while loop." << std::endl;

      //SgStatement *s_cond = ws->get_condition();
      //if ( s_cond ) accessAssociator(true, s_cond, stmt);

      //if(debug) std::cout << "==========================================" << std::endl;
      return;
    }

    // do statements here too
    SgDoWhileStmt *dws = isSgDoWhileStmt(stmt);
    if ( dws ) {
      //if(debug) std::cout << "Handing do loop." << std::endl;

      //SgStatement *s_cond = dws->get_condition();
      //if ( s_cond ) accessAssociator(true, s_cond, stmt);

      //if(debug) std::cout << "==========================================" << std::endl;
      return;
    }

    // switch statements here
    SgSwitchStatement *sws = isSgSwitchStatement(stmt);
    if ( sws ) {
      if(debug) std::cout << "Handing switch statement." << std::endl;

      SgStatement *s_scrutinee = sws->get_item_selector();
      if ( s_scrutinee ) accessAssociator(true, s_scrutinee, stmt);

      if(debug) std::cout << "==========================================" << std::endl;
      return;
    }

    // default
    if(debug) std::cout << "Handling other stmt ("
              << getSgVariant(stmt->variantT()) << ")"          << std::endl;
    accessAssociator(false, stmt, stmt);
    if(debug) std::cout << "============================================" << std::endl;
  }
}

class InferAccessSignatures : public FunctionBodyProcessing {
  public:
    void visitStatement (SgStatement* stmt) {
    PROFILE_FUNC
      StmtSimpleProcessing *map =
              new StmtSimpleProcessing(inferAccessInStatement);
      map->traverse(stmt, postorder);
    }
};

/* If a return stmt is just a null expression or a constant value expression we
 * are done and return NULL.
 *
 * Otherwise, we reduces expressions based on the function's return type: We
 * grab the function's return type and if it is unsigned, int, float, or
 * pointer then we build a return statement that returns 0 for the respective
 * type.
 *
 * If the return type is void we build just a "return;" statement. (which
 * should already be the case, right?)
 *
 * In all other cases we return NULL signifying that there was no simplication.
 *
 * Note: This returns NULL both when there is no work to be done and when it
 * can't figure out what work to do.
 */

SgReturnStmt*
simplifyReturn(SgReturnStmt* stmt) {
PROFILE_FUNC
  if ( stmt ) {
    SgExpression* expr         = stmt->get_expression();
    if ( ! expr || isSgNullExpression(expr) || isConstantValueExp(expr) ) {
      return NULL;
    }
    SgFunctionDeclaration* func =
        SageInterface::getEnclosingFunctionDeclaration(stmt);
    if ( func ) {
      SgType* ret    = func->get_type()->get_return_type();
      SgType* ret_ty = ret ? ret->stripTypedefsAndModifiers() : NULL;
      if ( ret_ty ) {
        if      ( ret_ty->isUnsignedType() ) {
          SgUnsignedIntVal* zero = SageBuilder::buildUnsignedIntVal();
          return SageBuilder::buildReturnStmt(zero);
        }
        else if ( ret_ty->isIntegerType() ) {
          SgIntVal* zero = SageBuilder::buildIntVal();
          return SageBuilder::buildReturnStmt(zero);
        }
        else if ( ret_ty->isFloatType() ) {
          SgFloatVal* zero = SageBuilder::buildFloatVal();
          return SageBuilder::buildReturnStmt(zero);
        }
        else if ( SageInterface::isPointerType(ret_ty) ) {
          SgNullExpression* null = SageBuilder::buildNullExpression();
          return SageBuilder::buildReturnStmt(null);
        }
        else if ( isSgTypeVoid(ret_ty) ) {
          return SageBuilder::buildReturnStmt();
        }
      }
    }
  }
  return NULL;
}


typedef enum { PlaceBefore        // Insert stmt before current statement
             , PlaceAfter         // Insert stmt before current statement
             , PrependInSameScope // Prepend stmt at end of current scope
             , AppendInSameScope  // Append stmt at end of current scope
             }
Placement;

/* Replace the conditional check of branches with a function call to either
 * coinflip (for if-stmts and loops) or a call to choose for switch statements.
 * We copy over the original body of the branches.
 *
 * Note: This function is only intended to be called for branches (if, switch,
 * dowhile, while, for-loops have already been removed)
 */

SgStatement*
replaceWithBranchPredictor(SgStatement* host) {
PROFILE_FUNC
  ROSE_ASSERT( host );

  // SgScopeStatement* cur_scope = SageInterface::getScope(host);
  // SageBuilder::pushScopeStack(cur_scope);

  SgStatement* new_branch;

  switch ( host->variantT() ) {
    case V_SgSwitchStatement: {
      /*
       * let switcher = buildSwitchPredictor $ isSgSwitchStatement(host)
       * f_switcher <- addDeclToTopLevel switcher
       * replaceStatement branch $ call f_switcher
       */
      SgSwitchStatement* sws = isSgSwitchStatement(host);   ROSE_ASSERT( sws );

      /*
       * Doing a full copy of the switch's body here, and then updating that
       * copy in-place. It's not clear how to safely build a switch body
       * from a list of statements (zero or more case options, zero or one
       * default).
       */
      SgStatement* body = SageInterface::copyStatement(sws->get_body());
                                                            ROSE_ASSERT( body );
      SgStatementPtrList& branches = isSgBasicBlock(body)->get_statements();

      SgExprStatement* choose =
        SageBuilder::buildExprStatement(
          SageBuilder::buildFunctionCallExp
            ( "choose"
            , SageBuilder::buildIntType()
            , SageBuilder::buildExprListExp
                (SageBuilder::buildIntVal(branches.size()))
            ));

      int case_index = 0;
      foreach(SgStatement* stmt, branches) {
        SgCaseOptionStmt* case_action       = isSgCaseOptionStmt(stmt);
        SgDefaultOptionStmt* default_action = isSgDefaultOptionStmt(stmt);
        if ( case_action ) {
          SgExpression* key   = SageBuilder::buildIntVal(case_index);
          SgStatement* action =
                SageInterface::copyStatement(case_action->get_body());
          // This is update in-place, but on a copy of the original switch's
          // body (or rather, one of its cases).
          SageInterface::replaceStatement
                ( case_action
                , SageBuilder::buildCaseOptionStmt(key, action)
                );
        } else if ( default_action ) {
          // no change needed
        }
        case_index++;
      }

      new_branch = SageBuilder::buildSwitchStatement(choose, body);
      break;
    }

    case V_SgIfStmt: {
      SgExprStatement* coinflip =
        SageBuilder::buildExprStatement(
          SageBuilder::buildFunctionCallExp( "coinflip"
                                           , SageBuilder::buildIntType()
                                           , SageBuilder::buildExprListExp()
                                           ));
      SgIfStmt* ifs = isSgIfStmt(host);                     ROSE_ASSERT( ifs );
      SgStatement* then_body =
              SageInterface::copyStatement(ifs->get_true_body());
      SgStatement* else_body =
              SageInterface::copyStatement(ifs->get_false_body());
      new_branch = SageBuilder::buildIfStmt(coinflip,then_body,else_body);
      break;
    }

    //case V_SgWhileStmt: {
    //  SgExprStatement* coinflip =
    //    SageBuilder::buildExprStatement(
    //      SageBuilder::buildFunctionCallExp( "coinflip"
    //                                       , SageBuilder::buildIntType()
    //                                       , SageBuilder::buildExprListExp()
    //                                       ));
    //  SgWhileStmt* ws = isSgWhileStmt(host);                ROSE_ASSERT( ws );
    //  SgStatement* body = SageInterface::copyStatement(ws->get_body());
    //  new_branch = SageBuilder::buildWhileStmt(coinflip,body);
    //  break;
    //}

    //case V_SgDoWhileStmt: {
    //  SgExprStatement* coinflip =
    //    SageBuilder::buildExprStatement(
    //      SageBuilder::buildFunctionCallExp( "coinflip"
    //                                       , SageBuilder::buildIntType()
    //                                       , SageBuilder::buildExprListExp()
    //                                       ));
    //  SgDoWhileStmt* dws = isSgDoWhileStmt(host);           ROSE_ASSERT( dws );
    //  SgStatement* body = SageInterface::copyStatement(dws->get_body());
    //  new_branch = SageBuilder::buildDoWhileStmt(coinflip,body);
    //  break;
    //}

    default: {
      if(debug) std::cout << "replaceWithBranchPredictor: not a (supported) branch: "
                          << getSgVariant(host->variantT())               << std::endl;
      new_branch = NULL;
      break;
    }
  }

  // SageBuilder::popScopeStack();

  return new_branch;
}

/*
 *
 * Transform source according to memory access signatures.
 *
 */
void replaceWithAbstractReadWrites(SgStatement* stmt) {
PROFILE_FUNC
  Placement placement = PlaceBefore;

  /* If stmt is null, just bail early */
  if ( stmt == NULL ) return;

  if(debug) std::cout << std::endl << std::endl;
  if(debug) std::cout << "============================================"
                                                              << std::endl;
  if(debug) std::cout << "Examining statement:"                         << std::endl
            << stmt->unparseToString()                        << std::endl;

  if ( isSgBasicBlock(stmt) ) {
      if(debug) std::cout << "<basic block; skipping>"                  << std::endl;
      return;
  }

  if ( stmt->attributeExists(GeneratedAccess) ) {
      if(debug) std::cout << "<generated code; skipping>"               << std::endl;
      return;
  }

  if ( stmt->attributeExists(VisitedForReplacement) ) {
      if(debug) std::cout << "<already visited; skipping>"              << std::endl;
      return;
  }
  markAsVisitedForReplacement(stmt);

  if ( ! stmt->attributeExists(MemoryAccessSignature) ) {
    if(debug) std::cout << "Not marked with access attributes, so should be ignored ("
              << getSgVariant(stmt->variantT()) << ")"        << std::endl;
    /* Nothing to do, so let's got out of here */
    return;
  }

  if ( getPresRemUnd(stmt) == pragma_preserve ) {
    if(debug) std::cout << "Marked as preserve, so should be ignored" << std::endl;
    return;
  }


  bool keep = false;          // should we keep this statement?
  bool supressSubExpr = false; // Should we suppress subexpressions?
  AstAttribute* attr   = stmt->getAttribute(MemoryAccessSignature);
  AccessSignature* sig = isAccessSignature(attr);

  if(debug) std::cout << "Has access attributes"         << std::endl << std::endl;

  // push current scope
  // TODO: Why do we push the scope?
  SgScopeStatement* cur_scope = SageInterface::getScope(stmt);
  if (  !isSgForStatement(stmt)
     && !isSgWhileStmt(stmt)
     && !isSgDoWhileStmt(stmt)
     && !isSgIfStmt(stmt)
     && !isSgSwitchStatement(stmt)
     && !isSgCaseOptionStmt(stmt)
     && !isSgDefaultOptionStmt(stmt)
     && !isSgBasicBlock(cur_scope)
      ) {
    if(debug) std::cout << "<not a loop, branch, or part of a basic block; skipping>"
                        << std::endl;
    return;
  }

  // push reads/writes for options into the basic block of the option body
  if ( isSgCaseOptionStmt(stmt) ) {
    SgCaseOptionStmt *cos = isSgCaseOptionStmt(stmt);
    SgStatement *body = cos->get_body();
    cur_scope = SageInterface::getScope(body);
    placement = PrependInSameScope;
    keep = true;
  }

  // push reads/writes for options into the basic block of the option body
  if ( isSgDefaultOptionStmt(stmt) ) {
    SgDefaultOptionStmt *dos = isSgDefaultOptionStmt(stmt);
    SgStatement *body = dos->get_body();
    cur_scope = SageInterface::getScope(body);
    placement = PrependInSameScope;
    keep = true;
  }

#if 0
  // push reads/writes for for loop into the basic block of the loop body
  if ( isSgForStatement(stmt) ) {
    SgForStatement *fs = isSgForStatement(stmt);
    // make sure the reads/writes for the loop get in the body by getting
    // the body and using its scope as the one to push onto the scope
    // stack.
    SgStatement *body = fs->get_loop_body();
    cur_scope = SageInterface::getScope(body);
    placement = AppendInSameScope;
    keep = true;
  }
#endif

#if 0
  if ( isSgIfStmt(stmt) ) {
    placement = PlaceBefore;
    keep = true;
  }

  if ( isSgSwitchStatement(stmt) ) {
    placement = PlaceBefore;
    keep = true;
  }

  if ( isSgWhileStmt(stmt) ) {
    placement = PlaceBefore;
    keep = true;
  }

  if ( isSgDoWhileStmt(stmt) ) {
    placement = PlaceBefore;
    keep = true;
  }
#endif

  // push scope onto stack
  if(debug) std::cout << "SCOPE::" << cur_scope->unparseToString() << std::endl;
  SageBuilder::pushScopeStack(cur_scope);

  std::vector<SgStatement*> new_stmts;

  foreach(AccessMap::value_type rw_map, sig->signature) {
    SgExpression* expr  = rw_map.first;
    MemoryAccess access = rw_map.second;
    int rs = rw_map.second.num_reads;
    int ws = rw_map.second.num_writes;

    if(debug) std::cout << getSgVariant(expr->variantT()) << " @ " << expr->unparseToString()
                        << ": (" << rs << ", " << ws << ")"
                        << std::endl;

    if ( isFunctionCall(expr) /* || partOfFunctionHead(expr) */ ) {
      /* TODO: If this function call is part of a larger statement then
       * we want to extract just the function call(s) and remove the rest of the
       * statement.
       *
       * For example, in the statement: A[i] = foo(A[i-1]);
       * We want to discard the assignment to A[i] and leave behind:
       * writer(A[i]);
       * foo(A[i-1]);
       *
       * Note: We don't need reader(A[i-1]) or reader(i) statements
       * because passing A[i-1] to foo() will force those reads.
       */
      if(debug) std::cout << "Function call or head; keeping"         << std::endl;
      keep = true;
      /* Becaues of the function call expr, we don't want to insert the read/write
       * statements later in this function body */
      supressSubExpr = true;
    }

    /*
     *
     * Loop indices are another special case. We want to mark them with
     * accesses, so we don't accidentally delete the enclosing loop.
     * However, we don't want to replace r/w accesses for them with
     * abstract reader / writer calls in all cases. Only when we have a
     * read(s) with no writes might the statement have no effect on the
     * loop (i.e., when the loop index is being read but not changed).
     *
     */
    if ( isLoopIndex(expr) ) {
      keep = keep || ws > 0;
      if(debug) std::cout << "Loop index; " << (keep ? "keeping" : "discarding")
                                                            << std::endl;
      continue;
    }

    if ( rs > 0 || ws > 0 ) { // i.e., some access, read or write
      if(debug) std::cout << "About to add calls to reader:";

      for ( int i = 0 ; i < rs ; i++ ) {
        SgType* void_t    = SageBuilder::buildVoidType ();
        SgExpression* arg = SageInterface::copyExpression(expr);
        ROSE_ASSERT( arg );
        SgExprListExp* params = SageBuilder::buildExprListExp(arg);

        SgExprStatement* read_call =
          SageBuilder::buildExprStatement(
            SageBuilder::buildFunctionCallExp
                  ( "reader"
                  , void_t
                  , params
                  ));

        markAsGenerated(read_call);
        new_stmts.push_back(read_call);
        if(debug) std::cout << " r";
      }
      if(debug) std::cout << std::endl;

      if(debug) std::cout << "About to add calls to writer:";
      for ( int i = 0 ; i < ws ; i++ ) {
        SgType* void_t  = SageBuilder::buildVoidType ();
        SgExpression* arg = SageInterface::copyExpression(expr);
        ROSE_ASSERT( arg );
        SgExprListExp* params = SageBuilder::buildExprListExp(arg);

        SgExprStatement* write_call =
          SageBuilder::buildExprStatement(
            SageBuilder::buildFunctionCallExp
                  ( "writer"
                  , void_t
                  , params
                  ));

        markAsGenerated(write_call);
        new_stmts.push_back(write_call);
        if(debug) std::cout << " w";
        if(debug) std::cout << " writer(" << expr->unparseToString() << "); ";
      }
      if(debug) std::cout << std::endl;

    } else { // access == noAccess
      if(debug) std::cout << "No accesses: will remove statement"     << std::endl;
    }
  }

  /*
   * Replace returns that have memory accesses with zero access
   * alternatives.
   */
  {
    SgReturnStmt* r_stmt = isSgReturnStmt(stmt);
    if ( r_stmt && ! keep ) {
      if(debug) std::cout << "  Return simplification candidate"        << std::endl;
      SgReturnStmt* new_r_stmt = simplifyReturn(r_stmt);
      if ( new_r_stmt ) {
        if(debug) std::cout << "   simplified"                          << std::endl;
        new_stmts.push_back(new_r_stmt);
      } else {
        if(debug) std::cout << "   left alone"                          << std::endl;
        keep = true;
      }
    }
  }
  if ( isBranch(stmt) ) {
    if ( stmt->attributeExists(ReplaceWithBranchPredictor) ) {
      ROSE_ASSERT ( isBranch(stmt) );
      SgStatement* new_branch = replaceWithBranchPredictor(stmt);
      if(debug) std::cout << "  Branch condition to be replaced"      << std::endl;
      if ( new_branch ) {
        if(debug) std::cout << "  replaced"                           << std::endl;
        new_stmts.push_back(new_branch);
        keep = false;
      } else {
        if(debug) std::cout << "  left alone"                         << std::endl;
        keep = true;
      }
    } else {
      if(debug) std::cout << "  Branch to be left intact"             << std::endl;
      keep = true;
    }
  }

  // First check to see if we should supress the reader/writer for subexpressions.
  // This would be the case of we are outputting the whole function call expression
  // and using a backend like Intel's PIN.
  if( !supressSubExpr ) {
    // Add new code.  Four possible scenarios:
    //   1. Insert before current statement (e.g., just a regular
    //      line of code).
    //   2. Insert after current statement (e.g., just a regular
    //      line of code).
    //   3. Prepend to current scope
    //   4. Append to current scope
    //
    // A flag is set up above to let us know which to do here.
    foreach(SgStatement* new_stmt, new_stmts) {
      if ( new_stmt ) {
        if(debug) std::cout << "Inserting "
                  << getSgVariant(new_stmt->variantT()) << " @ "
                  << stmt->get_file_info()->get_line()        << std::endl;
        switch ( placement ) {
        case PlaceBefore:
          SageInterface::insertStatementBefore(stmt, new_stmt);
          if(debug) std::cout << "Inserted statements before current"   << std::endl;
          break;

        case PlaceAfter:
          SageInterface::insertStatementAfter(stmt, new_stmt);
          if(debug) std::cout << "Inserted statements after current"    << std::endl;
          break;

        case PrependInSameScope:
          SageInterface::prependStatement(new_stmt);
          if(debug) std::cout << "Inserted statements at beginning of current scope"
                                                              << std::endl;
          break;

        case AppendInSameScope:
          SageInterface::appendStatement(new_stmt);
          if(debug) std::cout << "Inserted statements at end of current scope"
                                                              << std::endl;
          break;
        }
      }
    }
  }

  /*
   * If statement is in essence an expression wrapper (SgExprStatement,
   * SgReturnStmt, SgAssertStmt, SgComputedGotoStatement, perhaps more),
   * then rewrite it to the "empty" variant of that statement (i.e., either
   * SgNullExpression or IntValueExp 0).
   */
  if ( ! keep && ! isCrucial(stmt) ) { // || isEmptyCompoundStmt(stmt) ) {
    if(debug) std::cout << "Putting statement ("
              << getSgVariant(stmt->variantT())
              << ") in the trash"                           << std::endl;
    // trash.insert(stmt);
    SageInterface::removeStatement(stmt);
  }

  SageBuilder::popScopeStack();
  
}

/*
 *
 * Useful minimal application of AstSimpleProcessing: produce the flattened
 * list of statements corresponding to traversal performed. Intended usage:
 *
 * > GatherStatements* gather = new GatherStatements();
 * > gather->traverseInputFiles(project, postorder);
 * > std::vector<SgStatement*> stmts_in_postorder = gather->get_statements();
 *
 * This should also work for preorder traversals.
 *
 * The point of this is to allow a function that mutates statements to be
 * applied in an accumlative fashion, postorder. This ensures that when
 * examining a compound statement (and especially when transforming it),
 * the subject's children will have been processed (and updated in place).
 * This has the effect of chaining the transformations in the correct order.
 *
 */
class GatherStatements : public AstSimpleProcessing {
  public:
    void visit (SgNode* node) {
    PROFILE_FUNC
      if ( node ) {
        SgStatement* stmt = isSgStatement(node);
        if ( stmt ) {
          p_statements.push_back(stmt);
        }
      }
    }

    std::vector<SgStatement*> get_statements () {
      return p_statements;
    }

  private:
    std::vector<SgStatement*> p_statements;
};

/* This create a new assignment that can be used to replace an initializer.
 *
 * example:
 *
 * int i = 0;
 *
 * turns into:
 *
 * int i;
 * i = 0;
 *
 */
SgStatement*
buildInitializerAsAssign(SgInitializedName* var, SgInitializer* init) {
PROFILE_FUNC
  ROSE_ASSERT( var );

  SgAssignInitializer* assign = isSgAssignInitializer(init);
  if ( assign ) {
    SgScopeStatement* scope = SageInterface::getScope(var);
    SgExpression* init_val  = assign->get_operand_i();
    SgExpression* lhs       =
            SageBuilder::buildVarRefExp(var->get_name(), scope);
    return SageBuilder::buildExprStatement
                    (SageBuilder::buildAssignOp(lhs, init_val));
  }

  return NULL;
}

/* This breaks all declaration initializers into simpler statements so that
 * reads/writes are easier to analyze.
 *
 * See the comments below for details about the transformations applied.
 */

void
normalizeAllDeclInitializers(SgProject* project) {
PROFILE_FUNC
  // get the set of nodes of type SgVariableDeclaration
  NodeQuerySynthesizedAttributeType decls =
      NodeQuery::querySubTree(project, V_SgVariableDeclaration);

  foreach (SgNode *node, decls) {
    std::vector< SgStatement* > assign_inits;

    // get the SgVariableDeclaration out, with an assert
    // sanity check
    SgVariableDeclaration* decl = isSgVariableDeclaration(node);
    ROSE_ASSERT ( decl );

    // if this declaration occurs in the global scope, then
    // we want to skip it since it makes no sense to drop
    // statements at the top level in for initializing the
    // variable.
    SgNode *parent = decl->get_parent();
    if (isSgGlobal(parent)) {
      continue;
    }

    // get the set of variables associated with the declaration
    SgInitializedNamePtrList& vars = decl->get_variables();

    //
    // for each variable, we want to build an initializer that
    // resides in a single statement, and then will NULL out
    // the original initializer that occurred at declaration time.
    // 
    // example:
    //
    // int i = 0;
    //
    // turns into:
    //
    // int i;
    // i = 0;
    //
    foreach (SgInitializedName* var, vars) {
      if ( var ) {
        SgStatement* init =
                buildInitializerAsAssign(var, var->get_initializer());
        if ( init ) {
          // add it to replacer list
          assign_inits.push_back(init);
          // FIXME: zero out decl initializer
          // FIXME: set_initializer seems like an unsafe means to do this, as
          // it's not clear that this preserves invariants regarding AST
          // connectedness, through pointers like p_parent()
          var->set_initializer(NULL);
        }
      }
    }

    SageInterface::insertStatementListAfter(decl,assign_inits);
  }
}


/*
 *
 * Extracts the loop index's symbol and inserts it into the global list of
 * indices.
 *
 * Presumes SageInterface::normalizeForLoopInitDeclaration has been called
 * on this statement. This is induced by the use of
 * SageInterface::getLoopIndexVariable, which fails if the initializer
 * statement is a declaration.
 */
void markForLoopIndex(SgForStatement* loop) {
PROFILE_FUNC
  ROSE_ASSERT( loop );
  // Unpack the loop
  SgForInitStatement* init  = loop->get_for_init_stmt(); ROSE_ASSERT( init );
  SgStatementPtrList& inits = init->get_init_stmt ();

  if ( inits.size() == 1 ) {
    if(debug) std::cout << "markForLoopIndex: common case"                << std::endl;
    // Overwhelmingly common case: a single initializer statement.
    SgInitializedName* index = SageInterface::getLoopIndexVariable(loop);
    if ( index ) {
      SgSymbol* sym = index->search_for_symbol_from_symbol_table();
      if ( sym ) {
        if(debug) std::cout << "  -> Loop index identified: "
                  << sym->get_name().str()                      << std::endl;
        indices.insert(sym);
      }
    }
  } else {
    // Rare case: either no initializer statements, or multiple statements.
    // Here, we need to infer the loop index: anything written to inside inc.
    // This is an over-approximation, but hopefully this case occurs seldom
    // enough that we don't notice the rare false positives.
    SgExpression* inc = loop->get_increment();          ROSE_ASSERT( inc );

    if(debug) std::cout << "markForLoopIndex: rare case"                  << std::endl;
    // Add code to insert any vars written to by inc; spec.:
    //
    //       [ indices.insert(sym) | sym <- symbol(var)
    //                             , var <- filter isVarRef inc
    //                             , isUsedLValue(var)
    //                             ]
    NodeQuerySynthesizedAttributeType exps =
        NodeQuery::querySubTree(inc, V_SgExpression);
    foreach (SgNode *node, exps) {
      SgExpression* exp = isSgExpression(node);
      if ( exp ) {
        SgSymbol* sym = getSymbolFromRefExp(exp);
        if ( sym && exp->isUsedAsLValue() ) {
          if(debug) std::cout << "  -> Loop index identified: "
                    << sym->get_name().str()                    << std::endl;
          indices.insert(sym);
        }
      }
    }
  }
}

/* This function looks for variables that are read in the loop conditinal and
 * written to in the loop body. We assume those variables are loop indexes and
 * make note of them for later.
 *
 * The steps are:
 *
 * 1) Gather all the reads and writes for the loop conditional (cond) and the
 * loop body.
 *
 * 2) For each body expression that writes to a variable we store that variable
 * for our later tests.
 *
 * 3) For each var read in the cond we check if the var was written to in the
 * body.
 *
 * 4) When the above checks are satisfied we treat the var in question as an
 * index.
 */

void
markLoopIndex(SgStatement* cond, SgStatement* body) {
PROFILE_FUNC
  std::vector < SgNode* > cond_reads;
  std::vector < SgNode* > cond_writes;
  std::vector < SgNode* > body_reads;
  std::vector < SgNode* > body_writes;

  bool result;
  result = collectReadsWrites(cond, cond_reads, cond_writes);
  result = collectReadsWrites(body, body_reads, body_writes);

  /*

   [ i | rs <- collectReads(loop_cond)
       , ws <- collectWrites(loop_body)
       , r <- rs
       , r `elem` ws
       , i <- isVarRef r
       ]

   */

  std::set<SgSymbol*> body_vars;
  foreach(SgNode* node, body_writes) {
    SgExpression* exp = isSgExpression(node);
    if ( isVarRef(exp) ) {
      SgSymbol* sym = getSymbolFromRefExp(exp);
      if ( sym ) {
        body_vars.insert(sym);
      }
    }
  }

  foreach(SgNode* node, cond_reads) {
    SgExpression* exp = isSgExpression(node);
    if ( isVarRef(exp) ) {
      SgSymbol* sym = getSymbolFromRefExp(exp);
      if ( sym && body_vars.find(sym) != body_vars.end() ) {
        if(debug) std::cout << "  -> Loop index identified: "
                  << sym->get_name().str()                    << std::endl;
        indices.insert(sym);
      }
    }
  }
}

/* Processes all loops calling the respective indexing marking functions.
 */
void
markAllLoopIndices(SgProject* project) {
PROFILE_FUNC
  // FOR loops
  NodeQuerySynthesizedAttributeType fors =
      NodeQuery::querySubTree(project, V_SgForStatement);
  foreach (SgNode *node, fors) {
    SgForStatement* loop = isSgForStatement(node);      ROSE_ASSERT ( loop );
    SageInterface::normalizeForLoopInitDeclaration(loop);
    markForLoopIndex(loop);
  }

  // WHILE loops
  NodeQuerySynthesizedAttributeType whiles =
      NodeQuery::querySubTree(project, V_SgWhileStmt);
  foreach (SgNode *node, whiles) {
    SgWhileStmt* loop = isSgWhileStmt(node);            ROSE_ASSERT ( loop );
    markLoopIndex(loop->get_condition(), loop->get_body());
  }

  // DO-WHILE loops
  NodeQuerySynthesizedAttributeType do_whiles =
      NodeQuery::querySubTree(project, V_SgDoWhileStmt);
  foreach (SgNode *node, do_whiles) {
    SgDoWhileStmt* loop = isSgDoWhileStmt(node);        ROSE_ASSERT ( loop );
    markLoopIndex(loop->get_condition(), loop->get_body());
  }
}

/* This simply runs infer access signatures at all nodes in the AST
 */

void
inferAccessSignatures( SgProject* project ) {
PROFILE_FUNC
  InferAccessSignatures *infer = new InferAccessSignatures();
  infer->traverseInputFiles(project, postorder);
}

/* Applies gather statements at each AST node followed by a pass that does the
 * skeletonization of accesses.
 */

void
skeletonizeReadWrites( SgProject* project ) {
PROFILE_FUNC
  GatherStatements* gather = new GatherStatements();
  gather->traverseInputFiles(project, postorder);
  std::vector<SgStatement*> stmts = gather->get_statements();

  foreach(SgStatement* stmt, stmts) {
    replaceWithAbstractReadWrites(stmt);
  }
}


//
// main program
//
int main(int argc, char **argv) {
    bool genPDF  = false;  // if true, generate a PDF dump of the entire
                           // AST of the input program

#ifdef PROFILING_ENABLED
    TAU_PROFILE("int main(int, char **)", " ", TAU_DEFAULT);
    TAU_INIT(&argc, &argv);
#endif /* PROFILING_ENABLED */

    // Local Command Line Processing:
    Rose_STL_Container<std::string> l =
      CommandlineProcessing::generateArgListFromArgcArgv(argc, argv);
    if ( CommandlineProcessing::isOption(l,"-skel:","(d|debug)",true) ) {
        std::cout << "Debugging on" << std::endl;
        debug = true;
    }
    if ( CommandlineProcessing::isOption(l,"-skel:","(p|pdf)",true) ) {
        std::cout << "PDF to be generated" << std::endl;
        genPDF = true;
    }

    // set up a rose project
    SgProject* project = frontend(l);
    ROSE_ASSERT( project );

    /*
    protectedCalls["malloc"] = 1; // arity
    protectedCalls["free"] = 1;
    */

    // std::cout << "Running consistency checks on initial AST" << std::endl;
    // AstTests::runAllTests(project);

    // Set all parent/child relationships in project
    // ClassHierarchyWrapper chw(project);

    // std::cout << "Running consistency check on CHW'd AST" << std::endl;
    // AstTests::runAllTests(project);

    // STEP 1:
    // Run the SSA analysis
    if(debug) std::cout << "Running SSA analysis"               << std::endl;
    StaticSingleAssignment ssa(project);
    ssa.run(true, true);

    if (debug) std::cout << "Take note of all FOR-loop indices"            << std::endl;
    markAllLoopIndices(project);

    // Perform this transformation globally:
    //
    //    type var_1, .., var_n = expr;
    //
    //    ===>
    //
    //    type var_1;
    //    ...
    //    type var_n;
    //    var_n = expr;
    //
    // FIXME: what about the following:
    //
    //    type var_1, .., var_i = expr, .., var_n = expr;?
    // TODO: Why do we do this??
    normalizeAllDeclInitializers(project);

    // Simplify handling of FOR statements by converting them to WHILEs
    // TODO: Why?
    SageInterface::convertAllForsToWhiles(project);

    // Ensure all compound statements' bodies are SgBlockStatements, including
    // empty branches.
    // TODO: Why?
    SageInterface::changeAllBodiesToBlocks(project);

    std::cout << "Running consistency check on blockified AST" << std::endl;
    AstTests::runAllTests(project);

    // STEP 2:
    // Infer memory signatures
    if(debug) std::cout << "Inferring memory access signatures" << std::endl;
    inferAccessSignatures(project);
    if(debug) std::cout << "Traversal 1 done." << std::endl;

    // Annotate Pragmas: add attributes to AST so that skeletonizing plays
    // well with pragmas; must be done before skeletonization.
    if(debug) std::cout << "PreProcessing Pragmas"              << std::endl;
    annotatePragmas(project);

    // Process Pragmas
    if(debug) std::cout << "Processing Pragmas"                 << std::endl;
    processPragmas(project, false, NULL);

    if(debug) std::cout << "Running consistency check on annotated AST"   << std::endl;
    AstTests::runAllTests(project);

    if(debug) std::cout << "Rewriting annotated statements" << std::endl;
    skeletonizeReadWrites(project);
    if(debug) std::cout << "Traversal 2 done." << std::endl;

    // std::cout << "Dumping DOT for transformed AST"              << std::endl;
    // generateDOT(*project);

    /*
     * FIXME: in general, AstTests::runAllTests(project) fails. It seems to
     * be related to C++ features, and it seems like new statements are built
     * without a legitimate scope.
     */
    // std::cout << "Running consistency check on transformed AST" << std::endl;
    // AstTests::runAllTests(project);

    // Generating modified code:
    if(debug) std::cout << "Generating modified code"           << std::endl;
    project->skipfinalCompileStep(true);

    return backend(project);
}

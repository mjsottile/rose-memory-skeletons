
#include "MemoryAccessSignatures.h"

/*
 * Standard test and downcast pattern matching.
 */
AccessSignature* isAccessSignature (AstAttribute* a) {
  return dynamic_cast<AccessSignature *>(a);
}

/*
 * Given an SgNode, update its memory access signature attribute
 * such the given expression `expr` and is noted to have memory access `access`.
 * If the node has no such attribute, then this will create one and attach it
 * to the node. Otherwise, it updates the existing attribute.
 *
 * Requires: non-NULL SgExpression
 *
 */
void
annotate (SgNode* node, SgExpression* expr, MemoryAccess access) {
  AccessSignature* sig;
  ROSE_ASSERT( node );
  ROSE_ASSERT( expr );

  if ( node->attributeExists(MemoryAccessSignature) ) {
    AstAttribute* attr = node->getAttribute(MemoryAccessSignature);

    sig = isAccessSignature(attr);

    ROSE_ASSERT(attr); // Since attributeExists, attr should be non-NULL
    ROSE_ASSERT(sig);  // Since attr is non-NULL, sig should also be non-NULL

    sig->mark(expr, access);
  } else {
    sig = new AccessSignature(expr, access);
  }

  node->setAttribute(MemoryAccessSignature, sig);
}


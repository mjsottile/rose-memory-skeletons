
#ifndef __MemoryAccessSignatures_H_LOADED__
#define __MemoryAccessSignatures_H_LOADED__

#include <rose.h>

typedef enum { MEM_SKEL_IGNORE
             , MEM_SKEL_REWRITE
             , MEM_SKEL_DELETE
} Disposition;

typedef struct MemoryAccess_t {
  int  num_reads;          /* how many times the entity was read from */
  int  num_writes;         /* how many times the entity was written to */
  Disposition disposition; /* note to the final transform re how entity
                            * ought to be treated. */
} MemoryAccess;

typedef std::map<SgExpression*, MemoryAccess> AccessMap;

typedef std::pair<std::string, size_t> ProtectedFunction;
typedef std::map<std::string, size_t> ProtectedFunctions;

const MemoryAccess oneRead  = { 1, 0, MEM_SKEL_IGNORE };
const MemoryAccess oneWrite = { 0, 1, MEM_SKEL_IGNORE };
const MemoryAccess noAccess = { 0, 0, MEM_SKEL_IGNORE };

const std::string MemoryAccessSignature("Memory Access Signature");
const std::string GeneratedAccess("Generated Abstract Memory Access");
const std::string VisitedForReplacement("Visited for Replacement");
const std::string ReplaceWithBranchPredictor("Replace with a branch predictor");


/*
 * An access signature object is derived from a generic ROSE AstAttribute
 * and contains a map from symbols to memory access signatures (read/write
 * counts).
 */
class AccessSignature : public AstAttribute {
  public:
    AccessMap signature;

    // Constructor for creating an initial signature for a single symbol
    AccessSignature (SgExpression* varexpr, MemoryAccess access) {
      signature.insert(std::make_pair(varexpr, access));
    }

    // Given a symbol and a memory access signature, mark its entry in
    // the map with the signature.
    void mark (SgExpression* expr, MemoryAccess access) {
      AccessMap::iterator exists;
      std::pair<AccessMap::iterator, bool> quo;

      exists = signature.find(expr);
      if ( exists == signature.end() ) {
        // No entry for this expression (likely based on pointer equality)
        quo = signature.insert(std::make_pair(expr, access));
        if ( ! quo.second ) {
          // Can this happen?
          quo.first->second.num_reads  += access.num_reads;
          quo.first->second.num_writes += access.num_writes;
        }
      } else {
        // Already an entry for this expr, so update it
        exists->second.num_reads  += access.num_reads;
        exists->second.num_writes += access.num_writes;
      }
    }
};

/*
 * Standard test and downcast pattern matching.
 */
AccessSignature* isAccessSignature (AstAttribute* a);

/*
 * Given a reference to an `SgNode`, update its memory access signature
 * attribute such the given symbol `sym` and is noted to have memory access
 * `access`.
 *
 * If the node has no such attribute, then this will create one and attach it
 * to the node. Otherwise, it updates the existing attribute, adding the new
 * memory access counts..
 *
 * Requires: non-NULL SgVariableSymbol.
 *
 */
void
annotate (SgNode* node, SgExpression* expr, MemoryAccess access);

#endif /* __MemoryAccessSignatures_H_LOADED__ */


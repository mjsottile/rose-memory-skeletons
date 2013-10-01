#include <string>
#include <iostream>
#include "rose.h"
#include "processPragmas.h"

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;

// Types, Globals //////////////////////////////////////////////////////////////

static bool debugpp = true;

// Utilities ///////////////////////////////////////////////////////////////////

void exitWithMsg (SgStatement *stmt, const char *pragma_arg, const char *msg) {
  fprintf (stderr,
           "ERROR: In pragma '#pragma skel %s':\n  %s\n",
           pragma_arg,
           msg);

  Sg_File_Info &fileInfo = *(stmt->get_file_info());
  fprintf (stderr, "  (File %s, line %d, column %d)\n",
           fileInfo.get_filename(),
           fileInfo.get_line(),
           fileInfo.get_col());
  exit (1);
}

bool supportedFileType () {
  return ( SageInterface::is_C_language()
        || SageInterface::is_Cxx_language()
        || SageInterface::is_mixed_C_and_Cxx_language()
        );
}

// Library Functions ///////////////////////////////////////////////////////////

int match (const char *input, const char *prefix) {
  int i = 0;

  // create sscanf format string in 's':
  static const char fmt[] = "%n";
  char s[strlen(prefix)+strlen(fmt)];

  strcpy(s,prefix);
  strcat(s,fmt);

  sscanf(input, s, &i);
  return i;
}


// pragma parse & process //////////////////////////////////////////////////////

void process1pragma( SgPragmaDeclaration *p
                   , bool outline
                   , SgSymbols *pivots
                   ) {

  //static const char parseErrorMsg[] = "error in parsing pragma expression";

  string pragmaText = p->get_pragma()->get_pragma();
  SgStatement *stmt = SageInterface::getNextStatement(p);

  const char *s = pragmaText.c_str();
  if (debugpp) printf("visit: %s\n", s);

  // Move past 'skel' in pragma:
  {
    int i = 0;
    sscanf (s, "skel %n", &i);
    if (i == 0)
      return;  // i.e., sscanf made no progress, ignore pragma.
    s += i;    // point past "skel"
  }

  if (debugpp)
    cout << "#pragma skel:\n"
         << "  parameter:\n"
         << "   " << s << endl
         << "  target:\n"
         << "    " << stmt->unparseToString()
         << endl;

  if (match(s, "memory") != 0) {
    SgDeclarationStatement *decl = isSgDeclarationStatement(stmt);

    if ( decl ) {
      SgSymbol *sym = decl->search_for_symbol_from_symbol_table();
      assert(sym); // Every declaration has an associated symbol

      if ( pivots ) {
          if ( debugpp )
            cout << "pragmas: noting name '"
                 << (sym->get_name()).str()
                 << "'"
                 << endl;
          pivots->insert(sym);
      }
    }
  } else { /* not a recognized pragma for memory skel: */
      exitWithMsg (stmt, s, "unrecognized arguments");
  }
}

// Traversal to gather all pragmas /////////////////////////////////////////////

class CollectPragmaTargets : public AstSimpleProcessing
{
public:
  // Container of list statements, in order.
  typedef list<SgPragmaDeclaration *> TgtList_t;

  // Call this routine to gather the pragmas.
  static void collect (SgProject* p, TgtList_t& final)
  {
    CollectPragmaTargets collector (final);
    collector.traverseInputFiles (p, postorder);
  }

  virtual void visit (SgNode* n)
  {
    SgPragmaDeclaration* s = isSgPragmaDeclaration(n);
    if (s)
      final_targets_.push_back (s);
  }

private:
  CollectPragmaTargets (TgtList_t& final) : final_targets_ (final) {}
  TgtList_t& final_targets_; // Final list of targets.
};


// Main entry point ////////////////////////////////////////////////////////////

void processPragmas ( SgProject *project
                    , bool outline
                    , SgSymbols *pivots
                    ) {

  // Check that file types are supported:
  if ( ! supportedFileType() ) {
    printf("ERROR: Only C and C++ files are supported.\n");
    exit(1);
  }

  // Build a set of pragma targets.
  CollectPragmaTargets::TgtList_t ts;
  CollectPragmaTargets::collect (project, ts);

  // Process them all.
  for (CollectPragmaTargets::TgtList_t::iterator i = ts.begin ();
       i != ts.end ();
       ++i)
    process1pragma(*i, outline, pivots);
}

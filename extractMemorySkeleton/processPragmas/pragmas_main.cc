#include "rose.h"
#include "processPragmas.h"


int main( int argc, char * argv[] ) {
  // Build the AST used by ROSE
  SgProject* project = frontend(argc,argv);

  // Run internal consistency tests on AST
  AstTests::runAllTests(project);

  // reports
  generatePDF (*project);
  generateDOT (*project);

  // process all 'skel' pragmas
  std::list <std::string> pivots;
  processPragmas (project,false,&pivots);

  // Did we break anything?
  AstTests::runAllTests(project);

  // Generate source code from AST and call the vendor's compiler
  return backend(project);
}

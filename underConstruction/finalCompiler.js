const fs = require('fs');
const lexer = require('./underConstruction/lexer');
const Parser = require('./parser');
const semanticAnalysis = require('./semantic');
const CodeGenerator = require('./codegen');

function compileToJS(code) {
  let tokens, ast, symbolTable, jsCode;

  try {
    tokens = lexer(code);
  } catch (error) {
    throw new Error(`Lexer stage error: ${error.message}`);
  }

  try {
    ast = new Parser(tokens).parse();
  } catch (error) {
    throw new Error(`Parser stage error: ${error.message}`);
  }

  try {
    symbolTable = semanticAnalysis(ast);
  } catch (error) {
    throw new Error(`Semantic Analysis stage error: ${error.message}`);
  }

  try {
    jsCode = new CodeGenerator().generate(ast);
  } catch (error) {
    throw new Error(`Code Generation stage error: ${error.message}`);
  }

  return jsCode;
}

function main() {
  try {
    const cppCode = fs.readFileSync('input.cpp', 'utf8');
    const jsCode = compileToJS(cppCode);
    console.log('\nExecuting the generated code:');
    eval(jsCode);
    if (typeof x !== 'undefined') console.log('Final value of x:', x);
  } catch (error) {
    console.error('Error:', error.message);
  }
}

main();
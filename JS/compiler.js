const fs = require('fs');
const lexer = require('./lexer');

function compileToJS(code) {
  let tokens, ast, symbolTable, jsCode;

  try {
    tokens = lexer(code);
  } catch (error) {
    throw new Error(`Lexer stage error: ${error.message}`);
  }

  

  return tokens;
}

function main() {
  try {
    const cppCode = fs.readFileSync('input.cpp', 'utf8');
    const jsCode = compileToJS(cppCode);
    
  } catch (error) {
    console.error('Error:', error.message);
  }
}

main();
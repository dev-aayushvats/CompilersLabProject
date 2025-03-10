function semanticAnalysis(ast, symbolTable = {}) {
  if (ast.type === 'DECLARATION') {
    symbolTable[ast.value] = 'int';
  } else if (ast.type === 'ASSIGNMENT' || ast.type === 'IDENTIFIER') {
    if (!symbolTable[ast.value]) throw new Error(`Undefined variable: ${ast.value}`);
  }
  ast.children.forEach(child => semanticAnalysis(child, symbolTable));
  console.log('Output from Semantic Analysis stage:', JSON.stringify(symbolTable, null, 2));
  return symbolTable;
}

module.exports = semanticAnalysis;